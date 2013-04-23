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
using namespace std;

#ifdef _WIN32
#define  SAFE_SPRINTF   _snprintf_s
#else
#define  SAFE_SPRINTF   snprintf
#endif

#include "extclass.h"

//! 用于抽取类型、类型对应的引用
template<typename T>
struct type_ref_traits_t
{
	typedef T	value_t;
	typedef T&	ref_t;
	value_t		value; 
};
template<typename T>
struct type_ref_traits_t<T&>
{
	typedef T	value_t;
	typedef T&	ref_t;
	value_t		value; 
};
template<typename T>
struct type_ref_traits_t<const T&>
{
	typedef T	value_t;
	typedef T&	ref_t;
	value_t		value;
};

//! 用于python 可选参数
template<typename T>
struct pyoption_t
{
	typedef typename type_ref_traits_t<T>::value_t value_t;
	pyoption_t():m_set_flag(false){}
	bool is_set() const { return m_set_flag;}
	void set()			{ m_set_flag = true;}
	value_t&       value()    { return m_value;}
	const value_t& value() const{ return m_value;}
	
	const value_t& value(const value_t& default_)
	{
		if (is_set())
			return m_value;
		else
			return default_;
	}
	bool	m_set_flag;
	value_t m_value;
};

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

//! pytype_traits_t 封装 PyLong_FromLong 相关的操作，用于为调用python生成参数
template<typename T>
struct pytype_traits_t
{
    static PyObject* pyobj_from_cppobj(const T& val_)
    {
        return PyLong_FromLong(long(val_));
    }
    static int pyobj_to_cppobj(PyObject *pvalue_, T& m_ret)
    {
        if (true == PyLong_Check(pvalue_))
        {
            m_ret = (T)PyLong_AsLong(pvalue_);
            return 0;
        }
        else if (true == PyInt_Check(pvalue_))
        {
            m_ret = (T)PyInt_AsLong(pvalue_);
            return 0;
        }
        return -1;
    }
	static const char* get_typename() { return "long";}
};

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
            pyclass_base_info_t<T>::obj_data_t* pdest_obj = (pyclass_base_info_t<T>::obj_data_t*)palloc;
            //pdest_obj->obj = val_;
			::memcpy(&pdest_obj->obj, &val_, sizeof(pdest_obj->obj));
            pdest_obj->forbid_release = true;
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
        if (NULL == pysrc || !PyObject_TypeCheck(pysrc, pyclass_base_info_t<T>::pytype_info.pytype_def)) {
            Py_XDECREF(pysrc);
            return -1;
        }
        pyclass_base_info_t<T>::obj_data_t* pdest_obj = (pyclass_base_info_t<T>::obj_data_t*)pysrc;

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
struct pytype_traits_t<int>
{
    static PyObject* pyobj_from_cppobj(const int& val_)
    {
        return PyInt_FromLong(long(val_));
    }
    static int pyobj_to_cppobj(PyObject *pvalue_, int& m_ret)
    {
        if (true == PyLong_Check(pvalue_))
        {
            m_ret = (int)PyLong_AsLong(pvalue_);
            return 0;
        }
        else if (true == PyInt_Check(pvalue_))
        {
            m_ret = (int)PyInt_AsLong(pvalue_);
            return 0;
        }
        return -1;
    }
	static const char* get_typename() { return "int";}
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
        return PyString_FromString(val_.c_str());
    }
    static int pyobj_to_cppobj(PyObject *pvalue_, string& m_ret)
    {
        if (true == PyString_Check(pvalue_))
        {
            m_ret = PyString_AsString(pvalue_);
            return 0;
        }
        return -1;
    }
	static const char* get_typename() { return "string";}
};

template<>
struct pytype_traits_t<const char*>
{
    static PyObject* pyobj_from_cppobj(const char* val_)
    {
        return PyString_FromString(val_);
    }
    static int pyobj_to_cppobj(PyObject *pvalue_, const char*& m_ret)
    {
        if (true == PyString_Check(pvalue_))
        {
            m_ret = PyString_AsString(pvalue_);
            return 0;
        }
        return -1;
    }
	static const char* get_typename() { return "string";}
};

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
                T tmp();
                if (pytype_traits_t::pyobj_to_cppobj(PyTuple_GetItem(pvalue_, i), tmp))
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
                if (ret_tool.parse_value(PyList_GetItem(pvalue_, i)))
                {
                    return -1;
                }
                m_ret.push_back(tmp);
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
        for (list<T>::const_iterator it = val_.begin(); it != val_.end(); ++it)
        {
            PyList_SetItem(ret, i, pytype_traits_t<T>::pyobj_from_cppobj(*it));
        }
        return ret;
    }
    static int pyobj_to_cppobj(PyObject *pvalue_)
    {
        m_ret.clear();
        pytype_tool_impl_t<T> ret_tool;
        if (true == PyTuple_Check(pvalue_))
        {
            int n = PyTuple_Size(pvalue_);
            for (int i = 0; i < n; ++i)
            {
                T tmp();
                if (pytype_traits_t::pyobj_to_cppobj(PyTuple_GetItem(pvalue_, i), tmp))
                {
                    return -1;
                }
                m_ret.push_back(tmp);
            }
            return 0;
        }
        else if (true == PyList_Check(pvalue_))
        {
            int n = PyList_Size(pvalue_);
            for (int i = 0; i < n; ++i)
            {
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
        for (set<T>::const_iterator it = val_.begin(); it != val_.end(); ++it)
        {
            PyObject *v = pytype_traits_t<T>::pyobj_from_cppobj(*it);
            PySet_Add(ret, v);
            Py_DECREF(v);
        }
        return ret;
    }
    static int pyobj_to_cppobj(PyObject *pvalue_)
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

class pytype_tool_t
{
public:
    virtual ~pytype_tool_t(){};
    virtual int parse_value(PyObject *pvalue_) = 0;
    virtual const char* return_type() {return "";}
};

template<typename T>
class pytype_tool_impl_t;

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

struct pycall_t
{
    static int call_func(PyObject *pModule, const string& func_name_, pycall_arg_t& pyarg_, pytype_tool_t& pyret_, string& err_)
    {
        PyObject *pFunc = PyObject_GetAttrString(pModule, func_name_.c_str());
        if (pFunc && PyCallable_Check(pFunc)) {
            PyObject *pArgs = pyarg_.get_args();
            PyObject *pValue = PyObject_CallObject(pFunc, pArgs);
			pyarg_.release();//! 等价于Py_DECREF(pArgs);

            if (pValue != NULL) {
                if (pyret_.parse_value(pValue))
                {
                    err_ = "value returned is not ";
                    err_ += pyret_.return_type();
                }
                Py_DECREF(pValue);
            }
        }
        else
        {
            fprintf(stderr, "Cannot find function \"%s\"\n", func_name_.c_str());
        }

        Py_XDECREF(pFunc);
        if (PyErr_Occurred())
        {
            PyErr_Print();
            return 0;
        }
        return 0;
    }
    template<typename T>
    static const T& call(const string& file_name_, const string& func_name_, pycall_arg_t& pyarg_, pytype_tool_impl_t<T>& pyret)
    {
        PyObject *pName = NULL, *pModule = NULL;
        string err_msg;

        pName   = PyString_FromString(file_name_.c_str());
        pModule = PyImport_Import(pName);
        Py_DECREF(pName);
        if (NULL == pModule)
        {
            if (PyErr_Occurred())
                PyErr_Clear();
            err_msg = "Failed to load ";
            err_msg += file_name_;
            throw runtime_error(err_msg.c_str());
            return pyret.get_value();
        }

        call_func(pModule, func_name_, pyarg_, pyret, err_msg);
        Py_DECREF(pModule);

        if (!err_msg.empty())
        {
            if (PyErr_Occurred())
                PyErr_Clear();
            throw runtime_error(err_msg.c_str());
        }
        return pyret.get_value();
    }
};

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
    bool      m_err;//! 是否异常
    long      m_func_addr;
};


template<typename T>
struct pyext_return_tool_t;

template<typename T>
struct pyext_return_tool_t
{
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
	template<typename O, typename F>
	static PyObject* route_method_call(O o, F f)
	{
		return pytype_traits_t<T>::pyobj_from_cppobj((o->*f)());
	}
	template<typename O, typename F, typename ARG1>
	static PyObject* route_method_call(O o, F f, ARG1& a1)
	{
		return pytype_traits_t<T>::pyobj_from_cppobj((o->*f)(a1.value));
	}
	template<typename O, typename F, typename ARG1, typename ARG2>
	static PyObject* route_method_call(O o, F f, ARG1& a1, ARG2& a2)
	{
		return pytype_traits_t<T>::pyobj_from_cppobj((o->*f)(a1.value, a2.value));
	}
	template<typename O, typename F, typename ARG1, typename ARG2, typename ARG3>
	static PyObject* route_method_call(O o, F f, ARG1& a1, ARG2& a2, ARG3& a3)
	{
		return pytype_traits_t<T>::pyobj_from_cppobj((o->*f)(a1.value, a2.value, a3.value));
	}
	template<typename O, typename F, typename ARG1, typename ARG2, typename ARG3, typename ARG4>
	static PyObject* route_method_call(O o, F f, ARG1& a1, ARG2& a2, ARG3& a3, ARG4& a4)
	{
		return pytype_traits_t<T>::pyobj_from_cppobj((o->*f)(a1.value, a2.value, a3.value, a4.value));
	}
};

template<>
struct pyext_return_tool_t<void>
{
	template<typename F>
	static PyObject* route_call(F f)
	{
		f();
		return Py_RETURN_NONE;
	}
	template<typename F, typename ARG1>
	static PyObject* route_call(F f, ARG1& a1)
	{
		f(a1.value);
		return Py_RETURN_NONE;
	}
	template<typename F, typename ARG1, typename ARG2>
	static PyObject* route_call(F f, ARG1& a1, ARG2& a2)
	{
		f(a1.value, a2.value);
		return Py_RETURN_NONE;
	}
	template<typename F, typename ARG1, typename ARG2, typename ARG3>
	static PyObject* route_call(F f, ARG1& a1, ARG2& a2, ARG3& a3)
	{
		f(a1.value, a2.value, a3.value);
		return Py_RETURN_NONE;
	}
	template<typename F, typename ARG1, typename ARG2, typename ARG3, typename ARG4>
	static PyObject* route_call(F f, ARG1& a1, ARG2& a2, ARG3& a3, ARG4& a4)
	{
		f(a1.value, a2.value, a3.value, a4.value);
		return Py_RETURN_NONE;
	}
	template<typename O, typename F>
	static PyObject* route_method_call(O o, F f)
	{
		(o->*f)();
		return Py_RETURN_NONE;
	}
	template<typename O, typename F, typename ARG1>
	static PyObject* route_method_call(O o, F f, ARG1& a1)
	{
		(o->*f)(a1.value);
		return Py_RETURN_NONE;
	}
	template<typename O, typename F, typename ARG1, typename ARG2>
	static PyObject* route_method_call(O o, F f, ARG1& a1, ARG2& a2)
	{
		(o->*f)(a1.value, a2.value);
		return Py_RETURN_NONE;
	}
	template<typename O, typename F, typename ARG1, typename ARG2, typename ARG3>
	static PyObject* route_method_call(O o, F f, ARG1& a1, ARG2& a2, ARG3& a3)
	{
		(o->*f)(a1.value, a2.value, a3.value);
		return Py_RETURN_NONE;
	}
	template<typename O, typename F, typename ARG1, typename ARG2, typename ARG3, typename ARG4>
	static PyObject* route_method_call(O o, F f, ARG1& a1, ARG2& a2, ARG3& a3, ARG4& a4)
	{
		(o->*f)(a1.value, a2.value, a3.value, a4.value);
		return Py_RETURN_NONE;
	}
};
template <typename T>
struct pyext_func_traits_t;

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
		return pyoption_traits_t<type_ref_traits_t<ARG1>::value_t>::is();
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
		return pyoption_traits_t<type_ref_traits_t<ARG1>::value_t>::is() +
				pyoption_traits_t<type_ref_traits_t<ARG2>::value_t>::is();
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
		return pyoption_traits_t<type_ref_traits_t<ARG1>::value_t>::is() +
				pyoption_traits_t<type_ref_traits_t<ARG2>::value_t>::is() +
				pyoption_traits_t<type_ref_traits_t<ARG3>::value_t>::is();
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
		type_ref_traits_t<ARG2> a1;
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
		return pyoption_traits_t<type_ref_traits_t<ARG1>::value_t>::is() +
				pyoption_traits_t<type_ref_traits_t<ARG2>::value_t>::is() +
				pyoption_traits_t<type_ref_traits_t<ARG3>::value_t>::is() +
				pyoption_traits_t<type_ref_traits_t<ARG4>::value_t>::is();
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
		type_ref_traits_t<ARG2> a1;
		type_ref_traits_t<ARG3> a3;
		type_ref_traits_t<ARG4> a4;
		if (pyext_tool.parse_arg(a1.value).parse_arg(a2.value).parse_arg(a3.value).parse_arg(a4.value).is_err())
		{
			return NULL;
		}
		return pyext_return_tool_t<RET>::route_call(f, a1, a2, a3, a4);
    }
};

#ifndef PYCTOR
#define  PYCTOR int (*)
#endif

template<typename T>
struct void_ignore_t;

template<typename T>
struct void_ignore_t
{
    typedef T value_t;
};
#define  RET_V typename void_ignore_t<RET>::value_t

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
    ffpython_t(const string& mod_name_, string doc_ = ""):m_mod_name(mod_name_),m_mod_doc(doc_){}

    //! 注册static function，
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

    //! 将需要注册的函数、类型注册到python虚拟机
	int init();

	template<typename T, typename CTOR>
	pyclass_regigster_tool_t& reg_class(const string& class_name_, string doc_ = "", string inherit_name_ = "")
	{
        if (pyclass_base_info_t<T>::pytype_info.class_name.empty() == false)
            throw runtime_error("this type has been registed");

        pyclass_base_info_t<T>::pytype_info.class_name = class_name_;
        pyclass_base_info_t<T>::pytype_info.mod_name   = m_mod_name;
        pyclass_base_info_t<T>::pytype_info.total_args_num = pyext_func_traits_t<CTOR>::args_num() + 
                                                                     pyext_func_traits_t<CTOR>::option_args_num();

		pyclass_regigster_tool_t tmp;
		tmp.class_name		= class_name_;
		tmp.class_real_name	= class_name_ + "_internal_";
		tmp.inherit_name	= inherit_name_;
		tmp.doc             = doc_;
		tmp.dector			= (destructor)pyclass_base_info_t<T>::free_obj;
		tmp.init			= (initproc)pyclass_ctor_tool_t<T, CTOR>::init_obj;
		tmp.ctor			= pyclass_base_info_t<T>::alloc_obj;
		tmp.type_size		= sizeof(pyclass_base_info_t<T>::obj_data_t);
        tmp.args_num        = pyext_func_traits_t<CTOR>::args_num();
        tmp.option_args_num = pyext_func_traits_t<CTOR>::option_args_num();
        tmp.static_pytype_info = &(pyclass_base_info_t<T>::pytype_info);
		m_all_pyclass.push_back(tmp);
		return m_all_pyclass.back();
	}

    template<typename RET>
    RET_V call(const string& file_, const string& func_)
    {
        pycall_arg_t args(0);
        pytype_tool_impl_t<RET> pyret;
        return pycall_t::call<RET_V>(file_, func_, args, pyret);
    }
    template<typename RET, typename ARG1>
    RET_V call(const string& file_, const string& func_, const ARG1& a1)
    {
        pycall_arg_t args(1);
        args.add(a1);
        pytype_tool_impl_t<RET> pyret;
        return pycall_t::call<RET_V>(file_, func_, args, pyret);
    }
    template<typename RET, typename ARG1, typename ARG2>
    RET_V call(const string& file_, const string& func_, const ARG1& a1, const ARG2& a2)
    {
        pycall_arg_t args(2);
        args.add(a1).add(a2);
        pytype_tool_impl_t<RET> pyret;
        return pycall_t::call<RET_V>(file_, func_, args, pyret);
    }
    template<typename RET, typename ARG1, typename ARG2, typename ARG3>
    RET_V call(const string& file_, const string& func_, const ARG1& a1, const ARG2& a2, const ARG3& a3)
    {
        pycall_arg_t args(3);
        args.add(a1).add(a2).add(a3);
        pytype_tool_impl_t<RET> pyret;
        return pycall_t::call<RET_V>(file_, func_, args, pyret);
    }
    template<typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4>
    RET_V call(const string& file_, const string& func_, const ARG1& a1, const ARG2& a2, const ARG3& a3, const ARG4& a4)
    {
        pycall_arg_t args(4);
        args.add(a1).add(a2).add(a3).add(a4);
        pytype_tool_impl_t<RET> pyret;
        return pycall_t::call<RET_V>(file_, func_, args, pyret);
    }
    template<typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5>
    RET_V call(const string& file_, const string& func_, const ARG1& a1, const ARG2& a2, const ARG3& a3, const ARG4& a4,
                const ARG5& a5)
    {
        pycall_arg_t args(5);
        args.add(a1).add(a2).add(a3).add(a4).add(a5);
        pytype_tool_impl_t<RET> pyret;
        return pycall_t::call<RET_V>(file_, func_, args, pyret);
    }
    template<typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6>
    RET_V call(const string& file_, const string& func_, const ARG1& a1, const ARG2& a2, const ARG3& a3, const ARG4& a4,
                const ARG5& a5,const ARG6& a6)
    {
        pycall_arg_t args(6);
        args.add(a1).add(a2).add(a3).add(a4).add(a5).add(a6);
        pytype_tool_impl_t<RET> pyret;
        return pycall_t::call<RET_V>(file_, func_, args, pyret);
    }
    template<typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7>
    RET_V call(const string& file_, const string& func_, const ARG1& a1, const ARG2& a2, const ARG3& a3, const ARG4& a4,
                const ARG5& a5,const ARG6& a6,const ARG7& a7)
    {
        pycall_arg_t args(7);
        args.add(a1).add(a2).add(a3).add(a4).add(a5).add(a6).add(a7);
        pytype_tool_impl_t<RET> pyret;
        return pycall_t::call<RET_V>(file_, func_, args, pyret);
    }
    template<typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7,
             typename ARG8>
    RET_V call(const string& file_, const string& func_, const ARG1& a1, const ARG2& a2, const ARG3& a3, const ARG4& a4,
                const ARG5& a5, const ARG6& a6, const ARG7& a7, const ARG8& a8)
    {
        pycall_arg_t args(8);
        args.add(a1).add(a2).add(a3).add(a4).add(a5).add(a6).add(a7).add(a8);
        pytype_tool_impl_t<RET> pyret;
        return pycall_t::call<RET_V>(file_, func_, args, pyret);
    }
    template<typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7,
             typename ARG8, typename ARG9>
        RET_V call(const string& file_, const string& func_, const ARG1& a1, const ARG2& a2, const ARG3& a3, const ARG4& a4,
        const ARG5& a5, const ARG6& a6, const ARG7& a7, const ARG8& a8, const ARG9& a9)
    {
        pycall_arg_t args(9);
        args.add(a1).add(a2).add(a3).add(a4).add(a5).add(a6).add(a7).add(a8).add(a9);
        pytype_tool_impl_t<RET> pyret;
        return pycall_t::call<RET_V>(file_, func_, args, pyret);
    }
private:
    PyObject* init_method();
    int init_pyclass(PyObject* m, const string& mod_name_);

    bool is_method_exist(const vector<pyclass_regigster_tool_t::method_info_t>& src_, const string& new_);
    bool is_property_exist(const vector<pyclass_regigster_tool_t::property_info_t>& src_, const string& new_);
    pyclass_regigster_tool_t* get_pyclass_info_by_name(const string& name_);

private:
    string                              m_mod_name;
    string                              m_mod_doc;
	vector<PyMethodDef>					m_pymethod_defs;
	vector<reg_info_t>   				m_func_info;

	//! reg class
	vector<pyclass_regigster_tool_t>	m_all_pyclass;
	
};

int ffpython_t::init()
{
    PyObject* m = init_method();
    init_pyclass(m, m_mod_name);
    return 0;
}

PyObject* ffpython_t::init_method()
{
    string mod_name_ = m_mod_name;
    string doc_      = m_mod_doc;

    if (m_pymethod_defs.empty())
    {
        m_pymethod_defs.reserve(m_func_info.size() + 1);

        for (size_t i = 0; i < m_func_info.size(); ++i)
        {
            PyMethodDef tmp = {m_func_info[i].func_impl_name.c_str(), m_func_info[i].func,
                METH_VARARGS, m_func_info[i].doc_impl.c_str()};
            m_pymethod_defs.push_back(tmp);
        }
        PyMethodDef tmp = {NULL};
        m_pymethod_defs.push_back(tmp);
    }

    PyObject* m = Py_InitModule3(mod_name_.c_str(), &(m_pymethod_defs.front()), doc_.c_str());

    for (size_t i = 0; i < m_func_info.size(); ++i)
    {
        string pystr_args;
        string pystr_args_only_name;
        for (int j = 0; j < m_func_info[i].args_num; ++j)
        {
            stringstream ss;
            if (pystr_args.empty())
            {
                ss << "a" << (j+1);
                pystr_args += ss.str();
            }
            else
            {
                ss << ", a" << (j+1);
                pystr_args += ss.str();
            }
        }
        pystr_args_only_name = pystr_args;
        for (int j = 0; j < m_func_info[i].option_args_num; ++j)
        {
            stringstream ss;
            if (pystr_args.empty())
            {
                ss << "a" << (m_func_info[i].args_num + j+1);
                string tmp =  ss.str();
                pystr_args_only_name += tmp;
                pystr_args += tmp + " = None";
            }
            else
            {
                ss << ", a" << (m_func_info[i].args_num + j+1);
                string tmp =  ss.str();
                pystr_args_only_name += tmp;
                pystr_args += tmp + " = None";
            }
        }
        if (!pystr_args_only_name.empty())
            pystr_args_only_name += ",";

        char buff[1024];
        SAFE_SPRINTF(buff, sizeof(buff), 
            "def %s_%s(%s):\n"
            "\t'''%s'''\n"
            "\treturn %s.%s(%ld,%s)\n"
            "import %s\n"
            "%s.%s = %s_%s\n"
            "%s_%s = None\n",
            mod_name_.c_str(), m_func_info[i].func_name.c_str(), pystr_args.c_str(),
            m_func_info[i].doc.c_str(), 
            mod_name_.c_str(), m_func_info[i].func_impl_name.c_str(), m_func_info[i].func_addr, pystr_args_only_name.c_str(),
            mod_name_.c_str(),
            mod_name_.c_str(), m_func_info[i].func_name.c_str(), mod_name_.c_str(), m_func_info[i].func_name.c_str(),
            mod_name_.c_str(), m_func_info[i].func_name.c_str()
            );

        //printf(buff);
        PyRun_SimpleString(buff);
    }

    return m;
}

int ffpython_t::init_pyclass(PyObject* m, const string& mod_name_)
{
    for (size_t i = 0; i < m_all_pyclass.size(); ++i)
    {
        if (false == m_all_pyclass[i].inherit_name.empty())//! 存在基类
        {
            pyclass_regigster_tool_t* inherit_class = get_pyclass_info_by_name(m_all_pyclass[i].inherit_name);
            assert(inherit_class && "base class must be registed");
            for (size_t n = 0; n < inherit_class->methods_info.size(); ++n)
            {
                const string& method_name = inherit_class->methods_info[n].func_name;
                if (false == is_method_exist(m_all_pyclass[i].methods_info, method_name))
                {
                    m_all_pyclass[i].methods_info.push_back(inherit_class->methods_info[n]);
                }
            }
            for (size_t n = 0; n < inherit_class->propertys_info.size(); ++n)
            {
                const string& property_name = inherit_class->propertys_info[n].property_name;
                if (false == is_property_exist(m_all_pyclass[i].propertys_info, property_name))
                {
                    m_all_pyclass[i].propertys_info.push_back(inherit_class->propertys_info[n]);
                }
            }
        }
        //! init class property
        for (size_t j = 0; j < m_all_pyclass[i].propertys_info.size(); ++j)
        {
            PyGetSetDef tmp = {(char*)m_all_pyclass[i].propertys_info[j].property_name.c_str(),
                m_all_pyclass[i].propertys_info[j].getter_func, 
                m_all_pyclass[i].propertys_info[j].setter_func,
                (char*)m_all_pyclass[i].propertys_info[j].doc.c_str(),
                m_all_pyclass[i].propertys_info[j].ptr
            };
            m_all_pyclass[i].pyproperty_def.push_back(tmp);
        }
        PyGetSetDef tmp_property_def = {NULL};
        m_all_pyclass[i].pyproperty_def.push_back(tmp_property_def);
        //! init class method
        for (size_t j = 0; j < m_all_pyclass[i].methods_info.size(); ++j)
        {
            PyMethodDef tmp = {m_all_pyclass[i].methods_info[j].func_real_name.c_str(),
                m_all_pyclass[i].methods_info[j].func, 
                METH_VARARGS,
                m_all_pyclass[i].methods_info[j].doc.c_str()
            };
            m_all_pyclass[i].pymethod_def.push_back(tmp);

        }

        PyMethodDef tmp_method_def = {NULL};
        m_all_pyclass[i].pymethod_def.push_back(tmp_method_def);

        m_all_pyclass[i].class_name_with_mod = mod_name_ + "." + m_all_pyclass[i].class_name;
        m_all_pyclass[i].class_reel_name_with_mod = mod_name_ + "." + m_all_pyclass[i].class_real_name;

        PyTypeObject tmp_pytype_def = 
        {
            PyObject_HEAD_INIT(NULL)
            0,                         /*ob_size*/
            m_all_pyclass[i].class_reel_name_with_mod.c_str(),             /*tp_name*/
            m_all_pyclass[i].type_size,             /*tp_size*/
            0,                         /*tp_itemsize*/
            (destructor)m_all_pyclass[i].dector, /*tp_dealloc*/
            0,                         /*tp_print*/
            0,                         /*tp_getattr*/
            0,                         /*tp_setattr*/
            0,                         /*tp_compare*/
            0,                         /*tp_repr*/
            0,                         /*tp_as_number*/
            0,                         /*tp_as_sequence*/
            0,                         /*tp_as_mapping*/
            0,                         /*tp_hash */
            0,                         /*tp_call*/
            0,                         /*tp_str*/
            0,                         /*tp_getattro*/
            0,                         /*tp_setattro*/
            0,                         /*tp_as_buffer*/
            Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
            m_all_pyclass[i].doc.c_str(),           /* tp_doc */
            0,		               /* tp_traverse */
            0,		               /* tp_clear */
            0,		               /* tp_richcompare */
            0,		               /* tp_weaklistoffset */
            0,		               /* tp_iter */
            0,		               /* tp_iternext */
            &(m_all_pyclass[i].pymethod_def.front()),//Noddy_methods,             /* tp_methods */
            0,//Noddy_members,             /* tp_members */
            &(m_all_pyclass[i].pyproperty_def.front()),//Noddy_getseters,           /* tp_getset */
            0,                         /* tp_base */
            0,                         /* tp_dict */
            0,                         /* tp_descr_get */
            0,                         /* tp_descr_set */
            0,                         /* tp_dictoffset */
            (initproc)m_all_pyclass[i].init,      /* tp_init */
            0,                         /* tp_alloc */
            m_all_pyclass[i].ctor,                 /* tp_new */
        };
        m_all_pyclass[i].pytype_def = tmp_pytype_def;
        m_all_pyclass[i].static_pytype_info->pytype_def = &m_all_pyclass[i].pytype_def;

        if (PyType_Ready(&m_all_pyclass[i].pytype_def) < 0)
            return -1;
        Py_INCREF(&m_all_pyclass[i].pytype_def);
        PyModule_AddObject(m, m_all_pyclass[i].class_real_name.c_str(), (PyObject *)&m_all_pyclass[i].pytype_def);

        stringstream str_def_args;
        stringstream str_init_args;
        for (int a = 0; a < m_all_pyclass[i].args_num; ++a)
        {
            str_def_args << "a"<<(a+1)<<",";
            str_init_args << "a"<<(a+1)<<",";
        }
        for (int b = 0; b < m_all_pyclass[b].option_args_num; ++b)
        {
            str_def_args << "a"<<(m_all_pyclass[i].args_num+ b+1)<<" = None,";
            str_init_args << "a"<<(m_all_pyclass[i].args_num+ b+1)<<",";
        }

        char buff[1024];
        SAFE_SPRINTF(buff, sizeof(buff), 
            "import %s\n"
            "class %s_:\n"
            "\t'''%s'''\n"
            "\tdef __init__(self, %s assign_obj_ = 0):\n"//! 定义init函数
            "\t\t'''%s'''\n"
            "\t\tif True == isinstance(assign_obj_, %s):\n"
            "\t\t\tself.obj = assign_obj_\n"
            "\t\t\treturn\n"
            "\t\tself.obj = %s(0,(%s))\n"

            ,mod_name_.c_str(),
            m_all_pyclass[i].class_name.c_str(),
            "",//! TODO
            str_def_args.str().c_str(),
            "",//! TODO
            m_all_pyclass[i].class_reel_name_with_mod.c_str(),
            m_all_pyclass[i].class_reel_name_with_mod.c_str(), str_init_args.str().c_str()
            );

        string gen_class_str = buff;
        //! 增加属性
        for (size_t c = 0; c < m_all_pyclass[i].propertys_info.size(); ++c)
        {
            SAFE_SPRINTF(buff, sizeof(buff), 
                "\tdef get_%s(self):\n"
                "\t\treturn self.obj.%s\n"
                "\tdef set_%s(self, v):\n"
                "\t\tself.obj.%s = v\n"
                "\t@property\n"
                "\tdef %s(self):\n"
                "\t\treturn self.obj.%s\n"
                "\t@%s.setter\n"
                "\tdef %s(self, v):\n"
                "\t\tself.obj.%s = v\n",
                m_all_pyclass[i].propertys_info[c].property_name.c_str(),
                m_all_pyclass[i].propertys_info[c].property_name.c_str(),
                m_all_pyclass[i].propertys_info[c].property_name.c_str(),
                m_all_pyclass[i].propertys_info[c].property_name.c_str(),
                m_all_pyclass[i].propertys_info[c].property_name.c_str(),
                m_all_pyclass[i].propertys_info[c].property_name.c_str(),
                m_all_pyclass[i].propertys_info[c].property_name.c_str(),
                m_all_pyclass[i].propertys_info[c].property_name.c_str(),
                m_all_pyclass[i].propertys_info[c].property_name.c_str()
                );
            gen_class_str += buff;
        }

        for (size_t m = 0; m < m_all_pyclass[i].methods_info.size(); ++m)
        {
            string pystr_args;
            string pystr_args_only_name;
            for (int j = 0; j < m_all_pyclass[i].methods_info[m].args_num; ++j)
            {
                stringstream ss;
                if (pystr_args.empty())
                {
                    ss << "a" << (j+1);
                    pystr_args += ss.str();
                }
                else
                {
                    ss << ", a" << (j+1);
                    pystr_args += ss.str();
                }
            }
            pystr_args_only_name = pystr_args;
            for (int j = 0; j < m_all_pyclass[i].methods_info[m].option_args_num; ++j)
            {
                stringstream ss;
                if (pystr_args.empty())
                {
                    ss << "a" << (m_all_pyclass[i].methods_info[m].args_num + j+1);
                    string tmp =  ss.str();
                    pystr_args_only_name += tmp;
                    pystr_args += tmp + " = None";
                }
                else
                {
                    ss << ", a" << (m_all_pyclass[i].methods_info[m].args_num + j+1);
                    string tmp =  ss.str();
                    pystr_args_only_name += tmp;
                    pystr_args += tmp + " = None";
                }
            }
            if (!pystr_args_only_name.empty())
                pystr_args_only_name += ",";

            SAFE_SPRINTF(buff, sizeof(buff), 
                "\tdef %s(self,%s):\n"
                "\t\t'''%s'''\n"
                "\t\treturn self.obj.%s(%ld,(%s))\n"
                ,m_all_pyclass[i].methods_info[m].func_name.c_str(), pystr_args.c_str(),
                m_all_pyclass[i].methods_info[m].doc.c_str(),
                m_all_pyclass[i].methods_info[m].func_real_name.c_str(), m_all_pyclass[i].methods_info[m].func_addr, pystr_args_only_name.c_str()
                );
            gen_class_str += buff;
        }
        SAFE_SPRINTF(buff, sizeof(buff), 
            "%s.%s = %s_\n"
            "%s_ = None\n",
            mod_name_.c_str(), m_all_pyclass[i].class_name.c_str(), m_all_pyclass[i].class_name.c_str(),
            m_all_pyclass[i].class_name.c_str()
            );
        gen_class_str += buff;
        //printf(gen_class_str.c_str());
        PyRun_SimpleString(gen_class_str.c_str());
    }
    return 0;
}
bool ffpython_t::is_method_exist(const vector<pyclass_regigster_tool_t::method_info_t>& src_, const string& new_)
{
    for (size_t i = 0; i < src_.size(); ++i)
    {
        if (new_ == src_[i].func_name)
        {
            return true;
        }
    }
    return false;
}
bool ffpython_t::is_property_exist(const vector<pyclass_regigster_tool_t::property_info_t>& src_, const string& new_)
{
    for (size_t i = 0; i < src_.size(); ++i)
    {
        if (new_ == src_[i].property_name)
        {
            return true;
        }
    }
    return false;
}
pyclass_regigster_tool_t* ffpython_t::get_pyclass_info_by_name(const string& name_)
{
    for (size_t i = 0; i < m_all_pyclass.size(); ++i)
    {
        if (m_all_pyclass[i].class_name == name_)
        {
            return &(m_all_pyclass[i]);
        }
    }
    return NULL;
}

#endif
