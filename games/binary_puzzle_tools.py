"""Tools for generating binary puzzles."""
from itertools import tee, islice, permutations, product

def _nwise(iterable, n=2):
    iters = tee(iterable, n)
    for i, it in enumerate(iters):
        next(islice(it, i, i), None)
    return zip(*iters)

def _test_array_validity(values):
    # same number of zeros and ones in each column
    if values.count(0) != len(values) / 2:
        return False
    # maximum two of the same next to each other
    for triplet in _nwise(values, 3):
        total = sum(triplet)
        if total < 1 or total > 2:
            return False
    return True

def _test_columns_validity(data):
    columns = []
    for i in range(len(data)):
        column = tuple(row[i] for row in data)
        if not _test_array_validity(column):
            return False
        columns.append(column)
    # test column uniqueness
    if len(set(columns)) != len(data):
        return False
    return True

def _get_possible_rows(dimension):
    row_content = [0] * (dimension // 2) + [1] * (dimension // 2)
    possible_rows = set(permutations(row_content, dimension))
    return tuple(row for row in possible_rows if _test_array_validity(row))

def solutions(dimension):
    """Generate binary puzzle solutions."""
    assert dimension % 2 == 0

    possible_rows = _get_possible_rows(dimension)
    for indices in permutations(range(len(possible_rows)), dimension):
        possible_solution = tuple(possible_rows[i] for i in indices)
        if _test_columns_validity(possible_solution):
            yield possible_solution

def start():
    """Handle command line interface."""
    dimension = 6  # hardcode this for now
    result = solutions(dimension)
    for item in result:
        for row in item:
            print(" ".join([str(cell) for cell in row]))
        print()

if __name__ == "__main__":
    import cProfile
    cProfile.run('start()')
