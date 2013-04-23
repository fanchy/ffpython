#ifndef __EXTCLASS_H_
#define __EXTCLASS_H_

struct cpp_to_pyclass_reg_info_t
{
	static void add(const string& child_, const string& base_)
	{
		inherit_info[child_] = base_;
	}
	static bool is_child(const string& child_, const string& base_)
	{
		const string* c = &child_;
		for (size_t i = 0; i < inherit_info.size(); ++i)//! 避免循环继承
		{
			c = &(inherit_info[*c]);
			if (c->empty())
			{
				return false;
			}
			else if (*c == base_)
			{
				return true;
			}
		}
		return false;
	}
	static map<string, string> inherit_info;
};

struct static_pytype_info_t
{
    string class_name;
    string mod_name;
    int    total_args_num;
    PyTypeObject* pytype_def;
};

//! used to gen method of py ctor
template<typename T>
struct pyclass_base_info_t
{
	struct obj_data_t
	{
		obj_data_t():obj(NULL){}

		PyObject_HEAD
		T* obj;
		bool forbid_release;
	};
	
	static void free_obj(obj_data_t* self)
	{
		if  (false == self->forbid_release && self->obj)
		{
			delete self->obj;
			self->obj = NULL;
		}
		self->ob_type->tp_free((PyObject*)self);
	}
	
	static PyObject *alloc_obj(PyTypeObject *type, PyObject *args, PyObject *kwds)
	{
		obj_data_t *self = (obj_data_t *)type->tp_alloc(type, 0);
		return (PyObject *)self;
	}

    static static_pytype_info_t pytype_info;
};
template<typename T>
static_pytype_info_t pyclass_base_info_t<T>::pytype_info;

//! 方便生成pyclass 初始化函数
template <typename CLASS_TYPE, typename CTOR>
struct pyclass_ctor_tool_t;

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
//! used to gen method of py class
template<typename T>
struct pyclass_method_gen_t;

template <typename CLASS_TYPE, typename RET>
struct pyclass_member_func_gen_t
{
    typedef typename pyclass_base_info_t<CLASS_TYPE>::obj_data_t obj_data_t;
    typedef RET CLASS_TYPE::* property_ptr_t;

    static PyObject *getter_func(obj_data_t *self, void *closure)
    {
        property_ptr_t property_ptr;
        ::memcpy(&property_ptr, &closure, sizeof(closure));
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

	string		class_name;
	string		class_real_name;
	string		class_name_with_mod;
	string		class_reel_name_with_mod;
	string		inherit_name;
	int         type_size;
	string      doc;
    int         args_num;
    int         option_args_num;
	destructor  dector;
	initproc    init;
	pyobj_alloc_t ctor;

	//!  member functions
	vector<method_info_t>	methods_info;
    //! property 
    vector<property_info_t>	propertys_info;
	//! for init module
	PyTypeObject			pytype_def;
    //! method
	vector<PyMethodDef>		pymethod_def;
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
	static int option_args_num() { return pyoption_traits_t<type_ref_traits_t<ARG1>::value_t>::is();}

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
		return pyoption_traits_t<type_ref_traits_t<ARG1>::value_t>::is()+
				pyoption_traits_t<type_ref_traits_t<ARG2>::value_t>::is();
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
		return pyoption_traits_t<type_ref_traits_t<ARG1>::value_t>::is()+
				pyoption_traits_t<type_ref_traits_t<ARG2>::value_t>::is()+
				pyoption_traits_t<type_ref_traits_t<ARG3>::value_t>::is();
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
		return pyoption_traits_t<type_ref_traits_t<ARG1>::value_t>::is()+
				pyoption_traits_t<type_ref_traits_t<ARG2>::value_t>::is()+
				pyoption_traits_t<type_ref_traits_t<ARG3>::value_t>::is()+
				pyoption_traits_t<type_ref_traits_t<ARG4>::value_t>::is();
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
        return pyoption_traits_t<type_ref_traits_t<ARG1>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG2>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG3>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG4>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG5>::value_t>::is();
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
        return pyoption_traits_t<type_ref_traits_t<ARG1>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG2>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG3>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG4>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG5>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG6>::value_t>::is();
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
        return pyoption_traits_t<type_ref_traits_t<ARG1>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG2>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG3>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG4>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG5>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG6>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG7>::value_t>::is();
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
        return pyoption_traits_t<type_ref_traits_t<ARG1>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG2>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG3>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG4>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG5>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG6>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG7>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG8>::value_t>::is();
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
        return pyoption_traits_t<type_ref_traits_t<ARG1>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG2>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG3>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG4>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG5>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG6>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG7>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG8>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG9>::value_t>::is();
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
    static int option_args_num() { return pyoption_traits_t<type_ref_traits_t<ARG1>::value_t>::is();}

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
        return pyoption_traits_t<type_ref_traits_t<ARG1>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG2>::value_t>::is();
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
        return pyoption_traits_t<type_ref_traits_t<ARG1>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG2>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG3>::value_t>::is();
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
        return pyoption_traits_t<type_ref_traits_t<ARG1>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG2>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG3>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG4>::value_t>::is();
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
        return pyoption_traits_t<type_ref_traits_t<ARG1>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG2>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG3>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG4>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG5>::value_t>::is();
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
        return pyoption_traits_t<type_ref_traits_t<ARG1>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG2>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG3>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG4>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG5>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG6>::value_t>::is();
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
        return pyoption_traits_t<type_ref_traits_t<ARG1>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG2>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG3>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG4>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG5>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG6>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG7>::value_t>::is();
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
        return pyoption_traits_t<type_ref_traits_t<ARG1>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG2>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG3>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG4>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG5>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG6>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG7>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG8>::value_t>::is();
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
        return pyoption_traits_t<type_ref_traits_t<ARG1>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG2>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG3>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG4>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG5>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG6>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG7>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG8>::value_t>::is()+
            pyoption_traits_t<type_ref_traits_t<ARG9>::value_t>::is();
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

