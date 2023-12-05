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
 * @file  reservation_manager.c
 * @brief Implementation of methods in include/database/reservation_manager.h
 *
 * #### Example
 * See [the header file's documentation](@ref reservation_manager_examples).
 */

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>

#include "database/reservation_manager.h"
#include "types/reservation.h"
#include "utils/pool.h"
#include "utils/string_pool_no_duplicates.h"

/**
 * @struct reservation_manager
 * @brief  A data type that contains and manages all reservations in a database.
 *
 * @var reservation_manager::reservations
 *     @brief Set of reservations in the manager.
 * @var reservation_manager::hotel_name_pool
 *     @brief Pool for any string that may need to be stored in a reservation, no duplicates are
 *            stored.
 * @var reservation_manager::user_id_pool
 *     @brief Pool for user IDs, as they never repeat themselves.
 * @var reservation_manager::id_reservations_rel
 *     @brief Hash table for identifier -> reservations mapping.
 */
struct reservation_manager {
    pool_t                      *reservations;
    string_pool_no_duplicates_t *hotel_name_pool;
    string_pool_t               *user_id_pool;
    GHashTable                  *id_reservations_rel;
};

/** @brief Number of reservations in each block of ::reservation_manager::reservations. */
#define RESERVATION_MANAGER_RESERVATIONS_POOL_BLOCK_CAPACITY 50000

/**
 * @brief Number of characters in each block of ::reservation_manager::hotel_name_pool and in
 *        ::reservation_manager::user_id_pool.
 */
#define RESERVATION_MANAGER_STRINGS_POOL_BLOCK_CAPACITY 100000

reservation_manager_t *reservation_manager_create(void) {
    reservation_manager_t *manager = malloc(sizeof(struct reservation_manager));
    if (!manager)
        return NULL;

    manager->reservations =
        pool_create_from_size(reservation_sizeof(),
                              RESERVATION_MANAGER_RESERVATIONS_POOL_BLOCK_CAPACITY);
    if (!manager->reservations) {
        free(manager);
        return NULL;
    }

    manager->hotel_name_pool =
        string_pool_no_duplicates_create(RESERVATION_MANAGER_STRINGS_POOL_BLOCK_CAPACITY);
    if (!manager->hotel_name_pool) {
        pool_free(manager->reservations);
        free(manager);
        return NULL;
    }

    manager->user_id_pool = string_pool_create(RESERVATION_MANAGER_STRINGS_POOL_BLOCK_CAPACITY);
    if (!manager->user_id_pool) {
        string_pool_no_duplicates_free(manager->hotel_name_pool);
        pool_free(manager->reservations);
        free(manager);
        return NULL;
    }

    manager->id_reservations_rel = g_hash_table_new(g_direct_hash, g_direct_equal);

    return manager;
}

reservation_t *reservation_manager_add_reservation(reservation_manager_t *manager,
                                                   const reservation_t   *reservation) {
    reservation_t *pool_reservation =
        pool_put_item(reservation_t, manager->reservations, reservation);
    if (!pool_reservation)
        return NULL;

    /* Copy strings to string pool */
    const char *pool_user_id =
        string_pool_put(manager->user_id_pool, reservation_get_const_user_id(reservation));
    const char *pool_hotel_name =
        string_pool_no_duplicates_put(manager->hotel_name_pool,
                                      reservation_get_const_hotel_name(reservation));

    if (pool_user_id && pool_hotel_name) {
        reservation_set_user_id(pool_reservation, pool_user_id);
        reservation_set_hotel_name(pool_reservation, pool_hotel_name);

        /* Reservation identifier */
        size_t res_id = reservation_get_id((reservation_t *) reservation);

        if (!g_hash_table_insert(manager->id_reservations_rel,
                                 GINT_TO_POINTER(res_id),
                                 pool_reservation)) {
            fprintf(stderr,
                    "REPEATED RESERVATION ID \"%ld\". This shouldn't happen! Replacing it.\n",
                    res_id);
            /* Do not fail and return NULL. Show must go on */
        }

        return pool_reservation;
    } else {
        /* On allocation failure, it's impossible to remove anything already in a pool */
        reservation_invalidate(pool_reservation);
        return NULL;
    }
}

reservation_t *reservation_manager_get_by_id(const reservation_manager_t *manager, size_t id) {
    return g_hash_table_lookup(manager->id_reservations_rel, GINT_TO_POINTER(id));
}

/**
 * @struct reservation_manager_iter_reservation_data_t
 * @brief Internal data type for the `reservation_data` parameter in
 *        ::__reservation_manager_iter_callback.
 *
 * @var reservation_manager_iter_reservation_data_t::callback
 *     @brief Callback to be called for every valid reservation.
 * @var reservation_manager_iter_reservation_data_t::original_user_data.
 *     @brief `user_data` parameter for every
 *            ::reservation_manager_iter_reservation_data_t::callback.
 */
typedef struct {
    reservation_manager_iter_callback_t callback;
    void                               *original_user_data;
} reservation_manager_iter_reservation_data_t;

/**
 * @brief   Callback for every item in the reservation manager's pool.
 * @details Auxiliary function for ::reservation_manager_iter. Makes sure the target callback is
 *          only called for valid reservations.
 *
 * @param user_data A ::reservation_manager_iter_reservation_data_t.
 * @param item      A ::reservation_t in a reservation manager's pool.
 *
 * @return The return value of the target callback, or `0` for filtered-out items.
 */
int __reservation_manager_iter_callback(void *user_data, const void *item) {
    if (reservation_is_valid((const reservation_t *) item) == 0) {
        reservation_manager_iter_reservation_data_t *helper_data =
            (reservation_manager_iter_reservation_data_t *) user_data;
        return helper_data->callback(helper_data->original_user_data, (const reservation_t *) item);
    }
    return 0;
}

int reservation_manager_iter(reservation_manager_t              *manager,
                             reservation_manager_iter_callback_t callback,
                             void                               *user_data) {

    reservation_manager_iter_reservation_data_t helper_data = {.callback           = callback,
                                                               .original_user_data = user_data};
    return pool_iter(manager->reservations, __reservation_manager_iter_callback, &helper_data);
}

void reservation_manager_free(reservation_manager_t *manager) {
    pool_free(manager->reservations);
    string_pool_no_duplicates_free(manager->hotel_name_pool);
    string_pool_free(manager->user_id_pool);
    g_hash_table_destroy(manager->id_reservations_rel);
    free(manager);
}
