#include "ffpython.h"


//! 获取python异常信息

int pyops_t::traceback(std::string& ret_)
{
    PyObject* err = PyErr_Occurred();

    if (err != NULL) {
        PyObject *ptype = NULL, *pvalue = NULL, *ptraceback = NULL;
        PyObject *pyth_module = NULL, *pyth_func = NULL;

        PyErr_Fetch(&ptype, &pvalue, &ptraceback);
        if (pvalue)
        {
            if (true == PyList_Check(pvalue))
            {
                int64_t n = PyList_Size(pvalue);
                for (int64_t i = 0; i < n; ++i)
                {
                    PyObject *pystr = PyObject_Str(PyList_GetItem(pvalue, i));
                    ret_ += PyString_AsString(pystr);
                    ret_ += "\n";
                    Py_DECREF(pystr);
                }
            }
            if (true == PyTuple_Check(pvalue))
            {
				int64_t n = PyTuple_Size(pvalue);
                for (int64_t i = 0; i < n; ++i)
                {
                    PyObject* tmp_str = PyTuple_GetItem(pvalue, i);
                    if (true == PyTuple_Check(tmp_str))
                    {
						int64_t m = PyTuple_Size(tmp_str);
                        for (int64_t j = 0; j < m; ++j)
                        {
                            PyObject *pystr = PyObject_Str(PyTuple_GetItem(tmp_str, j));
                            ret_ += PyString_AsString(pystr);
                            ret_ += ",";
                            Py_DECREF(pystr);
                        }
                    }
                    else
                    {
                        PyObject *pystr = PyObject_Str(tmp_str);
                        ret_ += PyString_AsString(pystr);
                        Py_DECREF(pystr);
                    }
                    ret_ += "\n";
                }
            }
            else
            {
                PyObject *pystr = PyObject_Str(pvalue);
                if (pystr)
                {
                    ret_ += PyString_AsString(pystr);
                    ret_ += "\n";
                    Py_DECREF(pystr);
                }
            }
        }

        /* See if we can get a full traceback */
        PyObject *module_name = PyString_FromString("traceback");
        pyth_module = PyImport_Import(module_name);
        Py_DECREF(module_name);

        if (pyth_module && ptype && pvalue && ptraceback)
        {
            pyth_func = PyObject_GetAttrString(pyth_module, "format_exception");
            if (pyth_func && PyCallable_Check(pyth_func)) {
                PyObject *pyth_val = PyObject_CallFunctionObjArgs(pyth_func, ptype, pvalue, ptraceback, NULL);
                if (pyth_val && true == PyList_Check(pyth_val))
                {
					int64_t n = PyList_Size(pyth_val);
                    for (int64_t i = 0; i < n; ++i)
                    {
                        PyObject* tmp_str = PyList_GetItem(pyth_val, i);
                        PyObject *pystr = PyObject_Str(tmp_str);
                        if (pystr)
                        {
                            ret_ += PyString_AsString(pystr);

                            Py_DECREF(pystr);
                        }
                        ret_ += "\n";
                    }
                }
                Py_XDECREF(pyth_val);
            }
        }
        Py_XDECREF(pyth_func);
        Py_XDECREF(pyth_module);
        Py_XDECREF(ptype);
        Py_XDECREF(pvalue);
        Py_XDECREF(ptraceback);
        PyErr_Clear();
        return 0;
    }

    return -1;
}

ffpython_t::ffpython_t(ModInitFunc func)
{
	if (func) {
		std::string modName = func(*this);
		init(modName);
	}
	if (!Py_IsInitialized())
		Py_Initialize();
	rename_method();
	rename_pyclass();
}
ffpython_t::~ffpython_t()
{
    clear_cache_pyobject();
}


int ffpython_t::init_py()
{
    Py_Initialize();
    return 0;
}
int ffpython_t::final_py()
{
    Py_Finalize();
    return 0;
}

int ffpython_t::add_path(const std::string& path_)
{
    char buff[1024];
    SAFE_SPRINTF(buff, sizeof(buff), "import sys\nif '%s' not in sys.path:\n\tsys.path.append('%s')\n", path_.c_str(), path_.c_str());
    PyRun_SimpleString(buff);
    return 0;
}

int ffpython_t::run_string(const std::string& py_)
{
    PyRun_SimpleString(py_.c_str());
    return 0;
}

int ffpython_t::reload(const std::string& py_name_)
{
    PyObject *pName = NULL, *pModule = NULL;
    std::string err_msg;

    pName   = PyString_FromString(py_name_.c_str());
    pModule = PyImport_Import(pName);
    Py_DECREF(pName);
    if (NULL == pModule)
    {
        pyops_t::traceback(err_msg);
        throw runtime_error(err_msg.c_str());
        return -1;
    }

    PyObject *pNewMod = PyImport_ReloadModule(pModule);
    Py_DECREF(pModule);
    if (NULL == pNewMod)
    {
        pyops_t::traceback(err_msg);
        throw runtime_error(err_msg.c_str());
        return -1;
    }
    Py_DECREF(pNewMod);   
    return 0;
}
int ffpython_t::load(const std::string& py_name_)
{
    PyObject *pName = NULL, *pModule = NULL;
    std::string err_msg;

    pName   = PyString_FromString(py_name_.c_str());
    pModule = PyImport_Import(pName);
    Py_DECREF(pName);
    if (NULL == pModule)
    {
        pyops_t::traceback(err_msg);
        throw runtime_error(err_msg.c_str());
        return -1;
    }

    Py_DECREF(pModule);
    return 0;
}

int ffpython_t::init(const std::string& mod_name_, std::string doc_)
{
    m_mod_name = mod_name_;
    m_mod_doc  = doc_;
    init_method();
    init_pyclass();

	static std::vector<PyMethodDef>* vtmp;
	static std::vector<pyclass_regigster_tool_t>* all_pyclass;
	vtmp = &m_pymethod_defs;
	all_pyclass = &m_all_pyclass;
	static PyModuleDef EmbModule = {
		PyModuleDef_HEAD_INIT, m_mod_name.c_str(), NULL, -1, &((*vtmp)[0]),
		NULL, NULL, NULL, NULL
	};
	struct TmpLambda {
		static PyObject* PyInit_emb(void)
		{
			PyObject* retObj = PyModule_Create(&EmbModule);

			for (size_t i = 0; i < (*all_pyclass).size(); ++i)
			{
				if (PyType_Ready(&((*all_pyclass)[i].pytype_def)) < 0)
					return retObj;
				PyObject* tmpP = (PyObject*)(&((*all_pyclass)[i].pytype_def));
				Py_INCREF(tmpP);
				PyModule_AddObject(retObj, (*all_pyclass)[i].class_real_name.c_str(), (PyObject *)&(*all_pyclass)[i].pytype_def);
			}

			return retObj;
		}
	};
	PyImport_AppendInittab(m_mod_name.c_str(), TmpLambda::PyInit_emb);

    return 0;
}

void ffpython_t::init_method()
{
	std::string mod_name_ = m_mod_name;
	std::string doc_ = m_mod_doc;

	if (m_pymethod_defs.empty())
	{
		m_pymethod_defs.reserve(m_func_info.size() + 1);

		for (size_t i = 0; i < m_func_info.size(); ++i)
		{
			PyMethodDef tmp = { m_func_info[i].func_impl_name.c_str(), m_func_info[i].func,
				METH_VARARGS, m_func_info[i].doc_impl.c_str() };
			m_pymethod_defs.push_back(tmp);
		}
		PyMethodDef tmp = { NULL };
		m_pymethod_defs.push_back(tmp);
	}

	return;
}
void ffpython_t::rename_method()
{
    for (size_t i = 0; i < m_func_info.size(); ++i)
    {
        std::string pystr_args;
        std::string pystr_args_only_name;
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
                std::string tmp =  ss.str();
                pystr_args_only_name += tmp;
                pystr_args += tmp + " = None";
            }
            else
            {
                ss << ", a" << (m_func_info[i].args_num + j+1);
                std::string tmp =  ss.str();
                pystr_args_only_name += tmp;
                pystr_args += tmp + " = None";
            }
        }
        if (!pystr_args_only_name.empty())
            pystr_args_only_name += ",";

		std::ostringstream oss;//创建一个流
		oss << m_func_info[i].func_addr;//把值传递如流中
		std::string funcAddrStr = oss.str();//获取转换后的字符转并将其写入result

        char buff[1024];
        SAFE_SPRINTF(buff, sizeof(buff), 
            "_tmp_ff_ = None\nif '%s' in globals():\n\t_tmp_ff_ = globals()['%s']\n"
            "def %s(%s):\n"
            "\t'''%s'''\n"
            "\treturn %s.%s(%s,(%s))\n"
            "import %s\n"
            "%s.%s = %s\n"
            "%s = None\n"
            "if _tmp_ff_:\n\tglobals()['%s'] = _tmp_ff_\n_tmp_ff_ = None\n",
            m_func_info[i].func_name.c_str(), m_func_info[i].func_name.c_str(), 
            m_func_info[i].func_name.c_str(), pystr_args.c_str(),
            m_func_info[i].doc.c_str(), 
            m_mod_name.c_str(), m_func_info[i].func_impl_name.c_str(), funcAddrStr.c_str(), pystr_args_only_name.c_str(),
            m_mod_name.c_str(),
            m_mod_name.c_str(), m_func_info[i].func_name.c_str(), m_func_info[i].func_name.c_str(),
            m_func_info[i].func_name.c_str(),
            m_func_info[i].func_name.c_str()
            );

        //printf(buff);
        PyRun_SimpleString(buff);
    }
}

int ffpython_t::init_pyclass()
{
    for (size_t i = 0; i < m_all_pyclass.size(); ++i)
    {
        m_all_pyclass[i].static_pytype_info->mod_name = m_mod_name;
        if (false == m_all_pyclass[i].inherit_name.empty())
        {
            pyclass_regigster_tool_t* inherit_class = get_pyclass_info_by_name(m_all_pyclass[i].inherit_name);
            assert(inherit_class && "base class must be registed");
            for (size_t n = 0; n < inherit_class->methods_info.size(); ++n)
            {
                const std::string& method_name = inherit_class->methods_info[n].func_name;
                if (false == is_method_exist(m_all_pyclass[i].methods_info, method_name))
                {
                    m_all_pyclass[i].methods_info.push_back(inherit_class->methods_info[n]);
                }
            }
            for (size_t n = 0; n < inherit_class->propertys_info.size(); ++n)
            {
                const std::string& property_name = inherit_class->propertys_info[n].property_name;
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
        PyMethodDef tmp_del = {"delete",
            m_all_pyclass[i].delete_func,
            METH_VARARGS,
            "delete obj"
        };
        m_all_pyclass[i].pymethod_def.push_back(tmp_del);
        PyMethodDef tmp_method_def = {NULL};
        m_all_pyclass[i].pymethod_def.push_back(tmp_method_def);

        m_all_pyclass[i].class_name_with_mod = m_mod_name + "." + m_all_pyclass[i].class_name;
        m_all_pyclass[i].class_reel_name_with_mod = m_mod_name + "." + m_all_pyclass[i].class_real_name;

        PyTypeObject tmp_pytype_def = 
        {
			PyVarObject_HEAD_INIT(NULL, 0)
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
            0,                               /* tp_traverse */
            0,                               /* tp_clear */
            0,                               /* tp_richcompare */
            0,                               /* tp_weaklistoffset */
            0,                               /* tp_iter */
            0,                               /* tp_iternext */
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
        cpp_to_pyclass_reg_info_t::add(m_all_pyclass[i].class_name, m_all_pyclass[i].inherit_name, &m_all_pyclass[i].pytype_def);
    }

    return 0;
}

void ffpython_t::rename_pyclass()
{
	for (size_t i = 0; i < m_all_pyclass.size(); ++i)
	{
		stringstream str_def_args;
		stringstream str_init_args;
		for (int a = 0; a < m_all_pyclass[i].args_num; ++a)
		{
			str_def_args << "a" << (a + 1) << ",";
			str_init_args << "a" << (a + 1) << ",";
		}
		for (int b = 0; b < m_all_pyclass[b].option_args_num; ++b)
		{
			str_def_args << "a" << (m_all_pyclass[i].args_num + b + 1) << " = None,";
			str_init_args << "a" << (m_all_pyclass[i].args_num + b + 1) << ",";
		}

		char buff[1024];
		SAFE_SPRINTF(buff, sizeof(buff),
			"_tmp_ff_ = None\nif '%s' in globals():\n\t_tmp_ff_ = globals()['%s']\n"
			"import %s\n"
			"class %s(object):\n"
			"\t'''%s'''\n"
			"\tdef __init__(self, %s assign_obj_ = 0):\n"
			"\t\t'''%s'''\n"
			"\t\tif True == isinstance(assign_obj_, %s):\n"
			"\t\t\tself.obj = assign_obj_\n"
			"\t\t\treturn\n"
			"\t\tself.obj = %s(0,(%s))\n",
			m_all_pyclass[i].class_name.c_str(), m_all_pyclass[i].class_name.c_str(),
			m_mod_name.c_str(),
			m_all_pyclass[i].class_name.c_str(),
			m_all_pyclass[i].doc.c_str(),
			str_def_args.str().c_str(),
			"init class",
			m_all_pyclass[i].class_reel_name_with_mod.c_str(),
			m_all_pyclass[i].class_reel_name_with_mod.c_str(), str_init_args.str().c_str()
		);

		std::string gen_class_str = buff;
		SAFE_SPRINTF(buff, sizeof(buff),
			"\tdef delete(self):\n"
			"\t\t'''delete obj'''\n"
			"\t\tself.obj.delete()\n");
		gen_class_str += buff;
		//! 增加析构函数
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
			std::string pystr_args;
			std::string pystr_args_only_name;
			for (int j = 0; j < m_all_pyclass[i].methods_info[m].args_num; ++j)
			{
				stringstream ss;
				if (pystr_args.empty())
				{
					ss << "a" << (j + 1);
					pystr_args += ss.str();
				}
				else
				{
					ss << ", a" << (j + 1);
					pystr_args += ss.str();
				}
			}
			pystr_args_only_name = pystr_args;
			for (int j = 0; j < m_all_pyclass[i].methods_info[m].option_args_num; ++j)
			{
				stringstream ss;
				if (pystr_args.empty())
				{
					ss << "a" << (m_all_pyclass[i].methods_info[m].args_num + j + 1);
					std::string tmp = ss.str();
					pystr_args_only_name += tmp;
					pystr_args += tmp + " = None";
				}
				else
				{
					ss << ", a" << (m_all_pyclass[i].methods_info[m].args_num + j + 1);
					std::string tmp = ss.str();
					pystr_args_only_name += tmp;
					pystr_args += tmp + " = None";
				}
			}
			if (!pystr_args_only_name.empty())
				pystr_args_only_name += ",";

			std::ostringstream oss;//创建一个流
			oss << m_all_pyclass[i].methods_info[m].func_addr;//把值传递如流中
			std::string funcAddrStr = oss.str();//获取转换后的字符转并将其写入result

			SAFE_SPRINTF(buff, sizeof(buff),
				"\tdef %s(self,%s):\n"
				"\t\t'''%s'''\n"
				"\t\treturn self.obj.%s(%s,(%s))\n"
				, m_all_pyclass[i].methods_info[m].func_name.c_str(), pystr_args.c_str(),
				m_all_pyclass[i].methods_info[m].doc.c_str(),
				m_all_pyclass[i].methods_info[m].func_real_name.c_str(), funcAddrStr.c_str(), pystr_args_only_name.c_str()
			);
			gen_class_str += buff;
		}
		SAFE_SPRINTF(buff, sizeof(buff),
			"%s.%s = %s\n"
			"%s = None\n"
			"if _tmp_ff_:\n\tglobals()['%s'] = _tmp_ff_\n_tmp_ff_ = None\n",
			m_mod_name.c_str(), m_all_pyclass[i].class_name.c_str(), m_all_pyclass[i].class_name.c_str(),
			m_all_pyclass[i].class_name.c_str(),
			m_all_pyclass[i].class_name.c_str()
		);
		gen_class_str += buff;
		//printf(gen_class_str.c_str());
		PyRun_SimpleString(gen_class_str.c_str());
	}
}

bool ffpython_t::is_method_exist(const std::vector<pyclass_regigster_tool_t::method_info_t>& src_, const std::string& new_)
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
bool ffpython_t::is_property_exist(const std::vector<pyclass_regigster_tool_t::property_info_t>& src_, const std::string& new_)
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
pyclass_regigster_tool_t* ffpython_t::get_pyclass_info_by_name(const std::string& name_)
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

