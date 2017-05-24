"""Tests for binary puzzle tools."""
from time import perf_counter
import pytest
from games.binary_puzzle_tools import (
    solutions, _test_array_validity, _test_columns_validity)

def test_array_validity():
    assert _test_array_validity([0, 0, 1, 1])
    assert _test_array_validity([0, 1, 0, 1])
    assert not _test_array_validity([0, 1, 1, 1])
    assert not _test_array_validity([1, 0, 0, 0, 1, 1])

def test_columns_validity():
    # valid solution
    assert _test_columns_validity((0, 1, 1, 0))
    assert _test_columns_validity((
        1, 0, 0, 1,
        0, 1, 1, 0,
        0, 1, 0, 1,
        1, 0, 1, 0))
    # invalid columns
    assert not _test_columns_validity((
        1, 0, 0, 1,
        0, 1, 1, 0,
        0, 1, 0, 1,
        1, 0, 0, 1))
    # duplicate columns
    assert not _test_columns_validity((
        1, 0, 0, 1,
        0, 1, 1, 0,
        0, 1, 1, 0,
        1, 0, 0, 1))

def _delta_time(function):
    start = perf_counter()
    function()
    end = perf_counter()
    return end - start

def _performance():
    dimension_six = _delta_time(
        lambda: list(solutions(dimension=6)))
    print(f"dimension=6, all solutions: {dimension_six}")
    # dimension_thirty_six = _delta_time(
    #     lambda: next(solution(dimension=16)))
    # print(f"dimension=36, one solution: {dimension_thirty_six}")

if __name__ == "__main__":
    _performance()
