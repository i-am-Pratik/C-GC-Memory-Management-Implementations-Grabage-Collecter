# Object Memory Management — Reference Counting & Mark-and-Sweep Garbage Collection

This project is a C-based implementation of two fundamental memory management and garbage collection algorithms: **Reference Counting** and **Mark-and-Sweep**.

The project explores how dynamically allocated objects are created, stored, referenced, tracked, and eventually freed from memory. It demonstrates how different garbage collection strategies manage object lifetimes, handle nested data structures, identify unused memory, and prevent memory leaks.

The project is divided into two independent implementations, each demonstrating a different approach to memory management:

- **Reference Counting:** Tracks how many active references point to an object and frees it when its reference count reaches zero.
- **Mark-and-Sweep:** Tracks objects through a virtual machine (VM), identifies reachable objects using a marking and tracing process, and frees unreachable objects during the sweep phase.

Both implementations work with the same fundamental object concepts, including integers, floating-point numbers, strings, 3D vectors, and arrays. However, they manage object lifetimes differently.

> [!NOTE]
> **Inspiration & Learning Reference:** This project was inspired by and developed with reference to the [Memory Management in C](https://www.boot.dev) course by **Boot.dev**, which provided the foundational concepts and learning direction for implementing reference counting and mark-and-sweep garbage collection in C.

---

## Table of Contents

1. [Core Concepts & Technologies Used](#1-core-concepts--technologies-used)
2. [Understanding Garbage Collection](#2-understanding-garbage-collection)
3. [Object System & Memory Representation](#3-object-system--memory-representation)
4. [Reference Counting Garbage Collection](#4-reference-counting-garbage-collection)
5. [Mark-and-Sweep Garbage Collection](#5-mark-and-sweep-garbage-collection)
6. [The Three Stages of Mark-and-Sweep](#6-the-three-stages-of-mark-and-sweep)
7. [Object Types and Their Importance in Mark-and-Sweep](#7-object-types-and-their-importance-in-mark-and-sweep)
8. [Virtual Machine (VM)](#8-virtual-machine-vm)
9. [Stack Data Structure](#9-stack-data-structure)
10. [Mark-and-Sweep Implementation Details](#10-mark-and-sweep-implementation-details)
11. [Reference Counting vs Mark-and-Sweep](#11-reference-counting-vs-mark-and-sweep)
12. [Advantages and Disadvantages of Mark-and-Sweep](#12-advantages-and-disadvantages-of-mark-and-sweep)
13. [Project Testing & Demonstration](#13-project-testing--demonstration)
14. [Project Structure](#14-project-structure)
15. [Memory Management Lifecycle Comparison](#15-memory-management-lifecycle-comparison)
16. [Important Memory Management Considerations](#16-important-memory-management-considerations)
17. [Learning Outcomes](#17-learning-outcomes)
18. [Conclusion](#18-conclusion)

---

## 1. Core Concepts & Technologies Used

| Concept / Feature | Implementation | Role in the Project |
|---|---|---|
| C Programming Language | Entire project | Provides low-level memory management and direct control over dynamically allocated memory. |
| Dynamic Memory Allocation | `malloc()`, `calloc()`, `free()` | Allocates memory for objects, arrays, virtual machines, and stack structures at runtime. |
| Structures (`struct`) | `object_t`, `vm_t`, `frame_t` | Groups related data into structured representations. |
| Enumerations (`enum`) | `object_kind_t` | Defines the supported object types, such as `INTEGER`, `FLOAT`, `STRING`, `VECTOR3`, and `ARRAY`. |
| Unions (`union`) | `object_data_t` | Allows different object types to share the same memory space while storing type-specific values. |
| Pointers | Object references and array elements | Connects objects together and enables dynamic data structures. |
| Reference Counting | `refcount_inc()`, `refcount_dec()` | Tracks the number of references pointing to each object. |
| Mark-and-Sweep | `mark()`, `trace()`, `sweep()` | Identifies reachable and unreachable objects and reclaims unused memory. |
| Stack Data Structure | `stack_t` | Stores VM objects, stack frames, frame references, and temporary gray objects during tracing. |
| Virtual Machine (VM) | `vm_t` | Tracks allocated objects and maintains stack frames containing root references. |
| Assertions | `assert()` | Validates expected object states and verifies test results. |
| Standard C Libraries | `stdlib.h`, `stdio.h`, `string.h`, `stdbool.h`, `stddef.h` | Provides memory allocation, input/output, string operations, boolean types, and size definitions. |

---

## 2. Understanding Garbage Collection

Garbage collection is a memory management technique used to automatically identify and reclaim dynamically allocated memory that is no longer needed by a program.

In C, memory allocated using `malloc()` or `calloc()` must normally be released manually using `free()`.

For example, when an object is allocated dynamically, the program becomes responsible for managing its lifetime.

- If the program forgets to release an object, it can cause a **memory leak**.
- A memory leak occurs when allocated memory is no longer useful but remains occupied because it has not been freed.
- On the other hand, freeing memory too early can cause a **dangling pointer**, where a pointer still refers to memory that has already been released.

Garbage collection addresses these problems by introducing mechanisms that determine whether an allocated object is still needed.

Our project demonstrates two different approaches to solving this problem.

### Why Do We Need Garbage Collection?

Consider the following situation:

1. An integer object is dynamically allocated.
2. An array stores a pointer to that integer.
3. Another object stores a reference to the array.
4. The program stops using the original integer pointer.

Even though the original pointer is no longer being used, the integer is still accessible through the array. Therefore, it should not be freed.

This introduces an important distinction:

- An object can lose its original reference but still be **reachable** through another object.
- An object that cannot be reached through any valid program reference is considered **garbage**.

A garbage collector must understand these relationships before deciding which objects can safely be freed.

---

## 3. Object System & Memory Representation

Both implementations use a common conceptual object model.

The project supports five object types:

| Object Type | Description | Stored Data |
|---|---|---|
| `INTEGER` | Stores an integer value. | `int` |
| `FLOAT` | Stores a floating-point value. | `float` |
| `STRING` | Stores dynamically allocated character data. | `char *` |
| `VECTOR3` | Stores references to three objects representing X, Y, and Z components. | Three object pointers |
| `ARRAY` | Stores multiple object references in a dynamically allocated array. | Object pointer array |

### 3.1 Structures (`struct`)

Structures allow us to group related variables into one logical data type. The project uses structures to represent objects, arrays, vectors, virtual machines, and stack frames.

#### Object Structure

The central object representation is `object_t`. Each object contains:

- A **marking flag** used by Mark-and-Sweep.
- A **type identifier** describing the object's kind.
- A **union** containing the actual data associated with that type.

The marking flag is particularly important for Mark-and-Sweep because it indicates whether an object has been identified as reachable.

The reference-counting implementation instead maintains a **reference count** in its object structure.

This means the two implementations have slightly different object metadata to support their respective garbage collection strategies.

#### Array Structure

An array structure contains:

- `size`: The number of elements in the array.
- `elements`: A pointer to a dynamically allocated collection of object pointers.

This allows an array to store references to different object types. For example, an array can contain an integer, a string, a float, and a vector simultaneously.

#### Vector Structure

A vector structure contains three object pointers: `x`, `y`, `z`.

Rather than storing three primitive numeric values directly, the vector stores references to three independent objects. This is important because the garbage collector must understand that a vector owns references to its three components.

### 3.2 Enumerations (`enum`)

Enumerations define a collection of named integer constants. Our project uses an enumeration to identify the different object types.

The supported types are: `INTEGER`, `FLOAT`, `STRING`, `VECTOR3`, `ARRAY`.

**Why do we need this?**

Since every object uses the same general object structure, the program needs a way to determine which kind of data is currently stored inside that object. The enumeration provides this information.

For example, when an object has the type `STRING`, the program knows that its data should be interpreted as a character pointer rather than an integer or vector. The type identifier is also used in `switch` statements when freeing objects and tracing references.

### 3.3 Unions (`union`)

A union allows multiple variables of different types to occupy the same memory location. Our project uses a union called `object_data_t` to store the actual value associated with an object.

It supports:

- Integer values.
- Floating-point values.
- String pointers.
- Vector structures.
- Array structures.

**Why use a union instead of storing everything separately?**

Without a union, every object would need to reserve memory for every possible data type, even though an object only uses one type at a time. A union reduces unnecessary memory consumption because all its members share the same memory region. The enumeration tells us which union member is currently valid. Together, the `enum` and `union` provide a flexible object representation.

### 3.4 Pointers and Dynamic Memory Allocation

Pointers are essential throughout this project because objects are dynamically allocated and connected to one another.

We use:

- `malloc()` to allocate memory dynamically.
- `calloc()` to allocate memory and initialize it to zero.
- `free()` to release allocated memory.

For example, when creating an array:

1. Memory is allocated for the object itself.
2. Additional memory is allocated for its element pointers.
3. Each element initially contains `NULL` because `calloc()` initializes allocated memory to zero.

This allows arrays to be created with a predefined capacity and populated later. Strings also require separate dynamic allocation because their character data must remain available independently of the original input string.

---

## 4. Reference Counting Garbage Collection

Reference Counting is a memory management algorithm that maintains a counter for every allocated object. The counter represents how many active references point to that object.

- Whenever a new reference to an object is created, its reference count **increases**.
- Whenever a reference is removed, its reference count **decreases**.
- When the count reaches **zero**, the object is no longer referenced and can be freed.

### 4.1 How Reference Counting Works

The basic lifecycle is:

1. Create an object.
2. Initialize its reference count to `1`.
3. Increase the count whenever another reference is created.
4. Decrease the count whenever a reference is removed.
5. Free the object when its reference count reaches zero.

#### Example

Consider an integer object:

| Event | Reference Count |
|---|---|
| Object created | `1` |
| Another reference created | `2` |
| One reference removed | `1` |
| Final reference removed | `0` → **Object freed** |

This allows memory to be reclaimed immediately when an object's reference count reaches zero.

### 4.2 Reference Counting Implementation

The reference-counting implementation is located in the `ref_count` directory.

#### Object Creation

Every object is created through its corresponding constructor. The internal object creation mechanism:

1. Allocates memory for the object using `calloc()`.
2. Initializes the reference count to `1`.
3. Returns the newly allocated object.

The reference count starts at `1` because the newly created object has one initial owning reference.

The project supports constructors for:

- `new_integer()`
- `new_float()`
- `new_string()`
- `new_vector3()`
- `new_array()`

Each constructor initializes the object's type and corresponding data.

#### Reference Increment

The increment operation increases an object's reference count. It first checks whether the object pointer is `NULL`. If it is `NULL`, the function safely returns without doing anything. Otherwise, the reference count increases by one.

This operation is required whenever another object or variable acquires ownership of an existing object.

#### Reference Decrement

The decrement operation reduces an object's reference count. If the object pointer is `NULL`, it returns safely. Otherwise, it decreases the count. When the count becomes zero, the object is passed to the object-freeing function.

This ensures that objects are released once their final reference is removed.

#### Recursive Object Freeing

The object-freeing function uses a `switch` statement to determine how each object should be released. Different object types require different cleanup operations.

| Object Type | Cleanup Operation |
|---|---|
| `INTEGER` | No additional allocation needs to be released. |
| `FLOAT` | No additional allocation needs to be released. |
| `STRING` | Frees the dynamically allocated character buffer. |
| `VECTOR3` | Decrements the reference counts of X, Y, and Z. |
| `ARRAY` | Decrements the reference counts of its elements and frees the element-pointer array. |

After type-specific cleanup, the object itself is freed.

The most important part is that vectors and arrays can contain references to other objects. Therefore, freeing a vector or array must also release the references it owns. This can trigger further object freeing when those referenced objects reach a reference count of zero. As a result, reference counting can **recursively** clean up entire object structures.

### 4.3 Reference Counting and Arrays

Arrays require special handling because they can contain multiple object references.

When an object is inserted into an array:

1. The array and index are validated.
2. The reference count of the inserted object is increased.
3. If the array position already contains an object, its reference count is decreased.
4. The new object pointer is assigned to that position.

This is important because replacing an array element removes the array's previous reference to the old object. The new object receives an additional reference because the array now points to it.

#### Array Retrieval

Retrieving an array element does not automatically increment its reference count. The getter simply returns the stored pointer. This is important when interpreting ownership: retrieving a pointer does not necessarily create a new owning reference.

### 4.4 Handling Shared References

One important feature of reference counting is its ability to manage shared objects. Consider an integer object referenced by two different positions in an array.

- Its reference count increases for each stored reference.
- When one position is replaced or removed, the count decreases by one.
- The object remains allocated as long as another reference still exists.
- Only when all references are removed does its count reach zero.

This prevents an object from being freed while another part of the program still references it.

### 4.5 Reference Counting: Advantages and Disadvantages

#### Advantages

- Simple to understand and implement.
- Objects can be freed immediately when their reference count reaches zero.
- Does not require scanning the entire object collection during normal reference updates.
- Memory is reclaimed incrementally as references disappear.
- Shared references are handled naturally when increments and decrements are performed correctly.

#### Disadvantages

- Every reference creation and removal must be tracked correctly.
- Reference counting introduces additional operations whenever references change.
- Incorrect increments or decrements can cause memory leaks or premature freeing.
- Circular references can prevent objects from ever reaching a reference count of zero.

#### The Circular Reference Problem

Circular references are one of the biggest limitations of reference counting. Consider two objects:

- **Object A** references **Object B**.
- **Object B** references **Object A**.

Even if the program removes every external reference to both objects, each object still has one reference from the other. Their reference counts never reach zero. Therefore, neither object is automatically freed. **This creates a memory leak.**

This limitation motivates the use of other garbage collection algorithms, such as Mark-and-Sweep.

---

## 5. Mark-and-Sweep Garbage Collection

Mark-and-Sweep is a garbage collection algorithm that identifies objects that are still reachable from a set of known starting references, called **roots**.

Instead of maintaining a reference count for every object, it periodically examines the object graph to determine which objects are still accessible.

It operates in three main stages:

1. **Mark**
2. **Trace**
3. **Sweep**

The algorithm is particularly useful for managing complex object relationships, including nested arrays, vectors, and circular references.

### 5.1 Understanding Reachability

Reachability determines whether an object can still be accessed by the program.

The garbage collector starts from **root objects**. Roots are references held by active stack frames. From these roots, the collector follows references to other objects.

For example:

- A stack frame references an array.
- The array references a vector.
- The vector references three integer objects.

Even if the integer objects do not appear directly in the stack frame, they are still reachable through the array and vector. Therefore, all these objects must survive garbage collection.

Any object that cannot be reached through this process is considered **garbage**.

---

## 6. The Three Stages of Mark-and-Sweep

### 6.1 Mark Phase

The first stage identifies the root objects. In our implementation, roots are stored inside stack frames maintained by the virtual machine.

The marking process:

1. Iterates through all active stack frames.
2. Iterates through the references stored in each frame.
3. Retrieves each referenced object.
4. Sets its marking flag to `true`.

This identifies the objects directly referenced by the program. However, objects referenced by those roots have not necessarily been marked yet. That is why the tracing stage is required.

### 6.2 Trace Phase

The tracing phase follows references from already marked objects. Our implementation uses a temporary stack called the **gray object stack**.

The gray stack contains objects that have been identified as reachable but whose internal references still need to be examined.

The tracing process works as follows:

1. A temporary gray stack is created.
2. All previously marked root objects are pushed onto it.
3. The collector repeatedly pops an object.
4. It examines the object's type.
5. It follows any references stored inside that object.
6. Newly discovered objects are marked and pushed onto the gray stack.
7. The process continues until no gray objects remain.

Once the gray stack is empty, every object reachable from the roots has been marked.

#### Why Do We Need Tracing?

Marking alone only identifies direct references from stack frames. It does not automatically identify objects referenced indirectly through arrays or vectors. Tracing solves this problem by recursively exploring the object graph.

For example:

```
Stack Frame → Array → Vector → Integer Objects
```

The tracing process follows every relationship until it has explored the entire reachable object graph.

#### The Gray Object Stack

The gray object stack is an important part of our tracing implementation. It avoids requiring recursive function calls to explore nested objects. Instead, it uses an **iterative approach**.

When an object is discovered:

1. It is marked.
2. It is pushed onto the gray stack.
3. Its references are examined when it is later popped.

This also prevents repeatedly processing an object that has already been marked. The temporary stack is released after tracing is complete.

### 6.3 Sweep Phase

The final stage removes objects that were not marked. The collector iterates through the VM's tracked object collection.

For each object:

- If its marking flag is `true`, it is **reachable** and must be preserved.
- If its marking flag is `false`, it is **unreachable** and can be freed.

Unreachable objects are removed from the VM's tracked collection. After collection, surviving objects have their marking flags reset so that the next garbage collection cycle starts with a clean marking state.

The sweep stage is responsible for actually reclaiming unused memory.

---

## 7. Object Types and Their Importance in Mark-and-Sweep

Mark-and-Sweep must understand how each object stores references to other objects. This is why the different object types are important.

#### `INTEGER`
An integer contains only a primitive integer value. It does not reference any other objects. Therefore, tracing an integer does not require following any additional pointers.

#### `FLOAT`
A float contains a primitive floating-point value. Like integers, it does not contain references to other objects. Therefore, it requires no additional tracing.

#### `STRING`
A string stores a dynamically allocated character buffer. The string owns its character buffer, but it does not reference another managed object. The object-freeing operation must release the character buffer when the string becomes unreachable.

#### `VECTOR3`
A vector contains three object pointers: `X`, `Y`, `Z`. These references must be followed during tracing. If the vector is reachable, all three component objects must also remain reachable. For example, a vector may contain references to three independently allocated integer objects. If the vector is marked, the tracing phase marks those three integers as well.

#### `ARRAY`
An array contains a collection of object pointers. Every element must be examined during tracing. If the array is reachable, every non-`NULL` object stored inside it must also be marked.

This supports:

- Arrays containing primitive objects.
- Arrays containing strings.
- Arrays containing vectors.
- Nested arrays.
- Shared object references.

#### Why These Objects Matter

The garbage collector cannot simply mark the root objects and assume everything else is garbage. It must understand the internal relationships between objects.

The vector and array types make this especially important because they introduce object-to-object references. Without tracing these relationships, the collector could incorrectly free objects that are still accessible through another object.

---

## 8. Virtual Machine (VM)

The Virtual Machine is the central component of the Mark-and-Sweep implementation. It maintains information about allocated objects and the references that make those objects reachable.

The VM contains two important collections:

| VM Component | Description |
|---|---|
| `frames` | Stores the active stack frames. |
| `objects` | Stores all objects currently tracked by the VM. |

The VM provides a central location from which the garbage collector can discover objects, identify roots, trace references, and reclaim unreachable memory.

### 8.1 Object Tracking

Whenever a new object is created, it is registered with the VM. This means the VM maintains a collection of all allocated objects that it is responsible for tracking.

The object collection is important because the sweep phase needs to inspect every tracked object. Without this collection, the garbage collector would have no complete record of which allocated objects it needs to examine.

### 8.2 Stack Frames

A stack frame represents a collection of references that are currently considered roots. Each frame contains a reference stack. When an object is referenced from an active frame, it is registered as a root.

The marking phase iterates through these frames to discover the objects that must survive.

The VM supports:

- Creating new frames.
- Pushing frames onto the VM.
- Popping frames from the VM.
- Freeing individual frames.
- Registering object references inside frames.

### 8.3 Why Do We Need Stack Frames?

Imagine an object that is actively being used by the program. If the garbage collector does not know that the object is still in use, it may incorrectly identify it as garbage.

Stack frames solve this problem by explicitly recording references that must be treated as roots. The collector begins its reachability analysis from these roots.

### 8.4 VM Lifecycle

The VM lifecycle consists of:

1. Creating the VM.
2. Initializing the frame and object stacks.
3. Creating and tracking objects.
4. Creating stack frames.
5. Registering references in frames.
6. Running garbage collection.
7. Removing frames when they are no longer needed.
8. Freeing the VM and its remaining resources.

The VM also provides a cleanup operation that releases its stack frames, tracked objects, stack containers, and the VM itself.

---

## 9. Stack Data Structure

The project uses a stack implementation represented by `stack_t`. A stack is a **Last-In, First-Out (LIFO)** data structure. This means the most recently inserted element is the first element removed.

The stack provides operations such as:

- Creating a stack.
- Pushing elements.
- Popping elements.
- Freeing a stack.

Stacks are used in multiple parts of the Mark-and-Sweep implementation.

### 9.1 VM Object Stack

Stores all objects tracked by the VM. This allows the garbage collector to iterate over every allocated object during the sweep phase.

### 9.2 VM Frame Stack

Stores the active stack frames. The marking phase iterates through this stack to identify root references.

### 9.3 Frame Reference Stack

Each frame has its own reference stack. This stores the objects that are directly referenced by that frame. These references are the starting points for reachability analysis.

### 9.4 Gray Object Stack

The tracing phase uses a temporary stack to store objects that have been marked but still need their internal references examined. This allows the collector to traverse complex object graphs iteratively.

#### Why Use a Stack?

Stacks provide a convenient way to process objects that need to be explored. Instead of relying on recursive calls, the tracing algorithm explicitly manages pending objects. This makes the traversal easier to control and helps avoid excessive recursion depth when dealing with deeply nested structures.

---

## 10. Mark-and-Sweep Implementation Details

The Mark-and-Sweep implementation is organized around several functions.

#### `vm_new()`
Creates a new virtual machine. It dynamically allocates the VM structure and initializes the frame and object stacks.

#### `vm_track_object()`
Registers a newly allocated object inside the VM's object collection. This ensures that the collector can later inspect and potentially free it.

#### `vm_new_frame()`
Creates a new stack frame and initializes its reference stack. The new frame is then registered with the VM.

#### `vm_frame_push()`
Adds an existing frame to the VM's frame stack.

#### `vm_frame_pop()`
Removes and returns the most recently added frame.

#### `frame_reference_object()`
Registers an object as a reference inside a stack frame. These references become roots during garbage collection.

#### `frame_free()`
Releases the reference stack and the frame structure itself.

#### `mark()`
Iterates through all active frames and marks every object directly referenced by them.

#### `trace()`
Creates a temporary gray object stack, adds marked objects to it, and processes their internal references until every reachable object has been discovered.

#### `trace_blacken_object()`
Examines the object type and follows its references.
- For vectors, it processes X, Y, and Z.
- For arrays, it processes every element.
- For integers, floats, and strings, no additional managed-object references need to be followed.

#### `trace_mark_object()`
Checks whether an object is `NULL` or already marked. If neither condition applies, it marks the object and pushes it onto the gray stack.

#### `sweep()`
Iterates through all VM-tracked objects, frees unreachable objects, removes them from the tracked collection, and resets the marking state of surviving objects.

#### `vm_collect_garbage()`
Coordinates the complete garbage collection cycle. It combines the marking, tracing, and sweeping stages into one operation.

#### `vm_free()`
Releases the VM's remaining resources, including stack frames, tracked objects, stack containers, and the VM structure.

---

## 11. Reference Counting vs Mark-and-Sweep

Although both algorithms solve the same fundamental memory management problem, they use different approaches.

| Feature | Reference Counting | Mark-and-Sweep |
|---|---|---|
| Memory management approach | Tracks references to individual objects. | Tracks reachability from roots. |
| Object metadata | Reference count. | Marking flag. |
| When collection happens | When a reference count reaches zero. | During an explicit garbage collection cycle. |
| Object freeing | Immediate when count reaches zero. | During the sweep phase. |
| Circular references | Cannot automatically collect unreachable cycles. | Can collect unreachable cycles. |
| Tracking requirements | Every reference change must be managed. | Roots and object relationships must be tracked. |
| Runtime overhead | Increment and decrement operations. | Periodic graph traversal. |
| Memory reclamation | Incremental. | Batch-based. |
| Implementation complexity | Relatively simple. | More involved because of VM, roots, tracing, and sweeping. |

### Which Algorithm Is Better?

Neither algorithm is universally better.

- **Reference Counting** is useful when immediate memory reclamation is desirable and object relationships are manageable.
- **Mark-and-Sweep** is useful when the program contains complex object graphs and circular references that reference counting cannot reclaim.

The choice depends on the application's memory management requirements.

---

## 12. Advantages and Disadvantages of Mark-and-Sweep

### Advantages

- Can reclaim unreachable circular references.
- Does not require maintaining a reference count for every object.
- Handles complex object relationships.
- Supports nested arrays and vectors.
- Identifies garbage based on actual reachability.
- Can reclaim multiple unreachable objects in a single collection cycle.

### Disadvantages

- Requires periodically scanning the object collection.
- Tracing introduces additional processing overhead.
- Garbage collection can temporarily pause normal program execution.
- Requires accurate root tracking.
- Requires every object type to be handled correctly during tracing.
- Can use additional temporary memory for the gray object stack.

### Stop-the-World Consideration

A conventional Mark-and-Sweep collector may pause normal program execution while it marks, traces, and sweeps objects. Our implementation demonstrates this basic approach. It is not a concurrent or incremental garbage collector.

---

## 13. Project Testing & Demonstration

Both implementations include test programs that demonstrate their respective memory management strategies. The tests validate object creation, references, nested structures, memory reclamation, and invalid operations.

### 13.1 Reference Counting Tests

| Test | Description |
|---|---|
| Test 1 | Creates an integer and tests reference increment and decrement. |
| Test 2 | Creates and frees a floating-point object. |
| Test 3 | Creates and frees a string object. |
| Test 4 | Creates a vector containing three integer objects and verifies reference ownership. |
| Test 5 | Creates an array and verifies its initial elements are `NULL`. |
| Test 6 | Inserts integer, float, and string objects into an array and retrieves them. |
| Test 7 | Replaces an existing array element and verifies reference count changes. |
| Test 8 | Stores the same object in multiple array positions to demonstrate shared references. |
| Test 9 | Tests invalid array operations, including `NULL` pointers, invalid indices, and non-array objects. |
| Test 10 | Tests safe increment and decrement operations on `NULL` pointers. |
| Test 11 | Creates nested arrays and verifies recursive reference handling. |
| Test 12 | Releases original references and frees the remaining array and its elements. |

These tests demonstrate how reference counting manages ownership and object lifetimes.

### 13.2 Mark-and-Sweep Tests

| Test | Description |
|---|---|
| Test 1 | Creates integer, float, and string objects and verifies VM tracking. |
| Test 2 | Creates a vector and its three component objects. |
| Test 3 | Creates an array containing different object types. |
| Test 4 | Creates a stack frame and registers the array as a root. |
| Test 5 | Creates unreachable garbage objects, including a nested garbage array. |
| Test 6 | Runs the first garbage collection cycle and verifies that reachable objects survive. |
| Test 7 | Creates nested arrays and verifies that tracing preserves indirectly reachable objects. |
| Test 8 | Removes the root frame to make the previously reachable object graph unreachable. |
| Test 9 | Runs another collection cycle and verifies that all tracked objects are reclaimed. |
| Test 10 | Frees the virtual machine and its remaining resources. |

These tests demonstrate how the collector identifies roots, follows references, and reclaims unreachable objects.

---

## 14. Project Structure

The project is divided into two independent directories, each containing its own implementation.

```
Object-Memory-Management/
│
├── mark_n_sweep/
│   │
│   ├── main.c       # Mark-and-Sweep demonstration and tests
│   ├── objects.h    # Object structures, enums, unions and object declarations
│   ├── objects.c    # Object freeing and array operations
│   ├── new.h        # Object constructor declarations
│   ├── new.c        # Dynamic object creation and VM tracking
│   ├── vm.h         # Virtual machine, stack frame and GC declarations
│   ├── vm.c         # VM management, marking, tracing and sweeping
│   ├── stack.h      # Stack data structure declarations
│   └── stack.c      # Stack implementation
│
└── ref_count/
    │
    ├── main.c       # Reference Counting demonstration and tests
    ├── objects.h    # Object structures, enums, unions and declarations
    └── objects.c    # Object constructors, reference counting and cleanup
```

> [!NOTE]
> The filenames above reflect the renamed object-based naming convention. Adjust individual filenames if your actual source files use different names.

---

## 15. Memory Management Lifecycle Comparison

The following diagrams summarize how both implementations manage an object's lifetime.

### Reference Counting Lifecycle

```
       OBJECT CREATION
             |
             v
     Reference Count = 1
             |
             v
    NEW REFERENCE CREATED
             |
             v
       Increment Count
             |
             v
      REFERENCE REMOVED
             |
             v
       Decrement Count
             |
             v
       Is Count == 0?
          /       \
        No        Yes
        |          |
        v          v
   Keep Object  Free Object
        |          |
        v          v
    Continue   Memory Reclaimed
```

### Mark-and-Sweep Lifecycle

```
       OBJECT CREATION
             |
             v
      VM TRACKS OBJECT
             |
             v
      ROOT REFERENCES
             |
             v
         MARK PHASE
             |
             v
        TRACE PHASE
             |
             v
  FOLLOW OBJECT REFERENCES
             |
             v
        SWEEP PHASE
             |
          /       \
       Marked    Unmarked
        |            |
        v            v
   Keep Object   Free Object
        |            |
        v            v
    Reset Mark   Memory Reclaimed
```

---

## 16. Important Memory Management Considerations

Implementing garbage collection in C requires careful handling of pointers and object ownership. This project demonstrates several important considerations.

#### 1. NULL Pointer Safety
Functions that operate on object pointers should validate their inputs before accessing object data. The array operations and reference-counting operations include `NULL` checks to avoid invalid pointer dereferences.

#### 2. Memory Allocation Failure
Dynamic memory allocation can fail. Constructors should check the result of `malloc()` and `calloc()` before accessing allocated memory. If allocation fails, partially created resources must be released.

#### 3. Ownership of Nested Objects
Arrays and vectors contain references to other objects. The garbage collector must understand these relationships. Reference Counting must increase and decrease the appropriate reference counts. Mark-and-Sweep must follow these references during tracing.

#### 4. Double Free Prevention
An object must not be freed more than once. Reference Counting relies on correct reference bookkeeping. Mark-and-Sweep relies on maintaining an accurate tracked-object collection and removing objects when they are freed.

#### 5. Circular References
Reference Counting cannot automatically reclaim unreachable circular structures. Mark-and-Sweep can identify these structures as unreachable when they are no longer connected to any root.

#### 6. Root Management
Mark-and-Sweep depends on correct root registration. If an actively used object is not registered as a root or reachable through another root, the collector may incorrectly classify it as garbage.

#### 7. Object Relationships
Every object type that can contain references must be handled by the tracing algorithm. If a new object type is introduced in the future, its internal references must also be included in tracing.

---

## 17. Learning Outcomes

This project demonstrates several important systems programming concepts:

- Understanding dynamic memory allocation and deallocation in C.
- Designing custom object representations using structures, enumerations, and unions.
- Working with pointers and nested object relationships.
- Understanding object ownership and reference lifetimes.
- Implementing reference counting manually.
- Understanding the limitations of reference counting.
- Implementing a Mark-and-Sweep garbage collector.
- Understanding roots, reachability, marking, tracing, and sweeping.
- Designing and using stack-based data structures.
- Building a virtual machine to track allocated objects.
- Handling nested arrays, vectors, and shared object references.
- Understanding circular references and memory leaks.
- Comparing different memory management strategies.

---

## 18. Conclusion

This project provides a practical exploration of how memory management and garbage collection work at a lower level using the C programming language.

By implementing Reference Counting and Mark-and-Sweep independently, we can understand two fundamentally different approaches to managing dynamically allocated memory.

**Reference Counting** demonstrates how object lifetimes can be controlled through explicit reference tracking. It provides immediate reclamation when the reference count reaches zero, but introduces additional bookkeeping and struggles with circular references.

**Mark-and-Sweep** demonstrates how a virtual machine can maintain an object registry, identify root references through stack frames, traverse complex object relationships, and reclaim unreachable objects through a dedicated garbage collection cycle.

The project also highlights the importance of structures, enumerations, unions, pointers, dynamic memory allocation, stack data structures, and object ownership in building a memory management system.

Ultimately, the two implementations demonstrate that garbage collection is not simply about freeing memory. It is about correctly determining which objects are still needed, understanding how objects reference one another, and reclaiming memory without destroying data that is still in use.

The main objective of this project is to understand and implement the underlying mechanisms of memory management.
