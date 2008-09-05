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

typedef struct {
    Camera *camera;
    CameraWidget *config;
    CameraWidget *childConfig;
    CameraList *list;
    CameraFilePath path;
    CameraFile *file;
    GPContext *context;
    
    char *virtFolder;
} GPhoto2Camera;

typedef struct {
    CameraEventType type;
    CameraFilePath *path;
} GPhoto2CameraEvent;

#define EVENT_UNKNOWN rb_str_new2("unknown")
#define EVENT_TIMEOUT rb_str_new2("timeout")
#define EVENT_FILE_ADDED rb_str_new2("file added")
#define EVENT_FOLDER_ADDED rb_str_new2("folder added")

static VALUE rb_mGPhoto2;
static VALUE rb_cGPhoto2Camera;
static VALUE rb_cGPhoto2CameraEvent;

static VALUE rb_cGPhoto2Exception;
static VALUE rb_cGPhoto2ConfigurationError;

static void rb_raise_gp_result(int retval);
static void rb_raise_programmer_error(const char* fName);

static VALUE getRadio(CameraWidget *cc);
static VALUE listRadio(CameraWidget *cc);
static VALUE setRadio(VALUE self, GPhoto2Camera *c, VALUE newVal, int save);
static VALUE getRange(CameraWidget *cc);
static VALUE listRange(CameraWidget *cc);
static VALUE setRange(VALUE self, GPhoto2Camera *c, VALUE newNum, int save);
static void saveConfigs(VALUE self, GPhoto2Camera *c);

static void populateWithConfigs(CameraWidget *cc, VALUE arr);

static void camera_mark(GPhoto2Camera *c);
static void camera_free(GPhoto2Camera *c);
static VALUE camera_allocate(VALUE klass);

static VALUE camera_initialize(int argc, VALUE *argv, VALUE self);
static VALUE camera_class_ports(VALUE klass);

static VALUE camera_capture(int argc, VALUE *argv, VALUE self);

static VALUE camera_save(int argc, VALUE *argv, VALUE self);

static VALUE camera_get_config(VALUE self);
static VALUE camera_config_merge(VALUE self, VALUE hash);

static VALUE camera_get_value(int argc, VALUE *argv, VALUE self);
static VALUE camera_set_value(VALUE self, VALUE str, VALUE newVal);

static VALUE camera_folder(VALUE self);
static VALUE camera_subfolders(VALUE self);
static VALUE camera_files(VALUE self);
static VALUE camera_folder_up(VALUE self);
static VALUE camera_folder_down(VALUE self, VALUE folder);
