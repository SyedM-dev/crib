SRC_DIR := src
BIN_DIR := bin
OBJ_DIR := build

TARGET_DEBUG := $(BIN_DIR)/crib-dbg
TARGET_RELEASE := $(BIN_DIR)/crib

CXX_DEBUG := g++
CXX_RELEASE := clang++

CFLAGS_DEBUG := -std=c++20 -Wall -Wextra -O0 -g -fno-inline -gsplit-dwarf
CFLAGS_RELEASE := -std=c++20 -O3 -march=native -flto=thin \
	-fno-exceptions -fno-rtti -fstrict-aliasing -ffast-math -funroll-loops \
	-fomit-frame-pointer -DNDEBUG -s \
	-mllvm -inline-threshold=10000 \
	-mllvm -vectorize-loops \
	-mllvm -force-vector-width=8 \
	-mllvm -unroll-threshold=500000

UNICODE_SRC := $(wildcard libs/unicode_width/*.c)

UNICODE_OBJ_DEBUG := $(patsubst libs/unicode_width/%.c,$(OBJ_DIR)/debug/unicode_width/%.o,$(UNICODE_SRC))
UNICODE_OBJ_RELEASE := $(patsubst libs/unicode_width/%.c,$(OBJ_DIR)/release/unicode_width/%.o,$(UNICODE_SRC))

LIBS := \
	libs/libgrapheme/libgrapheme.a \
	libs/tree-sitter/libtree-sitter.a \
	libs/tree-sitter-ruby/libtree-sitter-ruby.a \
	-lpcre2-8


SRC := $(wildcard $(SRC_DIR)/*.cc)
OBJ_DEBUG := $(patsubst $(SRC_DIR)/%.cc,$(OBJ_DIR)/debug/%.o,$(SRC))
OBJ_RELEASE := $(patsubst $(SRC_DIR)/%.cc,$(OBJ_DIR)/release/%.o,$(SRC))

DEP_DEBUG := $(OBJ_DEBUG:.o=.d)
DEP_RELEASE := $(OBJ_RELEASE:.o=.d)

.PHONY: all test release clean

all: debug

test: $(TARGET_DEBUG)

release: $(TARGET_RELEASE)

$(TARGET_DEBUG): $(OBJ_DEBUG) $(UNICODE_OBJ_DEBUG)
	mkdir -p $(BIN_DIR)
	$(CXX_DEBUG) $(CFLAGS_DEBUG) -o $@ $^ $(LIBS)

$(TARGET_RELEASE): $(OBJ_RELEASE) $(UNICODE_OBJ_RELEASE)
	mkdir -p $(BIN_DIR)
	$(CXX_RELEASE) $(CFLAGS_RELEASE) -o $@ $^ $(LIBS)

# Pattern rules for object files + dependency generation
$(OBJ_DIR)/debug/%.o: $(SRC_DIR)/%.cc
	mkdir -p $(dir $@)
	$(CXX_DEBUG) $(CFLAGS_DEBUG) -MMD -MP -c $< -o $@

$(OBJ_DIR)/release/%.o: $(SRC_DIR)/%.cc
	mkdir -p $(dir $@)
	$(CXX_RELEASE) $(CFLAGS_RELEASE) -MMD -MP -c $< -o $@

$(OBJ_DIR)/debug/unicode_width/%.o: libs/unicode_width/%.c
	mkdir -p $(dir $@)
	$(CXX_DEBUG) $(CFLAGS_DEBUG) -MMD -MP -c $< -o $@

$(OBJ_DIR)/release/unicode_width/%.o: libs/unicode_width/%.c
	mkdir -p $(dir $@)
	$(CXX_RELEASE) $(CFLAGS_RELEASE) -MMD -MP -c $< -o $@

# Include deps if they exist
DEP_DEBUG += $(UNICODE_OBJ_DEBUG:.o=.d)
DEP_RELEASE += $(UNICODE_OBJ_RELEASE:.o=.d)

-include $(DEP_DEBUG)
-include $(DEP_RELEASE)

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
