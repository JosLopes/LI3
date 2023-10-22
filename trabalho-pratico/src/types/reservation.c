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
    char                   *user_id;
    char                   *hotel_name;
    int                    *rating;
    enum includes_breakfast includes_breakfast;
    date_t                  begin_date;
    date_t                  end_date;
    size_t                  id;
    int                     hotel_id;
    int                     hotel_stars;
    int                     city_tax;
    int                     price_per_night;
};

reservation_t *create_reservation(void) {
    reservation_t *new_reservation = malloc(sizeof(reservation_t));

    new_reservation->user_id    = NULL;
    new_reservation->hotel_name = NULL;
    new_reservation->rating     = NULL;

    return new_reservation;
}

void set_reservation_user_id(reservation_t *reservation, char *user_id) {
    reservation->user_id = strdup(user_id);
}

void set_reservation_hotel_id(reservation_t *reservation, int hotel_id) {
    reservation->hotel_id = hotel_id;
}

void set_reservation_hotel_name(reservation_t *reservation, char *hotel_name) {
    reservation->hotel_name = strdup(hotel_name);
}

void set_reservation_rating(reservation_t *reservation, int *parsed_rating) {
    reservation->rating = parsed_rating;
}

void set_reservation_includes_breakfast(reservation_t *reservation,
                                        char          *includes_breakfast_string) {
    enum includes_breakfast includes_breakfast;

    if (includes_breakfast_string == NULL)
        includes_breakfast = no_input;
    else {
        switch (includes_breakfast_string[0]) {
            case 'f':
                if (strlen(includes_breakfast_string) == 1)
                    includes_breakfast = f;
                else
                    includes_breakfast = _false;
                break;
            case 't':
                if (strlen(includes_breakfast_string) == 1)
                    includes_breakfast = t;
                else
                    includes_breakfast = _true;
                break;
            case '1':
                includes_breakfast = one;
                break;
            default:
                includes_breakfast = zero;
                break;
        }
    }

    reservation->includes_breakfast = includes_breakfast;
}

void set_reservation_hotel_stars(reservation_t *reservation, int hotel_stars) {
    reservation->hotel_stars = hotel_stars;
}

void set_reservation_id(reservation_t *reservation, size_t id) {
    reservation->id = id;
}

void set_reservation_begin_date(reservation_t *reservation, date_t begin_date) {
    reservation->begin_date = begin_date;
}

void set_reservation_end_date(reservation_t *reservation, date_t end_date) {
    reservation->end_date = end_date;
}

void set_reservation_city_tax(reservation_t *reservation, int city_tax) {
    reservation->city_tax = city_tax;
}

void set_reservation_price_per_night(reservation_t *reservation, int price_per_night) {
    reservation->price_per_night = price_per_night;
}

const char *get_const_reservation_user_id(reservation_t *reservation) {
    return reservation->user_id;
}

int get_const_reservation_hotel_id(reservation_t *reservation) {
    return reservation->hotel_id;
}

const char *get_const_reservation_hotel_name(reservation_t *reservation) {
    return reservation->hotel_name;
}

const int *get_const_reservation_rating(reservation_t *reservation) {
    return reservation->rating;
}

enum includes_breakfast get_reservation_includes_breakfast(reservation_t *reservation) {
    return reservation->includes_breakfast;
}

size_t get_reservation_id(reservation_t *reservation) {
    return reservation->id;
}

int get_reservation_hotel_stars(reservation_t *reservation) {
    return reservation->hotel_stars;
}

date_t get_reservation_begin_date(reservation_t *reservation) {
    return reservation->begin_date;
}

date_t get_reservation_end_date(reservation_t *reservation) {
    return reservation->end_date;
}

int get_reservation_city_tax(reservation_t *reservation) {
    return reservation->city_tax;
}

int get_reservation_price_per_night(reservation_t *reservation) {
    return reservation->price_per_night;
}

void free_reservation(reservation_t *reservation) {
    free(reservation->user_id);
    free(reservation->hotel_name);
    free(reservation->rating);

    free(reservation);
}
