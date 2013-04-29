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

 
