
#include <cstdlib>
#include <Python.h>
#include "ffpython.h"

static int helloworld_impl(pyoption_t<string&>& v)//, int a2, float a3, char a4, string a5, const char* a6, string a7,
                           //const string& a8, double a9)
{
	printf("in....helloworld_impl, v[%s]\n", v.value("nonearg").c_str());
    return 1024;
}

struct foo_t
{
	foo_t():val(100)
	{
		
	}
	~foo_t()
	{
		printf("~foo deconstructor %s\n", __FUNCTION__);
	}
	int dump() const
	{
		printf("in %s[%d,%p]\n", __FUNCTION__, val, this);
		return 778899;
	}
	string go(bool a)//, int a2, char a3, const char* a4, string& a5, foo_t* a6, float a7, double a8, char* a9) const
	{
		printf("in %s[%d, %d, %p]\n", __FUNCTION__, val, a, this);
		return __FUNCTION__;
	}
	int val;
};
struct foo2_t: public foo_t
{
    foo2_t(int a)
    {
        val += 1000;
    }
    int nice(const foo_t* p)
    {
        printf("in %s[%p,%d]\n", __FUNCTION__, p, p->val);
        return 205;
    }
};


void callpy(ffpython_t& ffpython)
{
    pycall_arg_t args(1);

    vector<int> vt_int;vt_int.push_back(111);
    vector<vector<int> > vt_vt_int;vt_vt_int.push_back(vt_int);
    set<string> set_str; set_str.insert("gogo");set_str.insert("ttt");
    map<string, double> map_int_double;map_int_double["xx"] = 56.3f;

    foo_t foo;
    foo.val = 15566;
	const foo_t* pf = &foo;
    //args.add('a').add(100).add("300str").add(vt_int).add(vt_vt_int).add(set_str).add(map_int_double).add(&foo);
    args.add(pf);

    pytype_tool_impl_t<map<string, int> >  pyret;
    //pycall_t::call("fftest", "foo", args, pyret);
    ffpython.call<void>("fftest", "foo", pf);
    int value = ffpython.call<int>("fftest", "foo", pf);

    printf("pyret =%d\n", value);
}

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

int main(int argc, char* argv[])
{
    Py_Initialize();
    string err;
    pyops_t::traceback(err);

	ffpython_t ffpython("helloworld");
    ffpython.reg(helloworld_impl, "helloworld", "print helloworld")
			.reg_class<foo_t, PYCTOR()>("foo_t")
			.reg(&foo_t::dump, "dump")
			.reg(&foo_t::go, "go")
            .reg_property(&foo_t::val, "val");

	ffpython.reg_class<foo2_t, PYCTOR(int)>("foo2_t", "foo2_t", "foo_t")
            .reg(&foo2_t::nice, "nice");
	ffpython.init();

    PyRun_SimpleString("from time import time,ctime\n"
                     "print 'Today is',ctime(time())\n");

    TestGuard("test_base", test_base(ffpython));
    TestGuard("test_stl", test_stl(ffpython));
    try
    {
        ffpython.set_global_var("fftest", "var", "Ohfuck");
        printf("global var[%s]\n", ffpython.get_global_var<string>("fftest", "var").c_str());
	    callpy(ffpython);
    }
    catch(exception& e_)
    {
        printf("exception<%s>\n", e_.what());
    }

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