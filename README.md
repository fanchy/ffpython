# ffpython

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

## Supported Clojure versions
 * python2.5 python2.6 python2.7, win / linux
 * python3.x is being developed, but unfortunately， python3.x api is so different to python2.x, even diffent between python3.2
	and python3.3, Headache!!

## Embed Python script in C++
### Get / Set varialbe in  python script/module
``` c++
	printf("sys.version=%s\n", ffpython.get_global_var<string>("sys", "version").c_str());
    ffpython.set_global_var("fftest", "global_var", "OhNice");
    printf("fftest.global_var=%s\n", ffpython.get_global_var<string>("fftest", "global_var").c_str());
```
### call python function, Support all base type as arg or return value. Nine args can be supported.
``` c++
	int a1 = 100; float a2 = 3.14f; string a3 = "OhWell";
    ffpython.call<void>("fftest", "test_base", a1, a2, a3);
```
### call python function, Support all STL type as arg or return value. Nine args can be supported. Vector and List for tuple and list in python,
	map for dict in python.
``` c++
	vector<int> a1;a1.push_back(100);a1.push_back(200);
    list<string> a2; a2.push_back("Oh");a2.push_back("Nice");
    vector<list<string> > a3;a3.push_back(a2);
    
    ffpython.call<bool>("fftest", "test_stl", a1, a2, a3);
	typedef map<string, list<vector<int> > > ret_t;
    ret_t val = ffpython.call<ret_t>("fftest", "test_return_stl");
```
## Extend Python
### register c++ static function, all base type supported. Arg num can be nine.
``` c++
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
```
### register c++ class, python can use it just like builtin types.
``` c++

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
```
### Register c++ class which inherit a class having been registered.
``` c++	
ffpython.call<void>("fftest", "test_register_inherit_class");
```
### C++ object pointer can be as a arg to python, and object can be access as a instance of builtin type in python.
``` c++	
void test_cpp_obj_to_py(ffpython_t& ffpython)
{
    foo_t tmp_foo(2013);
    ffpython.call<void>("fftest", "test_cpp_obj_to_py", &tmp_foo);
}
void test_cpp_obj_py_obj(ffpython_t& ffpython)
{
    dumy_t tmp_foo(2013);
    
    foo_t* p = ffpython.call<foo_t*>("fftest", "test_cpp_obj_py_obj", &tmp_foo);
}
```
## Python test script
``` pythhon
def test_base(a1, a2, a3):
	print('test_base', a1, a2, a3)
	return 0

def test_stl(a1, a2, a3):
	print('test_stl', a1, a2, a3)
	return True

def test_return_stl():
	print('test_return_stl')
	#map<string, list<vector<int> > >
	ret = {'Oh':[[111,222], [333, 444] ] }
	return ret

def test_reg_function():
	import ext1
	ext1.print_val(123, 45.6 , "----789---", [3.14])
	ret = ext1.return_stl()
	print('test_reg_function', ret)

def test_register_base_class():
	import ext2
	foo = ext2.foo_t(20130426)
	print("test_register_base_class get_val:", foo.get_value())
	foo.set_value(778899)
	print("test_register_base_class get_val:", foo.get_value(), foo.m_value)
	foo.test_stl({"key": [11,22,33] })
	print('test_register_base_class test_register_base_class', foo)

def test_register_inherit_class():
	import ext2
	dumy = ext2.dumy_t(20130426)
	print("test_register_inherit_class get_val:", dumy.get_value())
	dumy.set_value(778899)
	print("test_register_inherit_class get_val:", dumy.get_value(), dumy.m_value)
	dumy.test_stl({"key": [11,22,33] })
	dumy.dump()
	print('test_register_inherit_class', dumy)

def test_cpp_obj_to_py(foo):
	import ext2
	print("test_cpp_obj_to_py get_val:", foo.get_value())
	foo.set_value(778899)
	print("test_cpp_obj_to_py get_val:", foo.get_value(), foo.m_value)
	foo.test_stl({"key": [11,22,33] })
	print('test_cpp_obj_to_py test_register_base_class', foo)

def test_cpp_obj_py_obj(dumy):
	import ext2
	print("test_cpp_obj_py_obj get_val:", dumy.get_value())
	dumy.set_value(778899)
	print("test_cpp_obj_py_obj get_val:", dumy.get_value(), dumy.m_value)
	dumy.test_stl({"key": [11,22,33] })
	dumy.dump()
	ext2.obj_test(dumy)
	print('test_cpp_obj_py_obj', dumy)
	
	return dumy

``` 

## Summary
* ffpython Only One implement head file, it is easy to itegrate to project.
* ffpython is simplely wrap for python api, so it is efficient.


