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
 * @file  q01.h
 * @brief A query to provide information about an entity in the dataset.
 *
 * ### Examples
 *
 * ```text
 * 1 JéssiTavares910
 * 1 DGarcia429
 * 1 LGarcia1208
 * 1 BSimões1222
 * 1 AdriaBranco1042
 * 1 CarAssunção255
 * 1 EdSousa
 * 1 0000000029
 * 1 0000000444
 * 1 0000000946
 * 1 0000000296
 * 1 0000000678
 * 1 Book0000000048
 * 1 Book0000033110
 * 1 Book0000000156
 * 1 Book0000020828
 * 1 Book0000001504
 * 1 Book0000024352
 * 1F JéssiTavares910
 * 1F DGarcia429
 * 1F LGarcia1208
 * 1F BSimões1222
 * 1F AdriaBranco1042
 * 1F CarAssunção255
 * 1F EdSousa
 * 1F 0000000029
 * 1F 0000000444
 * 1F 0000000946
 * 1F 0000000296
 * 1F 0000000678
 * 1F Book0000000048
 * 1F Book0000033110
 * 1F Book0000000156
 * 1F Book0000020828
 * 1F Book0000001504
 * 1F Book0000024352
 * ```
 */

#ifndef Q1_H
#define Q1_H

#include "queries/query_type.h"

/**
 * @brief   Initializes the definition of queries of type 1.
 * @details This is done automatically in [query_type_list](@ref query_type_list.c).
 * @return  On success, a pointer to a ::query_type_t that must be deleted with ::query_type_free,
 *          or `NULL` allocation on failure.
 */
query_type_t *q01_create(void);

#endif
