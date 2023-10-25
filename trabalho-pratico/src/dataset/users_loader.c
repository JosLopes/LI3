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
 * @file  users_loader.c
 * @brief Implementation of methods in include/dataset/users_loader.h
 */

#include <ctype.h>
#include <string.h>

#include "dataset/users_loader.h"
#include "types/email.h"
#include "utils/dataset_parser.h"
#include "utils/date.h"
#include "utils/date_and_time.h"

/** @brief Table header for user errors */
#define USER_LOADER_HEADER                                                                         \
    "id;name;email;phone_number;birth_date;sex;passport;country_code;address;account_creation;"    \
    "pay_method;account_status"

/**
 * @brief Temporary data needed to load a set of users.
 *
 * @var users_loader_t::dataset
 *     @brief Dataset loader, so that errors can be reported.
 * @var users_loader_t::database
 *     @brief Database in ::users_loader_t::dataset
 * @var users_loader_t::error_line
 *     @brief Current line being processed, in case it needs to be put in the error file.
 * @var users_loader_t::id_length
 *     @brief Length of the user's identifier, needed to reconstruct it after a line is parsed.
 * @var users_loader_t::name_length
 *     @brief Length of the user's name, needed to reconstruct it after a line is parsed.
 * @var users_loader_t::passport_length
 *     @brief Length of the user's passport number, needed to reconstruct it after a line is parsed.
 */
typedef struct {
    dataset_loader_t *dataset;
    database_t       *database;

    char *error_line;

    size_t id_length, name_length, passport_length;
} users_loader_t;

int __users_loader_before_parse_line(void *loader_data, char *line) {
    ((users_loader_t *) loader_data)->error_line = line;
    return 0;
}

/* TODO - write values to a user field in users_loader_t */

int __user_loader_parse_id(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;
    users_loader_t *loader = (users_loader_t *) loader_data;

    loader->id_length = strlen(token);
    return loader->id_length == 0; /* Fail on empty IDs */
}

int __user_loader_parse_name(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;
    users_loader_t *loader = (users_loader_t *) loader_data;

    loader->name_length = strlen(token);
    return loader->name_length == 0; /* Fail on empty names */
}

int __user_loader_parse_email(void *loader_data, char *token, size_t ntoken) {
    (void) loader_data;
    (void) ntoken;

    return email_validate_string(token);
}

int __user_loader_parse_phone_number(void *loader_data, char *token, size_t ntoken) {
    (void) loader_data;
    (void) ntoken;

    return (*token == '\0'); /* Fail on empty phone numbers */
}

int __user_loader_parse_birth_date(void *loader_data, char *token, size_t ntoken) {
    (void) loader_data;
    (void) ntoken;

    date_t date; /* Later replace with date in the user */
    return date_from_string(&date, token);
}

int __user_loader_parse_sex(void *loader_data, char *token, size_t ntoken) {
    (void) loader_data;
    (void) ntoken;

    /* TODO - use sex when user type is ready */
    return (*token == 0); /* Fail on empty sexes */
}

int __user_loader_parse_passport(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;
    users_loader_t *loader = (users_loader_t *) loader_data;

    loader->passport_length = strlen(token);
    return loader->passport_length == 0; /* Fail on empty names */
}

int __user_loader_parse_country_code(void *loader_data, char *token, size_t ntoken) {
    (void) loader_data;
    (void) ntoken;

    /* TODO - make country code in user a typedef short with validation */
    if (*token && *(token + 1) && !*(token + 2) && isalpha(*token) && isalpha(*(token + 1)))
        return 0;
    return 1;
}

int __user_loader_parse_address(void *loader_data, char *token, size_t ntoken) {
    (void) loader_data;
    (void) ntoken;
    return (*token == 0); /* Fail on empty addresses */
}

int __user_loader_parse_account_creation_date(void *loader_data, char *token, size_t ntoken) {
    (void) loader_data;
    (void) ntoken;

    date_and_time_t date; /* Later replace with date in the user */
    return date_and_time_from_string(&date, token);
}

int __user_loader_parse_pay_method(void *loader_data, char *token, size_t ntoken) {
    (void) loader_data;
    (void) ntoken;
    return (*token == 0); /* Fail on empty payment methods */
}

int __user_loader_parse_account_status(void *loader_data, char *token, size_t ntoken) {
    /* TODO - use user's account status method when its merged */
#define ACCOUNT_STATUS_STRLEN_INACTIVE 8

    (void) loader_data;
    (void) ntoken;

    size_t len = strlen(token);
    if (len > ACCOUNT_STATUS_STRLEN_INACTIVE)
        return 1; /* Too long for "inactive" */

    char lower_case[ACCOUNT_STATUS_STRLEN_INACTIVE + 1];
    for (size_t i = 0; i < len; ++i)
        lower_case[i] = tolower(token[i]);
    lower_case[len] = '\0';

    if (strcmp(lower_case, "inactive") == 0) {
        return 0;
    } else if (strcmp(lower_case, "active") == 0) {
        return 0;
    } else {
        return 1;
    }
}

int __users_loader_after_parse_line(void *loader_data, int retval) {
    if (retval) {
        users_loader_t *loader = (users_loader_t *) loader_data;
        dataset_loader_report_users_error(loader->dataset, loader->error_line);
    }
    return 0;
}

void users_loader_load(dataset_loader_t *dataset_loader, FILE *stream) {
    dataset_loader_report_users_error(dataset_loader, USER_LOADER_HEADER);
    users_loader_t data = {.dataset  = dataset_loader,
                           .database = dataset_loader_get_database(dataset_loader)};

    fixed_n_delimiter_parser_iter_callback_t token_callbacks[12] = {
        __user_loader_parse_id,
        __user_loader_parse_name,
        __user_loader_parse_email,
        __user_loader_parse_phone_number,
        __user_loader_parse_birth_date,
        __user_loader_parse_sex,
        __user_loader_parse_passport,
        __user_loader_parse_country_code,
        __user_loader_parse_address,
        __user_loader_parse_account_creation_date,
        __user_loader_parse_pay_method,
        __user_loader_parse_account_status,
    };

    fixed_n_delimiter_parser_grammar_t *line_grammar =
        fixed_n_delimiter_parser_grammar_new(';', 12, token_callbacks);

    dataset_parser_grammar_t *grammar = dataset_parser_grammar_new('\n',
                                                                   line_grammar,
                                                                   __users_loader_before_parse_line,
                                                                   __users_loader_after_parse_line);
    dataset_parser_parse(stream, grammar, &data);
}
