
#ifdef _WIN32
#include <cstdlib>
#endif

#include "ffpython.h"

#define  TestGuard(X, Y) printf("-------%s begin-----------\n", X);try {Y;}catch(exception& e_){printf("exception<%s>\n", e_.what());}\
        printf("-------%s end-----------\n", X);

void test_base(ffpython_t& ffpython)
{
    printf("sys.version=%s\n", ffpython.get_global_var<string>("sys", "version").c_str());
    ffpython.set_global_var("fftest", "global_var", "OhNice");
    printf("fftest.global_var=%s\n", ffpython.get_global_var<string>("fftest", "global_var").c_str());
    printf("time.asctime=%s\n", ffpython.call<string>("time", "asctime").c_str());
    int a1 = 100; float a2 = 3.14f; string a3 = "OhWell";
    ffpython.call<void>("fftest", "test_base", a1, a2, a3);
}

void test_stl(ffpython_t& ffpython)
{
    vector<int> a1;a1.push_back(100);a1.push_back(200);
    list<string> a2; a2.push_back("Oh");a2.push_back("Nice");
    vector<list<string> > a3;a3.push_back(a2);
    
    ffpython.call<bool>("fftest", "test_stl", a1, a2, a3);

}
void test_return_stl(ffpython_t& ffpython)
{
    typedef map<string, list<vector<int> > > ret_t;
    ret_t val = ffpython.call<ret_t>("fftest", "test_return_stl");
}

static int print_val(int a1, float a2, const string& a3, const vector<double>& a4)
{
    printf("%s[%d,%f,%s,%d]\n", __FUNCTION__, a1, a2, a3.c_str(), a4.size());
    return 0;
}
struct ops_t
{
    static list<int> return_stl()
    {
        list<int> ret;ret.push_back(1024);
        printf("%s\n", __FUNCTION__);
        return ret;
    }
};

void test_reg_function()
{
    ffpython_t ffpython;//("ext1");
    ffpython.reg(&print_val, "print_val")
            .reg(&ops_t::return_stl, "return_stl");
    ffpython.init("ext1");
    ffpython.call<void>("fftest", "test_reg_function");
}

class foo_t
{
public:
	foo_t(int v_):m_value(v_)
	{
		printf("%s\n", __FUNCTION__);
	}
    virtual ~foo_t()
    {
        printf("%s\n", __FUNCTION__);
    }
	int get_value() const { return m_value; }
	void set_value(int v_) { m_value = v_; }
	void test_stl(map<string, list<int> >& v_) 
	{
		printf("%s\n", __FUNCTION__);
	}
	int m_value;
};

class dumy_t: public foo_t
{
public:
    dumy_t(int v_):foo_t(v_)
    {
        printf("%s\n", __FUNCTION__);
    }
    ~dumy_t()
    {
        printf("%s\n", __FUNCTION__);
    }
    void dump() 
    {
        printf("%s\n", __FUNCTION__);
    }
};


static foo_t* obj_test(dumy_t* p)
{
    printf("%s\n", __FUNCTION__);
    return p;
}

void test_register_base_class(ffpython_t& ffpython)
{
	ffpython.reg_class<foo_t, PYCTOR(int)>("foo_t")
			.reg(&foo_t::get_value, "get_value")
			.reg(&foo_t::set_value, "set_value")
			.reg(&foo_t::test_stl, "test_stl")
            .reg_property(&foo_t::m_value, "m_value");

    ffpython.reg_class<dumy_t, PYCTOR(int)>("dumy_t", "dumy_t class inherit foo_t ctor <int>", "foo_t")
        .reg(&dumy_t::dump, "dump");

    ffpython.reg(obj_test, "obj_test");

    ffpython.init("ext2");
    ffpython.call<void>("fftest", "test_register_base_class");
};

void test_register_inherit_class(ffpython_t& ffpython)
{
    ffpython.call<void>("fftest", "test_register_inherit_class");
};

void test_cpp_obj_to_py(ffpython_t& ffpython)
{
    foo_t tmp_foo(2013);
	vector<foo_t*> vt;
	vt.push_back(&tmp_foo);
    ffpython.call<void>("fftest", "test_cpp_obj_to_py", &tmp_foo);
	printf("test_cpp_obj_to_py changed m_value=%d\n", tmp_foo.m_value);
	ffpython.call<void>("fftest", "test_cpp_obj_to_py_ext", vt);
}

void test_cpp_obj_py_obj(ffpython_t& ffpython)
{
    dumy_t tmp_foo(2013);
    
    foo_t* p = ffpython.call<foo_t*>("fftest", "test_cpp_obj_py_obj", &tmp_foo);
}


int main(int argc, char* argv[])
{
    Py_Initialize();

	ffpython_t::add_path("./");
    ffpython_t ffpython;//("ext2");

    TestGuard("test_base", test_base(ffpython));
    TestGuard("test_stl", test_stl(ffpython));
    TestGuard("test_reg_function", test_reg_function());
    TestGuard("test_register_base_class", test_register_base_class(ffpython));

    TestGuard("test_register_inherit_class", test_register_inherit_class(ffpython));
    TestGuard("test_cpp_obj_to_py", test_cpp_obj_to_py(ffpython));
    TestGuard("test_cpp_obj_py_obj", test_cpp_obj_py_obj(ffpython));
    
#ifdef _WIN32
	system("pause");
#endif
	Py_Finalize();
    
    return 0;
}
