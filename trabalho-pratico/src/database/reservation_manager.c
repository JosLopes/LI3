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
 * ### Example
 * See [the header file's documentation](@ref reservation_manager_examples).
 */

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>

#include "database/reservation_manager.h"

/**
 * @struct reservation_manager
 * @brief  A data type that contains and manages all reservations in a database.
 *
 * @var reservation_manager::reservations
 *     @brief Allocator for reservations in the manager.
 * @var reservation_manager::hotel_name_pool
 *     @brief Allocator for hotel names in reservations.
 * @var reservation_manager::user_id_pool
 *     @brief Allocators for user identifiers in reservations.
 * @var reservation_manager::id_reservations_rel
 *     @brief Hash table for ::reservation_id_t -> ::reservation_t mapping.
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
#define RESERVATION_MANAGER_STRING_POOLS_BLOCK_CAPACITY 100000

reservation_manager_t *reservation_manager_create(void) {
    reservation_manager_t *const manager = malloc(sizeof(reservation_manager_t));
    if (!manager)
        goto DEFER_1;

    manager->reservations =
        pool_create_from_size(reservation_sizeof(),
                              RESERVATION_MANAGER_RESERVATIONS_POOL_BLOCK_CAPACITY);
    if (!manager->reservations)
        goto DEFER_2;

    manager->hotel_name_pool =
        string_pool_no_duplicates_create(RESERVATION_MANAGER_STRING_POOLS_BLOCK_CAPACITY);
    if (!manager->hotel_name_pool)
        goto DEFER_3;

    manager->user_id_pool = string_pool_create(RESERVATION_MANAGER_STRING_POOLS_BLOCK_CAPACITY);
    if (!manager->user_id_pool)
        goto DEFER_4;

    manager->id_reservations_rel = g_hash_table_new(g_direct_hash, g_direct_equal);
    return manager;

DEFER_4:
    string_pool_no_duplicates_free(manager->hotel_name_pool);
DEFER_3:
    pool_free(manager->reservations);
DEFER_2:
    free(manager);
DEFER_1:
    return NULL;
}

reservation_manager_t *reservation_manager_clone(const reservation_manager_t *manager) {
    reservation_manager_t *const clone = reservation_manager_create();
    if (!clone)
        return NULL;

    if (reservation_manager_iter(
            manager,
            (reservation_manager_iter_callback_t) reservation_manager_add_reservation,
            clone)) {

        reservation_manager_free(clone);
        return NULL;
    }

    return clone;
}

int reservation_manager_add_reservation(reservation_manager_t *manager,
                                        const reservation_t   *reservation) {

    reservation_t *const pool_reservation = reservation_clone(manager->reservations,
                                                              manager->user_id_pool,
                                                              manager->hotel_name_pool,
                                                              reservation);
    if (!pool_reservation)
        return 1;

    reservation_id_t res_id = reservation_get_id(reservation);
    if (!g_hash_table_insert(manager->id_reservations_rel,
                             GUINT_TO_POINTER(res_id),
                             pool_reservation)) {

        /* Do not fatally fail (just print a warning). Show must go on. */
        char id_str[RESERVATION_ID_SPRINTF_MIN_BUFFER_SIZE];
        reservation_id_sprintf(id_str, res_id);
        fprintf(stderr,
                "REPEATED RESERVATION ID \"%s\". This shouldn't happen! Replacing it.\n",
                id_str);
    }

    return 0;
}

const reservation_t *reservation_manager_get_by_id(const reservation_manager_t *manager,
                                                   reservation_id_t             id) {
    return g_hash_table_lookup(manager->id_reservations_rel, GUINT_TO_POINTER(id));
}

int reservation_manager_iter(const reservation_manager_t        *manager,
                             reservation_manager_iter_callback_t callback,
                             void                               *user_data) {
    return pool_iter(manager->reservations, (pool_iter_callback_t) callback, user_data);
}

void reservation_manager_free(reservation_manager_t *manager) {
    pool_free(manager->reservations);
    string_pool_no_duplicates_free(manager->hotel_name_pool);
    string_pool_free(manager->user_id_pool);
    g_hash_table_unref(manager->id_reservations_rel);
    free(manager);
}
