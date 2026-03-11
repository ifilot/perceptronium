/*******************************************************************************
 *  
 *  Copyright (C) 2024 Ivo Filot <ivo@ivofilot.nl>
 *  
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 * 
 ******************************************************************************/

#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <vector>
#include <ostream>

// Preprocessor directive to switch between float and double
// If -DFLOATY=1 is used, 32-bit floats are used instead of 64-bit doubles
#ifdef FLOATY
    typedef float floaty;
    inline floaty CFLOATY_min(const floaty& thisfloaty) {
        return std::min(1.0f, thisfloaty);
    }
    inline floaty CFLOATY_max(const floaty& thisfloaty) {
        return std::max(1.0f, thisfloaty);
    }
#else
    typedef double floaty;
    inline floaty CFLOATY_min(const floaty& thisfloaty) {
        return std::min(1.0, thisfloaty);
    }
    inline floaty CFLOATY_max(const floaty& thisfloaty) {
        return std::max(1.0, thisfloaty);
    }
#endif

struct Vector3f {
    floaty x = 0.0;
    floaty y = 0.0;
    floaty z = 0.0;

    floaty norm() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    floaty dot(const Vector3f& other) const {
        return x * other.x + y * other.y + z * other.z;
    }

    floaty operator[](std::size_t idx) const {
        if (idx == 0) return x;
        if (idx == 1) return y;
        if (idx == 2) return z;
        throw std::out_of_range("Vector3f index out of range");
    }
};

inline Vector3f operator+(const Vector3f& lhs, const Vector3f& rhs) {
    return {lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z};
}

inline Vector3f operator-(const Vector3f& lhs, const Vector3f& rhs) {
    return {lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z};
}

inline Vector3f operator*(const Vector3f& v, floaty scalar) {
    return {v.x * scalar, v.y * scalar, v.z * scalar};
}

inline Vector3f operator*(floaty scalar, const Vector3f& v) {
    return v * scalar;
}

class SimpleMatrix {
public:
    SimpleMatrix() = default;
    SimpleMatrix(std::size_t rows, std::size_t cols)
        : rows_(rows), cols_(cols), data_(rows * cols, static_cast<floaty>(0.0)) {}

    static SimpleMatrix Zero(std::size_t rows, std::size_t cols) {
        return SimpleMatrix(rows, cols);
    }

    static SimpleMatrix Identity(std::size_t rows, std::size_t cols) {
        SimpleMatrix matrix(rows, cols);
        const std::size_t diagonal = std::min(rows, cols);
        for (std::size_t idx = 0; idx < diagonal; idx++) {
            matrix(idx, idx) = static_cast<floaty>(1.0);
        }
        return matrix;
    }

    std::size_t rows() const { return rows_; }
    std::size_t cols() const { return cols_; }

    floaty& operator()(std::size_t row, std::size_t col) {
        return data_.at(row * cols_ + col);
    }

    const floaty& operator()(std::size_t row, std::size_t col) const {
        return data_.at(row * cols_ + col);
    }

    Vector3f get_row3(std::size_t row) const {
        if (cols_ < 3) {
            throw std::runtime_error("Matrix does not have 3 columns");
        }
        return {(*this)(row, 0), (*this)(row, 1), (*this)(row, 2)};
    }

    void set_row3(std::size_t row, const Vector3f& value) {
        if (cols_ < 3) {
            throw std::runtime_error("Matrix does not have 3 columns");
        }
        (*this)(row, 0) = value.x;
        (*this)(row, 1) = value.y;
        (*this)(row, 2) = value.z;
    }

    void swap(SimpleMatrix& other) {
        std::swap(rows_, other.rows_);
        std::swap(cols_, other.cols_);
        data_.swap(other.data_);
    }

    bool operator==(const SimpleMatrix& other) const {
        return rows_ == other.rows_ && cols_ == other.cols_ && data_ == other.data_;
    }

private:
    std::size_t rows_ = 0;
    std::size_t cols_ = 0;
    std::vector<floaty> data_;
};

inline std::ostream& operator<<(std::ostream& os, const SimpleMatrix& matrix) {
    os << "[";
    for (std::size_t row = 0; row < matrix.rows(); row++) {
        if (row > 0) {
            os << "; ";
        }
        os << "[";
        for (std::size_t col = 0; col < matrix.cols(); col++) {
            if (col > 0) {
                os << ", ";
            }
            os << matrix(row, col);
        }
        os << "]";
    }
    os << "]";
    return os;
}

inline SimpleMatrix operator*(floaty scalar, const SimpleMatrix& matrix) {
    SimpleMatrix result = matrix;
    for (std::size_t row = 0; row < result.rows(); row++) {
        for (std::size_t col = 0; col < result.cols(); col++) {
            result(row, col) *= scalar;
        }
    }
    return result;
}

using DenseVector3 = Vector3f;
using DenseMatrix = SimpleMatrix;

// Backward-compatible aliases retained to limit refactor churn.
typedef SimpleMatrix EigenMatrixXff;
typedef SimpleMatrix EigenMatrixX3f;
typedef Vector3f EigenRowVector3ff;
typedef SimpleMatrix EigenMatrix3ff;

/**
 * @brief Fast approximation of power function std::pow(a, b).
 *
 * This implementation directly manipulates the IEEE754 representation of 
 * floating point numbers for increased speed at the cost of accuracy.
 * 
 * Reference: N.N. Schraudolph (1998), DOI: 10.1162/089976699300016467.
 *
 * @note Conditions:
 *  - a > 0: If a == 0, byte overflow can occur. If a < 0 and b is non-integer, the result would be imaginary.
 *  - b > 0: Otherwise, bit shifts fail. For b < 0, use (1 / fastPow()) instead.
 *  - b == 0: Always works.
 */
#ifdef FLOATY
    inline floaty fastPow(const floaty a, const floaty b) {
        union {
            floaty d;  // Align memory blocks
            int x;
        } u = { a };

        // 1065353216 - 486411 = 1064866805 to minimize RMSE (staircase correction)
        u.x = (int)(b * (u.x - 1064866805) + 1064866805);
        return u.d;
    }
#else
    inline floaty fastPow(const floaty a, const floaty b) {
        union {
            floaty d;
            int x[2]; // Using array instead of struct to avoid endianness checks
        } u = { a };

        #ifdef ENDIAN
            u.x[1] = (int)(b * (u.x[1] - 1072632447) + 1072632447);
            u.x[0] = 0;
        #else
            u.x[0] = (int)(b * (u.x[0] - 1072632447) + 1072632447);
            u.x[1] = 0;
        #endif

        return u.d;
    }
#endif

/**
 * @brief Defines the power function for use in the program.
 *
 * Implements a fallback for cases where the base value is close to zero.
 */
#ifdef POWGLOB
    inline floaty powglob(const floaty a, const floaty b) {
        static floaty x;
        if (a > 1e-7) {
            x = fastPow(a, b);
            if (x < -1e-7) {
                return 0.0;  // Avoid overflow when a is too small
            }
            #ifdef FLOATY
                else if (x != x) {  // Check for NaN
                    return std::pow(a, b);  // Floats may underflow into sNaN
                }
            #endif
            return fastPow(a, b);
        } 
        return 0.0;
    }
#else
    inline floaty powglob(const floaty a, const floaty b) {
        return std::pow(a, b); // Inline achieves the same result as an alias
    }
#endif
