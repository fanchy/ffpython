

.PHONY:all

all:
	g++ -o app_test example.cpp -I /usr/include/python2.6/ -L /usr/lib64/ -lpython2.6
