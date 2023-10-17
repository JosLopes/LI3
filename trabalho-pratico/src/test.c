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
#include <stdlib.h>

#include "utils/date.h"

/**
 * @brief The entry point to the test program.
 * @details Test for dates.
 * @retval 0 Success
 * @retval 1 Insuccess
 */
int main(void) {
    const char *birthdays[12] = {
        "2004/10/16", /* Valid date */

        "2002/7/12",   /* One digit missing */
        "13000/04/12", /* One extra digit */
        "-203/06/01",  /* Invalid digit */
        "-203/1-/01",  /* Invalid digit */

        "2022/04/31", /* Valid date */
        "2022/05/32", /* Out-of-range day */
        "1333/00/12", /* Out-of-range month */
        "1333/13/12", /* Out-of-range month */

        "2023/11/21/11", /* Too many data points */
        "2023/11",       /* Too few data points */

        "9990/01/01" /* Valid but will overflow */
    };

    for (int i = 0; i < 12; ++i) {
        date_t date;
        int    success = date_from_string_const(&date, birthdays[i]);

        if (success) {
            fprintf(stderr, "Failed to parse date \"%s\".\n", birthdays[i]);
        } else {
            /* This may fail for large years (max year is 9999) */
            int set_result = date_set_year(&date, date_get_year(date) + 10);

            char str[DATE_SPRINTF_MIN_BUFFER_SIZE];
            date_sprintf(str, date);

            if (set_result) {
                fprintf(stderr, "Date overflow for %s!\n", str);
            } else {
                printf("Your 10th birthday was / will be in %s\n", str);
            }
        }
    }

    return 0;
}
