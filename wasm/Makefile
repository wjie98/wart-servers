WASI_SDK=$(HOME)/wasi-sdk-15.0
CC = $(WASI_SDK)/bin/clang
CXX = $(WASI_SDK)/bin/clang++
AR = $(WASI_SDK)/bin/ar

CFLAGS := -O3 -Wall
CFLAGS += -I./include -I./capi

CXXFLAGS := -O3 --std=c++20 -fno-exceptions -Wall
CXXFLAGS += -I./include -I./capi

C_OBJECTS := $(patsubst %.c, %.o, $(shell find ./capi -name "*.c"))
# OBJECTS := $(patsubst %.cpp, %.o, $(shell find ./src -name "*.cpp"))

$(info * C_OBJECTS: $(C_OBJECTS))
$(info * OBJECTS: $(OBJECTS))

.PHONY: all
all: $(OBJECTS) $(C_OBJECTS)
	$(AR) -rv libwart.a $(OBJECTS) $(C_OBJECTS)

$(OBJECTS):%.o: %.cpp
	$(CXX) -c $< -o $@ $(CXXFLAGS)

$(C_OBJECTS):%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

.PHONY:clean
clean:
	rm -rf $(OBJECTS)
	rm -rf $(C_OBJECTS)
	rm -rf ./libwart.a