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
 * strings in the said array, and whether lines should be grouped in blocks (the definition of a
 * block can also be found on the documentation for ::activity_paging_run).
 *
 * The generated pages will look like the following on screen. In this example there are three lines
 * per block and three total blocks (one hidden on the next page).
 *
 * ```text
 * +---------------------Query output--------------------+
 * | 1º line of the 1º block                             |
 * | 2º line of the 1º block                             |
 * |                                                     |
 * | 1º line of the 2º block                             |
 * | 2º line of the 2º block                             |
 * |                                                     |
 * | Use the ← and → to navigate                   1 / 2 |
 * +-----------------------------------------------------+
 * ```
 */
#ifndef ACTIVITY_PAGING_H
#define ACTIVITY_PAGING_H

/**
 * @brief Runs a TUI activity for a paginator.
 *
 * @details When dividing @p lines into pages, you may not want page breaks to separate contiguous
 *          lines of text. Those are blocks, contiguous sets of lines with an empty line at the end.
 *          This paginator supports either no blocks, or fixed-sized blocks, whose size is
 *          automatically determined.
 *
 *
 * @param n        The number of @p lines.
 * @param lines    The lines of output be shown on the screen.
 * @param blocking If text blocks should be considered in page separation.
 *
 * @retval 0 Success.
 * @retval 1 Allocation failure.
 *
 * #### Examples
 * See [the header file's documentation](@ref activity_paging_examples).
 */
int activity_paging_run(size_t n, const char *const lines[n], int blocking);

#endif
