/*
 * Copyright 2023 Humberto Gomes, José Lopes, José Matos
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file  ncurses_utils.h
 * @brief Utilities for ncurses.
 *
 * @anchor ncurses_utils_examples
 * ### Examples
 *
 * #### Rectangle rendering
 *
 * For example, `ncurses_render_rectangle(1, 1, 5, 5)` generates the following rectangle:
 *
 * ```text
 * +-----+
 * |     |
 * |     |
 * |     |
 * |     |
 * |     |
 * +-----+
 * ```
 *
 * The internal area of the rectangle is `5 x 5`, but the rectangle becomes `7 x 7` when its borders
 * are considered. The internal rectangle area starts spans from `(1, 1)` until `(5, 5)`. However,
 * the rectangle's border touches the top-left corner of the screen (`(0, 0)` to `(7, 7)`).
 *
 * #### String measurement
 *
 * Here is an example of the widths of different characters. `g_utf8_get_char` in this example, for
 * extracting Unicode characters from a string.
 *
 *  - `ncurses_measure_character(g_utf8_get_char("G"))` returns 1;
 *  - `ncurses_measure_character(g_utf8_get_char("命"))` returns 2;
 *
 * UTF-8 strings can be measured using ::ncurses_measure_string:
 *
 *  - `ncurses_measure_string("Life!")` returns 5;
 *  - `ncurses_measure_string("命!")` returns 3, as 命 is a double-width character;
 *
 * If you have a UTF-32 string, please use ::ncurses_measure_unicode_string.
 *
 * #### String width limiting
 *
 * Before rendering a string in a limited space, you might want to know how many characters you
 * can draw before overflowing the drawing area. That can be done using the following methods:
 *
 * - ::ncurses_prefix_from_maximum_length calculates the width of the longest drawable prefix;
 * - ::ncurses_suffix_from_maximum_length calculates the width of the longest drawable suffix.
 *
 * Suppose you can only draw 3 monospace characters and need to render the string `"花火!"`. We can
 * convert it to UTF-32 in the following way:
 *
 * ```c
 * glong length;
 * size_t width;
 * gunichar *utf32 = g_utf8_to_ucs4_fast("花火!", -1, &length);
 * ```
 *
 * - `ncurses_prefix_from_maximum_length(utf32, length, &width)` will return 1, referring to the
 *   length of the prefix `"花"`, which is 2 characters wide, value that will be written to width.
 *
 * - `ncurses_suffix_from_maximum_length(utf32, length, &width)` will return 2, referring to the
 *   length of the suffix `"火!"`, which is 3 characters wide, value that will be written to width.
 *
 * Don't forget to call `g_free` for `utf32` in the end.
 */

#include <glib.h>
#include <inttypes.h>

/**
 * @brief   Renders a rectangle using ncurses.
 * @details The position and size of the rendered rectangle do not include its border, which is one
 *          character thick. Any partially out-of-bounds rectangle (including border) will result
 *          in undefined behavior.
 *
 * @param x      Horizontal position of the rectangle.
 * @param y      Vertical position of the rectangle.
 * @param width  Horizontal size of the rectangle.
 * @param height Vertical size of the rectangle.
 *
 * #### Examples
 * See [the header file's documentation](@ref ncurses_utils_examples).
 */
void ncurses_render_rectangle(int x, int y, int width, int height);

/**
 * @brief   Measures the width of a unicode codepoint (interpreted as a single glyph) for ncurses
 *          rendering.
 * @details This doesn't always return `1`, as there are both `0`-width and double-width characters.
 *          Examples of this are unprintable characters and most CJK characters, respectively.
 *
 * @param c Character to be measured.
 *
 * @return Width of the character, in relation to a single-width character. A monospace font is
 *         assumed.
 *
 * #### Examples
 * See [the header file's documentation](@ref ncurses_utils_examples).
 */
int8_t ncurses_measure_character(gunichar c);

/**
 * @brief   Measures the width of a null-terminated UTF-32 string for `ncurses` rendering.
 * @details This is different from the length of the string (number of codepoints), for reasons
 *          described ::ncurses_measure_character. Emojis (or any other multi-codepoint character
 *          not formed by diacritics) aren't supported. Multi-line strings aren't supported as well.
 *
 *          This function is somewhat slow, as it requires lots of lookups in Unicode tables. Try to
 *          keep its use to a minimum.
 *
 * @param str Null-terminated UTF-32 string to have its width measured.
 *
 * @return The width of @p str, in relation to a single-width character. A monospace font is
 *         assumed.
 *
 * #### Examples
 * See [the header file's documentation](@ref ncurses_utils_examples).
 */
size_t ncurses_measure_unicode_string(const gunichar *str);

/**
 * @brief   Measures the width of a null-terminated UTF-8 string for `ncurses` rendering.
 * @details This is different from the length of the string in bytes. First, there can be multi-byte
 *          characters. Secondly, it's not the same as the number of codepoints for reasons
 *          described ::ncurses_measure_character. Emojis (or any other multi-codepoint character
 *          not formed by diacritics) aren't supported. Multi-line strings aren't supported as well.
 *
 *          This function is somewhat slow, as it requires allocations and lots of lookups in
 *          Unicode tables. Try to keep its use to a minimum.
 *
 * @param str Null-terminated UTF-8 string to have its width measured.
 *
 * @return The width of @p str, in relation to a single-width character. A monospace font is
 *         assumed.
 *
 * #### Examples
 * See [the header file's documentation](@ref ncurses_utils_examples).
 */
size_t ncurses_measure_string(const char *str);

/**
 * @brief   Calculates the length of the longest prefix of @p str that can be printed without going
 *          over a given width limit.
 * @details Emojis (or any other multi-codepoint character not formed by diacritics) aren't
 *          supported. Multi-line strings aren't supported as well.
 *
 *          This function is somewhat slow, as it requires lots of lookups in Unicode tables. Try to
 *          keep its use to a minimum.
 *
 * @param str   Null-terminated UTF-32 string to have its width limited.
 * @param max   Maximum width limit.
 * @param width Where the width of the longest printable prefix will be placed. Can be `NULL`.
 *
 * @return The length of the longest prefix of @p str that can be printed without going over the
 *         width limit.
 *
 * #### Examples
 * See [the header file's documentation](@ref ncurses_utils_examples).
 */
size_t ncurses_prefix_from_maximum_length(const gunichar *str, size_t max, size_t *width);

/**
 * @brief   Calculates the length of the longest suffix of @p str that can be printed without going
 *          over a given width limit.
 * @details Emojis (or any other multi-codepoint character not formed by diacritics) aren't
 *          supported. Multi-line strings aren't supported as well.
 *
 *          This function is somewhat slow, as it requires lots of lookups in Unicode tables. Try to
 *          keep its use to a minimum.
 *
 * @param str   Null-terminated UTF-32 string to have its width limited.
 * @param len   Length of @p str.
 * @param max   Maximum width limit.
 * @param width Where the width of the longest printable suffix will be placed. Can be `NULL`.
 *
 * @return The length of the longest suffix of @p str that can be printed without going over the
 *         width limit.
 *
 * #### Examples
 * See [the header file's documentation](@ref ncurses_utils_examples).
 */
size_t
    ncurses_suffix_from_maximum_length(const gunichar *str, size_t len, size_t max, size_t *width);
