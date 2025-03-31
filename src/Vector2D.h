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

    // NOTE: Removed the incorrect member operator*(int)
    static float Distance(const Vector2D& a, const Vector2D& b); // Keep existing Distance
    static float DistanceSq(const Vector2D& a, const Vector2D& b); // Add Squared Distance
    
    // Declare stream output operator as a friend if needed for private members (not needed here)
    friend std::ostream& operator<<(std::ostream& stream, const Vector2D& vec);
};

// --- Declarations for NON-MEMBER Free Functions ---

// Corrected Binary Arithmetic Operators (Return new Vector2D)
Vector2D operator+(const Vector2D& v1, const Vector2D& v2);
Vector2D operator-(const Vector2D& v1, const Vector2D& v2); // Takes const& now
Vector2D operator*(const Vector2D& v1, const Vector2D& v2); // Element-wise
Vector2D operator/(const Vector2D& v1, const Vector2D& v2); // Element-wise

// Corrected Scalar Multiplication (Return new Vector2D)
Vector2D operator*(const Vector2D& vec, float scalar);
Vector2D operator*(float scalar, const Vector2D& vec);