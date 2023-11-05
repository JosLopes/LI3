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
 * @file main.c
 * @brief Contains the entry point to the program.
 */
#include <stdio.h>

#include "dataset/dataset_loader.h"

/**
 * @brief The entry point to the main program.
 * @retval 0 Success
 * @retval 1 Insuccess
 */
int main(int argc, char **argv) {
    if (argc == 1) {
        /* Interactive mode */
        fputs("Interactive mode not yet implemented!\n", stderr);
        fputs("We're very lucky if we manage to finish batch mode in time.\n", stderr);
        return 1;
    } else if (argc == 3) {
        /* Batch mode */
        char *dataset_dir = argv[1], *query_file = argv[2];
        (void) query_file;

        database_t *database = database_create();
        if (!database) {
            fprintf(stderr, "Failed to allocate database!");
            return 1;
        }

        if (dataset_loader_load(database, dataset_dir)) {
            fputs("Failed to load dataset files!\n", stderr);
            return 1;
        }

        /* Parse and execute queries here */

        database_free(database);
    } else {
        fputs("Invalid command-line arguments! Usage:\n\n", stderr);
        fputs("./programa-principal - Interactive mode\n", stderr);
        fputs("./programa-principal [dataset] [query file] - Batch mode\n", stderr);
        return 1;
    }

    return 0;
}
