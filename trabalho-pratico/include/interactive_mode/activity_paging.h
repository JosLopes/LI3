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
 * @file  activity_paging.h
 * @brief A paginator to display the output of queries for the interactive mode.
 *
 * @anchor activity_paging_examples
 * ### Examples
 *
 * Paging output is as simple as calling ::activity_paging_run. Provide an array of strings (lines,
 * with the restrictions specified in the documentation for ::activity_paging_run), the number of
 * strings in the said array, and the number of lines per block (the definition of block can also be
 * found on the documentation for ::activity_paging_run).
 *
 * The pages generated will look like the following on screen (in this particular case there are
 * three lines per block and three total blocks, one hidden on the next page):
 *
 * ```text
 * +---------------------Query output--------------------+
 * | 1º line of the 1º block                             |
 * | 2º line of the 1º block                             |
 * |                                                     |
 * | 1º line of the 2º block                             |
 * | 2º line of the 2º block                             |
 * |                                                     |
 * | Use the left and right arrows to navigate     1 / 2 |
 * +-----------------------------------------------------+
 * ```
 */
#ifndef ACTIVITY_PAGING_H
#define ACTIVITY_PAGING_H

#include "interactive_mode/activity.h"

/**
 * @brief Runs a TUI activity for a paginator.
 *
 * @details The @p lines array needs to be composed of chunks of lines (strings) with a fixed size,
 *          called blocks. Each block must contain an empty string at the end, that should be
 *          considered for the @p blocks_length, which means, in practice, that the lower possible
 *          size for a block should be two. This restrictions are implied to simplify the process of
 *          displaying multiple chunks of lines through possible multiple pages.
 *
 * @param lines        The lines of output be shown on the screen.
 * @param lines_length The length of @p lines.
 * @param block_length The number of lines in a block.
 *
 * @retval 0 Success.
 * @retval 1 Allocation Failure.
 *
 * #### Examples
 * See [the header file's documentation](@ref activity_paging_examples).
 */
int activity_paging_run(const char **lines, size_t lines_length, size_t block_length);

#endif
