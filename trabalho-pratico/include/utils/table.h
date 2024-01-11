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
 * @file  table.h
 * @brief A table of items.
 */

#ifndef TABLE_H
#define TABLE_H

#include <wchar.h>

typedef struct table table_t;

#define TABLE_STRING_POOL_BLOCK_SIZE 8

table_t *table_create(size_t height, size_t width);

void table_insert(table_t *table, const wchar_t *string, size_t x, size_t y);

void table_insert_double(table_t *table, double number, size_t x, size_t y);

void table_draw(FILE *output, table_t *table);

void table_free(table_t *table);

#endif
