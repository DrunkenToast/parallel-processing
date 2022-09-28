TARGET=main
BUILD_DIR=./build

cl_helper.o:
	g++  -o $(BUILD_DIR)/cl_helper.o -c cl_helper.cpp -I . -lOpenCL

build: cl_helper.o
	g++  -o $(BUILD_DIR)/$(TARGET) $(TARGET).cpp $(BUILD_DIR)/cl_helper.o -lOpenCL

run:
	$(BUILD_DIR)/$(TARGET)

clean:
	rm -rf $(BUILD_DIR)

start: build run

