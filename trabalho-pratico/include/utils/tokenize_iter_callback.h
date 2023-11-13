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
 * @file tokenize_iter_callback.h
 * @brief Defines a callback type for tokenizers.
 *
 */

#ifndef TOKENIZE_ITER_CALLBACK_H
#define TOKENIZE_ITER_CALLBACK_H

/**
 * @brief Callback method for many tokenizers, called for every token read.
 *
 * @param user_data Pointer, kept from call to call, so that this callback can modify the program's
 *                  state.
 * @param token     The token that was read.
 *
 * @return `0` on success, other value for immediate termination of tokenization. It's recommended
 *         that this value is positive, not to risk being confused with
 *         ::STRING_CONST_TOKENIZE_FAILED_MALLOC (or others).
 */
typedef int (*tokenize_iter_callback_t)(void *user_data, char *token);

#endif
