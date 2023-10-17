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

#include "types/user.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

struct user {
    char* name;
    char* passport;
    char* country_code;
    char  sex;
    bool  active_status;
    int   account_creation_date;
};

user* create_user (void) {
    user* new_user = malloc (sizeof (user));
    new_user -> name = NULL;
    new_user -> passport = NULL;
    new_user -> country_code = NULL;

    return new_user;
}

void set_user_name (user* user, char* parsed_name) {user -> name = strdup (parsed_name);} 

void set_user_passport (user* user, char* parsed_passport)
    {user -> passport = strdup (parsed_passport);}

void set_user_country_code (user* user, char* parsed_country_code)
    {user -> country_code = strdup (parsed_country_code);}

void set_user_sex (user* user, char* parsed_sex_string) {
  char sex;

  if (!strcmp(parsed_sex_string, "M")) {
    sex = 'M';
  } else {
    sex = 'F';
  }

  user -> sex = sex;
}

void set_user_account_status (user* user, bool active_status)
    {user -> active_status = active_status;}

void set_user_account_creation_date (user* user, int parsed_date)
    {user -> account_creation_date = parsed_date;}

const char* get_user_name (user* user) {return user -> name;}

const char* get_user_passport (user* user) {return user -> country_code;}

const char* get_user_country_code (user* user) {return user -> country_code;}

char get_user_sex (user* user) {return user -> sex;}

bool get_user_account_status (user* user) {return user -> active_status;}

int get_user_account_creation_date (user* user)
    {return user -> account_creation_date;}