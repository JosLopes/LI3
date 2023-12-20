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
 * @brief Contains the entry point to main the program.
 */
#include <stdio.h>

#include "batch_mode.h"
#include "interactive_mode/interactive_mode.h"

/**
 * @brief  The entry point to the main program.
 * @retval 0 Success
 * @retval 1 Insuccess
 */
int main(int argc, char **argv) {
    if (argc == 1) {
        return interactive_mode_run();
    } else if (argc == 3) {
        return batch_mode_run(argv[1], argv[2], NULL);
    } else {
        fputs("Invalid command-line arguments! Usage:\n\n", stderr);
        fputs("./programa-principal - Interactive mode\n", stderr);
        fputs("./programa-principal [dataset] [query file] - Batch mode\n", stderr);
        return 1;
    }

    return 0;
}
