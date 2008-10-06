/**
 *
 * Copyright 2008 neq4 company <http://neq4.com>
 * Author: Sergey Kruk <sergey.kruk@gmail.com>
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

#include <gphoto2/gphoto2.h>
#include <ruby.h>

#ifndef _INC_CAMERA_EVENT
#define _INC_CAMERA_EVENT

typedef struct {
    CameraEventType type;
    CameraFilePath *path;
} GPhoto2CameraEvent;

VALUE rb_cGPhoto2CameraEvent;

#define EVENT_UNKNOWN rb_str_new2("unknown")
#define EVENT_TIMEOUT rb_str_new2("timeout")
#define EVENT_FILE_ADDED rb_str_new2("file added")
#define EVENT_FOLDER_ADDED rb_str_new2("folder added")

void camera_event_mark(GPhoto2CameraEvent *ce);
void camera_event_free(GPhoto2CameraEvent *ce);

VALUE camera_event_type(VALUE self);
VALUE camera_event_file(VALUE self);

#endif /* _INC_CAMERA_EVENT */
