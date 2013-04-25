
#include <cstdlib>
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
    typedef map<string, list<vector<int> > > ret_t;

    ret_t val = ffpython.call<ret_t>("fftest", "test_stl", a1, a2, a3);

}

class foo_t
{
public:
	foo_t(int v_):m_value(v_)
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

void test_register_class()
{
	ffpython_t ffpython("ext1");
	ffpython.reg_class<foo_t, PYCTOR(int)>("foo_t")
			.reg(&foo_t::get_value, "get_value")
			.reg(&foo_t::set_value, "set_value")
			.reg(&foo_t::test_stl, "test_stl")
            .reg_property(&foo_t::m_value, "m_value");
};

int main(int argc, char* argv[])
{
    Py_Initialize();
    string err;
    pyops_t::traceback(err);

	ffpython_t ffpython("helloworld");
	/*
    ffpython.reg(helloworld_impl, "helloworld", "print helloworld")
			.reg_class<foo_t, PYCTOR()>("foo_t")
			.reg(&foo_t::dump, "dump")
			.reg(&foo_t::go, "go")
            .reg_property(&foo_t::val, "val");
			
	ffpython.reg_class<foo2_t, PYCTOR(int)>("foo2_t", "foo2_t", "foo_t")
            .reg(&foo2_t::nice, "nice");
			*/
	ffpython.init();

    TestGuard("test_base", test_base(ffpython));
    TestGuard("test_stl", test_stl(ffpython));

	system("pause");
	Py_Finalize();
    
    return 0;
}

template <typename T>
struct obj_guard_t
{
    obj_guard_t()
    {
        //inc(_FUNCTIOM)
    }
    ~obj_guard_t()
    {
        //Dec
    }
};