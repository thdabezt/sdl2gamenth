#include "Vector2D.h"
#include <cmath>
#include <iostream> 

Vector2D::Vector2D() : x(0.0f), y(0.0f) {}

Vector2D::Vector2D(float x, float y) : x(x), y(y) {}

Vector2D& Vector2D::Add(const Vector2D& vec) {
    this->x += vec.x;
    this->y += vec.y;
    return *this;
}

Vector2D& Vector2D::Subtract(const Vector2D& vec) {
    this->x -= vec.x;
    this->y -= vec.y;
    return *this;
}

Vector2D& Vector2D::Multiply(const Vector2D& vec) {
    this->x *= vec.x;
    this->y *= vec.y;
    return *this;
}

Vector2D& Vector2D::Divide(const Vector2D& vec) {

    if (vec.x != 0.0f) this->x /= vec.x; else this->x = 0.0f; 
    if (vec.y != 0.0f) this->y /= vec.y; else this->y = 0.0f; 
    return *this;
}

Vector2D& Vector2D::Normalize() {
    float magnitude = std::sqrt(x * x + y * y);
    if (magnitude > 1e-6f) { 
        float invMagnitude = 1.0f / magnitude;
        x *= invMagnitude;
        y *= invMagnitude;
    } else {
        x = 0.0f; 
        y = 0.0f;
    }
    return *this;
}

Vector2D& Vector2D::Zero() {
    this->x = 0.0f;
    this->y = 0.0f;
    return *this;
}

Vector2D& Vector2D::operator+=(const Vector2D& vec) {
    return this->Add(vec);
}

Vector2D& Vector2D::operator-=(const Vector2D& vec) {
    return this->Subtract(vec);
}

Vector2D& Vector2D::operator*=(const Vector2D& vec) {
    return this->Multiply(vec);
}

Vector2D& Vector2D::operator/=(const Vector2D& vec) {
    return this->Divide(vec);
}

float Vector2D::Distance(const Vector2D& a, const Vector2D& b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return std::sqrt(dx * dx + dy * dy);
}

float Vector2D::DistanceSq(const Vector2D& a, const Vector2D& b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return dx * dx + dy * dy; 
}

Vector2D operator+(const Vector2D& v1, const Vector2D& v2) {
    return Vector2D(v1.x + v2.x, v1.y + v2.y);
}

Vector2D operator-(const Vector2D& v1, const Vector2D& v2) {
    return Vector2D(v1.x - v2.x, v1.y - v2.y);
}

Vector2D operator*(const Vector2D& v1, const Vector2D& v2) {
    return Vector2D(v1.x * v2.x, v1.y * v2.y);
}

Vector2D operator/(const Vector2D& v1, const Vector2D& v2) {

    float res_x = (v2.x != 0.0f) ? v1.x / v2.x : 0.0f; 
    float res_y = (v2.y != 0.0f) ? v1.y / v2.y : 0.0f; 
    return Vector2D(res_x, res_y);
}

Vector2D operator*(const Vector2D& vec, float scalar) {
    return Vector2D(vec.x * scalar, vec.y * scalar);
}

Vector2D operator*(float scalar, const Vector2D& vec) {
    return Vector2D(vec.x * scalar, vec.y * scalar); 
}

std::ostream& operator<<(std::ostream& stream, const Vector2D& vec) {
    stream << "(" << vec.x << "," << vec.y << ")";
    return stream;
}