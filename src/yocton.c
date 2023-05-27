//
// Copyright (c) 2022, Simon Howard
//
// Permission to use, copy, modify, and/or distribute this software
// for any purpose with or without fee is hereby granted, provided
// that the above copyright notice and this permission notice appear
// in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
// WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
// AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR
// CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
// LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
// NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
// CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <string.h>

#include "yocton.h"

#ifdef ALLOC_TESTING
#include "alloc-testing.h"
#endif

#define ERROR_BUF_SIZE 100

#define ERROR_ALLOC "memory allocation failure"
#define ERROR_EOF   "unexpected EOF"

#define CHECK_OR_RETURN(condition, value) \
	if (!(condition)) { return value; }

struct yocton_buffer {
	uint8_t *data;
	size_t len;
};

enum token_type {
	TOKEN_STRING,
	TOKEN_COLON,
	TOKEN_OPEN_BRACE,
	TOKEN_CLOSE_BRACE,
	TOKEN_EOF,
	TOKEN_ERROR,
};

struct yocton_instream {
	// callback gets invoked to read more data from input.
	yocton_read callback;
	void *callback_handle;
	// buf is the input buffer containing last data read by callback.
	// buf[buf_offset:buf_len] is still to read.
	uint8_t *buf;
	size_t buf_len, buf_size;
	unsigned int buf_offset;
	// string contains the last string token read.
	struct yocton_buffer string;
	size_t string_size;
	// error_buf is non-empty if an error occurs during parsing.
	char *error_buf;
	int lineno;
	struct yocton_object *root;
};

static const uint8_t utf8_bom[] = { 0xef, 0xbb, 0xbf };

static void input_error(struct yocton_instream *s, char *fmt, ...)
{
	va_list args;

	// We only store the first error.
	if (strlen(s->error_buf) > 0) {
		return;
	}
	va_start(args, fmt);
	vsnprintf(s->error_buf, ERROR_BUF_SIZE, fmt, args);
}

// Assign the result of an allocation, storing an error if result == NULL.
static int assign_alloc(void *ptr, struct yocton_instream *s, void *result)
{
	if (result == NULL) {
		input_error(s, ERROR_ALLOC);
		return 0;
	}
	* ((void **) ptr) = result;
	return 1;
}

static int buffer_dup(struct yocton_instream *s, struct yocton_buffer *to,
                      const struct yocton_buffer *from)
{
	CHECK_OR_RETURN(
	    assign_alloc(&to->data, s, malloc(from->len + 1)), 0);
	memcpy(to->data, from->data, from->len);
	to->data[from->len] = '\0';
	to->len = from->len;
	return 1;
}

static int peek_next_byte(struct yocton_instream *s, uint8_t *c)
{
	if (s->buf_offset >= s->buf_len) {
		s->buf_len = s->callback(s->buf, s->buf_size,
		                         s->callback_handle);
		if (s->buf_len == 0) {
			return 0;
		}
		s->buf_offset = 0;
	}
	*c = s->buf[s->buf_offset];
	return 1;
}

// Read next byte from input. Failure stores an error.
static int read_next_byte(struct yocton_instream *s, uint8_t *c)
{
	if (!peek_next_byte(s, c)) {
		input_error(s, ERROR_EOF);
		return 0;
	}
	++s->buf_offset;
	if (*c == '\n') {
		++s->lineno;
	}
	return 1;
}

static int is_symbol_byte(int c)
{
       return isalnum(c) || strchr("_-+.", c) != NULL;
}

static int append_string_byte(struct yocton_instream *s, uint8_t c)
{
	if (s->string.len + 1 >= s->string_size) {
		s->string_size = s->string_size == 0 ? 64 : s->string_size * 2;
		CHECK_OR_RETURN(
		    assign_alloc(&s->string.data, s,
		        realloc(s->string.data, s->string_size)), 0);
	}
	s->string.data[s->string.len] = c;
	++s->string.len;
	return 1;
}

static int read_escape_sequence(struct yocton_instream *s, uint8_t *c)
{
	uint8_t xcs[3];
	CHECK_OR_RETURN(read_next_byte(s, c), 0);
	switch (*c) {
		case 'n':  *c = '\n'; return 1;
		case 't':  *c = '\t'; return 1;
		case '\\': *c = '\\'; return 1;
		case '"':  *c = '\"'; return 1;
		case 'x':
			if (!read_next_byte(s, &xcs[0]) || !isxdigit(xcs[0])
			 || !read_next_byte(s, &xcs[1]) || !isxdigit(xcs[1])) {
				input_error(s, "\\x sequence must be followed "
				            "by two hexadecimal characters");
				return 0;
			}
			xcs[2] = '\0';
			*c = (uint8_t) strtoul((const char *) xcs, NULL, 16);
			if (*c == 0) {
				input_error(s, "NUL byte not allowed in "
				            "\\x escape sequence.");
				return 0;
			}
			if (*c >= 0x20) {
				input_error(s, "\\x escape sequence can only "
				            "be used for control characters "
				            "(ASCII 0x01-0x1f range)");
				return 0;
			}
			return 1;
		default:
			input_error(s, "unknown string escape: \\%c", *c);
			return 0;
	}
}

// Read quote-delimited "C style" string.
static enum token_type read_string(struct yocton_instream *s)
{
	uint8_t c;
	s->string.len = 0;
	for (;;) {
		CHECK_OR_RETURN(read_next_byte(s, &c), TOKEN_ERROR);
		if (c == '"') {
			return TOKEN_STRING;
		} else if (c == '\\') {
			if (!read_escape_sequence(s, &c)) {
				return TOKEN_ERROR;
			}
		} else if (c < 0x20) {
			input_error(s, "control character not allowed inside "
			            "string (ASCII char 0x%02x)", c);
			return TOKEN_ERROR;
		}
		CHECK_OR_RETURN(append_string_byte(s, c), TOKEN_ERROR);
	}
}

static enum token_type read_symbol(struct yocton_instream *s, uint8_t first)
{
	uint8_t c;
	if (!is_symbol_byte(first)) {
		input_error(s, "unknown token: not valid symbol character");
		return TOKEN_ERROR;
	}
	s->string.len = 0;
	CHECK_OR_RETURN(append_string_byte(s, first), TOKEN_ERROR);
	// Reaching EOF in the middle of the string is explicitly okay here:
	while (peek_next_byte(s, &c) && is_symbol_byte(c)) {
		CHECK_OR_RETURN(read_next_byte(s, &c), TOKEN_ERROR);
		CHECK_OR_RETURN(append_string_byte(s, c), TOKEN_ERROR);
	}
	return TOKEN_STRING;
}

static enum token_type read_next_token(struct yocton_instream *s)
{
	uint8_t c, c2, c3;
	if (strlen(s->error_buf) > 0) {
		return TOKEN_ERROR;
	}
	// Skip past any spaces. Reaching EOF is not always an error.
	do {
		if (!peek_next_byte(s, &c)) {
			return TOKEN_EOF;
		}
		CHECK_OR_RETURN(read_next_byte(s, &c), TOKEN_ERROR);
		// If we encounter a comment we skip past it. Note that
		// ending with EOF is also okay.
		if (c == '/' && peek_next_byte(s, &c2) && c2 =='/') {
			while (peek_next_byte(s, &c) && c != '\n') {
				CHECK_OR_RETURN(read_next_byte(s, &c),
				                TOKEN_ERROR);
			}
			c = ' ';
		} else if (c == utf8_bom[0]) {
			CHECK_OR_RETURN(
			    read_next_byte(s, &c2) && c2 == utf8_bom[1]
			 && read_next_byte(s, &c3) && c3 == utf8_bom[2],
			    TOKEN_ERROR);
			c = ' ';
		}
	} while (isspace(c));

	switch (c) {
		case ':':  return TOKEN_COLON;
		case '{':  return TOKEN_OPEN_BRACE;
		case '}':  return TOKEN_CLOSE_BRACE;
		case '\"': return read_string(s);
		default:   return read_symbol(s, c);
	}
}

struct yocton_object {
	struct yocton_instream *instream;
	struct yocton_field *field;
	int done;
};

struct yocton_field {
	enum yocton_field_type type;
	struct yocton_buffer name, value;
	struct yocton_object *parent, *child;
};

static void free_obj(struct yocton_object *obj);

static void free_field(struct yocton_field *field)
{
	if (field == NULL) {
		return;
	}
	free_obj(field->child);
	field->child = NULL;

	free(field->name.data);
	free(field->value.data);
	free(field);
}

static void free_obj(struct yocton_object *obj)
{
	if (obj == NULL) {
		return;
	}
	free_field(obj->field);
	obj->field = NULL;
	free(obj);
}

static void free_instream(struct yocton_instream *instream)
{
	if (instream == NULL) {
		return;
	}
	free(instream->buf);
	free(instream->error_buf);
	free(instream->string.data);
	free(instream);
}

static int init_instream(struct yocton_instream *instream)
{
	instream->lineno = 1;
	instream->buf_len = 0;
	instream->buf_offset = 0;
	instream->buf_size = 256;
	instream->error_buf = (char *) calloc(ERROR_BUF_SIZE, 1);
	CHECK_OR_RETURN(instream->error_buf != NULL, 0);
	instream->buf =
	    (uint8_t *) calloc(instream->buf_size, sizeof(uint8_t));
	CHECK_OR_RETURN(instream->buf != NULL, 0);
	instream->string_size = 0;
	instream->string.data = NULL;

	return 1;
}

static struct yocton_instream *new_instream(yocton_read callback, void *handle)
{
	struct yocton_instream *instream = NULL;

	instream = (struct yocton_instream *)
	    calloc(1, sizeof(struct yocton_instream));
	CHECK_OR_RETURN(instream != NULL, NULL);

	if (!init_instream(instream)) {
		free_instream(instream);
		return NULL;
	}

	instream->callback = callback;
	instream->callback_handle = handle;
	return instream;
}

static size_t fread_wrapper(void *buf, size_t buf_size, void *handle)
{
	return fread(buf, 1, buf_size, (FILE *) handle);
}

struct yocton_object *yocton_read_from(FILE *fstream)
{
	return yocton_read_with(fread_wrapper, fstream);
}

struct yocton_object *yocton_read_with(yocton_read callback, void *handle)
{
	struct yocton_object *obj = NULL;

	obj = (struct yocton_object *) calloc(1, sizeof(struct yocton_object));
	CHECK_OR_RETURN(obj != NULL, NULL);

	obj->instream = new_instream(callback, handle);
	if (obj->instream == NULL) {
		free_obj(obj);
		return NULL;
	}

	obj->field = NULL;
	obj->done = 0;
	obj->instream->root = obj;

	return obj;
}

int yocton_have_error(struct yocton_object *obj, int *lineno,
                      const char **error_msg)
{
	if (strlen(obj->instream->error_buf) == 0) {
		return 0;
	}
	if (lineno != NULL) {
		*lineno = obj->instream->lineno;
	}
	if (error_msg != NULL) {
		*error_msg = obj->instream->error_buf;
	}
	return 1;
}

void yocton_check(struct yocton_object *obj, const char *error_msg,
                  int normally_true)
{
	if (!normally_true) {
		if (obj->field != NULL) {
			input_error(obj->instream, "field '%s': %s",
			            obj->field->name.data, error_msg);
		} else {
			input_error(obj->instream, "%s", error_msg);
		}
	}
}

void yocton_free(struct yocton_object *obj)
{
	if (obj != obj->instream->root) {
		return;
	}

	free_instream(obj->instream);
	free_obj(obj);
}

// If we're partway through reading a child object, skip through any
// of its fields so we can read the next of ours.
static void skip_forward(struct yocton_object *obj)
{
	struct yocton_object *child;
	if (obj->field == NULL || obj->field->child == NULL) {
		return;
	}
	child = obj->field->child;
	// Read out all subfields until we get a NULL response and have
	// finished skipping over them.
	while (yocton_next_field(child) != NULL);
	free_obj(child);
	obj->field->child = NULL;
}

static int parse_next_field(struct yocton_object *obj, struct yocton_field *f)
{
	switch (read_next_token(obj->instream)) {
		case TOKEN_COLON:
			// This is the string:string case.
			f->type = YOCTON_FIELD_STRING;
			if (read_next_token(obj->instream) != TOKEN_STRING) {
				input_error(obj->instream, "string expected "
				            "to follow ':'");
				return 0;
			}
			CHECK_OR_RETURN(
			    buffer_dup(obj->instream, &f->value,
			               &obj->instream->string), 0);
			return 1;
		case TOKEN_OPEN_BRACE:
			f->type = YOCTON_FIELD_OBJECT;
			CHECK_OR_RETURN(
			    assign_alloc(&f->child, obj->instream,
			        calloc(1, sizeof(struct yocton_object))), 0);
			f->child->instream = obj->instream;
			f->child->done = 0;
			return 1;
		default:
			input_error(obj->instream, "':' or '{' expected to "
			            "follow field name");
			return 0;
	}
}

static struct yocton_field *next_field(struct yocton_object *obj)
{
	struct yocton_field *f = NULL;

	CHECK_OR_RETURN(
	    assign_alloc(&f, obj->instream,
	        calloc(1, sizeof(struct yocton_field))), NULL);
	obj->field = f;
	f->parent = obj;

	if (!buffer_dup(obj->instream, &f->name, &obj->instream->string)
	 || !parse_next_field(obj, f)) {
		free_field(f);
		obj->field = NULL;
		return NULL;
	}

	return f;
}

struct yocton_field *yocton_next_field(struct yocton_object *obj)
{
	if (obj == NULL || obj->done || strlen(obj->instream->error_buf) > 0) {
		return NULL;
	}

	skip_forward(obj);
	free_field(obj->field);
	obj->field = NULL;

	switch (read_next_token(obj->instream)) {
		case TOKEN_STRING:
			return next_field(obj);
		case TOKEN_CLOSE_BRACE:
			if (obj == obj->instream->root) {
				input_error(obj->instream, "closing brace "
				            "'}' not expected at top level");
				return NULL;
			}
			obj->done = 1;
			return NULL;
		case TOKEN_EOF:
			// EOF is only valid at the top level.
			if (obj != obj->instream->root) {
				input_error(obj->instream, ERROR_EOF);
				return NULL;
			}
			obj->done = 1;
			return NULL;
		default:
			input_error(obj->instream, "expected start of "
			            "next field");
			return NULL;
	}
}

enum yocton_field_type yocton_field_type(struct yocton_field *f)
{
	return f->type;
}

const char *yocton_field_name(struct yocton_field *f)
{
	return (const char *) f->name.data;
}

const char *yocton_field_value(struct yocton_field *f)
{
	if (f->type != YOCTON_FIELD_STRING) {
		input_error(f->parent->instream, "field '%s' has object, "
		            "not string type", f->name.data);
		return "";
	}
	return (const char *) f->value.data;
}

struct yocton_object *yocton_field_inner(struct yocton_field *f)
{
	if (f->type != YOCTON_FIELD_OBJECT) {
		input_error(f->parent->instream, "field '%s' has string, "
		            "not object type", f->name.data);
		return NULL;
	}
	return f->child;
}

signed long long yocton_field_int(struct yocton_field *f, size_t n)
{
	signed long long result, min, max;
	const char *value;
	char *endptr;

	if (n == 0 || n > sizeof(long long)) {
		input_error(f->parent->instream, "unsupported "
		            "integer size: %d-bit", n * 8);
		return 0;
	} else if (n == sizeof(long long)) {
		min = LLONG_MIN;
		max = LLONG_MAX;
	} else {
		min = -(1ULL << (n * 8 - 1));
		max = -min - 1;
	}

	value = yocton_field_value(f);
	errno = 0;
	result = strtoll(value, &endptr, 10);
	// Must be entire string, not empty, nothing leading or trailing:
	if (*value == '\0' || isspace(*value) || *endptr != '\0') {
		input_error(f->parent->instream, "not a valid integer "
		            "value: '%s'", value);
		return 0;
	}

	if (((result == LLONG_MIN || result == LLONG_MAX) && errno == ERANGE)
	 || result < min || result > max) {
		input_error(f->parent->instream, "value not in range of a "
		            "%d-bit signed integer: %s", n * 8, value);
		return 0;
	}
	return result;
}

unsigned long long yocton_field_uint(struct yocton_field *f, size_t n)
{
	unsigned long long result, max;
	const char *value;
	char *endptr;

	if (n == 0 || n > sizeof(unsigned long long)) {
		input_error(f->parent->instream, "unsupported "
		            "integer size: %d-bit", n * 8);
		return 0;
	} else if (n == sizeof(unsigned long long)) {
		max = ULLONG_MAX;
	} else {
		max = (1ULL << (n * 8)) - 1;
	}

	value = yocton_field_value(f);
	errno = 0;
	result = strtoull(value, &endptr, 10);
	if (*value == '\0' || isspace(*value) || *endptr != '\0') {
		input_error(f->parent->instream, "not a valid integer "
		            "value: '%s'", value);
		return 0;
	}

	if ((result == ULLONG_MAX && errno == ERANGE) || result > max) {
		input_error(f->parent->instream, "value not in range of a "
		            "%d-bit unsigned integer: %s", n * 8, value);
		return 0;
	}
	return result;
}

unsigned int yocton_field_enum(struct yocton_field *f, const char **values)
{
	const char *value = yocton_field_value(f);
	int i;

	for (i = 0; values[i] != NULL; ++i) {
		if (!strcmp(values[i], value)) {
			return i;
		}
	}

	// Unknown value.
	input_error(f->parent->instream, "unknown enum value: '%s'",
	            value);
	return 0;
}
