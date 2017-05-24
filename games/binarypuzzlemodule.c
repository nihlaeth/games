#include <Python.h>
#include <stdbool.h>

// Macro for reading individual bits
// courtesy of https://stackoverflow.com/a/5376604
#define GET_BIT(p, n) ((((uint8_t *)p)[n/8] >> (n%8)) & 0x01)

// Lookup table for fast calculation of bits set in 8-bit unsigned char.
// courtesy of https://stackoverflow.com/a/109915

static uint8_t oneBitsInUChar[] = {
//  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F (<- n)
//  =====================================================
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8,
};

// Function for fast calculation of bits set in 16-bit unsigned short.
uint8_t oneBitsInUShort (unsigned short x) {
    return oneBitsInUChar [x >>    8]
         + oneBitsInUChar [x &  0xff];
}

// Function for fast calculation of bits set in 32-bit unsigned int.
uint8_t oneBitsInUInt (unsigned int x) {
    return oneBitsInUShort (x >>     16)
         + oneBitsInUShort (x &  0xffff);
}

static bool binary_puzzle_is_array_valid(uint8_t *values, uint8_t dimension) {
    // make sure there is the same number of zeros and ones
    int counter = 0;
    int i;
    for(i = 0; i < dimension / 8; i++)
        counter += oneBitsInUChar[values[i]];
    if (counter != dimension /2)
        return false;
    // maximum to the same next to each other
    for(i = 0; i < dimension - 2; i++){
        counter =
            GET_BIT(values, i) + GET_BIT(values, i + 1) + GET_BIT(values, i + 2);
        if (counter < 1 || counter > 2)
            return false;
    };
    return true;
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
