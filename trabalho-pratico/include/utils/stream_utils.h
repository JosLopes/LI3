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
 * @file stream_utils.h
 * @brief 
 * 
 */

#ifndef STREAM_UTILS_H
#define STREAM_UTILS_H

#include "utils/string_utils.h"

int stream_tokenize(FILE                    *file,
                    char                     delimiter,
                    tokenize_iter_callback_t callback,
                    void                    *user_data);

#endif

// Perguntar berto se é necessário stream_tokenize_const

// TODO: Documentation