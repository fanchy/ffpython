import time
import helloworld

#import noddy3
#a = noddy3.Noddy()
#print("-"*10, a.first)
#import inc

#import noddy3 as noddy
#print("foo2_t dir:", dir(helloworld.foo2_t))
#vn = helloworld.foo2_t(5566)
#print("Noddy", vn)
#print(vn.dump(), vn.go(2345))
#vn= None

def foo(a,):#b,c,d,e,f,g, func_addr):
	a.go(False)
	a.val = 12345
	print ("fooooooooooooo", a.get_val(), a.dump(), type(a), isinstance(a, helloworld.foo_t), "end")
	vn = helloworld.foo2_t(5566)
	print("-"*10)
	vn.nice(a)
	print(vn.val)
	return
	#print ("fooooooooooooo",a, b, c,d,e,f,g)
	#print("from ext", helloworld.helloworld(func_addr, (1, 2)))
	print("from ext", helloworld.helloworld())#'1099'
	
	#return [1024, 90, 89]#2147483648
	r = set()
	r.add(100)
	r.add(200)
	return {'a':100, 'b':200}

	
print "XXXXXXend ffpyXXXXXXXX"