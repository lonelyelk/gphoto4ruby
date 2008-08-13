/**
 *
 * Copyright 2008 neq4 company
 * Author: Sergey Kruk (sergey.kruk@gmail.com)
 *
 * This file is part of GPhoto4Ruby.
 *
 * GPhoto4Ruby is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * GPhoto4Ruby is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GPhoto4Ruby. If not, see <http://www.gnu.org/licenses/>.
 *
 */

typedef struct {
    Camera *camera;
    CameraWidget *config;
    CameraWidget *childConfig;
    CameraFilePath filepath;
    GPContext *context;
} GPhoto2Camera;

