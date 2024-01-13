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

#include <stdlib.h>
#include <string.h>

#include "types/email.h"
#include "utils/fixed_n_delimiter_parser.h"

/**
 * @brief   Grammar for validating emails.
 * @details Shall not be modified apart from its creation. It's not constant because it requires
 *          run-time initialization. This global variable is justified for the following reasons:
 *
 *          -# It's not modified (no mutable global state);
 *          -# It's module-local (no breaking of encapsulation);
 *          -# Helps performance, as a new grammar doesn't need to be generated for every email to
 *             be validated.
 */
fixed_n_delimiter_parser_grammar_t *__email_grammar = NULL;

/**
 * @brief   Grammar for validating email domains.
 * @details Shall not be modified apart from its creation. It's not constant because it requires
 *          run-time initialization. This global variable is justified for the following reasons:
 *
 *          -# It's not modified (no mutable global state);
 *          -# It's module-local (no breaking of encapsulation);
 *          -# Helps performance, as a new grammar doesn't need to be generated for every email to
 *             be validated.
 */
fixed_n_delimiter_parser_grammar_t *__email_domain_grammar = NULL;

/** @brief Auxiliary method for ::email_validate_string. Validates the username in an email. */
int __email_validate_username(void *user_data, char *token, size_t ntoken) {
    (void) user_data;
    (void) ntoken;
    return (*token == '\0'); /* Fail on empty string */
}

/** @brief Auxiliary method for ::__email_validate_domain. Validates the name of an email's domain. */
int __email_validate_domain_name(void *user_data, char *token, size_t ntoken) {
    (void) user_data;
    (void) ntoken;
    return (*token == '\0'); /* Fail on empty string */
}

/** @brief Auxiliary method for ::__email_validate_domain. Validates the TLD in an email's domain. */
int __email_validate_domain_tld(void *user_data, char *token, size_t ntoken) {
    (void) user_data;
    (void) ntoken;
    return (strlen(token) < 2);
}

/** @brief Auxiliary method for ::email_validate_string. Validates a domain in an email. */
int __email_validate_domain(void *user_data, char *token, size_t ntoken) {
    (void) user_data;
    (void) ntoken;
    return fixed_n_delimiter_parser_parse_string(token, __email_domain_grammar, NULL);
}

/**
 * @brief Automatically initializes ::__email_grammar and ::__email_domain_grammar when the program
 *        starts.
 */
void __attribute__((constructor)) __email_grammars_create(void) {
    const fixed_n_delimiter_parser_iter_callback_t email_callbacks[2] = {__email_validate_username,
                                                                         __email_validate_domain};
    __email_grammar = fixed_n_delimiter_parser_grammar_new('@', 2, email_callbacks);

    const fixed_n_delimiter_parser_iter_callback_t domain_callbacks[2] = {
        __email_validate_domain_name,
        __email_validate_domain_tld};
    __email_domain_grammar = fixed_n_delimiter_parser_grammar_new('.', 2, domain_callbacks);
}

/**
 * @brief Automatically frees ::__email_grammar and ::__email_domain_grammar when the program
 *        terminates.
 */
void __attribute__((destructor)) __email_grammars_free(void) {
    fixed_n_delimiter_parser_grammar_free(__email_grammar);
    fixed_n_delimiter_parser_grammar_free(__email_domain_grammar);
}

int email_validate_string(char *input) {
    return fixed_n_delimiter_parser_parse_string(input, __email_grammar, NULL) != 0;
}

int email_validate_string_const(const char *input) {
    char *buffer = strdup(input);
    if (!buffer)
        return 1;

    const int retval = email_validate_string(buffer);
    free(buffer);
    return retval;
}
