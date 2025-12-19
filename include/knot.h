#ifndef ROPE_H
#define ROPE_H

#include "./utils.h"
#include <cstdint>
#include <vector>

#define MIN_CHUNK_SIZE 64       // 64 Bytes
#define MAX_CHUNK_SIZE 1024 * 8 // 8192 Bytes (8 KiB)
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define DEPTH(n) ((n) ? (n)->depth : 0)

// Rope node definition
typedef struct Knot {
  Knot *left;
  Knot *right;
  uint8_t depth;
  uint32_t chunk_size;
  uint32_t line_count;
  uint32_t char_count;
  char data[];
} Knot;

typedef struct LineIterator {
  Knot *node;
  uint8_t top;
  uint32_t offset;
  Knot *stack[64];
  char *buffer;
  size_t capacity;
} LineIterator;

typedef struct LeafIterator {
  Knot *node;
  uint8_t top;
  uint32_t offset;
  uint32_t adjustment;
  Knot *stack[64];
} LeafIterator;

typedef struct ByteIterator {
  LeafIterator *it;
  uint32_t offset_l;
  uint32_t offset_g;
  uint32_t char_count;
  char *data;
} ByteIterator;

// Rope operations

// Takes lengt of string to be converted
// to rope and returns a suitable chunk size
// but rope should work with any positive chunk size
uint32_t optimal_chunk_size(uint64_t length);

// Takes a string (no need for null termination) and returns a rope
// len is the length of the string, and chunk size is the size of each chunk
// load does not free or consume the string.
// and the str can be freed after load has been run.
Knot *load(char *str, uint32_t len, uint32_t chunk_size);

// Balances the rope and returns the root
// n is no longer valid / do not free
// As rope is balanced by other functions
// this is not to be used directly
Knot *balance(Knot *n);

// Concatenates two ropes and returns the joined root
// Balances the ropes too, if needed
// left and right are no longer valid / do not free
// ! left and right should have the same chunk size !
Knot *concat(Knot *left, Knot *right);

// Used to insert text into the rope
// node (the rope being inserted into) is no longer valid after call
// instead use return value as the new node
// offset is the position of the insertion relative to the start of the rope
// str is the string to be inserted (no need for null termination)
// len is the length of the string
Knot *insert(Knot *node, uint32_t offset, char *str, uint32_t len);

// Similar to insert but for deletion
// node (the rope being deleted from) is no longer valid after call
// instead use return value as the new node
// offset is the position of the deletion relative to the start of the rope
// len is the length of the deletion
Knot *erase(Knot *node, uint32_t offset, uint32_t len);

// Used to read a string from the rope
// root is the rope to be read from
// offset is the position of the read relative to the start of the rope
// len is the length of the read
// returns a null terminated string, should be freed by the caller
char *read(Knot *root, uint32_t offset, uint32_t len);

// Used to split the rope into left and right ropes
// node is the rope to be split (it is no longer valid after call / do not free)
// offset is the position of the split relative to the start of the rope
// left and right are pointers set to the root of that side of the split
void split(Knot *node, uint32_t offset, Knot **left, Knot **right);

// Used to convert a byte offset to a line number that contains that byte
uint32_t byte_to_line(Knot *node, uint32_t offset, uint32_t *out_col);

// Used to convert a line number to a byte offset (start of the line)
// also sets out_len to the length of the line
uint32_t line_to_byte(Knot *node, uint32_t line, uint32_t *out_len);

// Used to start a line iterator from the start_line number
// root is the root of the rope
// returned iterator must be freed after iteration is done
LineIterator *begin_l_iter(Knot *root, uint32_t start_line);

// Each subsequent call returns the next line as a null terminated string
// `it` is the iterator returned from begin_l_iter
// After getting the necessary lines free the iterator (no need to go upto the
// end) returns null if there are no more lines All return strings `must` be
// freed by the caller
char *next_line(LineIterator *it, uint32_t *out_len);

// Returns the previous line as a null terminated string
// `it` is the iterator returned from begin_l_iter
// it can be used to iterate backwards
// and can be used along with next_line
// doing prev_line then next_line or vice versa will return the same line
// `out_len` is set to the length of the returned string
char *prev_line(LineIterator *it, uint32_t *out_len);

// Used to start an iterator over leaf data
// root is the root of the rope
// the caller must free the iterator after use
// start_offset is the byte from which the iterator should start
LeafIterator *begin_k_iter(Knot *root, uint32_t start_offset);

// Returns the next leaf data as a null terminated string
// `it` is the iterator returned from begin_k_iter
// ! Strings returned must never be freed by the caller !
// to mutate the string a copy must be made
// `out_len` is set to the length of the returned string
char *next_leaf(LeafIterator *it, uint32_t *out_len);

// Used to start an iterator over byte data (one byte at a time)
// Uses leaf iterator internally
// root is the root of the rope, the caller must free the iterator after use
ByteIterator *begin_b_iter(Knot *root);

// Returns the next byte from the iterator
// Returns '\0' if there are no more bytes left
// `it` is the iterator returned from begin_b_iter
char next_byte(ByteIterator *it);

// Returns a leaf data as a null terminated string
// root is the root of the rope
// start_offset is the byte from which the leaf data should start
// `out_len` is set to the length of the returned string
// return value must never be freed
char *leaf_from_offset(Knot *root, uint32_t start_offset, uint32_t *out_len);

// Used to search for a pattern in the rope
// Pattern is a null terminated string representing a regular expression (DFA
// compliant) I.e some forms of backtracking etc. are not supported
// root is the root of the rope to be searched
// Returns a vector of pairs of start and length offsets (in bytes)
std::vector<std::pair<size_t, size_t>> search_rope(Knot *root,
                                                   const char *pattern);

// Helper function to free the rope
// root is the root of the rope
// the root is no longer valid after call
// This must be called only once when the rope is no longer needed
void free_rope(Knot *root);

#endif // ROPE_H
