
#include "objects.h"
#include <stdio.h>
#include <assert.h>

void print_refcount(const char *name, object_t *obj)
{
    if (obj == NULL)
    {
        printf("%s: NULL\n", name);
    }
    else
    {
        printf("%s refcount: %d\n", name, obj->refcount);
    }
}

int main(void)
{
    printf("====================================\n");
    printf("   OBJECT REFERENCE COUNT TEST\n");
    printf("====================================\n\n");

    // ==================================
    // TEST 1: INTEGER OBJECT
    // ==================================

    printf("TEST 1: INTEGER OBJECT\n");

    object_t *integer = new_integer(42);

    assert(integer != NULL);
    assert(integer->kind == INTEGER);
    assert(integer->data.v_int == 42);
    assert(integer->refcount == 1);

    print_refcount("Integer", integer);

    refcount_inc(integer);
    print_refcount("After increment", integer);

    refcount_dec(integer);
    print_refcount("After decrement", integer);

    refcount_dec(integer);

    printf("Integer freed.\n\n");

    // ==================================
    // TEST 2: FLOAT OBJECT
    // ==================================

    printf("TEST 2: FLOAT OBJECT\n");

    object_t *floating = new_float(3.14f);

    assert(floating != NULL);
    assert(floating->kind == FLOAT);

    printf("Float value: %.2f\n", floating->data.v_float);
    print_refcount("Float", floating);

    refcount_dec(floating);

    printf("Float freed.\n\n");

    // ==================================
    // TEST 3: STRING OBJECT
    // ==================================

    printf("TEST 3: STRING OBJECT\n");

    object_t *string = new_string("Hello!");

    assert(string != NULL);
    assert(string->kind == STRING);

    printf("String value: %s\n", string->data.v_string);
    print_refcount("String", string);

    refcount_dec(string);

    printf("String freed.\n\n");

    // ==================================
    // TEST 4: VECTOR3 OBJECT
    // ==================================

    printf("TEST 4: VECTOR3 OBJECT\n");

    object_t *x = new_integer(10);
    object_t *y = new_integer(20);
    object_t *z = new_integer(30);

    object_t *vector = new_vector3(x, y, z);

    assert(vector != NULL);
    assert(vector->kind == VECTOR3);

    printf("Vector values: (%d, %d, %d)\n",
           vector->data.v_vector3.x->data.v_int,
           vector->data.v_vector3.y->data.v_int,
           vector->data.v_vector3.z->data.v_int);

    printf("\nReference counts after vector creation:\n");
    print_refcount("x", x);
    print_refcount("y", y);
    print_refcount("z", z);

    refcount_dec(x);
    refcount_dec(y);
    refcount_dec(z);

    printf("\nAfter releasing original references:\n");
    print_refcount("x", vector->data.v_vector3.x);
    print_refcount("y", vector->data.v_vector3.y);
    print_refcount("z", vector->data.v_vector3.z);

    refcount_dec(vector);

    printf("Vector and its components freed.\n\n");

    // ==================================
    // TEST 5: ARRAY CREATION
    // ==================================

    printf("TEST 5: ARRAY CREATION\n");

    object_t *array = new_array(3);

    assert(array != NULL);
    assert(array->kind == ARRAY);
    assert(array->data.v_array.size == 3);

    printf("Array size: %zu\n", array->data.v_array.size);

    for (size_t i = 0; i < array->data.v_array.size; i++)
    {
        assert(array->data.v_array.elements[i] == NULL);
        printf("Element %zu: NULL\n", i);
    }

    printf("\n");

    // ==================================
    // TEST 6: ARRAY SET AND GET
    // ==================================

    printf("TEST 6: ARRAY SET AND GET\n");

    object_t *a = new_integer(100);
    object_t *b = new_float(5.5f);
    object_t *c = new_string("Array element");

    assert(array_set(array, 0, a) == true);
    assert(array_set(array, 1, b) == true);
    assert(array_set(array, 2, c) == true);

    printf("Elements inserted successfully.\n");

    print_refcount("a", a);
    print_refcount("b", b);
    print_refcount("c", c);

    object_t *got_a = array_get(array, 0);
    object_t *got_b = array_get(array, 1);
    object_t *got_c = array_get(array, 2);

    assert(got_a == a);
    assert(got_b == b);
    assert(got_c == c);

    printf("\nRetrieved values:\n");
    printf("Integer: %d\n", got_a->data.v_int);
    printf("Float: %.2f\n", got_b->data.v_float);
    printf("String: %s\n\n", got_c->data.v_string);

    // ==================================
    // TEST 7: ARRAY REPLACEMENT
    // ==================================

    printf("TEST 7: ARRAY REPLACEMENT\n");

    object_t *replacement = new_integer(999);

    print_refcount("Original a before replacement", a);
    print_refcount("Replacement before insertion", replacement);

    assert(array_set(array, 0, replacement) == true);

    printf("\nAfter replacement:\n");

    print_refcount("Original a", a);
    print_refcount("Replacement", replacement);

    assert(array_get(array, 0) == replacement);

    printf("Array element replaced successfully.\n\n");

    // ==================================
    // TEST 8: SAME OBJECT IN MULTIPLE
    // ARRAY POSITIONS
    // ==================================

    printf("TEST 8: SHARED ARRAY REFERENCES\n");

    object_t *shared = new_integer(777);

    array_set(array, 0, shared);
    array_set(array, 1, shared);

    print_refcount("Shared object", shared);

    printf("The same object is stored in two positions.\n\n");

    // ==================================
    // TEST 9: INVALID ARRAY OPERATIONS
    // ==================================

    printf("TEST 9: INVALID ARRAY OPERATIONS\n");

    assert(array_set(NULL, 0, shared) == false);
    printf("NULL array: rejected.\n");

    assert(array_set(array, 0, NULL) == false);
    printf("NULL value: rejected.\n");

    assert(array_set(array, 10, shared) == false);
    printf("Out-of-bounds index: rejected.\n");

    assert(array_get(NULL, 0) == NULL);
    printf("Getting from NULL array: rejected.\n");

    assert(array_get(array, 10) == NULL);
    printf("Getting out-of-bounds index: rejected.\n");

    object_t *not_array = new_integer(123);

    assert(array_set(not_array, 0, shared) == false);
    printf("Setting an element in a non-array: rejected.\n");

    assert(array_get(not_array, 0) == NULL);
    printf("Getting an element from a non-array: rejected.\n");

    refcount_dec(not_array);

    printf("\n");

    // ==================================
    // TEST 10: NULL REFERENCE COUNT
    // OPERATIONS
    // ==================================

    printf("TEST 10: NULL REFERENCE COUNT OPERATIONS\n");

    refcount_inc(NULL);
    refcount_dec(NULL);

    printf("Incrementing and decrementing NULL: safe.\n\n");

    // ==================================
    // TEST 11: NESTED ARRAYS
    // ==================================

    printf("TEST 11: NESTED ARRAYS\n");

    object_t *inner = new_array(1);
    object_t *outer = new_array(1);
    object_t *nested_value = new_integer(555);

    assert(inner != NULL);
    assert(outer != NULL);
    assert(nested_value != NULL);

    array_set(inner, 0, nested_value);
    array_set(outer, 0, inner);

    print_refcount("Inner array", inner);
    print_refcount("Nested value", nested_value);

    printf("Nested array created successfully.\n");

    refcount_dec(nested_value);
    refcount_dec(inner);
    refcount_dec(outer);

    printf("Nested arrays and contents freed.\n\n");

    // ==================================
    // TEST 12: FINAL CLEANUP
    // ==================================

    printf("TEST 12: FINAL CLEANUP\n");

    // Release original references.
    refcount_dec(a);
    refcount_dec(b);
    refcount_dec(c);
    refcount_dec(replacement);
    refcount_dec(shared);

    printf("Original references released.\n");

    // Free array and all remaining elements.
    refcount_dec(array);

    printf("Array and its remaining elements freed.\n\n");

    printf("====================================\n");
    printf("       ALL TESTS COMPLETED!\n");
    printf("====================================\n");

    return 0;
}
