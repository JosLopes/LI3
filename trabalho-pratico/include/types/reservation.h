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

#ifndef FLIGHT_H
#define FLIGHT_H

enum includes_breakfast {zero, one, no_string, f, false, t, true};

typedef struct reservation reservation;

reservation* create_reservation (void);

void set_reservation_user_id (reservation* reservation, char* parsed_user_id);

void set_reservation_user_hotel_id (reservation* reservation, char* parsed_hotel_id);

void set_reservation_user_hotel_name (reservation* reservation, char* parsed_hotel_name);

void set_reservation_rating (reservation* reservation, int* parsed_rating);

void set_reservation_includes_breakfast (reservation* reservation,
                                         char* parsed_includes_breakfast);

void set_reservation_hotel_stars (reservation* reservation, int parsed_hotel_stars);

void set_reservation_begin_date (reservation* reservation, int parsed_begin_date);

void set_reservation_end_date (reservation* reservation, int parsed_end_date);

void set_reservation_city_tax (reservation* reservation, int parsed_city_tax);

void set_reservation_price_per_night (reservation* reservation, int parsed_price_per_night);

const char* get_const_reservation_user_id (reservation* reservation);

const char* get_const_reservation_hotel_id (reservation* reservation);

const char* get_const_reservation_hotel_name (reservation* reservation);

const int* get_const_reservation_rating (reservation* reservation);

enum includes_breakfast get_reservation_includes_breakfast (reservation* reservation);

int get_reservation_hotel_stars (reservation* reservation);

int get_reservation_begin_date (reservation* reservation);

int get_reservation_end_date (reservation* reservation);

int get_reservation_city_tax (reservation* reservation);

int get_reservation_price_per_night (reservation* reservation);

void free_reservation (reservation* reservation);

#endif