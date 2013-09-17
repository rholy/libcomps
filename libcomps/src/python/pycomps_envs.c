/* libcomps - C alternative to yum.comps library
 * Copyright (C) 2013 Jindrich Luza
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to  Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA
 */

#include "pycomps_23macros.h"
#include "pycomps_envs.h"

PyObject* PyCOMPSEnv_convert(void *c) {
    (void)c;
}

PyObject* PyCOMPSEnv_union(PyObject *self, PyObject *other) {
    COMPS_DocEnv *e;
    PyObject *res;

    if (Py_TYPE(other) != &PyCOMPS_EnvType) {
        PyErr_SetString(PyExc_TypeError, "Not Environment instance");
        return NULL;
    }
    e = comps_docenv_union(((PyCOMPS_Env*)self)->env,
                           ((PyCOMPS_Env*)other)->env);

    res = PyCOMPSEnv_new(&PyCOMPS_EnvType, NULL, NULL);
    PyCOMPSEnv_init((PyCOMPS_Env*)res, NULL, NULL);
    COMPS_OBJECT_DESTROY(((PyCOMPS_Env*)res)->env);
    ((PyCOMPS_Env*)res)->env = e;
    return res;
}


void PyCOMPSEnv_dealloc(PyObject *self)
{
    #define env ((PyCOMPS_Env*)self)
    Py_XDECREF(env->p_group_list);
    Py_XDECREF(env->p_option_list);
    Py_XDECREF(env->p_name_by_lang);
    Py_XDECREF(env->p_desc_by_lang);
    COMPS_OBJECT_DESTROY(env);
    Py_TYPE(self)->tp_free((PyObject*)self);
    #undef env
}

PyObject* PyCOMPSEnv_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    PyCOMPS_Env *self;

    (void)args;
    (void)kwds;

    self = (PyCOMPS_Env*) type->tp_alloc(type, 0);
    if (self != NULL) {
        self->env = (COMPS_DocEnv*)comps_object_create(&COMPS_DocEnv_ObjInfo,
                                                       NULL);
        self->p_group_list = NULL;
        self->p_option_list = NULL;
        self->p_name_by_lang = NULL;
        self->p_desc_by_lang = NULL;
    }
    return (PyObject*) self;
}

int PyCOMPSEnv_init(PyCOMPS_Env *self, PyObject *args, PyObject *kwds)
{
    char *name, *id, *desc;
    unsigned disp_ord;
    static char *kwlist[] = {"id", "name", "desc", "display_order", NULL};

    name = NULL;
    id = NULL;
    desc = NULL;
    disp_ord = 0;
    if (!args && !kwds){
        return 0;
    }
    else if (PyArg_ParseTupleAndKeywords(args, kwds, "|sssi", kwlist,
                                    &id, &name, &desc, &disp_ord)) {
        comps_docenv_set_id(self->env, id, 1);
        comps_docenv_set_name(self->env, name, 1);
        comps_docenv_set_desc(self->env, desc, 1);
        comps_docenv_set_display_order(self->env, disp_ord);
        return 0;
    } else {
        return -1;
    }
}

int PyCOMPSEnv_print(PyObject *self, FILE *f, int flags) {
    COMPS_ObjListIt *it;
    COMPS_HSList *pairlist;
    COMPS_HSListItem *hsit;
    char *id, *name, *desc, *display_order;
    COMPS_Object *tmp_prop;

    #define _env_ ((PyCOMPS_Env*)self)->env
    (void)flags;

    tmp_prop = (COMPS_Object*)comps_docenv_get_id(_env_);
    id = (tmp_prop)?comps_object_tostr(tmp_prop): NULL;
    tmp_prop = (COMPS_Object*)comps_docenv_get_name(_env_);
    name = (tmp_prop)?comps_object_tostr(tmp_prop): NULL;
    tmp_prop = (COMPS_Object*)comps_docenv_get_desc(_env_);
    desc = (tmp_prop)?comps_object_tostr(tmp_prop): NULL;
    tmp_prop = (COMPS_Object*)comps_docenv_get_display_order(_env_);
    display_order = (tmp_prop)?comps_object_tostr(tmp_prop): NULL;

    fprintf(f, "<COMPS_Environment: id='%s'"
               "name='%s' description='%s' display_order=%s", id, name, desc,
                display_order);
    free(id);
    free(name);
    free(desc);
    free(display_order);

    fprintf(f, ", name_by_lang={");
    pairlist = comps_objrtree_pairs(_env_->name_by_lang);
    for (hsit = pairlist->first; hsit != pairlist->last; hsit = hsit->next) {
        printf("'%s': '%s', ", ((COMPS_ObjRTreePair*)hsit->data)->key,
                               (char*)((COMPS_ObjRTreePair*)hsit->data)->data);
    }
    if (hsit)
        printf("'%s': '%s'}", ((COMPS_ObjRTreePair*)hsit->data)->key,
                           (char*)((COMPS_ObjRTreePair*)hsit->data)->data);
    comps_hslist_destroy(&pairlist);

    fprintf(f, ", desc_by_lang={");
    pairlist = comps_objrtree_pairs(_env_->desc_by_lang);
    for (hsit = pairlist->first; hsit != pairlist->last; hsit = hsit->next) {
        printf("'%s': '%s', ", ((COMPS_ObjRTreePair*)hsit->data)->key,
                               (char*)((COMPS_ObjRTreePair*)hsit->data)->data);
    }
    if (hsit)
        printf("'%s': '%s'}", ((COMPS_ObjRTreePair*)hsit->data)->key,
                           (char*)((COMPS_ObjRTreePair*)hsit->data)->data);
    comps_hslist_destroy(&pairlist);

    fprintf(f, ", group_list=[");
    if (_env_->group_list) {
        for (it = _env_->group_list->first;
             it != _env_->group_list->last; it = it->next){
            id = comps_object_tostr(it->comps_obj);
            fprintf(f, "'%s', ", id);
            free(id);
        }
        if (it) {
            id = comps_object_tostr(it->comps_obj);
            fprintf(f, "'%s'", id);
            free(id);
        }
    }
    fprintf(f, "]");
    fprintf(f, ", ");
    fprintf(f, ", option_list=[");
    if (_env_->option_list) {
        for (it = _env_->option_list->first;
             it != _env_->option_list->last; it = it->next){
            id = comps_object_tostr(it->comps_obj);
            fprintf(f, "'%s', ", id);
            free(id);
        }
        if (it) {
            id = comps_object_tostr(it->comps_obj);
            fprintf(f, "'%s'", id);
            free(id);
        }
    }
    fprintf(f, "]>");
    return 0;
    #undef _env_
}

PyObject* PyCOMPSEnv_str(PyObject *self) {
    return PyUnicode_FromString(comps_object_tostr((COMPS_Object*)
                                                   ((PyCOMPS_Env*)self)->env));
}

PyObject* PyCOMPSEnv_cmp(PyObject *self, PyObject *other, int op) {
    char ret;
    CMP_OP_EQ_NE_CHECK(op)

    if (other == NULL || ( Py_TYPE(other) != &PyCOMPS_EnvType
                           && other != Py_None)) {
        PyErr_SetString(PyExc_TypeError, "Not Environment instance");
        return NULL;
    }
    CMP_NONE_CHECK(op, self, other)
    ret = COMPS_OBJECT_CMP((COMPS_Object*)((PyCOMPS_Env*)self)->env,
                           (COMPS_Object*)((PyCOMPS_Env*)other)->env);
    if (op == Py_EQ) {
        if (!ret) Py_RETURN_FALSE;
    } else {
        if (ret) Py_RETURN_FALSE;
    }
    Py_RETURN_TRUE;
}

PyObject* PyCOMPSEnv_get_ids(PyCOMPS_Env *self, void *closure) {
    #define _closure_ ((PyCOMPS_Env_ListGetSetClosure*)closure)

    PyObject *ret = (PyObject*)GET_FROM(self, _closure_->p_offset);
    if (!ret) {
        ret = PyCOMPSSeq_new(&PyCOMPS_GIDsType, NULL, NULL);
        
        //TODO
    } else {
        ret = self->p_group_list;
        //TODO
    }
    return  ret;
    #undef _closure_
}

int PyCOMPSEnv_set_groupids(PyCOMPS_Env *self,
                                  PyObject *value, void *closure) {
    (void) closure;
    (void) self;
    if (!value) {
        PyErr_SetString(PyExc_TypeError, "Cannot delete attribute option_ids");
        return -1;
    }
    if (value->ob_type != &PyCOMPS_GIDsType) {
        PyErr_SetString(PyExc_TypeError, "Not GroupIds instance");
        return -1;
    }
    //TODO
    return 0;
}

PyObject* PyCOMPSEnv_get_optionids(PyCOMPS_Env *self, void *closure) {
    (void) closure;
    PyObject *ret;
    if (!self->p_option_list) {
        //TODO
    } else {
        //TODO
    }
    return  ret;
}

int PyCOMPSEnv_set_optionids(PyCOMPS_Env *self,
                                  PyObject *value, void *closure) {
    (void) closure;
    (void) self;
    if (!value) {
        PyErr_SetString(PyExc_TypeError, "Cannot delete attribute option_ids");
        return -1;
    }
    if (value->ob_type != &PyCOMPS_GIDsType) {
        PyErr_Format(PyExc_TypeError, "Not %s instance", PyCOMPS_GIDsType.tp_name);
        return -1;
    }
    //TODO
    return 0;
}

inline PyObject* PyCOMPSEnv_get_name_by_lang(PyObject *self, void *closure) {
    (void) closure;
    (void) self;
    //TODO
}

inline int PyCOMPSEnv_set_name_by_lang(PyObject *self,
                                       PyObject *value, void *closure) {
    (void) closure;
    (void) value;
    (void) self;
    //TODO
}

inline PyObject* PyCOMPSEnv_get_desc_by_lang(PyObject *self, void *closure) {
    (void) closure;
    (void) self;
    //TODO
}

inline int PyCOMPSEnv_set_desc_by_lang(PyObject *self, PyObject *value,
                                                          void *closure) {
    (void) closure;
    (void) value;
    (void) self;
    //TODO
}

int pycomps_env_strattr_setter(PyObject *self, PyObject *val, void *closure) {
    #define _closure_ ((PyCOMPS_Env_PropGetSetClosure*)closure)
    char *tmp;
    if (__pycomps_stringable_to_char(val, &tmp) < 0) {
        return -1;
    }
    _closure_->set_f(((PyCOMPS_Env*)self)->env, tmp, 0);
    #undef _closure_
}

PyObject* pycomps_env_strattr_getter(PyObject *self, void *closure) {
    #define _closure_ ((PyCOMPS_Env_PropGetSetClosure*)closure)
    COMPS_Object* tmp_prop;
    tmp_prop = _closure_->get_f(((PyCOMPS_Env*)self)->env);
    if (tmp_prop)
        return PyUnicode_FromString(comps_object_tostr(tmp_prop));
    else
        Py_RETURN_NONE;
    #undef _closure_
}

PyMemberDef PyCOMPSEnv_members[] = {
    {NULL}};

PyMethodDef PyCOMPSEnv_methods[] = {
    {NULL}  /* Sentinel */
};

PyCOMPS_Env_PropGetSetClosure DocEnv_IdClosure = {
    .get_f = &comps_docenv_get_id,
    .set_f = &comps_docenv_set_id
};

PyCOMPS_Env_PropGetSetClosure DocEnv_NameClosure = {
    .get_f = &comps_docenv_get_name,
    .set_f = &comps_docenv_set_name
};

PyCOMPS_Env_PropGetSetClosure DocEnv_DescClosure = {
    .get_f = &comps_docenv_get_desc,
    .set_f = &comps_docenv_set_desc
};

PyGetSetDef PyCOMPSEnv_getset[] = {
    {"id",
     (getter)pycomps_env_strattr_getter, (setter)pycomps_env_strattr_setter,
     "Env id", (void*)&DocEnv_IdClosure},
    {"name",
     (getter)pycomps_env_strattr_getter, (setter)pycomps_env_strattr_setter,
     "Env name", (void*)&DocEnv_NameClosure},
    {"desc",
     (getter)pycomps_env_strattr_getter, (setter)pycomps_env_strattr_setter,
     "Env description", (void*)&DocEnv_DescClosure},
    {"group_ids",
     (getter)PyCOMPSEnv_get_groupids, (setter)PyCOMPSEnv_set_groupids,
     "Env group ids",
     NULL},
    {"option_ids",
     (getter)PyCOMPSEnv_get_optionids, (setter)PyCOMPSEnv_set_optionids,
     "Env group ids",
     NULL},
    {"name_by_lang",
    (getter)PyCOMPSEnv_get_name_by_lang, (setter)PyCOMPSEnv_set_name_by_lang,
     "Env name locales", NULL},
    {"desc_by_lang",
    (getter)PyCOMPSEnv_get_desc_by_lang, (setter)PyCOMPSEnv_set_desc_by_lang,
     "Env description locales", NULL},
    {NULL}  /* Sentinel */
};

PyNumberMethods PyCOMPSEnv_Nums = {
    .nb_add = PyCOMPSEnv_union
};

/*PyCOMPS_CtoPySeqItemMan PyCOMPSEnv_ItemMan = {
    .item_type = &PyCOMPS_EnvType,
    .ctopy_convert = &PyCOMPSEnv_convert,
    .data_decref = &pycomps_env_decref,
    .data_incref = &comps_env_incref,
    .data_cmp = &comps_docenv_cmp_v,
    .fprint_f = &pycomps_env_print,
    .str_f = &comps_env_str
};*/

PyTypeObject PyCOMPS_EnvType = {
    PY_OBJ_HEAD_INIT
    "libcomps.Env",   /*tp_name*/
    sizeof(PyCOMPS_Env), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)PyCOMPSEnv_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,//PyCOMPSEnv_cmp,            /*tp_compare*/
    0,                         /*tp_repr*/
    &PyCOMPSEnv_Nums,          /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    PyCOMPS_hash,              /*tp_hash */
    0,                         /*tp_call*/
    &PyCOMPSEnv_str,            /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_TYPE_SUBCLASS,        /*tp_flags*/
    "Comps Env",          /* tp_doc */
    0,                    /* tp_traverse */
    0,                     /* tp_clear */
    &PyCOMPSEnv_cmp,                     /* tp_richcompare */
    0,                     /* tp_weaklistoffset */
    0,                     /* tp_iter */
    0,                     /* tp_iternext */
    PyCOMPSEnv_methods,        /* tp_methods */
    PyCOMPSEnv_members,        /* tp_members */
    PyCOMPSEnv_getset,         /* tp_getset */
    0,//&PyCOMPS_SeqItemType,        /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)PyCOMPSEnv_init,      /* tp_init */
    0,                               /* tp_alloc */
    PyCOMPSEnv_new,                 /* tp_new */};

/*
int PyCOMPSEnvs_init(PyCOMPS_CtoPySeq *self, PyObject *args, PyObject *kwds)
{
    (void) args;
    (void) kwds;
    self->item_man = &PyCOMPSEnv_ItemMan;
    return 0;
}

inline PyObject* PyCOMPSEnvs_append(PyObject * self, PyObject *item) {
    if (item == NULL || Py_TYPE(item) != &PyCOMPS_EnvType) {
        PyErr_SetString(PyExc_TypeError, "Not Env instance");
        return NULL;
    }
    PyCOMPSSeq_append(self, item);
    Py_RETURN_NONE;
}
COMPS_List* comps_envs_union(COMPS_List *envs1, COMPS_List *envs2) {
    COMPS_ListItem *it, *newit;
    COMPS_HSListItem *hsit;
    COMPS_Set *set;
    COMPS_List *res;
    COMPS_DocEnv *tmpenv, *tmpenv2, *unienv;

    res = comps_list_create();
    comps_list_init(res);

    set = comps_set_create();
    comps_set_init(set, NULL, NULL, NULL, &__comps_docenv_idcmp);

    it = (envs1)?envs1->first:NULL;
    for (; it != NULL; it = it->next) {
        comps_env_incref(it->data);
        comps_set_add(set, it->data);
    }

    it = (envs2)?envs2->first:NULL;
    for (; it != NULL; it = it->next) {
        tmpenv = it->data;
        if (tmpenv->group_list == NULL) {
            tmpenv->group_list = comps_list_create();
            comps_list_init(tmpenv->group_list);
            comps_env_get_extra(tmpenv)->group_list_citem->data =
                                                         tmpenv->group_list;
        }
        if (tmpenv->option_list == NULL) {
            tmpenv->option_list = comps_list_create();
            comps_list_init(tmpenv->option_list);
            comps_env_get_extra(tmpenv)->option_list_citem->data =
                                                         tmpenv->option_list;
        }
        if (comps_set_in(set, tmpenv)) {
            tmpenv2 = (COMPS_DocEnv*)comps_set_data_at(set, tmpenv);
            unienv = comps_docenv_union(tmpenv, tmpenv2);
            unienv->reserved = comps_docenv_extra_create();
            ((COMPS_DocEnvExtra*)(unienv->reserved))->citem =
                               ctopy_citem_create(unienv, &pycomps_env_destroy);
            ((COMPS_DocEnvExtra*)(unienv->reserved))->group_list_citem =
                                        ctopy_citem_create(unienv->group_list,
                                                           &comps_list_destroy_v);
            ((COMPS_DocEnvExtra*)(unienv->reserved))->option_list_citem =
                                        ctopy_citem_create(unienv->option_list,
                                                           &comps_list_destroy_v);
            ((COMPS_DocEnvExtra*)(unienv->reserved))->name_by_lang_citem =
                                        ctopy_citem_create(unienv->name_by_lang,
                                                           &comps_dict_destroy_v);
            ((COMPS_DocEnvExtra*)(unienv->reserved))->desc_by_lang_citem =
                                        ctopy_citem_create(unienv->desc_by_lang,
                                                           &comps_dict_destroy_v);
            pycomps_env_decref(tmpenv2);
            comps_set_remove(set, tmpenv2);
            comps_set_add(set, unienv);
        } else {
            comps_env_incref((void*)tmpenv);
            comps_set_add(set, tmpenv);
        }
    }
    for (hsit = set->data->first; hsit != NULL; hsit = hsit->next) {
        newit = comps_list_item_create(hsit->data, NULL, &pycomps_env_decref);
        comps_list_append(res, newit);
    }
    comps_set_destroy(&set);
    return res;


}
PyObject* PyCOMPSEnvs_union(PyObject *self, PyObject *other) {
    PyCOMPS_CtoPySeq *res;
    COMPS_List *res_list;

    if (other == NULL || Py_TYPE(other) != &PyCOMPS_EnvsType) {
        PyErr_SetString(PyExc_TypeError, "Not EnvironmentList instance");
        return NULL;
    }

    res = (PyCOMPS_CtoPySeq*) Py_TYPE(self)->tp_new(Py_TYPE(self), NULL, NULL);
    PyCOMPSEnvs_init(res, NULL, NULL);
    res_list = comps_envs_union(ctopy_get_list(self), ctopy_get_list(other));
    comps_list_destroy((COMPS_List**)&res->citem->data);
    res->citem->data = res_list;

    return (PyObject*)res;
}

PyMemberDef PyCOMPSEnvs_members[] = {
    {NULL}};
*/


//PyMethodDef PyCOMPSEnvs_methods[] = {
//    //{"append", (PyCFunction)PyCOMPSEnvs_append, METH_O,
//    //"Append item to new of the list"},
//    {NULL}  /* Sentinel */
//};

//PyNumberMethods PyCOMPSEnvs_Nums = {
//    .nb_add = PyCOMPSEnvs_union
//};


PyTypeObject PyCOMPS_EnvsType = {
    PY_OBJ_HEAD_INIT
    "libcomps.EnvList",   /*tp_name*/
    sizeof(PyCOMPS_Sequence), /*tp_basicsize*/
    sizeof(PyCOMPS_Env),   /*tp_itemsize*/
    (destructor)PyCOMPSCtoPySeq_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0, //&PyCOMPSEnvs_cmp,          /*tp_compare*/
    0,                         /*tp_repr*/
    &PyCOMPSEnvs_Nums,         /*tp_as_number*/
    0,//&PyCOMPSEnv_sequence,       /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    PyCOMPSCtoPySeq_str,           /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_TYPE_SUBCLASS,        /*tp_flags*/
    "Comps Env list",           /* tp_doc */
    0,                    /* tp_traverse */
    0,                     /* tp_clear */
    &PyCOMPSCtoPySeq_cmp,       /* tp_richcompare */
    0,                     /* tp_weaklistoffset */
    0,                     /* tp_iter */
    0,                     /* tp_iternext */
    PyCOMPSEnvs_methods,        /* tp_methods */
    PyCOMPSEnvs_members,        /* tp_members */
    0,                         /* tp_getset */
    &PyCOMPS_CtoPySeqType,           /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)PyCOMPSEnvs_init,      /* tp_init */
    0,                               /* tp_alloc */
    PyCOMPSCtoPySeq_new,                 /* tp_new */};

