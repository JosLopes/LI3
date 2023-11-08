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
 * @brief Contains the entry point to the program.
 */

#include <stdio.h>

#include "database/reservation_manager.h"
#include "dataset/dataset_loader.h"

/**
 * @brief Callback called for every reservation in the database, that prints it to the screen.
 *
 * @param reservation_data `NULL`.
 * @param reservation      reservation to be printed to `stdout`.
 *
 * @retval Always `0`, as this cannot fail.
 */
int iter_callback(void *reservation_data, reservation_t *reservation) {
    (void) reservation_data;

    const char *user_id         = reservation_get_const_user_id(reservation);
    const char *hotel_name      = reservation_get_const_hotel_name(reservation);
    size_t      id              = reservation_get_id(reservation);
    int         rating          = reservation_get_rating(reservation);
    int         hotel_id        = reservation_get_hotel_id(reservation);
    int         hotel_stars     = reservation_get_hotel_stars(reservation);
    int         city_tax        = reservation_get_city_tax(reservation);
    int         price_per_night = reservation_get_price_per_night(reservation);

    char includes_breakfast[INCLUDES_BREAKFAST_SPRINTF_MIN_BUFFER_SIZE];
    includes_breakfast_sprintf(includes_breakfast, reservation_get_includes_breakfast(reservation));

    char begin_date[DATE_SPRINTF_MIN_BUFFER_SIZE];
    date_sprintf(begin_date, reservation_get_begin_date(reservation));

    char end_date[DATE_SPRINTF_MIN_BUFFER_SIZE];
    date_sprintf(end_date, reservation_get_end_date(reservation));

    printf(
        "--- reservation ---\nuser_id: %s\nhotel_name: %s\nincludes_breakfast: %s\n"
        "begin_date: %s\nend_date: %s\nid: BOOK%ld\nrating: %d\nhotel_id: HTL%d\nhotel_stars: %d\n"
        "city_tax: %d\nprice_per_night: %d\n\n",
        user_id,
        hotel_name,
        includes_breakfast,
        begin_date,
        end_date,
        id,
        rating,
        hotel_id,
        hotel_stars,
        city_tax,
        price_per_night);
    return 0;
}

/**
 * @brief The entry point to the test program.
 * @details Tests for dataset parsing.

 * @retval 0 Success
 * @retval 1 Failure
 */
int main(void) {
    database_t *database = database_create();
    if (!database) {
        fprintf(stderr, "Failed to allocate database!");
        return 1;
    }

    if (dataset_loader_load(database, "/home/jose/Desktop/datasets/dataset/data")) {
        fputs("Failed to open dataset to be parsed.\n", stderr);
        return 1;
    }

    reservation_manager_iter(database_get_reservations(database), iter_callback, NULL);

    database_free(database);
    return 0;
}
