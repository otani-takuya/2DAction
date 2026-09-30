#pragma once

//========================================
// Vector2
//========================================
struct Vector2 {
	float x;
	float y;
};

//========================================
// Vector3
//========================================
struct Vector3 {
	float x;
	float y;
	float z;

	Vector3 operator+(const Vector3& obj) const {
		return {
			x + obj.x,
			y + obj.y,
			z + obj.z
		};
	}

	Vector3 operator+=(const Vector3& obj) {
		x += obj.x;
		y += obj.y;
		z += obj.z;
		return *this;
	}

    Vector3 operator*(float scalar) const { 
		return {
			x * scalar, 
			y * scalar,
			z * scalar
		}; 
	}

	Vector3 operator/(float scalar) const {
		return {
			x / scalar,
			y / scalar, 
			z / scalar
		}; 
	}
};

//========================================
// Vector4
//========================================
struct Vector4 {
	float x;
	float y;
	float z;
	float w;
};