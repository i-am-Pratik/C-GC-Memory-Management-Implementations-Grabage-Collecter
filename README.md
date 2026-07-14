# Garbage Collector in C

This project implements a simple Virtual Machine (VM) in C that features two custom garbage collection algorithms:
1. **Reference Counting**: Tracks how many references point to an object. When the count drops to zero, the object is immediately freed.
2. **Mark-and-Sweep**: A tracing garbage collector that handles circular references by traversing all reachable objects from the VM's stack (roots) and freeing everything else.

## Directory Structure
- `src/vm.h`: Core data structures (`Object`, `VM`, `ObjectType`).
- `src/vm.c`: VM initialization, object allocation, and stack operations.
- `src/ref_count.c`: Reference Counting implementation (`retain`, `release`).
- `src/mark_sweep.c`: Mark-and-Sweep implementation (`mark`, `sweep`, `gc`).
- `src/main.c`: Contains tests demonstrating both GC algorithms and circular reference resolution.

## How to Compile and Run

1. Make sure you have a standard C compiler like `gcc` installed.
2. Navigate to the project root directory.
3. Compile the source code:
   ```bash
   gcc src/*.c -o gc_demo
   ```
4. Run the executable:
   ```bash
   ./gc_demo
   ```

You should see console output detailing object allocations and deallocations under both GC algorithms.
