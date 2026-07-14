#include "vm.h"
#include <stdio.h>
#include <stdlib.h>

void mark(Object* obj) {
    if (obj == NULL || obj->marked) return;

    obj->marked = true;

    if (obj->type == OBJ_PAIR) {
        mark(obj->head);
        mark(obj->tail);
    }
}

void markAll(VM* vm) {
    for (int i = 0; i < vm->stackSize; i++) {
        mark(vm->stack[i]);
    }
}

void sweep(VM* vm) {
    Object** object = &vm->firstObject;
    while (*object) {
        if (!(*object)->marked) {
            // This object wasn't reached, so remove it from the list and free it
            Object* unreached = *object;

            *object = unreached->next;
            vm->numObjects--;
            free(unreached);
        } else {
            // This object was reached, so unmark it for the next GC cycle
            (*object)->marked = false;
            object = &(*object)->next;
        }
    }
}

void gc(VM* vm) {
    int numObjects = vm->numObjects;

    markAll(vm);
    sweep(vm);

    vm->maxObjects = vm->numObjects == 0 ? INITIAL_GC_THRESHOLD : vm->numObjects * 2;
    
    // Optional: Print GC stats
    // printf("Collected %d objects, %d remaining.\n", numObjects - vm->numObjects, vm->numObjects);
}
