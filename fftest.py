

def test_base(a1, a2, a3):
	print('test_base', a1, a2, a3)
	return 0

def test_stl(a1, a2, a3):
	print('test_stl', a1, a2, a3)
	return True

def test_return_stl():
	print('test_return_stl')
	#map<string, list<vector<int> > >
	ret = {'Oh':[[111,222], [333, 444] }
	return ret