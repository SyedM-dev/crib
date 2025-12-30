SRC_DIR := src
BIN_DIR := bin
OBJ_DIR := build
INCLUDE_DIR := include

TARGET_DEBUG := $(BIN_DIR)/crib-dbg
TARGET_RELEASE := $(BIN_DIR)/crib

PCH_DEBUG := $(OBJ_DIR)/debug/pch.h.gch
PCH_RELEASE := $(OBJ_DIR)/release/pch.h.gch

CCACHE := ccache
CXX_DEBUG := $(CCACHE) g++
CXX_RELEASE := $(CCACHE) clang++

CFLAGS_DEBUG := -std=c++20 -Wall -Wextra -O0 -fno-inline -gsplit-dwarf -g -fsanitize=address -fno-omit-frame-pointer -I./include -I./libs
CFLAGS_RELEASE := -std=c++20 -O3 -march=native -flto=thin \
	-fno-exceptions -fno-rtti -fstrict-aliasing \
	-ffast-math -funroll-loops \
	-fvisibility=hidden \
	-fomit-frame-pointer -DNDEBUG -s \
	-mllvm -vectorize-loops \
	-fno-unwind-tables -fno-asynchronous-unwind-tables\
	-I./include -I./libs

PCH_CFLAGS_DEBUG := $(CFLAGS_DEBUG) -x c++-header
PCH_CFLAGS_RELEASE := $(CFLAGS_RELEASE) -x c++-header

UNICODE_SRC := $(wildcard libs/unicode_width/*.c)

UNICODE_OBJ_DEBUG := $(patsubst libs/unicode_width/%.c,$(OBJ_DIR)/debug/unicode_width/%.o,$(UNICODE_SRC))
UNICODE_OBJ_RELEASE := $(patsubst libs/unicode_width/%.c,$(OBJ_DIR)/release/unicode_width/%.o,$(UNICODE_SRC))

TREE_SITTER_LIBS := $(wildcard libs/tree-sitter-*/libtree-sitter*.a)

PHP_LIB := libs/tree-sitter-php/php/libtree-sitter-php.a

TSX_LIB := libs/tree-sitter-typescript/tsx/libtree-sitter-tsx.a

NGINX_OBJ_PARSER := libs/tree-sitter-nginx/build/Release/obj.target/tree_sitter_nginx_binding/src/parser.o

GITIGNORE_OBJ_PARSER := libs/tree-sitter-gitignore/build/Release/obj.target/tree_sitter_ignore_binding/src/parser.o

FISH_OBJ_PARSER := libs/tree-sitter-fish/build/Release/obj.target/tree_sitter_fish_binding/src/parser.o
FISH_OBJ_SCANNER := libs/tree-sitter-fish/build/Release/obj.target/tree_sitter_fish_binding/src/scanner.o

MAN_OBJ_PARSER := libs/tree-sitter-man/build/Release/obj.target/tree_sitter_man_binding/src/parser.o
MAN_OBJ_SCANNER := libs/tree-sitter-man/build/Release/obj.target/tree_sitter_man_binding/src/scanner.o

MD_OBJ_PARSER := libs/tree-sitter-markdown/build/Release/obj.target/tree_sitter_markdown_binding/tree-sitter-markdown/src/parser.o
MD_OBJ_SCANNER := libs/tree-sitter-markdown/build/Release/obj.target/tree_sitter_markdown_binding/tree-sitter-markdown/src/scanner.o

MD_I_OBJ_PARSER := libs/tree-sitter-markdown/build/Release/obj.target/tree_sitter_markdown_binding/tree-sitter-markdown-inline/src/parser.o
MD_I_OBJ_SCANNER := libs/tree-sitter-markdown/build/Release/obj.target/tree_sitter_markdown_binding/tree-sitter-markdown-inline/src/scanner.o

LIBS := \
	libs/libgrapheme/libgrapheme.a \
	libs/tree-sitter/libtree-sitter.a \
	$(TREE_SITTER_LIBS) \
	$(PHP_LIB) \
	$(TSX_LIB) \
	$(NGINX_OBJ_PARSER) \
	$(GITIGNORE_OBJ_PARSER) \
	$(FISH_OBJ_PARSER) \
	$(FISH_OBJ_SCANNER) \
	$(MAN_OBJ_PARSER) \
	$(MAN_OBJ_SCANNER) \
	$(MD_OBJ_PARSER) \
	$(MD_OBJ_SCANNER) \
	$(MD_I_OBJ_PARSER) \
	$(MD_I_OBJ_SCANNER) \
	-lpcre2-8 -lmagic

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
	$(CXX_DEBUG) $(PCH_CFLAGS_DEBUG) -o $@ $<

$(PCH_RELEASE): $(INCLUDE_DIR)/pch.h
	mkdir -p $(dir $@)
	$(CXX_RELEASE) $(PCH_CFLAGS_RELEASE) -o $@ $<

$(TARGET_DEBUG): $(PCH_DEBUG) $(OBJ_DEBUG) $(UNICODE_OBJ_DEBUG)
	mkdir -p $(BIN_DIR)
	$(CXX_DEBUG) $(CFLAGS_DEBUG) -o $@ $(OBJ_DEBUG) $(UNICODE_OBJ_DEBUG) $(LIBS)

$(TARGET_RELEASE): $(PCH_RELEASE) $(OBJ_RELEASE) $(UNICODE_OBJ_RELEASE)
	mkdir -p $(BIN_DIR)
	$(CXX_RELEASE) $(CFLAGS_RELEASE) -o $@ $(OBJ_RELEASE) $(UNICODE_OBJ_RELEASE) $(LIBS)

$(OBJ_DIR)/debug/%.o: $(SRC_DIR)/%.cc $(PCH_DEBUG)
	mkdir -p $(dir $@)
	$(CXX_DEBUG) $(CFLAGS_DEBUG) -include $(INCLUDE_DIR)/pch.h -MMD -MP -c $< -o $@

$(OBJ_DIR)/release/%.o: $(SRC_DIR)/%.cc $(PCH_RELEASE)
	mkdir -p $(dir $@)
	$(CXX_RELEASE) $(CFLAGS_RELEASE) -include $(INCLUDE_DIR)/pch.h -MMD -MP -c $< -o $@

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
