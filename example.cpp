
#ifdef _WIN32
#include <cstdlib>
#endif

#include "ffpython.h"

#define  TestGuard(X, Y) printf("-------%s begin-----------\n", X);try {Y;}catch(std::exception& e_){printf("exception<%s>\n", e_.what());}\
        printf("-------%s end-----------\n", X);
using namespace ff;

void testBase(FFPython& ffpython)
{
    printf("sys.version=%s\n", ffpython.getVar<std::string>("sys", "version").c_str());
    ffpython.setVar("fftest", "global_var", "OhNice");
    printf("fftest.global_var=%s\n", ffpython.getVar<std::string>("fftest", "global_var").c_str());
	printf("os.getcwd=%s\n", ffpython.call<std::string>("os", "getcwd").c_str());
    printf("time.asctime=%s\n", ffpython.call<std::string>("time", "asctime").c_str());
    int a1 = 100; float a2 = 3.14f; std::string a3 = "OhWell";
    ffpython.call<void>("fftest", "testBase", a1, a2, a3);
}

void testStl(FFPython& ffpython)
{
    std::vector<int> a1;a1.push_back(100);a1.push_back(200);
    std::list<std::string> a2; a2.push_back("Oh");a2.push_back("Nice");
    std::vector<std::list<std::string> > a3;a3.push_back(a2);
    
    ffpython.call<bool>("fftest", "testStl", a1, a2, a3);

}
void test_returnStl(FFPython& ffpython)
{
    //typedef std::map<std::string, std::list<std::vector<int> > > ret_t;
    //ret_t val = ffpython.call<ret_t>("fftest", "test_returnStl");
}

static int printVal(int a1, float a2, const std::string& a3, const std::vector<double>& a4)
{
    printf("%s[%d,%g,%s,%d]\n", __FUNCTION__, a1, a2, a3.c_str(), (int)a4.size());
    return 0;
}
struct OpsTest
{
    static std::list<int> returnStl()
    {
        std::list<int> ret;ret.push_back(1024);
        printf("%s\n", __FUNCTION__);
        return ret;
    }
};
void testRegisterBaseClass(FFPython& ffpython)
{
	ffpython.call<void>("fftest", "testRegisterBaseClass");
};

class Foo
{
public:
	Foo(int v_) :nValue(v_)
	{
		printf("%s\n", __FUNCTION__);
	}
	virtual ~Foo()
	{
		printf("%s\n", __FUNCTION__);
	}
	int getValue()  { return nValue; }
	void setValue(int v_) { nValue = v_; }
	void testStl(std::map<std::string, std::list<int> >& v_)
	{
		printf("%s\n", __FUNCTION__);
	}
	int nValue;
};

class Dumy : public Foo
{
public:
	Dumy(int v_) :Foo(v_)
	{
		printf("%s\n", __FUNCTION__);
	}
	~Dumy()
	{
		printf("%s\n", __FUNCTION__);
	}
	void dump()
	{
		printf("%s\n", __FUNCTION__);
	}
};


static Foo* objTest(Dumy* p)
{
	printf("%s\n", __FUNCTION__);
	return p;
}

std::string testRegFunction(FFPython& ffpython)
{
    ffpython.regFunc(&printVal, "printVal")
            .regFunc(&OpsTest::returnStl, "returnStl");

	ffpython.regClass<Foo(int)>("Foo")
		.regMethod(&Foo::getValue, "getValue")
		.regMethod(&Foo::setValue, "setValue")
		.regMethod(&Foo::testStl, "testStl")
		.regField(&Foo::nValue, "nValue");

	ffpython.regClass<Dumy(int)>("Dumy", "Foo")
		.regMethod(&Dumy::dump, "dump");

	ffpython.regFunc(objTest, "objTest");
	return "cppext";
}


void testRegisterInheritClass(FFPython& ffpython)
{
    ffpython.call<void>("fftest", "testRegisterInheritClass");
};

void testCppObjToPy(FFPython& ffpython)
{
	Dumy tmp_foo(2013);
	std::vector<Dumy*> vt;
	vt.push_back(&tmp_foo);
    ffpython.call<void>("fftest", "testCppObjToPy", &tmp_foo);
	printf("testCppObjToPy changed nValue=%d\n", tmp_foo.nValue);
	ffpython.call<void>("fftest", "testCppObjToPy2", vt);
}


void testPyClassLambda(FFPython& ffpython)
{
    PyObject* pFunc= ffpython.call<PyObject*>("fftest", "testCppObjReturnPyLambda");
	std::vector<PyObject*> args;
	args.push_back(ScriptCppOps<int>::scriptFromCpp(1066));
    ffpython.callFuncByObjRet<void>(pFunc, args);
    Py_XDECREF(pFunc);

	PyObject* pobj = ffpython.call<PyObject*>("fftest", "testCppObjReturnPyObj");
	Py_INCREF(pobj);
	std::vector<PyObject*> args1;
	args1.push_back(ScriptCppOps<int>::scriptFromCpp(1111));
	args1.push_back(ScriptCppOps<std::string>::scriptFromCpp("soNice"));
	ffpython.callMethodByObjRet<void>(pobj, "sayHi", args1);
	Py_XDECREF(pobj);

}

int main(int argc, char* argv[])
{
	try {

		FFPython ffpython;
		testRegFunction(ffpython);
		ffpython.addPath("./");
		ffpython.addPath("../");

		TestGuard("testBase", testBase(ffpython));

		TestGuard("testStl", testStl(ffpython));
		ffpython.call<void>("fftest", "testRegFunction");

		TestGuard("testRegisterBaseClass", testRegisterBaseClass(ffpython));

		TestGuard("testRegisterInheritClass", testRegisterInheritClass(ffpython));
		
		TestGuard("testCppObjToPy", testCppObjToPy(ffpython));

		TestGuard("testPyClassLambda", testPyClassLambda(ffpython));

#ifdef _WIN32
		system("pause");
#endif
	}
	catch(std::exception& e) {
		printf("exception<%s>\n", e.what());
	}
    printf("main exit...\n");
    return 0;
}
