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
	TOKEN_NONE,
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
	int token_lineno;
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

// Peek ahead in the input stream. If there are any space characters or
// comments, skip ahead and ignore them. On success, TOKEN_NONE is returned
// and the following character is not consumed.
static enum token_type skip_past_spaces(struct yocton_instream *s, uint8_t *c)
{
	uint8_t c2, c3;

	// Skip past any spaces. Reaching EOF is not always an error.
	while (peek_next_byte(s, c)) {
		if (*c == '/') {
			// Skip past comment.
			CHECK_OR_RETURN(
			    read_next_byte(s, c) && *c == '/'
			 && read_next_byte(s, &c2) && c2 == '/',
			    TOKEN_ERROR);
			while (peek_next_byte(s, c) && *c != '\n') {
				CHECK_OR_RETURN(read_next_byte(s, c),
				                TOKEN_ERROR);
			}
		} else if (*c == utf8_bom[0]) {
			CHECK_OR_RETURN(
			    read_next_byte(s, c) && *c == utf8_bom[0]
			 && read_next_byte(s, &c2) && c2 == utf8_bom[1]
			 && read_next_byte(s, &c3) && c3 == utf8_bom[2],
			    TOKEN_ERROR);
		} else if (isspace(*c)) {
			CHECK_OR_RETURN(read_next_byte(s, c), TOKEN_ERROR);
		} else {
			return TOKEN_NONE;
		}
	}

	return TOKEN_EOF;
}

// Called at end of a string chunk ('"') to skip forward and see if there is
// another chunk separated by an '&'. For concatenated/multiline strings.
static enum token_type next_string_chunk(struct yocton_instream *s)
{
	enum token_type tt;
	uint8_t c;

	tt = skip_past_spaces(s, &c);
	if (tt == TOKEN_EOF) {
		return TOKEN_STRING;
	} else if (tt != TOKEN_NONE) {
		return tt;
	} else if (c != '&') {
		return TOKEN_STRING;
	}
	CHECK_OR_RETURN(read_next_byte(s, &c), TOKEN_ERROR);
	s->token_lineno = s->lineno;

	// We have read the '&' joining operator. Skip ahead to the next '"';
	// it is an error now if we don't reach one.
	if (skip_past_spaces(s, &c) != TOKEN_NONE || c != '"') {
		input_error(s, "quoted string should follow '&' operator");
		return TOKEN_ERROR;
	}
	CHECK_OR_RETURN(read_next_byte(s, &c), TOKEN_ERROR);
	s->token_lineno = s->lineno;

	return TOKEN_NONE;
}

// Read quote-delimited "C style" string.
static enum token_type read_string(struct yocton_instream *s)
{
	uint8_t c;
	s->string.len = 0;
	for (;;) {
		CHECK_OR_RETURN(read_next_byte(s, &c), TOKEN_ERROR);
		if (c == '"') {
			enum token_type tt = next_string_chunk(s);
			if (tt != TOKEN_NONE) {
				return tt;
			}
			continue;
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
	uint8_t c;
	enum token_type result;

	if (strlen(s->error_buf) > 0) {
		return TOKEN_ERROR;
	}
	result = skip_past_spaces(s, &c);
	if (result != TOKEN_NONE) {
		// Line number where error/EOF was encountered.
		s->token_lineno = s->lineno;
		return result;
	}
	CHECK_OR_RETURN(read_next_byte(s, &c), TOKEN_EOF);

	s->token_lineno = s->lineno;
	switch (c) {
		case ':':  return TOKEN_COLON;
		case '{':  return TOKEN_OPEN_BRACE;
		case '}':  return TOKEN_CLOSE_BRACE;
		case '\"': return read_string(s);
		case '&':
			input_error(s, "'&' operator can only be used to "
			            "join quoted strings");
			return TOKEN_ERROR;
		default:   return read_symbol(s, c);
	}
}

struct yocton_object {
	struct yocton_instream *instream;
	struct yocton_prop *property;
	int done;
};

struct yocton_prop {
	enum yocton_prop_type type;
	struct yocton_buffer name, value;
	struct yocton_object *parent, *child;
};

static void free_obj(struct yocton_object *obj);

static void free_property(struct yocton_prop *property)
{
	if (property == NULL) {
		return;
	}
	free_obj(property->child);
	property->child = NULL;

	free(property->name.data);
	free(property->value.data);
	free(property);
}

static void free_obj(struct yocton_object *obj)
{
	if (obj == NULL) {
		return;
	}
	free_property(obj->property);
	obj->property = NULL;
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

	obj->property = NULL;
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
		*lineno = obj->instream->token_lineno;
	}
	if (error_msg != NULL) {
		*error_msg = obj->instream->error_buf;
	}
	return 1;
}

int __yocton_prop_have_error(struct yocton_prop *p)
{
	return yocton_have_error(p->parent, NULL, NULL);
}

void yocton_check(struct yocton_object *obj, const char *error_msg,
                  int normally_true)
{
	if (!normally_true) {
		if (obj->property != NULL) {
			input_error(obj->instream, "property '%s': %s",
			            obj->property->name.data, error_msg);
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
// of its properties so we can read the next of ours.
static void skip_forward(struct yocton_object *obj)
{
	struct yocton_object *child;
	if (obj->property == NULL || obj->property->child == NULL) {
		return;
	}
	child = obj->property->child;
	// Read out all subproperties until we get a NULL response and have
	// finished skipping over them.
	while (yocton_next_prop(child) != NULL);
	free_obj(child);
	obj->property->child = NULL;
}

static int parse_next_prop(struct yocton_object *obj, struct yocton_prop *p)
{
	switch (read_next_token(obj->instream)) {
		case TOKEN_COLON:
			// This is the string:string case.
			p->type = YOCTON_PROP_STRING;
			if (read_next_token(obj->instream) != TOKEN_STRING) {
				input_error(obj->instream, "string expected "
				            "to follow ':'");
				return 0;
			}
			CHECK_OR_RETURN(
			    buffer_dup(obj->instream, &p->value,
			               &obj->instream->string), 0);
			return 1;
		case TOKEN_OPEN_BRACE:
			p->type = YOCTON_PROP_OBJECT;
			CHECK_OR_RETURN(
			    assign_alloc(&p->child, obj->instream,
			        calloc(1, sizeof(struct yocton_object))), 0);
			p->child->instream = obj->instream;
			p->child->done = 0;
			return 1;
		default:
			input_error(obj->instream, "':' or '{' expected to "
			            "follow property name");
			return 0;
	}
}

static struct yocton_prop *next_prop(struct yocton_object *obj)
{
	struct yocton_prop *p = NULL;

	CHECK_OR_RETURN(
	    assign_alloc(&p, obj->instream,
	        calloc(1, sizeof(struct yocton_prop))), NULL);
	obj->property = p;
	p->parent = obj;

	if (!buffer_dup(obj->instream, &p->name, &obj->instream->string)
	 || !parse_next_prop(obj, p)) {
		free_property(p);
		obj->property = NULL;
		return NULL;
	}

	return p;
}

struct yocton_prop *yocton_next_prop(struct yocton_object *obj)
{
	if (obj == NULL || obj->done || strlen(obj->instream->error_buf) > 0) {
		return NULL;
	}

	skip_forward(obj);
	free_property(obj->property);
	obj->property = NULL;

	switch (read_next_token(obj->instream)) {
		case TOKEN_STRING:
			return next_prop(obj);
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
			            "next property");
			return NULL;
	}
}

enum yocton_prop_type yocton_prop_type(struct yocton_prop *p)
{
	return p->type;
}

const char *yocton_prop_name(struct yocton_prop *p)
{
	return (const char *) p->name.data;
}

const char *yocton_prop_value(struct yocton_prop *p)
{
	if (p->type != YOCTON_PROP_STRING) {
		input_error(p->parent->instream, "property '%s' has object, "
		            "not string type", p->name.data);
		return "";
	}
	return (const char *) p->value.data;
}

char *yocton_prop_value_dup(struct yocton_prop *p)
{
	const char *value = yocton_prop_value(p);
	char *result;
	if (value == NULL) {
		return NULL;
	}
	result = strdup(value);
	yocton_check(p->parent, ERROR_ALLOC, result != NULL);
	return result;
}

struct yocton_object *yocton_prop_inner(struct yocton_prop *p)
{
	if (p->type != YOCTON_PROP_OBJECT) {
		input_error(p->parent->instream, "property '%s' has string, "
		            "not object type", p->name.data);
		return NULL;
	}
	return p->child;
}

signed long long yocton_prop_int(struct yocton_prop *p, size_t n)
{
	signed long long result, min, max;
	const char *value;
	char *endptr;

	if (n == 0 || n > sizeof(long long)) {
		input_error(p->parent->instream, "unsupported "
		            "integer size: %d-bit", n * 8);
		return 0;
	} else if (n == sizeof(long long)) {
		min = LLONG_MIN;
		max = LLONG_MAX;
	} else {
		min = -(1ULL << (n * 8 - 1));
		max = -min - 1;
	}

	value = yocton_prop_value(p);
	errno = 0;
	result = strtoll(value, &endptr, 10);
	// Must be entire string, not empty, nothing leading or trailing:
	if (*value == '\0' || isspace(*value) || *endptr != '\0') {
		input_error(p->parent->instream, "not a valid integer "
		            "value: '%s'", value);
		return 0;
	}

	if (((result == LLONG_MIN || result == LLONG_MAX) && errno == ERANGE)
	 || result < min || result > max) {
		input_error(p->parent->instream, "value not in range of a "
		            "%d-bit signed integer: %s", n * 8, value);
		return 0;
	}
	return result;
}

unsigned long long yocton_prop_uint(struct yocton_prop *p, size_t n)
{
	unsigned long long result, max;
	const char *value;
	char *endptr;

	if (n == 0 || n > sizeof(unsigned long long)) {
		input_error(p->parent->instream, "unsupported "
		            "integer size: %d-bit", n * 8);
		return 0;
	} else if (n == sizeof(unsigned long long)) {
		max = ULLONG_MAX;
	} else {
		max = (1ULL << (n * 8)) - 1;
	}

	value = yocton_prop_value(p);
	errno = 0;
	result = strtoull(value, &endptr, 10);
	if (*value == '\0' || isspace(*value) || *endptr != '\0') {
		input_error(p->parent->instream, "not a valid integer "
		            "value: '%s'", value);
		return 0;
	}

	if ((result == ULLONG_MAX && errno == ERANGE) || result > max) {
		input_error(p->parent->instream, "value not in range of a "
		            "%d-bit unsigned integer: %s", n * 8, value);
		return 0;
	}
	return result;
}

unsigned int yocton_prop_enum(struct yocton_prop *p, const char **values)
{
	const char *value = yocton_prop_value(p);
	int i;

	for (i = 0; values[i] != NULL; ++i) {
		if (!strcmp(values[i], value)) {
			return i;
		}
	}

	// Unknown value.
	input_error(p->parent->instream, "unknown enum value: '%s'",
	            value);
	return 0;
}

// Helper function for array macros. Reallocates the given array one element
// longer so that it can be (potentially) extended in length.
int __yocton_reserve_array(struct yocton_prop *p, void **array, size_t nmemb,
                           size_t size)
{
	void *new_array;

	if (nmemb == 0) {
		*array = NULL;
	}
	new_array = realloc(*array, (nmemb + 1) * size);
	if (new_array == NULL) {
		input_error(p->parent->instream, ERROR_ALLOC);
		return 0;
	}

	*array = new_array;
	return 1;
}

int __yocton_prop_alloc(struct yocton_prop *p, void **ptr, size_t size)
{
	if (*ptr != NULL) {
		input_error(p->parent->instream, "pointer is non-NULL; "
		            "property may be duplicated unexpectedly");
		return 0;
	}

	*ptr = calloc(1, size);
	if (*ptr == NULL) {
		input_error(p->parent->instream, ERROR_ALLOC);
		return 0;
	}

	return 1;
}
