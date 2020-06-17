

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
