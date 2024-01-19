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
 * @file  activity_license.h
 * @brief Activity where the user can read the license of the application.
 *
 * @anchor activity_license_examples
 * ### Examples
 *
 * Showing the Apache License is as simple as calling ::activity_license_run. Here's how the top
 * part of the license will appear on screen.
 *
 * ```text
 * +----------LICENSE----------+
 * |       Apache License      |
 * | Version 2.0, January 2004 |
 * |            ...            |
 * +---------------------------+
 * ```
 */

#ifndef ACTIVITY_LICENSE_H
#define ACTIVITY_LICENSE_H

/**
 * @brief Runs a TUI activity for showing the Apache License.
 *
 * #### Examples
 * See [the header file's documentation](@ref activity_license_examples).
 */
void activity_license_run(void);

#endif
