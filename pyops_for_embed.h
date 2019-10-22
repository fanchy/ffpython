#ifndef  __PYOPS_FOR_EMBED
#define  __PYOPS_FOR_EMBED

#include "pyops_base.h"





template<typename T>
const T& pycall_t::call(const std::string& mod_name_, const std::string& func_name_, pycall_arg_t& pyarg_, pytype_tool_impl_t<T>& pyret)
{
    PyObject *pName = NULL, *pModule = NULL;
    std::string err_msg;

    pName   = PyString_FromString(mod_name_.c_str());
    pModule = PyImport_Import(pName);
    Py_DECREF(pName);
    if (NULL == pModule)
    {
        pyops_t::traceback(err_msg);
        throw std::runtime_error(err_msg.c_str());
        return pyret.get_value();
    }

    call_func(pModule, mod_name_, func_name_, pyarg_, pyret, err_msg);
    Py_DECREF(pModule);

    if (!err_msg.empty())
    {
        throw std::runtime_error(err_msg.c_str());
    }
    return pyret.get_value();
}
template<typename T>
const T& pycall_t::call_obj_method(PyObject *pObj, const std::string& func_name_, pycall_arg_t& pyarg_, pytype_tool_impl_t<T>& pyret)
{
    std::string err_msg;
    if (NULL == pObj)
    {
        pyops_t::traceback(err_msg);
        throw std::runtime_error(err_msg.c_str());
        return pyret.get_value();
    }

    static std::string mod_name_ = "NaN";
    call_func(pObj, mod_name_, func_name_, pyarg_, pyret, err_msg);

    if (!err_msg.empty())
    {
        throw std::runtime_error(err_msg.c_str());
    }
    return pyret.get_value();
}

template<typename T>
const T& pycall_t::call_lambda(PyObject *pFunc, pycall_arg_t& pyarg_, pytype_tool_impl_t<T>& pyret)
{
    std::string err_msg;
    if (NULL == pFunc)
    {
        err_msg = "can not call null PyObject";
        throw std::runtime_error(err_msg.c_str());
        return pyret.get_value();
    }

    call_func_obj(pFunc, pyarg_, pyret, err_msg);

    if (!err_msg.empty())
    {
        throw std::runtime_error(err_msg.c_str());
    }
    return pyret.get_value();
}


#endif
