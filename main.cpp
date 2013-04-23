
#include <cstdlib>
#include <Python.h>
#include "ffpython.h"

static int helloworld_impl(pyoption_t<string&>& v)
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
	int dump()
	{
		printf("in %s[%d,%p]\n", __FUNCTION__, ++val, this);
		return 778899;
	}
	string go(bool a)
	{
		printf("in %s[%d, %d, %p]\n", __FUNCTION__, ++val, a, this);
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


void callpy()
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
    pycall_t::call("fftest", "foo", args, pyret);

    printf("pyret =%d\n", pyret.get_value().size());
}

int main(int argc, char* argv[])
{
    Py_Initialize();
    	
	ffpython_t ffpython("helloworld");
    ffpython.reg(helloworld_impl, "helloworld", "print helloworld")
			.reg_class<foo_t, PYCTOR()>("foo_t")
			.reg(&foo_t::dump, "dump")
			.reg(&foo_t::go, "go")
            .reg_property(&foo_t::val, "val");

	ffpython.reg_class<foo2_t, PYCTOR(int)>("foo2_t", "foo2_t", "foo_t")
            .reg(&foo2_t::nice, "nice");
	ffpython.init_mod();

    PyRun_SimpleString("from time import time,ctime\n"
                     "print 'Today is',ctime(time())\n");

	callpy();

	system("pause");
	Py_Finalize();
    
    return 0;
}
