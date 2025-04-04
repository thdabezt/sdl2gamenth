#pragma once
#include <iostream> // For std::ostream
#include <cmath>    // For std::sqrt

class Vector2D {
public:
    float x;
    float y;

    Vector2D();
    Vector2D(float x, float y);

    // Mutator Methods (Modify 'this' object)
    Vector2D& Add(const Vector2D& vec);
    Vector2D& Subtract(const Vector2D& vec);
    Vector2D& Multiply(const Vector2D& vec); // Element-wise
    Vector2D& Divide(const Vector2D& vec);   // Element-wise
    Vector2D& Normalize();
    Vector2D& Zero();

    // Compound Assignment Operators (Modify 'this' object)
    Vector2D& operator+=(const Vector2D& vec);
    Vector2D& operator-=(const Vector2D& vec);
    Vector2D& operator*=(const Vector2D& vec); // Element-wise
    Vector2D& operator/=(const Vector2D& vec); // Element-wise

    // Static Distance methods
    static float Distance(const Vector2D& a, const Vector2D& b);
    static float DistanceSq(const Vector2D& a, const Vector2D& b); // Squared Distance

    // Stream output operator
    friend std::ostream& operator<<(std::ostream& stream, const Vector2D& vec);
};

// --- Non-Member Free Functions ---

// Binary Arithmetic Operators (Return new Vector2D)
Vector2D operator+(const Vector2D& v1, const Vector2D& v2);
Vector2D operator-(const Vector2D& v1, const Vector2D& v2);
Vector2D operator*(const Vector2D& v1, const Vector2D& v2); // Element-wise
Vector2D operator/(const Vector2D& v1, const Vector2D& v2); // Element-wise

// Scalar Multiplication (Return new Vector2D)
Vector2D operator*(const Vector2D& vec, float scalar);
Vector2D operator*(float scalar, const Vector2D& vec);