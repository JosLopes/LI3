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

#ifndef USER_MANAGER_H
#define USER_MANAGER_H

#include "types/flight.h"

/**
 * @brief A data type that contains and manages all flights in a database.
 */
typedef struct flight_manager flight_manager_t;

/**
 * @brief   Callback type for user manager iterations.
 * @details Method called by ::user_manager_iter for every item in a ::user_manager_t.
 *
 * @param flight_data Argument passed to ::user_manager_iter that is passed to every callback, so
 *                    that this method can change the program's state.
 * @param flight      Flight in the manager.
 *
 * @return `0` on success, or any other value to order iteration to stop.
 */
typedef int (*flight_manager_iter_callback_t)(void *flight_data, flight_t *flight);

#endif
