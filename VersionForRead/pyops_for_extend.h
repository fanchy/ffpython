#ifndef  __PYOPS_FOR_EXTEND_
#define  __PYOPS_FOR_EXTEND_

#include "pyops_base.h"



//! 用于扩展python的工具类，用来解析参数
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
        ::memcpy((void*)&f, (const void*)&m_func_addr, sizeof(m_func_addr));
        return f;
    }
    PyObject* m_args;
    PyObject* m_arg_tuple;
    int       m_index;
    int       m_size;
    bool      m_err;//! 是否异常
    long      m_func_addr;
};



//! 用于扩展python，生成pyobject类型的返回值给python
template<typename T>
struct pyext_return_tool_t
{
    //! 用于静态方法
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
    //! 用于成员方法
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



//! 用于扩展python，traits出注册给python的函数接口
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

//! const类型成员函数---------------------------------------------------------------------------------------------

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
