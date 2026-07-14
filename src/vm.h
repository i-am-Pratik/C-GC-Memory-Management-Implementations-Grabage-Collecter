#ifndef VM_H
#define VM_H

#include <stdbool.h>
#include <stddef.h>

#define STACK_MAX 256
#define INITIAL_GC_THRESHOLD 8

typedef enum {
    OBJ_INT,
    OBJ_PAIR
} ObjectType;

typedef struct sObject {
    ObjectType type;
    unsigned int ref_count; // For Reference Counting
    bool marked;            // For Mark and Sweep

    // The next object in the linked list of all allocated objects.
    struct sObject* next;

    union {
        // OBJ_INT
        int value;

        // OBJ_PAIR
        struct {
            struct sObject* head;
            struct sObject* tail;
        };
    };
} Object;

typedef struct {
    Object* stack[STACK_MAX];
    int stackSize;

    // The first object in the linked list of all objects on the heap.
    Object* firstObject;

    // The total number of currently allocated objects.
    int numObjects;

    // The number of objects required to trigger a GC.
    int maxObjects;
} VM;

// vm.c prototypes
VM* newVM();
void push(VM* vm, Object* value);
Object* pop(VM* vm);
Object* newObject(VM* vm, ObjectType type);
Object* pushInt(VM* vm, int intValue);
Object* pushPair(VM* vm);
void freeVM(VM* vm);

// ref_count.c prototypes
void retain(Object* obj);
void release(VM* vm, Object* obj);

// mark_sweep.c prototypes
void gc(VM* vm);
void mark(Object* obj);
void sweep(VM* vm);

#endif // VM_H
