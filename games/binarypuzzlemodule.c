#include <Python.h>
#include <stdbool.h>

// Macro for reading individual bits
// courtesy of https://stackoverflow.com/a/5376604
#define GET_BIT(p, n) ((((uint8_t *)p)[n/8] >> (n%8)) & 0x01)
// this only works for setting bits to 1
#define SET_BIT(p, n) ((uint8_t *)p)[n/8] |= 0x01 << (n%8)

//
// structures and their methods
//

typedef struct {
    uint8_t *solution;
    uint64_t index;
    uint8_t row, column, row_sum;
    uint8_t *column_sums;
} partial_solution_t;

static void construct_partial_solution(
        partial_solution_t *partial_solution,
        uint8_t dimension) {
    uint8_t num_ints = dimension * dimension / 8;
    if ((dimension * dimension) % 8 != 0)
        num_ints++;
    uint8_t *solution;
    solution = (uint8_t *)calloc(num_ints, sizeof(uint8_t));
    if(solution == NULL) {
        PyErr_NoMemory();
        return;
    };
    partial_solution->solution = solution;
    partial_solution->index = 0;
    partial_solution->row = 0;
    partial_solution->row_sum = 0;
    partial_solution->column = 0;
    partial_solution->column_sums = (uint8_t *)calloc(dimension, sizeof(uint8_t));
    if(partial_solution->column_sums  == NULL) {
        PyErr_NoMemory();
        return;
    };
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
    uint8_t max_size, dimension;
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
    contents = (partial_solution_t **)calloc(
            dimension + 1, sizeof(partial_solution_t *));
    if(contents == NULL) {
        PyErr_NoMemory();
        return;
    };
    partial_solution_stack->contents = contents;
    partial_solution_stack->top = -1;
    partial_solution_stack->max_size = dimension * dimension + 1;
    partial_solution_stack->dimension = dimension;
};

static void deconstruct_partial_solution_stack(
        partial_solution_stack_t *partial_solution_stack) {
    while(!partial_solution_stack_is_empty(partial_solution_stack))
        free(partial_solution_stack_pop(partial_solution_stack));
    free(partial_solution_stack->contents);
    partial_solution_stack->top = -1;
    partial_solution_stack->max_size = 0;
};

//
// where the magic happens
//

static void * cleanup_partial_solution(partial_solution_t *partial_solution) {
    deconstruct_partial_solution(partial_solution);
    free(partial_solution);
    return NULL;
};

static uint8_t * consume_partial_solution_stack(
        partial_solution_stack_t *partial_solution_stack) {
    if (partial_solution_stack_is_empty(partial_solution_stack))
        return NULL;
    uint8_t dimension = partial_solution_stack->dimension;
    partial_solution_t *partial_solution = partial_solution_stack_pop(partial_solution_stack);
    uint8_t count;
    // check triplet sum
    if (partial_solution->column > 1) {
        count =
            GET_BIT(partial_solution->solution, partial_solution->index) +
            GET_BIT(partial_solution->solution, partial_solution->index - 1) +
            GET_BIT(partial_solution->solution, partial_solution->index - 2);
        if (count < 1 || count > 2)
            return cleanup_partial_solution(partial_solution);
    };
    if (partial_solution->row > 1) {
        count =
            GET_BIT(partial_solution->solution, partial_solution->index) +
            GET_BIT(partial_solution->solution, partial_solution->index - dimension) +
            GET_BIT(partial_solution->solution, partial_solution->index - 2 * dimension);
        if (count < 1 || count > 2)
            return cleanup_partial_solution(partial_solution);
    };
    // check row and column triplet sums
    count = partial_solution->row_sum;
    if (count > dimension / 2 || partial_solution->column + 1 - count > dimension / 2)
        return cleanup_partial_solution(partial_solution);
    count = partial_solution->column_sums[partial_solution->column];
    if (count > dimension / 2 || partial_solution->row + 1 - count > dimension / 2)
        return cleanup_partial_solution(partial_solution);
    // check for uniqueness if we've just finished a row or column
    if (partial_solution->column == dimension - 1) {
        // TODO
    };
    if (partial_solution->row == dimension - 1) {
        // TODO
    };
    // check if we are at the end, if so return solution and clean up
    if (partial_solution->index == dimension * dimension - 1) {
        uint8_t *solution;
        solution = calloc(1, sizeof(partial_solution->solution));
        if(solution == NULL) {
            PyErr_NoMemory();
            return NULL;
        };
        memcpy(
                solution,
                partial_solution->solution,
                sizeof(*partial_solution->solution));
        cleanup_partial_solution(partial_solution);
        return solution;
    };
    // increment place counters
    partial_solution->index++;
    if (partial_solution->column < dimension - 1)
        partial_solution->column++;
    else {
        partial_solution->column = 0;
        partial_solution->row++;
        partial_solution->row_sum = 0;
    };
    // push partial solution back onto the stack
    partial_solution_stack_push(partial_solution_stack, partial_solution);
    // construct another partial solution with a one bit set at new position
    partial_solution_t *second_partial_solution;
    second_partial_solution = calloc(1, sizeof(partial_solution_t));
    if(second_partial_solution == NULL) {
        PyErr_NoMemory();
        return NULL;
    };
    construct_partial_solution(second_partial_solution, dimension);
    if (PyErr_Occurred() != NULL)
        return NULL;
    memcpy(
            second_partial_solution->solution,
            partial_solution->solution,
            sizeof(*partial_solution->solution));
    second_partial_solution->index = partial_solution->index;
    SET_BIT(second_partial_solution->solution, second_partial_solution->index);
    second_partial_solution->row = partial_solution->row;
    second_partial_solution->row_sum = partial_solution->row_sum + 1;
    second_partial_solution->column = partial_solution->column;
    memcpy(
            second_partial_solution->column_sums,
            partial_solution->column_sums,
            sizeof(*partial_solution->column_sums));
    second_partial_solution->column_sums[second_partial_solution->column]++;
    // push second solution onto the stack
    partial_solution_stack_push(partial_solution_stack, second_partial_solution);
    return NULL;
};

//
// Python exposed objects here
//

typedef struct {
    PyObject_HEAD
    partial_solution_stack_t *partial_solution_stack;
} solutions_state_t;

static PyObject *
binary_puzzle_solutions_new(PyTypeObject *type, PyObject *args, PyObject *kwargs) {
    uint8_t dimension;
    static char *kwlist[] = {"dimension"};
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "b", kwlist, &dimension))
        return NULL;
    if (dimension % 2 != 0 || dimension >= 64) {
        // TODO
        return NULL;
    }
    partial_solution_stack_t *partial_solution_stack;
    partial_solution_stack = calloc(1, sizeof(partial_solution_stack_t));
    if(partial_solution_stack == NULL) {
        return PyErr_NoMemory();
    };
    construct_partial_solution_stack(partial_solution_stack, dimension);
    if (PyErr_Occurred() != NULL)
        return NULL;
    // construct two partial solutions and push them onto the stack
    partial_solution_t *partial_solution;
    partial_solution = calloc(1, sizeof(partial_solution_t));
    if(partial_solution == NULL) {
        return PyErr_NoMemory();
    };
    construct_partial_solution(partial_solution, dimension);
    if (PyErr_Occurred() != NULL)
        return NULL;
    partial_solution_stack_push(partial_solution_stack, partial_solution);
    partial_solution = calloc(1, sizeof(partial_solution_t));
    if(partial_solution == NULL) {
        return PyErr_NoMemory();
    };
    construct_partial_solution(partial_solution, dimension);
    if (PyErr_Occurred() != NULL)
        return NULL;
    partial_solution->row_sum++;
    partial_solution->column_sums[0]++;
    SET_BIT(partial_solution->solution, 0);
    partial_solution_stack_push(partial_solution_stack, partial_solution);

    solutions_state_t *solutions_state = (solutions_state_t *)type->tp_alloc(type, 0);
    if (!solutions_state)
        return NULL;
    solutions_state->partial_solution_stack = partial_solution_stack;

    return (PyObject *)solutions_state;
};

static void solutions_state_dealloc(solutions_state_t *solutions_state) {
    deconstruct_partial_solution_stack(solutions_state->partial_solution_stack);
    free(solutions_state->partial_solution_stack);
    Py_TYPE(solutions_state)->tp_free(solutions_state);
};

static PyObject * binary_puzzle_solutions_next(solutions_state_t *solutions_state) {
    if (partial_solution_stack_is_empty(solutions_state->partial_solution_stack))
        return NULL;
    uint8_t *solution = consume_partial_solution_stack(solutions_state->partial_solution_stack);
    if (PyErr_Occurred() != NULL)
        return NULL;
    while (solution == NULL && !partial_solution_stack_is_empty(solutions_state->partial_solution_stack)) {
        solution = consume_partial_solution_stack(solutions_state->partial_solution_stack);
        if (PyErr_Occurred() != NULL)
            return NULL;
    };
    if (solution != NULL) {
        // convert solution to Python friendly format and return it
        uint64_t i;
        uint8_t *result = calloc(solutions_state->partial_solution_stack->dimension * solutions_state->partial_solution_stack->dimension + 1, sizeof(uint8_t));
        if(result == NULL) {
            return PyErr_NoMemory();
        };
        for(i = 0; i < solutions_state->partial_solution_stack->dimension * solutions_state->partial_solution_stack->dimension; i++){
            if (GET_BIT(solution, i))
                result[i] = 0x31; // 1
            else
                result[i] = 0x30; // 0
        };
        result[solutions_state->partial_solution_stack->dimension * solutions_state->partial_solution_stack->dimension] = 0;
        free(solution);
        PyObject * python_string = Py_BuildValue("s", result);
        free(result);
        return python_string;
    };
    return NULL;
};

PyTypeObject PySolutions_Type = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
    "solutions", /* tp_name */
    sizeof(solutions_state_t), /* tp_basicsize */
    0, /* tp_itemsize */
    (destructor)solutions_state_dealloc, /* tp_dealloc */
    0, /* tp_print */
    0, /* tp_getattr */
    0, /* tp_setattr */
    0, /* tp_reserved */
    0, /* tp_repr */
    0, /* tp_as_number */
    0, /* tp_as_sequence */
    0, /* tp_as_mapping */
    0, /* tp_hash */
    0, /* tp_call */
    0, /* tp_str */
    0, /* tp_getattro */
    0, /* tp_setattro */
    0, /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT, /* tp_flags */
    0, /* tp_doc */
    0, /* tp_traverse */
    0, /* tp_clear */
    0, /* tp_richcompare */
    0, /* tp_weaklistoffset */
    PyObject_SelfIter, /* tp_iter */
    (iternextfunc)binary_puzzle_solutions_next, /* tp_iternext */
    0, /* tp_methods */
    0, /* tp_members */
    0, /* tp_getset */
    0, /* tp_base */
    0, /* tp_dict */
    0, /* tp_descr_get */
    0, /* tp_descr_set */
    0, /* tp_dictoffset */
    0, /* tp_init */
    PyType_GenericAlloc, /* tp_alloc */
    binary_puzzle_solutions_new, /* tp_new */
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
};

PyMODINIT_FUNC PyInit_binary_puzzle(void) {
    PyObject *module = PyModule_Create(&binarypuzzlemodule);
    if (!module)
        return NULL;
    if (PyType_Ready(&PySolutions_Type) < 0)
        return NULL;
    Py_INCREF((PyObject *)&PySolutions_Type);
    PyModule_AddObject(module, "solutions", (PyObject *)&PySolutions_Type);
    return module;
};
