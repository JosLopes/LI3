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
 * @file    screen_loading_dataset.h
 * @brief   An `ncurses` screen that shows that a dataset is being loaded and that the program may
 *          become unresponsive.
 * @details Its appearance on screen will be the following:
 *
 * ```text
 * +------------------------------------------------------------+
 * |                                                            |
 * | Loading dataset. The application will become unresponsive. |
 * |                                                            |
 * +------------------------------------------------------------+
 * ```
 */

/**
 * @brief Renders an `ncurses`'s screen that shows that a dataset is being loaded and that the
 *        program may become unresponsive.
 */
void screen_loading_dataset_render(void);
