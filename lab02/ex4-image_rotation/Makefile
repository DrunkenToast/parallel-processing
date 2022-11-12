TARGET=main
BUILD_DIR=./build

bmpfuncs.o: bmpfuncs.c bmpfuncs.h
	g++  -o $(BUILD_DIR)/bmpfuncs.o -c bmpfuncs.c -I . -lOpenCL

cl_helper.o: cl_helper.cpp cl_helper.hpp
	g++  -o $(BUILD_DIR)/cl_helper.o -c cl_helper.cpp -I . -lOpenCL

build: bmpfuncs.o cl_helper.o $(TARGET).cpp
	g++  -o $(BUILD_DIR)/$(TARGET) $(TARGET).cpp $(BUILD_DIR)/bmpfuncs.o $(BUILD_DIR)/cl_helper.o -lOpenCL

run:
	$(BUILD_DIR)/$(TARGET) $(ARGS)

clean:
	rm -rf $(BUILD_DIR)

start: build run

