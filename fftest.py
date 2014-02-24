

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

def test_cpp_obj_to_py_ext(foo):
	print('test_cpp_obj_to_py_ext', len(foo))
	for k in range(0, len(foo)):
		print('test_cpp_obj_to_py_ext', k, foo[k].m_value)
	
def test_cpp_obj_to_py(foo):
	import ext2
	print("test_cpp_obj_to_py get_val:", foo.get_value())
	foo.set_value(778899)
	print("test_cpp_obj_to_py get_val:", foo.get_value(), foo.m_value)
	foo.test_stl({"key": [11,22,33] })
	foo.m_value = 100
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