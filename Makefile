

.PHONY:all

all:
	g++ -o app_test example.cpp -I /usr/include/python2.7/ -L /usr/lib64/ -lpython2.7
