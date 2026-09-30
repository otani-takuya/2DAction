#pragma once

#include "KamataEngine.h"
#include "Collision.h"

// ================================
// ワールド行列更新
// ================================
void UpdateWorldTransform(KamataEngine::WorldTransform& worldTransform);

KamataEngine::Vector3 Lerp(const KamataEngine::Vector3& start, const KamataEngine::Vector3& end, float t);

bool IsCollision(const AABB& aabb1, const AABB& aabb2);