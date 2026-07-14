#include "vm.h"
#include <stdio.h>
#include <stdlib.h>

VM* newVM() {
    VM* vm = (VM*)malloc(sizeof(VM));
    if (vm == NULL) {
        fprintf(stderr, "Failed to allocate VM\n");
        exit(1);
    }
    vm->stackSize = 0;
    vm->firstObject = NULL;
    vm->numObjects = 0;
    vm->maxObjects = INITIAL_GC_THRESHOLD;
    return vm;
}

void push(VM* vm, Object* value) {
    if (vm->stackSize >= STACK_MAX) {
        fprintf(stderr, "Stack overflow!\n");
        exit(1);
    }
    vm->stack[vm->stackSize++] = value;
}

Object* pop(VM* vm) {
    if (vm->stackSize == 0) {
        fprintf(stderr, "Stack underflow!\n");
        exit(1);
    }
    return vm->stack[--vm->stackSize];
}

Object* newObject(VM* vm, ObjectType type) {
    // If we reached the threshold, we could trigger Mark-and-Sweep here:
    // if (vm->numObjects == vm->maxObjects) gc(vm);

    Object* object = (Object*)malloc(sizeof(Object));
    if (object == NULL) {
        fprintf(stderr, "Failed to allocate Object\n");
        exit(1);
    }
    object->type = type;
    object->marked = false;
    object->ref_count = 0; // Starts with 0 references, wait for retain()

    // Insert it into the linked list of all objects
    object->next = vm->firstObject;
    vm->firstObject = object;
    vm->numObjects++;

    return object;
}

Object* pushInt(VM* vm, int intValue) {
    Object* object = newObject(vm, OBJ_INT);
    object->value = intValue;
    push(vm, object);
    return object;
}

Object* pushPair(VM* vm) {
    Object* object = newObject(vm, OBJ_PAIR);
    object->tail = pop(vm);
    object->head = pop(vm);

    push(vm, object);
    return object;
}

void freeVM(VM* vm) {
    // We could either sweep the whole heap or rely on the GC tests.
    // For simplicity, let's just sweep everything by ensuring the stack is empty.
    vm->stackSize = 0;
    gc(vm);
    free(vm);
}
