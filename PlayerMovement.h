#pragma once

#include "KamataEngine.h"
#include "MapChipField.h"

#include <array>

// ==============================
// プレイヤーの向き
// ==============================
enum class LRDirection {
	kRight,
	kLeft,
};

// ==============================
// プレイヤーの当たり判定サイズ
// ==============================
inline constexpr float kPlayerWidth = 0.8f;
inline constexpr float kPlayerHeight = 0.8f;

// ==============================
// プレイヤーの四隅
// ==============================
enum Corner {
	kRightBottom,
	kLeftBottom,
	kRightTop,
	kLeftTop,

	kNumCorner
};

// ==============================
// マップ衝突結果
// ==============================
struct CollisionMapInfo {

	bool isCollided = false;
	bool isGrounded = false;
	bool isCeiling = false;
	bool hitWall = false;

	KamataEngine::Vector3 blockPosition = {0.0f, 0.0f, 0.0f};
};

// ==============================
// プレイヤー移動クラス
// ==============================
class PlayerMovement {
public:
	// ==============================
	// 基本処理
	// ==============================

	void Initialize(KamataEngine::WorldTransform* worldTransform, MapChipField* mapChipField);

	// 通常時の移動更新
	void Update();

	// ==============================
	// 外部から利用する移動処理
	// ==============================

	// 任意の移動量にマップ衝突を適用する
	KamataEngine::Vector3 MoveWithCollision(const KamataEngine::Vector3& velocity);

	// ==============================
	// Setter
	// ==============================

	void SetMapChipField(MapChipField* mapChipField) { mapChipField_ = mapChipField; }

	// ==============================
	// Getter
	// ==============================

	const KamataEngine::Vector3& GetVelocity() const { return velocity_; }

	LRDirection GetDirection() const { return lrDirection_; }

	bool IsGrounded() const { return onGrounded_; }

	// 外部処理で速度を止める場合などに使用
	void SetVelocity(const KamataEngine::Vector3& velocity) { velocity_ = velocity; }

private:
	// ==============================
	// 入力・移動
	// ==============================

	void InputMove();
	void Turn();

	void Move(const CollisionMapInfo& info);

	// ==============================
	// マップ衝突
	// ==============================

	void MapChipCollision(CollisionMapInfo& info);

	void MapChipCollisionUp(CollisionMapInfo& info);
	void MapChipCollisionDown(CollisionMapInfo& info);
	void MapChipCollisionRight(CollisionMapInfo& info);
	void MapChipCollisionLeft(CollisionMapInfo& info);

	KamataEngine::Vector3 CornerPosition(const KamataEngine::Vector3& center, Corner corner) const;

	// ==============================
	// 衝突後処理
	// ==============================

	void CeilingCollision(const CollisionMapInfo& info);
	void OnGround(const CollisionMapInfo& info);
	void WallCollision(const CollisionMapInfo& info);

private:
	// Player本体が所有しているTransformへの参照
	KamataEngine::WorldTransform* worldTransform_ = nullptr;

	// マップ
	MapChipField* mapChipField_ = nullptr;

	// ==============================
	// 移動
	// ==============================

	KamataEngine::Vector3 velocity_ = {0.0f, 0.0f, 0.0f};

	LRDirection lrDirection_ = LRDirection::kRight;

	// ==============================
	// 旋回
	// ==============================

	float turnFirstRotationY_ = 0.0f;
	float turnTimer_ = 0.0f;

	// ==============================
	// 接地
	// ==============================

	bool onGrounded_ = true;

private:
	// ==============================
	// 移動パラメータ
	// ==============================

	static constexpr float kAcceleration = 0.01f;
	static constexpr float kAttenuation = 0.05f;
	static constexpr float kLimitRunSpeed = 0.5f;

	static constexpr float kTimeTurn = 0.3f;

	static constexpr float kGravityAcceleration = 0.02f;
	static constexpr float kLimitFallSpeed = -1.5f;
	static constexpr float kJumpAcceleration = 0.5f;

	static constexpr float kAttenuationLanding = 0.2f;
	static constexpr float kAttenuationWall = 0.1f;
};