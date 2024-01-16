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
 *
 * @details Many internal methods in this module are lacking parameter documentation, as they all
 *          follow the same convention: all are ::fixed_n_delimiter_parser_iter_callback_t. The
 *          `loader_data` is a pointer to a ::users_loader_t.
 */

#include "dataset/users_loader.h"

#include "dataset/dataset_parser.h"
#include "types/email.h"

/** @cond FALSE */
#ifndef __GNUC__
    #define __builtin_unreachable() return 0;
#endif
/** @endcond */

/**
 * @struct users_loader_t
 * @brief Temporary data needed to load a set of users.
 *
 * @var users_loader_t::output
 *     @brief Where to output dataset errors to.
 * @var users_loader_t::database
 *     @brief Database to add new users to.
 * @var users_loader_t::error_line
 *     @brief Current line being processed, in case it needs to be put in the error file.
 * @var users_loader_t::current_user
 *     @brief User being currently parsed, whose fields are still being filled in.
 */
typedef struct {
    dataset_error_output_t *const output;
    database_t *const             database;

    const char   *error_line;
    user_t *const current_user;
} users_loader_t;

/**
 * @brief Stores the beginning of the current line, in case it needs to be printed to the errors
 *        file.
 *
 * @param loader_data A pointer to a ::users_loader_t.
 * @param line        Line that is going to be parsed.
 *
 * @retval 0 Always successful.
 */
int __users_loader_before_parse_line(void *loader_data, char *line) {
    ((users_loader_t *) loader_data)->error_line = line;
    return 0;
}

/** @brief Parses a user's identifier. */
int __user_loader_parse_id(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;
    return user_set_id(NULL, ((users_loader_t *) loader_data)->current_user, token);
}

/** @brief Parses a user's name. */
int __user_loader_parse_name(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;
    return user_set_name(NULL, ((users_loader_t *) loader_data)->current_user, token);
}

/** @brief Parses a user's email address. */
int __user_loader_parse_email(void *loader_data, char *token, size_t ntoken) {
    (void) loader_data;
    (void) ntoken;
    return email_validate_string(token);
}

/**
 * @brief   Parses the strings in a user that cannot be empty, but that are not stored.
 * @details These are the phone number, the address and the payment method.
 */
int __user_loader_parse_non_empty(void *loader_data, char *token, size_t ntoken) {
    (void) loader_data;
    (void) ntoken;
    return (*token == '\0'); /* Fail on empty phone numbers */
}

/** @brief Parses a user's birth date. */
int __user_loader_parse_birth_date(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;

    date_t    date;
    const int retval = date_from_string(&date, token);
    if (retval)
        return retval;

    return user_set_birth_date(((users_loader_t *) loader_data)->current_user, date);
}

/** @brief Parses a user's sex. */
int __user_loader_parse_sex(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;

    sex_t     sex;
    const int retval = sex_from_string(&sex, token);
    if (retval)
        return retval;

    user_set_sex(((users_loader_t *) loader_data)->current_user, sex);
    return 0;
}

/** @brief Parses a user's passport number. */
int __user_loader_parse_passport(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;
    return user_set_passport(NULL, ((users_loader_t *) loader_data)->current_user, token);
}

/** @brief Parses a user's country code. */
int __user_loader_parse_country_code(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;

    country_code_t country;
    const int      retval = country_code_from_string(&country, token);
    if (retval)
        return retval;

    user_set_country_code(((users_loader_t *) loader_data)->current_user, country);
    return 0;
}

/** @brief Parses a user's account creation date. */
int __user_loader_parse_account_creation_date(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;

    date_and_time_t date;
    const int       retval = date_and_time_from_string(&date, token);
    if (retval)
        return retval;

    return user_set_account_creation_date(((users_loader_t *) loader_data)->current_user, date);
}

/** @brief Parses a user's account status. */
int __user_loader_parse_account_status(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;

    account_status_t status;
    const int        retval = account_status_from_string(&status, token);
    if (retval)
        return retval;

    user_set_account_status(((users_loader_t *) loader_data)->current_user, status);
    return 0;
}

/**
 * @brief Places a parsed user in the database and handles errors.
 *
 * @param loader_data A pointer to a ::users_loader_t.
 * @param retval      Value returned by the last token callback (`0` for success, another value for
 *                    a parsing error).
 *
 * @retval 0 Success.
 * @retval 1 Allocation failure.
 */
int __users_loader_after_parse_line(void *loader_data, int retval) {
    users_loader_t *const loader = loader_data;

    if (retval) {
        dataset_error_output_report_user_error(loader->output, loader->error_line);
        retval = 0;
    } else {
        retval = database_add_user(loader->database, loader->current_user);
    }

    user_reset_dates(loader->current_user);
    return retval;
}

int users_loader_load(FILE *stream, database_t *database, dataset_error_output_t *output) {
    users_loader_t data = {.output       = output,
                           .database     = database,
                           .current_user = user_create(NULL)};

    if (!data.current_user)
        return 1; /* Allocation failure */

    const fixed_n_delimiter_parser_iter_callback_t token_callbacks[12] = {
        __user_loader_parse_id,
        __user_loader_parse_name,
        __user_loader_parse_email,
        __user_loader_parse_non_empty,
        __user_loader_parse_birth_date,
        __user_loader_parse_sex,
        __user_loader_parse_passport,
        __user_loader_parse_country_code,
        __user_loader_parse_non_empty,
        __user_loader_parse_account_creation_date,
        __user_loader_parse_non_empty,
        __user_loader_parse_account_status,
    };

    fixed_n_delimiter_parser_grammar_t *const line_grammar =
        fixed_n_delimiter_parser_grammar_new(';', 12, token_callbacks);
    if (!line_grammar) {
        user_free(data.current_user);
        return 1;
    }

    dataset_parser_grammar_t *const grammar =
        dataset_parser_grammar_new('\n',
                                   line_grammar,
                                   __users_loader_before_parse_line,
                                   __users_loader_after_parse_line);
    if (!grammar) {
        fixed_n_delimiter_parser_grammar_free(line_grammar);
        user_free(data.current_user);
        return 1;
    }

    const int retval = dataset_parser_parse(stream, grammar, &data);

    fixed_n_delimiter_parser_grammar_free(line_grammar);
    dataset_parser_grammar_free(grammar);
    user_free(data.current_user);

    return retval != 0;
}
