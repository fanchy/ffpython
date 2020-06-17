
def gencode_pyops():
    template = '''template<>
struct ScriptCppOps<int32_t>{
    static PyObject* scriptFromCpp(int32_t n) { return PyLong_FromLong(long(n)); }
    static bool scriptToCpp(PyObject* pvalue, int32_t& ret){
        if (PyLong_Check(pvalue)){ ret = (int32_t)PyLong_AsLong(pvalue); }
        else if (PyBool_Check(pvalue)) { ret = (Py_False == pvalue?0:1); }
        else if (PyFloat_Check(pvalue)) { ret = (int32_t)PyFloat_AsDouble(pvalue); }
        else if (PyUnicode_Check(pvalue)) { ret = (int32_t)atol(PyUnicode_AsUTF8(pvalue)); }
        else { ret = 0; return false; }
        return true;
    }
};'''
    for k in ['int8_t', 'uint8_t', 'int16_t', 'uint16_t', 'int32_t', 'uint32_t', 'int64_t', 'uint64_t']:
        code = template.replace('int32_t', k)
        print(code)

def gencode_call():
    
    for m in range(10):
        argtypename = ''
        argtypelist = ''
        arglist = ''
        for i in range(m):
            k = i+ 1
            if k == 5:
                argtypename += '\n    '
                argtypelist += '\n    '
                    
            argtypename += ', typename ARG%d'%(k)
            argtypelist += ', const ARG%d& arg%d'%(k, k)
            arglist += '    args.push_back(ScriptCppOps<ARG%d>::scriptFromCpp(arg%d));\n'%(k, k)
        template = '''template<typename RET%s>
RET_V call(const std::string& modName, const std::string& funcName
          %s)
{
    RET_V ret = InitValueTrait<RET_V>::value();
    std::vector<PyObject*>& args = allocArgList();
%s
    PyObjRefGuard retObj(callFunc(modName, funcName, args));
    ScriptCppOps<RET_V>::scriptToCpp(retObj.value, ret);
    return ret;
}'''%(argtypename, argtypelist, arglist)
        code = template
        print(code)
def gencode_ScriptFuncImpl(isVoid=False):
    for m in range(10):
        argtypename = ''
        argtype2 = ''
        argtypelist = ''
        arglist = ''
        arglowerlist = ''
        for i in range(m):
            k = i+ 1
            if k == 5:
                argtypename += '\n    '
                    
            argtypename += ', typename ARG%d'%(k)
            if argtype2:
                argtype2 += ','
            argtype2 += 'ARG%d'%(k)
            argtypelist += '        typename RefTypeTraits<ARG%d>::RealType arg%d = InitValueTrait<typename RefTypeTraits<ARG%d>::RealType>::value();\n'%(k, k, k)
            arglist += '        ScriptCppOps<typename RefTypeTraits<ARG%d>::RealType>::scriptToCpp(arg(%d), arg%d);\n'%(k, k, k)
            if arglowerlist:
                arglowerlist += ','
            arglowerlist += 'arg%d'%(k)
        retStr = 'return ScriptCppOps<RET>::scriptFromCpp(func(%s));'%(arglowerlist)
        if isVoid:
            retStr = 'func(%s);\n        Py_RETURN_NONE;'%(arglowerlist);
        template = '''template <typename RET %s> 
class ScriptFuncImpl<RET(*)(%s)> : public ScriptIterface{
public: 
    typedef RET(*FuncType)(%s); FuncType func;
    ScriptFuncImpl(FuncType f) :ScriptIterface(%d), func(f) {}
    virtual PyObject* handleRun() {
%s%s
        %s
    }
};'''%(argtypename, argtype2, argtype2, m, argtypelist, arglist, retStr)
        code = template
        if isVoid:
            code = code.replace('typename RET ,', '')
            code = code.replace('typename RET ', '')
            code = code.replace('RET(', 'void(')
        print(code)
    return
def genClassImpl():
    for m in range(10):
        argtypename = ''
        argtype2 = ''
        argtypelist = ''
        arglist = ''
        arglowerlist = ''
        for i in range(m):
            k = i+ 1
            if k == 5:
                argtypename += '\n    '
                    
            argtypename += ', typename ARG%d'%(k)
            if argtype2:
                argtype2 += ','
            argtype2 += 'ARG%d'%(k)
            argtypelist += '        typename RefTypeTraits<ARG%d>::RealType arg%d = InitValueTrait<typename RefTypeTraits<ARG%d>::RealType>::value();\n'%(k, k, k)
            arglist += '        ScriptCppOps<typename RefTypeTraits<ARG%d>::RealType>::scriptToCpp(arg(%d), arg%d);\n'%(k, k, k)
            if arglowerlist:
                arglowerlist += ','
            arglowerlist += 'arg%d'%(k)
        retStr = 'return ScriptCppOps<RET>::scriptFromCpp(func(%s));'%(arglowerlist)
        # if isVoid:
            # retStr = 'func(%s);\n        Py_RETURN_NONE;'%(arglowerlist);
        template = '''template<typename CLASS_TYPE%s>
class ScriptClassImpl<CLASS_TYPE(%s)> :public ScriptIterface
{
public:
    typedef CLASS_TYPE ClassTypeReal;
    ScriptClassImpl() :ScriptIterface(%d) {}
    virtual void* handleNew() {
%s%s        return new CLASS_TYPE(%s);
    }
    virtual void handleDel() {
        delete (CLASS_TYPE*)(this->pobjArg);
    }
};'''%(argtypename, argtype2, m, argtypelist, arglist, arglowerlist)
        code = template
        # if isVoid:
            # code = code.replace('typename RET ,', '')
            # code = code.replace('typename RET ', '')
            # code = code.replace('RET(', 'void(')
        print(code)
def MethodImpl(isVoid=False):
    for m in range(10):
        argtypename = ''
        argtype2 = ''
        argtypelist = ''
        arglist = ''
        arglowerlist = ''
        for i in range(m):
            k = i+ 1
            if k == 5 and False:
                argtypename += '\n    '
                    
            argtypename += ', typename ARG%d'%(k)
            if argtype2:
                argtype2 += ','
            argtype2 += 'ARG%d'%(k)
            argtypelist += '        typename RefTypeTraits<ARG%d>::RealType arg%d = InitValueTrait<typename RefTypeTraits<ARG%d>::RealType>::value();\n'%(k, k, k)
            arglist += '        ScriptCppOps<typename RefTypeTraits<ARG%d>::RealType>::scriptToCpp(arg(%d), arg%d);\n'%(k, k, k)
            if arglowerlist:
                arglowerlist += ','
            arglowerlist += 'arg%d'%(k)
        retStr = 'return ScriptCppOps<RET>::scriptFromCpp(((CLASS_TYPE*)pobjArg->*func)(%s));'%(arglowerlist)
        if isVoid:
            retStr = '((CLASS_TYPE*)pobjArg->*func)(%s);\n        Py_RETURN_NONE;'%(arglowerlist);
        template = '''template <typename CLASS_TYPE, typename RET %s>
class ScriptMethodImpl<RET(CLASS_TYPE::*)(%s)> : public ScriptIterface {
public:
    typedef RET(CLASS_TYPE::*FuncType)(%s); FuncType func;
    ScriptMethodImpl(FuncType f) :ScriptIterface(%d), func(f) {}
    virtual PyObject* handleRun() {
%s%s
        %s
    }
};'''%(argtypename, argtype2, argtype2, m, argtypelist, arglist, retStr)
        code = template
        if isVoid:
            code = code.replace('typename RET ,', '')
            code = code.replace('typename RET ', '')
            code = code.replace('RET(', 'void(')
            code = code.replace('template <typename CLASS_TYPE, >', 'template <typename CLASS_TYPE>')
        print(code)
    return
def foo(a= None):
    import ffpython
    #print('ffpython.callExt', ffpython.callExt(0, 0, [1]))
    #print('ffpython.printVal', ffpython.printVal(22))
    #print('foo:%s'%(str(a)))
    objMyExt = ffpython.MyExt(156)
    print(objMyExt)
    print('MyExt', objMyExt.add(20), objMyExt.testMethod(25), objMyExt.nNum)
    objMyExt.nNum = 2099
    print('objMyExt.nNum', objMyExt.nNum)
    
    return '123'
if __name__ == '__main__':
    #MethodImpl(True)
    pass

def GenField(className, fieldName, nFuncID):
    classType = getattr(ffpython, className)
    callExt = ffpython.callExt
    def getFieldVal(self):
        return callExt(self._cppInterObj_, nFuncID, (), 3)
    def setFieldVal(self, value):
        print('setVal', nFuncID, value)
        return callExt(self._cppInterObj_, nFuncID, (value), 3)
    setattr(classType, fieldName, property(getFieldVal, setFieldVal))#add property
    return
class Base:
    def __init__(self):
        print('base init')
        pass
    def __del__(self):
        print('Base del')
class Student(Base):
    def __init__(self):
        Base.__init__(self)
        self._score = 100
    def __del__(self):
        print('Student del')
# GenField(Student, 'score', 11)

# s.score = 200
# print('score', s.score)
# print(dir(Student))
def buildTmpObj(className, ptr):
    srcType = getattr(ffpython, className)
    if srcType:
        return srcType(cppTmpPtr=ptr)
    return None

