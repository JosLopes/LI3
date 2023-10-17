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
  * @file   user.h
  * @brief  Contains data structures and functions that apply to the type @typedef struct user. 
  */

#ifndef USER_H
#define USER_H

#include <stdbool.h>

typedef struct user user;

user* create_user (void);

void set_user_name (user* user, char* parsed_name);

void set_user_passport (user* user, char* parsed_passport);

void set_user_country_code (user* user, char* parsed_country_code);

void set_user_sex (user* user, char* parsed_sex_string);

void set_user_account_status (user* user, bool active_status);

void set_user_account_creation_date (user* user, int parsed_date);

const char* get_user_name (user* user);

const char* get_user_passport (user* user);

const char* get_user_country_code (user* user);

char get_user_sex (user* user);

bool get_user_account_status (user* user);

int get_user_account_creation_date (user* user);

#endif