#define NOMINMAX

#include "Player.h"

#include "BaseEnemy.h"
#include "math.h"
#include <cassert>
#include <cmath>

#include <numbers>

using namespace KamataEngine;

// ==============================
// 初期化
// ==============================
void Player::Initialize(const PlayerModels& models, Model* modelAttack, Camera* camera, const Vector3& position) {

	// ==============================
	// Player本体
	// ==============================

	worldTransform_.Initialize();

	camera_ = camera;

	// 初期回転
	worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;

	// 初期位置
	worldTransform_.translation_ = position;

	// 初期サイズ
	worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};

	// ==============================
	// 状態
	// ==============================

	behavior_ = Behavior::kRoot;
	behaviorRequest_ = Behavior::kUnknown;

	isDead_ = false;

	// ==============================
	// 行列
	// ==============================

	UpdateWorldTransform(worldTransform_);
	worldTransform_.TransferMatrix();

	// ==============================
	// CharacterParts初期化
	// ==============================

	// Player全体のTransformを親に設定
	characterParts_.SetRootTransform(&worldTransform_);

	// ==============================
	// Playerの7パーツを登録
	// ==============================

	// 胴体
	characterParts_.AddPart(
	    "Body", models.body, {0.0f, -0.4f, 0.0f}, // Translation
	    {0.0f, 0.0f, 0.0f},                      // Rotation（ラジアン）
	    {1.0f, 1.0f, 1.0f}                       // Scale
	);

	// 頭
	characterParts_.AddPart("Head", models.head, {0.0f, -0.4f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f});

	// 左手
	characterParts_.AddPart("LeftHand", models.leftHand, {0.0f, -0.4f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f});

	// 右手
	characterParts_.AddPart("RightHand", models.rightHand, {0.0f, -0.4f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f});

	// 左足
	characterParts_.AddPart("LeftFoot", models.leftFoot, {0.0f, -0.4f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f});

	// 右足
	characterParts_.AddPart("RightFoot", models.rightFoot, {0.0f, -0.4f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f});

	// 武器
	characterParts_.AddPart("Weapon", models.weapon, {0.0f, -0.4f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f});

	// ==============================
	// Weaponの位置をRightHand基準へ変換
	// ==============================

	auto* rightHand = characterParts_.GetPart("RightHand");
	auto* weapon = characterParts_.GetPart("Weapon");

	if (rightHand != nullptr && weapon != nullptr) {

		// 現在のPlayer基準の位置を取得
		const Vector3 rightHandPosition = rightHand->worldTransform.translation_;

		const Vector3 weaponPosition = weapon->worldTransform.translation_;

		// Weaponの位置をRightHand基準へ変更
		weapon->worldTransform.translation_ = {weaponPosition.x - rightHandPosition.x, weaponPosition.y - rightHandPosition.y, weaponPosition.z - rightHandPosition.z};

		// 新しい位置を初期姿勢として保存
		weapon->baseTranslation = weapon->worldTransform.translation_;
	}

	// ==============================
	// WeaponをRightHandの子に設定
	// ==============================

	// 右手を親、武器を子にする
	const bool isWeaponParentSet = characterParts_.SetParent("Weapon", "RightHand");

	// 親子関係の設定に失敗していないか確認
	assert(isWeaponParentSet);

	// ========================================
	// アニメーション管理クラスを初期化
	// ========================================
	animationController_.Initialize(&characterParts_);

	// 初回の行列更新
	characterParts_.Update();

	// ==============================
	// PlayerMovement
	// ==============================

	movement_.Initialize(&worldTransform_, mapChipField_);

	// ==============================
	// PlayerCombat
	// ==============================

	combat_.Initialize(&worldTransform_, modelAttack, camera_, &movement_);
}

// ==============================
// 更新
// ==============================
void Player::Update() {

	// ==============================
	// ノックバック要求
	// ==============================

	if (isKnockbackRequested_) {

		behaviorRequest_ = Behavior::kKnockback;

		isKnockbackRequested_ = false;
	}

	// ==============================
	// 状態変更
	// ==============================

	if (behaviorRequest_ != Behavior::kUnknown) {

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

		behaviorRequest_ = Behavior::kUnknown;
	}

	// ==============================
	// 現在の状態を更新
	// ==============================

	switch (behavior_) {

	case Behavior::kRoot:

		BehaviorRootUpdate();

		// 攻撃
		if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {

			behaviorRequest_ = Behavior::kAttack;
		}

		break;

	case Behavior::kAttack:

		BehaviorAttackUpdate();

		break;

	case Behavior::kKnockback:

		BehaviorKnockbackUpdate();

		break;

	case Behavior::kUnknown:
	default:

		break;
	}

	// ========================================
	// Player全体のワールド行列更新
	// ========================================
	UpdateWorldTransform(worldTransform_);

	// ========================================
	// Playerの移動状態を判定
	// ========================================

	// PlayerMovementから移動速度を取得
	const KamataEngine::Vector3& velocity = movement_.GetVelocity();

	// ========================================
	// AnimationControllerへ移動速度を渡す
	// ========================================
	animationController_.SetMovementSpeed(velocity.x);

	// X方向の移動速度
	const float velocityX = velocity.x;

	// 左右に移動しているか判定
	constexpr float kMoveThreshold = 0.01f;

	const bool isMoving = std::abs(velocityX) > kMoveThreshold;

	const bool isGrounded = movement_.IsGrounded();

	// ========================================
	// AnimationControllerへ情報を渡す
	// ========================================

	animationController_.SetMovementSpeed(velocity.x);

	animationController_.SetJumpInfo(isGrounded, velocity.y);

	// ========================================
	// Idle / Walk / Jumpの状態切り替え
	// ========================================

	if (behavior_ == Behavior::kRoot) {

		if (!isGrounded) {

			// 空中ではJumpを優先
			animationController_.SetAnimationState(PlayerAnimationController::AnimationState::kJump);

		} else if (isMoving) {

			// 接地中かつ移動中はWalk
			animationController_.SetAnimationState(PlayerAnimationController::AnimationState::kWalk);

		} else {

			// 接地中かつ停止中はIdle
			animationController_.SetAnimationState(PlayerAnimationController::AnimationState::kIdle);
		}
	}

	// ========================================
	// アニメーション更新
	// ========================================

	// 60FPSを基準とした固定時間
	constexpr float kDeltaTime = 1.0f / 60.0f;

	animationController_.Update(kDeltaTime);

	// ========================================
	// 各パーツのワールド行列更新
	// ========================================
	characterParts_.Update();

}

// ==============================
// 描画
// ==============================
void Player::Draw() {

	// カメラが設定されていない場合は描画しない
	if (camera_ == nullptr) {
		return;
	}

	// 死亡中は描画しない
	if (isDead_) {
		return;
	}

	// ==============================
	// Playerの7パーツを描画
	// ==============================
	characterParts_.Draw(camera_);

	// ==============================
	// 攻撃エフェクト描画
	// ==============================
	if (behavior_ == Behavior::kAttack) {
		combat_.Draw();
	}
}

// ==============================
// ワールド座標
// ==============================
Vector3 Player::GetWorldPosition() const {

	Vector3 worldPosition{};

	worldPosition.x = worldTransform_.translation_.x;

	worldPosition.y = worldTransform_.translation_.y;

	worldPosition.z = worldTransform_.translation_.z;

	return worldPosition;
}

// ==============================
// AABB
// ==============================
AABB Player::GetAABB() {

	const Vector3 worldPosition = GetWorldPosition();

	AABB aabb{};

	aabb.min = {
	    worldPosition.x - kPlayerWidth / 2.0f,

	    worldPosition.y - kPlayerHeight / 2.0f,

	    worldPosition.z - kPlayerWidth / 2.0f};

	aabb.max = {
	    worldPosition.x + kPlayerWidth / 2.0f,

	    worldPosition.y + kPlayerHeight / 2.0f,

	    worldPosition.z + kPlayerWidth / 2.0f};

	return aabb;
}

// ==============================
// 敵との衝突
// ==============================
void Player::OnCollision(const BaseEnemy* enemy) {

	(void)enemy;

	// 攻撃中はダメージ無効
	if (IsAttack()) {
		return;
	}

	isDead_ = true;
}

// ==============================
// Root初期化
// ==============================
void Player::BehaviorRootInitialize() {

	// 戦闘状態をリセット
	combat_.Reset();

	// ノックバック速度をリセット
	knockbackVelocity_ = {0.0f, 0.0f, 0.0f};

	UpdateWorldTransform(worldTransform_);
	worldTransform_.TransferMatrix();
}

// ==============================
// Root更新
// ==============================
void Player::BehaviorRootUpdate() {

	// 移動処理はPlayerMovementへ任せる
	movement_.Update();

	// Player本体の行列更新
	UpdateWorldTransform(worldTransform_);
	worldTransform_.TransferMatrix();
}

// ==============================
// Attack初期化
// ==============================
void Player::BehaviorAttackInitialize() {

	// PlayerCombatへ攻撃開始を依頼
	combat_.StartAttack();
}

// ==============================
// Attack更新
// ==============================
void Player::BehaviorAttackUpdate() {

	// 攻撃処理
	combat_.Update();

	// 攻撃終了
	if (combat_.IsFinished()) {

		behaviorRequest_ = Behavior::kRoot;
	}
}

// ==============================
// 攻撃中か
// ==============================
bool Player::IsAttack() const { return behavior_ == Behavior::kAttack; }

// ==============================
// ノックバック要求
// ==============================
void Player::RequestKnockback(const Vector3& direction) {

	isKnockbackRequested_ = true;

	knockbackRequestDirection_ = direction;
}

// ==============================
// Knockback初期化
// ==============================
void Player::BehaviorKnockbackInitialize() {

	knockbackTimer_ = 0.0f;

	// ==============================
	// 攻撃を強制終了
	// ==============================

	combat_.Reset();

	// ==============================
	// 通常移動を停止
	// ==============================

	movement_.SetVelocity({0.0f, 0.0f, 0.0f});

	// ==============================
	// ノックバック速度
	// ==============================

	knockbackVelocity_.x = knockbackRequestDirection_.x * kKnockbackSpeed;

	knockbackVelocity_.y = 0.0f;
	knockbackVelocity_.z = 0.0f;

	UpdateWorldTransform(worldTransform_);
	worldTransform_.TransferMatrix();
}

// ==============================
// Knockback更新
// ==============================
void Player::BehaviorKnockbackUpdate() {

	constexpr float kDeltaTime = 1.0f / 60.0f;

	knockbackTimer_ += kDeltaTime;

	// ==============================
	// ノックバック移動
	// ==============================

	worldTransform_.translation_.x += knockbackVelocity_.x;

	// 徐々に減速
	knockbackVelocity_.x *= 0.85f;

	// ==============================
	// 行列更新
	// ==============================

	UpdateWorldTransform(worldTransform_);
	worldTransform_.TransferMatrix();

	// ==============================
	// ノックバック終了
	// ==============================

	if (knockbackTimer_ >= kKnockbackDuration) {

		knockbackVelocity_ = {0.0f, 0.0f, 0.0f};

		behaviorRequest_ = Behavior::kRoot;
	}
}

// ==============================
// パーツ調整用ImGui
// ==============================
void Player::DrawPartsImGui() {

#ifdef USE_IMGUI

	characterParts_.DrawImGui();

#endif
}