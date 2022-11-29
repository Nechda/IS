#include "fft.hpp"
#include <omp.h>

static std::vector<base> w_;

namespace {

void fft_prep(base *x, int n, bool invert) {
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        for (; j >= bit; bit >>= 1)
            j -= bit;
        j += bit;
        if (i < j) {
            std::swap(x[i], x[j]);
        }
    }

    if (w_.size() == static_cast<size_t>(n))
        return;

    w_.resize(n);

    constexpr long double PI = 3.14159265358979323846264338327950288419716939937510L;
    auto ang = 2.0 * PI / n * (invert ? -1 : 1);
    for (int j = 0; j < n; j++) {
        w_[j] = base(std::cos(ang * j), std::sin(ang * j));
    }
}

template <int len> void fft_small_fast(base __restrict *x, int n) {
    if (len > n)
        return;

    constexpr auto len_div_2 = len / 2;

#pragma omp parallel for shared(x, w_)
    for (int i = 0; i < n; i += len) {
#pragma GCC unroll(1)
        for (int j = 0; j < len_div_2; j++) {
            size_t factor = n / len;
            auto w = w_[j * factor];
            auto u = x[i + j];
            auto v = x[i + j + len_div_2];
            x[i + j] = u + v * w;
            x[i + j + len_div_2] = u - v * w;
        }
    }
}

void fft_imp(base __restrict *x, int n, bool invert) {
    fft_small_fast<2>(x, n);
    fft_small_fast<4>(x, n);
    fft_small_fast<8>(x, n);

    for (int len = 16; len <= n; len <<= 1) {
#pragma omp parallel for shared(x, w_)
        for (int i = 0; i < n; i += len) {
            for (int j = 0; j < len / 2; j++) {
                size_t factor = n / len;
                auto w = w_[j * factor];
                auto u = x[i + j];
                auto v = x[i + j + len / 2];
                x[i + j] = u + v * w;
                x[i + j + len / 2] = u - v * w;
            }
        }
    }

    double normalize = invert ? 1.0 / n : 1.0;
#pragma omp parallel for shared(x)
    for (int i = 0; i < n; i++) {
        x[i] *= normalize;
    }
}
} // namespace

bool is_pow_2(size_t x) {
    x = x & (x - 1);
    return x == 0;
}

void fft(std::vector<base> &x, bool invert) {
    if (!is_pow_2(x.size())) {
        std::abort(); // the size must be a power of two
    }

    fft_prep(x.data(), x.size(), invert);
    fft_imp(x.data(), x.size(), invert);
}