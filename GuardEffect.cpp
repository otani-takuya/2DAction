#include "GuardEffect.h"

#include "math.h"

#include <algorithm>
#include <cassert>
#include <numbers>

using namespace KamataEngine;

// ==============================
// 静的メンバ変数
// ==============================
Model* GuardEffect::model_ = nullptr;
Camera* GuardEffect::camera_ = nullptr;

// ==============================
// 生成
// ==============================
GuardEffect* GuardEffect::Create(const Vector3& position) {

	GuardEffect* instance = new GuardEffect();
	assert(instance);

	instance->Initialize(position);

	return instance;
}

// ==============================
// 初期化
// ==============================
void GuardEffect::Initialize(const Vector3& position) {

	worldTransform_.Initialize();

	worldTransform_.translation_ = position;

	// 初めは小さくする
	worldTransform_.scale_ = {0.05f, 0.05f, 0.05f};

	UpdateWorldTransform(worldTransform_);

	state_ = State::kAppear;

	AppearInitialize();
}

// ==============================
// 状態変更
// ==============================
void GuardEffect::ChangeState(State newState) {

	state_ = newState;

	switch (state_) {

	case State::kAppear:
		AppearInitialize();
		break;

	case State::kFade:
		FadeInitialize();
		break;

	case State::kDead:
	default:
		break;
	}
}

// ==============================
// 更新
// ==============================
void GuardEffect::Update() {

	switch (state_) {

	case State::kAppear:
		AppearUpdate();
		break;

	case State::kFade:
		FadeUpdate();
		break;

	case State::kDead:
	default:
		break;
	}
}

// ==============================
// 出現初期化
// ==============================
void GuardEffect::AppearInitialize() { stateTimer_ = 0.0f; }

// ==============================
// 出現更新
// ==============================
void GuardEffect::AppearUpdate() {

	constexpr float kDeltaTime = 1.0f / 60.0f;

	stateTimer_ += kDeltaTime;

	float t = stateTimer_ / kAppearDuration;
	t = std::clamp(t, 0.0f, 1.0f);

	// 滑らかなイージング
	const float easedT = t * t * (3.0f - 2.0f * t);

	// 小さい状態から一気に拡大
	const float scale = 0.05f + (1.7f - 0.05f) * easedT;

	worldTransform_.scale_ = {scale, scale, scale};

	// 少し回転させる
	worldTransform_.rotation_.z += 0.12f;

	UpdateWorldTransform(worldTransform_);

	if (stateTimer_ >= kAppearDuration) {
		ChangeState(State::kFade);
	}
}

// ==============================
// フェード初期化
// ==============================
void GuardEffect::FadeInitialize() { stateTimer_ = 0.0f; }

// ==============================
// フェード更新
// ==============================
void GuardEffect::FadeUpdate() {

	constexpr float kDeltaTime = 1.0f / 60.0f;

	stateTimer_ += kDeltaTime;

	float t = stateTimer_ / kFadeDuration;
	t = std::clamp(t, 0.0f, 1.0f);

	const float easedT = t * t * (3.0f - 2.0f * t);

	// 1.3から0へ縮小
	const float scale = 1.3f * (1.0f - easedT);

	worldTransform_.scale_ = {scale, scale, scale};

	// フェード中も回転
	worldTransform_.rotation_.z += 0.08f;

	UpdateWorldTransform(worldTransform_);

	if (stateTimer_ >= kFadeDuration) {
		ChangeState(State::kDead);
	}
}

// ==============================
// 描画
// ==============================
void GuardEffect::Draw() {

	if (state_ == State::kDead) {
		return;
	}

	if (model_ == nullptr || camera_ == nullptr) {
		return;
	}

	model_->Draw(worldTransform_, *camera_);
}