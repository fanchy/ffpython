# ffpython
## support python3 ,  python2 

ffpython is a c++ lib,which is to simplify task that embed python and extend python. 
For example, call python function, register c++ function to python, register c++ class to python. 
Only one implement c++ header file.

## Project Goals
 * easier to embed python script
 * easier to call python function
 * easier to set or get var in python script
 * easier to extend python with c++ static function
 * easier to extend python with c++ class. C++ class Once registed, python can use it like builtin type.
 * when python exception throw, ffpython will wrap it as a std exception which includes python traceback info.

## Supported Python versions
 * python2.7  python3, win / linux


## Embed Python script in C++
### Get / Set varialbe in  python script/module
``` c++
    printf("sys.version=%s\n", ffpython.getVar<std::string>("sys", "version").c_str());
    ffpython.setVar("fftest", "global_var", "OhNice");
    printf("fftest.global_var=%s\n", ffpython.getVar<std::string>("fftest", "global_var").c_str());
```
### call python function, Support all base type as arg or return value. Nine args can be supported.
``` c++
	int a1 = 100; float a2 = 3.14f; std::string a3 = "OhWell";
    ffpython.call<void>("fftest", "testBase", a1, a2, a3);
```
### call python function, Support all STL type as arg or return value. Nine args can be supported. Vector and List for tuple and list in python,map for dict in python.
``` c++
	std::vector<int> a1;a1.push_back(100);a1.push_back(200);
    std::list<std::string> a2; a2.push_back("Oh");a2.push_back("Nice");
    std::vector<std::list<std::string> > a3;a3.push_back(a2);
    
    ffpython.call<bool>("fftest", "testStl", a1, a2, a3);
```

### register c++ class, python can use it just like builtin types.
``` c++

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

```
### Register c++ class which inherit a class having been registered.
``` c++	
ffpython.call<void>("fftest", "testRegisterInheritClass");
```
### C++ object pointer can be as a arg to python, and object can be access as a instance of builtin type in python.
``` c++	
	Dumy tmp_foo(2013);
	std::vector<Dumy*> vt;
	vt.push_back(&tmp_foo);
    ffpython.call<void>("fftest", "testCppObjToPy", &tmp_foo);
	printf("testCppObjToPy changed nValue=%d\n", tmp_foo.nValue);
	ffpython.call<void>("fftest", "testCppObjToPy2", vt);
```
## Extend Python
### register c++ static function, all base type supported. Arg num can be nine.
``` c++

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

```
## Python test script
``` python



def testBase(a1, a2, a3):
    print('testBase', a1, a2, a3)
    return 0

def testStl(a1, a2, a3):
    print('testStl', a1, a2, a3)
    return True

def test_returnStl():
    print('test_returnStl')
    #map<string, list<vector<int> > >
    ret = {'Oh':[[111,222], [333, 444] ] }
    return ret

def testRegFunction():
    import ffpython
    ffpython.printVal(123, 45.6 , "----789---", [3.14])
    ret = ffpython.returnStl()
    print('testRegFunction', ret)

def testRegisterBaseClass():
    import ffpython
    foo = ffpython.Foo(20130426)
    
    print("testRegisterBaseClass get_val:", foo.getValue())
    foo.setValue(778899)
    print("testRegisterBaseClass get_val:", foo.getValue(), foo.nValue)
    foo.testStl({"key": [11,22,33] })
    print('testRegisterBaseClass testRegisterBaseClass', foo)

def testRegisterInheritClass():
    import ffpython
    dumy = ffpython.Dumy(20130426)
    print("testRegisterInheritClass get_val:", dumy.getValue())
    dumy.setValue(778899)
    print("testRegisterInheritClass get_val:", dumy.getValue(), dumy.nValue)
    dumy.testStl({"key": [11,22,33] })
    dumy.dump()
    print('testRegisterInheritClass', dumy)

def testCppObjToPy_ext(foo):
    print('testCppObjToPy_ext', len(foo))
    for k in range(0, len(foo)):
        print('testCppObjToPy_ext', k, foo[k].nValue)
    
def testCppObjToPy(foo):
    import ffpython
    print("testCppObjToPy get_val:", foo.getValue())
    foo.setValue(778899)
    print("testCppObjToPy get_val:", foo.getValue(), foo.nValue)
    foo.testStl({"key": [11,22,33] })
    foo.nValue = 100
    print('testCppObjToPy testRegisterBaseClass', foo)

def testCppObjToPy2(dumyList):
    dumy = dumyList[0]
    import ffpython
    print("testCppObjToPy get_val:", dumy.getValue())
    dumy.setValue(778899)
    print("testCppObjToPy get_val:", dumy.getValue(), dumy.nValue)
    dumy.testStl({"key": [11,22,33] })
    dumy.dump()
    ffpython.objTest(dumy)
    print('testCppObjToPy', dumy)
    
    return dumy

class PyClass:
    def __init__(self):
        print('PyClass init....')
    def sayHi(self, a1, a2):
        print('sayHi..', a1, a2)
def testCppObjReturnPyObj():
    import ffpython
    return PyClass()
def testCppObjReturnPyLambda():
    def testLambda(a1):
        print('testLambda....', a1)
    return testLambda

```

## Summary
* ffpython Only One implement head file, it is easy to itegrate to project.
* ffpython is simplely wrap for python api, so it is efficient.


