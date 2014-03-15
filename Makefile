opencl: info.cpp main.cpp util.cpp
	g++ -o opencl -ggdb3 -Wall $^ -l OpenCL

.PHONY: clean
clean: ; -rm opencl
