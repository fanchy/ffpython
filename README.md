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

 
