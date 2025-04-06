#pragma once
#include <iostream> 
#include <cmath>    

class Vector2D {
public:
    float x;
    float y;

    Vector2D();
    Vector2D(float x, float y);

    Vector2D& Add(const Vector2D& vec);
    Vector2D& Subtract(const Vector2D& vec);
    Vector2D& Multiply(const Vector2D& vec); 
    Vector2D& Divide(const Vector2D& vec);   
    Vector2D& Normalize();
    Vector2D& Zero();

    Vector2D& operator+=(const Vector2D& vec);
    Vector2D& operator-=(const Vector2D& vec);
    Vector2D& operator*=(const Vector2D& vec); 
    Vector2D& operator/=(const Vector2D& vec); 

    static float Distance(const Vector2D& a, const Vector2D& b);
    static float DistanceSq(const Vector2D& a, const Vector2D& b); 

    friend std::ostream& operator<<(std::ostream& stream, const Vector2D& vec);
};

Vector2D operator+(const Vector2D& v1, const Vector2D& v2);
Vector2D operator-(const Vector2D& v1, const Vector2D& v2);
Vector2D operator*(const Vector2D& v1, const Vector2D& v2); 
Vector2D operator/(const Vector2D& v1, const Vector2D& v2); 

Vector2D operator*(const Vector2D& vec, float scalar);
Vector2D operator*(float scalar, const Vector2D& vec);