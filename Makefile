SRC_DIR := src
BIN_DIR := bin
OBJ_DIR := build
INCLUDE_DIR := include

TARGET_DEBUG := $(BIN_DIR)/crib-dbg
TARGET_RELEASE := $(BIN_DIR)/crib

PCH_DEBUG := $(OBJ_DIR)/debug/pch.h.gch
PCH_RELEASE := $(OBJ_DIR)/release/pch.h.gch

GENERATED_HEADER := $(INCLUDE_DIR)/scripting/ruby_compiled.h

CCACHE := ccache
CXX := $(CCACHE) clang++
CC := $(CCACHE) musl-clang

CFLAGS_DEBUG :=\
	-std=c++20 -Wall -Wextra \
	-O0 -fno-inline -gsplit-dwarf \
	-g -fno-omit-frame-pointer \
	-Wno-unused-command-line-argument \
	-I./include -I./libs -I/home/syed/main/crib/libs/mruby/include

# C_SANITIZER := -fsanitize=address

CFLAGS_RELEASE :=\
	-static --target=x86_64-linux-musl \
	-std=c++20 -O3 -march=x86-64 -mtune=generic \
  -fno-rtti \
	-ffast-math -flto=thin \
	-fvisibility=hidden \
	-fomit-frame-pointer -DNDEBUG -s \
	-mllvm -vectorize-loops \
	-Wno-unused-command-line-argument \
	-I./include -I./libs -I/home/syed/main/crib/libs/mruby/include

PCH_CFLAGS_DEBUG := $(CFLAGS_DEBUG) -x c++-header
PCH_CFLAGS_RELEASE := $(CFLAGS_RELEASE) -x c++-header

UNICODE_SRC := $(wildcard libs/unicode_width/*.c)

UNICODE_OBJ_DEBUG := $(patsubst libs/unicode_width/%.c,$(OBJ_DIR)/debug/unicode_width/%.o,$(UNICODE_SRC))
UNICODE_OBJ_RELEASE := $(patsubst libs/unicode_width/%.c,$(OBJ_DIR)/release/unicode_width/%.o,$(UNICODE_SRC))

LIBS_RELEASE := \
	libs/libgrapheme/libgrapheme.a ./libs/mruby/build/host/lib/libmruby.a \
	-Wl,-Bstatic,--gc-sections -lpcre2-8

LIBS_DEBUG := \
	libs/libgrapheme/libgrapheme.a ./libs/mruby/build/host/lib/libmruby.a \
	-Wl,-Bdynamic -lpcre2-8

SRC := $(wildcard $(SRC_DIR)/**/*.cc) $(wildcard $(SRC_DIR)/*.cc)
OBJ_DEBUG := $(patsubst $(SRC_DIR)/%.cc,$(OBJ_DIR)/debug/%.o,$(SRC))
OBJ_RELEASE := $(patsubst $(SRC_DIR)/%.cc,$(OBJ_DIR)/release/%.o,$(SRC))

DEP_DEBUG := $(OBJ_DEBUG:.o=.d)
DEP_RELEASE := $(OBJ_RELEASE:.o=.d)

.PHONY: all test release clean

all: debug

test: $(TARGET_DEBUG)

release: $(TARGET_RELEASE)

$(GENERATED_HEADER): $(INCLUDE_DIR)/syntax/tokens.def $(INCLUDE_DIR)/ruby/libcrib.rb src/ruby_compile.sh
	src/ruby_compile.sh

$(PCH_DEBUG): $(INCLUDE_DIR)/pch.h
	mkdir -p $(dir $@)
	$(CXX) $(PCH_CFLAGS_DEBUG) -o $@ $<

$(PCH_RELEASE): $(INCLUDE_DIR)/pch.h
	mkdir -p $(dir $@)
	$(CXX) $(PCH_CFLAGS_RELEASE) -o $@ $<

$(TARGET_DEBUG): $(PCH_DEBUG) $(OBJ_DEBUG) $(UNICODE_OBJ_DEBUG)
	mkdir -p $(BIN_DIR)
	$(CXX) $(CFLAGS_DEBUG) $(C_SANITIZER) -o $@ $(OBJ_DEBUG) $(UNICODE_OBJ_DEBUG) $(LIBS_DEBUG)

$(TARGET_RELEASE): $(PCH_RELEASE) $(OBJ_RELEASE) $(UNICODE_OBJ_RELEASE)
	mkdir -p $(BIN_DIR)
	$(CXX) $(CFLAGS_RELEASE) -o $@ $(OBJ_RELEASE) $(UNICODE_OBJ_RELEASE) $(LIBS_RELEASE)

$(OBJ_DIR)/debug/%.o: $(SRC_DIR)/%.cc $(PCH_DEBUG) $(GENERATED_HEADER)
	mkdir -p $(dir $@)
	$(CXX) $(CFLAGS_DEBUG) -include $(INCLUDE_DIR)/pch.h -MMD -MP -c $< -o $@

$(OBJ_DIR)/release/%.o: $(SRC_DIR)/%.cc $(PCH_RELEASE) $(GENERATED_HEADER)
	mkdir -p $(dir $@)
	$(CXX) $(CFLAGS_RELEASE) -include $(INCLUDE_DIR)/pch.h -MMD -MP -c $< -o $@

$(OBJ_DIR)/debug/unicode_width/%.o: libs/unicode_width/%.c
	mkdir -p $(dir $@)
	$(CC) -MMD -MP -c $< -o $@

$(OBJ_DIR)/release/unicode_width/%.o: libs/unicode_width/%.c
	mkdir -p $(dir $@)
	$(CC) -MMD -MP -c $< -o $@

DEP_DEBUG += $(UNICODE_OBJ_DEBUG:.o=.d)
DEP_RELEASE += $(UNICODE_OBJ_RELEASE:.o=.d)

-include $(DEP_DEBUG)
-include $(DEP_RELEASE)

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
