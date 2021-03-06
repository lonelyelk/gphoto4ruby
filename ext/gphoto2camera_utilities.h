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
#include <string.h>
#include <gphoto2/gphoto2.h>
#include <ruby.h>


#ifndef RSTRING_PTR
#define RSTRING_PTR(c) (RSTRING(c)->ptr)
#endif
#ifndef RARRAY_PTR
#define RARRAY_PTR(c) (RARRAY(c)->ptr)
#endif
#ifndef RARRAY_LEN
#define RARRAY_LEN(c) (RARRAY(c)->len)
#endif

#ifndef _INC_CAMERA_UTILITIES
#define _INC_CAMERA_UTILITIES

typedef struct {
    Camera *camera;
    CameraWidget *config;
    CameraWidget *childConfig;
    GPContext *context;
    
    char *virtFolder;
    char *lastName;

    CameraAbilities abilities;
    int disposed;
} GPhoto2Camera;

extern VALUE rb_cGPhoto2Exception;

void rb_raise_gp_result(int retval);
int gp_result_check(int retval);
void check_disposed(GPhoto2Camera* c);

VALUE getRadio(CameraWidget *cc);
VALUE listRadio(CameraWidget *cc);
VALUE setRadio(VALUE self, GPhoto2Camera *c, VALUE newVal, int save);
VALUE getText(CameraWidget *cc);
VALUE setText(VALUE self, GPhoto2Camera *c, VALUE newVal, int save);
VALUE getRange(CameraWidget *cc);
VALUE listRange(CameraWidget *cc);
VALUE setRange(VALUE self, GPhoto2Camera *c, VALUE newNum, int save);
VALUE getToggle(CameraWidget *cc);
VALUE setToggle(VALUE self, GPhoto2Camera *c, VALUE newVal, int save);
VALUE getDate(CameraWidget *cc);
VALUE setDate(VALUE self, GPhoto2Camera *c, VALUE newNum, int save);
void saveConfigs(VALUE self, GPhoto2Camera *c);

void populateWithConfigs(CameraWidget *cc, VALUE arr);

#endif /* _INC_CAMERA_UTILITIES */

