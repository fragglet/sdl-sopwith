// Emacs style mode select -*- C++ -*-
//---------------------------------------------------------------------------
//
// $Id$
//
// Copyright(C) 2001-2005 Simon Howard
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2 of the License, or (at your
// option) any later version. This program is distributed in the hope that
// it will be useful, but WITHOUT ANY WARRANTY; without even the implied
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
// the GNU General Public License for more details. You should have
// received a copy of the GNU General Public License along with this
// program; if not, write to the Free Software Foundation, Inc., 59 Temple
// Place - Suite 330, Boston, MA 02111-1307, USA.
//
//--------------------------------------------------------------------------
//
// SDL Sound Code
//
// This cleverly imitates a pc speaker using SDL sound routines
//
// Note: I have made this API compatible with a small library I wrote
// for DJGPP to drive the pc speaker, in case a DOS version is ever
// wanted.
//
//---------------------------------------------------------------------------

#include <SDL.h>
#include <math.h>
#include <assert.h>

#include "sw.h"
#include "swsound.h"

#define VOLUME 10 /* out of 127 */
#define FILTER_KERNEL_LEN 51

// The following values give the cutoff range for our band-pass
// filter. These frequencies come from a frequency analysis of
// the LGR video "Playing Silpheed on an IBM 5150 PC".
#define FILTER_LOW_CUTOFF_FREQ  1500 /* hz */
#define FILTER_HI_CUTOFF_FREQ   6000 /* hz */

#define TIMER_FREQ 1193280

struct filter {
	float *kernel;
	size_t kernel_len;
	// Circular buffer of recent samples:
	float *samples;
	int samples_next;
};

int snd_tinnyfilter = 1;
static int output_freq;
static struct filter tinny_filter;
static int speaker_on = 0;
static float current_freq = 0xff;

static float FilterNext(struct filter *f, float sample)
{
	float accum;
	int i, sidx;

	f->samples[f->samples_next] = sample;
	f->samples_next = (f->samples_next + 1) % f->kernel_len;

	accum = 0;
	sidx = f->samples_next;
	for (i = 0; i < f->kernel_len; ++i) {
		accum += f->kernel[i] * f->samples[sidx];
		sidx = (sidx + 1) % f->kernel_len;
	}

	return accum;
}

static float SincFunction(int idx, int kernel_len, float freq)
{
	// Relative to center of filter kernel.
	idx -= (kernel_len - 1) / 2;
	if (idx == 0) {
		return 1 / M_PI;
	}
	return sin(2 * M_PI * freq * idx) / (idx * M_PI);
}

// https://en.wikipedia.org/wiki/Window_function#Blackman_window
static float BlackmanFunction(int idx, int kernel_len)
{
	return 0.42
	     - 0.5 * cos(2 * M_PI * idx / (kernel_len - 1))
	     + 0.08 * cos(4 * M_PI * idx / (kernel_len - 1));
}

// Initialize f as a lowpass filter.
static void MakeLowPassFilter(struct filter *f, float sample_rate,
                              float cutoff)
{
	int i;

	// Kernel length must be an odd number.
	assert((f->kernel_len % 2) == 1);

	f->kernel = calloc(f->kernel_len, sizeof(float));
	assert(f->kernel != NULL);
	f->samples = calloc(f->kernel_len, sizeof(float));
	assert(f->samples != NULL);
	f->samples_next = 0;

	// For a good intro to how this works, read "The Scientist
	// and Engineer's Guide to Digital Signal Processing" chapter 16.
	for (i = 0; i < f->kernel_len; ++i) {
		// The sinc() function gives the inverse fourier
		// transform of an ideal lowpass filter that rejects
		// everything below the cutoff frequency. Frequency
		// is expressed as a ratio of the sample rate, so will
		// always be in the range 0.0-0.5.
		f->kernel[i] = SincFunction(i, f->kernel_len,
		                            cutoff / sample_rate);
		// It is impossible to realise an ideal lowpass filter
		// in practice, and trying to do so produces a
		// frequency response with ringing around the cutoff
		// frequency. To mitigate this we use a windowing
		// function to smooth the frequency response.
		f->kernel[i] *= BlackmanFunction(i, f->kernel_len);
	}
}

// Spectrally invert given filter.
static void InvertFilter(struct filter *f)
{
	float sum;
	int i;

	sum = 0;
	for (i = 0; i < f->kernel_len; ++i) {
		sum += f->kernel[i];
	}
	for (i = 0; i < f->kernel_len; ++i) {
		f->kernel[i] /= sum;
	}

	// From "The Scientist and Engineer's Guide to Digital Signal
	// Processing", chapter 14:
	// > Example of spectral inversion. [...] A high-pass filter kernel is
	// > formed by changing the sign of each sample in (a), and adding one
	// > to the sample at the center of symmetry. This action in the time
	// > domain inverts frequency spectrum (ie. flips it top-for-bottom)
	for (i = 0; i < f->kernel_len; ++i) {
		f->kernel[i] = -f->kernel[i];
	}
	f->kernel[(f->kernel_len - 1) / 2] += 1.0;
}

// Initialize f as a highpass filter.
static void MakeHighPassFilter(struct filter *f, float sample_rate,
                               float cutoff)
{
	MakeLowPassFilter(f, sample_rate, cutoff);

	// See comment in InvertFilter() above.
	// Or in other words: lowpass(f) + highpass(f) = 1.0, so
	// highpass(f) = 1.0 - lowpass(f)
	InvertFilter(f);
}

static void FreeFilter(struct filter *f)
{
	free(f->kernel);
	free(f->samples);
}

static void AddFilters(struct filter *f1, struct filter *f2)
{
	int i;

	assert(f1->kernel_len == f2->kernel_len);
	for (i = 0; i < f1->kernel_len; ++i) {
		f1->kernel[i] += f2->kernel[i];
	}
}

// square wave function for sound generation
static inline float square_wave(float time)
{
	int l = (int) time;
	return time - l < 0.5 ? -0.5 : 0.5;
}

// callback function to generate sound
static void snd_callback(void *userdata, Uint8 * stream, int len)
{
	static int lasttime;
	static float lastfreq;
	float sample;
	int i;

	swsndupdate();

	// lasttime stores the time offset from the last call
	// we save the time so that the multiple time slices
	// all fit together smoothly

	// if we have changed frequency since last time, we need
	// to adjust lasttime to the new frequency

	lasttime *= lastfreq / current_freq;

	for (i = 0; i < len; ++i) {
		if (!speaker_on) {
			sample = 0;
		} else {
			sample = square_wave(current_freq * (i + lasttime)
			                   / output_freq);
		}
		sample = FilterNext(&tinny_filter, sample);
		stream[i] = 128 + (signed int) (sample * VOLUME);
	}

	lasttime += len;
	if (current_freq > 0.1) {
		lasttime = lasttime % (int) (1000 * output_freq / current_freq);
	}
	lastfreq = current_freq;
}

// set the speaker tone
// f is the 'count' sent to the timer chip to specify the frequency
void Speaker_Output(unsigned short count)
{
	if (!count) {
		current_freq = 0;
		speaker_on = 0;
		return;
	}
	speaker_on = 1;
	current_freq = TIMER_FREQ / (float) count;
}

void Speaker_Off(void)
{
	speaker_on = 0;
}

static int sound_initted = 0;

void Speaker_Shutdown(void)
{
	if (!sound_initted) {
		return;
	}

	SDL_QuitSubSystem(SDL_INIT_AUDIO);

	sound_initted = 0;
}

static void InitializeTinnyFilter(unsigned int sample_rate)
{
	struct filter hp_filter;

	// We use a band-pass filter as a model for the PC speaker. Rationale
	// is as follows: the PC speaker is a small, low-quality speaker with
	// no good low-end response; the speaker is usually inside a metal PC
	// case that muffles any high-end response.

	// To get a band-pass filter, we initialize a band-stop filter for
	// our desired frequency range, then spectrally invert it.
	tinny_filter.kernel_len = FILTER_KERNEL_LEN;
	MakeLowPassFilter(&tinny_filter, (float) sample_rate,
	                  FILTER_LOW_CUTOFF_FREQ);

	hp_filter.kernel_len = FILTER_KERNEL_LEN;
	MakeHighPassFilter(&hp_filter, (float) sample_rate,
	                   FILTER_HI_CUTOFF_FREQ);

	AddFilters(&tinny_filter, &hp_filter);
	InvertFilter(&tinny_filter);

	FreeFilter(&hp_filter);
}

static void InitializeNullFilter(void)
{
	static float null_kernel, one_sample;
	tinny_filter.kernel_len = 1;
	tinny_filter.kernel = &null_kernel;
	tinny_filter.samples = &one_sample;
	tinny_filter.samples_next = 0;

	// We want relatively similar volume whether the filter is on or not.
	// The filter makes things quite a bit quieter.
	null_kernel = 0.4;
	one_sample = 0.0;
}


// initialize sound
void Speaker_Init(void)
{
	static SDL_AudioSpec audiospec, audiospec_actual;

	if (sound_initted) {
		return;
	}

	SDL_Init(SDL_INIT_AUDIO);

	audiospec.samples = 1024;
	audiospec.freq = 48000;
	audiospec.format = AUDIO_U8;
	audiospec.channels = 1;
	audiospec.callback = &snd_callback;

	fflush(stdout);

	if (SDL_OpenAudio(&audiospec, &audiospec_actual) < 0) {
		fprintf(stderr, "Failed to initialize sound: %s\n",
			SDL_GetError());
		return;
	}

	output_freq = audiospec_actual.freq;

	sound_initted = 1;
	if (snd_tinnyfilter) {
		InitializeTinnyFilter(audiospec.freq);
	} else {
		InitializeNullFilter();
	}

	SDL_PauseAudio(0);
}

//-----------------------------------------------------------------------
//
// $Log$
// Revision 1.2  2005/04/29 19:25:29  fraggle
// Update copyright to 2005
//
// Revision 1.1.1.1  2003/02/14 19:03:36  fraggle
// Initial Sourceforge CVS import
//
//
// sdh 14/2/2003: change license header to GPL
// sdh 21/10/2001: added cvs tags
//
//-----------------------------------------------------------------------
