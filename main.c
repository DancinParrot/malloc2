#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h> // for sbrk

// Article reference:
// http://dmitrysoshnikov.com/compilers/writing-a-memory-allocator/#mutator-allocator-collector
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

/*
 * Align given block to the word boundary (i.e. at least 8 on 64-bit systems or
 * multiples of 8)
 */
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
 * First-fit approach whereby traverse the linked-list of blocks from start
 * until an usused block with the same size is found. Otherwise, return null
 *
 * Cons: Not fast as need traverse the whole linked-list of unused and used
 * blocks of different sizes.
 */
Block *firstFit(size_t size) {
  Block *block = start;

  while (block != NULL) {
    if (block->used || block->size != size) {
      block = block->next;
      continue;
    }
    return block;
  }

  return NULL;
}

Block *findBlock(size_t size) { return firstFit(size); }

/*
 * Move memory by shifting program break (brk) pointer using sbkr()
 * Only invoke this function when previously allocated and unused
 * blocks have been exhausted.
 */
word_t *alloc(size_t size) {

  // Byte alignment
  size = align(size);

  Block *block = findBlock(size);

  if (block) {
    return block->data;
  }

  block = getBrk(size);

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
 * Get block including header given block's data
 */
Block *getBlock(word_t *data) {
  // return (Block *)((char *)data + sizeof((Block *)0)->data - sizeof(Block));
  return (Block *)((char *)data + sizeof((Block *)0)->data - sizeof(Block));
}

void freeBlock(word_t *data) {
  Block *block = getBlock(data);

  block->used = false;
}

int main() {
  word_t *data1 = alloc(3);
  Block *block1 = getBlock(data1);
  assert(block1->size == sizeof(word_t));

  word_t *data2 = alloc(8);
  Block *block2 = getBlock(data2);
  assert(block2->size == 8);

  // Free block2 for later use
  freeBlock(data2);
  assert(block2->used == false);

  word_t *data3 = alloc(8);
  Block *block3 = getBlock(data3);
  assert(block3 == block2); // reused previous block2

  return 0;
}
