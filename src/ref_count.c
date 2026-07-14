#include "vm.h"
#include <stdio.h>
#include <stdlib.h>

void retain(Object* obj) {
    if (obj == NULL) return;
    obj->ref_count++;
}

void release(VM* vm, Object* obj) {
    if (obj == NULL) return;
    if (obj->ref_count > 0) {
        obj->ref_count--;
    }

    if (obj->ref_count == 0) {
        // If it's a pair, we need to release its children
        if (obj->type == OBJ_PAIR) {
            release(vm, obj->head);
            release(vm, obj->tail);
        }

        // Before freeing, we must remove it from the VM's linked list to keep it consistent
        Object** curr = &vm->firstObject;
        while (*curr != NULL) {
            if (*curr == obj) {
                *curr = obj->next;
                break;
            }
            curr = &(*curr)->next;
        }
        
        vm->numObjects--;
        free(obj);
    }
}
