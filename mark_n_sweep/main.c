
#include "vm.h"
#include "object_new.h"
#include <stdio.h>
#include <assert.h>

void print_objects(vm_t *vm, const char *message)
{
    printf("%s\n", message);
    printf("Objects tracked by VM: %zu\n\n", vm->objects->count);
}

int main(void)
{
    printf("====================================\n");
    printf("   MARK AND SWEEP GARBAGE COLLECTOR\n");
    printf("====================================\n\n");

    vm_t *vm = vm_new();
    assert(vm != NULL);

    // ==================================
    // TEST 1: OBJECT CREATION
    // ==================================

    printf("TEST 1: OBJECT CREATION\n");

    object_t *num = new_integer(vm, 42);
    object_t *floating = new_float(vm, 3.14f);
    object_t *string = new_string(vm, "Hello Snek!");

    assert(num != NULL);
    assert(floating != NULL);
    assert(string != NULL);

    printf("Integer: %d\n", num->data.v_int);
    printf("Float: %.2f\n", floating->data.v_float);
    printf("String: %s\n", string->data.v_string);

    print_objects(vm, "After creating three objects:");

    // ==================================
    // TEST 2: VECTOR CREATION
    // ==================================

    printf("TEST 2: VECTOR CREATION\n");

    object_t *x = new_integer(vm, 10);
    object_t *y = new_integer(vm, 20);
    object_t *z = new_integer(vm, 30);

    object_t *vector = new_vector3(vm, x, y, z);

    assert(vector != NULL);

    printf("Vector: (%d, %d, %d)\n",
           vector->data.v_vector3.x->data.v_int,
           vector->data.v_vector3.y->data.v_int,
           vector->data.v_vector3.z->data.v_int);

    print_objects(vm, "After creating vector and components:");

    // ==================================
    // TEST 3: ARRAY CREATION
    // ==================================

    printf("TEST 3: ARRAY CREATION\n");

    object_t *array = new_array(vm, 4);

    assert(array != NULL);

    array_set(array, 0, num);
    array_set(array, 1, string);
    array_set(array, 2, vector);
    array_set(array, 3, floating);

    printf("Array created with %zu elements.\n",
           array->data.v_array.size);

    print_objects(vm, "After creating array:");

    // ==================================
    // TEST 4: STACK FRAME ROOTS
    // ==================================

    printf("TEST 4: STACK FRAME ROOTS\n");

    frame_t *frame = vm_new_frame(vm);

    assert(frame != NULL);

    // The array is now a root object.
    frame_reference_object(frame, array);

    printf("Array added to stack frame.\n");

    // ==================================
    // TEST 5: CREATE UNREACHABLE OBJECTS
    // ==================================

    printf("TEST 5: CREATE GARBAGE OBJECTS\n");

    object_t *garbage1 = new_integer(vm, 999);
    object_t *garbage2 = new_string(vm, "I am garbage!");

    object_t *garbage_array = new_array(vm, 2);
    object_t *garbage_child = new_float(vm, 99.9f);

    array_set(garbage_array, 0, garbage_child);

    assert(garbage1 != NULL);
    assert(garbage2 != NULL);
    assert(garbage_array != NULL);
    assert(garbage_child != NULL);

    printf("Created four unreachable objects.\n");

    print_objects(vm, "Before garbage collection:");

    // ==================================
    // TEST 6: MARK AND SWEEP
    // ==================================

    printf("TEST 6: FIRST GARBAGE COLLECTION\n");

    vm_collect_garbage(vm);

    print_objects(vm, "After garbage collection:");

    // All objects reachable from array should survive.
    assert(array->data.v_array.elements[0]->data.v_int == 42);
    assert(array->data.v_array.elements[1]->kind == STRING);
    assert(array->data.v_array.elements[2]->kind == VECTOR3);
    assert(array->data.v_array.elements[3]->data.v_float == 3.14f);

    printf("Reachable objects survived!\n");
    printf("Unreachable objects were collected!\n\n");

    // ==================================
    // TEST 7: NESTED OBJECT REFERENCES
    // ==================================

    printf("TEST 7: NESTED OBJECT REFERENCES\n");

    object_t *inner = new_array(vm, 1);
    object_t *nested_num = new_integer(vm, 555);

    array_set(inner, 0, nested_num);

    // Attach the inner array to the already-rooted outer array.
    // Replace the previous string reference.
    array_set(array, 1, inner);

    // The old string is now unreachable.
    // The nested array and nested integer are reachable.

    vm_collect_garbage(vm);

    assert(array->data.v_array.elements[1] == inner);
    assert(inner->data.v_array.elements[0] == nested_num);

    printf("Nested array survived collection.\n");
    printf("Nested integer survived collection.\n\n");

    // ==================================
    // TEST 8: REMOVE ROOT
    // ==================================

    printf("TEST 8: REMOVE ROOT REFERENCE\n");

    frame_t *removed_frame = vm_frame_pop(vm);

    assert(removed_frame == frame);

    frame_free(removed_frame);

    printf("Root frame removed.\n");
    printf("Array is no longer reachable from a root.\n\n");

    // ==================================
    // TEST 9: COLLECT EVERYTHING
    // ==================================

    printf("TEST 9: SECOND GARBAGE COLLECTION\n");

    vm_collect_garbage(vm);

    print_objects(vm, "After removing the root and collecting:");

    assert(vm->objects->count == 0);

    printf("All unreachable objects collected!\n\n");

    // ==================================
    // TEST 10: CLEANUP
    // ==================================

    printf("TEST 10: VM CLEANUP\n");

    vm_free(vm);

    printf("Virtual machine freed successfully.\n\n");

    printf("====================================\n");
    printf("       ALL TESTS COMPLETED!\n");
    printf("====================================\n");

    return 0;
}
