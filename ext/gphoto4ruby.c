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
#include "gphoto4ruby.h"


static VALUE rb_mGPhoto2;
static VALUE rb_cGPhoto2Camera;
static VALUE rb_cGPhoto2Exception;
static VALUE rb_cGPhoto2ConfigurationError;
static VALUE rb_cGPhoto2ProgrammerError;

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
    retval = gp_camera_free(c->camera);
    free(c->context);
    free(c);
}

static VALUE camera_allocate(VALUE klass) {
    int retval;
    GPhoto2Camera *c;
    c = (GPhoto2Camera*) malloc(sizeof(GPhoto2Camera));
    c->context = gp_context_new();
    retval = gp_camera_new(&(c->camera));
    if (retval == GP_OK) {
        retval = gp_camera_get_config(c->camera, &(c->config), c->context);
        if (retval == GP_OK) {
            return Data_Wrap_Struct(klass, camera_mark, camera_free, c);
        }
    }
    rb_raise_gp_result(retval);
    return Qnil;
}

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

static VALUE camera_capture(VALUE self) {
    int retval;
    GPhoto2Camera *c;

    Data_Get_Struct(self, GPhoto2Camera, c);

    retval = gp_camera_capture(c->camera, GP_CAPTURE_IMAGE, &(c->filepath), c->context);
    if (retval == GP_OK) {
//        printf("captured: %s/%s\n", c->filepath.folder, c->filepath.name);
        return self;
    } else {
        rb_raise_gp_result(retval);
        return Qnil;
    }
}

static VALUE camera_get_configs(VALUE self) {
    GPhoto2Camera *c;
    VALUE arr = rb_ary_new();

    Data_Get_Struct(self, GPhoto2Camera, c);

    populateWithConfigs(c->config, arr);
    
    return arr;
}

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

void Init_gphoto4ruby() {
    rb_mGPhoto2 = rb_define_module("GPhoto2");
    rb_cGPhoto2Camera = rb_define_class_under(rb_mGPhoto2, "Camera", rb_cObject);
    rb_cGPhoto2Exception = rb_define_class_under(rb_mGPhoto2, "Exception", rb_eStandardError);
    rb_cGPhoto2ConfigurationError = rb_define_class_under(rb_mGPhoto2, "ConfigurationError", rb_eStandardError);
    rb_cGPhoto2ProgrammerError = rb_define_class_under(rb_mGPhoto2, "ProgrammerError", rb_eStandardError);
    rb_define_alloc_func(rb_cGPhoto2Camera, camera_allocate);
    rb_define_module_function(rb_cGPhoto2Camera, "ports", camera_class_ports, 0);
    rb_define_method(rb_cGPhoto2Camera, "initialize", camera_initialize, -1);
    rb_define_method(rb_cGPhoto2Camera, "configs", camera_get_configs, 0);
    rb_define_method(rb_cGPhoto2Camera, "[]", camera_get_value, -1);
    rb_define_method(rb_cGPhoto2Camera, "[]=", camera_set_value, 2);
    rb_define_method(rb_cGPhoto2Camera, "capture", camera_capture, 0);
}
