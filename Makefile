SRC_DIR := src
BIN_DIR := bin
OBJ_DIR := build

TARGET_DEBUG := $(BIN_DIR)/crib-dbg
TARGET_RELEASE := $(BIN_DIR)/crib

CCACHE := ccache
CXX_DEBUG := $(CCACHE) g++
CXX_RELEASE := $(CCACHE) clang++

CFLAGS_DEBUG := -std=c++20 -Wall -Wextra -O0 -fno-inline -gsplit-dwarf -g -fsanitize=address -fno-omit-frame-pointer
CFLAGS_RELEASE := -std=c++20 -O3 -march=native -flto=thin \
	-fno-exceptions -fno-rtti -fstrict-aliasing \
	-ffast-math -funroll-loops \
	-fvisibility=hidden \
	-fomit-frame-pointer -DNDEBUG -s \
	-mllvm -vectorize-loops \
	-fno-unwind-tables -fno-asynchronous-unwind-tables

UNICODE_SRC := $(wildcard libs/unicode_width/*.c)

UNICODE_OBJ_DEBUG := $(patsubst libs/unicode_width/%.c,$(OBJ_DIR)/debug/unicode_width/%.o,$(UNICODE_SRC))
UNICODE_OBJ_RELEASE := $(patsubst libs/unicode_width/%.c,$(OBJ_DIR)/release/unicode_width/%.o,$(UNICODE_SRC))

TREE_SITTER_LIBS := $(wildcard libs/tree-sitter-*/libtree-sitter*.a)
FISH_OBJ_PARSER := libs/tree-sitter-fish/build/Release/obj.target/tree_sitter_fish_binding/src/parser.o
FISH_OBJ_SCANNER := libs/tree-sitter-fish/build/Release/obj.target/tree_sitter_fish_binding/src/scanner.o

LIBS := \
	libs/libgrapheme/libgrapheme.a \
	libs/tree-sitter/libtree-sitter.a \
	$(TREE_SITTER_LIBS) \
	$(FISH_OBJ_PARSER) \
	$(FISH_OBJ_SCANNER) \
	-lpcre2-8 -lmagic

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

DEP_DEBUG += $(UNICODE_OBJ_DEBUG:.o=.d)
DEP_RELEASE += $(UNICODE_OBJ_RELEASE:.o=.d)

-include $(DEP_DEBUG)
-include $(DEP_RELEASE)

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
