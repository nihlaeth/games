"""Tools for generating binary puzzles."""
from itertools import tee, islice
from functools import partial
from constraint import (
    Problem,
    ExactSumConstraint,
    MaxSumConstraint,
    MinSumConstraint,
    FunctionConstraint,
    BacktrackingSolver,
    RecursiveBacktrackingSolver,
    MinConflictsSolver)

def _nwise(iterable, n=2):
    iters = tee(iterable, n)
    for i, it in enumerate(iters):
        next(islice(it, i, i), None)
    return zip(*iters)

def _test_uniqueness(*values, dimension=6):
    rows = set()
    columns = set()
    for i in range(dimension):
        row_positions = range(
            i * dimension,
            (i + 1) * dimension)
        column_positions = range(i, dimension**2, dimension)
        rows.add("".join([
            str(values[position]) for position in row_positions]))
        columns.add("".join([
            str(values[position]) for position in column_positions]))
    if len(rows) < dimension or len(columns) < dimension:
        return False
    return True

def solution(dimension, solver=None):
    """Generate binary puzzle solution."""
    problem = Problem()
    if solver is not None:
        problem.setSolver(solver)
    else:
        problem.setSolver(BacktrackingSolver())
    problem.addVariables(range(dimension**2), [0, 1])
    for i in range(dimension):
        row_positions = range(
            i * dimension,
            (i + 1) * dimension)
        column_positions = range(i, dimension**2, dimension)
        # same number of zeros and ones in each row
        problem.addConstraint(
            ExactSumConstraint(dimension / 2), row_positions)
        problem.addConstraint(
            ExactSumConstraint(dimension / 2), column_positions)
        # maximum two of the same next to each other
        for triplet in _nwise(row_positions, 3):
            problem.addConstraint(
                MaxSumConstraint(2), triplet)
            problem.addConstraint(
                MinSumConstraint(1), triplet)
        for triplet in _nwise(column_positions, 3):
            problem.addConstraint(
                MaxSumConstraint(2), triplet)
            problem.addConstraint(
                MinSumConstraint(1), triplet)
    # unique rows and columns
    problem.addConstraint(
        FunctionConstraint(
            partial(_test_uniqueness, dimension=dimension)),
        range(dimension**2))
    if isinstance(solver, RecursiveBacktrackingSolver):
        return problem.getSolutions()
    if isinstance(solver, MinConflictsSolver):
        return (problem.getSolution(),)
    return problem.getSolutionIter()

def start():
    """Handle command line interface."""
    dimension = 18  # hardcode this for now
    result = solution(dimension)
    for item in result:
        for i in range(dimension):
            row_positions = range(
                i * dimension,
                (i + 1) * dimension)
            print(" ".join([str(item[position]) for position in row_positions]))
        print()
