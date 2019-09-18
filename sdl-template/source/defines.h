#pragma once

#include <cstdlib>

/// Returns a random float between zero and 1
inline float rand_float() { return static_cast<float>((rand()) / (RAND_MAX + 1.0)); }

/// Returns a random float between x and y
inline float rand_in_range(float x, float y) { return x + rand_float()*(y - x); }

/// Returns a random int between from and to
inline int rand_in_range(int from, int to) { return from + rand() % (to - from); }

/// Modulo that works with negative number as well
template <class T>
T modulo(T x, T m) { return (x % m + m) % m; }
