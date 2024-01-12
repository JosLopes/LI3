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
 * @file  path_utils.h
 * @brief Contains utility methods for dealing with paths.
 *
 * @anchor path_utils_examples
 * ### Examples
 *
 * Here's an example on how to use ::path_normalize, and what its expeceted output is.
 *
 * ```c
 * const char *test_paths[18] = {// .. and . in absolute paths
 *                               "/abc/def/..",
 *                               "/abc/def/.",

 *                               // .. in absolute and relative paths
 *                               "/abc/def/../../..",
 *                               "abc/def/../../..",
 *                               "./abc/def/../../..",
 *                               "././abc/def/../../..",
 *                               "./../abc/def/../../..",

 *                               // Relative paths with parent directory
 *                               "../abc",
 *                               "../../abc",
 *                               "../..//abc",
 *                               "/../../abc",

 *                               // Multiple . and ..
 *                               "/.",
 *                               "/././../.",
 *                               "././../.",
 *                               "/..",

 *                               // Others
 *                               "/",
 *                               "////",
 *                               ""};

 * for (size_t i = 0; i < 18; ++i) {
 *     char path[PATH_MAX];
 *     strcpy(path, test_paths[i]);

 *     path_normalize(path);
 *     printf("%25s -> %s\n", test_paths[i], path);
 * }
 * ```
 *
 * Here's the expected output:
 *
 * ```text
 *           /abc/def/.. -> /abc
 *            /abc/def/. -> /abc/def
 *     /abc/def/../../.. -> /
 *      abc/def/../../.. -> ..
 *    ./abc/def/../../.. -> ..
 *  ././abc/def/../../.. -> ..
 * ./../abc/def/../../.. -> ../..
 *                ../abc -> ../abc
 *             ../../abc -> ../../abc
 *            ../..//abc -> ../../abc
 *            /../../abc -> /abc
 *                    /. -> /
 *             /././../. -> /
 *              ././../. -> ..
 *                   /.. -> /
 *                     / -> /
 *                  //// -> /
 *                       -> .
 * ```
 *
 * Path concatenation can be achieved with ::path_concat. It is a simple path concatenation (`path`
 * and `add` will become `path/add`), followed by ::path_normalize normalization, and then
 * outputted to `path`. For example, trying to concatenate `/home/user` and `..` will result in
 * `/home`.
 */

#ifndef PATH_UTILS_H
#define PATH_UTILS_H

/**
 * @brief   Normalizes a @p path.
 * @details Removes consecutive path separators (slashes) and limits the use of structures such as
 *          `.` and `..` to when they're strictly necessary.
 *
 *          This is a bit of an expensive operation, so avoid using it for bulk paths unless you
 *          need to.
 *
 * @param path Pointer to path that will be normalized, and written over. It is assumed that it can
 *             fit `PATH_MAX` bytes.
 *
 * #### Examples
 * See [the header file's documentation](@ref path_utils_examples).
 */
void path_normalize(char *path);

/**
 * @brief   Concatenates two paths (`path/add`).
 * @details Path concatenation is followed by normalization.
 *
 * @param path Pointer to path that will be added to, and written over. It is assumed that it can
 *             fit `PATH_MAX` bytes.
 * @param add  Path to be added to the end of @p path.
 *
 * #### Examples
 * See [the header file's documentation](@ref path_utils_examples).
 */
void path_concat(char *path, const char *add);

#endif
