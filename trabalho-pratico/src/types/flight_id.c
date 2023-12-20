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
 * @file  flight_id.c
 * @brief Implementation of methods in include/types/flight_id.h
 */

#include <stdio.h>

#include "types/flight_id.h"
#include "utils/int_utils.h"

int flight_id_from_string(flight_id_t *output, const char *input) {
    uint64_t id;
    int      retval = int_utils_parse_positive(&id, input);
    if (retval)
        return *input ? 2 : 1;

    *output = (flight_id_t) id;
    return 0;
}

void flight_id_sprintf(char *output, flight_id_t id) {
    sprintf(output, "%010" PRIu32, id);
}
