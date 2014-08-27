#ifndef __FFPYTHON_H_
#define __FFPYTHON_H_

#include <Python.h>
#include <structmember.h>

#include <string>
#include <sstream>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <stdexcept>
using namespace std;

#ifdef _WIN32
#define  SAFE_SPRINTF   _snprintf_s
#else
#define  SAFE_SPRINTF   snprintf
#endif


//! ��ȡpython�쳣��Ϣ
struct pyops_t
{
    static int traceback(string& ret_);
};
struct cpp_void_t{};

//! ���ڳ�ȡ���͡����Ͷ�Ӧ������
template<typename T>
struct type_ref_traits_t;

//! ����python ��ѡ����
template<typename T>
struct pyoption_t
{
        typedef typename type_ref_traits_t<T>::value_t value_t;
        pyoption_t():m_set_flag(false){}
        bool is_set() const { return m_set_flag;}
        void set()                        { m_set_flag = true;}
        value_t&       value()    { return m_value;}
        const value_t& value() const{ return m_value;}
        
        const value_t& value(const value_t& default_)
        {
            if (is_set())
                return m_value;
            else
                return default_;
        }
        bool        m_set_flag;
        value_t m_value;
};
//! �����ж��Ƿ��ǿ�ѡ����
template<typename T>
struct pyoption_traits_t;

//! pytype_traits_t ��װ PyLong_FromLong ���صĲ���������Ϊ����python���ɲ���
template<typename T>
struct pytype_traits_t;

//! ���ڵ���python����������tuple���͵�python���������Ĺ�����
struct pycall_arg_t
{
    pycall_arg_t(int arg_num):
        arg_index(0),
        pargs_tuple(PyTuple_New(arg_num))
    {}
    ~pycall_arg_t()
    {
        release();
    }
    PyObject * get_args() const
    {
        return pargs_tuple;
    }
    template<typename T>
    pycall_arg_t& add(const T& val_)
    {
        if (arg_index < PyTuple_Size(pargs_tuple))
        {
            PyObject* tmp_arg = pytype_traits_t<T>::pyobj_from_cppobj(val_);
            PyTuple_SetItem(pargs_tuple, arg_index, tmp_arg);
            ++arg_index;
        }
        return *this;
    }
    void release()
    {
        if (pargs_tuple)
        {
            Py_DECREF(pargs_tuple);
            pargs_tuple = NULL;
        }
    }
    int         arg_index;
    PyObject *  pargs_tuple;
};

//! ���ڵ���python��������ȡ����ֵ�Ĺ�����
class pytype_tool_t
{
public:
    virtual ~pytype_tool_t(){};
    virtual int parse_value(PyObject *pvalue_) = 0;
    virtual const char* return_type() {return "";}
};

//! ���ڵ���python��������ȡ����ֵ�Ĺ��߷�����
template<typename T>
class pytype_tool_impl_t;
//! ��װ����python������C API
struct pycall_t
{
    static int call_func(PyObject *pModule, const string& mod_name_, const string& func_name_,
                         pycall_arg_t& pyarg_, pytype_tool_t& pyret_, string& err_);
    static int call_func_obj(PyObject *pFunc, pycall_arg_t& pyarg_, pytype_tool_t& pyret_, string& err_);
    template<typename T>
    static const T& call(const string& mod_name_, const string& func_name_, pycall_arg_t& pyarg_, pytype_tool_impl_t<T>& pyret);
    template<typename T>
    static const T& call_obj_method(PyObject *pObj, const string& func_name_, pycall_arg_t& pyarg_, pytype_tool_impl_t<T>& pyret);
    template<typename T>
    static const T& call_lambda(PyObject *pFunc, pycall_arg_t& pyarg_, pytype_tool_impl_t<T>& pyret);
};
//! ������չpython�Ĺ����࣬������������
struct pyext_tool_t;


template<typename T>
struct pyext_return_tool_t;

//! ������չpython������pyobject���͵ķ���ֵ��python
template <typename T>
struct pyext_func_traits_t;

//! ������չpython��traits��ע����python�ĺ����ӿ�
#ifndef PYCTOR
#define  PYCTOR int (*)
#endif
//! ��ʾvoid���ͣ�����void���Ͳ���return����void_ignore_t����
template<typename T>
struct void_ignore_t;

template<typename T>
struct void_ignore_t
{
    typedef T value_t;
};

template<>
struct void_ignore_t<void>
{
    typedef cpp_void_t value_t;
};

#define  RET_V typename void_ignore_t<RET>::value_t

//! ��¼�����������������໥��ϵ
struct cpp_to_pyclass_reg_info_t
{
    struct inherit_info_t
    {
        inherit_info_t():pytype_def(NULL){}
        PyTypeObject* pytype_def;
        string inherit_name;
        set<PyTypeObject*> all_child;
    };
    typedef map<string, inherit_info_t> inherit_info_map_t;
    static inherit_info_map_t& get_all_info()
    {
        static inherit_info_map_t inherit_info;
        return inherit_info;
    }

    static void add(const string& child_, const string& base_, PyTypeObject* def_)
    {
        inherit_info_t tmp;
        tmp.inherit_name = base_;
        tmp.pytype_def = def_;
        get_all_info()[child_] = tmp;
        get_all_info()[base_].all_child.insert(def_);
    }
    static bool is_instance(PyObject* pysrc, const string& class_)
    {
        inherit_info_map_t& inherit_info = get_all_info();
        inherit_info_t& tmp = inherit_info[class_];
        if (tmp.pytype_def && PyObject_TypeCheck(pysrc, tmp.pytype_def))
        {
            return true;
        }
        for (set<PyTypeObject*>::iterator it = tmp.all_child.begin(); it != tmp.all_child.end(); ++it)
        {
            if (*it && PyObject_TypeCheck(pysrc, *it))
            {
                return true;
            }
        }

        return false;
    }
    
};


//! ��¼C++ class ��Ӧ��python�е����ơ�������Ϣ��,ȫ��
struct static_pytype_info_t
{
    string class_name;
    string mod_name;
    int    total_args_num;
    PyTypeObject* pytype_def;
};

//! �����࣬�������ɷ���python class�Ľӿڣ��������䡢�ͷ�
template<typename T>
struct pyclass_base_info_t
{
    struct obj_data_t
    {
        obj_data_t():obj(NULL){}

        PyObject_HEAD;
        T* obj;
        bool forbid_release;
        void disable_auto_release(){ forbid_release = true; }
        void release()
        {
            if (obj)
            {
                delete obj;
                obj = NULL;
            }
        }
    };

    static void free_obj(obj_data_t* self)
    {
        if  (false == self->forbid_release && self->obj)
        {
            self->release();
        }
        self->ob_type->tp_free((PyObject*)self);
    }

    static PyObject *alloc_obj(PyTypeObject *type, PyObject *args, PyObject *kwds)
    {
        obj_data_t *self = (obj_data_t *)type->tp_alloc(type, 0);
        return (PyObject *)self;
    }
        static PyObject *release(PyTypeObject *type, PyObject *args)
    {
        obj_data_t *self = (obj_data_t *)type;
                self->release();
                Py_RETURN_TRUE;
    }
    static static_pytype_info_t pytype_info;
};
template<typename T>
static_pytype_info_t pyclass_base_info_t<T>::pytype_info;

//! ��������pyclass ��ʼ������
template <typename CLASS_TYPE, typename CTOR>
struct pyclass_ctor_tool_t;

//! used to gen method of py class
template<typename T>
struct pyclass_method_gen_t;

//! ��ֹ����ָ��ΪNULL���ó���
#define  NULL_PTR_GUARD(X) if (NULL == X) {PyErr_SetString(PyExc_TypeError, "obj data ptr NULL");return NULL;}

//! ��������python ��getter��setter�ӿڣ�������c++ class�ĳ�Ա����
template <typename CLASS_TYPE, typename RET>
struct pyclass_member_func_gen_t
{
    typedef typename pyclass_base_info_t<CLASS_TYPE>::obj_data_t obj_data_t;
    typedef RET CLASS_TYPE::* property_ptr_t;

    static PyObject *getter_func(obj_data_t *self, void *closure)
    {
        property_ptr_t property_ptr;
        ::memcpy(&property_ptr, &closure, sizeof(closure));
        NULL_PTR_GUARD(self->obj);
        CLASS_TYPE* p = self->obj;
        return pytype_traits_t<RET>::pyobj_from_cppobj(p->*property_ptr);
    }
    static int setter_func(obj_data_t *self, PyObject *value, void *closure)
    {
        property_ptr_t property_ptr;
        ::memcpy(&property_ptr, &closure, sizeof(closure));
        CLASS_TYPE* p = self->obj;

        return pytype_traits_t<RET>::pyobj_to_cppobj(value, p->*property_ptr);
    }
};

//! ����C++ ע��class�Ĺ����࣬����¼class��Ӧ�����ơ���Ա��������Ա����
class pyclass_regigster_tool_t
{
public:
    struct method_info_t
    {
        PyCFunction func;
        string func_name;
        string func_real_name;
        string doc;
        string doc_real;
        int  args_num;
        int  option_args_num;
        long func_addr;
    };
    struct property_info_t
    {
        void*   ptr;
        string  property_name;
        string  doc;
        getter  getter_func;
        setter  setter_func;
    };
    virtual ~pyclass_regigster_tool_t(){}

    typedef PyObject *(*pyobj_alloc_t)(PyTypeObject*, PyObject*, PyObject*);

    string                class_name;
    string                class_real_name;
    string                class_name_with_mod;
    string                class_reel_name_with_mod;
    string                inherit_name;
    int         type_size;
    string      doc;
    int         args_num;
    int         option_args_num;
    destructor  dector;
    initproc    init;
    pyobj_alloc_t ctor;

    //!  member functions
    PyCFunction                      delete_func;
    vector<method_info_t>        methods_info;
    //! property 
    vector<property_info_t>        propertys_info;
    //! for init module
    PyTypeObject                        pytype_def;
    //! method
    vector<PyMethodDef>                pymethod_def;
    //! property
    vector<PyGetSetDef>     pyproperty_def;

    //! ��̬������Ҫȫ�ּ�¼�����ͱ�ע��������python ����
    static_pytype_info_t*   static_pytype_info;

    template<typename FUNC>
    pyclass_regigster_tool_t& reg(FUNC f_, const string& func_name_, string doc_ = "")
    {
        method_info_t tmp;
        tmp.func_name = func_name_;
        tmp.func_real_name = func_name_ + "_internal_";
        tmp.doc       = doc_;
        tmp.doc_real  = "internal use";
        tmp.func      = (PyCFunction)pyclass_method_gen_t<FUNC>::pymethod;
        tmp.args_num = pyclass_method_gen_t<FUNC>::args_num();
        tmp.option_args_num = pyclass_method_gen_t<FUNC>::option_args_num();
        ::memcpy(&tmp.func_addr, &f_, sizeof(f_));
        methods_info.push_back(tmp);
        return *this;
    }
    template<typename CLASS_TYPE,typename RET>
    pyclass_regigster_tool_t& reg_property(RET CLASS_TYPE::* member_, const string& member_name_, string doc_ = "")
    {
        property_info_t tmp;
        ::memcpy(&tmp.ptr, &member_, sizeof(member_));
        tmp.property_name = member_name_;
        tmp.doc = doc_;
        tmp.getter_func = (getter)pyclass_member_func_gen_t<CLASS_TYPE, RET>::getter_func;
        tmp.setter_func = (setter)pyclass_member_func_gen_t<CLASS_TYPE, RET>::setter_func;
        propertys_info.push_back(tmp);
        return *this;
    }
};


class ffpython_t
{
    struct reg_info_t
    {
        reg_info_t():args_num(0),option_args_num(0),func_addr(0){}
        int  args_num;
        int  option_args_num;
        long func_addr;
        PyCFunction func;
        string func_name;
        string func_impl_name;
        string doc;
        string doc_impl;
    };

public:
    ffpython_t()
    {
        if (!Py_IsInitialized())
            Py_Initialize();
    }
    ~ffpython_t()
    {
        clear_cache_pyobject();
    }
    static int init_py();
    static int final_py();
    static int add_path(const string& path_);
    static int run_string(const string& py_);
    static int reload(const string& py_);
    static int load(const string& py_);
    //! ע��static function��
    template<typename T>
    ffpython_t& reg(T func_, const string& func_name_, string doc_ = "")
    {
        reg_info_t tmp;
        tmp.args_num = pyext_func_traits_t<T>::args_num();
        tmp.option_args_num = pyext_func_traits_t<T>::option_args_num();
        tmp.func     = (PyCFunction)pyext_func_traits_t<T>::pyfunc;
        tmp.func_name= func_name_;
        tmp.func_impl_name = func_name_ + "_internal_";
        tmp.doc      = doc_;
        tmp.doc_impl = string("internal use, please call ") + func_name_;
        tmp.func_addr= (long)func_;
        
        m_func_info.push_back(tmp);
        return *this;
    }

    //! ע��c++ class
    template<typename T, typename CTOR>
    pyclass_regigster_tool_t& reg_class(const string& class_name_, string doc_ = "", string inherit_name_ = "")
    {
        if (pyclass_base_info_t<T>::pytype_info.class_name.empty() == false)
            throw runtime_error("this type has been registed");

        pyclass_base_info_t<T>::pytype_info.class_name = class_name_;
        //pyclass_base_info_t<T>::pytype_info.mod_name   = m_mod_name;
        pyclass_base_info_t<T>::pytype_info.total_args_num = pyext_func_traits_t<CTOR>::args_num() + 
                                                                     pyext_func_traits_t<CTOR>::option_args_num();

        pyclass_regigster_tool_t tmp;
        tmp.class_name                = class_name_;
        tmp.class_real_name        = class_name_ + "_internal_";
        tmp.inherit_name        = inherit_name_;
        tmp.doc             = doc_;
        tmp.dector                        = (destructor)pyclass_base_info_t<T>::free_obj;
        tmp.init                        = (initproc)pyclass_ctor_tool_t<T, CTOR>::init_obj;
        tmp.ctor                        = pyclass_base_info_t<T>::alloc_obj;
        tmp.type_size                = sizeof(typename pyclass_base_info_t<T>::obj_data_t);
        tmp.args_num        = pyext_func_traits_t<CTOR>::args_num();
        tmp.option_args_num = pyext_func_traits_t<CTOR>::option_args_num();
        tmp.static_pytype_info = &(pyclass_base_info_t<T>::pytype_info);
        //! ע����������,python����������������,��������gcʱ�Զ�����
        tmp.delete_func = (PyCFunction)pyclass_base_info_t<T>::release;
            m_all_pyclass.push_back(tmp);

            return m_all_pyclass.back();
    }

    //! ����Ҫע���ĺ���������ע�ᵽpython������
    int init(const string& mod_name_, string doc_ = "");

    //! ����python����������֧��9������
    template<typename RET>
    RET_V call(const string& mod_name_, const string& func_)
    {
        pycall_arg_t args(0);
        pytype_tool_impl_t<RET_V> pyret;
        return pycall_t::call<RET_V>(mod_name_, func_, args, pyret);
    }
    template<typename RET, typename ARG1>
    RET_V call(const string& mod_name_, const string& func_, const ARG1& a1)
    {
        pycall_arg_t args(1);
        args.add(a1);
        pytype_tool_impl_t<RET_V> pyret;
        return pycall_t::call<RET_V>(mod_name_, func_, args, pyret);
    }
    template<typename RET, typename ARG1, typename ARG2>
    RET_V call(const string& mod_name_, const string& func_, const ARG1& a1, const ARG2& a2)
    {
        pycall_arg_t args(2);
        args.add(a1).add(a2);
        pytype_tool_impl_t<RET_V> pyret;
        return pycall_t::call<RET_V>(mod_name_, func_, args, pyret);
    }
    template<typename RET, typename ARG1, typename ARG2, typename ARG3>
    RET_V call(const string& mod_name_, const string& func_, const ARG1& a1, const ARG2& a2, const ARG3& a3)
    {
        pycall_arg_t args(3);
        args.add(a1).add(a2).add(a3);
        pytype_tool_impl_t<RET_V> pyret;
        return pycall_t::call<RET_V>(mod_name_, func_, args, pyret);
    }
    template<typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4>
    RET_V call(const string& mod_name_, const string& func_, const ARG1& a1, const ARG2& a2, const ARG3& a3, const ARG4& a4)
    {
        pycall_arg_t args(4);
        args.add(a1).add(a2).add(a3).add(a4);
        pytype_tool_impl_t<RET_V> pyret;
        return pycall_t::call<RET_V>(mod_name_, func_, args, pyret);
    }
    template<typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5>
    RET_V call(const string& mod_name_, const string& func_, const ARG1& a1, const ARG2& a2, const ARG3& a3, const ARG4& a4,
                const ARG5& a5)
    {
        pycall_arg_t args(5);
        args.add(a1).add(a2).add(a3).add(a4).add(a5);
        pytype_tool_impl_t<RET_V> pyret;
        return pycall_t::call<RET_V>(mod_name_, func_, args, pyret);
    }
    template<typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6>
    RET_V call(const string& mod_name_, const string& func_, const ARG1& a1, const ARG2& a2, const ARG3& a3, const ARG4& a4,
                const ARG5& a5,const ARG6& a6)
    {
        pycall_arg_t args(6);
        args.add(a1).add(a2).add(a3).add(a4).add(a5).add(a6);
        pytype_tool_impl_t<RET_V> pyret;
        return pycall_t::call<RET_V>(mod_name_, func_, args, pyret);
    }
    template<typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7>
    RET_V call(const string& mod_name_, const string& func_, const ARG1& a1, const ARG2& a2, const ARG3& a3, const ARG4& a4,
                const ARG5& a5,const ARG6& a6,const ARG7& a7)
    {
        pycall_arg_t args(7);
        args.add(a1).add(a2).add(a3).add(a4).add(a5).add(a6).add(a7);
        pytype_tool_impl_t<RET_V> pyret;
        return pycall_t::call<RET_V>(mod_name_, func_, args, pyret);
    }
    template<typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7,
             typename ARG8>
    RET_V call(const string& mod_name_, const string& func_, const ARG1& a1, const ARG2& a2, const ARG3& a3, const ARG4& a4,
                const ARG5& a5, const ARG6& a6, const ARG7& a7, const ARG8& a8)
    {
        pycall_arg_t args(8);
        args.add(a1).add(a2).add(a3).add(a4).add(a5).add(a6).add(a7).add(a8);
        pytype_tool_impl_t<RET_V> pyret;
        return pycall_t::call<RET_V>(mod_name_, func_, args, pyret);
    }
    template<typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7,
             typename ARG8, typename ARG9>
    RET_V call(const string& mod_name_, const string& func_, const ARG1& a1, const ARG2& a2, const ARG3& a3, const ARG4& a4,
        const ARG5& a5, const ARG6& a6, const ARG7& a7, const ARG8& a8, const ARG9& a9)
    {
        pycall_arg_t args(9);
        args.add(a1).add(a2).add(a3).add(a4).add(a5).add(a6).add(a7).add(a8).add(a9);
        pytype_tool_impl_t<RET_V> pyret;
        return pycall_t::call<RET_V>(mod_name_, func_, args, pyret);
    }
    //!call python class method begin******************************************************************
    template<typename RET>
    RET_V obj_call(PyObject* pobj, const string& func_)
    {
        pycall_arg_t args(0);
        pytype_tool_impl_t<RET_V> pyret;
        return pycall_t::call_obj_method<RET_V>(pobj, func_, args, pyret);
    }
    template<typename RET, typename ARG1>
    RET_V obj_call(PyObject* pobj, const string& func_, const ARG1& a1)
    {
        pycall_arg_t args(1);
        args.add(a1);
        pytype_tool_impl_t<RET_V> pyret;
        return pycall_t::call_obj_method<RET_V>(pobj, func_, args, pyret);
    }
    template<typename RET, typename ARG1, typename ARG2>
    RET_V obj_call(PyObject* pobj, const string& func_, const ARG1& a1, const ARG2& a2)
    {
        pycall_arg_t args(2);
        args.add(a1).add(a2);
        pytype_tool_impl_t<RET_V> pyret;
        return pycall_t::call_obj_method<RET_V>(pobj, func_, args, pyret);
    }
    template<typename RET, typename ARG1, typename ARG2, typename ARG3>
    RET_V obj_call(PyObject* pobj, const string& func_, const ARG1& a1, const ARG2& a2, const ARG3& a3)
    {
        pycall_arg_t args(3);
        args.add(a1).add(a2).add(a3);
        pytype_tool_impl_t<RET_V> pyret;
        return pycall_t::call_obj_method<RET_V>(pobj, func_, args, pyret);
    }
    template<typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4>
    RET_V obj_call(PyObject* pobj, const string& func_, const ARG1& a1, const ARG2& a2, const ARG3& a3, const ARG4& a4)
    {
        pycall_arg_t args(4);
        args.add(a1).add(a2).add(a3).add(a4);
        pytype_tool_impl_t<RET_V> pyret;
        return pycall_t::call_obj_method<RET_V>(pobj, func_, args, pyret);
    }
    template<typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5>
    RET_V obj_call(PyObject* pobj, const string& func_, const ARG1& a1, const ARG2& a2, const ARG3& a3, const ARG4& a4,
                const ARG5& a5)
    {
        pycall_arg_t args(5);
        args.add(a1).add(a2).add(a3).add(a4).add(a5);
        pytype_tool_impl_t<RET_V> pyret;
        return pycall_t::call_obj_method<RET_V>(pobj, func_, args, pyret);
    }
    template<typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6>
    RET_V obj_call(PyObject* pobj, const string& func_, const ARG1& a1, const ARG2& a2, const ARG3& a3, const ARG4& a4,
                const ARG5& a5,const ARG6& a6)
    {
        pycall_arg_t args(6);
        args.add(a1).add(a2).add(a3).add(a4).add(a5).add(a6);
        pytype_tool_impl_t<RET_V> pyret;
        return pycall_t::call_obj_method<RET_V>(pobj, func_, args, pyret);
    }
    template<typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7>
    RET_V obj_call(PyObject* pobj, const string& func_, const ARG1& a1, const ARG2& a2, const ARG3& a3, const ARG4& a4,
                const ARG5& a5,const ARG6& a6,const ARG7& a7)
    {
        pycall_arg_t args(7);
        args.add(a1).add(a2).add(a3).add(a4).add(a5).add(a6).add(a7);
        pytype_tool_impl_t<RET_V> pyret;
        return pycall_t::call_obj_method<RET_V>(pobj, func_, args, pyret);
    }
    template<typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7,
             typename ARG8>
    RET_V obj_call(PyObject* pobj, const string& func_, const ARG1& a1, const ARG2& a2, const ARG3& a3, const ARG4& a4,
                const ARG5& a5, const ARG6& a6, const ARG7& a7, const ARG8& a8)
    {
        pycall_arg_t args(8);
        args.add(a1).add(a2).add(a3).add(a4).add(a5).add(a6).add(a7).add(a8);
        pytype_tool_impl_t<RET_V> pyret;
        return pycall_t::call_obj_method<RET_V>(pobj, func_, args, pyret);
    }
    template<typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7,
             typename ARG8, typename ARG9>
    RET_V obj_call(PyObject* pobj, const string& func_, const ARG1& a1, const ARG2& a2, const ARG3& a3, const ARG4& a4,
        const ARG5& a5, const ARG6& a6, const ARG7& a7, const ARG8& a8, const ARG9& a9)
    {
        pycall_arg_t args(9);
        args.add(a1).add(a2).add(a3).add(a4).add(a5).add(a6).add(a7).add(a8).add(a9);
        pytype_tool_impl_t<RET_V> pyret;
        return pycall_t::call_obj_method<RET_V>(pobj, func_, args, pyret);
    }
    //!call python class method end******************************************************************
    
    //!call python lambad function begin ############################################################
    template<typename RET>
    RET_V call_lambda(PyObject* pobj)
    {
        pycall_arg_t args(0);
        pytype_tool_impl_t<RET_V> pyret;
        return pycall_t::call_lambda<RET_V>(pobj, args, pyret);
    }
    template<typename RET, typename ARG1>
    RET_V call_lambda(PyObject* pobj, const ARG1& a1)
    {
        pycall_arg_t args(1);
        args.add(a1);
        pytype_tool_impl_t<RET_V> pyret;
        return pycall_t::call_lambda<RET_V>(pobj, args, pyret);
    }
    template<typename RET, typename ARG1, typename ARG2>
    RET_V call_lambda(PyObject* pobj, const ARG1& a1, const ARG2& a2)
    {
        pycall_arg_t args(2);
        args.add(a1).add(a2);
        pytype_tool_impl_t<RET_V> pyret;
        return pycall_t::call_lambda<RET_V>(pobj, args, pyret);
    }
    template<typename RET, typename ARG1, typename ARG2, typename ARG3>
    RET_V call_lambda(PyObject* pobj, const ARG1& a1, const ARG2& a2, const ARG3& a3)
    {
        pycall_arg_t args(3);
        args.add(a1).add(a2).add(a3);
        pytype_tool_impl_t<RET_V> pyret;
        return pycall_t::call_lambda<RET_V>(pobj, args, pyret);
    }
    template<typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4>
    RET_V call_lambda(PyObject* pobj, const ARG1& a1, const ARG2& a2, const ARG3& a3, const ARG4& a4)
    {
        pycall_arg_t args(4);
        args.add(a1).add(a2).add(a3).add(a4);
        pytype_tool_impl_t<RET_V> pyret;
        return pycall_t::call_lambda<RET_V>(pobj, args, pyret);
    }
    template<typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5>
    RET_V call_lambda(PyObject* pobj, const ARG1& a1, const ARG2& a2, const ARG3& a3, const ARG4& a4,
                const ARG5& a5)
    {
        pycall_arg_t args(5);
        args.add(a1).add(a2).add(a3).add(a4).add(a5);
        pytype_tool_impl_t<RET_V> pyret;
        return pycall_t::call_lambda<RET_V>(pobj, args, pyret);
    }
    template<typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6>
    RET_V call_lambda(PyObject* pobj, const ARG1& a1, const ARG2& a2, const ARG3& a3, const ARG4& a4,
                const ARG5& a5,const ARG6& a6)
    {
        pycall_arg_t args(6);
        args.add(a1).add(a2).add(a3).add(a4).add(a5).add(a6);
        pytype_tool_impl_t<RET_V> pyret;
        return pycall_t::call_lambda<RET_V>(pobj, args, pyret);
    }
    template<typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7>
    RET_V call_lambda(PyObject* pobj, const ARG1& a1, const ARG2& a2, const ARG3& a3, const ARG4& a4,
                const ARG5& a5,const ARG6& a6,const ARG7& a7)
    {
        pycall_arg_t args(7);
        args.add(a1).add(a2).add(a3).add(a4).add(a5).add(a6).add(a7);
        pytype_tool_impl_t<RET_V> pyret;
        return pycall_t::call_lambda<RET_V>(pobj, args, pyret);
    }
    template<typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7,
             typename ARG8>
    RET_V call_lambda(PyObject* pobj, const ARG1& a1, const ARG2& a2, const ARG3& a3, const ARG4& a4,
                const ARG5& a5, const ARG6& a6, const ARG7& a7, const ARG8& a8)
    {
        pycall_arg_t args(8);
        args.add(a1).add(a2).add(a3).add(a4).add(a5).add(a6).add(a7).add(a8);
        pytype_tool_impl_t<RET_V> pyret;
        return pycall_t::call_lambda<RET_V>(pobj, args, pyret);
    }
    template<typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7,
             typename ARG8, typename ARG9>
    RET_V call_lambda(PyObject* pobj, const string& func_, const ARG1& a1, const ARG2& a2, const ARG3& a3, const ARG4& a4,
        const ARG5& a5, const ARG6& a6, const ARG7& a7, const ARG8& a8, const ARG9& a9)
    {
        pycall_arg_t args(9);
        args.add(a1).add(a2).add(a3).add(a4).add(a5).add(a6).add(a7).add(a8).add(a9);
        pytype_tool_impl_t<RET_V> pyret;
        return pycall_t::call_lambda<RET_V>(pobj, args, pyret);
    }
    //!call python lambad function ennd ############################################################
    template<typename RET>
    RET_V get_global_var(const string& mod_name_, const string& var_name_)
    {
        PyObject *pName = NULL, *pModule = NULL;
        string err_msg;

        pName   = PyString_FromString(mod_name_.c_str());
        pModule = PyImport_Import(pName);
        Py_DECREF(pName);
        if (NULL == pModule)
        {
            pyops_t::traceback(err_msg);
            throw runtime_error(err_msg.c_str());
        }

        pytype_tool_impl_t<RET_V> pyret;
        PyObject *pvalue = PyObject_GetAttrString(pModule, var_name_.c_str());
        Py_DECREF(pModule);

        if (!pvalue)
        {
            pyops_t::traceback(err_msg);
            throw runtime_error(err_msg.c_str());
        }

        if (pyret.parse_value(pvalue))
        {
            Py_XDECREF(pvalue);
            throw runtime_error("type invalid");
        }
        Py_XDECREF(pvalue);
        return pyret.get_value();
    }

    template<typename T>
    int set_global_var(const string& mod_name_, const string& var_name_, T val_)
    {
        PyObject *pName = NULL, *pModule = NULL;
        string err_msg;

        pName   = PyString_FromString(mod_name_.c_str());
        pModule = PyImport_Import(pName);
        Py_DECREF(pName);
        if (NULL == pModule)
        {
            pyops_t::traceback(err_msg);
            throw runtime_error(err_msg.c_str());
        }

        PyObject* pval = pytype_traits_t<T>::pyobj_from_cppobj(val_);
        int ret = PyObject_SetAttrString(pModule, var_name_.c_str(), pval);
        Py_DECREF(pModule);

        return ret != -1? 0: -1;
    }
    
    void cache_pyobject(PyObject* pobj)
    {
        m_cache_pyobject.push_back(pobj);
    }
    void clear_cache_pyobject()
    {
        if (Py_IsInitialized())
        {
            for (size_t i = 0; i < m_cache_pyobject.size(); ++i)
            {
                Py_XDECREF(m_cache_pyobject[i]);
            }
            m_cache_pyobject.clear();
        }
    }
private:
    PyObject* init_method();
    int init_pyclass(PyObject* m);

    bool is_method_exist(const vector<pyclass_regigster_tool_t::method_info_t>& src_, const string& new_);
    bool is_property_exist(const vector<pyclass_regigster_tool_t::property_info_t>& src_, const string& new_);
    pyclass_regigster_tool_t* get_pyclass_info_by_name(const string& name_);


private:
    string                              m_mod_name;
    string                              m_mod_doc;
    vector<PyMethodDef>                 m_pymethod_defs;
    vector<reg_info_t>                  m_func_info;
    
    //! reg class
    vector<pyclass_regigster_tool_t>    m_all_pyclass;
    //! cache some pyobject for optimize
    vector<PyObject*>                   m_cache_pyobject;
};

template<typename T>
struct type_ref_traits_t
{
    typedef T        value_t;
    typedef T&        ref_t;
    value_t                value; 
};
template<typename T>
struct type_ref_traits_t<T&>
{
    typedef T        value_t;
    typedef T&        ref_t;
    value_t                value; 
};
template<typename T>
struct type_ref_traits_t<const T&>
{
    typedef T        value_t;
    typedef T&        ref_t;
    value_t                value;
};
//! �����ж��Ƿ��ǿ�ѡ����
template<typename T>
struct pyoption_traits_t
{
    static int is() { return 0;}
};
template<typename T>
struct pyoption_traits_t<pyoption_t<T> >
{
    static int is() { return 1;}
};


//! pytype_traits_t ��װ PyLong_FromLong ���صĲ���������Ϊ����python���ɲ���

template<>//typename T>
struct pytype_traits_t<long>
{
    static PyObject* pyobj_from_cppobj(const long& val_)
    {
        return PyLong_FromLong(long(val_));
    }
    static int pyobj_to_cppobj(PyObject *pvalue_, long& m_ret)
    {
        if (true == PyLong_Check(pvalue_))
        {
            m_ret = (long)PyLong_AsLong(pvalue_);
            return 0;
        }
        else if (true == PyInt_Check(pvalue_))
        {
            m_ret = (long)PyInt_AsLong(pvalue_);
            return 0;
        }
        return -1;
    }
    static const char* get_typename() { return "long";}
};

#define  IMPL_INT_CODE(X) \
template<> \
struct pytype_traits_t<X> \
{ \
    static PyObject* pyobj_from_cppobj(const X& val_) \
    { \
        return PyInt_FromLong(long(val_)); \
    } \
    static int pyobj_to_cppobj(PyObject *pvalue_, X& m_ret) \
    { \
        if (true == PyLong_Check(pvalue_)) \
        { \
            m_ret = (X)PyLong_AsLong(pvalue_); \
            return 0; \
        } \
        else if (true == PyInt_Check(pvalue_)) \
        { \
            m_ret = (X)PyInt_AsLong(pvalue_); \
            return 0; \
        } \
        return -1; \
    } \
    static const char* get_typename() { return #X;} \
};

IMPL_INT_CODE(int)
IMPL_INT_CODE(unsigned int)
IMPL_INT_CODE(short)
IMPL_INT_CODE(unsigned short)
IMPL_INT_CODE(char)
IMPL_INT_CODE(unsigned char)

#ifdef _WIN32
IMPL_INT_CODE(unsigned long)
#else
#ifndef __x86_64__
IMPL_INT_CODE(int64_t)
#endif
IMPL_INT_CODE(uint64_t)
#endif

template<typename T>
struct pytype_traits_t<const T*>
{
    static PyObject* pyobj_from_cppobj(const T* val_)
    {
        const string& mod_name = pyclass_base_info_t<T>::pytype_info.mod_name;
        const string& class_name = pyclass_base_info_t<T>::pytype_info.class_name;
        PyObject *pName = NULL, *pModule = NULL, *pValue = NULL;

        if (class_name.empty())
            return pytype_traits_t<long>::pyobj_from_cppobj(long(val_));

        pName   = PyString_FromString(mod_name.c_str());
        pModule = PyImport_Import(pName);
        Py_DECREF(pName);
        if (NULL == pModule)
        {
            if (PyErr_Occurred())
                PyErr_Print();
            fprintf(stderr, "Failed to load \"%s\"\n", mod_name.c_str());
            assert(NULL && "this can not be happened");
            return NULL;
        }
        PyObject *pyclass = PyObject_GetAttrString(pModule, class_name.c_str());
        if (pyclass && PyCallable_Check(pyclass)) {
            PyObject *pArgs = PyTuple_New(pyclass_base_info_t<T>::pytype_info.total_args_num+1);
            for (int i = 0; i< pyclass_base_info_t<T>::pytype_info.total_args_num; ++i)
            {
                PyTuple_SetItem(pArgs, i, pytype_traits_t<int>::pyobj_from_cppobj(0));
            }

            PyObject *palloc = pyclass_base_info_t<T>::alloc_obj(pyclass_base_info_t<T>::pytype_info.pytype_def, NULL, NULL);
            typename pyclass_base_info_t<T>::obj_data_t* pdest_obj = (typename pyclass_base_info_t<T>::obj_data_t*)palloc;
            //pdest_obj->obj = val_;
            ::memcpy(&pdest_obj->obj, &val_, sizeof(pdest_obj->obj));
            pdest_obj->disable_auto_release();
            PyTuple_SetItem(pArgs, pyclass_base_info_t<T>::pytype_info.total_args_num, palloc);
            pValue = PyObject_CallObject(pyclass, pArgs);
        }

        Py_XDECREF(pyclass);
        Py_DECREF(pModule);
        return pValue;
    }

    static int pyobj_to_cppobj(PyObject *pvalue_, T*& m_ret)
    {
        PyObject *pysrc = PyObject_GetAttrString(pvalue_, "obj");
        //!PyObject_TypeCheck(pysrc, pyclass_base_info_t<T>::pytype_info.pytype_def)) {
        if (NULL == pysrc || false == cpp_to_pyclass_reg_info_t::is_instance(pysrc, pyclass_base_info_t<T>::pytype_info.class_name))
        {
            Py_XDECREF(pysrc);
            return -1;
        }
        typename pyclass_base_info_t<T>::obj_data_t* pdest_obj = (typename pyclass_base_info_t<T>::obj_data_t*)pysrc;

        m_ret = pdest_obj->obj;
        Py_XDECREF(pysrc);
        return 0;
    }

    static const char* get_typename() { return pyclass_base_info_t<T>::pytype_info.class_name.c_str();}
};

template<typename T>
struct pytype_traits_t<T*>
{
    static PyObject* pyobj_from_cppobj(T* val_)
    {
        return pytype_traits_t<const T*>::pyobj_from_cppobj(val_);
    }
    static int pyobj_to_cppobj(PyObject *pvalue_,T*& m_ret)
    {
        return pytype_traits_t<const T*>::pyobj_to_cppobj(pvalue_, m_ret);
    }
    static const char* get_typename() { return pyclass_base_info_t<T>::pytype_info.class_name.c_str();}
};

template<>
struct pytype_traits_t<PyObject*>
{
    static PyObject* pyobj_from_cppobj(PyObject* val_)
    {
        return val_;
    }
    static int pyobj_to_cppobj(PyObject *pvalue_, PyObject*& m_ret)
    {
        m_ret = pvalue_;
        return 0;
    }
    static const char* get_typename() { return "PyObject";}
};

template<>
struct pytype_traits_t<const char*>
{
    static PyObject* pyobj_from_cppobj(const char*& val_)
    {
        return PyString_FromString(val_);
    }
    /*
    static int pyobj_to_cppobj(PyObject *pvalue_, char*& m_ret)
    {
        if (true == PyString_Check(pvalue_))
        {
            m_ret = PyString_AsString(pvalue_);
            return 0;
        }
        return -1;
    }
    */
    static const char* get_typename() { return "string";}
};

template<>
struct pytype_traits_t<char*>
{
    static PyObject* pyobj_from_cppobj(const char*& val_)
    {
        return PyString_FromString(val_);
    }
    /*
    static int pyobj_to_cppobj(PyObject *pvalue_, char*& m_ret)
    {
        if (true == PyString_Check(pvalue_))
        {
            m_ret = PyString_AsString(pvalue_);
            return 0;
        }
        return -1;
    }
    */
    static const char* get_typename() { return "string";}
};

template<>
struct pytype_traits_t<bool>
{
    static PyObject* pyobj_from_cppobj(bool val_)
    {
        if (val_)
        {
            Py_RETURN_TRUE;
        }
        Py_RETURN_FALSE;
    }
    static int pyobj_to_cppobj(PyObject *pvalue_, bool& m_ret)
    {
        if (Py_False ==  pvalue_|| Py_None == pvalue_)
        {
            m_ret = false;
        }
        else
        {
            m_ret = true;
        }
        return 0;
    }
    static const char* get_typename() { return "bool";}
};

template<typename T>
struct pytype_traits_t<pyoption_t<T> >
{
    static int pyobj_to_cppobj(PyObject *pvalue_, pyoption_t<T>& m_ret)
    {
        if (Py_None == pvalue_)
        {
            return 0;
        }
        else if (0 == pytype_traits_t<typename pyoption_t<T>::value_t>::pyobj_to_cppobj(pvalue_, m_ret.value()))
        {
            m_ret.set();
            return 0;
        }
        return -1;
    }
    static const char* get_typename() { return "pyoption_t";}
};

template<>
struct pytype_traits_t<string>
{
    static PyObject* pyobj_from_cppobj(const string& val_)
    {
        return PyString_FromStringAndSize(val_.c_str(), val_.size());
    }
    static int pyobj_to_cppobj(PyObject *pvalue_, string& m_ret)
    {
        if (true == PyString_Check(pvalue_))
        {
            char* pDest = NULL;
            Py_ssize_t  nLen    = 0;
            PyString_AsStringAndSize(pvalue_, &pDest, &nLen);
            
            m_ret.assign(pDest, nLen);
            return 0;
        }
        else if (true == PyUnicode_Check(pvalue_))
        {
            char* pDest = NULL;
            Py_ssize_t  nLen    = 0;
            PyString_AsStringAndSize(pvalue_, &pDest, &nLen);
            m_ret.assign(pDest, nLen);
            return 0;
            /*
#ifdef _WIN32
            PyObject* retStr = PyUnicode_AsEncodedString(pvalue_, "gbk", "");
#else
            PyObject* retStr = PyUnicode_AsUTF8String(pvalue_);
#endif
            if (retStr)
            {
                m_ret = PyString_AsString(retStr);
                Py_XDECREF(retStr);
                return 0;
            }
            */
        }
        return -1;
    }
    static const char* get_typename() { return "string";}
};

#ifdef _WIN32
template<>
struct pytype_traits_t<wstring>
{
    static PyObject* pyobj_from_cppobj(const wstring& wstr)
    {
		return PyUnicode_FromWideChar(wstr.c_str(), wstr.length());
    }
    static int pyobj_to_cppobj(PyObject *pvalue_, wstring& wstr_ret)
    {
		//wstr_ret.reserve(10000);
		//PyUnicode_AsWideChar(pvalue_,(wchar_t *) wstr_ret.data(), 100);
		//return 0;
		string m_ret;
        if (true == PyString_Check(pvalue_))
        {
			PyObject* retStr = PyUnicode_FromObject(pvalue_);
			if (retStr)
			{
				int n = PyUnicode_GetSize(retStr);
				wstr_ret.resize(n);
				n = PyUnicode_AsWideChar((PyUnicodeObject*)retStr, &(wstr_ret[0]), n);
				Py_XDECREF(retStr);
				return 0;
			}
            return 0;
        }
        else if (true == PyUnicode_Check(pvalue_))
        {
			int n = PyUnicode_GetSize(pvalue_);
			wstr_ret.resize(n);
			n = PyUnicode_AsWideChar((PyUnicodeObject*)pvalue_, &(wstr_ret[0]), n);
			return 0;
        }
        return -1;
    }
    static const char* get_typename() { return "wstring";}
};
#endif

template<>
struct pytype_traits_t<float>
{
    static PyObject* pyobj_from_cppobj(float val_)
    {
        return PyFloat_FromDouble(double(val_));
    }
    static int pyobj_to_cppobj(PyObject *pvalue_, float& m_ret)
    {
        if (true == PyFloat_Check(pvalue_))
        {
            m_ret = (float)PyFloat_AsDouble(pvalue_);
            return 0;
        }
        return -1;
    }
    static const char* get_typename() { return "float";}
};

template<>
struct pytype_traits_t<double>
{
    static PyObject* pyobj_from_cppobj(double val_)
    {
        return PyFloat_FromDouble(val_);
    }
    static int pyobj_to_cppobj(PyObject *pvalue_, double& m_ret)
    {
        if (true == PyFloat_Check(pvalue_))
        {
            m_ret = PyFloat_AsDouble(pvalue_);
            return 0;
        }
        return -1;
    }
    static const char* get_typename() { return "double";}
};

template<typename T>
struct pytype_traits_t<vector<T> >
{
    static PyObject* pyobj_from_cppobj(const vector<T>& val_)
    {
        PyObject* ret = PyList_New(val_.size());
        for (size_t i = 0; i < val_.size(); ++i)
        {
            PyList_SetItem(ret, i, pytype_traits_t<T>::pyobj_from_cppobj(val_[i]));
        }
        return ret;
    }
    static int pyobj_to_cppobj(PyObject *pvalue_, vector<T>& m_ret)
    {
        m_ret.clear();
        if (true == PyTuple_Check(pvalue_))
        {
            int n = PyTuple_Size(pvalue_);
            for (int i = 0; i < n; ++i)
            {
                pytype_tool_impl_t<T> ret_tool;
                if (ret_tool.parse_value(PyTuple_GetItem(pvalue_, i)))
                {
                    return -1;
                }
                m_ret.push_back(ret_tool.get_value());
            }
            return 0;
        }
        else if (true == PyList_Check(pvalue_))
        {
            int n = PyList_Size(pvalue_);
            for (int i = 0; i < n; ++i)
            {
                pytype_tool_impl_t<T> ret_tool;
                if (ret_tool.parse_value(PyList_GetItem(pvalue_, i)))
                {
                    return -1;
                }
                m_ret.push_back(ret_tool.get_value());
            }
            return 0;
        }
        return -1;
    }
    static const char* get_typename() { return "vector";}
};
template<typename T>
struct pytype_traits_t<list<T> >
{
    static PyObject* pyobj_from_cppobj(const list<T>& val_)
    {
        size_t n = val_.size();
        PyObject* ret = PyList_New(n);
        int i = 0;
        for (typename list<T>::const_iterator it = val_.begin(); it != val_.end(); ++it)
        {
            PyList_SetItem(ret, i++, pytype_traits_t<T>::pyobj_from_cppobj(*it));
        }
        return ret;
    }
    static int pyobj_to_cppobj(PyObject *pvalue_, list<T>& m_ret)
    {
        pytype_tool_impl_t<T> ret_tool;
        if (true == PyTuple_Check(pvalue_))
        {
            int n = PyTuple_Size(pvalue_);
            for (int i = 0; i < n; ++i)
            {
                pytype_tool_impl_t<T> ret_tool;
                if (ret_tool.parse_value(PyTuple_GetItem(pvalue_, i)))
                {
                    return -1;
                }
                m_ret.push_back(ret_tool.get_value());
            }
            return 0;
        }
        else if (true == PyList_Check(pvalue_))
        {
            int n = PyList_Size(pvalue_);
            for (int i = 0; i < n; ++i)
            {
                pytype_tool_impl_t<T> ret_tool;
                if (ret_tool.parse_value(PyList_GetItem(pvalue_, i)))
                {
                    return -1;
                }
                m_ret.push_back(ret_tool.get_value());
            }
            return 0;
        }
        return -1;
    }
    static const char* get_typename() { return "list";}
};
template<typename T>
struct pytype_traits_t<set<T> >
{
    static PyObject* pyobj_from_cppobj(const set<T>& val_)
    {
        PyObject* ret = PySet_New(NULL);
        for (typename set<T>::const_iterator it = val_.begin(); it != val_.end(); ++it)
        {
            PyObject *v = pytype_traits_t<T>::pyobj_from_cppobj(*it);
            PySet_Add(ret, v);
            Py_DECREF(v);
        }
        return ret;
    }
    static int pyobj_to_cppobj(PyObject *pvalue_, set<T>& m_ret)
    {
        m_ret.clear();
        pytype_tool_impl_t<T> ret_tool;
        PyObject *iter = PyObject_GetIter(pvalue_);
        PyObject *item = NULL;
        while (NULL != iter && NULL != (item = PyIter_Next(iter)))
        {
            T tmp();
            if (pytype_traits_t::pyobj_to_cppobj(item, tmp))
            {
                Py_DECREF(item);
                Py_DECREF(iter);
                return -1;
            }
            m_ret.insert(tmp);
            Py_DECREF(item);
        }
        if (iter)
        {
            Py_DECREF(iter);
            return 0;
        }
        return -1;
    }
    static const char* get_typename() { return "set";}
};
template<typename T, typename R>
struct pytype_traits_t<map<T, R> >
{
    static PyObject* pyobj_from_cppobj(const map<T, R>& val_)
    {
        PyObject* ret = PyDict_New();
        for (typename map<T, R>::const_iterator it = val_.begin(); it != val_.end(); ++it)
        {
            PyObject *k = pytype_traits_t<T>::pyobj_from_cppobj(it->first);
            PyObject *v = pytype_traits_t<R>::pyobj_from_cppobj(it->second);
            PyDict_SetItem(ret, k, v);
            Py_DECREF(k);
            Py_DECREF(v);
        }
        return ret;
    }
    static int pyobj_to_cppobj(PyObject *pvalue_, map<T, R>& m_ret)
    {
        m_ret.clear();
        pytype_tool_impl_t<T> ret_tool_T;
        pytype_tool_impl_t<R> ret_tool_R;
        if (true == PyDict_Check(pvalue_))
        {
            PyObject *key = NULL, *value = NULL;
            Py_ssize_t pos = 0;

            while (PyDict_Next(pvalue_, &pos, &key, &value))
            {
                T tmp_key;
                R tmp_value;
                if (pytype_traits_t<T>::pyobj_to_cppobj(key, tmp_key) ||
                    pytype_traits_t<R>::pyobj_to_cppobj(value, tmp_value))
                {
                    return -1;
                }
                m_ret[tmp_key] = tmp_value;
            }
            return 0;
        }
        return -1;
    }
    static const char* get_typename() { return "map";}
};

//! ��ȡpython�����ķ���ֵ,������
template<typename T>
class pytype_tool_impl_t: public pytype_tool_t
{
public:
    pytype_tool_impl_t():m_ret(){}

    virtual int parse_value(PyObject *pvalue_)
    {
        if (pytype_traits_t<T>::pyobj_to_cppobj(pvalue_, m_ret))
        {
            return -1;
        }
        return 0;
    }

    const T& get_value() const { return m_ret; }
    virtual const char* return_type() {return pytype_traits_t<T>::get_typename();}
private:
    T    m_ret;
};

template<>
class pytype_tool_impl_t<cpp_void_t>: public pytype_tool_t
{
public:
    pytype_tool_impl_t():m_ret(){}

    virtual int parse_value(PyObject *pvalue_)
    {
        return 0;
    }

    const cpp_void_t& get_value() const { return m_ret; }
    virtual const char* return_type() { return "void";}
private:
    cpp_void_t    m_ret;
};
template<typename T>
class pytype_tool_impl_t<const T*>: public pytype_tool_t
{
public:
    pytype_tool_impl_t():m_ret(){}

    virtual int parse_value(PyObject *pvalue_)
    {
        if (pytype_traits_t<T*>::pyobj_to_cppobj(pvalue_, m_ret))
        {
            return -1;
        }
        return 0;
    }

    T* get_value() const { return m_ret; }
private:
    T*    m_ret;
};


template<typename T>
const T& pycall_t::call(const string& mod_name_, const string& func_name_, pycall_arg_t& pyarg_, pytype_tool_impl_t<T>& pyret)
{
    PyObject *pName = NULL, *pModule = NULL;
    string err_msg;

    pName   = PyString_FromString(mod_name_.c_str());
    pModule = PyImport_Import(pName);
    Py_DECREF(pName);
    if (NULL == pModule)
    {
        pyops_t::traceback(err_msg);
        throw runtime_error(err_msg.c_str());
        return pyret.get_value();
    }

    call_func(pModule, mod_name_, func_name_, pyarg_, pyret, err_msg);
    Py_DECREF(pModule);

    if (!err_msg.empty())
    {
        throw runtime_error(err_msg.c_str());
    }
    return pyret.get_value();
}
template<typename T>
const T& pycall_t::call_obj_method(PyObject *pObj, const string& func_name_, pycall_arg_t& pyarg_, pytype_tool_impl_t<T>& pyret)
{
    string err_msg;
    if (NULL == pObj)
    {
        pyops_t::traceback(err_msg);
        throw runtime_error(err_msg.c_str());
        return pyret.get_value();
    }

    static string mod_name_ = "NaN";
    call_func(pObj, mod_name_, func_name_, pyarg_, pyret, err_msg);

    if (!err_msg.empty())
    {
        throw runtime_error(err_msg.c_str());
    }
    return pyret.get_value();
}

template<typename T>
const T& pycall_t::call_lambda(PyObject *pFunc, pycall_arg_t& pyarg_, pytype_tool_impl_t<T>& pyret)
{
    string err_msg;
    if (NULL == pFunc)
    {
        err_msg = "can not call null PyObject";
        throw runtime_error(err_msg.c_str());
        return pyret.get_value();
    }

    call_func_obj(pFunc, pyarg_, pyret, err_msg);

    if (!err_msg.empty())
    {
        throw runtime_error(err_msg.c_str());
    }
    return pyret.get_value();
}

    
//! ������չpython�Ĺ����࣬������������
struct pyext_tool_t
{
    pyext_tool_t(PyObject* args_):
        m_args(args_),
        m_arg_tuple(NULL),
        m_index(0),
        m_err(false),
        m_func_addr(0)
    {
        if (!PyArg_ParseTuple(args_, "lO", &m_func_addr, &m_arg_tuple)) {
            m_err = true;
            return;
        }
        if (NULL == m_arg_tuple || false == PyTuple_Check(m_arg_tuple))
        {
            PyErr_SetString(PyExc_TypeError, "arg type invalid(shoule func_name, args tuple)");
            m_err = true;
            return;
        }
        m_size = PyTuple_Size(m_arg_tuple);
    }

    template<typename T>
    pyext_tool_t& parse_arg(T& ret_arg_)
    {
        typedef typename type_ref_traits_t<T>::value_t value_t;
        if (false == m_err)
        {
            if (m_index >= m_size)
            {
                stringstream ss;
                ss << "param num invalid, only["<< m_index + 1 <<"] provided";
                PyErr_SetString(PyExc_TypeError, ss.str().c_str());
                m_err = true;
                return *this;
            }

            pytype_tool_impl_t<T> ret_tool;
            if (ret_tool.parse_value(PyTuple_GetItem(m_arg_tuple, m_index)))
            {
                stringstream ss;
                ss << "param[" << m_index + 1 << "] type invalid, "<< pytype_traits_t<T>::get_typename() << " needed";
                PyErr_SetString(PyExc_TypeError, ss.str().c_str());
                m_err = true;
                return *this;
            }
            ++m_index;
            ret_arg_ = ret_tool.get_value();
        }
        return *this;
    }

    bool is_err() const { return m_err;}
    long get_func_addr() const { return m_func_addr;}

    template<typename FUNC>
    FUNC get_func_ptr() const 
    {
        FUNC f = NULL;
        ::memcpy(&f, &m_func_addr, sizeof(m_func_addr));
        return f;
    }
    PyObject* m_args;
    PyObject* m_arg_tuple;
    int       m_index;
    int       m_size;
    bool      m_err;//! �Ƿ��쳣
    long      m_func_addr;
};


//! ������չpython������pyobject���͵ķ���ֵ��python
template<typename T>
struct pyext_return_tool_t
{
   //! ���ھ�̬����
    template<typename F>
    static PyObject* route_call(F f)
    {
        return pytype_traits_t<T>::pyobj_from_cppobj(f());
    }
    template<typename F, typename ARG1>
    static PyObject* route_call(F f, ARG1& a1)
    {
        return pytype_traits_t<T>::pyobj_from_cppobj(f(a1.value));
    }
    template<typename F, typename ARG1, typename ARG2>
    static PyObject* route_call(F f, ARG1& a1, ARG2& a2)
    {
        return pytype_traits_t<T>::pyobj_from_cppobj(f(a1.value, a2.value));
    }
    template<typename F, typename ARG1, typename ARG2, typename ARG3>
    static PyObject* route_call(F f, ARG1& a1, ARG2& a2, ARG3& a3)
    {
        return pytype_traits_t<T>::pyobj_from_cppobj(f(a1.value, a2.value, a3.value));
    }
    template<typename F, typename ARG1, typename ARG2, typename ARG3, typename ARG4>
    static PyObject* route_call(F f, ARG1& a1, ARG2& a2, ARG3& a3, ARG4& a4)
    {
        return pytype_traits_t<T>::pyobj_from_cppobj(f(a1.value, a2.value, a3.value, a4.value));
    }
    template<typename F, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5>
    static PyObject* route_call(F f, ARG1& a1, ARG2& a2, ARG3& a3, ARG4& a4, ARG5& a5)
    {
        return pytype_traits_t<T>::pyobj_from_cppobj(f(a1.value, a2.value, a3.value, a4.value, a5.value));
    }
    template<typename F, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6>
    static PyObject* route_call(F f, ARG1& a1, ARG2& a2, ARG3& a3, ARG4& a4, ARG5& a5, ARG6& a6)
    {
        return pytype_traits_t<T>::pyobj_from_cppobj(f(a1.value, a2.value, a3.value, a4.value, a5.value, a6.value));
    }
    template<typename F, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6,
        typename ARG7>
        static PyObject* route_call(F f, ARG1& a1, ARG2& a2, ARG3& a3, ARG4& a4, ARG5& a5, ARG6& a6, ARG7& a7)
    {
        return pytype_traits_t<T>::pyobj_from_cppobj(f(a1.value, a2.value, a3.value, a4.value, a5.value, a6.value,
            a7.value));
    }
    template<typename F, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6,
        typename ARG7, typename ARG8>
        static PyObject* route_call(F f, ARG1& a1, ARG2& a2, ARG3& a3, ARG4& a4, ARG5& a5, ARG6& a6, ARG7& a7, ARG8& a8)
    {
        return pytype_traits_t<T>::pyobj_from_cppobj(f(a1.value, a2.value, a3.value, a4.value, a5.value, a6.value,
            a7.value, a8.value));
    }
    template<typename F, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6,
        typename ARG7, typename ARG8, typename ARG9>
        static PyObject* route_call(F f, ARG1& a1, ARG2& a2, ARG3& a3, ARG4& a4, ARG5& a5, ARG6& a6, ARG7& a7, ARG8& a8, ARG9& a9)
    {
        return pytype_traits_t<T>::pyobj_from_cppobj(f(a1.value, a2.value, a3.value, a4.value, a5.value, a6.value,
            a7.value, a8.value, a9.value));
    }
    //! ���ڳ�Ա����
    template<typename O, typename F>
    static PyObject* route_method_call(O o, F f)
    {
        NULL_PTR_GUARD(o);
        return pytype_traits_t<T>::pyobj_from_cppobj((o->*f)());
    }
    template<typename O, typename F, typename ARG1>
    static PyObject* route_method_call(O o, F f, ARG1& a1)
    {
        NULL_PTR_GUARD(o);
        return pytype_traits_t<T>::pyobj_from_cppobj((o->*f)(a1.value));
    }
    template<typename O, typename F, typename ARG1, typename ARG2>
    static PyObject* route_method_call(O o, F f, ARG1& a1, ARG2& a2)
    {
        NULL_PTR_GUARD(o);
        return pytype_traits_t<T>::pyobj_from_cppobj((o->*f)(a1.value, a2.value));
    }
    template<typename O, typename F, typename ARG1, typename ARG2, typename ARG3>
    static PyObject* route_method_call(O o, F f, ARG1& a1, ARG2& a2, ARG3& a3)
    {
        NULL_PTR_GUARD(o);
        return pytype_traits_t<T>::pyobj_from_cppobj((o->*f)(a1.value, a2.value, a3.value));
    }
    template<typename O, typename F, typename ARG1, typename ARG2, typename ARG3, typename ARG4>
    static PyObject* route_method_call(O o, F f, ARG1& a1, ARG2& a2, ARG3& a3, ARG4& a4)
    {
        NULL_PTR_GUARD(o);
        return pytype_traits_t<T>::pyobj_from_cppobj((o->*f)(a1.value, a2.value, a3.value, a4.value));
    }
    template<typename O, typename F, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5>
    static PyObject* route_method_call(O o, F f, ARG1& a1, ARG2& a2, ARG3& a3, ARG4& a4, ARG5& a5)
    {
        NULL_PTR_GUARD(o);
        return pytype_traits_t<T>::pyobj_from_cppobj((o->*f)(a1.value, a2.value, a3.value, a4.value, a5.value));
    }
    template<typename O, typename F, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6>
    static PyObject* route_method_call(O o, F f, ARG1& a1, ARG2& a2, ARG3& a3, ARG4& a4, ARG5& a5, ARG6& a6)
    {
        NULL_PTR_GUARD(o);
        return pytype_traits_t<T>::pyobj_from_cppobj((o->*f)(a1.value, a2.value, a3.value, a4.value, a5.value, a6.value));
    }
    template<typename O, typename F, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6,
        typename ARG7>
        static PyObject* route_method_call(O o, F f, ARG1& a1, ARG2& a2, ARG3& a3, ARG4& a4, ARG5& a5, ARG6& a6, ARG7& a7)
    {
        NULL_PTR_GUARD(o);
        return pytype_traits_t<T>::pyobj_from_cppobj((o->*f)(a1.value, a2.value, a3.value, a4.value, a5.value, a6.value,
            a7.value));
    }
    template<typename O, typename F, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6,
        typename ARG7, typename ARG8>
        static PyObject* route_method_call(O o, F f, ARG1& a1, ARG2& a2, ARG3& a3, ARG4& a4, ARG5& a5, ARG6& a6, ARG7& a7, ARG8& a8)
    {
        NULL_PTR_GUARD(o);
        return pytype_traits_t<T>::pyobj_from_cppobj((o->*f)(a1.value, a2.value, a3.value, a4.value, a5.value, a6.value,
            a7.value, a8.value));
    }
    template<typename O, typename F, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6,
        typename ARG7, typename ARG8, typename ARG9>
        static PyObject* route_method_call(O o, F f, ARG1& a1, ARG2& a2, ARG3& a3, ARG4& a4, ARG5& a5, ARG6& a6, ARG7& a7, ARG8& a8, ARG9& a9)
    {
        NULL_PTR_GUARD(o);
        return pytype_traits_t<T>::pyobj_from_cppobj((o->*f)(a1.value, a2.value, a3.value, a4.value, a5.value, a6.value,
            a7.value, a8.value, a9.value));
    }
};

template<>
struct pyext_return_tool_t<void>
{
    template<typename F>
    static PyObject* route_call(F f)
    {
        f();
         Py_RETURN_NONE;
    }
    template<typename F, typename ARG1>
    static PyObject* route_call(F f, ARG1& a1)
    {
        f(a1.value);
         Py_RETURN_NONE;
    }
    template<typename F, typename ARG1, typename ARG2>
    static PyObject* route_call(F f, ARG1& a1, ARG2& a2)
    {
        f(a1.value, a2.value);
         Py_RETURN_NONE;
    }
    template<typename F, typename ARG1, typename ARG2, typename ARG3>
    static PyObject* route_call(F f, ARG1& a1, ARG2& a2, ARG3& a3)
    {
        f(a1.value, a2.value, a3.value);
         Py_RETURN_NONE;
    }
    template<typename F, typename ARG1, typename ARG2, typename ARG3, typename ARG4>
    static PyObject* route_call(F f, ARG1& a1, ARG2& a2, ARG3& a3, ARG4& a4)
    {
        f(a1.value, a2.value, a3.value, a4.value);
         Py_RETURN_NONE;
    }
    template<typename F, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5>
    static PyObject* route_call(F f, ARG1& a1, ARG2& a2, ARG3& a3, ARG4& a4, ARG5& a5)
    {
        f(a1.value, a2.value, a3.value, a4.value, a5.value);
         Py_RETURN_NONE;
    }
    template<typename F, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6>
    static PyObject* route_call(F f, ARG1& a1, ARG2& a2, ARG3& a3, ARG4& a4, ARG5& a5, ARG6& a6)
    {
        f(a1.value, a2.value, a3.value, a4.value, a5.value, a6.value);
         Py_RETURN_NONE;
    }
    template<typename F, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7>
    static PyObject* route_call(F f, ARG1& a1, ARG2& a2, ARG3& a3, ARG4& a4, ARG5& a5, ARG6& a6, ARG7& a7)
    {
        f(a1.value, a2.value, a3.value, a4.value, a5.value, a6.value, a7.value);
         Py_RETURN_NONE;
    }
    template<typename F, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7,
        typename ARG8>
        static PyObject* route_call(F f, ARG1& a1, ARG2& a2, ARG3& a3, ARG4& a4, ARG5& a5, ARG6& a6, ARG7& a7, ARG8& a8)
    {
        f(a1.value, a2.value, a3.value, a4.value, a5.value, a6.value, a7.value, a8.value);
         Py_RETURN_NONE;
    }
    template<typename F, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7,
        typename ARG8, typename ARG9>
        static PyObject* route_call(F f, ARG1& a1, ARG2& a2, ARG3& a3, ARG4& a4, ARG5& a5, ARG6& a6, ARG7& a7, ARG8& a8, ARG9& a9)
    {
        f(a1.value, a2.value, a3.value, a4.value, a5.value, a6.value, a7.value, a8.value, a9.value);
         Py_RETURN_NONE;
    }
    template<typename O, typename F>
    static PyObject* route_method_call(O o, F f)
    {
        (o->*f)();
         Py_RETURN_NONE;
    }
    template<typename O, typename F, typename ARG1>
    static PyObject* route_method_call(O o, F f, ARG1& a1)
    {
        (o->*f)(a1.value);
         Py_RETURN_NONE;
    }
    template<typename O, typename F, typename ARG1, typename ARG2>
    static PyObject* route_method_call(O o, F f, ARG1& a1, ARG2& a2)
    {
        (o->*f)(a1.value, a2.value);
         Py_RETURN_NONE;
    }
    template<typename O, typename F, typename ARG1, typename ARG2, typename ARG3>
    static PyObject* route_method_call(O o, F f, ARG1& a1, ARG2& a2, ARG3& a3)
    {
        (o->*f)(a1.value, a2.value, a3.value);
         Py_RETURN_NONE;
    }
    template<typename O, typename F, typename ARG1, typename ARG2, typename ARG3, typename ARG4>
    static PyObject* route_method_call(O o, F f, ARG1& a1, ARG2& a2, ARG3& a3, ARG4& a4)
    {
        (o->*f)(a1.value, a2.value, a3.value, a4.value);
         Py_RETURN_NONE;
    }
    template<typename O, typename F, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5>
    static PyObject* route_method_call(O o, F f, ARG1& a1, ARG2& a2, ARG3& a3, ARG4& a4, ARG5& a5)
    {
        (o->*f)(a1.value, a2.value, a3.value, a4.value, a5.value);
         Py_RETURN_NONE;
    }
    template<typename O, typename F, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6>
    static PyObject* route_method_call(O o, F f, ARG1& a1, ARG2& a2, ARG3& a3, ARG4& a4, ARG5& a5, ARG6& a6)
    {
        (o->*f)(a1.value, a2.value, a3.value, a4.value, a5.value, a6.value);
         Py_RETURN_NONE;
    }
    template<typename O, typename F, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6,
        typename ARG7>
        static PyObject* route_method_call(O o, F f, ARG1& a1, ARG2& a2, ARG3& a3, ARG4& a4, ARG5& a5, ARG6& a6, ARG7& a7)
    {
        (o->*f)(a1.value, a2.value, a3.value, a4.value, a5.value, a6.value, a7.value);
         Py_RETURN_NONE;
    }
    template<typename O, typename F, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6,
        typename ARG7, typename ARG8>
        static PyObject* route_method_call(O o, F f, ARG1& a1, ARG2& a2, ARG3& a3, ARG4& a4, ARG5& a5, ARG6& a6, ARG7& a7, ARG8& a8)
    {
        (o->*f)(a1.value, a2.value, a3.value, a4.value, a5.value, a6.value, a7.value, a8.value);
         Py_RETURN_NONE;
    }
    template<typename O, typename F, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6,
        typename ARG7, typename ARG8, typename ARG9>
        static PyObject* route_method_call(O o, F f, ARG1& a1, ARG2& a2, ARG3& a3, ARG4& a4, ARG5& a5, ARG6& a6, ARG7& a7, ARG8& a8,  ARG9& a9)
    {
        (o->*f)(a1.value, a2.value, a3.value, a4.value, a5.value, a6.value, a7.value, a8.value, a9.value);
         Py_RETURN_NONE;
    }
};


//! ������չpython��traits��ע����python�ĺ����ӿ�
template <typename RET>
struct pyext_func_traits_t<RET (*)()>
{
    typedef RET (*func_t)();
    static int args_num() { return 0;}
    static int option_args_num() { return 0;}
    static PyObject* pyfunc(PyObject* self, PyObject* args)
    {
        pyext_tool_t pyext_tool(args);
        if (pyext_tool.is_err())
        {
            return NULL;
        }
        func_t f = (func_t)pyext_tool.get_func_addr();
        if (0 == f)
        {
            PyErr_SetString(PyExc_TypeError, "func address must provided");
            return NULL;
        }
        return pyext_return_tool_t<RET>::route_call(f);
    }
};

template <typename RET, typename ARG1>
struct pyext_func_traits_t<RET (*)(ARG1)>
{
    typedef RET (*func_t)(ARG1);
    static int args_num(){ return 1-option_args_num();}
    static int option_args_num()
    {
        return pyoption_traits_t<typename type_ref_traits_t<ARG1>::value_t>::is();
    }
    static PyObject* pyfunc(PyObject* self, PyObject* args)
    {
        pyext_tool_t pyext_tool(args);
        if (pyext_tool.is_err())
        {
            return NULL;
        }
        func_t f = (func_t)pyext_tool.get_func_addr();
        if (0 == f)
        {
            PyErr_SetString(PyExc_TypeError, "func address must provided");
            return NULL;
        }
        type_ref_traits_t<ARG1> a1;
        if (pyext_tool.parse_arg(a1.value).is_err())
        {
            return NULL;
        }
        return pyext_return_tool_t<RET>::route_call(f, a1);
    }
};

template <typename RET, typename ARG1, typename ARG2>
struct pyext_func_traits_t<RET (*)(ARG1, ARG2)>
{
    typedef RET (*func_t)(ARG1, ARG2);
    static int args_num() { return 2 - option_args_num();}
    static int option_args_num()
    {
        return pyoption_traits_t<typename type_ref_traits_t<ARG1>::value_t>::is() +
            pyoption_traits_t<typename type_ref_traits_t<ARG2>::value_t>::is();
    }
    static PyObject* pyfunc(PyObject* self, PyObject* args)
    {
        pyext_tool_t pyext_tool(args);
        if (pyext_tool.is_err())
        {
            return NULL;
        }
        func_t f = (func_t)pyext_tool.get_func_addr();
        if (0 == f)
        {
            PyErr_SetString(PyExc_TypeError, "func address must provided");
            return NULL;
        }
        type_ref_traits_t<ARG1> a1;
        type_ref_traits_t<ARG2> a2;
        if (pyext_tool.parse_arg(a1.value).parse_arg(a2.value).is_err())
        {
            return NULL;
        }
        return pyext_return_tool_t<RET>::route_call(f, a1, a2);
    }
};

template <typename RET, typename ARG1, typename ARG2, typename ARG3>
struct pyext_func_traits_t<RET (*)(ARG1, ARG2, ARG3)>
{
    typedef RET (*func_t)(ARG1, ARG2, ARG3);
    static int args_num() { return 3-option_args_num();}
    static int option_args_num() 
    { 
        return pyoption_traits_t<typename type_ref_traits_t<ARG1>::value_t>::is() +
            pyoption_traits_t<typename type_ref_traits_t<ARG2>::value_t>::is() +
            pyoption_traits_t<typename type_ref_traits_t<ARG3>::value_t>::is();
    }
    static PyObject* pyfunc(PyObject* self, PyObject* args)
    {
        pyext_tool_t pyext_tool(args);
        if (pyext_tool.is_err())
        {
            return NULL;
        }
        func_t f = (func_t)pyext_tool.get_func_addr();
        if (0 == f)
        {
            PyErr_SetString(PyExc_TypeError, "func address must provided");
            return NULL;
        }
        type_ref_traits_t<ARG1> a1;
        type_ref_traits_t<ARG2> a2;
        type_ref_traits_t<ARG3> a3;
        if (pyext_tool.parse_arg(a1.value).parse_arg(a2.value).parse_arg(a3.value).is_err())
        {
            return NULL;
        }
        return pyext_return_tool_t<RET>::route_call(f, a1, a2, a3);
    }
};
template <typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4>
struct pyext_func_traits_t<RET (*)(ARG1, ARG2, ARG3, ARG4)>
{
    typedef RET (*func_t)(ARG1, ARG2, ARG3, ARG4);
    static int args_num() { return 4-option_args_num();}
    static int option_args_num() { 
        return pyoption_traits_t<typename type_ref_traits_t<ARG1>::value_t>::is() +
            pyoption_traits_t<typename type_ref_traits_t<ARG2>::value_t>::is() +
            pyoption_traits_t<typename type_ref_traits_t<ARG3>::value_t>::is() +
            pyoption_traits_t<typename type_ref_traits_t<ARG4>::value_t>::is();
    }
    static PyObject* pyfunc(PyObject* self, PyObject* args)
    {
        pyext_tool_t pyext_tool(args);
        if (pyext_tool.is_err())
        {
            return NULL;
        }
        func_t f = (func_t)pyext_tool.get_func_addr();
        if (0 == f)
        {
            PyErr_SetString(PyExc_TypeError, "func address must provided");
            return NULL;
        }
        type_ref_traits_t<ARG1> a1;
        type_ref_traits_t<ARG2> a2;
        type_ref_traits_t<ARG3> a3;
        type_ref_traits_t<ARG4> a4;
        if (pyext_tool.parse_arg(a1.value).parse_arg(a2.value).parse_arg(a3.value).parse_arg(a4.value).is_err())
        {
            return NULL;
        }
        return pyext_return_tool_t<RET>::route_call(f, a1, a2, a3, a4);
    }
};

template <typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5>
struct pyext_func_traits_t<RET (*)(ARG1, ARG2, ARG3, ARG4, ARG5)>
{
    typedef RET (*func_t)(ARG1, ARG2, ARG3, ARG4, ARG5);
    static int args_num() { return 5-option_args_num();}
    static int option_args_num() { 
        return pyoption_traits_t<typename type_ref_traits_t<ARG1>::value_t>::is() +
            pyoption_traits_t<typename type_ref_traits_t<ARG2>::value_t>::is() +
            pyoption_traits_t<typename type_ref_traits_t<ARG3>::value_t>::is() +
            pyoption_traits_t<typename type_ref_traits_t<ARG4>::value_t>::is() +
            pyoption_traits_t<typename type_ref_traits_t<ARG5>::value_t>::is();
    }
    static PyObject* pyfunc(PyObject* self, PyObject* args)
    {
        pyext_tool_t pyext_tool(args);
        if (pyext_tool.is_err())
        {
            return NULL;
        }
        func_t f = (func_t)pyext_tool.get_func_addr();
        if (0 == f)
        {
            PyErr_SetString(PyExc_TypeError, "func address must provided");
            return NULL;
        }
        type_ref_traits_t<ARG1> a1;
        type_ref_traits_t<ARG2> a2;
        type_ref_traits_t<ARG3> a3;
        type_ref_traits_t<ARG4> a4;
        type_ref_traits_t<ARG5> a5;
        if (pyext_tool.parse_arg(a1.value).parse_arg(a2.value).parse_arg(a3.value).parse_arg(a4.value)
            .parse_arg(a5.value).is_err())
        {
            return NULL;
        }
        return pyext_return_tool_t<RET>::route_call(f, a1, a2, a3, a4, a5);
    }
};

template <typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6>
struct pyext_func_traits_t<RET (*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6)>
{
    typedef RET (*func_t)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6);
    static int args_num() { return 6-option_args_num();}
    static int option_args_num() { 
        return pyoption_traits_t<typename type_ref_traits_t<ARG1>::value_t>::is() +
            pyoption_traits_t<typename type_ref_traits_t<ARG2>::value_t>::is() +
            pyoption_traits_t<typename type_ref_traits_t<ARG3>::value_t>::is() +
            pyoption_traits_t<typename type_ref_traits_t<ARG4>::value_t>::is() +
            pyoption_traits_t<typename type_ref_traits_t<ARG5>::value_t>::is() +
            pyoption_traits_t<typename type_ref_traits_t<ARG6>::value_t>::is();
    }
    static PyObject* pyfunc(PyObject* self, PyObject* args)
    {
        pyext_tool_t pyext_tool(args);
        if (pyext_tool.is_err())
        {
            return NULL;
        }
        func_t f = (func_t)pyext_tool.get_func_addr();
        if (0 == f)
        {
            PyErr_SetString(PyExc_TypeError, "func address must provided");
            return NULL;
        }
        type_ref_traits_t<ARG1> a1;
        type_ref_traits_t<ARG2> a2;
        type_ref_traits_t<ARG3> a3;
        type_ref_traits_t<ARG4> a4;
        type_ref_traits_t<ARG5> a5;
        type_ref_traits_t<ARG6> a6;
        if (pyext_tool.parse_arg(a1.value).parse_arg(a2.value).parse_arg(a3.value).parse_arg(a4.value)
            .parse_arg(a5.value).parse_arg(a6.value).is_err())
        {
            return NULL;
        }
        return pyext_return_tool_t<RET>::route_call(f, a1, a2, a3, a4, a5, a6);
    }
};

template <typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6,
typename ARG7>
struct pyext_func_traits_t<RET (*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7)>
{
    typedef RET (*func_t)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7);
    static int args_num() { return 7-option_args_num();}
    static int option_args_num() { 
        return pyoption_traits_t<typename type_ref_traits_t<ARG1>::value_t>::is() +
            pyoption_traits_t<typename type_ref_traits_t<ARG2>::value_t>::is() +
            pyoption_traits_t<typename type_ref_traits_t<ARG3>::value_t>::is() +
            pyoption_traits_t<typename type_ref_traits_t<ARG4>::value_t>::is() +
            pyoption_traits_t<typename type_ref_traits_t<ARG5>::value_t>::is() +
            pyoption_traits_t<typename type_ref_traits_t<ARG6>::value_t>::is() +
            pyoption_traits_t<typename type_ref_traits_t<ARG7>::value_t>::is();
    }
    static PyObject* pyfunc(PyObject* self, PyObject* args)
    {
        pyext_tool_t pyext_tool(args);
        if (pyext_tool.is_err())
        {
            return NULL;
        }
        func_t f = (func_t)pyext_tool.get_func_addr();
        if (0 == f)
        {
            PyErr_SetString(PyExc_TypeError, "func address must provided");
            return NULL;
        }
        type_ref_traits_t<ARG1> a1;
        type_ref_traits_t<ARG2> a2;
        type_ref_traits_t<ARG3> a3;
        type_ref_traits_t<ARG4> a4;
        type_ref_traits_t<ARG5> a5;
        type_ref_traits_t<ARG6> a6;
        type_ref_traits_t<ARG7> a7;
        if (pyext_tool.parse_arg(a1.value).parse_arg(a2.value).parse_arg(a3.value).parse_arg(a4.value)
            .parse_arg(a5.value).parse_arg(a6.value).parse_arg(a7.value).is_err())
        {
            return NULL;
        }
        return pyext_return_tool_t<RET>::route_call(f, a1, a2, a3, a4, a5, a6, a7);
    }
};

template <typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6,
typename ARG7, typename ARG8>
struct pyext_func_traits_t<RET (*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8)>
{
    typedef RET (*func_t)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8);
    static int args_num() { return 8-option_args_num();}
    static int option_args_num() { 
        return pyoption_traits_t<typename type_ref_traits_t<ARG1>::value_t>::is() +
            pyoption_traits_t<typename type_ref_traits_t<ARG2>::value_t>::is() +
            pyoption_traits_t<typename type_ref_traits_t<ARG3>::value_t>::is() +
            pyoption_traits_t<typename type_ref_traits_t<ARG4>::value_t>::is() +
            pyoption_traits_t<typename type_ref_traits_t<ARG5>::value_t>::is() +
            pyoption_traits_t<typename type_ref_traits_t<ARG6>::value_t>::is() +
            pyoption_traits_t<typename type_ref_traits_t<ARG7>::value_t>::is() +
            pyoption_traits_t<typename type_ref_traits_t<ARG8>::value_t>::is();
    }
    static PyObject* pyfunc(PyObject* self, PyObject* args)
    {
        pyext_tool_t pyext_tool(args);
        if (pyext_tool.is_err())
        {
            return NULL;
        }
        func_t f = (func_t)pyext_tool.get_func_addr();
        if (0 == f)
        {
            PyErr_SetString(PyExc_TypeError, "func address must provided");
            return NULL;
        }
        type_ref_traits_t<ARG1> a1;
        type_ref_traits_t<ARG2> a2;
        type_ref_traits_t<ARG3> a3;
        type_ref_traits_t<ARG4> a4;
        type_ref_traits_t<ARG5> a5;
        type_ref_traits_t<ARG6> a6;
        type_ref_traits_t<ARG7> a7;
        type_ref_traits_t<ARG8> a8;
        if (pyext_tool.parse_arg(a1.value).parse_arg(a2.value).parse_arg(a3.value).parse_arg(a4.value)
            .parse_arg(a5.value).parse_arg(a6.value).parse_arg(a7.value).parse_arg(a8.value).is_err())
        {
            return NULL;
        }
        return pyext_return_tool_t<RET>::route_call(f, a1, a2, a3, a4, a5, a6, a7, a8);
    }
};

template <typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6,
typename ARG7, typename ARG8, typename ARG9>
struct pyext_func_traits_t<RET (*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9)>
{
    typedef RET (*func_t)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9);
    static int args_num() { return 9-option_args_num();}
    static int option_args_num() { 
        return pyoption_traits_t<typename type_ref_traits_t<ARG1>::value_t>::is() +
            pyoption_traits_t<typename type_ref_traits_t<ARG2>::value_t>::is() +
            pyoption_traits_t<typename type_ref_traits_t<ARG3>::value_t>::is() +
            pyoption_traits_t<typename type_ref_traits_t<ARG4>::value_t>::is() +
            pyoption_traits_t<typename type_ref_traits_t<ARG5>::value_t>::is() +
            pyoption_traits_t<typename type_ref_traits_t<ARG6>::value_t>::is() +
            pyoption_traits_t<typename type_ref_traits_t<ARG7>::value_t>::is() +
            pyoption_traits_t<typename type_ref_traits_t<ARG8>::value_t>::is() +
            pyoption_traits_t<typename type_ref_traits_t<ARG9>::value_t>::is();
    }
    static PyObject* pyfunc(PyObject* self, PyObject* args)
    {
        pyext_tool_t pyext_tool(args);
        if (pyext_tool.is_err())
        {
            return NULL;
        }
        func_t f = (func_t)pyext_tool.get_func_addr();
        if (0 == f)
        {
            PyErr_SetString(PyExc_TypeError, "func address must provided");
            return NULL;
        }
        type_ref_traits_t<ARG1> a1;
        type_ref_traits_t<ARG2> a2;
        type_ref_traits_t<ARG3> a3;
        type_ref_traits_t<ARG4> a4;
        type_ref_traits_t<ARG5> a5;
        type_ref_traits_t<ARG6> a6;
        type_ref_traits_t<ARG7> a7;
        type_ref_traits_t<ARG8> a8;
        type_ref_traits_t<ARG9> a9;
        if (pyext_tool.parse_arg(a1.value).parse_arg(a2.value).parse_arg(a3.value).parse_arg(a4.value)
            .parse_arg(a5.value).parse_arg(a6.value).parse_arg(a7.value).parse_arg(a8.value).parse_arg(a9.value).is_err())
        {
            return NULL;
        }
        return pyext_return_tool_t<RET>::route_call(f, a1, a2, a3, a4, a5, a6, a7, a8, a9);
    }
};

//! ��������pyclass ��ʼ������
template <typename CLASS_TYPE>
struct pyclass_ctor_tool_t<CLASS_TYPE, int(*)()>
{
    typedef typename pyclass_base_info_t<CLASS_TYPE>::obj_data_t obj_data_t;
    static int init_obj(obj_data_t *self, PyObject *args, PyObject *kwds)
    {
        if (self->obj) return 0;

        self->obj = new CLASS_TYPE();
        return 0;
    }
};

template <typename CLASS_TYPE, typename ARG1>
struct pyclass_ctor_tool_t<CLASS_TYPE, int(*)(ARG1)>
{
    typedef typename pyclass_base_info_t<CLASS_TYPE>::obj_data_t obj_data_t;
    static int init_obj(obj_data_t *self, PyObject *args, PyObject *kwds)
    {
        if (self->obj) return 0;
        pyext_tool_t pyext_tool(args);
        if (pyext_tool.is_err())
        {
            return -1;
        }
        type_ref_traits_t<ARG1> a1;
        if (pyext_tool.parse_arg(a1.value).is_err())
        {
            return -1;
        }
        self->obj = new CLASS_TYPE(a1.value);
        return 0;
    }
};

template <typename CLASS_TYPE, typename ARG1, typename ARG2>
struct pyclass_ctor_tool_t<CLASS_TYPE, int(*)(ARG1,ARG2)>
{
    typedef typename pyclass_base_info_t<CLASS_TYPE>::obj_data_t obj_data_t;
    static int init_obj(obj_data_t *self, PyObject *args, PyObject *kwds)
    {
        if (self->obj) return 0;
        pyext_tool_t pyext_tool(args);
        if (pyext_tool.is_err())
        {
            return -1;
        }
        type_ref_traits_t<ARG1> a1;
        type_ref_traits_t<ARG2> a2;
        if (pyext_tool.parse_arg(a1.value).parse_arg(a2.value).is_err())
        {
            return -1;
        }
        self->obj = new CLASS_TYPE(a1.value, a2.value);
        return 0;
    }
};

template <typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3>
struct pyclass_ctor_tool_t<CLASS_TYPE, int(*)(ARG1,ARG2,ARG3)>
{
    typedef typename pyclass_base_info_t<CLASS_TYPE>::obj_data_t obj_data_t;
    static int init_obj(obj_data_t *self, PyObject *args, PyObject *kwds)
    {
        if (self->obj) return 0;
        pyext_tool_t pyext_tool(args);
        if (pyext_tool.is_err())
        {
            return -1;
        }
        type_ref_traits_t<ARG1> a1;
        type_ref_traits_t<ARG2> a2;
        type_ref_traits_t<ARG3> a3;
        if (pyext_tool.parse_arg(a1.value).parse_arg(a2.value).parse_arg(a3.value).is_err())
        {
            return -1;
        }
        self->obj = new CLASS_TYPE(a1.value, a2.value, a3.value);
        return 0;
    }
};

template <typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4>
struct pyclass_ctor_tool_t<CLASS_TYPE, int(*)(ARG1,ARG2,ARG3,ARG4)>
{
    typedef typename pyclass_base_info_t<CLASS_TYPE>::obj_data_t obj_data_t;
    static int init_obj(obj_data_t *self, PyObject *args, PyObject *kwds)
    {
        if (self->obj) return 0;
        pyext_tool_t pyext_tool(args);
        if (pyext_tool.is_err())
        {
            return -1;
        }
        type_ref_traits_t<ARG1> a1;
        type_ref_traits_t<ARG2> a2;
        type_ref_traits_t<ARG3> a3;
        type_ref_traits_t<ARG4> a4;
        if (pyext_tool.parse_arg(a1.value).parse_arg(a2.value).parse_arg(a3.value).parse_arg(a4.value).is_err())
        {
            return -1;
        }
        self->obj = new CLASS_TYPE(a1.value, a2.value, a3.value, a4.value);
        return 0;
    }
};

template <typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5>
struct pyclass_ctor_tool_t<CLASS_TYPE, int(*)(ARG1,ARG2,ARG3,ARG4,ARG5)>
{
    typedef typename pyclass_base_info_t<CLASS_TYPE>::obj_data_t obj_data_t;
    static int init_obj(obj_data_t *self, PyObject *args, PyObject *kwds)
    {
        if (self->obj) return 0;
        pyext_tool_t pyext_tool(args);
        if (pyext_tool.is_err())
        {
            return -1;
        }
        type_ref_traits_t<ARG1> a1;
        type_ref_traits_t<ARG2> a2;
        type_ref_traits_t<ARG3> a3;
        type_ref_traits_t<ARG4> a4;
        type_ref_traits_t<ARG5> a5;
        if (pyext_tool.parse_arg(a1.value).parse_arg(a2.value).parse_arg(a3.value).parse_arg(a4.value)
            .parse_arg(a5.value).is_err())
        {
            return -1;
        }
        self->obj = new CLASS_TYPE(a1.value, a2.value, a3.value, a4.value, a5.value);
        return 0;
    }
};

template <typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6>
struct pyclass_ctor_tool_t<CLASS_TYPE, int(*)(ARG1,ARG2,ARG3,ARG4,ARG5,ARG6)>
{
    typedef typename pyclass_base_info_t<CLASS_TYPE>::obj_data_t obj_data_t;
    static int init_obj(obj_data_t *self, PyObject *args, PyObject *kwds)
    {
        if (self->obj) return 0;
        pyext_tool_t pyext_tool(args);
        if (pyext_tool.is_err())
        {
            return -1;
        }
        type_ref_traits_t<ARG1> a1;
        type_ref_traits_t<ARG2> a2;
        type_ref_traits_t<ARG3> a3;
        type_ref_traits_t<ARG4> a4;
        type_ref_traits_t<ARG5> a5;
        type_ref_traits_t<ARG6> a6;
        if (pyext_tool.parse_arg(a1.value).parse_arg(a2.value).parse_arg(a3.value).parse_arg(a4.value)
            .parse_arg(a5.value).parse_arg(a6.value).is_err())
        {
            return -1;
        }
        self->obj = new CLASS_TYPE(a1.value, a2.value, a3.value, a4.value, a5.value, a6.value);
        return 0;
    }
};

template <typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6,
typename ARG7>
struct pyclass_ctor_tool_t<CLASS_TYPE, int(*)(ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7)>
{
    typedef typename pyclass_base_info_t<CLASS_TYPE>::obj_data_t obj_data_t;
    static int init_obj(obj_data_t *self, PyObject *args, PyObject *kwds)
    {
        if (self->obj) return 0;
        pyext_tool_t pyext_tool(args);
        if (pyext_tool.is_err())
        {
            return -1;
        }
        type_ref_traits_t<ARG1> a1;
        type_ref_traits_t<ARG2> a2;
        type_ref_traits_t<ARG3> a3;
        type_ref_traits_t<ARG4> a4;
        type_ref_traits_t<ARG5> a5;
        type_ref_traits_t<ARG6> a6;
        type_ref_traits_t<ARG7> a7;
        if (pyext_tool.parse_arg(a1.value).parse_arg(a2.value).parse_arg(a3.value).parse_arg(a4.value)
            .parse_arg(a5.value).parse_arg(a6.value).parse_arg(a7.value).is_err())
        {
            return -1;
        }
        self->obj = new CLASS_TYPE(a1.value, a2.value, a3.value, a4.value, a5.value, a6.value, a7.value);
        return 0;
    }
};

template <typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6,
typename ARG7,typename ARG8>
struct pyclass_ctor_tool_t<CLASS_TYPE, int(*)(ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8)>
{
    typedef typename pyclass_base_info_t<CLASS_TYPE>::obj_data_t obj_data_t;
    static int init_obj(obj_data_t *self, PyObject *args, PyObject *kwds)
    {
        if (self->obj) return 0;
        pyext_tool_t pyext_tool(args);
        if (pyext_tool.is_err())
        {
            return -1;
        }
        type_ref_traits_t<ARG1> a1;
        type_ref_traits_t<ARG2> a2;
        type_ref_traits_t<ARG3> a3;
        type_ref_traits_t<ARG4> a4;
        type_ref_traits_t<ARG5> a5;
        type_ref_traits_t<ARG6> a6;
        type_ref_traits_t<ARG7> a7;
        type_ref_traits_t<ARG8> a8;
        if (pyext_tool.parse_arg(a1.value).parse_arg(a2.value).parse_arg(a3.value).parse_arg(a4.value)
            .parse_arg(a5.value).parse_arg(a6.value).parse_arg(a7.value).parse_arg(a8.value).is_err())
        {
            return -1;
        }
        self->obj = new CLASS_TYPE(a1.value, a2.value, a3.value, a4.value, a5.value, a6.value, a7.value, a8.value);
        return 0;
    }
};

template <typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6,
typename ARG7,typename ARG8,typename ARG9>
struct pyclass_ctor_tool_t<CLASS_TYPE, int(*)(ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,ARG9)>
{
    typedef typename pyclass_base_info_t<CLASS_TYPE>::obj_data_t obj_data_t;
    static int init_obj(obj_data_t *self, PyObject *args, PyObject *kwds)
    {
        if (self->obj) return 0;
        pyext_tool_t pyext_tool(args);
        if (pyext_tool.is_err())
        {
            return -1;
        }
        type_ref_traits_t<ARG1> a1;
        type_ref_traits_t<ARG2> a2;
        type_ref_traits_t<ARG3> a3;
        type_ref_traits_t<ARG4> a4;
        type_ref_traits_t<ARG5> a5;
        type_ref_traits_t<ARG6> a6;
        type_ref_traits_t<ARG7> a7;
        type_ref_traits_t<ARG8> a8;
        type_ref_traits_t<ARG9> a9;
        if (pyext_tool.parse_arg(a1.value).parse_arg(a2.value).parse_arg(a3.value).parse_arg(a4.value)
            .parse_arg(a5.value).parse_arg(a6.value).parse_arg(a7.value).parse_arg(a8.value).parse_arg(a9.value).is_err())
        {
            return -1;
        }
        self->obj = new CLASS_TYPE(a1.value, a2.value, a3.value, a4.value, a5.value, a6.value, a7.value, a8.value, a9.value);
        return 0;
    }
};

template<typename RET, typename CLASS_TYPE>
struct pyclass_method_gen_t<RET (CLASS_TYPE::*)()>
{
    typedef RET (CLASS_TYPE::*func_t)();
    static int args_num() { return 0;}
    static int option_args_num() { return 0;}
    static PyObject *pymethod(typename pyclass_base_info_t<CLASS_TYPE>::obj_data_t* self, PyObject* args)
    {
        pyext_tool_t pyext_tool(args);
        if (pyext_tool.is_err())
        {
            return NULL;
        }
        func_t f = pyext_tool.get_func_ptr<func_t>();
        if (0 == f)
        {
            PyErr_SetString(PyExc_TypeError, "func address must provided");
            return NULL;
        }
        return pyext_return_tool_t<RET>::route_method_call(self->obj, f);;
    }
};

template<typename RET, typename CLASS_TYPE, typename ARG1>
struct pyclass_method_gen_t<RET (CLASS_TYPE::*)(ARG1)>
{
    typedef RET (CLASS_TYPE::*func_t)(ARG1);
    static int args_num() { return 1-option_args_num();}
    static int option_args_num() { return pyoption_traits_t<typename type_ref_traits_t<ARG1>::value_t>::is();}

    static PyObject *pymethod(typename pyclass_base_info_t<CLASS_TYPE>::obj_data_t* self, PyObject* args)
    {
        pyext_tool_t pyext_tool(args);
        if (pyext_tool.is_err())
        {
            return NULL;
        }
        func_t f = pyext_tool.get_func_ptr<func_t>();
        if (0 == f)
        {
            PyErr_SetString(PyExc_TypeError, "func address must provided");
            return NULL;
        }
        type_ref_traits_t<ARG1> a1;
        if (pyext_tool.parse_arg(a1.value).is_err())
        {
            return NULL;
        }
        return pyext_return_tool_t<RET>::route_method_call(self->obj, f, a1);;
    }
};

template<typename RET, typename CLASS_TYPE, typename ARG1, typename ARG2>
struct pyclass_method_gen_t<RET (CLASS_TYPE::*)(ARG1, ARG2)>
{
    typedef RET (CLASS_TYPE::*func_t)(ARG1, ARG2);
    static int args_num() { return 2-option_args_num();}
    static int option_args_num() { 
        return pyoption_traits_t<typename type_ref_traits_t<ARG1>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG2>::value_t>::is();
    }

    static PyObject *pymethod(typename pyclass_base_info_t<CLASS_TYPE>::obj_data_t* self, PyObject* args)
    {
        pyext_tool_t pyext_tool(args);
        if (pyext_tool.is_err())
        {
            return NULL;
        }
        func_t f = pyext_tool.get_func_ptr<func_t>();
        if (0 == f)
        {
            PyErr_SetString(PyExc_TypeError, "func address must provided");
            return NULL;
        }
        type_ref_traits_t<ARG1> a1;
        type_ref_traits_t<ARG2> a2;
        if (pyext_tool.parse_arg(a1.value).parse_arg(a2.value).is_err())
        {
            return NULL;
        }
        return pyext_return_tool_t<RET>::route_method_call(self->obj, f, a1, a2);;
    }
};


template<typename RET, typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3>
struct pyclass_method_gen_t<RET (CLASS_TYPE::*)(ARG1, ARG2, ARG3)>
{
    typedef RET (CLASS_TYPE::*func_t)(ARG1, ARG2, ARG3);
    static int args_num() { return 3-option_args_num();}
    static int option_args_num() { 
        return pyoption_traits_t<typename type_ref_traits_t<ARG1>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG2>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG3>::value_t>::is();
    }

    static PyObject *pymethod(typename pyclass_base_info_t<CLASS_TYPE>::obj_data_t* self, PyObject* args)
    {
        pyext_tool_t pyext_tool(args);
        if (pyext_tool.is_err())
        {
            return NULL;
        }
        func_t f = pyext_tool.get_func_ptr<func_t>();
        if (0 == f)
        {
            PyErr_SetString(PyExc_TypeError, "func address must provided");
            return NULL;
        }
        type_ref_traits_t<ARG1> a1;
        type_ref_traits_t<ARG2> a2;
        type_ref_traits_t<ARG3> a3;
        if (pyext_tool.parse_arg(a1.value).parse_arg(a2.value).parse_arg(a3.value).is_err())
        {
            return NULL;
        }
        return pyext_return_tool_t<RET>::route_method_call(self->obj, f, a1, a2, a3);;
    }
};

template<typename RET, typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4>
struct pyclass_method_gen_t<RET (CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4)>
{
    typedef RET (CLASS_TYPE::*func_t)(ARG1, ARG2, ARG3, ARG4);
    static int args_num() { return 4-option_args_num();}
    static int option_args_num() { 
        return pyoption_traits_t<typename type_ref_traits_t<ARG1>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG2>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG3>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG4>::value_t>::is();
    }

    static PyObject *pymethod(typename pyclass_base_info_t<CLASS_TYPE>::obj_data_t* self, PyObject* args)
    {
        pyext_tool_t pyext_tool(args);
        if (pyext_tool.is_err())
        {
            return NULL;
        }
        func_t f = pyext_tool.get_func_ptr<func_t>();
        if (0 == f)
        {
            PyErr_SetString(PyExc_TypeError, "func address must provided");
            return NULL;
        }
        type_ref_traits_t<ARG1> a1;
        type_ref_traits_t<ARG2> a2;
        type_ref_traits_t<ARG3> a3;
        type_ref_traits_t<ARG4> a4;
        if (pyext_tool.parse_arg(a1.value).parse_arg(a2.value).parse_arg(a3.value).parse_arg(a4.value).is_err())
        {
            return NULL;
        }
        return pyext_return_tool_t<RET>::route_method_call(self->obj, f, a1, a2, a3, a4);;
    }
};

template<typename RET, typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5>
struct pyclass_method_gen_t<RET (CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5)>
{
    typedef RET (CLASS_TYPE::*func_t)(ARG1, ARG2, ARG3, ARG4, ARG5);
    static int args_num() { return 5-option_args_num();}
    static int option_args_num() { 
        return pyoption_traits_t<typename type_ref_traits_t<ARG1>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG2>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG3>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG4>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG5>::value_t>::is();
    }

    static PyObject *pymethod(typename pyclass_base_info_t<CLASS_TYPE>::obj_data_t* self, PyObject* args)
    {
        pyext_tool_t pyext_tool(args);
        if (pyext_tool.is_err())
        {
            return NULL;
        }
        func_t f = pyext_tool.get_func_ptr<func_t>();
        if (0 == f)
        {
            PyErr_SetString(PyExc_TypeError, "func address must provided");
            return NULL;
        }
        type_ref_traits_t<ARG1> a1;
        type_ref_traits_t<ARG2> a2;
        type_ref_traits_t<ARG3> a3;
        type_ref_traits_t<ARG4> a4;
        type_ref_traits_t<ARG5> a5;
        if (pyext_tool.parse_arg(a1.value).parse_arg(a2.value).parse_arg(a3.value).parse_arg(a4.value)
            .parse_arg(a5.value).is_err())
        {
            return NULL;
        }
        return pyext_return_tool_t<RET>::route_method_call(self->obj, f, a1, a2, a3, a4, a5);;
    }
};

template<typename RET, typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5,
typename ARG6>
struct pyclass_method_gen_t<RET (CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6)>
{
    typedef RET (CLASS_TYPE::*func_t)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6);
    static int args_num() { return 6-option_args_num();}
    static int option_args_num() { 
        return pyoption_traits_t<typename type_ref_traits_t<ARG1>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG2>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG3>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG4>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG5>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG6>::value_t>::is();
    }

    static PyObject *pymethod(typename pyclass_base_info_t<CLASS_TYPE>::obj_data_t* self, PyObject* args)
    {
        pyext_tool_t pyext_tool(args);
        if (pyext_tool.is_err())
        {
            return NULL;
        }
        func_t f = pyext_tool.get_func_ptr<func_t>();
        if (0 == f)
        {
            PyErr_SetString(PyExc_TypeError, "func address must provided");
            return NULL;
        }
        type_ref_traits_t<ARG1> a1;
        type_ref_traits_t<ARG2> a2;
        type_ref_traits_t<ARG3> a3;
        type_ref_traits_t<ARG4> a4;
        type_ref_traits_t<ARG5> a5;
        type_ref_traits_t<ARG6> a6;
        if (pyext_tool.parse_arg(a1.value).parse_arg(a2.value).parse_arg(a3.value).parse_arg(a4.value)
            .parse_arg(a5.value).parse_arg(a6.value).is_err())
        {
            return NULL;
        }
        return pyext_return_tool_t<RET>::route_method_call(self->obj, f, a1, a2, a3, a4, a5, a6);;
    }
};

template<typename RET, typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5,
typename ARG6, typename ARG7>
struct pyclass_method_gen_t<RET (CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7)>
{
    typedef RET (CLASS_TYPE::*func_t)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7);
    static int args_num() { return 7-option_args_num();}
    static int option_args_num() { 
        return pyoption_traits_t<typename type_ref_traits_t<ARG1>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG2>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG3>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG4>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG5>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG6>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG7>::value_t>::is();
    }

    static PyObject *pymethod(typename pyclass_base_info_t<CLASS_TYPE>::obj_data_t* self, PyObject* args)
    {
        pyext_tool_t pyext_tool(args);
        if (pyext_tool.is_err())
        {
            return NULL;
        }
        func_t f = pyext_tool.get_func_ptr<func_t>();
        if (0 == f)
        {
            PyErr_SetString(PyExc_TypeError, "func address must provided");
            return NULL;
        }
        type_ref_traits_t<ARG1> a1;
        type_ref_traits_t<ARG2> a2;
        type_ref_traits_t<ARG3> a3;
        type_ref_traits_t<ARG4> a4;
        type_ref_traits_t<ARG5> a5;
        type_ref_traits_t<ARG6> a6;
        type_ref_traits_t<ARG7> a7;
        if (pyext_tool.parse_arg(a1.value).parse_arg(a2.value).parse_arg(a3.value).parse_arg(a4.value)
            .parse_arg(a5.value).parse_arg(a6.value).parse_arg(a7.value).is_err())
        {
            return NULL;
        }
        return pyext_return_tool_t<RET>::route_method_call(self->obj, f, a1, a2, a3, a4, a5, a6, a7);;
    }
};

template<typename RET, typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5,
typename ARG6, typename ARG7, typename ARG8>
struct pyclass_method_gen_t<RET (CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8)>
{
    typedef RET (CLASS_TYPE::*func_t)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8);
    static int args_num() { return 8-option_args_num();}
    static int option_args_num() { 
        return pyoption_traits_t<typename type_ref_traits_t<ARG1>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG2>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG3>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG4>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG5>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG6>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG7>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG8>::value_t>::is();
    }

    static PyObject *pymethod(typename pyclass_base_info_t<CLASS_TYPE>::obj_data_t* self, PyObject* args)
    {
        pyext_tool_t pyext_tool(args);
        if (pyext_tool.is_err())
        {
            return NULL;
        }
        func_t f = pyext_tool.get_func_ptr<func_t>();
        if (0 == f)
        {
            PyErr_SetString(PyExc_TypeError, "func address must provided");
            return NULL;
        }
        type_ref_traits_t<ARG1> a1;
        type_ref_traits_t<ARG2> a2;
        type_ref_traits_t<ARG3> a3;
        type_ref_traits_t<ARG4> a4;
        type_ref_traits_t<ARG5> a5;
        type_ref_traits_t<ARG6> a6;
        type_ref_traits_t<ARG7> a7;
        type_ref_traits_t<ARG8> a8;
        if (pyext_tool.parse_arg(a1.value).parse_arg(a2.value).parse_arg(a3.value).parse_arg(a4.value)
            .parse_arg(a5.value).parse_arg(a6.value).parse_arg(a7.value).parse_arg(a8.value).is_err())
        {
            return NULL;
        }
        return pyext_return_tool_t<RET>::route_method_call(self->obj, f, a1, a2, a3, a4, a5, a6, a7, a8);;
    }
};


template<typename RET, typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5,
typename ARG6, typename ARG7, typename ARG8, typename ARG9>
struct pyclass_method_gen_t<RET (CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9)>
{
    typedef RET (CLASS_TYPE::*func_t)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9);
    static int args_num() { return 9-option_args_num();}
    static int option_args_num() { 
        return pyoption_traits_t<typename type_ref_traits_t<ARG1>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG2>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG3>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG4>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG5>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG6>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG7>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG8>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG9>::value_t>::is();
    }

    static PyObject *pymethod(typename pyclass_base_info_t<CLASS_TYPE>::obj_data_t* self, PyObject* args)
    {
        pyext_tool_t pyext_tool(args);
        if (pyext_tool.is_err())
        {
            return NULL;
        }
        func_t f = pyext_tool.get_func_ptr<func_t>();
        if (0 == f)
        {
            PyErr_SetString(PyExc_TypeError, "func address must provided");
            return NULL;
        }
        type_ref_traits_t<ARG1> a1;
        type_ref_traits_t<ARG2> a2;
        type_ref_traits_t<ARG3> a3;
        type_ref_traits_t<ARG4> a4;
        type_ref_traits_t<ARG5> a5;
        type_ref_traits_t<ARG6> a6;
        type_ref_traits_t<ARG7> a7;
        type_ref_traits_t<ARG8> a8;
        type_ref_traits_t<ARG9> a9;
        if (pyext_tool.parse_arg(a1.value).parse_arg(a2.value).parse_arg(a3.value).parse_arg(a4.value)
            .parse_arg(a5.value).parse_arg(a6.value).parse_arg(a7.value).parse_arg(a8.value).parse_arg(a9.value).is_err())
        {
            return NULL;
        }
        return pyext_return_tool_t<RET>::route_method_call(self->obj, f, a1, a2, a3, a4, a5, a6, a7, a8, a9);;
    }
};

//! const���ͳ�Ա����---------------------------------------------------------------------------------------------

template<typename RET, typename CLASS_TYPE>
struct pyclass_method_gen_t<RET (CLASS_TYPE::*)() const>
{
    typedef RET (CLASS_TYPE::*func_t)() const;
    static int args_num() { return 0;}
    static int option_args_num() { return 0;}
    static PyObject *pymethod(typename pyclass_base_info_t<CLASS_TYPE>::obj_data_t* self, PyObject* args)
    {
        pyext_tool_t pyext_tool(args);
        if (pyext_tool.is_err())
        {
            return NULL;
        }
        func_t f = pyext_tool.get_func_ptr<func_t>();
        if (0 == f)
        {
            PyErr_SetString(PyExc_TypeError, "func address must provided");
            return NULL;
        }
        return pyext_return_tool_t<RET>::route_method_call(self->obj, f);;
    }
};

template<typename RET, typename CLASS_TYPE, typename ARG1>
struct pyclass_method_gen_t<RET (CLASS_TYPE::*)(ARG1) const>
{
    typedef RET (CLASS_TYPE::*func_t)(ARG1) const;
    static int args_num() { return 1-option_args_num();}
    static int option_args_num() { return pyoption_traits_t<typename type_ref_traits_t<ARG1>::value_t>::is();}

    static PyObject *pymethod(typename pyclass_base_info_t<CLASS_TYPE>::obj_data_t* self, PyObject* args)
    {
        pyext_tool_t pyext_tool(args);
        if (pyext_tool.is_err())
        {
            return NULL;
        }
        func_t f = pyext_tool.get_func_ptr<func_t>();
        if (0 == f)
        {
            PyErr_SetString(PyExc_TypeError, "func address must provided");
            return NULL;
        }
        type_ref_traits_t<ARG1> a1;
        if (pyext_tool.parse_arg(a1.value).is_err())
        {
            return NULL;
        }
        return pyext_return_tool_t<RET>::route_method_call(self->obj, f, a1);;
    }
};

template<typename RET, typename CLASS_TYPE, typename ARG1, typename ARG2>
struct pyclass_method_gen_t<RET (CLASS_TYPE::*)(ARG1, ARG2) const>
{
    typedef RET (CLASS_TYPE::*func_t)(ARG1, ARG2) const;
    static int args_num() { return 2-option_args_num();}
    static int option_args_num() { 
        return pyoption_traits_t<typename type_ref_traits_t<ARG1>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG2>::value_t>::is();
    }

    static PyObject *pymethod(typename pyclass_base_info_t<CLASS_TYPE>::obj_data_t* self, PyObject* args)
    {
        pyext_tool_t pyext_tool(args);
        if (pyext_tool.is_err())
        {
            return NULL;
        }
        func_t f = pyext_tool.get_func_ptr<func_t>();
        if (0 == f)
        {
            PyErr_SetString(PyExc_TypeError, "func address must provided");
            return NULL;
        }
        type_ref_traits_t<ARG1> a1;
        type_ref_traits_t<ARG2> a2;
        if (pyext_tool.parse_arg(a1.value).parse_arg(a2.value).is_err())
        {
            return NULL;
        }
        return pyext_return_tool_t<RET>::route_method_call(self->obj, f, a1, a2);;
    }
};


template<typename RET, typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3>
struct pyclass_method_gen_t<RET (CLASS_TYPE::*)(ARG1, ARG2, ARG3) const>
{
    typedef RET (CLASS_TYPE::*func_t)(ARG1, ARG2, ARG3) const;
    static int args_num() { return 3-option_args_num();}
    static int option_args_num() { 
        return pyoption_traits_t<typename type_ref_traits_t<ARG1>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG2>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG3>::value_t>::is();
    }

    static PyObject *pymethod(typename pyclass_base_info_t<CLASS_TYPE>::obj_data_t* self, PyObject* args)
    {
        pyext_tool_t pyext_tool(args);
        if (pyext_tool.is_err())
        {
            return NULL;
        }
        func_t f = pyext_tool.get_func_ptr<func_t>();
        if (0 == f)
        {
            PyErr_SetString(PyExc_TypeError, "func address must provided");
            return NULL;
        }
        type_ref_traits_t<ARG1> a1;
        type_ref_traits_t<ARG2> a2;
        type_ref_traits_t<ARG3> a3;
        if (pyext_tool.parse_arg(a1.value).parse_arg(a2.value).parse_arg(a3.value).is_err())
        {
            return NULL;
        }
        return pyext_return_tool_t<RET>::route_method_call(self->obj, f, a1, a2, a3);;
    }
};

template<typename RET, typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4>
struct pyclass_method_gen_t<RET (CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4) const>
{
    typedef RET (CLASS_TYPE::*func_t)(ARG1, ARG2, ARG3, ARG4) const;
    static int args_num() { return 4-option_args_num();}
    static int option_args_num() { 
        return pyoption_traits_t<typename type_ref_traits_t<ARG1>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG2>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG3>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG4>::value_t>::is();
    }

    static PyObject *pymethod(typename pyclass_base_info_t<CLASS_TYPE>::obj_data_t* self, PyObject* args)
    {
        pyext_tool_t pyext_tool(args);
        if (pyext_tool.is_err())
        {
            return NULL;
        }
        func_t f = pyext_tool.get_func_ptr<func_t>();
        if (0 == f)
        {
            PyErr_SetString(PyExc_TypeError, "func address must provided");
            return NULL;
        }
        type_ref_traits_t<ARG1> a1;
        type_ref_traits_t<ARG2> a2;
        type_ref_traits_t<ARG3> a3;
        type_ref_traits_t<ARG4> a4;
        if (pyext_tool.parse_arg(a1.value).parse_arg(a2.value).parse_arg(a3.value).parse_arg(a4.value).is_err())
        {
            return NULL;
        }
        return pyext_return_tool_t<RET>::route_method_call(self->obj, f, a1, a2, a3, a4);;
    }
};

template<typename RET, typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5>
struct pyclass_method_gen_t<RET (CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5) const>
{
    typedef RET (CLASS_TYPE::*func_t)(ARG1, ARG2, ARG3, ARG4, ARG5) const;
    static int args_num() { return 5-option_args_num();}
    static int option_args_num() { 
        return pyoption_traits_t<typename type_ref_traits_t<ARG1>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG2>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG3>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG4>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG5>::value_t>::is();
    }

    static PyObject *pymethod(typename pyclass_base_info_t<CLASS_TYPE>::obj_data_t* self, PyObject* args)
    {
        pyext_tool_t pyext_tool(args);
        if (pyext_tool.is_err())
        {
            return NULL;
        }
        func_t f = pyext_tool.get_func_ptr<func_t>();
        if (0 == f)
        {
            PyErr_SetString(PyExc_TypeError, "func address must provided");
            return NULL;
        }
        type_ref_traits_t<ARG1> a1;
        type_ref_traits_t<ARG2> a2;
        type_ref_traits_t<ARG3> a3;
        type_ref_traits_t<ARG4> a4;
        type_ref_traits_t<ARG5> a5;
        if (pyext_tool.parse_arg(a1.value).parse_arg(a2.value).parse_arg(a3.value).parse_arg(a4.value)
            .parse_arg(a5.value).is_err())
        {
            return NULL;
        }
        return pyext_return_tool_t<RET>::route_method_call(self->obj, f, a1, a2, a3, a4, a5);;
    }
};

template<typename RET, typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5,
typename ARG6>
struct pyclass_method_gen_t<RET (CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6) const>
{
    typedef RET (CLASS_TYPE::*func_t)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6) const;
    static int args_num() { return 6-option_args_num();}
    static int option_args_num() { 
        return pyoption_traits_t<typename type_ref_traits_t<ARG1>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG2>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG3>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG4>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG5>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG6>::value_t>::is();
    }

    static PyObject *pymethod(typename pyclass_base_info_t<CLASS_TYPE>::obj_data_t* self, PyObject* args)
    {
        pyext_tool_t pyext_tool(args);
        if (pyext_tool.is_err())
        {
            return NULL;
        }
        func_t f = pyext_tool.get_func_ptr<func_t>();
        if (0 == f)
        {
            PyErr_SetString(PyExc_TypeError, "func address must provided");
            return NULL;
        }
        type_ref_traits_t<ARG1> a1;
        type_ref_traits_t<ARG2> a2;
        type_ref_traits_t<ARG3> a3;
        type_ref_traits_t<ARG4> a4;
        type_ref_traits_t<ARG5> a5;
        type_ref_traits_t<ARG6> a6;
        if (pyext_tool.parse_arg(a1.value).parse_arg(a2.value).parse_arg(a3.value).parse_arg(a4.value)
            .parse_arg(a5.value).parse_arg(a6.value).is_err())
        {
            return NULL;
        }
        return pyext_return_tool_t<RET>::route_method_call(self->obj, f, a1, a2, a3, a4, a5, a6);;
    }
};

template<typename RET, typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5,
typename ARG6, typename ARG7>
struct pyclass_method_gen_t<RET (CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7) const>
{
    typedef RET (CLASS_TYPE::*func_t)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7) const;
    static int args_num() { return 7-option_args_num();}
    static int option_args_num() { 
        return pyoption_traits_t<typename type_ref_traits_t<ARG1>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG2>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG3>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG4>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG5>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG6>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG7>::value_t>::is();
    }

    static PyObject *pymethod(typename pyclass_base_info_t<CLASS_TYPE>::obj_data_t* self, PyObject* args)
    {
        pyext_tool_t pyext_tool(args);
        if (pyext_tool.is_err())
        {
            return NULL;
        }
        func_t f = pyext_tool.get_func_ptr<func_t>();
        if (0 == f)
        {
            PyErr_SetString(PyExc_TypeError, "func address must provided");
            return NULL;
        }
        type_ref_traits_t<ARG1> a1;
        type_ref_traits_t<ARG2> a2;
        type_ref_traits_t<ARG3> a3;
        type_ref_traits_t<ARG4> a4;
        type_ref_traits_t<ARG5> a5;
        type_ref_traits_t<ARG6> a6;
        type_ref_traits_t<ARG7> a7;
        if (pyext_tool.parse_arg(a1.value).parse_arg(a2.value).parse_arg(a3.value).parse_arg(a4.value)
            .parse_arg(a5.value).parse_arg(a6.value).parse_arg(a7.value).is_err())
        {
            return NULL;
        }
        return pyext_return_tool_t<RET>::route_method_call(self->obj, f, a1, a2, a3, a4, a5, a6, a7);;
    }
};

template<typename RET, typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5,
typename ARG6, typename ARG7, typename ARG8>
struct pyclass_method_gen_t<RET (CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8) const>
{
    typedef RET (CLASS_TYPE::*func_t)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8) const;
    static int args_num() { return 8-option_args_num();}
    static int option_args_num() { 
        return pyoption_traits_t<typename type_ref_traits_t<ARG1>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG2>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG3>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG4>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG5>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG6>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG7>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG8>::value_t>::is();
    }

    static PyObject *pymethod(typename pyclass_base_info_t<CLASS_TYPE>::obj_data_t* self, PyObject* args)
    {
        pyext_tool_t pyext_tool(args);
        if (pyext_tool.is_err())
        {
            return NULL;
        }
        func_t f = pyext_tool.get_func_ptr<func_t>();
        if (0 == f)
        {
            PyErr_SetString(PyExc_TypeError, "func address must provided");
            return NULL;
        }
        type_ref_traits_t<ARG1> a1;
        type_ref_traits_t<ARG2> a2;
        type_ref_traits_t<ARG3> a3;
        type_ref_traits_t<ARG4> a4;
        type_ref_traits_t<ARG5> a5;
        type_ref_traits_t<ARG6> a6;
        type_ref_traits_t<ARG7> a7;
        type_ref_traits_t<ARG8> a8;
        if (pyext_tool.parse_arg(a1.value).parse_arg(a2.value).parse_arg(a3.value).parse_arg(a4.value)
            .parse_arg(a5.value).parse_arg(a6.value).parse_arg(a7.value).parse_arg(a8.value).is_err())
        {
            return NULL;
        }
        return pyext_return_tool_t<RET>::route_method_call(self->obj, f, a1, a2, a3, a4, a5, a6, a7, a8);;
    }
};


template<typename RET, typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5,
typename ARG6, typename ARG7, typename ARG8, typename ARG9>
struct pyclass_method_gen_t<RET (CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9) const>
{
    typedef RET (CLASS_TYPE::*func_t)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9) const;
    static int args_num() { return 9-option_args_num();}
    static int option_args_num() { 
        return pyoption_traits_t<typename type_ref_traits_t<ARG1>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG2>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG3>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG4>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG5>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG6>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG7>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG8>::value_t>::is()+
            pyoption_traits_t<typename type_ref_traits_t<ARG9>::value_t>::is();
    }

    static PyObject *pymethod(typename pyclass_base_info_t<CLASS_TYPE>::obj_data_t* self, PyObject* args)
    {
        pyext_tool_t pyext_tool(args);
        if (pyext_tool.is_err())
        {
            return NULL;
        }
        func_t f = pyext_tool.get_func_ptr<func_t>();
        if (0 == f)
        {
            PyErr_SetString(PyExc_TypeError, "func address must provided");
            return NULL;
        }
        type_ref_traits_t<ARG1> a1;
        type_ref_traits_t<ARG2> a2;
        type_ref_traits_t<ARG3> a3;
        type_ref_traits_t<ARG4> a4;
        type_ref_traits_t<ARG5> a5;
        type_ref_traits_t<ARG6> a6;
        type_ref_traits_t<ARG7> a7;
        type_ref_traits_t<ARG8> a8;
        type_ref_traits_t<ARG9> a9;
        if (pyext_tool.parse_arg(a1.value).parse_arg(a2.value).parse_arg(a3.value).parse_arg(a4.value)
            .parse_arg(a5.value).parse_arg(a6.value).parse_arg(a7.value).parse_arg(a8.value).parse_arg(a9.value).is_err())
        {
            return NULL;
        }
        return pyext_return_tool_t<RET>::route_method_call(self->obj, f, a1, a2, a3, a4, a5, a6, a7, a8, a9);;
    }
};

#endif
