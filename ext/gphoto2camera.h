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

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <gphoto2/gphoto2.h>
#include <ruby.h>
#include "gphoto2camera_event.h"
#include "gphoto2camera_utilities.h"

#ifndef _INC_CAMERA
#define _INC_CAMERA

VALUE rb_mGPhoto2;
VALUE rb_cGPhoto2Camera;

VALUE rb_cGPhoto2ConfigurationError;

void camera_mark(GPhoto2Camera *c);
void camera_free(GPhoto2Camera *c);
VALUE camera_allocate(VALUE klass);

VALUE camera_initialize(int argc, VALUE *argv, VALUE self);
VALUE camera_class_ports(VALUE klass);

VALUE camera_capture(int argc, VALUE *argv, VALUE self);

VALUE camera_save(int argc, VALUE *argv, VALUE self);
VALUE camera_delete(int argc, VALUE *argv, VALUE self);

VALUE camera_get_config(VALUE self);
VALUE camera_config_merge(VALUE self, VALUE hash);

VALUE camera_get_value(int argc, VALUE *argv, VALUE self);
VALUE camera_set_value(VALUE self, VALUE str, VALUE newVal);

VALUE camera_folder(VALUE self);
VALUE camera_subfolders(VALUE self);
VALUE camera_files(VALUE self);
VALUE camera_folder_up(VALUE self);
VALUE camera_folder_down(VALUE self, VALUE folder);
VALUE camera_create_folder(VALUE self, VALUE folder);

VALUE camera_wait(int argc, VALUE *argv, VALUE self);

#endif /* _INC_CAMERA */
