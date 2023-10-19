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

#include "types/reservation.h"
#include <stdlib.h>
#include <string.h>

struct reservation {
    char* user_id;
    char* hotel_id;
    char* hotel_name;
    int*  rating;    //if Null the rating doesn't exist;
    enum  includes_breakfast includes_breakfast;
    int   id; // + BOOK behind;
    int   hotel_stars;
    int   begin_date;
    int   end_date;
    int   city_tax;
    int   price_per_night;
};

reservation* create_reservation (void) {
    reservation* new_reservation = malloc (sizeof (reservation));

    new_reservation -> user_id = NULL;
    new_reservation -> hotel_id = NULL;
    new_reservation -> hotel_name = NULL;
    new_reservation -> rating = NULL;

    return new_reservation;
}

void set_reservation_user_id (reservation* reservation, char* parsed_user_id)
    {reservation -> user_id = strdup (parsed_user_id);}

void set_reservation_user_hotel_id (reservation* reservation, char* parsed_hotel_id)
    {reservation -> hotel_id = strdup (parsed_hotel_id);}

void set_reservation_user_hotel_name (reservation* reservation, char* parsed_hotel_name)
    {reservation -> hotel_name = strdup (parsed_hotel_name);}

void set_reservation_rating (reservation* reservation, int* parsed_rating)
    {reservation -> rating = parsed_rating;}

void set_reservation_includes_breakfast (reservation* reservation,
                                         char*        parsed_includes_breakfast) {
    enum includes_breakfast includes_breakfast;

    if (parsed_includes_breakfast == NULL) includes_breakfast = no_string;
    else {
        switch (parsed_includes_breakfast[0]) {
            case 'f':
                if (strlen (parsed_includes_breakfast) == 1) includes_breakfast = f;
                else includes_breakfast = false;
                break;
            case 't':
                if (strlen (parsed_includes_breakfast) == 1) includes_breakfast = t;
                else includes_breakfast = true;
                break;
            case 1:
                includes_breakfast = one;
                break;
            default:
                includes_breakfast = zero;
                break;
        }
    }

    reservation -> includes_breakfast = includes_breakfast;
}

void set_reservation_hotel_stars (reservation* reservation, int parsed_hotel_stars)
    {reservation -> hotel_stars = parsed_hotel_stars;}

void set_reservation_begin_date (reservation* reservation, int parsed_begin_date)
    {reservation -> begin_date = parsed_begin_date;}

void set_reservation_end_date (reservation* reservation, int parsed_end_date)
    {reservation -> end_date = parsed_end_date;}

void set_reservation_city_tax (reservation* reservation, int parsed_city_tax)
    {reservation -> city_tax = parsed_city_tax;}

void set_reservation_price_per_night (reservation* reservation, int parsed_price_per_night)
    {reservation -> price_per_night = parsed_price_per_night;}

const char* get_const_reservation_user_id (reservation* reservation)
    {return reservation -> user_id;}

const char* get_const_reservation_hotel_id (reservation* reservation)
    {return reservation -> hotel_id;}

const char* get_const_reservation_hotel_name (reservation* reservation)
    {return reservation -> hotel_name;}

const int* get_const_reservation_rating (reservation* reservation)
    {return reservation -> rating;}

enum includes_breakfast get_reservation_includes_breakfast (reservation* reservation)
    {return reservation -> includes_breakfast;}

int get_reservation_hotel_stars (reservation* reservation) {return reservation -> hotel_stars;}

int get_reservation_begin_date (reservation* reservation) {return reservation -> begin_date;}

int get_reservation_end_date (reservation* reservation) {return reservation -> end_date;}

int get_reservation_city_tax (reservation* reservation) {return reservation -> city_tax;}

int get_reservation_price_per_night (reservation* reservation)
    {return reservation -> price_per_night;}

void free_reservation (reservation* reservation) {
    free (reservation -> user_id);
    free (reservation -> hotel_id);
    free (reservation -> hotel_name);
    free (reservation -> rating);

    free (reservation);
}