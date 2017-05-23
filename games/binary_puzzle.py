"""Tools for generating binary puzzles."""
from itertools import tee, islice
from constraint import (
    Problem,
    ExactSumConstraint,
    MaxSumConstraint,
    MinSumConstraint,
    FunctionConstraint)

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

def start():
    """Generate binary puzzle solution."""
    problem = Problem()
    dimension = 6  # hardcode this for now
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
        FunctionConstraint(_test_uniqueness), range(dimension**2))
    result = problem.getSolution()
    if result is not None:
        for i in range(dimension):
            row_positions = range(
                i * dimension,
                (i + 1) * dimension)
            print(" ".join([str(result[position]) for position in row_positions]))
