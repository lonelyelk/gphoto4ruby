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

#include "gphoto2camera_utilities.h"

VALUE rb_cGPhoto2Exception;

void rb_raise_gp_result(int retval) {
    rb_raise(rb_cGPhoto2Exception, "LibGPhoto2 function returned: %s", gp_result_as_string(retval));
}

int gp_result_check(int retval) {
    if (retval < GP_OK) {
        rb_raise_gp_result(retval);
    }
    return retval;
}

void check_disposed(GPhoto2Camera* c) {
  if ((*c).disposed) {
    rb_raise(rb_cGPhoto2Exception, "Camera has been disposed");
  }
}


VALUE getRadio(CameraWidget *cc) {
    const char *val;
    
    gp_result_check(gp_widget_get_value(cc, &val));
    if (val == NULL) {
      return Qnil;
    } else {
      return rb_str_new2(val);
    }
}

VALUE listRadio(CameraWidget *cc) {
    int i, choicesTotal;
    const char *choice;
    VALUE arr;

    choicesTotal = gp_result_check(gp_widget_count_choices(cc));
    arr = rb_ary_new();
    for (i = 0; i < choicesTotal; i++) {
        gp_result_check(gp_widget_get_choice(cc, i, &choice));
        rb_ary_push(arr, rb_str_new2(choice));
    }
    return arr;}

VALUE setRadio(VALUE self, GPhoto2Camera *c, VALUE newVal, int save) {
    const char *val;

    val = RSTRING_PTR(rb_funcall(newVal, rb_intern("to_s"), 0));

    gp_result_check(gp_widget_set_value(c->childConfig, val));
    if (save == 1) {
        saveConfigs(self, c);
    }
    return newVal;
}

VALUE getText(CameraWidget *cc) {
    const char *val;
    
    gp_result_check(gp_widget_get_value(cc, &val));
    if (val == NULL) {
      return Qnil;
    } else {
      return rb_str_new2(val);
    }
}

VALUE setText(VALUE self, GPhoto2Camera *c, VALUE newVal, int save) {
    const char *val;

    val = RSTRING_PTR(rb_funcall(newVal, rb_intern("to_s"), 0));

    gp_result_check(gp_widget_set_value(c->childConfig, val));
    if (save == 1) {
        saveConfigs(self, c);
    }
    return newVal;
}

VALUE getRange(CameraWidget *cc) {
    float val;
    gp_result_check(gp_widget_get_value(cc, &val));
    return rb_float_new(val);
}

VALUE listRange(CameraWidget *cc) {
    float min, max, inc, i;
    VALUE arr;

    gp_result_check(gp_widget_get_range(cc, &min, &max, &inc));
    arr = rb_ary_new();
    if (inc > 0) {
        for (i = min; i <= max; i = i + inc) {
            rb_ary_push(arr, rb_float_new(i));
        }
    } else {
        rb_ary_push(arr, rb_float_new(min));
    }
    return arr;
}

VALUE setRange(VALUE self, GPhoto2Camera *c, VALUE newNum, int save) {
    float val;

    val = NUM2DBL(rb_funcall(newNum, rb_intern("to_f"), 0));

    gp_result_check(gp_widget_set_value(c->childConfig, &val));
    if (save == 1) {
        saveConfigs(self, c);
    }
    return newNum;
}

VALUE getToggle(CameraWidget *cc) {
    int val;
    gp_result_check(gp_widget_get_value(cc, &val));
    if (val == 1) {
        return Qtrue;
    } else {
        return Qfalse;
    }
}

VALUE setToggle(VALUE self, GPhoto2Camera *c, VALUE newVal, int save) {
    int val = -1;
    const char *nV;
    
    switch(TYPE(newVal)) {
        case T_TRUE:
            val = 1;
            break;
        case T_FALSE:
            val = 0;
            break;
        case T_SYMBOL:
            nV = rb_id2name(SYM2ID(newVal));
            if (strcmp(nV, "true") == 0) {
                val = 1;
            } else if(strcmp(nV, "false") == 0) {
                val = 0;
            }
    }
    
    if (val >= 0) {
        gp_result_check(gp_widget_set_value(c->childConfig, &val));
        if (save == 1) {
            saveConfigs(self, c);
        }
        if (val == 1) {
            return Qtrue;
        } else {
            return Qfalse;
        }
    } else {
        return Qnil;
    }
}

VALUE getDate(CameraWidget *cc) {
    int val;
    char str[25];
    gp_result_check(gp_widget_get_value(cc, &val));
//    printf("got: %d\n",val);
    sprintf(str, "Time.at(%d).utc", val);
    return rb_eval_string(str);
}

VALUE setDate(VALUE self, GPhoto2Camera *c, VALUE newNum, int save) {
    int val;

    val = NUM2INT(rb_funcall(newNum, rb_intern("to_i"), 0));

//    printf("setting: %d\n", val);
    gp_result_check(gp_widget_set_value(c->childConfig, &val));
    if (save == 1) {
        saveConfigs(self, c);
    }
    return newNum;
}

void saveConfigs(VALUE self, GPhoto2Camera *c) {
    VALUE cfgs, cfg_changed, name;
    CameraWidgetType widgettype;
    
    gp_result_check(gp_camera_set_config(c->camera, c->config, c->context));
    gp_widget_free(c->config);
    gp_result_check(gp_camera_get_config(c->camera, &(c->config), c->context));
    cfg_changed = rb_iv_get(self, "@configs_changed");
    cfgs = rb_iv_get(self, "@configuration");
    name = rb_ary_shift(cfg_changed);
    while (TYPE(name) != T_NIL) {
        gp_result_check(gp_widget_get_child_by_name(c->config, RSTRING_PTR(name), &(c->childConfig)));
        gp_result_check(gp_widget_get_type(c->childConfig, &widgettype));
        switch (widgettype) {
            case GP_WIDGET_RADIO:
                rb_hash_aset(cfgs, name, getRadio(c->childConfig));
                break;
            case GP_WIDGET_TEXT:
                rb_hash_aset(cfgs, name, getText(c->childConfig));
                break;
            case GP_WIDGET_RANGE:
                rb_hash_aset(cfgs, name, getRange(c->childConfig));
                break;
            case GP_WIDGET_TOGGLE:
                rb_hash_aset(cfgs, name, getToggle(c->childConfig));
                break;
            case GP_WIDGET_DATE:
                rb_hash_aset(cfgs, name, getDate(c->childConfig));
                break;
            default:
                break;
        }
        name = rb_ary_shift(cfg_changed);
    }
}

void populateWithConfigs(CameraWidget *cc, VALUE hash) {
    int i, childrenTotal;
    const char *name;
    CameraWidget *child;
    CameraWidgetType widgettype;
    
    gp_result_check(gp_widget_get_type(cc, &widgettype));
    switch (widgettype) {
        case GP_WIDGET_RADIO:
            gp_result_check(gp_widget_get_name(cc, &name));
            rb_hash_aset(hash, rb_str_new2(name), getRadio(cc));
            break;
        case GP_WIDGET_TEXT:
            gp_result_check(gp_widget_get_name(cc, &name));
            rb_hash_aset(hash, rb_str_new2(name), getText(cc));
            break;
        case GP_WIDGET_RANGE:
            gp_result_check(gp_widget_get_name(cc, &name));
            rb_hash_aset(hash, rb_str_new2(name), getRange(cc));
            break;
        case GP_WIDGET_TOGGLE:
            gp_result_check(gp_widget_get_name(cc, &name));
            rb_hash_aset(hash, rb_str_new2(name), getToggle(cc));
            break;
        case GP_WIDGET_DATE:
            gp_result_check(gp_widget_get_name(cc, &name));
            rb_hash_aset(hash, rb_str_new2(name), getDate(cc));
            break;
        case GP_WIDGET_WINDOW:
        case GP_WIDGET_SECTION:
            childrenTotal = gp_result_check(gp_widget_count_children(cc));
            for (i = 0; i < childrenTotal; i ++) {
                gp_result_check(gp_widget_get_child(cc, i, &child));
                populateWithConfigs(child, hash);
            }
            break;
        default:
            break;
    }
}



