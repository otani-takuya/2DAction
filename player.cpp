#define NOMINMAX

#include "Player.h"
#include "math.h"
#include <algorithm>
#include <numbers>
#include "BaseEnemy.h"

using namespace KamataEngine;

// ==============================
// 初期化
// ==============================
void Player::Initialize(Model* model, Model* modelAttack, Camera* camera, const KamataEngine::Vector3& position) {

	// ワールド変換初期化
	worldTransform_.Initialize();
	worldTransformAttack_.Initialize();

	// モデル
	model_ = model;
	modelAttack_ = modelAttack;

	// カメラ
	camera_ = camera;

	// 回転
	worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;

	// 初期座標
	worldTransform_.translation_.x = position.x;
	worldTransform_.translation_.y = position.y;
	worldTransform_.translation_.z = position.z;

	// サイズ調整
	worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};

	behavior_ = Behavior::kRoot;
	behaviorRequest_ = Behavior::kUnknown;

	// 初期行列を反映
	UpdateWorldTransform(worldTransform_);
	worldTransform_.TransferMatrix();

	// 攻撃モデルの初期行列
	worldTransformAttack_.translation_ = position;
	worldTransformAttack_.scale_ = {1.0f, 1.0f, 1.0f};

	UpdateWorldTransform(worldTransformAttack_);
	worldTransformAttack_.TransferMatrix();
}

// ==============================
// 更新
// ==============================
void Player::Update() {

	// 外部からノックバック要求が来た場合
	if (isKnockbackRequested_) {

		// ノックバック状態への変更をリクエスト
		behaviorRequest_ = Behavior::kKnockback;

		// 使用済みリクエストをリセット
		isKnockbackRequested_ = false;
	}

	// 状態変更のリクエストが来たときだけ初期化する
	if (behaviorRequest_ != Behavior::kUnknown) {

		// 新しい振る舞いに変更
		behavior_ = behaviorRequest_;

		switch (behavior_) {

		case Behavior::kRoot:
			BehaviorRootInitialize();
			break;

		case Behavior::kAttack:
			BehaviorAttackInitialize();
			break;

		case Behavior::kKnockback:
			BehaviorKnockbackInitialize();
			break;

		case Behavior::kUnknown:
		default:
			break;
		}

		// 必ずリクエストをリセットする
		behaviorRequest_ = Behavior::kUnknown;
	}

	// 現在の状態の更新処理は毎フレーム実行する
	// 各振る舞いごとの初期化を実行
	switch (behavior_) {
	// 通常
	case Behavior::kRoot:
	default:

		Player::BehaviorRootUpdate();

		// 攻撃キーを押したら
		if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
			// 攻撃ビヘイビアをリクエスト
			behaviorRequest_ = Behavior::kAttack;
		}

		break;

		// アタック
	case Behavior::kAttack:

		Player::BehaviorAttackUpdate();

		break;
	
	case Behavior::kKnockback:
		BehaviorKnockbackUpdate();
		break;

	case Behavior::kUnknown:
		break;
	}
}

// ==============================
// 描画
// ==============================
void Player::Draw() {

	// 安全チェック
	if (model_ == nullptr) {
		return;
	}
	if (!isDead_) {
		model_->Draw(worldTransform_, *camera_);

		if (nowAttackPhase_ == AttackPhase::Tackle) 
		{
			modelAttack_->Draw(worldTransformAttack_, *camera_);
		}
	}
}

void Player::InputMove() {
	// ジャンプ
	// 接地状態
	if (onGrounded_) {
		// 左右移動操作
		// 左右加速
		KamataEngine::Vector3 acceleration = {};

		// 右入力
		if (Input::GetInstance()->PushKey(DIK_RIGHT)) {
			if (lrDirection_ != LRDirection::kRight) {
				lrDirection_ = LRDirection::kRight;
			}

			// 旋回開始時の角度を保存
			trunFirstRotationY_ = worldTransform_.rotation_.y;
			// 旋回タイマーに時間を設定する
			turnTimer_ = 0.0f;

			// 左入力中の右入力
			if (velocity_.x < 0.0f) {
				acceleration.x = (1.0f - kAcceleration);
			}

			acceleration.x = kAcceleration;
		}
		// 左入力
		else if (Input::GetInstance()->PushKey(DIK_LEFT)) {

			if (lrDirection_ != LRDirection::kLeft) {
				lrDirection_ = LRDirection::kLeft;
			}

			// 旋回開始時の角度を保存
			trunFirstRotationY_ = worldTransform_.rotation_.y;
			// 旋回タイマーに時間を設定する
			turnTimer_ = 0.0f;

			// 右入力中の左入力
			if (velocity_.x > 0.0f) {
				acceleration.x = (1.0f - kAcceleration);
			}
			acceleration.x = -kAcceleration;
		}
		// 入力なし → 減衰
		else {
			velocity_.x *= (1.0f - kAttenuation);
		}
		// 加速
		velocity_.x += acceleration.x;

		// 速度上限
		velocity_.x = std::clamp(velocity_.x, -kLimitRumSpeed, kLimitRumSpeed);

		if (Input::GetInstance()->PushKey(DIK_UP)) {
			// ジャンプ初速
			velocity_.y = kJumpAcceleration;
			onGrounded_ = false;
		}
	} else {
		// 落下速度
		velocity_.y -= kGravityAcceleration;
		// 落下速度制限
		// velocity_.y = std::max(velocity_.y, -kLimitFallSpeed);
	}

	bool landing = false;

	// 着地判定
	if (onGrounded_) {
		// ジャンプ開始
		if (velocity_.y > 0.0f) {
			// 空中状態に移行
			onGrounded_ = false;
		}
	} else {
		// 着地
		if (landing) {
			// めり込み排除
			worldTransform_.translation_.y = 1.0f;
			// 摩擦で横方向速度が減衰する
			velocity_.x *= (1.0f - kAttenuation);
			// 下方向速度をリセット
			velocity_.y = 0.0f;
			// 設置状態に移行
			onGrounded_ = true;
		}
	}
}

KamataEngine::Vector3 Player::CornerPosition(const KamataEngine::Vector3& center, Corner corner) {
	KamataEngine::Vector3 offsetTable[kNumCorner] = {
	    {+kPlayerWidth / 2.0f, -kPlayerHeight / 2.0f, 0.0f}, //  kRightBottom
	    {-kPlayerWidth / 2.0f, -kPlayerHeight / 2.0f, 0.0f}, //  kLeftBottom
	    {+kPlayerWidth / 2.0f, +kPlayerHeight / 2.0f, 0.0f}, //  kRightTop
	    {-kPlayerWidth / 2.0f, +kPlayerHeight / 2.0f, 0.0f}, //  kLeftTop
	};
	return center + offsetTable[static_cast<uint32_t>(corner)];
}

void Player::MapChipCollision(CollisionMapInfo& info) {
	MapChipCollisionUp(info);
	MapChipCollisionDown(info);
	MapChipCollisionRight(info);
	MapChipCollisionLeft(info);
}

void Player::MapChipCollisionUp(CollisionMapInfo& info) {
	MapChipType mapChipType;
	MapChipType mapChipTypeNext;

	// 上に移動していないなら判定しない
	if (info.blockPosition.y <= 0.0f) {
		return;
	}

	// 移動後の4角を求める
	std::array<KamataEngine::Vector3, static_cast<uint32_t>(Corner::kNumCorner)> positionsNew;

	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.blockPosition, static_cast<Corner>(i));
	}

	bool hit = false;
	IndexSet indexSet = {};

	// 左上
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex + 1);

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

	if (hit) {
		IndexSet indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(CornerPosition(worldTransform_.translation_, kLeftTop));

		IndexSet indexSetAfter = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]);

		if (indexSetNow.yIndex != indexSetAfter.yIndex) {
			Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);

			float playerTop = CornerPosition(worldTransform_.translation_, kLeftTop).y;

			info.blockPosition.y = std::max(0.0f, rect.bottom - playerTop);

			info.isCeiling = true;
		}
	}
}

void Player::MapChipCollisionDown(CollisionMapInfo& info) {
	MapChipType mapChipType;
	MapChipType mapChipTypeNext;

	// 下に移動していなければ判定しない
	if (info.blockPosition.y >= 0.0f) {
		return;
	}

	// 移動後の4角
	std::array<KamataEngine::Vector3, static_cast<uint32_t>(Corner::kNumCorner)> positionsNew;

	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.blockPosition, static_cast<Corner>(i));
	}

	bool hit = false;
	IndexSet indexSet{};

	// 左下
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex - 1);
	// 隣接セルがともにブロックであればヒット
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}

	// 右下
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex - 1);
	// 隣接セルがともにブロックであればヒット
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}

	if (hit) {

		// 移動前の足元のセル
		IndexSet indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(CornerPosition(worldTransform_.translation_, kLeftBottom));

		// 移動後の足元のセル
		IndexSet indexSetAfter = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom]);

		// セルが変わったときだけ補正する
		if (indexSetNow.yIndex != indexSetAfter.yIndex) {

			Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);

			float playerBottom = CornerPosition(worldTransform_.translation_, kLeftBottom).y;

			info.blockPosition.y = std::min(0.0f, rect.top - playerBottom);

			info.isGrounded = true;
		}
	}
}

void Player::MapChipCollisionRight(CollisionMapInfo& info) {
	MapChipType mapChipType;
	MapChipType mapChipTypeNext;

	// 右に移動していないなら判定しない
	if (info.blockPosition.x <= 0.0f) {
		return;
	}

	// 移動後の4つの角
	std::array<KamataEngine::Vector3, static_cast<uint32_t>(Corner::kNumCorner)> positionsNew;

	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.blockPosition, static_cast<Corner>(i));
	}

	bool hit = false;
	IndexSet indexSet;

	// 右上
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex - 1, indexSet.yIndex);

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

	// ブロックに衝突
	if (hit) {
		IndexSet indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(CornerPosition(worldTransform_.translation_, kRightTop));

		IndexSet indexSetAfter = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightTop]);

		if (indexSetNow.xIndex != indexSetAfter.xIndex) {
			Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);

			float playerRight = CornerPosition(worldTransform_.translation_, kRightTop).x;

			info.blockPosition.x = std::min(0.0f, rect.left - playerRight);

			info.hitWall = true;
		}
	}
}

void Player::MapChipCollisionLeft(CollisionMapInfo& info) {
	MapChipType mapChipType;
	MapChipType mapChipTypeNext;

	// 左に移動していないなら判定しない
	if (info.blockPosition.x >= 0.0f) {
		return;
	}

	// 移動後の4つの角
	std::array<KamataEngine::Vector3, static_cast<uint32_t>(Corner::kNumCorner)> positionsNew;

	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.blockPosition, static_cast<Corner>(i));
	}

	bool hit = false;
	IndexSet indexSet;

	// 左上
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex + 1, indexSet.yIndex);

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

	if (hit) {
		IndexSet indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(CornerPosition(worldTransform_.translation_, kLeftTop));

		IndexSet indexSetAfter = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]);

		if (indexSetNow.xIndex != indexSetAfter.xIndex) {
			Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);

			float playerLeft = CornerPosition(worldTransform_.translation_, kLeftTop).x;

			info.blockPosition.x = std::max(0.0f, rect.right - playerLeft);

			info.hitWall = true;
		}
	}
}

void Player::Turn() {

	if (turnTimer_ >= kTimeTurn) {
		return;
	}

	turnTimer_ += 1.0f / 60.0f;

	float destinationRotationYTable[] = {
	    std::numbers::pi_v<float> / 2.0f,
	    std::numbers::pi_v<float> * 3.0f / 2.0f,
	};

	float destinationRotationY = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)];

	worldTransform_.rotation_.y = trunFirstRotationY_ + (destinationRotationY - trunFirstRotationY_) * turnTimer_ / kTimeTurn;
}

void Player::Move(const CollisionMapInfo& info) {

	worldTransform_.translation_.x += info.blockPosition.x;
	worldTransform_.translation_.y += info.blockPosition.y;
	worldTransform_.translation_.z += info.blockPosition.z;
}

void Player::CeilingCollision(CollisionMapInfo& info) {
	if (info.isCeiling) {
		// DebugText::GetInstance()->ConsolePrintf("hit ceiling\n");
		velocity_.y = 0.0f;
	}
}

void Player::OnGround(const CollisionMapInfo& info) {

	// 接地状態
	if (onGrounded_) {

		// ジャンプ開始
		if (velocity_.y > 0.0f) {
			onGrounded_ = false;
		} else {
			// 落下判定
			bool hit = false;

			// 足元を少し下にずらして判定
			const float kGroundCheckOffset = 0.1f;

			KamataEngine::Vector3 leftBottom = CornerPosition(worldTransform_.translation_, kLeftBottom) + KamataEngine::Vector3(0.0f, -kGroundCheckOffset, 0.0f);

			KamataEngine::Vector3 rightBottom = CornerPosition(worldTransform_.translation_, kRightBottom) + KamataEngine::Vector3(0.0f, -kGroundCheckOffset, 0.0f);

			IndexSet indexSet;

			// 左下
			indexSet = mapChipField_->GetMapChipIndexSetByPosition(leftBottom);
			if (mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex) == MapChipType::kBlock) {
				hit = true;
			}

			// 右下
			indexSet = mapChipField_->GetMapChipIndexSetByPosition(rightBottom);
			if (mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex) == MapChipType::kBlock) {
				hit = true;
			}

			// 足元にブロックがなければ空中へ
			if (!hit) {
				onGrounded_ = false;
			}
		}

	} else {
		// 空中状態

		// 着地した
		if (info.isGrounded) {
			onGrounded_ = true;

			// 着地時に横速度を少し落とす
			velocity_.x *= (1.0f - kAttenuationLanding);

			// Y速度を止める
			velocity_.y = 0.0f;
		}
	}
}

void Player::WallCollision(const CollisionMapInfo& info) {

	// 壁接触による減速
	if (info.hitWall) {

		velocity_.x *= (1.0f - kAttenuationWall);
	}
}

KamataEngine::Vector3 Player::GetWorldPosition() const {
	// ワールド座標を入れる変数
	Vector3 worldPos;
	// ワールド行列の平行移動成分を取得(ワールド座標)
	worldPos.x = worldTransform_.translation_.x;
	worldPos.y = worldTransform_.translation_.y;
	worldPos.z = worldTransform_.translation_.z;

	return worldPos;
}

AABB Player::GetAABB() {

	Vector3 worldPos = GetWorldPosition();

	AABB aabb;

	aabb.min = {worldPos.x - kPlayerWidth / 2.0f, worldPos.y - kPlayerHeight / 2.0f, worldPos.z - kPlayerWidth / 2.0f};

	aabb.max = {worldPos.x + kPlayerWidth / 2.0f, worldPos.y + kPlayerHeight / 2.0f, worldPos.z + kPlayerWidth / 2.0f};

	return aabb;
}

void Player::OnCollision(const BaseEnemy* enemy) {
	(void)enemy;

	if (IsAttack()) {
		return; //攻撃中はダメージ無効
	}

	// 死亡フラグを立てる
	isDead_ = true;
}

void Player::BehaviorRootUpdate() {
	// ①移動入力
	InputMove();

	// ②移動量を加味して衝突判定する
	CollisionMapInfo collisionMapInfo;
	collisionMapInfo.blockPosition = velocity_;

	MapChipCollision(collisionMapInfo);

	// ③判定結果を反映して移動させる
	Move(collisionMapInfo);

	// ④天井に接触している場合の処理
	CeilingCollision(collisionMapInfo);

	// ⑤地面に接触している場合の処理
	WallCollision(collisionMapInfo);

	// ⑥接地状態の切り替え
	OnGround(collisionMapInfo);

	// ⑦旋回制御
	Turn();

	// ⑧行列計算
	UpdateWorldTransform(worldTransform_);
	worldTransform_.TransferMatrix();
}

void Player::BehaviorAttackUpdate() {

	attackParameter_++;

	Vector3 velocity{};
	float t = 0.0f;

	switch (nowAttackPhase_) {

	// ===================================
	// 溜め動作
	// ===================================
	case AttackPhase::ChargeUp:

		// 15フレームかけて0.0～1.0へ
		t = static_cast<float>(attackParameter_) / 15.0f;
		t = std::clamp(t, 0.0f, 1.0f);

		// 横幅を細くする
		worldTransform_.scale_.x = 1.0f + (0.4f - 1.0f) * t;

		// 縦に少し伸びる
		worldTransform_.scale_.y = 1.0f + (1.7f - 1.0f) * t;

		// 奥行きは少し細くする
		worldTransform_.scale_.z = 1.0f + (1.0f - 1.0f) * t;

		// 向いている方向とは反対へ少し下がる
		if (lrDirection_ == LRDirection::kRight) {
			velocity.x = -0.03f;
		} else {
			velocity.x = 0.03f;
		}

		if (attackParameter_ >= 18) {
			nowAttackPhase_ = AttackPhase::Tackle;
			attackParameter_ = 0;
		}

		break;

	// ===================================
	// 突進動作
	// ===================================
	case AttackPhase::Tackle:

		// プレイヤーが向いている方向へ突進
		if (lrDirection_ == LRDirection::kRight) {
			velocity.x = attackVelocity_.x;
		} else {
			velocity.x = -attackVelocity_.x;
		}

		// 8フレームかけて変形
		t = static_cast<float>(attackParameter_) / 8.0f;
		t = std::clamp(t, 0.0f, 1.0f);

		// 進行方向へ大きく伸びる
		worldTransform_.scale_.x = 0.4f + (2.5f - 0.4f) * t;

		// 縦方向には潰れる
		worldTransform_.scale_.y = 1.7f + (0.3f - 1.7f) * t;

		// 奥行きは通常に戻す
		worldTransform_.scale_.z = 1.0f;

		//トランスフォームの値をコピー
		worldTransformAttack_.translation_ = worldTransform_.translation_;
		worldTransformAttack_.rotation_ = worldTransform_.rotation_;

		// 攻撃モデルの大きさ
		worldTransformAttack_.scale_ = {1.0f, 1.0f, 1.0f};

		// プレイヤーの前へ配置
		if (lrDirection_ == LRDirection::kRight) {
			worldTransformAttack_.translation_.x += 0.6f;
		} else {
			worldTransformAttack_.translation_.x -= 0.6f;
		}


		if (attackParameter_ >= 8) {
			nowAttackPhase_ = AttackPhase::After;
			attackParameter_ = 0;
		}

		break;

	// ===================================
	// 余韻動作
	// ===================================
	case AttackPhase::After:

		// 12フレームで通常サイズへ戻す
		t = static_cast<float>(attackParameter_) / 12.0f;
		t = std::clamp(t, 0.0f, 1.0f);

		worldTransform_.scale_.x = 2.5f + (1.0f - 2.5f) * t;

		worldTransform_.scale_.y = 0.3f + (1.0f - 0.3f) * t;

		worldTransform_.scale_.z = 1.0f;

		if (attackParameter_ >= 12) {
			// 念のため完全に元のサイズに戻す
			worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};

			behaviorRequest_ = Behavior::kRoot;
			attackParameter_ = 0;
		}

		break;
	}

	// ===================================
	// マップとの衝突判定
	// ===================================
	CollisionMapInfo collisionMapInfo{};
	collisionMapInfo.blockPosition = velocity;

	MapChipCollision(collisionMapInfo);

	// 衝突判定後の移動量を反映
	worldTransform_.translation_.x += collisionMapInfo.blockPosition.x;

	worldTransform_.translation_.y += collisionMapInfo.blockPosition.y;

	// ===================================
	// 行列更新
	// ===================================
	
	// プレイヤー本体
	UpdateWorldTransform(worldTransform_);
	worldTransform_.TransferMatrix();

	// 攻撃エフェクト
	UpdateWorldTransform(worldTransformAttack_);
	worldTransformAttack_.TransferMatrix();
}

void Player::BehaviorRootInitialize() {

	// 通常サイズに戻す
	worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};

	// 攻撃情報をリセット
	attackParameter_ = 0;
	nowAttackPhase_ = AttackPhase::ChargeUp;

	// ノックバック速度をリセット
	knockbackVelocity_ = {0.0f, 0.0f, 0.0f};

	UpdateWorldTransform(worldTransform_);
	worldTransform_.TransferMatrix();
}

void Player::BehaviorAttackInitialize() {
	// カウンター初期化
	attackParameter_ = 0;

	nowAttackPhase_ = AttackPhase::ChargeUp;

	// 前の攻撃の変形が残らないようにする
	worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};
}

bool Player::IsAttack() const {
	return behavior_ == Behavior::kAttack;
}

void Player::RequestKnockback(const KamataEngine::Vector3& direction) {

	// リクエストだけ保存する
	isKnockbackRequested_ = true;
	knockbackRequestDirection_ = direction;
}

void Player::BehaviorKnockbackInitialize() {

	// ノックバック時間をリセット
	knockbackTimer_ = 0.0f;

	// 攻撃モーションを強制終了
	attackParameter_ = 0;
	nowAttackPhase_ = AttackPhase::ChargeUp;

	// 攻撃で変形した大きさを元に戻す
	worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};

	// 攻撃モデルも元の大きさに戻す
	worldTransformAttack_.scale_ = {1.0f, 1.0f, 1.0f};

	// 通常移動の速度を止める
	velocity_.x = 0.0f;

	// 渡された方向をノックバック速度にする
	knockbackVelocity_.x = knockbackRequestDirection_.x * kKnockbackSpeed;

	knockbackVelocity_.y = 0.0f;
	knockbackVelocity_.z = 0.0f;

	UpdateWorldTransform(worldTransform_);
	worldTransform_.TransferMatrix();
}

void Player::BehaviorKnockbackUpdate() {

	constexpr float kDeltaTime = 1.0f / 60.0f;

	knockbackTimer_ += kDeltaTime;

	// ノックバック移動
	worldTransform_.translation_.x += knockbackVelocity_.x;

	// 少しずつ減速
	knockbackVelocity_.x *= 0.85f;

	// ワールド行列更新
	UpdateWorldTransform(worldTransform_);
	worldTransform_.TransferMatrix();

	// 一定時間経過したら通常状態へ戻る
	if (knockbackTimer_ >= kKnockbackDuration) {

		knockbackVelocity_ = {0.0f, 0.0f, 0.0f};

		behaviorRequest_ = Behavior::kRoot;
	}
}