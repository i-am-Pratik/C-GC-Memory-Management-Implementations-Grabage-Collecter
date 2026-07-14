#include "vm.h"
#include <stdio.h>

void testReferenceCounting() {
    printf("=== Test: Reference Counting ===\n");
    VM* vm = newVM();

    Object* intObj1 = pushInt(vm, 10);
    retain(intObj1); // Ref count: 1
    printf("Created intObj1, ref_count: %d, numObjects: %d\n", intObj1->ref_count, vm->numObjects);

    Object* intObj2 = pushInt(vm, 20);
    retain(intObj2); // Ref count: 1
    printf("Created intObj2, ref_count: %d, numObjects: %d\n", intObj2->ref_count, vm->numObjects);

    // Create pair pointing to intObj1 and intObj2
    Object* pair = pushPair(vm); // This pops intObj1 and intObj2 from stack, but they still have ref_count 1
    retain(pair);
    
    // As they are part of the pair, we should increment their ref count for the pair reference.
    retain(pair->head); // intObj2
    retain(pair->tail); // intObj1
    
    printf("Created pair, pair ref: %d, numObjects: %d\n", pair->ref_count, vm->numObjects);
    
    // Release intObj1 and intObj2 base references, they should still exist because pair references them
    release(vm, intObj1);
    release(vm, intObj2);
    printf("Released int objs base refs, numObjects: %d\n", vm->numObjects);
    
    // Release pair, this should free the pair and cascade to its children
    release(vm, pair);
    printf("Released pair, numObjects: %d\n", vm->numObjects);
    
    freeVM(vm);
    printf("\n");
}

void testMarkSweep() {
    printf("=== Test: Mark and Sweep ===\n");
    VM* vm = newVM();

    printf("Pushing objects to stack...\n");
    pushInt(vm, 1);
    pushInt(vm, 2);
    
    printf("Before GC: numObjects = %d\n", vm->numObjects);
    gc(vm);
    printf("After GC (with objects on stack): numObjects = %d\n", vm->numObjects);
    
    // Pop objects, making them unreachable
    pop(vm);
    pop(vm);
    
    gc(vm);
    printf("After GC (with no objects on stack): numObjects = %d\n", vm->numObjects);
    
    // Test circular reference
    printf("\nTesting circular reference...\n");
    pushInt(vm, 1);
    pushInt(vm, 2);
    Object* pair1 = pushPair(vm);
    pushInt(vm, 3);
    pushInt(vm, 4);
    Object* pair2 = pushPair(vm);
    
    // Make them point to each other
    pair1->tail = pair2;
    pair2->tail = pair1;
    
    printf("Before GC (circular ref on stack): numObjects = %d\n", vm->numObjects);
    gc(vm);
    
    // Pop them from stack
    pop(vm); // pop pair2
    pop(vm); // pop pair1
    
    printf("After popping from stack, before GC: numObjects = %d\n", vm->numObjects);
    gc(vm);
    printf("After GC: numObjects = %d\n", vm->numObjects);

    freeVM(vm);
    printf("\n");
}

int main() {
    testReferenceCounting();
    testMarkSweep();
    return 0;
}
