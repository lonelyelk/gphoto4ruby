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

#include <ruby.h>
#include <gphoto2/gphoto2-version.h>
#include "gphoto2camera_event.h"
#include "gphoto2camera_utilities.h"
#include "gphoto2camera.h"

void Init_gphoto4ruby() {
    const char **v = gp_library_version(GP_VERSION_SHORT);
    char libGPVersion[256];
    int i;

    strcpy(libGPVersion, v[0]);
    for (i = 1; v[i] != NULL; i++) {
        if (v[i+1] != NULL) {
            strcat(libGPVersion, v[i]);
            strcat(libGPVersion, ", ");
        } else {
            strcat(libGPVersion, v[i]);
        }
    }

    /*
     * Module contains camera class definition and some exceptions.
     */
    rb_mGPhoto2 = rb_define_module("GPhoto2");
    /*
     * Version of libgphoto used by gem
     */
    rb_define_const(rb_mGPhoto2, "LIBGPHOTO2_VERSION", rb_str_new2(libGPVersion));
    /*
     * GPhoto2::Camera object is a Ruby wrapping aroung gphoto2 C library.
     */
    rb_cGPhoto2Camera = rb_define_class_under(rb_mGPhoto2, "Camera", rb_cObject);
    /*
     * GPhoto2::CameraEvent is returned by camera.wait function. Probable usage
     * is waiting for camera to save files if you capture them manually and not
     * with camera.capture method. This class has no constructor.
     */
    rb_cGPhoto2CameraEvent = rb_define_class_under(rb_mGPhoto2, "CameraEvent", rb_cObject);
    /*
     * GPhoto2::Exception is raised when libgphoto2 functions from C core
     * return any error code.
     */
    rb_cGPhoto2Exception = rb_define_class_under(rb_mGPhoto2, "Exception", rb_eStandardError);
    /*
     * GPhoto2::ConfigurationError is raised when trying to set configuration
     * values that are not allowed or trying to access properties that are not
     * supported.
     */
    rb_cGPhoto2ConfigurationError = rb_define_class_under(rb_mGPhoto2, "ConfigurationError", rb_eStandardError);
    
    rb_define_const(rb_cGPhoto2Camera, "CONFIG_TYPE_RADIO", INT2FIX(GP_WIDGET_RADIO));
    rb_define_const(rb_cGPhoto2Camera, "CONFIG_TYPE_TEXT", INT2FIX(GP_WIDGET_TEXT));
    rb_define_const(rb_cGPhoto2Camera, "CONFIG_TYPE_RANGE", INT2FIX(GP_WIDGET_RANGE));
    rb_define_const(rb_cGPhoto2Camera, "CONFIG_TYPE_TOGGLE", INT2FIX(GP_WIDGET_TOGGLE));
    rb_define_const(rb_cGPhoto2Camera, "CONFIG_TYPE_DATE", INT2FIX(GP_WIDGET_DATE));

    rb_define_const(rb_cGPhoto2CameraEvent, "EVENT_TYPE_UNKNOWN", EVENT_UNKNOWN);
    rb_define_const(rb_cGPhoto2CameraEvent, "EVENT_TYPE_TIMEOUT", EVENT_TIMEOUT);
    rb_define_const(rb_cGPhoto2CameraEvent, "EVENT_TYPE_FILE_ADDED", EVENT_FILE_ADDED);
    rb_define_const(rb_cGPhoto2CameraEvent, "EVENT_TYPE_FOLDER_ADDED", EVENT_FOLDER_ADDED);
    
    rb_define_alloc_func(rb_cGPhoto2Camera, camera_allocate);
    rb_define_module_function(rb_cGPhoto2Camera, "ports", camera_class_ports, 0); /* in gphoto2camera.c */
    rb_define_method(rb_cGPhoto2Camera, "initialize", camera_initialize, -1); /* in gphoto2camera.c */
    rb_define_method(rb_cGPhoto2Camera, "config", camera_get_config, -1); /* in gphoto2camera.c */
    rb_define_method(rb_cGPhoto2Camera, "config_merge", camera_config_merge, 1); /* in gphoto2camera.c */
    rb_define_method(rb_cGPhoto2Camera, "[]", camera_get_value, -1); /* in gphoto2camera.c */
    rb_define_method(rb_cGPhoto2Camera, "[]=", camera_set_value, 2); /* in gphoto2camera.c */
    rb_define_method(rb_cGPhoto2Camera, "capture", camera_capture, -1); /* in gphoto2camera.c */
    rb_define_method(rb_cGPhoto2Camera, "save", camera_save, -1); /* in gphoto2camera.c */
    rb_define_method(rb_cGPhoto2Camera, "delete", camera_delete, -1); /* in gphoto2camera.c */
    rb_define_method(rb_cGPhoto2Camera, "folder", camera_folder, 0); /* in gphoto2camera.c */
    rb_define_method(rb_cGPhoto2Camera, "subfolders", camera_subfolders, 0); /* in gphoto2camera.c */
    rb_define_method(rb_cGPhoto2Camera, "files", camera_files, -1); /* in gphoto2camera.c */
    rb_define_method(rb_cGPhoto2Camera, "files_count", camera_files_count, 0); /* in gphoto2camera.c */
    rb_define_method(rb_cGPhoto2Camera, "folder_up", camera_folder_up, 0); /* in gphoto2camera.c */
    rb_define_method(rb_cGPhoto2Camera, "folder_down", camera_folder_down, 1); /* in gphoto2camera.c */
    rb_define_method(rb_cGPhoto2Camera, "create_folder", camera_create_folder, 1); /* in gphoto2camera.c */
    rb_define_method(rb_cGPhoto2Camera, "wait", camera_wait, -1); /* in gphoto2camera.c */

    rb_define_method(rb_cGPhoto2CameraEvent, "type", camera_event_type, 0); /* in gphoto2camera_event.c */
    rb_define_method(rb_cGPhoto2CameraEvent, "file", camera_event_file, 0); /* in gphoto2camera_event.c */
}

