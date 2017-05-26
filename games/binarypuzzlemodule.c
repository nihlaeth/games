#include <Python.h>
#include <stdbool.h>

// Macro for reading individual bits
// courtesy of https://stackoverflow.com/a/5376604
#define GET_BIT(p, n) ((((uint8_t *)p)[n/8] >> (n%8)) & 0x01)
// this only works for setting bits to 1
#define SET_BIT(p, n, b) ((uint8_t *)p)[n/8] |= b >> (n%8)

// Lookup table for fast calculation of bits set in 8-bit unsigned char.
// courtesy of https://stackoverflow.com/a/109915

static uint8_t oneBitsInUChar[] = {
//  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F (<- n)
//  =====================================================
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4, // 0n
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, // 1n
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, // 2n
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, // 3n
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, // 4n
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, // 5n
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, // 6n
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, // 7n
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, // 8n
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, // 9n
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, // An
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, // Bn
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, // Cn
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, // Dn
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, // En
    4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8, // Fn
};

// Function for fast calculation of bits set in 16-bit unsigned short.
uint8_t oneBitsInUShort (uint16_t x) {
    return oneBitsInUChar [x >>    8]
         + oneBitsInUChar [x &  0xff];
}

// Function for fast calculation of bits set in 32-bit unsigned int.
uint8_t oneBitsInUInt (uint32_t x) {
    return oneBitsInUShort (x >>     16)
         + oneBitsInUShort (x &  0xffff);
}

// Function for fast calculation of bits set in 64-bit unsigned int.
uint8_t oneBitsInULong (uint64_t x) {
    return oneBitsInUInt (x >>     32)
         + oneBitsInUInt (x &  0xffffffff);
}

typedef struct {
    uint8_t *solution;
    uint64_t index;
    uint8_t row, column, row_sum;
    uint8_t *column_sums;
} partial_solution_t;

static void construct_partial_solution(
        partial_solution_t *partial_solution,
        uint8_t dimension) {
    uint8_t num_ints = dimension / 8;
    if (dimension % 8 != 0)
        num_ints++;
    uint8_t *solution;
    solution = (uint8_t *)calloc(sizeof(uint8_t) * num_ints);
    if(solution == NULL) {
        fprintf(stderr, "out of memory\n");
        // TODO throw error
    };
    partial_solution->solution = solution;
    partial_solution->index = 0;
    partial_solution->row = 0;
    partial_solution->row_sum = 0;
    partial_solution->column = 0;
    partial_solution->column_sums = (uint8_t *)calloc(sizeof(uint8_t) * dimension);
};

static void deconstruct_partial_solution(
        partial_solution_t *partial_solution) {
    free(partial_solution->solution);
    partial_solution->index = 0;
    partial_solution->row = 0;
    partial_solution->row_sum = 0;
    partial_solution->column = 0;
    free(partial_solution->column_sums);
};

typedef struct {
    partial_solution_t **contents;
    int16_t top;
    uint8_t max_size;
} partial_solution_stack_t;

static bool partial_solution_stack_is_empty(
        partial_solution_stack_t *partial_solution_stack) {
      return partial_solution_stack->top < 0;
};

static bool partial_solution_stack_is_full(
        partial_solution_stack_t *partial_solution_stack) {
    return partial_solution_stack->top >= partial_solution_stack->max_size - 1;
};

static void partial_solution_stack_push(
        partial_solution_stack_t *partial_solution_stack,
        partial_solution_t *partial_solution)
{
    if (partial_solution_stack_is_full(partial_solution_stack)) {
        fprintf(stderr, "Can't push element on stack: stack is full.\n");
        // TODO throw error
    }
    /* Put information in array; update top. */
    partial_solution_stack->contents[++partial_solution_stack->top] = partial_solution;
};

static partial_solution_t * partial_solution_stack_pop(
        partial_solution_stack_t *partial_solution_stack) {
    if (partial_solution_stack_is_empty(partial_solution_stack)) {
        fprintf(stderr, "Can't pop element from stack: stack is empty.\n");
        // TODO throw error
    }
    return partial_solution_stack->contents[partial_solution_stack->top--];
};

static void construct_partial_solution_stack(
        partial_solution_stack_t *partial_solution_stack,
        uint8_t dimension) {
    partial_solution_t **contents;
    contents = (partial_solution_t *)calloc(
            sizeof(partial_solution_t *) * (dimension + 1));
    if(contents == NULL) {
        fprintf(stderr, "out of memory\n");
        // TODO throw error
    };
    partial_solution_stack->contents = contents;
    partial_solution_stack->top = -1;
    partial_solution_stack->max_size = dimension + 1;
};

static void deconstruct_partial_solution_stack(
        partial_solution_stack_t *partial_solution_stack) {
    while(!partial_solution_stack_is_empty(partial_solution_stack))
        free(partial_solution_stack_pop(partial_solution_stack));
    free(partial_solution_stack->contents);
    partial_solution_stack->top = -1;
    partial_solution_stack->max_size = 0;
};

static PyObject *
binary_puzzle_solutions(PyObject *self, PyObject *args) {
    uint8_t dimension;
    if (!PyArg_ParseTuple(args, "b", &dimension))
        return NULL;
    Py_INCREF(Py_None);
    return Py_None;
};

static PyMethodDef BinaryPuzzleMethods[] = {
    {"solutions",  binary_puzzle_solutions, METH_VARARGS,
     "Return all possible valid binary puzzle solutions for a given dimension."},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

PyDoc_STRVAR(
    binary_puzzle_doc,
    "Tools for generating binary puzzles.");

static struct PyModuleDef binarypuzzlemodule = {
    PyModuleDef_HEAD_INIT,
    "binary_puzzle",   /* name of module */
    binary_puzzle_doc, /* module documentation, may be NULL */
    -1,       /* size of per-interpreter state of the module,
                 or -1 if the module keeps state in global variables. */
    BinaryPuzzleMethods
};

PyMODINIT_FUNC
PyInit_binary_puzzle(void)
{
        return PyModule_Create(&binarypuzzlemodule);
};
