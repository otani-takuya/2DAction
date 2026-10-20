#include "PlayerCombat.h"
#include "PlayerMovement.h"

#include <algorithm>
#include <cassert>

using namespace KamataEngine;

// ==============================
// 初期化
// ==============================
void PlayerCombat::Initialize(WorldTransform* playerWorldTransform, Model* attackModel, Camera* camera, PlayerMovement* movement) {

	assert(playerWorldTransform);
	assert(movement);

	playerWorldTransform_ = playerWorldTransform;
	attackModel_ = attackModel;
	camera_ = camera;
	movement_ = movement;

	attackWorldTransform_.Initialize();

	attackWorldTransform_.translation_ = playerWorldTransform_->translation_;

	attackWorldTransform_.scale_ = {1.0f, 1.0f, 1.0f};

	UpdateWorldTransform(attackWorldTransform_);
	attackWorldTransform_.TransferMatrix();

	Reset();
}

// ==============================
// 攻撃開始
// ==============================
void PlayerCombat::StartAttack() {

	attackParameter_ = 0;
	attackPhase_ = AttackPhase::kChargeUp;
	isFinished_ = false;

	// 前回の変形をリセット
	playerWorldTransform_->scale_ = {1.0f, 1.0f, 1.0f};
}

// ==============================
// 攻撃更新
// ==============================
void PlayerCombat::Update() {

	if (playerWorldTransform_ == nullptr || movement_ == nullptr) {
		return;
	}

	attackParameter_++;

	Vector3 velocity{};
	float t = 0.0f;

	switch (attackPhase_) {

	// ==============================
	// 溜め
	// ==============================
	case AttackPhase::kChargeUp:

		t = static_cast<float>(attackParameter_) / 15.0f;

		t = std::clamp(t, 0.0f, 1.0f);

		playerWorldTransform_->scale_.x = 1.0f + (0.4f - 1.0f) * t;

		playerWorldTransform_->scale_.y = 1.0f + (1.7f - 1.0f) * t;

		playerWorldTransform_->scale_.z = 1.0f;

		// 向いている方向と逆へ少し下がる
		if (movement_->GetDirection() == LRDirection::kRight) {

			velocity.x = -0.03f;

		} else {

			velocity.x = 0.03f;
		}

		if (attackParameter_ >= 18) {

			attackPhase_ = AttackPhase::kTackle;

			attackParameter_ = 0;
		}

		break;

	// ==============================
	// 突進
	// ==============================
	case AttackPhase::kTackle:

		if (movement_->GetDirection() == LRDirection::kRight) {

			velocity.x = attackVelocity_.x;

		} else {

			velocity.x = -attackVelocity_.x;
		}

		t = static_cast<float>(attackParameter_) / 8.0f;

		t = std::clamp(t, 0.0f, 1.0f);

		playerWorldTransform_->scale_.x = 0.4f + (2.5f - 0.4f) * t;

		playerWorldTransform_->scale_.y = 1.7f + (0.3f - 1.7f) * t;

		playerWorldTransform_->scale_.z = 1.0f;

		// 攻撃モデルをPlayerへ追従
		attackWorldTransform_.translation_ = playerWorldTransform_->translation_;

		attackWorldTransform_.rotation_ = playerWorldTransform_->rotation_;

		attackWorldTransform_.scale_ = {1.0f, 1.0f, 1.0f};

		if (movement_->GetDirection() == LRDirection::kRight) {

			attackWorldTransform_.translation_.x += 0.6f;

		} else {

			attackWorldTransform_.translation_.x -= 0.6f;
		}

		if (attackParameter_ >= 8) {

			attackPhase_ = AttackPhase::kAfter;

			attackParameter_ = 0;
		}

		break;

	// ==============================
	// 攻撃後
	// ==============================
	case AttackPhase::kAfter:

		t = static_cast<float>(attackParameter_) / 12.0f;

		t = std::clamp(t, 0.0f, 1.0f);

		playerWorldTransform_->scale_.x = 2.5f + (1.0f - 2.5f) * t;

		playerWorldTransform_->scale_.y = 0.3f + (1.0f - 0.3f) * t;

		playerWorldTransform_->scale_.z = 1.0f;

		if (attackParameter_ >= 12) {

			playerWorldTransform_->scale_ = {1.0f, 1.0f, 1.0f};

			attackParameter_ = 0;

			isFinished_ = true;
		}

		break;
	}

	// マップとの衝突込みで移動
	movement_->MoveWithCollision(velocity);

	// Player本体
	UpdateWorldTransform(*playerWorldTransform_);
	playerWorldTransform_->TransferMatrix();

	// 攻撃モデル
	UpdateWorldTransform(attackWorldTransform_);
	attackWorldTransform_.TransferMatrix();
}

// ==============================
// 描画
// ==============================
void PlayerCombat::Draw() {

	if (attackModel_ == nullptr || camera_ == nullptr) {
		return;
	}

	if (attackPhase_ != AttackPhase::kTackle) {
		return;
	}

	attackModel_->Draw(attackWorldTransform_, *camera_);
}

// ==============================
// リセット
// ==============================
void PlayerCombat::Reset() {

	attackParameter_ = 0;
	attackPhase_ = AttackPhase::kChargeUp;
	isFinished_ = false;

	if (playerWorldTransform_ != nullptr) {

		playerWorldTransform_->scale_ = {1.0f, 1.0f, 1.0f};
	}

	attackWorldTransform_.scale_ = {1.0f, 1.0f, 1.0f};
}