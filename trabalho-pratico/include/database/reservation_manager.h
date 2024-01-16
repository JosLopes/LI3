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
 * @file    reservation_manager.h
 * @brief   Contains and manages all reservations in a database.
 * @details Usually, a reservation manager won't be created by itself, but instead by a
 *          ::database_t.
 *
 * @anchor reservation_manager_examples
 * ### Examples
 *
 * In the following example, a dataset is loaded into a database. The reservation manager is then
 * extracted from the database, and the program iterates over all reservations.
 *
 * ```c
 * #include <stdio.h>
 *
 * #include "database/reservation_manager.h"
 * #include "dataset/dataset_loader.h"
 *
 * // Callback called for every reservation in the database, that prints it to the screen.
 * int iter_callback(void *user_data, const reservation_t *reservation) {
 *     (void) user_data;
 *
 *     const char      *user_id         = reservation_get_const_user_id(reservation);
 *     const char      *hotel_name      = reservation_get_const_hotel_name(reservation);
 *     reservation_id_t id              = reservation_get_id(reservation);
 *     uint8_t          rating          = reservation_get_rating(reservation);
 *     hotel_id_t       hotel_id        = reservation_get_hotel_id(reservation);
 *     uint8_t          hotel_stars     = reservation_get_hotel_stars(reservation);
 *     uint8_t          city_tax        = reservation_get_city_tax(reservation);
 *     uint16_t         price_per_night = reservation_get_price_per_night(reservation);
 *
 *     char includes_breakfast[INCLUDES_BREAKFAST_SPRINTF_MIN_BUFFER_SIZE];
 *     includes_breakfast_sprintf(includes_breakfast,
 *         reservation_get_includes_breakfast(reservation));
 *
 *     char begin_date[DATE_SPRINTF_MIN_BUFFER_SIZE];
 *     date_sprintf(begin_date, reservation_get_begin_date(reservation));
 *
 *     char end_date[DATE_SPRINTF_MIN_BUFFER_SIZE];
 *     date_sprintf(end_date, reservation_get_end_date(reservation));
 *
 *     char hotel_id_str[HOTEL_ID_SPRINTF_MIN_BUFFER_SIZE];
 *     hotel_id_sprintf(hotel_id_str, hotel_id);
 *
 *     char reservation_id_str[RESERVATION_ID_SPRINTF_MIN_BUFFER_SIZE];
 *     reservation_id_sprintf(reservation_id_str, id);
 *
 *     printf("--- RESERVATION ---\nuser_id: %s\nhotel_name: %s\nincludes_breakfast: %s\n"
 *            "begin_date: %s\nend_date: %s\nid: %s\nrating: " PRIu8 "\nhotel_id: %s\n"
 *            "hotel_stars: " PRIu8 "\n city_tax: " PRIu8 "\nprice_per_night: " PRIu16 "\n\n",
 *            user_id,
 *            hotel_name,
 *            includes_breakfast,
 *            begin_date,
 *            end_date,
 *            reservation_id_str,
 *            rating,
 *            hotel_id_str,
 *            hotel_stars,
 *            city_tax,
 *            price_per_night);
 *
 *     return 0; // You can return a value other than 0 to order iteration to stop
 * }
 *
 * int main(void) {
 *     database_t *database = database_create();
 *     if (!database) {
 *         fprintf(stderr, "Failed to allocate database!\n");
 *         return 1;
 *     }
 *
 *     if (dataset_loader_load(database, "/path/to/dataset/directory")) {
 *         fputs("Failed to open dataset to be parsed.\n", stderr);
 *         return 1;
 *     }
 *
 *     reservation_manager_iter(database_get_reservations(database), iter_callback, NULL);
 *
 *     database_free(database);
 *     return 0;
 * }
 * ```
 *
 * Another operation (other than iteration) that can be performed on a ::reservation_manager_t is a
 * lookup by reservation identifier (::reservation_manager_get_by_id).
 *
 * If you'd rather not use a database, you could create the reservation manager yourself with
 * ::reservation_manager_create, add reservations to it using ::reservation_manager_add_reservation,
 * and free it in the end with ::reservation_manager_free. Just keep in mind that added reservations
 * and their associated strings will be copied to memory pools.
 */

#ifndef RESERVATION_MANAGER_H
#define RESERVATION_MANAGER_H

#include "types/reservation.h"

/** @brief A data type that contains and manages all reservations in a database. */
typedef struct reservation_manager reservation_manager_t;

/**
 * @brief   Callback type for reservation manager iterations.
 * @details Method called by ::reservation_manager_iter for every item in a ::reservation_manager_t.
 *
 * @param user_data   Argument passed to ::reservation_manager_iter, that is then passed to every
 *                    callback, so that this method can change the program's state.
 * @param reservation Reservation in the manager.
 *
 * @return `0` on success, or any other value to order iteration to stop.
 */
typedef int (*reservation_manager_iter_callback_t)(void                *user_data,
                                                   const reservation_t *reservation);

/**
 * @brief   Instantiates a new ::reservation_manager_t.
 * @details The returned value is owned by the called and should be `free`'d with
 *          ::reservation_manager_free.
 * @return  The new reservation manager, or `NULL` on failure.
 */
reservation_manager_t *reservation_manager_create(void);

/**
 * @brief   Creates a deep copy of a reservation manager.
 * @details Managers usually contain lots of data, possibly even gigabytes! Keep that in mind for
 *          performance and memory usage reasons.
 *
 * @param manager Manager to be copied.
 *
 * @return A copy of @p manager, that must be `free`d with ::reservation_manager_free. `NULL` can
 *         also be returned, meaning an allocation failure happened.
 */
reservation_manager_t *reservation_manager_clone(const reservation_manager_t *manager);

/**
 * @brief Adds a reservation to a reservation manager.
 *
 * @param manager     Reservation manager to add @p reservation to.
 * @param reservation Reservation to be added to @p manager.
 *
 * @retval 0 Success.
 * @retval 1 Allocation failure.
 */
int reservation_manager_add_reservation(reservation_manager_t *manager,
                                        const reservation_t   *reservation);

/**
 * @brief Gets a reservation stored in a reservation manager by its identifier.
 *
 * @param manager Reservation manager where to perform the lookup.
 * @param id      Identifier of the reservation to find.
 *
 * @return A pointer to a ::reservation_t if it's found, `NULL` if it's not.
 */
const reservation_t *reservation_manager_get_by_id(const reservation_manager_t *manager,
                                                   reservation_id_t             id);

/**
 * @brief Iterates through every reservation in a reservation manager, calling a callback for each
 *        one.
 *
 * @param manager   Reservation manager to iterate through.
 * @param callback  Method to be called for every reservation stored in @p manager.
 * @param user_data Pointer to be passed to every @p callback, so that it can modify the program's
 *                  state.
 *
 * @return The return value of the last-called @p callback (`0` means success, another value means
 *         the iteration was stopped by a callback).
 *
 * #### Example
 * See [the header file's documentation](@ref reservation_manager_examples).
 */
int reservation_manager_iter(const reservation_manager_t        *manager,
                             reservation_manager_iter_callback_t callback,
                             void                               *user_data);

/**
 * @brief Frees memory used by a reservation manager.
 * @param manager Reservation manager whose memory is to be `free`d.
 */
void reservation_manager_free(reservation_manager_t *manager);

#endif
