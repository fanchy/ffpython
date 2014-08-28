#ifndef  __PYOPS_BASE_
#define  __PYOPS_BASE_


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



//! 获取python异常信息
struct pyops_t
{
    static int traceback(string& ret_);
};
struct cpp_void_t{};

//! 用于抽取类型、类型对应的引用
template<typename T>
struct type_ref_traits_t;

//! 用于python 可选参数
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
//! 用于判断是否是可选参数
template<typename T>
struct pyoption_traits_t;

//! pytype_traits_t 封装 PyLong_FromLong 相关的操作，用于为调用python生成参数
template<typename T>
struct pytype_traits_t
{
    static PyObject* pyobj_from_cppobj(const char* const val_)
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

//! 用于调用python函数，生成tuple类型的python函数参数的工具类
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

//! 用于调用python函数，获取返回值的工具类
class pytype_tool_t
{
public:
    virtual ~pytype_tool_t(){};
    virtual int parse_value(PyObject *pvalue_) = 0;
    virtual const char* return_type() {return "";}
};

//! 用于调用python函数，获取返回值的工具泛型类
template<typename T>
class pytype_tool_impl_t;
//! 封装调用python函数的C API
struct pycall_t
{
    static int call_func(PyObject *pModule, const string& mod_name_, const string& func_name_,
                         pycall_arg_t& pyarg_, pytype_tool_t& pyret_, string& err_)
    {
        PyObject *pFunc = PyObject_GetAttrString(pModule, func_name_.c_str());
        if (pFunc && PyCallable_Check(pFunc)) {
            PyObject *pArgs = pyarg_.get_args();
            PyObject *pValue = PyObject_CallObject(pFunc, pArgs);
            pyarg_.release();//! 等价于Py_DECREF(pArgs);

            if (pValue != NULL) {
                if (pyret_.parse_value(pValue))
                {
                    err_ += "value returned is not ";
                    err_ += pyret_.return_type();
                    err_ += string(" ") + func_name_  + " in " + mod_name_;
                }
                Py_DECREF(pValue);
            }
        }
        else
        {
            err_ += "Cannot find function ";
            err_ += func_name_ + " in " + mod_name_ + ",";
        }

        Py_XDECREF(pFunc);
        if (PyErr_Occurred())
        {
            pyops_t::traceback(err_);
            return 0;
        }
        return 0;
    }
    static int call_func_obj(PyObject *pFunc, pycall_arg_t& pyarg_, pytype_tool_t& pyret_, string& err_)
    {
        if (pFunc && PyCallable_Check(pFunc)) {
            PyObject *pArgs = pyarg_.get_args();
            PyObject *pValue = PyObject_CallObject(pFunc, pArgs);
            pyarg_.release();//! 等价于Py_DECREF(pArgs);

            if (pValue != NULL) {
                if (pyret_.parse_value(pValue))
                {
                    err_ += "value returned is not ";
                    err_ += pyret_.return_type();
                }
                Py_DECREF(pValue);
            }
        }
        else
        {
            err_ += "invalid function";
        }

        if (PyErr_Occurred())
        {
            pyops_t::traceback(err_);
            return 0;
        }
        return 0;
    }
    template<typename T>
    static const T& call(const string& mod_name_, const string& func_name_, pycall_arg_t& pyarg_, pytype_tool_impl_t<T>& pyret);
    template<typename T>
    static const T& call_obj_method(PyObject *pObj, const string& func_name_, pycall_arg_t& pyarg_, pytype_tool_impl_t<T>& pyret);
    template<typename T>
    static const T& call_lambda(PyObject *pFunc, pycall_arg_t& pyarg_, pytype_tool_impl_t<T>& pyret);
};
//! 用于扩展python的工具类，用来解析参数
struct pyext_tool_t;


template<typename T>
struct pyext_return_tool_t;

//! 用于扩展python，生成pyobject类型的返回值给python
template <typename T>
struct pyext_func_traits_t;

//! 用于扩展python，traits出注册给python的函数接口
#ifndef PYCTOR
#define  PYCTOR int (*)
#endif
//! 表示void类型，由于void类型不能return，用void_ignore_t适配
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

//! 记录各个基类和子类的相互关系
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


//! 记录C++ class 对应到python中的名称、参数信息等,全局
struct static_pytype_info_t
{
    string class_name;
    string mod_name;
    int    total_args_num;
    PyTypeObject* pytype_def;
};

//! 工具类，用于生成分配python class的接口，包括分配、释放
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

//! 方便生成pyclass 初始化函数
template <typename CLASS_TYPE, typename CTOR>
struct pyclass_ctor_tool_t;

//! used to gen method of py class
template<typename T>
struct pyclass_method_gen_t;

//! 防止出现指针为NULL调用出错
#define  NULL_PTR_GUARD(X) if (NULL == X) {PyErr_SetString(PyExc_TypeError, "obj data ptr NULL");return NULL;}

//! 用于生成python 的getter和setter接口，适配于c++ class的成员变量
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

//! 用于C++ 注册class的工具类，会记录class对应的名称、成员方法、成员变量
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

    //! 静态类型需要全局记录该类型被注册成神马python 类型
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
//! 用于判断是否是可选参数
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
struct pytype_traits_t<const char* const>
{
    static PyObject* pyobj_from_cppobj(const char* const val_)
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


#endif
