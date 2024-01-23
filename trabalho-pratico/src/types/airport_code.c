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
 * @file  airport_code.c
 * @brief Implementation of methods in include/types/airport_code.h
 *
 * ### Examples
 * See [the header file's documentation](@ref airport_code_examples).
 */

#include "types/airport_code.h"

/**
 * @brief   Macro implementation of `isalpha` (for inlining).
 * @details See musl's implementation of `isalpha` for source.
 */
#define inline_isalpha(c) ((((unsigned) (c) | 32) - 'a') < 26)

int airport_code_from_string(airport_code_t *output, const char *input) {
    if (input[0] && input[1] && input[2] && !input[3]) { /* inline strlen(input) == 3 */
        if (inline_isalpha(input[0]) && inline_isalpha(input[1]) && inline_isalpha(input[2])) {

            /*
             * "Vectorized" toupper, when we're certain all characters are letters.
             * In musl's implementation of toupper, c & 0x5f is done.
             *
             * Why do this? Because I have a test about vectorization tomorrow and want to study.
             */
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
            *output = (*(const int32_t *) input) & 0x5f5f5f00;
#else
            *output = (*(const int32_t *) input) & 0x005f5f5f;
#endif
            return 0;
        }
    }
    return 1;
}

void airport_code_sprintf(char *output, airport_code_t airport) {
    *((airport_code_t *) output) = airport;
}
