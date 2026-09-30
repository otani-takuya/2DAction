#include "math.h"

using namespace KamataEngine;

// ======================================
// ワールド行列更新
// ======================================
void UpdateWorldTransform(WorldTransform& worldTransform) {

	// --- 行列生成 ---
	Matrix4x4 scale = MathUtility::MakeScaleMatrix(worldTransform.scale_);

	Matrix4x4 rotX = MathUtility::MakeRotateXMatrix(worldTransform.rotation_.x);
	Matrix4x4 rotY = MathUtility::MakeRotateYMatrix(worldTransform.rotation_.y);
	Matrix4x4 rotZ = MathUtility::MakeRotateZMatrix(worldTransform.rotation_.z);

	// 回転順序によって見た目が変わるため
	Matrix4x4 rotation = MathUtility::operator*(rotZ, MathUtility::operator*(rotX, rotY));

	Matrix4x4 translation = MathUtility::MakeTranslateMatrix(worldTransform.translation_);

	// S → R → T（基本の順序）
	worldTransform.matWorld_ = MathUtility::operator*(scale, MathUtility::operator*(rotation, translation));

	// 転送
	worldTransform.TransferMatrix();
}

Vector3 Lerp(const Vector3& start, const Vector3& end, float t) {
	return {
		start.x + (end.x - start.x) * t,
		start.y + (end.y - start.y) * t,
		start.z + (end.z - start.z) * t
	}; 
}

bool IsCollision(const AABB& aabb1, const AABB& aabb2) {
	return (
	    aabb1.min.x <= aabb2.max.x && aabb1.max.x >= aabb2.min.x &&

	    aabb1.min.y <= aabb2.max.y && aabb1.max.y >= aabb2.min.y &&

	    aabb1.min.z <= aabb2.max.z && aabb1.max.z >= aabb2.min.z);
}
