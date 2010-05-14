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

#include "gphoto2camera_event.h"

VALUE rb_cGPhoto2CameraEvent;

void camera_event_mark(GPhoto2CameraEvent *ce) {
}

void camera_event_free(GPhoto2CameraEvent *ce) {
    free(ce);
}

/*
 * call-seq:
 *   type                           =>      string
 *
 * Returns type of event. Can be compared to EVENT_TYPE class constants
 *
 * Examples:
 *
 *   c = GPhoto2::Camera.new
 *   # capture the image manually
 *   evt = c.wait
 *   evt.type                       #=>     "file added"
 *   evt.type == GPhoto2::CameraEvent::EVENT_TYPE_FILE_ADDED
 *                                  #=>     true
 *   evt.file                       #=>     "DSC_0384.JPG"
 *   
 *   # do nothing
 *   c.wait(1).type                 #=>     "timeout"
 */
VALUE camera_event_type(VALUE self) {
    GPhoto2CameraEvent *ce;
    
    Data_Get_Struct(self, GPhoto2CameraEvent, ce);
    
    switch (ce->type) {
        case GP_EVENT_FILE_ADDED:
            return EVENT_FILE_ADDED;
        case GP_EVENT_FOLDER_ADDED:
            return EVENT_FOLDER_ADDED;
        case GP_EVENT_TIMEOUT:
            return EVENT_TIMEOUT;
        case GP_EVENT_UNKNOWN:
            return EVENT_UNKNOWN;
        default:
            return Qnil;
    }
}

/*
 * call-seq:
 *   file                           =>      string or nil
 *
 * Returns file name of manually captured image. Only applies to
 * EVENT_TYPE_FILE_ADDED or EVENT_TYPE_FOLDER_ADDED event.
 *
 * Examples:
 *
 *   c = GPhoto2::Camera.new
 *   # capture the image manually
 *   evt = c.wait
 *   evt.type                       #=>     "file added"
 *   evt.type == GPhoto2::CameraEvent::EVENT_TYPE_FILE_ADDED
 *                                  #=>     true
 *   evt.file                       #=>     "DSC_0384.JPG"
 *   
 *   # do nothing
 *   c.wait(1).type                 #=>     "timeout"
 */
VALUE camera_event_file(VALUE self) {
    GPhoto2CameraEvent *ce;
    
    Data_Get_Struct(self, GPhoto2CameraEvent, ce);
    
    if ((ce->type == GP_EVENT_FILE_ADDED) ||
        (ce->type == GP_EVENT_FOLDER_ADDED)) {
        return rb_str_new2(ce->path->name);
    } else {
        return Qnil;
    }
}

/*
 * call-seq:
 *   folder                         =>      string or nil
 *
 * Returns file name of manually captured image. Only applies to
 * EVENT_TYPE_FILE_ADDED or EVENT_TYPE_FOLDER_ADDED event.
 *
 * Examples:
 *
 *   c = GPhoto2::Camera.new
 *   # capture the image manually
 *   evt = c.wait
 *   evt.type                       #=>     "file added"
 *   evt.type == GPhoto2::CameraEvent::EVENT_TYPE_FOLDER_ADDED
 *                                  #=>     true
 *   evt.file                       #=>     "101NCD80"
 *   evt.folder                     #=>     "/store_00010001/DCIM"
 * 
 * In example above is default behaviour for that type of event when it occurs
 */
VALUE camera_event_folder(VALUE self) {
    GPhoto2CameraEvent *ce;
    
    Data_Get_Struct(self, GPhoto2CameraEvent, ce);
    
    if ((ce->type == GP_EVENT_FILE_ADDED) ||
        (ce->type == GP_EVENT_FOLDER_ADDED)) {
        return rb_str_new2(ce->path->folder);
    } else {
        return Qnil;
    }
}

