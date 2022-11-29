#pragma once
#include <complex>
#include <vector>

using base = std::complex<long double>;

bool is_pow_2(size_t x);

void fft(std::vector<base> &x, bool invert);
