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

#ifndef YOCTON_H
#define YOCTON_H

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>

/**
 * @file yocton.h
 *
 * Functions for parsing the contents of a Yocton file. The entrypoint
 * for reading is to use @ref yocton_read_with or @ref yocton_read_from.
 */

/**
 * Callback invoked to read more data from the input.
 *
 * @param buf       Buffer to populate with new data.
 * @param buf_size  Size of buffer in bytes.
 * @param handle    Arbitrary pointer, passed through from
 *                  @ref yocton_read_with.
 * @return          Number of bytes written into the buffer, or zero to
 *                  indicate end of file.
 */
typedef size_t (*yocton_read)(void *buf, size_t buf_size, void *handle);

/** Type of a @ref yocton_field. */
enum yocton_field_type {
	/**
	 * Field that has a string value. @ref yocton_field_value can be
	 * used to get the value.
	 */
	YOCTON_FIELD_STRING,

	/**
	 * Field that has an object value. @ref yocton_field_inner can be
	 * used to read the inner object.
	 */
	YOCTON_FIELD_OBJECT,
};

struct yocton_object;
struct yocton_field;

#ifdef __DOXYGEN__

/**
 * The object is the main abstraction of the Yocton format. Each object
 * can have multiple fields (@ref yocton_field), which can themselves
 * contain more objects.
 */
typedef struct yocton_object yocton_object;

/**
 * An object has multiple fields. Each field has a name which is always
 * a string. It also always has a value, which is either a string
 * (@ref YOCTON_FIELD_STRING) or an object (@ref YOCTON_FIELD_OBJECT).
 * Fields only have a very limited lifetime and are only valid until
 * @ref yocton_next_field is called to read the next field of their
 * parent object.
 */
typedef struct yocton_field yocton_field;

#endif

/**
 * Start reading a new stream of yocton-encoded data, using the given
 * callback to read more data.
 *
 * @param callback   Callback function to invoke to read more data.
 * @param handle     Arbitrary pointer passed through when callback is
 *                   invoked.
 * @return           A @ref yocton_object representing the top-level object.
 */
struct yocton_object *yocton_read_with(yocton_read callback, void *handle);

/**
 * Start reading a new stream of yocton-encoded data, using the given
 * FILE handle to read more data.
 *
 * @param fstream    File handle.
 * @return           A @ref yocton_object representing the top-level object.
 */
struct yocton_object *yocton_read_from(FILE *fstream);

/**
 * Query whether an error occurred during parsing. This should be called
 * once no more data is returned from obj (ie. when @ref yocton_next_field
 * returns NULL for the top-level object).
 *
 * @param obj        Top-level @ref yocton_object.
 * @param lineno     If an error occurs and this is not NULL, the line number
 *                   on which the error occurred is saved to the pointer
 *                   address.
 * @param error_msg  If an error occurs and this is not NULL, an error message
 *                   describing the error is saved to the pointer address.
 * @return           Non-zero if an error occurred.
 */
int yocton_have_error(struct yocton_object *obj, int *lineno,
                      const char **error_msg);

/**
 * Free the top-level object and stop reading from the input stream.
 *
 * @param obj        Top-level @ref yocton_object.
 */
void yocton_free(struct yocton_object *obj);

/**
 * Perform an assertion and fail with an error if it isn't true.
 *
 * @param obj            @ref yocton_object; may or may not be the top-level
 *                       object.
 * @param error_msg      The error message to log if normally_true is zero.
 * @param normally_true  If this is zero, an error is logged.
 */
void yocton_check(struct yocton_object *obj, const char *error_msg,
                  int normally_true);

/**
 * Read the next field of an object.
 *
 * @param obj  @ref yocton_object to read from.
 * @return     @ref yocton_field or NULL if there are no more fields to be
 *             read. NULL is also returned if an error occurs in parsing
 *             the input; @ref yocton_have_error should be used to
 *             distinguish the two. If a field is returned, it is only
 *             valid until the next field is read from the same object.
 */
struct yocton_field *yocton_next_field(struct yocton_object *obj);

/**
 * Get the type of a @ref yocton_field.
 *
 * @param f  The field.
 * @return   Type of the field.
 */
enum yocton_field_type yocton_field_type(struct yocton_field *f);

/**
 * Get the name of a @ref yocton_field. Multiple fields of the same object
 * may have the same name. Encoding of the name depends on the encoding of
 * the input file.
 *
 * @param f  The field.
 * @return   Name of the field. The returned string is only valid for the
 *           lifetime of the field itself.
 */
const char *yocton_field_name(struct yocton_field *f);

/**
 * Get the string value of a @ref yocton_field of type
 * @ref YOCTON_FIELD_STRING. It is an error to call this for a field that
 * is not of this type. Encoding of the string depends on the input file.
 *
 * @param f  The field.
 * @return   String value of this field, or NULL if it is not a field of
 *           type @ref YOCTON_FIELD_STRING. The returned string is only
 *           valid for the lifetime of the field itself.
 */
const char *yocton_field_value(struct yocton_field *f);

/**
 * Get the inner object associated with a @ref yocton_field of type
 * @ref YOCTON_FIELD_OBJECT. It is an error to call this for a field that
 * is not of this type.
 *
 * @param f  The field.
 * @return   Inner @ref yocton_object, or NULL if the field is not of type
 *           @ref YOCTON_FIELD_OBJECT. The returned object is only only
 *           valid for the lifetime of the field itself.
 */
struct yocton_object *yocton_field_inner(struct yocton_field *f);

/**
 * Parse the field value as a signed integer.
 *
 * If the field value is not a valid integer of the given size, zero is
 * returned and an error is set.
 *
 * @param f   The field.
 * @param n   Size of the expected field in bytes, eg. sizeof(uint16_t).
 * @return    The integer value, or zero if it cannot be parsed as an
 *            integer of that size. Although the return value is a long
 *            long type, it will always be in the range of an integer
 *            of the given size and can be safely cast to one.
 */
signed long long yocton_field_int(struct yocton_field *f, size_t n);

#define YOCTON_FIELD_INT(field, my_struct, field_type, name) \
	do { \
		if (!strcmp(yocton_field_name(field), #name)) { \
			(my_struct).name = (field_type) \
				yocton_field_int(field, sizeof(field_type)); \
		} \
	} while (0)

/**
 * Parse the field value as a unsigned integer.
 *
 * If the field value is not a valid integer of the given size, zero is
 * returned and an error is set.
 *
 * @param f   The field.
 * @param n   Size of the expected field in bytes, eg. sizeof(uint16_t).
 * @return    The integer value, or zero if it cannot be parsed as an
 *            signed integer of that size. Although the return value is a
 *            long long type, it will always be in the range of an integer
 *            of the given size and can be safely cast to one.
 */
unsigned long long yocton_field_uint(struct yocton_field *f, size_t n);

#define YOCTON_FIELD_UINT(field, my_struct, field_type, name) \
	do { \
		if (!strcmp(yocton_field_name(field), #name)) { \
			(my_struct).name = (field_type) \
				yocton_field_uint(field, sizeof(field_type)); \
		} \
	} while (0)

#ifdef __cplusplus
}
#endif

#endif /* #ifndef YOCTON_H */

