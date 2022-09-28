TARGET=main
BUILD_DIR=./build

all: build run

build: main.cpp kernel.cl
	g++  -o $(BUILD_DIR)/$(TARGET) $(TARGET).cpp -lOpenCL

run:
	$(BUILD_DIR)/$(TARGET)

clean:
	rm -rf $(BUILD_DIR)
