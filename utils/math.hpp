#pragma once

#include <iostream>
#include <cmath>

// Forward declaration
template <typename T>
class Vec2;

// Type definitions for common vector types
using Vec2i = Vec2<int>;
using Vec2f = Vec2<float>;
using Vec2d = Vec2<double>;

/**
 * Math helper functions
 */

// given 2 points A and B, returns the normal vector perpendicular to the segment AB
template <typename T>
Vec2<T> normal2Segment(const Vec2<T>& A, const Vec2<T>& B) {
    Vec2<T> delta = B - A;
    T length = delta.length();
    if (length == 0) {
        return Vec2<T>(0, 0);
    }
    return Vec2<T>(delta.y / length, -delta.x / length);
}

/**
 * @brief 2D vector template class
 * 
 * Implements a 2D vector with basic arithmetic operations
 * including scalar operations, dot product and cross product.
 */
template <typename T>
class Vec2 {
public:
    T x, y;

    // Constructors
    Vec2() : x(0), y(0) {}
    Vec2(T x, T y) : x(x), y(y) {}
    Vec2(const Vec2<T>& v) : x(v.x), y(v.y) {}

    // Type conversion constructor
    template <typename U>
    explicit Vec2(const Vec2<U>& v) : x(static_cast<T>(v.x)), y(static_cast<T>(v.y)) {}

    // Assignment operator
    Vec2<T>& operator=(const Vec2<T>& v) {
        if (this != &v) {
            x = v.x;
            y = v.y;
        }
        return *this;
    }

    // Vector addition
    Vec2<T> operator+(const Vec2<T>& v) const {
        return Vec2<T>(x + v.x, y + v.y);
    }

    Vec2<T>& operator+=(const Vec2<T>& v) {
        x += v.x;
        y += v.y;
        return *this;
    }

    // Vector subtraction
    Vec2<T> operator-(const Vec2<T>& v) const {
        return Vec2<T>(x - v.x, y - v.y);
    }

    Vec2<T>& operator-=(const Vec2<T>& v) {
        x -= v.x;
        y -= v.y;
        return *this;
    }

    // Scalar multiplication
    Vec2<T> operator*(T scalar) const {
        return Vec2<T>(x * scalar, y * scalar);
    }

    Vec2<T>& operator*=(T scalar) {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    // Scalar division
    Vec2<T> operator/(T scalar) const {
        // Check for division by zero
        if (scalar == 0) {
            throw std::invalid_argument("Division by zero");
        }
        return Vec2<T>(x / scalar, y / scalar);
    }

    Vec2<T>& operator/=(T scalar) {
        if (scalar == 0) {
            throw std::invalid_argument("Division by zero");
        }
        x /= scalar;
        y /= scalar;
        return *this;
    }

    // Cross product (2D cross product returns a scalar)
    T cross(const Vec2<T>& v) const {
        return x * v.y - y * v.x;
    }

    // Dot product
    T dot(const Vec2<T>& v) const {
        return x * v.x + y * v.y;
    }

    // Multiplication operator for cross product
    T operator*(const Vec2<T>& v) const {
        return cross(v);
    }

    // Length/magnitude of the vector
    T length() const {
        return static_cast<T>(std::sqrt(x * x + y * y));
    }

    // Squared length (avoids square root for performance)
    T lengthSquared() const {
        return x * x + y * y;
    }

    // Normalize the vector (make it unit length)
    Vec2<T> normalized() const {
        T len = length();
        if (len == 0) {
            return *this;
        }
        return *this / len;
    }

    // Normalize this vector in place
    void normalize() {
        T len = length();
        if (len != 0) {
            *this /= len;
        }
    }

    // Check if vectors are equal
    bool operator==(const Vec2<T>& v) const {
        return (x == v.x && y == v.y);
    }

    bool operator!=(const Vec2<T>& v) const {
        return !(*this == v);
    }

    // String representation for debugging
    friend std::ostream& operator<<(std::ostream& os, const Vec2<T>& v) {
        os << "Vec2(" << v.x << ", " << v.y << ")";
        return os;
    }
};

// Allow scalar * vector (in that order)
template <typename T, typename U>
Vec2<T> operator*(U scalar, const Vec2<T>& v) {
    return Vec2<T>(v.x * scalar, v.y * scalar);
}