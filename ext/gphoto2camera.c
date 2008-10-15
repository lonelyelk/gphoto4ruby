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

#include "gphoto2camera.h"

void camera_mark(GPhoto2Camera *c) {
}

void camera_free(GPhoto2Camera *c) {
    gp_result_check(gp_camera_exit(c->camera, c->context));
    gp_result_check(gp_widget_free(c->config));
    gp_result_check(gp_list_free(c->list));
    gp_result_check(gp_file_free(c->file));
    gp_result_check(gp_camera_free(c->camera));
    free(c->path);
    free(c->virtFolder);
    free(c->context);
    free(c);
}

VALUE camera_allocate(VALUE klass) {
    GPhoto2Camera *c;
    c = (GPhoto2Camera*) malloc(sizeof(GPhoto2Camera));
    c->virtFolder = (char*) malloc(sizeof(char)*1024);
    c->path = (CameraFilePath*) malloc(sizeof(CameraFilePath));
    strcpy(c->virtFolder, "/");
    c->context = gp_context_new();
    gp_result_check(gp_camera_new(&(c->camera)));
    gp_result_check(gp_camera_get_config(c->camera, &(c->config), c->context));
    gp_result_check(gp_list_new(&(c->list)));
    gp_result_check(gp_file_new(&(c->file)));
    return Data_Wrap_Struct(klass, camera_mark, camera_free, c);
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
VALUE camera_initialize(int argc, VALUE *argv, VALUE self) {
    GPhoto2Camera *c;
    VALUE cfgs;

    Data_Get_Struct(self, GPhoto2Camera, c);

    switch (argc) {
        case 0:
            break;
        case 1:
            Check_Type(argv[0], T_STRING);
            int portIndex;
            GPPortInfoList *portInfoList;
            GPPortInfo p;
            
            gp_result_check(gp_port_info_list_new(&portInfoList));
            gp_result_check(gp_port_info_list_load(portInfoList));
            portIndex = gp_result_check(gp_port_info_list_lookup_path(portInfoList, RSTRING(argv[0])->ptr));
            gp_result_check(gp_port_info_list_get_info(portInfoList, portIndex, &p));
            gp_result_check(gp_camera_set_port_info(c->camera, p));
            break;
        default:
            rb_raise(rb_eArgError, "Wrong number of arguments (%d for 0 or 1)", argc);
            return Qnil;
    }
    
    cfgs = rb_hash_new();
    populateWithConfigs(c->config, cfgs);
    rb_iv_set(self, "@configuration", cfgs);
    rb_iv_set(self, "@configs_changed", rb_ary_new());
    
    return self;
}

/*
 * call-seq:
 *   GPhoto2::Camera.ports          =>      array
 *
 * Returns an array of usb port paths with cameras. Port paths are the same
 * as in <b>gphoto2 --auto-detect</b> output. Assuming that if there are
 * cameras detected with long port paths, then the one with short port path
 * is a duplicate of one of the others.
 *
 * Examples:
 *
 *   # with one camera connected
 *   GPhoto2::Camera.ports          #=>     ["usb:"]
 *   # with two cameras connected
 *   GPhoto2::Camera.ports          #=>     ["usb:005,004", "usb:005,006"]
 *
 */
VALUE camera_class_ports(VALUE klass) {
    int i, camsTotal, retVal;
    GPContext *context;
    CameraAbilitiesList *abilList;
    GPPortInfoList *portInfoList;
    CameraList *camList;
    const char *pName = NULL;
    VALUE arr = rb_ary_new();

    context = gp_context_new();
    retVal = gp_port_info_list_new(&portInfoList);
    if (retVal == GP_OK) {
        retVal = gp_port_info_list_load(portInfoList);
        if (retVal == GP_OK) {
            retVal = gp_abilities_list_new(&abilList);
            if (retVal == GP_OK) {
                retVal = gp_abilities_list_load(abilList, context);
                if (retVal == GP_OK) {
                    retVal = gp_list_new(&camList);
                    if (retVal == GP_OK) {
                        retVal = gp_abilities_list_detect(abilList, portInfoList, camList, context);
                        if (retVal == GP_OK) {
                            retVal = gp_list_count(camList);
                            if (retVal >= GP_OK) {
                                camsTotal = retVal;
                                for(i = 0; i < camsTotal; i++) {
                                    retVal = gp_list_get_value(camList, i, &pName);
                                    if ((retVal == GP_OK) &&
                                      ((camsTotal == 1) || (strlen(pName) > 4))) {
                                        rb_ary_push(arr, rb_str_new2(pName));
                                    }
                                }
                            }
                        }
                        gp_list_free(camList);
                    }
                }
                gp_abilities_list_free(abilList);
            }
        }
        gp_port_info_list_free(portInfoList);
    }
    free(context);
    if (retVal < GP_OK) {
        rb_raise_gp_result(retVal);
    }
    return arr;
}

/*
 * call-seq:
 *   capture(config={})             =>      camera
 *
 * Sends command to camera to capture image with current or provided
 * configuration. Provided configuration is kept after capture.
 *
 * Examples:
 *
 *   c = GPhoto2::Camera.new
 *   c.capture
 *   c.capture "exptime" => "0.010", "iso" => "400"
 *
 */
VALUE camera_capture(int argc, VALUE *argv, VALUE self) {
    GPhoto2Camera *c;

    Data_Get_Struct(self, GPhoto2Camera, c);
    
    if (argc == 1) {
        camera_config_merge(self, argv[0]);
    } else if (argc != 0) {
        rb_raise(rb_eArgError, "Wrong number of arguments (%d for 0 or 1)", argc);
        return Qnil;
    }

    gp_result_check(gp_camera_capture(c->camera, GP_CAPTURE_IMAGE, c->path, c->context));
    printf("captured: %s/%s\n", c->path->folder, c->path->name);
    strcpy(c->virtFolder, c->path->folder);
    return self;
}

/*
 * call-seq:
 *   save(options={})               =>      camera
 *
 * Downloads file from camera to hard drive.
 * Available options are:
 * * :file - Name of the file to download from camera. File is expected
 *   to be found in current path. If this option is not specified, last
 *   captured image is downloaded. If symbols <b>:first</b> or <b>:last</b>
 *   are passed, the first or the last file from current camera path is
 *   downloaded.
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
VALUE camera_save(int argc, VALUE *argv, VALUE self) {
    int i, count;
    int newName = 0;
    CameraFileType fileType = GP_FILE_TYPE_NORMAL;
    GPhoto2Camera *c;
    const char *fData, *key, *val, *name;
    char *fPath, *newNameStr, *pchNew, *pchSrc;
    char fName[100], cFileName[100], cFolderName[100];
    unsigned long int fSize;
    int fd;
    VALUE arr, hVal;
    
    Data_Get_Struct(self, GPhoto2Camera, c);
    
    strcpy(fName, "");
    strcpy(cFileName, c->path->name);
    strcpy(cFolderName, c->path->folder);

    gp_result_check(gp_filesystem_reset(c->camera->fs));

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
                        newName = 1;
                    }
                } else if (strcmp(key, "type") == 0) {
                    hVal = rb_hash_aref(argv[0], RARRAY(arr)->ptr[i]);
                    Check_Type(hVal, T_SYMBOL);
                    val = rb_id2name(rb_to_id(hVal));
                    if (strcmp(val, "normal") == 0) {
                        fileType = GP_FILE_TYPE_NORMAL;
                    } else if (strcmp(val, "preview") == 0) {
                        fileType = GP_FILE_TYPE_PREVIEW;
                    }
                } else if (strcmp(key, "file") == 0) {
                    hVal = rb_hash_aref(argv[0], RARRAY(arr)->ptr[i]);
                    switch(TYPE(hVal)) {
                        case T_STRING:
                            strcpy(cFolderName, c->virtFolder);
                            strcpy(cFileName, RSTRING(hVal)->ptr);
                            break;
                        case T_SYMBOL:
                            val = rb_id2name(rb_to_id(hVal));
                            gp_result_check(gp_camera_folder_list_files(c->camera, c->virtFolder, c->list, c->context));
                            if (strcmp(val, "first") == 0) {
                                count = 0;
                            } else if (strcmp(val, "last") == 0) {
                                count = gp_result_check(gp_list_count(c->list)) - 1;
                            } else {
                                count = -1;
                            }
                            gp_result_check(gp_list_get_name(c->list, count, &name));
                            strcpy(cFileName, name);
                            break;
                        default:
                            rb_raise(rb_eTypeError, "Not valid value type");
                            return Qnil;
                    }
                }
            }
            break;
        default:
            rb_raise(rb_eArgError, "Wrong number of arguments (%d for 0 or 1)", argc);
            return Qnil;
    }
    
    gp_result_check(gp_camera_file_get(c->camera, cFolderName, cFileName, fileType, c->file, c->context));
    gp_result_check(gp_file_get_data_and_size(c->file, &fData, &fSize));
    if (newName == 1)  {
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
 *   c.delete :all
 *
 */
VALUE camera_delete(int argc, VALUE *argv, VALUE self) {
    int i;
    int one = 1;
    GPhoto2Camera *c;
    const char *key;
    char cFileName[100], cFolderName[100];
    VALUE arr;
    
    Data_Get_Struct(self, GPhoto2Camera, c);
    
    strcpy(cFileName, c->path->name);
    strcpy(cFolderName, c->path->folder);

    switch(argc) {
        case 0:
            break;
        case 1:
            switch(TYPE(argv[0])) {
                case T_HASH:
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
                case T_SYMBOL:
                    key = rb_id2name(rb_to_id(argv[0]));
                    if (strcmp(key, "all") == 0) {
                        strcpy(cFolderName, c->virtFolder);
                        one = 0;
                    }
                    break;
            }
            break;
        default:
            rb_raise(rb_eArgError, "Wrong number of arguments (%d for 0 or 1)", argc);
            return Qnil;
    }
    
    gp_result_check(gp_filesystem_reset(c->camera->fs));
    if (one == 1) {
        gp_result_check(gp_camera_file_delete(c->camera, cFolderName, cFileName, c->context));
    } else {
        gp_result_check(gp_camera_folder_delete_all(c->camera, cFolderName, c->context));
    }
    return self;
}

/*
 * call-seq:
 *   config                         =>      hash
 *   config :no_cache               =>      hash
 *
 * Returns cached hash of adjustable camera configuration with their values.
 * Can be run with directive <b>:no_cache</b>
 *
 * Examples:
 *
 *   c = GPhoto2::Camera.new
 *   # with Nikon DSC D80
 *   c.config.keys                  #=>     ["capturetarget", "imgquality",
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
VALUE camera_get_config(int argc, VALUE *argv, VALUE self) {
    int i;
    const char *key;
    GPhoto2Camera *c;
    CameraWidgetType widgettype;
    VALUE arr, cfg;

    cfg = rb_iv_get(self, "@configuration");

    switch (argc) {
        case 1:
            Check_Type(argv[0], T_SYMBOL);
            Data_Get_Struct(self, GPhoto2Camera, c);
            
            if (strcmp(rb_id2name(rb_to_id(argv[0])), "no_cache") == 0) {
                gp_result_check(gp_camera_get_config(c->camera, &(c->config), c->context));
                arr = rb_funcall(cfg, rb_intern("keys"), 0);
                for (i = 0; i < RARRAY(arr)->len; i++) {
                    key = RSTRING(RARRAY(arr)->ptr[i])->ptr;
                    gp_result_check(gp_widget_get_child_by_name(c->config, key, &(c->childConfig)));
                    gp_result_check(gp_widget_get_type(c->childConfig, &widgettype));
                    switch (widgettype) {
                        case GP_WIDGET_RADIO:
                            rb_hash_aset(cfg, RARRAY(arr)->ptr[i], getRadio(c->childConfig));
                            break;
                        case GP_WIDGET_TEXT:
                            rb_hash_aset(cfg, RARRAY(arr)->ptr[i], getText(c->childConfig));
                            break;
                        case GP_WIDGET_RANGE:
                            rb_hash_aset(cfg, RARRAY(arr)->ptr[i], getRange(c->childConfig));
                            break;
                        case GP_WIDGET_TOGGLE:
                            rb_hash_aset(cfg, RARRAY(arr)->ptr[i], getToggle(c->childConfig));
                            break;
                        default:
                            break;
                    }
                }
            } else {
                rb_raise(rb_cGPhoto2ConfigurationError, "Unknown directive '%s'", rb_id2name(rb_to_id(argv[0])));
                return Qnil;
            }
        case 0:
            return cfg;
        default:
            rb_raise(rb_eArgError, "Wrong number of arguments (%d for 0 or 1)", argc);
            return Qnil;
    }
}

/*
 * call-seq:
 *   config_merge(hash)             =>      hash
 *
 * Adjusts camera configuration with given values.
 *
 * Examples:
 *
 *   c = GPhoto2::Camera.new
 *   # with Nikon DSC D80
 *   c.config_merge "f-number" => "f/4", "exptime" => "0.010", "iso" => "200"
 *
 */
VALUE camera_config_merge(VALUE self, VALUE hash) {
    Check_Type(hash, T_HASH);

    int i;
    const char *key;
    GPhoto2Camera *c;
    CameraWidgetType widgettype;
    VALUE arr, cfgs, cfg_changed;

    Data_Get_Struct(self, GPhoto2Camera, c);

    arr = rb_funcall(hash, rb_intern("keys"), 0);
    cfgs = rb_iv_get(self, "@configuration");
    cfg_changed = rb_iv_get(self, "@configs_changed");
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
        if (TYPE(rb_funcall(cfgs, rb_intern("has_key?"), 1, rb_str_new2(key))) == T_TRUE) {
            gp_result_check(gp_widget_get_child_by_name(c->config, key, &(c->childConfig)));
            gp_result_check(gp_widget_get_type(c->childConfig, &widgettype));
            switch (widgettype) {
                case GP_WIDGET_RADIO:
                    rb_ary_push(cfg_changed, rb_str_new2(key));
                    setRadio(self, c, rb_hash_aref(hash, RARRAY(arr)->ptr[i]), 0);
                    break;
                case GP_WIDGET_TEXT:
                    rb_ary_push(cfg_changed, rb_str_new2(key));
                    setText(self, c, rb_hash_aref(hash, RARRAY(arr)->ptr[i]), 0);
                    break;
                case GP_WIDGET_RANGE:
                    rb_ary_push(cfg_changed, rb_str_new2(key));
                    setRange(self, c, rb_hash_aref(hash, RARRAY(arr)->ptr[i]), 0);
                    break;
                case GP_WIDGET_TOGGLE:
                    rb_ary_push(cfg_changed, rb_str_new2(key));
                    setToggle(self, c, rb_hash_aref(hash, RARRAY(arr)->ptr[i]), 0);
                    break;
                default:
                    break;
            }
        }
    }
    saveConfigs(self, c);
    return cfgs;
}

/*
 * call-seq:
 *   cam[cfg]                       =>      float or string
 *   cam[cfg, :all]                 =>      array
 *   cam[cfg, :type]                =>      fixnum
 *
 * Returns current value of specified camera configuration. Configuration name
 * (cfg) can be string or symbol and must be in configs method returned array.
 * Configuration is cached in @configuration instance variable.
 * 
 * Possible directives:
 * * <b>:no_cache</b> doesn't use cached configuration value
 * * <b>:all</b> returns an array of allowed values;
 * * <b>:type</b> returns one of available CONFIG_TYPE constats
 *
 * Examples:
 *
 *   c = GPhoto2::Camera.new
 *   # with Nikon DSC D80
 *   c["f-number"]                  #=>     "f/4.5"
 *   c[:focallength]                #=>     10.5
 *   c[:focusmode, :all]            #=>     ["Manual", "AF-S", "AF-C", "AF-A"]
 *   c[:exptime, :type] == GPhoto2::Camera::CONFIG_TYPE_RADIO
 *                                  #=>     true
 *
 */
VALUE camera_get_value(int argc, VALUE *argv, VALUE self) {
    const char *name;
    GPhoto2Camera *c;
    CameraWidgetType widgettype;
    VALUE str, dir;
    VALUE val;

    str = argv[0];
    
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

    switch (argc) {
        case 1:
            return rb_hash_aref(rb_iv_get(self, "@configuration"), rb_str_new2(name));
            break;
        case 2:
            dir = argv[1];
            Check_Type(dir, T_SYMBOL);
            Data_Get_Struct(self, GPhoto2Camera, c);
            
            if (strcmp(rb_id2name(rb_to_id(dir)), "no_cache") == 0) {
                gp_result_check(gp_camera_get_config(c->camera, &(c->config), c->context));
            }

            gp_result_check(gp_widget_get_child_by_name(c->config, name, &(c->childConfig)));
            gp_result_check(gp_widget_get_type(c->childConfig, &widgettype));
            switch (widgettype) {
                case GP_WIDGET_RADIO:
                    if (strcmp(rb_id2name(rb_to_id(dir)), "no_cache") == 0) {
                        val = getRadio(c->childConfig);
                        rb_hash_aset(rb_iv_get(self, "@configuration"), rb_str_new2(name), val);
                        return val;
                    } else if (strcmp(rb_id2name(rb_to_id(dir)), "all") == 0) {
                        return listRadio(c->childConfig);
                    } else if (strcmp(rb_id2name(rb_to_id(dir)), "type") == 0) {
                        return INT2FIX(GP_WIDGET_RADIO);
                    } else {
                        rb_raise(rb_cGPhoto2ConfigurationError, "Unknown directive '%s'", rb_id2name(rb_to_id(dir)));
                        return Qnil;
                    }
                    break;
                case GP_WIDGET_TEXT:
                    if (strcmp(rb_id2name(rb_to_id(dir)), "no_cache") == 0) {
                        val = getText(c->childConfig);
                        rb_hash_aset(rb_iv_get(self, "@configuration"), rb_str_new2(name), val);
                        return val;
                    } else if (strcmp(rb_id2name(rb_to_id(dir)), "all") == 0) {
                        return rb_ary_new();
                    } else if (strcmp(rb_id2name(rb_to_id(dir)), "type") == 0) {
                        return INT2FIX(GP_WIDGET_TEXT);
                    } else {
                        rb_raise(rb_cGPhoto2ConfigurationError, "Unknown directive '%s'", rb_id2name(rb_to_id(dir)));
                        return Qnil;
                    }
                    break;
                case GP_WIDGET_RANGE:
                    if (strcmp(rb_id2name(rb_to_id(dir)), "no_cache") == 0) {
                        val = getRange(c->childConfig);
                        rb_hash_aset(rb_iv_get(self, "@configuration"), rb_str_new2(name), val);
                        return val;
                    } else if (strcmp(rb_id2name(rb_to_id(dir)), "all") == 0) {
                        return listRange(c->childConfig);
                    } else if (strcmp(rb_id2name(rb_to_id(dir)), "type") == 0) {
                        return INT2FIX(GP_WIDGET_RANGE);
                    } else {
                        rb_raise(rb_cGPhoto2ConfigurationError, "Unknown directive '%s'", rb_id2name(rb_to_id(dir)));
                        return Qnil;
                    }
                    break;
                case GP_WIDGET_TOGGLE:
                    if (strcmp(rb_id2name(rb_to_id(dir)), "no_cache") == 0) {
                        val = getToggle(c->childConfig);
                        rb_hash_aset(rb_iv_get(self, "@configuration"), rb_str_new2(name), val);
                        return val;
                    } else if (strcmp(rb_id2name(rb_to_id(dir)), "all") == 0) {
                        VALUE arr = rb_ary_new();
                        rb_ary_push(arr, Qtrue);
                        rb_ary_push(arr, Qfalse);
                        return arr;
                    } else if (strcmp(rb_id2name(rb_to_id(dir)), "type") == 0) {
                        return INT2FIX(GP_WIDGET_TOGGLE);
                    } else {
                        rb_raise(rb_cGPhoto2ConfigurationError, "Unknown directive '%s'", rb_id2name(rb_to_id(dir)));
                        return Qnil;
                    }
                    break;
                default:
                    rb_raise(rb_cGPhoto2ConfigurationError, "Not supported yet");
                    return Qnil;
            }
            break;
        default:
            rb_raise(rb_eArgError, "Wrong number of arguments (%d for 1 or 2)", argc);
            return Qnil;
    }
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
VALUE camera_set_value(VALUE self, VALUE str, VALUE newVal) {
    const char *name;
    GPhoto2Camera *c;
    CameraWidgetType widgettype;
    VALUE cfg_changed;
    
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

    gp_result_check(gp_widget_get_child_by_name(c->config, name, &(c->childConfig)));
    gp_result_check(gp_widget_get_type(c->childConfig, &widgettype));
    switch (widgettype) {
        case GP_WIDGET_RADIO:
            cfg_changed = rb_iv_get(self, "@configs_changed");
            rb_ary_push(cfg_changed, rb_str_new2(name));
            return setRadio(self, c, newVal, 1);
            break;
        case GP_WIDGET_TEXT:
            cfg_changed = rb_iv_get(self, "@configs_changed");
            rb_ary_push(cfg_changed, rb_str_new2(name));
            return setText(self, c, newVal, 1);
            break;
        case GP_WIDGET_RANGE:
            cfg_changed = rb_iv_get(self, "@configs_changed");
            rb_ary_push(cfg_changed, rb_str_new2(name));
            return setRange(self, c, newVal, 1);
            break;
        case GP_WIDGET_TOGGLE:
            cfg_changed = rb_iv_get(self, "@configs_changed");
            rb_ary_push(cfg_changed, rb_str_new2(name));
            return setToggle(self, c, newVal, 1);
            break;
        default:
            rb_raise(rb_cGPhoto2ConfigurationError, "Cannot access this setting");
            return Qnil;
    }
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
VALUE camera_folder(VALUE self) {
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
VALUE camera_subfolders(VALUE self) {
    int i, count;
    const char *name;
    GPhoto2Camera *c;
    VALUE arr;
    
    Data_Get_Struct(self, GPhoto2Camera, c);
    
    gp_result_check(gp_camera_folder_list_folders(c->camera, c->virtFolder, c->list, c->context));
    count = gp_result_check(gp_list_count(c->list));
    arr = rb_ary_new();
    for (i = 0; i < count; i++) {
        gp_result_check(gp_list_get_name(c->list, i, &name));
        rb_ary_push(arr, rb_str_new2(name));
    }
    return arr;
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
VALUE camera_files(VALUE self) {
    int i, count;
    const char *name;
    GPhoto2Camera *c;
    VALUE arr;
    
    Data_Get_Struct(self, GPhoto2Camera, c);
    
    gp_result_check(gp_filesystem_reset(c->camera->fs));
    gp_result_check(gp_camera_folder_list_files(c->camera, c->virtFolder, c->list, c->context));
    count = gp_result_check(gp_list_count(c->list));
    arr = rb_ary_new();
    for (i = 0; i < count; i++) {
        gp_result_check(gp_list_get_name(c->list, i, &name));
        rb_ary_push(arr, rb_str_new2(name));
    }
    return arr;
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
VALUE camera_folder_up(VALUE self) {
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
VALUE camera_folder_down(VALUE self, VALUE folder) {
    Check_Type(folder, T_STRING);

    const char *name;
    int index;
    GPhoto2Camera *c;
    
    Data_Get_Struct(self, GPhoto2Camera, c);
    
    name = RSTRING(folder)->ptr;
    gp_result_check(gp_camera_folder_list_folders(c->camera, c->virtFolder, c->list, c->context));
    gp_result_check(gp_list_find_by_name(c->list, &index, name));
    if (strlen(c->virtFolder) > 1) {
        strcat(c->virtFolder, "/");
    }
    strcat(c->virtFolder, name);
    return self;
}

/*
 * call-seq:
 *   create_folder(name)            =>      camera
 *
 * Creates subfolder in current camera path with specified name. If not
 * supported by camera throws "Unspecified error" GPhoto2::Exception
 *
 * Examples:
 *
 *   c = GPhoto2::Camera.new
 *   c.create_folder "my_store"
 *
 */
VALUE camera_create_folder(VALUE self, VALUE folder) {
    Check_Type(folder, T_STRING);

    const char *name;
    GPhoto2Camera *c;
    
    Data_Get_Struct(self, GPhoto2Camera, c);
    
    name = RSTRING(folder)->ptr;
    gp_result_check(gp_camera_folder_make_dir(c->camera, c->virtFolder, name, c->context));
    return self;
}

/*
 * call-seq:
 *   wait(timeout=2000)             =>      camera event
 *
 * Waits for an event from camera for specified amount of milliseconds.
 * Returns an instance of GPhoto2::CameraEvent. When capturing image manually
 * with camera connected through USB, images are not saved on memory card
 * until you call this method. During tests EVENT_TYPE_FILE_ADDED event
 * was always followed by EVENT_TYPE_UNKNOWN. So in the case of 
 * EVENT_TYPE_FILE_ADDED or EVENT_TYPE_FOLDER_ADDED, an extra call is made
 * with 100ms timeout which result is ignored.
 *
 * When image is captured manually and then event is caught, camera virtual
 * folder is changed to the one where files are saved.
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
VALUE camera_wait(int argc, VALUE *argv, VALUE self) {
    GPhoto2Camera *c;
    GPhoto2CameraEvent *ce;
    CameraEventType fakeType;
    void *evtData, *fakeData;
    int to;
    
    switch (argc) {
        case 0:
            to = 2000;
            break;
        case 1:
            to = FIX2INT(rb_funcall(argv[0], rb_intern("to_i"), 0));
            break;
        default:
            rb_raise(rb_eArgError, "Wrong number of arguments (%d for 0 or 1)", argc);
            return Qnil;
    }
    
    Data_Get_Struct(self, GPhoto2Camera, c);
    ce = (GPhoto2CameraEvent*) malloc(sizeof(GPhoto2CameraEvent));
    
    gp_result_check(gp_camera_wait_for_event(c->camera, to, &(ce->type), &evtData, c->context));
    
    switch (ce->type) {
        case GP_EVENT_FILE_ADDED:
        case GP_EVENT_FOLDER_ADDED:
            ce->path = (CameraFilePath*)evtData;
            free(c->path);
            c->path = ce->path;
            strcpy(c->virtFolder, c->path->folder);
            gp_result_check(gp_camera_wait_for_event(c->camera, 100, &fakeType, &fakeData, c->context));
            break;
        case GP_EVENT_UNKNOWN:
            gp_result_check(gp_camera_wait_for_event(c->camera, 100, &fakeType, &fakeData, c->context));
            break;
        default:
            break;
    }
    return Data_Wrap_Struct(rb_cGPhoto2CameraEvent, camera_event_mark, camera_event_free, ce);
}

