#define NOMINMAX

#include "PlayerMovement.h"

#include <algorithm>
#include <cassert>
#include <numbers>

using namespace KamataEngine;

// ==============================
// 初期化
// ==============================
void PlayerMovement::Initialize(WorldTransform* worldTransform, MapChipField* mapChipField) {

	assert(worldTransform);

	worldTransform_ = worldTransform;
	mapChipField_ = mapChipField;

	velocity_ = {0.0f, 0.0f, 0.0f};

	lrDirection_ = LRDirection::kRight;

	turnFirstRotationY_ = worldTransform_->rotation_.y;
	turnTimer_ = kTimeTurn;

	onGrounded_ = true;
}

// ==============================
// 通常移動更新
// ==============================
void PlayerMovement::Update() {

	if (worldTransform_ == nullptr) {
		return;
	}

	// ① 入力・速度計算
	InputMove();

	// マップが設定されていない場合
	if (mapChipField_ == nullptr) {

		worldTransform_->translation_.x += velocity_.x;
		worldTransform_->translation_.y += velocity_.y;
		worldTransform_->translation_.z += velocity_.z;
		Turn();

		return;
	}

	// ② 移動量
	CollisionMapInfo collisionMapInfo{};
	collisionMapInfo.blockPosition = velocity_;

	// ③ マップとの衝突
	MapChipCollision(collisionMapInfo);

	// ④ 移動
	Move(collisionMapInfo);

	// ⑤ 衝突後処理
	CeilingCollision(collisionMapInfo);
	WallCollision(collisionMapInfo);
	OnGround(collisionMapInfo);

	// ⑥ 旋回
	Turn();
}

// ==============================
// 任意速度で衝突付き移動
// ==============================
Vector3 PlayerMovement::MoveWithCollision(const Vector3& velocity) {

	if (worldTransform_ == nullptr) {
		return {};
	}

	CollisionMapInfo collisionMapInfo{};
	collisionMapInfo.blockPosition = velocity;

	if (mapChipField_ != nullptr) {
		MapChipCollision(collisionMapInfo);
	}

	Move(collisionMapInfo);

	return collisionMapInfo.blockPosition;
}

// ==============================
// 入力・速度計算
// ==============================
void PlayerMovement::InputMove() {

	if (onGrounded_) {

		Vector3 acceleration{};

		// 右
		if (Input::GetInstance()->PushKey(DIK_RIGHT)) {

			if (lrDirection_ != LRDirection::kRight) {

				lrDirection_ = LRDirection::kRight;

				turnFirstRotationY_ = worldTransform_->rotation_.y;

				turnTimer_ = 0.0f;
			}

			acceleration.x = kAcceleration;
		}

		// 左
		else if (Input::GetInstance()->PushKey(DIK_LEFT)) {

			if (lrDirection_ != LRDirection::kLeft) {

				lrDirection_ = LRDirection::kLeft;

				turnFirstRotationY_ = worldTransform_->rotation_.y;

				turnTimer_ = 0.0f;
			}

			acceleration.x = -kAcceleration;
		}

		// 入力なし
		else {

			velocity_.x *= (1.0f - kAttenuation);
		}

		velocity_.x += acceleration.x;

		velocity_.x = std::clamp(velocity_.x, -kLimitRunSpeed, kLimitRunSpeed);

		// ジャンプ
		if (Input::GetInstance()->PushKey(DIK_UP)) {

			velocity_.y = kJumpAcceleration;
			onGrounded_ = false;
		}

	} else {

		// 重力
		velocity_.y -= kGravityAcceleration;

		// 元コードでは最大落下速度制限が
		// コメントアウトされていたため、今回は変更しない
	}
}

// ==============================
// 四隅取得
// ==============================
Vector3 PlayerMovement::CornerPosition(const Vector3& center, Corner corner) const {

	const Vector3 offsetTable[kNumCorner] = {

	    {+kPlayerWidth / 2.0f, -kPlayerHeight / 2.0f, 0.0f},

	    {-kPlayerWidth / 2.0f, -kPlayerHeight / 2.0f, 0.0f},

	    {+kPlayerWidth / 2.0f, +kPlayerHeight / 2.0f, 0.0f},

	    {-kPlayerWidth / 2.0f, +kPlayerHeight / 2.0f, 0.0f},
	};

	return center + offsetTable[static_cast<uint32_t>(corner)];
}

// ==============================
// マップ衝突
// ==============================
void PlayerMovement::MapChipCollision(CollisionMapInfo& info) {

	if (mapChipField_ == nullptr || worldTransform_ == nullptr) {
		return;
	}

	MapChipCollisionUp(info);
	MapChipCollisionDown(info);
	MapChipCollisionRight(info);
	MapChipCollisionLeft(info);
}

// ==============================
// 上
// ==============================
void PlayerMovement::MapChipCollisionUp(CollisionMapInfo& info) {

	if (info.blockPosition.y <= 0.0f) {
		return;
	}

	std::array<Vector3, kNumCorner> positionsNew;

	for (uint32_t i = 0; i < positionsNew.size(); ++i) {

		positionsNew[i] = CornerPosition(worldTransform_->translation_ + info.blockPosition, static_cast<Corner>(i));
	}

	bool hit = false;
	IndexSet indexSet{};

	// 左上
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]);

	MapChipType mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);

	MapChipType mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex + 1);

	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {

		hit = true;
	}

	// 右上
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightTop]);

	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);

	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex + 1);

	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {

		hit = true;
	}

	if (!hit) {
		return;
	}

	IndexSet indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(CornerPosition(worldTransform_->translation_, kLeftTop));

	IndexSet indexSetAfter = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]);

	if (indexSetNow.yIndex == indexSetAfter.yIndex) {
		return;
	}

	Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);

	float playerTop = CornerPosition(worldTransform_->translation_, kLeftTop).y;

	info.blockPosition.y = std::max(0.0f, rect.bottom - playerTop);

	info.isCeiling = true;
}

// ==============================
// 下
// ==============================
void PlayerMovement::MapChipCollisionDown(CollisionMapInfo& info) {

	if (info.blockPosition.y >= 0.0f) {
		return;
	}

	std::array<Vector3, kNumCorner> positionsNew;

	for (uint32_t i = 0; i < positionsNew.size(); ++i) {

		positionsNew[i] = CornerPosition(worldTransform_->translation_ + info.blockPosition, static_cast<Corner>(i));
	}

	bool hit = false;
	IndexSet indexSet{};

	// 左下
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom]);

	MapChipType mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);

	MapChipType mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex - 1);

	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {

		hit = true;
	}

	// 右下
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom]);

	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);

	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex - 1);

	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {

		hit = true;
	}

	if (!hit) {
		return;
	}

	IndexSet indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(CornerPosition(worldTransform_->translation_, kLeftBottom));

	IndexSet indexSetAfter = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom]);

	if (indexSetNow.yIndex == indexSetAfter.yIndex) {
		return;
	}

	Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);

	float playerBottom = CornerPosition(worldTransform_->translation_, kLeftBottom).y;

	info.blockPosition.y = std::min(0.0f, rect.top - playerBottom);

	info.isGrounded = true;
}

// ==============================
// 右
// ==============================
void PlayerMovement::MapChipCollisionRight(CollisionMapInfo& info) {

	if (info.blockPosition.x <= 0.0f) {
		return;
	}

	std::array<Vector3, kNumCorner> positionsNew;

	for (uint32_t i = 0; i < positionsNew.size(); ++i) {

		positionsNew[i] = CornerPosition(worldTransform_->translation_ + info.blockPosition, static_cast<Corner>(i));
	}

	bool hit = false;
	IndexSet indexSet{};

	// 右上
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightTop]);

	MapChipType mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);

	MapChipType mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex - 1, indexSet.yIndex);

	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {

		hit = true;
	}

	// 右下
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom]);

	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);

	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex - 1, indexSet.yIndex);

	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {

		hit = true;
	}

	if (!hit) {
		return;
	}

	IndexSet indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(CornerPosition(worldTransform_->translation_, kRightTop));

	IndexSet indexSetAfter = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightTop]);

	if (indexSetNow.xIndex == indexSetAfter.xIndex) {
		return;
	}

	Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);

	float playerRight = CornerPosition(worldTransform_->translation_, kRightTop).x;

	info.blockPosition.x = std::min(0.0f, rect.left - playerRight);

	info.hitWall = true;
}

// ==============================
// 左
// ==============================
void PlayerMovement::MapChipCollisionLeft(CollisionMapInfo& info) {

	if (info.blockPosition.x >= 0.0f) {
		return;
	}

	std::array<Vector3, kNumCorner> positionsNew;

	for (uint32_t i = 0; i < positionsNew.size(); ++i) {

		positionsNew[i] = CornerPosition(worldTransform_->translation_ + info.blockPosition, static_cast<Corner>(i));
	}

	bool hit = false;
	IndexSet indexSet{};

	// 左上
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]);

	MapChipType mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);

	MapChipType mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex + 1, indexSet.yIndex);

	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {

		hit = true;
	}

	// 左下
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom]);

	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);

	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex + 1, indexSet.yIndex);

	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {

		hit = true;
	}

	if (!hit) {
		return;
	}

	IndexSet indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(CornerPosition(worldTransform_->translation_, kLeftTop));

	IndexSet indexSetAfter = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]);

	if (indexSetNow.xIndex == indexSetAfter.xIndex) {
		return;
	}

	Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);

	float playerLeft = CornerPosition(worldTransform_->translation_, kLeftTop).x;

	info.blockPosition.x = std::max(0.0f, rect.right - playerLeft);

	info.hitWall = true;
}

// ==============================
// 移動反映
// ==============================
void PlayerMovement::Move(const CollisionMapInfo& info) {

	worldTransform_->translation_.x += info.blockPosition.x;
	worldTransform_->translation_.y += info.blockPosition.y;
	worldTransform_->translation_.z += info.blockPosition.z;
}

// ==============================
// 天井
// ==============================
void PlayerMovement::CeilingCollision(const CollisionMapInfo& info) {

	if (info.isCeiling) {
		velocity_.y = 0.0f;
	}
}

// ==============================
// 接地
// ==============================
void PlayerMovement::OnGround(const CollisionMapInfo& info) {

	if (onGrounded_) {

		if (velocity_.y > 0.0f) {

			onGrounded_ = false;

		} else {

			bool hit = false;

			constexpr float kGroundCheckOffset = 0.1f;

			Vector3 leftBottom = CornerPosition(worldTransform_->translation_, kLeftBottom) + Vector3(0.0f, -kGroundCheckOffset, 0.0f);

			Vector3 rightBottom = CornerPosition(worldTransform_->translation_, kRightBottom) + Vector3(0.0f, -kGroundCheckOffset, 0.0f);

			IndexSet indexSet{};

			indexSet = mapChipField_->GetMapChipIndexSetByPosition(leftBottom);

			if (mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex) == MapChipType::kBlock) {

				hit = true;
			}

			indexSet = mapChipField_->GetMapChipIndexSetByPosition(rightBottom);

			if (mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex) == MapChipType::kBlock) {

				hit = true;
			}

			if (!hit) {
				onGrounded_ = false;
			}
		}

	} else {

		if (info.isGrounded) {

			onGrounded_ = true;

			velocity_.x *= (1.0f - kAttenuationLanding);

			velocity_.y = 0.0f;
		}
	}
}

// ==============================
// 壁
// ==============================
void PlayerMovement::WallCollision(const CollisionMapInfo& info) {

	if (info.hitWall) {

		velocity_.x *= (1.0f - kAttenuationWall);
	}
}

// ==============================
// 旋回
// ==============================
void PlayerMovement::Turn() {

	if (turnTimer_ >= kTimeTurn) {
		return;
	}

	turnTimer_ += 1.0f / 60.0f;

	const float destinationRotationYTable[] = {

	    std::numbers::pi_v<float> / 2.0f,

	    std::numbers::pi_v<float> * 3.0f / 2.0f,
	};

	const float destinationRotationY = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)];

	worldTransform_->rotation_.y = turnFirstRotationY_ + (destinationRotationY - turnFirstRotationY_) * turnTimer_ / kTimeTurn;
}