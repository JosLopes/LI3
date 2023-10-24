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
 * @file  email.c
 * @brief Implementation of methods in include/types/email.h
 *
 * ### Examples
 * See [the header file's documentation](@ref email_examples).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types/email.h"
#include "utils/fixed_n_delimiter_parser.h"

/**
 * @brief Grammar for parsing emails.
 * @details Shall not be modified apart from its creation.
 */
fixed_n_delimiter_parser_grammar_t *email_grammar = NULL;

/**
 * @brief Grammar for parsing email domains.
 * @details Shall not be modified apart from its creation.
 */
fixed_n_delimiter_parser_grammar_t *domain_grammar = NULL;

/** @brief Auxiliary method for ::email_validate_string. Validates a username in an email. */
int __email_validate_username(void *user_data, char *token, size_t ntoken) {
    (void) user_data;
    (void) ntoken;
    return (*token == '\0'); /* Fail on empty string */
}

/** @brief Method used with `atexit` to free the grammar created by ::__email_validate_domain. */
void __email_validate_domain_free_grammar(void) {
    free(domain_grammar);
}

/** @brief Auxiliary method for ::__email_validate_domain. Validates a name in an email's domain. */
int __email_validate_domain_name(void *user_data, char *token, size_t ntoken) {
    (void) user_data;
    (void) ntoken;
    return (*token == '\0'); /* Fail on empty string */
}

/** @brief Auxiliary method for ::__email_validate_domain. Validates a TLD in an email's domain. */
int __email_validate_domain_tld(void *user_data, char *token, size_t ntoken) {
    (void) user_data;
    (void) ntoken;
    return (strlen(token) < 2);
}

/** @brief Auxiliary method for ::email_validate_string. Validates a domain in an email. */
int __email_validate_domain(void *user_data, char *token, size_t ntoken) {
    (void) user_data;
    (void) ntoken;

    /* Create grammar if needed */
    if (!domain_grammar) {
        fixed_n_delimiter_parser_iter_callback_t callbacks[2] = {__email_validate_domain_name,
                                                                 __email_validate_domain_tld};

        /* NOTE - Make thread-safe if multithreading gets implemented */
        domain_grammar = fixed_n_delimiter_parser_grammar_new('.', 2, callbacks);
        if (!domain_grammar) {
            return 1;
        }
        atexit(__email_validate_domain_free_grammar);
    }

    return fixed_n_delimiter_parser_parse_string(token, domain_grammar, NULL);
}

/** @brief Method used with `atexit` to free the grammar created by ::email_validate_string. */
void __email_validate_free_grammar(void) {
    free(email_grammar);
}

int email_validate_string(char *input) {
    /* Create grammar if needed */
    if (!email_grammar) {
        fixed_n_delimiter_parser_iter_callback_t callbacks[2] = {__email_validate_username,
                                                                 __email_validate_domain};

        /* NOTE - Make thread-safe if multithreading gets implemented */
        email_grammar = fixed_n_delimiter_parser_grammar_new('@', 2, callbacks);
        if (!email_grammar) {
            return 1;
        }
        atexit(__email_validate_free_grammar);
    }

    int retval = fixed_n_delimiter_parser_parse_string(input, email_grammar, NULL);
    if (retval)
        return 1; /* Transform all error codes into 1 */
    return 0;
}

int email_validate_string_const(const char *input) {
    char *buffer = strdup(input);
    if (!buffer)
        return 1;

    int retval = email_validate_string(buffer);
    free(buffer);
    return retval;
}
