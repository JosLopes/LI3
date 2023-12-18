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
 * @file  hotel_id.c
 * @brief Implementation of methods in include/types/hotel_id.h
 */

#include <stdio.h>
#include <string.h>

#include "types/hotel_id.h"
#include "utils/int_utils.h"

int hotel_id_from_string(hotel_id_t *output, const char *input) {
    size_t length = strnlen(input, 4);
    if (length > 3) { /* Skip "HTL" before any reservation ID */
        uint64_t id;
        int      retval = int_utils_parse_positive(&id, input + 3);

        if (retval)
            return 2;

        *output = (hotel_id_t) id;
        return 0;
    }

    return (length == 0) ? 1 : 2;

}

void hotel_id_sprintf(char *output, hotel_id_t id) {
    sprintf(output, "HTL%" PRIu32, id);
}
