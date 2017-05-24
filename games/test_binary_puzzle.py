"""Tests for binary puzzle tools."""
from time import perf_counter
from constraint import BacktrackingSolver, RecursiveBacktrackingSolver
from games.binary_puzzle import solution

def _delta_time(function):
    start = perf_counter()
    function()
    end = perf_counter()
    return end - start

def _performance():
    backtracking_solver = _delta_time(
        lambda: list(solution(dimension=6, solver=BacktrackingSolver())))
    print(f"dimension=6, BacktrackingSolver: {backtracking_solver}")
    recursive_backtracking_solver = _delta_time(
        lambda: solution(
            dimension=6, solver=RecursiveBacktrackingSolver()))
    print(f"dimension=6, RecursiveBacktrackingSolver: {recursive_backtracking_solver}")
    # the minimum conflict solver only returns a single solution so it
    # really doesn't compare and it seems to be broken in Python 3.6
    # min_conflicts_solver = _delta_time(
    #     lambda: solution(dimension=6, solver=MinConflictsSolver()))
    # print(f"dimension=6, MinConflictsSolver: {min_conflicts_solver}")

if __name__ == "__main__":
    _performance()
