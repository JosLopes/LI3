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
 * @file test.c
 * @brief Contains the entry point to the test program.
 */

#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "utils/path_utils.h"

/**
 * @brief The entry point to the test program.
 * @details Tests for path utilities.
 * @retval 0 Success
 * @retval 1 Failure
 */
int main(void) {
    const char *test_paths[18] = {/* .. and . in absolute paths */
                                  "/abc/def/..",
                                  "/abc/def/.",

                                  /* .. in absolute and relative paths */
                                  "/abc/def/../../..",
                                  "abc/def/../../..",
                                  "./abc/def/../../..",
                                  "././abc/def/../../..",
                                  "./../abc/def/../../..",

                                  /* Relative paths with parent directory */
                                  "../abc",
                                  "../../abc",
                                  "../..//abc",
                                  "/../../abc",

                                  /* Multiple . and .. */
                                  "/.",
                                  "/././../.",
                                  "././../.",
                                  "/..",

                                  /* Others */
                                  "/",
                                  "////",
                                  ""};

    for (size_t i = 0; i < 18; ++i) {
        char path[PATH_MAX];
        strcpy(path, test_paths[i]);

        path_normalize(path);
        printf("%25s -> %s\n", test_paths[i], path);
    }
    return 0;
}
