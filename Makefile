SRC_DIR := src
BIN_DIR := bin
OBJ_DIR := build
INCLUDE_DIR := include

TARGET_DEBUG := $(BIN_DIR)/crib-dbg
TARGET_RELEASE := $(BIN_DIR)/crib

PCH_DEBUG := $(OBJ_DIR)/debug/pch.h.gch
PCH_RELEASE := $(OBJ_DIR)/release/pch.h.gch

CCACHE := ccache
CXX := $(CCACHE) clang++

CFLAGS_DEBUG :=\
	-std=c++20 -Wall -Wextra \
	-O0 -fno-inline -gsplit-dwarf\
	-g -fno-omit-frame-pointer\
	-Wno-unused-command-line-argument \
	-I./include -I./libs \
	-I/usr/include/ruby-3.4.0 -I/usr/include/ruby-3.4.0/x86_64-linux
CFLAGS_RELEASE :=\
	-std=c++20 -O3 -march=native \
  -fno-rtti -fstrict-aliasing \
	-ffast-math -flto=thin \
	-fvisibility=hidden -fuse-ld=lld \
	-fomit-frame-pointer -DNDEBUG -s \
	-mllvm -vectorize-loops \
	-fno-unwind-tables -fno-asynchronous-unwind-tables\
	-Wno-unused-command-line-argument \
	-I./include -I./libs \
	-I/usr/include/ruby-3.4.0 -I/usr/include/ruby-3.4.0/x86_64-linux

PCH_CFLAGS_DEBUG := $(CFLAGS_DEBUG) -x c++-header
PCH_CFLAGS_RELEASE := $(CFLAGS_RELEASE) -x c++-header

UNICODE_SRC := $(wildcard libs/unicode_width/*.c)

UNICODE_OBJ_DEBUG := $(patsubst libs/unicode_width/%.c,$(OBJ_DIR)/debug/unicode_width/%.o,$(UNICODE_SRC))
UNICODE_OBJ_RELEASE := $(patsubst libs/unicode_width/%.c,$(OBJ_DIR)/release/unicode_width/%.o,$(UNICODE_SRC))

LIBS := \
	libs/libgrapheme/libgrapheme.a \
	-lpcre2-8 -lmagic -lruby

SRC := $(wildcard $(SRC_DIR)/**/*.cc) $(wildcard $(SRC_DIR)/*.cc)
OBJ_DEBUG := $(patsubst $(SRC_DIR)/%.cc,$(OBJ_DIR)/debug/%.o,$(SRC))
OBJ_RELEASE := $(patsubst $(SRC_DIR)/%.cc,$(OBJ_DIR)/release/%.o,$(SRC))

DEP_DEBUG := $(OBJ_DEBUG:.o=.d)
DEP_RELEASE := $(OBJ_RELEASE:.o=.d)

.PHONY: all test release clean

all: debug

test: $(TARGET_DEBUG)

release: $(TARGET_RELEASE)

$(PCH_DEBUG): $(INCLUDE_DIR)/pch.h
	mkdir -p $(dir $@)
	$(CXX) $(PCH_CFLAGS_DEBUG) -o $@ $<

$(PCH_RELEASE): $(INCLUDE_DIR)/pch.h
	mkdir -p $(dir $@)
	$(CXX) $(PCH_CFLAGS_RELEASE) -o $@ $<

$(TARGET_DEBUG): $(PCH_DEBUG) $(OBJ_DEBUG) $(UNICODE_OBJ_DEBUG)
	mkdir -p $(BIN_DIR)
	$(CXX) $(CFLAGS_DEBUG) -o $@ $(OBJ_DEBUG) $(UNICODE_OBJ_DEBUG) $(LIBS)

$(TARGET_RELEASE): $(PCH_RELEASE) $(OBJ_RELEASE) $(UNICODE_OBJ_RELEASE)
	mkdir -p $(BIN_DIR)
	$(CXX) $(CFLAGS_RELEASE) -o $@ $(OBJ_RELEASE) $(UNICODE_OBJ_RELEASE) $(LIBS)

$(OBJ_DIR)/debug/%.o: $(SRC_DIR)/%.cc $(PCH_DEBUG)
	mkdir -p $(dir $@)
	$(CXX) $(CFLAGS_DEBUG) -include $(INCLUDE_DIR)/pch.h -MMD -MP -c $< -o $@

$(OBJ_DIR)/release/%.o: $(SRC_DIR)/%.cc $(PCH_RELEASE)
	mkdir -p $(dir $@)
	$(CXX) $(CFLAGS_RELEASE) -include $(INCLUDE_DIR)/pch.h -MMD -MP -c $< -o $@

$(OBJ_DIR)/debug/unicode_width/%.o: libs/unicode_width/%.c
	mkdir -p $(dir $@)
	$(CXX) $(CFLAGS_DEBUG) -MMD -MP -c $< -o $@

$(OBJ_DIR)/release/unicode_width/%.o: libs/unicode_width/%.c
	mkdir -p $(dir $@)
	$(CXX) $(CFLAGS_RELEASE) -MMD -MP -c $< -o $@

DEP_DEBUG += $(UNICODE_OBJ_DEBUG:.o=.d)
DEP_RELEASE += $(UNICODE_OBJ_RELEASE:.o=.d)

-include $(DEP_DEBUG)
-include $(DEP_RELEASE)

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
