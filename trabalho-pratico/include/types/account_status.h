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
 * @file     account_status.h
 * @brief    Account status of a ::user_t.
 */

#ifndef ACCOUNT_STATUS_H
#define ACCOUNT_STATUS_H

/** @brief The account status of a ::user_t. */
typedef enum {
    ACCOUNT_STATUS_INACTIVE, /**< @brief Inactive account */
    ACCOUNT_STATUS_ACTIVE    /**< @brief Active account */
} account_status_t;

/**
 * @brief Parses a user's account status.
 *
 * @param output Where the parsed status will be placed (only on success).
 * @param input  Input (`"active"` or `"inactive"`, case-insensitive).
 *
 * @retval 0 Parsing success
 * @retval 1 Parsing failure
 */
int account_status_from_string(account_status_t *output, const char *input);

#endif
