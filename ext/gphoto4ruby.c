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
#include "gphoto4ruby.h"

static void rb_raise_gp_result(int retval) {
    rb_raise(rb_cGPhoto2Exception, "LibGPhoto2 function returned: %s", gp_result_as_string(retval));
}

static void rb_raise_programmer_error(const char* fName) {
    rb_raise(rb_cGPhoto2ProgrammerError, "Program was not supposed to get here. Function: %s", fName);
}

static VALUE getRadio(CameraWidget *cc) {
    int retval;
    const char *val;
    retval = gp_widget_get_value(cc, &val);
    if (retval == GP_OK) {
        return rb_str_new2(val);
    } else {
        rb_raise_gp_result(retval);
        return Qnil;
    }
}

static VALUE listRadio(CameraWidget *cc) {
    int retval, i, choicesTotal;
    const char *choice;
    VALUE arr;

    choicesTotal = gp_widget_count_choices(cc);
    arr = rb_ary_new();
    for (i = 0; i < choicesTotal; i++) {
        retval = gp_widget_get_choice(cc, i, &choice);
        if (retval == GP_OK) {
            rb_ary_push(arr, rb_str_new2(choice));
        } else {
            rb_raise_gp_result(retval);
            return Qnil;
        }
    }
    return arr;}

static VALUE setRadio(GPhoto2Camera *c, VALUE newVal) {
    int retval, i, choicesTotal;
    const char *choice;
    const char *val;

    Check_Type(newVal, T_STRING);
    val = RSTRING(newVal)->ptr;

    choicesTotal = gp_widget_count_choices(c->childConfig);
    for (i = 0; i < choicesTotal; i++) {
        gp_widget_get_choice(c->childConfig, i, &choice);
        if (strcmp(choice, val) == 0) {
            retval = gp_widget_set_value(c->childConfig, val);
            if (retval == GP_OK) {
                retval = gp_camera_set_config(c->camera, c->config, c->context);
                if (retval == GP_OK) {
                    return newVal;
                }
            }
        }
    }
    if (retval != GP_OK) {
        rb_raise_gp_result(retval);
    } else {
        rb_raise(rb_cGPhoto2ConfigurationError, "Value '%s' is not allowed", val);
    }
    return Qnil;
}

static VALUE getRange(CameraWidget *cc) {
    int retval;
    float val;
    retval = gp_widget_get_value(cc, &val);
    if (retval == GP_OK) {
        return rb_float_new(val);
    } else {
        rb_raise_gp_result(retval);
        return Qnil;
    }
}

static VALUE listRange(CameraWidget *cc) {
    int retval;
    float min, max, inc, i;
    VALUE arr;

    retval = gp_widget_get_range(cc, &min, &max, &inc);
    if ((retval == GP_OK) && (inc > 0)) {
        arr = rb_ary_new();
        for (i = min; i <= max; i = i + inc) {
            rb_ary_push(arr, rb_float_new(i));
        }
        return arr;
    } else if (retval != GP_OK) {
        rb_raise_gp_result(retval);
    } else {
        return rb_ary_new();
    }
    return Qnil;
}

static VALUE setRange(GPhoto2Camera *c, VALUE newNum) {
    int retval;
    float min, max, inc, i;
    float val;

    Check_Type(newNum, T_FLOAT);
    val = NUM2DBL(newNum);

    retval = gp_widget_get_range(c->childConfig, &min, &max, &inc);
    if ((retval == GP_OK) && (val >= min) && (val <= max) && (inc > 0)) {
        for (i = min; i <= max; i = i + inc) {
            if ((val >= i) && (val <= (i+inc))) {
                if ((val - i) > (inc / 2.0)) {
                    val = i + inc;
                } else {
                    val = i;
                }
                retval = gp_widget_set_value(c->childConfig, &val);
                if (retval == GP_OK) {
                    retval = gp_camera_set_config(c->camera, c->config, c->context);
                    if (retval == GP_OK) {
                        return rb_float_new(val);
                    }
                }
            }
        }
    }
    if (retval != GP_OK) {
        rb_raise_gp_result(retval);
    } else {
        rb_raise(rb_cGPhoto2ConfigurationError, "Value has to be in range: %f .. %f", min, max);
    }
    return Qnil;
}

static void populateWithConfigs(CameraWidget *cc, VALUE arr) {
    int retval, i, childrenTotal;
    const char *name;
    CameraWidget *child;
    CameraWidgetType widgettype;
    
    retval = gp_widget_get_type(cc, &widgettype);
    if (retval == GP_OK) {
        switch (widgettype) {
            case GP_WIDGET_RADIO:
            case GP_WIDGET_RANGE:
                retval = gp_widget_get_name(cc, &name);
                if (retval == GP_OK) {
                    rb_ary_push(arr, rb_str_new2(name));
                }
                break;
            case GP_WIDGET_WINDOW:
            case GP_WIDGET_SECTION:
                childrenTotal = gp_widget_count_children(cc);
                for (i = 0; i < childrenTotal; i ++) {
                    retval = gp_widget_get_child(cc, i, &child);
                    if (retval == GP_OK) {
                        populateWithConfigs(child, arr);
                    }
                }
                break;
        }
    }
    if (retval != GP_OK) {
        rb_raise_gp_result(retval);
    }
}

static void camera_mark(GPhoto2Camera *c) {
}

static void camera_free(GPhoto2Camera *c) {
    int retval;
    retval = gp_camera_exit(c->camera, c->context);
    retval = gp_widget_free(c->config);
    retval = gp_list_free(c->list);
    retval = gp_file_free(c->file);
    retval = gp_camera_free(c->camera);
    free(c->virtFolder);
    free(c->context);
    free(c);
}

static VALUE camera_allocate(VALUE klass) {
    int retval;
    GPhoto2Camera *c;
    c = (GPhoto2Camera*) malloc(sizeof(GPhoto2Camera));
    c->virtFolder = (char*) malloc(sizeof(char)*100);
    strcpy(c->virtFolder, "/");
    c->context = gp_context_new();
    retval = gp_camera_new(&(c->camera));
    if (retval == GP_OK) {
        retval = gp_list_new(&(c->list));
        if (retval == GP_OK) {
            retval = gp_camera_get_config(c->camera, &(c->config), c->context);
            if (retval == GP_OK) {
                retval = gp_file_new(&(c->file));
                if (retval == GP_OK) {
                    return Data_Wrap_Struct(klass, camera_mark, camera_free, c);
                }
            }
        }
    }
    rb_raise_gp_result(retval);
    return Qnil;
}

/*
 * call-seq:
 *   GPhoto2::Camera.new(port=nil)
 *
 * Returns camera object. Camera should be connected at a time constructor
 * is called. If there is more than one camera connected through usb ports,
 * port parameter can be passed to specify which camera is addressed with
 * object.
 *
 * Examples:
 *
 *   GPhoto2::Camera.new
 *   GPhoto2::Capera.new(GPhoto2::Camera.ports[0])
 *
 */
static VALUE camera_initialize(int argc, VALUE *argv, VALUE self) {
    switch (argc) {
        case 0:
            return self;
            break;
        case 1:
            Check_Type(argv[0], T_STRING);
            int retval, portIndex;
            GPPortInfoList *portInfoList;
            GPPortInfo p;
            GPhoto2Camera *c;

            Data_Get_Struct(self, GPhoto2Camera, c);
            
            retval = gp_port_info_list_new(&portInfoList);
            if (retval == GP_OK) {
                retval = gp_port_info_list_load(portInfoList);
                if (retval == GP_OK) {
                    portIndex = gp_port_info_list_lookup_path(portInfoList, RSTRING(argv[0])->ptr);
                    if (portIndex >= 0) {
                        retval = gp_port_info_list_get_info(portInfoList, portIndex, &p);
                        if (retval == GP_OK) {
                            retval = gp_camera_set_port_info(c->camera, p);
                            if (retval == GP_OK) {
                                return self;
                            }
                        }
                    } else {
                        rb_raise_gp_result(portIndex);
                        return Qnil;
                    }
                }
            }
            rb_raise_gp_result(retval);
            return Qnil;
        default:
            rb_raise(rb_eArgError, "Wrong number of arguments (%d for 0 or 1)", argc);
            return Qnil;
    }
}

/*
 * call-seq:
 *   GPhoto2::Camera.ports          =>      array
 *
 * Returns an array of usb port paths with cameras. If only one camera
 * is connected, returned array is empty.
 *
 * Examples:
 *
 *   # with one camera connected
 *   GPhoto2::Camera.ports          #=>     []
 *   # with two cameras connected
 *   GPhoto2::Camera.ports          #=>     ["usb:005,004", "usb:005,006"]
 *
 */
static VALUE camera_class_ports(VALUE klass) {
    int retval, i, portsTotal;
    GPPortInfoList *portInfoList;
    GPPortInfo p;
    VALUE arr;
    
    retval = gp_port_info_list_new(&portInfoList);
    if (retval == GP_OK) {
        retval = gp_port_info_list_load(portInfoList);
        if (retval == GP_OK) {
            portsTotal = gp_port_info_list_count(portInfoList);
            arr = rb_ary_new();
            for(i = 0; i < portsTotal; i++) {
                retval = gp_port_info_list_get_info(portInfoList, i, &p);
                if ((retval == GP_OK) && (strlen(p.path) > 4) && (strncmp(p.path, "usb:", 4) == 0)) {
                    rb_ary_push(arr, rb_str_new2(p.path));
                }
            }
            retval = gp_port_info_list_free(portInfoList);
            if (retval == GP_OK) {
                return arr;
            }
        }
    }
    rb_raise_gp_result(retval);
    return Qnil;
}

/*
 * call-seq:
 *   capture                        =>      camera
 *
 * Sends command to camera to capture image with current configuration
 *
 * Examples:
 *
 *   c = GPhoto2::Camera.new
 *   c.capture
 *
 */
static VALUE camera_capture(VALUE self) {
    int retval;
    GPhoto2Camera *c;

    Data_Get_Struct(self, GPhoto2Camera, c);

    retval = gp_camera_capture(c->camera, GP_CAPTURE_IMAGE, &(c->path), c->context);
    if (retval == GP_OK) {
        strcpy(c->virtFolder, c->path.folder);
        printf("captured: %s/%s\n", c->path.folder, c->path.name);
        return self;
    } else {
        rb_raise_gp_result(retval);
        return Qnil;
    }
}

/*
 * call-seq:
 *   save(options={})               =>      camera
 *
 * Downloads file from camera to hard drive.
 * Available options are:
 * * :file - Name of the file to download from camera. File is expected
 *   to be found in current path. If this option is not specified, last
 *   captured image is downloaded.
 * * :new_name - New file name to be used when saving file on hard drive.
 *   If this option is not specified, camera file system filename is used.
 * * :to_folder - Folder path on hard drive to save downloaded image to.
 * * :type - Type of file to download from camera. Available types are 
 *   <b>:normal</b> (default) and <b>:preview</b>
 *
 * Examples:
 *
 *   c = GPhoto2::Camera.new
 *   c.capture.save :type => :preview,              => Downloads preview of
 *               :new_name => "PREVIEW.JPG"            captured image
 *   c.save :file => "DSC_0144.JPG",                => Downloads specified file
 *               :to_folder => "/home/user",           to /home/user/xyz.gf.JPG
 *               :new_name => "xyz.gf",
 *
 */
static VALUE camera_save(int argc, VALUE *argv, VALUE self) {
    int retval, i;
    int newName = -1;
    CameraFileType fileType = GP_FILE_TYPE_NORMAL;
    GPhoto2Camera *c;
    const char *fData, *key, *val;
    char *fPath, *newNameStr, *pchNew, *pchSrc;
    char fName[100], cFileName[100], cFolderName[100];
    unsigned long int fSize;
    int fd;
    VALUE arr, hVal;
    
    Data_Get_Struct(self, GPhoto2Camera, c);
    
    strcpy(fName, "");
    strcpy(cFileName, c->path.name);
    strcpy(cFolderName, c->path.folder);

    switch(argc) {
        case 0:
            break;
        case 1:
            Check_Type(argv[0], T_HASH);
            arr = rb_funcall(argv[0], rb_intern("keys"), 0);
            for (i = 0; i < RARRAY(arr)->len; i++) {
                switch(TYPE(RARRAY(arr)->ptr[i])) {
                    case T_STRING:
                        key = RSTRING(RARRAY(arr)->ptr[i])->ptr;
                        break;
                    case T_SYMBOL:
                        key = rb_id2name(rb_to_id(RARRAY(arr)->ptr[i]));
                        break;
                    default:
                        rb_raise(rb_eTypeError, "Not valid key type");
                        return Qnil;
                }
                if (strcmp(key, "to_folder") == 0) {
                    fPath = RSTRING(rb_hash_aref(argv[0], RARRAY(arr)->ptr[i]))->ptr;
                    if (strlen(fPath) > 0) {
                        if (fPath[strlen(fPath)] == '/') {
                            strcpy(fName, fPath);
                        } else {
                            strcpy(fName, fPath);
                            strcat(fName, "/");
                        }
                    } 
                } else if (strcmp(key, "new_name") == 0) {
                    newNameStr = RSTRING(rb_hash_aref(argv[0], RARRAY(arr)->ptr[i]))->ptr;
                    if (strlen(newNameStr) > 0) {
                        newName = 0;
                    }
                } else if (strcmp(key, "type") == 0) {
                    hVal = rb_hash_aref(argv[0], RARRAY(arr)->ptr[i]);
                    switch(TYPE(hVal)) {
                        case T_STRING:
                            val = RSTRING(hVal)->ptr;
                            break;
                        case T_SYMBOL:
                            val = rb_id2name(rb_to_id(hVal));
                            break;
                        default:
                            rb_raise(rb_eTypeError, "Not valid value type");
                            return Qnil;
                    }
                    if (strcmp(val, "normal") == 0) {
                        fileType = GP_FILE_TYPE_NORMAL;
                    } else if (strcmp(val, "preview") == 0) {
                        fileType = GP_FILE_TYPE_PREVIEW;
                    }
                } else if (strcmp(key, "file") == 0) {
                    strcpy(cFolderName, c->virtFolder);
                    strcpy(cFileName, RSTRING(rb_hash_aref(argv[0], RARRAY(arr)->ptr[i]))->ptr);
                }
            }
            break;
        default:
            rb_raise(rb_eArgError, "Wrong number of arguments (%d for 0 or 1)", argc);
            return Qnil;
    }
    
    retval = gp_filesystem_reset(c->camera->fs);
    if (retval == GP_OK) {
        retval = gp_camera_file_get(c->camera, cFolderName, cFileName, fileType, c->file, c->context);
        if (retval == GP_OK) {
            retval = gp_file_get_data_and_size(c->file, &fData, &fSize);
            if (retval == GP_OK) {
                if (newName == 0)  {
                    strcat(fName, newNameStr);
                    pchNew = strrchr(newNameStr, '.');
                    pchSrc = strrchr(cFileName, '.');
                    if (pchNew == NULL) {
                        strcat(fName, pchSrc);
                    } else if (strcmp(pchNew, pchSrc) != 0) {
                        strcat(fName, pchSrc);
                    }
                } else {
                    strcat(fName, cFileName);
                }
                fd = open(fName, O_CREAT | O_WRONLY, 0644);
                write(fd, fData, fSize);
                close(fd);
                return self;
            }
        }
    }
    rb_raise_gp_result(retval);
    return Qnil;
}

/*
 * call-seq:
 *   delete(options={})             =>      camera
 *
 * Deletes file from camera.
 * Available options are:
 * * :file - Name of the file to delete from camera. File is expected
 *   to be found in current path. If this option is not specified, last
 *   captured image is deleted.
 *
 * Examples:
 *
 *   c = GPhoto2::Camera.new
 *   c.capture.save.delete
 *   c.delete :file => "DSC_0144.JPG"
 *
 */
static VALUE camera_delete(int argc, VALUE *argv, VALUE self) {
    int retval, i;
    GPhoto2Camera *c;
    const char *key;
    char cFileName[100], cFolderName[100];
    VALUE arr;
    
    Data_Get_Struct(self, GPhoto2Camera, c);
    
    strcpy(cFileName, c->path.name);
    strcpy(cFolderName, c->path.folder);

    switch(argc) {
        case 0:
            break;
        case 1:
            Check_Type(argv[0], T_HASH);
            arr = rb_funcall(argv[0], rb_intern("keys"), 0);
            for (i = 0; i < RARRAY(arr)->len; i++) {
                switch(TYPE(RARRAY(arr)->ptr[i])) {
                    case T_STRING:
                        key = RSTRING(RARRAY(arr)->ptr[i])->ptr;
                        break;
                    case T_SYMBOL:
                        key = rb_id2name(rb_to_id(RARRAY(arr)->ptr[i]));
                        break;
                    default:
                        rb_raise(rb_eTypeError, "Not valid key type");
                        return Qnil;
                }
                if (strcmp(key, "file") == 0) {
                    strcpy(cFolderName, c->virtFolder);
                    strcpy(cFileName, RSTRING(rb_hash_aref(argv[0], RARRAY(arr)->ptr[i]))->ptr);
                }
            }
            break;
        default:
            rb_raise(rb_eArgError, "Wrong number of arguments (%d for 0 or 1)", argc);
            return Qnil;
    }
    
    retval = gp_filesystem_reset(c->camera->fs);
    if (retval == GP_OK) {
        retval = gp_camera_file_delete(c->camera, cFolderName, cFileName, c->context);
        if (retval == GP_OK) {
            return self;
        }
    }
    rb_raise_gp_result(retval);
    return Qnil;
}

/*
 * call-seq:
 *   configs                        =>      array
 *
 * Returns an array of adjustable camera configurations.
 *
 * Examples:
 *
 *   c = GPhoto2::Camera.new
 *   # with Nikon DSC D80
 *   c.configs                      #=>     ["capturetarget", "imgquality",
 *                                           "imgsize", "whitebalance",
 *                                           "f-number", "focallength",
 *                                           "focusmode", "iso",
 *                                           "exposurebiascompensation",
 *                                           "exptime", "expprogram",
 *                                           "capturemode", "focusmetermode",
 *                                           "exposuremetermode", "flashmode",
 *                                           "burstnumber", "accessmode",
 *                                           "channel", "encryption"]
 *
 */
static VALUE camera_get_configs(VALUE self) {
    GPhoto2Camera *c;
    VALUE arr = rb_ary_new();

    Data_Get_Struct(self, GPhoto2Camera, c);

    populateWithConfigs(c->config, arr);
    
    return arr;
}

/*
 * call-seq:
 *   cam[cfg]                       =>      float or string
 *   cam[cfg, :all]                 =>      array
 *
 * Returns current value of specified camera configuration. Configuration name
 * (cfg) can be string or symbol and must be in configs method returned array.
 * When called with directive <b>:all</b> returns an array of allowed values
 *
 * Examples:
 *
 *   c = GPhoto2::Camera.new
 *   # with Nikon DSC D80
 *   c["f-number"]                  #=>     "f/4.5"
 *   c[:focallength]                #=>     10.5
 *   c[:focusmode, :all]            #=>     ["Manual", "AF-S", "AF-C", "AF-A"]
 *
 */
static VALUE camera_get_value(int argc, VALUE *argv, VALUE self) {
    int retval;
    const char *name;
    GPhoto2Camera *c;
    CameraWidgetType widgettype;
    VALUE str, dir;
    
    switch (argc) {
        case 1:
            str = argv[0];
            break;
        case 2:
            str = argv[0];
            dir = argv[1];
            break;
        default:
            rb_raise(rb_eArgError, "Wrong number of arguments (%d for 1 or 2)", argc);
            return Qnil;
    }
    
    switch (TYPE(str)) {
        case T_STRING:
            name = RSTRING(str)->ptr;
            break;
        case T_SYMBOL:
            name = rb_id2name(rb_to_id(str));
            break;
        default:
            rb_raise(rb_eTypeError, "Not valid parameter type");
            return Qnil;
    }
    
    Data_Get_Struct(self, GPhoto2Camera, c);

    retval = gp_widget_get_child_by_name(c->config, name, &(c->childConfig));
    if (retval == GP_OK) {
        retval = gp_widget_get_type(c->childConfig, &widgettype);
        if (retval == GP_OK) {
            switch (widgettype) {
                case GP_WIDGET_RADIO:
                    if (argc == 1) {
                        return getRadio(c->childConfig);
                    } else if (strcmp(rb_id2name(rb_to_id(dir)), "all") == 0) {
                        return listRadio(c->childConfig);
                    } else {
                        rb_raise(rb_cGPhoto2ConfigurationError, "Second parameter not valid");
                        return Qnil;
                    }
                    break;
                case GP_WIDGET_RANGE:
                    if (argc == 1) {
                        return getRange(c->childConfig);
                    } else if (strcmp(rb_id2name(rb_to_id(dir)), "all") == 0) {
                        return listRange(c->childConfig);
                    } else {
                        rb_raise(rb_cGPhoto2ConfigurationError, "Second parameter not valid");
                        return Qnil;
                    }
                    break;
                default:
                    rb_raise(rb_cGPhoto2ConfigurationError, "Not supported yet");
                    return Qnil;
            }
        }
    }
    if (retval != GP_OK) {
        rb_raise_gp_result(retval);
    } else {
        rb_raise_programmer_error("camera_get_value");
    }
    return Qnil;
}

/*
 * call-seq:
 *   cam[cfg] = value               =>      float or string
 *
 * Sets specified camera configuration to specified value if value is allowed.
 *
 * Examples:
 *
 *   c = GPhoto2::Camera.new
 *   # with Nikon DSC D80
 *   c["f-number"] = "f/4.5"        #=>     "f/4.5"
 *   c[:focallength] = 10.5         #=>     10.5
 *
 */
static VALUE camera_set_value(VALUE self, VALUE str, VALUE newVal) {
    int retval;
    const char *name;
    GPhoto2Camera *c;
    CameraWidgetType widgettype;
    
    switch (TYPE(str)) {
        case T_STRING:
            name = RSTRING(str)->ptr;
            break;
        case T_SYMBOL:
            name = rb_id2name(rb_to_id(str));
            break;
        default:
            rb_raise(rb_eTypeError, "Not valid parameter type");
            return Qnil;
    }
    
    Data_Get_Struct(self, GPhoto2Camera, c);

    retval = gp_widget_get_child_by_name(c->config, name, &(c->childConfig));
    if (retval == GP_OK) {
        retval = gp_widget_get_type(c->childConfig, &widgettype);
        if (retval == GP_OK) {
            switch (widgettype) {
                case GP_WIDGET_RADIO:
                    return setRadio(c, newVal);
                    break;
                case GP_WIDGET_RANGE:
                    return setRange(c, newVal);
                    break;
                default:
                    rb_raise(rb_cGPhoto2ConfigurationError, "Cannot access this setting");
                    return Qnil;
            }
        }
    }
    if (retval != GP_OK) {
        rb_raise_gp_result(retval);
    } else {
        rb_raise_programmer_error("camera_set_value");
    }
    return Qnil;
}

/*
 * call-seq:
 *   folder                         =>      string
 *
 * Returns current camera path. When image is captured, folder changes to
 * path where files are saved on camera.
 *
 * Examples:
 *
 *   c = GPhoto2::Camera.new
 *   # with Nikon DSC D80
 *   c.folder                       #=>     "/"
 *   c.capture
 *   c.folder                       #=>     "/store_00010001/DCIM/100NCD80"
 *
 */
static VALUE camera_folder(VALUE self) {
    GPhoto2Camera *c;
    
    Data_Get_Struct(self, GPhoto2Camera, c);
    
    return rb_str_new2(c->virtFolder);
}

/*
 * call-seq:
 *   subfolders                     =>      array
 *
 * Returns an array of subfolder names in current camera path.
 *
 * Examples:
 *
 *   c = GPhoto2::Camera.new
 *   # with Nikon DSC D80
 *   c.folder                       #=>     "/"
 *   c.subfolders                   #=>     ["special", "store_00010001"]
 *
 */
static VALUE camera_subfolders(VALUE self) {
    int retval, i, count;
    const char *name;
    GPhoto2Camera *c;
    VALUE arr;
    
    Data_Get_Struct(self, GPhoto2Camera, c);
    
    retval = gp_camera_folder_list_folders(c->camera, c->virtFolder, c->list, c->context);
    if (retval == GP_OK) {
        count = gp_list_count(c->list);
        if (count < 0) {
            rb_raise_gp_result(retval);
            return Qnil;
        }
        arr = rb_ary_new();
        for (i = 0; i < count; i++) {
            retval = gp_list_get_name(c->list, i, &name);
            if (retval == GP_OK) {
                rb_ary_push(arr, rb_str_new2(name));
            }
        }
        return arr;
    }
    rb_raise_gp_result(retval);
    return Qnil;
}

/*
 * call-seq:
 *   files                          =>      array
 *
 * Returns an array of file names in current camera path.
 *
 * Examples:
 *
 *   c = GPhoto2::Camera.new
 *   # with Nikon DSC D80
 *   c.folder                       #=>     "/"
 *   c.files                        #=>     []
 *   c.capture
 *   c.files                        #=>     ["DSC_0001.JPG", "DSC_0002.JPG",
 *                                           "DSC_0003.JPG", ... ]
 *
 */
static VALUE camera_files(VALUE self) {
    int retval, i, count;
    const char *name;
    GPhoto2Camera *c;
    VALUE arr;
    
    Data_Get_Struct(self, GPhoto2Camera, c);
    
    retval = gp_filesystem_reset(c->camera->fs);
    if (retval == GP_OK) {
        retval = gp_camera_folder_list_files(c->camera, c->virtFolder, c->list, c->context);
        if (retval == GP_OK) {
            count = gp_list_count(c->list);
            if (count < 0) {
                rb_raise_gp_result(retval);
                return Qnil;
            }
            arr = rb_ary_new();
            for (i = 0; i < count; i++) {
                retval = gp_list_get_name(c->list, i, &name);
                if (retval == GP_OK) {
                    rb_ary_push(arr, rb_str_new2(name));
                }
            }
            return arr;
        }
    }
    rb_raise_gp_result(retval);
    return Qnil;
}

/*
 * call-seq:
 *   folder_up                      =>      camera
 *
 * Changes current camera path one level up.
 *
 * Examples:
 *
 *   c = GPhoto2::Camera.new
 *   # with Nikon DSC D80
 *   c.folder                       #=>     "/"
 *   c.capture
 *   c.folder                       #=>     "/store_00010001/DCIM/100NCD80"
 *   c.folder_up
 *   c.folder                       #=>     "/store_00010001/DCIM"
 *   c.folder_up.folder_up          #=>     "/"
 *
 */
static VALUE camera_folder_up(VALUE self) {
    char *pch;
    GPhoto2Camera *c;
    
    Data_Get_Struct(self, GPhoto2Camera, c);
    
    pch = strrchr(c->virtFolder, '/');
    if ((pch - c->virtFolder) == 0) {
        c->virtFolder[1] = '\0';
    } else {
        c->virtFolder[pch - c->virtFolder] = '\0';
    }
    
    return self;
}

/*
 * call-seq:
 *   folder_down(name)              =>      camera
 *
 * Changes current camera path one level down into subfolder with
 * specified name.
 *
 * Examples:
 *
 *   c = GPhoto2::Camera.new
 *   # with Nikon DSC D80
 *   c.folder                       #=>     "/"
 *   c.folder_down "store_00010001"
 *   c.folder                       #=>     "/store_00010001"
 *   c.folder_down("DCIM").folder_down("100NCD80")
 *   c.folder                       #=>     "/store_00010001/DCIM/100NCD80"
 *
 */
static VALUE camera_folder_down(VALUE self, VALUE folder) {
    Check_Type(folder, T_STRING);
    int retval;
    const char *name;
    int index;
    GPhoto2Camera *c;
    
    Data_Get_Struct(self, GPhoto2Camera, c);
    
    name = RSTRING(folder)->ptr;
    retval = gp_camera_folder_list_folders(c->camera, c->virtFolder, c->list, c->context);
    if (retval == GP_OK) {
        retval = gp_list_find_by_name(c->list, &index, name);
        if (retval == GP_OK) {
            if (strlen(c->virtFolder) > 1) {
                strcat(c->virtFolder, "/");
            }
            strcat(c->virtFolder, name);
        }
    }
    return self;
}

void Init_gphoto4ruby() {
    /*
     * Module contains camera class definition and some exceptions
     */
    rb_mGPhoto2 = rb_define_module("GPhoto2");
    /*
     * GPhoto2::Camera object is a Ruby wrapping aroung gphoto2 C library.
     */
    rb_cGPhoto2Camera = rb_define_class_under(rb_mGPhoto2, "Camera", rb_cObject);
    /*
     * GPhoto2::Exception is raised when libgphoto2 functions from C core
     * return any error code
     */
    rb_cGPhoto2Exception = rb_define_class_under(rb_mGPhoto2, "Exception", rb_eStandardError);
    /*
     * GPhoto2::ConfigurationError is raised when trying to set configuration
     * values that are not allowed or trying to access properties that are not
     * supported
     */
    rb_cGPhoto2ConfigurationError = rb_define_class_under(rb_mGPhoto2, "ConfigurationError", rb_eStandardError);
    /*
     * GPhoto2::ProgrammerError is never raised. :) Only when program gets
     * where it could never get.
     */
    rb_cGPhoto2ProgrammerError = rb_define_class_under(rb_mGPhoto2, "ProgrammerError", rb_eStandardError);
    rb_define_alloc_func(rb_cGPhoto2Camera, camera_allocate);
    rb_define_module_function(rb_cGPhoto2Camera, "ports", camera_class_ports, 0);
    rb_define_method(rb_cGPhoto2Camera, "initialize", camera_initialize, -1);
    rb_define_method(rb_cGPhoto2Camera, "configs", camera_get_configs, 0);
    rb_define_method(rb_cGPhoto2Camera, "[]", camera_get_value, -1);
    rb_define_method(rb_cGPhoto2Camera, "[]=", camera_set_value, 2);
    rb_define_method(rb_cGPhoto2Camera, "capture", camera_capture, 0);
    rb_define_method(rb_cGPhoto2Camera, "save", camera_save, -1);
    rb_define_method(rb_cGPhoto2Camera, "delete", camera_delete, -1);
    rb_define_method(rb_cGPhoto2Camera, "folder", camera_folder, 0);
    rb_define_method(rb_cGPhoto2Camera, "subfolders", camera_subfolders, 0);
    rb_define_method(rb_cGPhoto2Camera, "files", camera_files, 0);
    rb_define_method(rb_cGPhoto2Camera, "folder_up", camera_folder_up, 0);
    rb_define_method(rb_cGPhoto2Camera, "folder_down", camera_folder_down, 1);
}

