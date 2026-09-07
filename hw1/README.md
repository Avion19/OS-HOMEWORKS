## Info
Name: Avien Ramirez  
Date: 9/7/26  
Class: CS455-2  
Assignment: hw1-2

## Intro

This project implements and tests a double-ended queue (deque) in C. The deque supports adding, removing, searching, indexing, mapping, deleting, and converting its contents to a string.

## Description
The main files are:

- `deq.c`: deque implementation
- `deq.h`: public deque interface
- `main.c`: test suite
- `GNUmakefile`: build, run, and Valgrind targets

## How to Run

Build the program:

```sh
make
```

Run the test suite:

```sh
make run
```

Run the test suite with Valgrind:

```sh
make valgrind
```

Remove generated build files:

```sh
make clean
```

## Discussion
Implementing this was a fun wake-up call about how to code in C again. It had been a few months since I had programmed in pure C, so I was a little worried about whether I could get back into it seamlessly, but everything worked out. I find linked-list problems fun to solve because they are easy to visualize, so I enjoyed working on this assignment. It was especially interesting to realize that the head and tail endpoints served both as the beginning and end of the list and as traversal links, with the next and previous links of a node represented by the endpoints as well.

When I started implementing the functions, I began with put because inserting at the beginning or end of a linked list was easy to visualize, including its edge cases. I then moved on to get, since I realized I could use it when implementing rem if the matching element was located at either endpoint. get and put are similar in terms of edge cases and pointer logic, so implementing them was straightforward.

Thinking symmetrically was not something I was able to grasp easily at first. I found it easier to handle the head and tail cases explicitly. It was not until I thought about node traversal while working on rem that I realized we travel toward the opposite end from the one provided. This allowed me to use a ternary operator to determine the direction of traversal. Once I had that ternary operator, I realized that most of my code could be simplified to work with the selected endpoint and its calculated opposite instead of handling the head and tail cases separately.

Testing this assignment was not very fun because I was rusty at properly handling memory. While writing tests, I was getting the expected behavior from the doubly-linked list, but Valgrind was still reporting issues with variables created within the tests themselves. This reminded me how explicit the C language is and how everything used by a program takes up memory. Other than that, I used assert to ensure that within the test, the expected output or behavior was met so that we know a test is passing or not.


## Testing

The test suite in `main.c` covers:

- Empty deque operations
- Insertion at the head and tail
- Indexed access from both ends
- Removal from the head and tail
- Removal of matching data pointers
- Duplicate data pointers
- String conversion
- Single-operation edge cases
- Combined operations
- Queue contents after mutating operations

## Results

The complete test suite passes successfully:
```text
PASS: test_empty
PASS: test_null_behavior
PASS: test_put_and_ith
PASS: test_get
PASS: test_rem
PASS: test_duplicate_pointer_rem
PASS: test_string_after_operations
PASS: test_map
PASS: test_head_put_only
PASS: test_tail_put_only
PASS: test_head_get_only
PASS: test_tail_get_only
PASS: test_head_ith_only
PASS: test_tail_ith_only
PASS: test_head_rem_only
PASS: test_tail_rem_only
PASS: test_operations_in_tandem
All deque tests passed.
```

The program builds with `-Wall` without compiler warnings. Valgrind can be run with `make valgrind` to check for memory leaks and invalid memory operations.

Valgrind output:
```text
==3237071== HEAP SUMMARY:
==3237071==     in use at exit: 0 bytes in 0 blocks
==3237071==   total heap usage: 105 allocs, 105 frees, 2,935 bytes allocated
==3237071==
==3237071== All heap blocks were freed -- no leaks are possible
==3237071==
==3237071== For lists of detected and suppressed errors, rerun with: -s
==3237071== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)
```
