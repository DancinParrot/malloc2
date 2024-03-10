# Malloc 2

Reinventing a not so round wheel of allocating heap memory.

Article referenced: http://dmitrysoshnikov.com/compilers/writing-a-memory-allocator/#mutator-allocator-collector

## Compilation
Compile and run on Linux with GCC:
```bash
gcc main.c -o main
```

## Improvements
- Segregated-list search where same size blocks are grouped together to enhance performance
