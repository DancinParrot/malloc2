#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h> // for sbrk

// 64-bit systems would be 8-byte alignment
typedef unsigned long word_t;

// Free-List with Segregated-fit: Search block of same size, group block by size
// and partition heap
typedef struct Block {
  // Size of heap block
  size_t size;

  // Is the heap block allocated?
  bool used;

  // Pointer to the next Block
  struct Block *next;

  // Array with 1 machine word for user data
  word_t data[1];
} Block;

static Block *start = NULL;

static Block *top = NULL;

size_t align(size_t block_size) {
  return (block_size + sizeof(word_t) - 1) & ~(sizeof(word_t) - 1);
}

/*
 * Reserve memory space given the size along with additional space reserved for
 * Block header. Size of Block header can be obtained with:
 * total Block size - data size
 */
size_t allocSize(size_t size) {
  // Casts the null pointer 0 to type Foo*. It creates a pointer to a Foo
  // object, but it doesn't point to any actual object in memory.
  // https://stackoverflow.com/questions/3718910/can-i-get-the-size-of-a-struct-field-w-o-creating-an-instance-of-the-struct
  return size + sizeof(Block) - sizeof((Block *)0)->data;
}

Block *getBrk(size_t size) {
  // Get pointer to current heap brk
  Block *block = (Block *)sbrk(0);

  // Determine if enough space left for given size
  // On error, sbrk() returns (void *) -1
  if (sbrk(allocSize(size)) == (void *)-1) {
    return NULL;
  }

  return block;
}

/*
 * Move memory by shifting program break (brk) pointer using sbkr()
 * Only invoke this function when previously allocated and unused
 * blocks have been exhausted.
 */
word_t *alloc(size_t size) {

  // Byte alignment
  size = align(size);

  Block *block = getBrk(size);

  block->size = size;
  block->used = true;

  // Init heap if no current blocks
  if (start == NULL) {
    start = block;
  }

  if (top != NULL) {
    top->next = block;
  }

  top = block;

  return block->data;
}

/*
 * Get block header given block's data
 */
Block *getHeader(word_t *data) {
  // return (Block *)((char *)data + sizeof((Block *)0)->data - sizeof(Block));
  return (Block *)((char *)data + sizeof((Block *)0)->data - sizeof(Block));
}

int main() {
  // size_t size = align(3);
  // size_t size2 = align(14);
  // printf("Aligned 3 bytes to %zu\n", size);
  // printf("Aligned 14 bytes to %zu\n", size2);

  word_t block1 = *alloc(3);
  Block *header = getHeader(&block1);

  printf("Size of Block in Header: %zu\n", header->size);
  printf("Size of data in Block1: %lu\n", sizeof(block1));
  // assert(header->size == sizeof(block1));

  return 0;
}