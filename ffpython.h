#ifndef __FFPYTHON_H_
#define __FFPYTHON_H_

#ifdef _DEBUG
#undef _DEBUG
#include <python.h>
#define _DEBUG
#else
#include <python.h>
#endif
#include <structmember.h>

#include <string>
using namespace std;

#include "pyops_base.h"
#include "pyops_for_embed.h"
#include "pyops_for_extend.h"

class ffpython_t
{
public:
	typedef std::string (*ModInitFunc)(ffpython_t&);

    struct reg_info_t
    {
        reg_info_t():args_num(0),option_args_num(0),func_addr(0){}
        int  args_num;
        int  option_args_num;
		PTR_NUMBER func_addr;
        PyCFunction func;
        std::string func_name;
        std::string func_impl_name;
        std::string doc;
        std::string doc_impl;
    };

public:
    ffpython_t(ModInitFunc func = NULL);
    ~ffpython_t();

    static int init_py();
    static int final_py();

    static int add_path(const std::string& path_);

    static int run_string(const std::string& py_);
    static int reload(const std::string& py_name_);
    static int load(const std::string& py_name_);



    //! 注册static function，
    template<typename T>
    ffpython_t& reg(T func_, const std::string& func_name_, std::string doc_ = "")
    {
        reg_info_t tmp;
        tmp.args_num = pyext_func_traits_t<T>::args_num();
        tmp.option_args_num = pyext_func_traits_t<T>::option_args_num();
        tmp.func     = (PyCFunction)pyext_func_traits_t<T>::pyfunc;
        tmp.func_name= func_name_;
        tmp.func_impl_name = func_name_ + "_internal_";
        tmp.doc      = doc_;
        tmp.doc_impl = std::string("internal use, please call ") + func_name_;
        tmp.func_addr= (PTR_NUMBER)func_;
        
        m_func_info.push_back(tmp);
        return *this;
    }

    //! 注册c++ class
    template<typename T, typename CTOR>
    pyclass_regigster_tool_t& reg_class(const std::string& class_name_, std::string doc_ = "", std::string inherit_name_ = "")
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
        //! 注册析构函数,python若不调用析构函数,当对象被gc时自动调用
        tmp.delete_func = (PyCFunction)pyclass_base_info_t<T>::release;
            m_all_pyclass.push_back(tmp);

            return m_all_pyclass.back();
    }


    //! 调用python函数，最多支持9个参数
    template<typename RET>
    RET_V call(const std::string& mod_name_, const std::string& func_)
    {
        pycall_arg_t args(0);
        pytype_tool_impl_t<RET_V> pyret;
        return pycall_t::call<RET_V>(mod_name_, func_, args, pyret);
    }
    template<typename RET, typename ARG1>
    RET_V call(const std::string& mod_name_, const std::string& func_, const ARG1& a1)
    {
        pycall_arg_t args(1);
        args.add(a1);
        pytype_tool_impl_t<RET_V> pyret;
        return pycall_t::call<RET_V>(mod_name_, func_, args, pyret);
    }
    template<typename RET, typename ARG1, typename ARG2>
    RET_V call(const std::string& mod_name_, const std::string& func_, const ARG1& a1, const ARG2& a2)
    {
        pycall_arg_t args(2);
        args.add(a1).add(a2);
        pytype_tool_impl_t<RET_V> pyret;
        return pycall_t::call<RET_V>(mod_name_, func_, args, pyret);
    }
    template<typename RET, typename ARG1, typename ARG2, typename ARG3>
    RET_V call(const std::string& mod_name_, const std::string& func_, const ARG1& a1, const ARG2& a2, const ARG3& a3)
    {
        pycall_arg_t args(3);
        args.add(a1).add(a2).add(a3);
        pytype_tool_impl_t<RET_V> pyret;
        return pycall_t::call<RET_V>(mod_name_, func_, args, pyret);
    }
    template<typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4>
    RET_V call(const std::string& mod_name_, const std::string& func_, const ARG1& a1, const ARG2& a2, const ARG3& a3, const ARG4& a4)
    {
        pycall_arg_t args(4);
        args.add(a1).add(a2).add(a3).add(a4);
        pytype_tool_impl_t<RET_V> pyret;
        return pycall_t::call<RET_V>(mod_name_, func_, args, pyret);
    }
    template<typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5>
    RET_V call(const std::string& mod_name_, const std::string& func_, const ARG1& a1, const ARG2& a2, const ARG3& a3, const ARG4& a4,
                const ARG5& a5)
    {
        pycall_arg_t args(5);
        args.add(a1).add(a2).add(a3).add(a4).add(a5);
        pytype_tool_impl_t<RET_V> pyret;
        return pycall_t::call<RET_V>(mod_name_, func_, args, pyret);
    }
    template<typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6>
    RET_V call(const std::string& mod_name_, const std::string& func_, const ARG1& a1, const ARG2& a2, const ARG3& a3, const ARG4& a4,
                const ARG5& a5,const ARG6& a6)
    {
        pycall_arg_t args(6);
        args.add(a1).add(a2).add(a3).add(a4).add(a5).add(a6);
        pytype_tool_impl_t<RET_V> pyret;
        return pycall_t::call<RET_V>(mod_name_, func_, args, pyret);
    }
    template<typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7>
    RET_V call(const std::string& mod_name_, const std::string& func_, const ARG1& a1, const ARG2& a2, const ARG3& a3, const ARG4& a4,
                const ARG5& a5,const ARG6& a6,const ARG7& a7)
    {
        pycall_arg_t args(7);
        args.add(a1).add(a2).add(a3).add(a4).add(a5).add(a6).add(a7);
        pytype_tool_impl_t<RET_V> pyret;
        return pycall_t::call<RET_V>(mod_name_, func_, args, pyret);
    }
    template<typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7,
             typename ARG8>
    RET_V call(const std::string& mod_name_, const std::string& func_, const ARG1& a1, const ARG2& a2, const ARG3& a3, const ARG4& a4,
                const ARG5& a5, const ARG6& a6, const ARG7& a7, const ARG8& a8)
    {
        pycall_arg_t args(8);
        args.add(a1).add(a2).add(a3).add(a4).add(a5).add(a6).add(a7).add(a8);
        pytype_tool_impl_t<RET_V> pyret;
        return pycall_t::call<RET_V>(mod_name_, func_, args, pyret);
    }
    template<typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7,
             typename ARG8, typename ARG9>
    RET_V call(const std::string& mod_name_, const std::string& func_, const ARG1& a1, const ARG2& a2, const ARG3& a3, const ARG4& a4,
        const ARG5& a5, const ARG6& a6, const ARG7& a7, const ARG8& a8, const ARG9& a9)
    {
        pycall_arg_t args(9);
        args.add(a1).add(a2).add(a3).add(a4).add(a5).add(a6).add(a7).add(a8).add(a9);
        pytype_tool_impl_t<RET_V> pyret;
        return pycall_t::call<RET_V>(mod_name_, func_, args, pyret);
    }
    //!call python class method begin******************************************************************
    template<typename RET>
    RET_V obj_call(PyObject* pobj, const std::string& func_)
    {
        pycall_arg_t args(0);
        pytype_tool_impl_t<RET_V> pyret;
        return pycall_t::call_obj_method<RET_V>(pobj, func_, args, pyret);
    }
    template<typename RET, typename ARG1>
    RET_V obj_call(PyObject* pobj, const std::string& func_, const ARG1& a1)
    {
        pycall_arg_t args(1);
        args.add(a1);
        pytype_tool_impl_t<RET_V> pyret;
        return pycall_t::call_obj_method<RET_V>(pobj, func_, args, pyret);
    }
    template<typename RET, typename ARG1, typename ARG2>
    RET_V obj_call(PyObject* pobj, const std::string& func_, const ARG1& a1, const ARG2& a2)
    {
        pycall_arg_t args(2);
        args.add(a1).add(a2);
        pytype_tool_impl_t<RET_V> pyret;
        return pycall_t::call_obj_method<RET_V>(pobj, func_, args, pyret);
    }
    template<typename RET, typename ARG1, typename ARG2, typename ARG3>
    RET_V obj_call(PyObject* pobj, const std::string& func_, const ARG1& a1, const ARG2& a2, const ARG3& a3)
    {
        pycall_arg_t args(3);
        args.add(a1).add(a2).add(a3);
        pytype_tool_impl_t<RET_V> pyret;
        return pycall_t::call_obj_method<RET_V>(pobj, func_, args, pyret);
    }
    template<typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4>
    RET_V obj_call(PyObject* pobj, const std::string& func_, const ARG1& a1, const ARG2& a2, const ARG3& a3, const ARG4& a4)
    {
        pycall_arg_t args(4);
        args.add(a1).add(a2).add(a3).add(a4);
        pytype_tool_impl_t<RET_V> pyret;
        return pycall_t::call_obj_method<RET_V>(pobj, func_, args, pyret);
    }
    template<typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5>
    RET_V obj_call(PyObject* pobj, const std::string& func_, const ARG1& a1, const ARG2& a2, const ARG3& a3, const ARG4& a4,
                const ARG5& a5)
    {
        pycall_arg_t args(5);
        args.add(a1).add(a2).add(a3).add(a4).add(a5);
        pytype_tool_impl_t<RET_V> pyret;
        return pycall_t::call_obj_method<RET_V>(pobj, func_, args, pyret);
    }
    template<typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6>
    RET_V obj_call(PyObject* pobj, const std::string& func_, const ARG1& a1, const ARG2& a2, const ARG3& a3, const ARG4& a4,
                const ARG5& a5,const ARG6& a6)
    {
        pycall_arg_t args(6);
        args.add(a1).add(a2).add(a3).add(a4).add(a5).add(a6);
        pytype_tool_impl_t<RET_V> pyret;
        return pycall_t::call_obj_method<RET_V>(pobj, func_, args, pyret);
    }
    template<typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7>
    RET_V obj_call(PyObject* pobj, const std::string& func_, const ARG1& a1, const ARG2& a2, const ARG3& a3, const ARG4& a4,
                const ARG5& a5,const ARG6& a6,const ARG7& a7)
    {
        pycall_arg_t args(7);
        args.add(a1).add(a2).add(a3).add(a4).add(a5).add(a6).add(a7);
        pytype_tool_impl_t<RET_V> pyret;
        return pycall_t::call_obj_method<RET_V>(pobj, func_, args, pyret);
    }
    template<typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7,
             typename ARG8>
    RET_V obj_call(PyObject* pobj, const std::string& func_, const ARG1& a1, const ARG2& a2, const ARG3& a3, const ARG4& a4,
                const ARG5& a5, const ARG6& a6, const ARG7& a7, const ARG8& a8)
    {
        pycall_arg_t args(8);
        args.add(a1).add(a2).add(a3).add(a4).add(a5).add(a6).add(a7).add(a8);
        pytype_tool_impl_t<RET_V> pyret;
        return pycall_t::call_obj_method<RET_V>(pobj, func_, args, pyret);
    }
    template<typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7,
             typename ARG8, typename ARG9>
    RET_V obj_call(PyObject* pobj, const std::string& func_, const ARG1& a1, const ARG2& a2, const ARG3& a3, const ARG4& a4,
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
    RET_V call_lambda(PyObject* pobj, const std::string& func_, const ARG1& a1, const ARG2& a2, const ARG3& a3, const ARG4& a4,
        const ARG5& a5, const ARG6& a6, const ARG7& a7, const ARG8& a8, const ARG9& a9)
    {
        pycall_arg_t args(9);
        args.add(a1).add(a2).add(a3).add(a4).add(a5).add(a6).add(a7).add(a8).add(a9);
        pytype_tool_impl_t<RET_V> pyret;
        return pycall_t::call_lambda<RET_V>(pobj, args, pyret);
    }
    //!call python lambad function ennd ############################################################
    template<typename RET>
    RET_V get_global_var(const std::string& mod_name_, const std::string& var_name_)
    {
        PyObject *pName = NULL, *pModule = NULL;
        std::string err_msg;

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
    int set_global_var(const std::string& mod_name_, const std::string& var_name_, const T& val_)
    {
        PyObject *pName = NULL, *pModule = NULL;
        std::string err_msg;

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

    template<typename RET>
    RET_V getattr(PyObject* pModule, const std::string& var_name_)
    {
        std::string err_msg;
        if (NULL == pModule)
        {
            throw runtime_error("getattr object ptr null");
        }

        pytype_tool_impl_t<RET_V> pyret;
        PyObject *pvalue = PyObject_GetAttrString(pModule, var_name_.c_str());

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
	//! 将需要注册的函数、类型注册到python虚拟机
	int init(const std::string& mod_name_, std::string doc_ = "");
    void init_method();
	void rename_method();
    int init_pyclass();
	void rename_pyclass();
       

    bool is_method_exist(const std::vector<pyclass_regigster_tool_t::method_info_t>& src_, const std::string& new_);
    bool is_property_exist(const std::vector<pyclass_regigster_tool_t::property_info_t>& src_, const std::string& new_);
    pyclass_regigster_tool_t* get_pyclass_info_by_name(const std::string& name_);

private:
    std::string                              m_mod_name;
    std::string                              m_mod_doc;
    std::vector<PyMethodDef>                 m_pymethod_defs;
    std::vector<reg_info_t>                  m_func_info;
    
    //! reg class
    std::vector<pyclass_regigster_tool_t>    m_all_pyclass;
    //! cache some pyobject for optimize
    std::vector<PyObject*>                   m_cache_pyobject;
};


#endif
