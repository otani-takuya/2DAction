#include "HitEffect.h"
#include "math.h"

#include <cassert>
#include <cmath>
#include <numbers>
#include <random>

using namespace KamataEngine;

// ==============================
// 静的メンバ変数の実体
// ==============================
Model* HitEffect::model_ = nullptr;
Camera* HitEffect::camera_ = nullptr;

// ==============================
// 生成
// ==============================
HitEffect* HitEffect::Create(const Vector3& position) {

	HitEffect* instance = new HitEffect();
	assert(instance);

	instance->Initialize(position);

	return instance;
}

// ==============================
// 初期化
// ==============================
void HitEffect::Initialize(const Vector3& position) {

	// ==============================
	// 乱数生成器
	// ==============================
	static std::random_device seedGenerator;
	static std::mt19937 randomEngine(seedGenerator());

	// 0～360度のランダム角度
	std::uniform_real_distribution<float> rotationDistribution(0.0f, std::numbers::pi_v<float> * 2.0f);

	// ==============================
	// 円形エフェクト初期化
	// ==============================
	circleWorldTransform_.Initialize();

	circleWorldTransform_.translation_ = position;



	UpdateWorldTransform(circleWorldTransform_);

	// ==============================
	// 楕円エフェクト初期化
	// ==============================
	for (size_t i = 0; i < ellipseWorldTransforms_.size(); ++i) {

		WorldTransform& worldTransform = ellipseWorldTransforms_[i];

		worldTransform.Initialize();

		// 発生位置
		worldTransform.translation_ = position;

		// 細長い形
		worldTransform.scale_ = {0.1f, 2.0f, 1.0f};

		// ランダムな角度
		float angle = rotationDistribution(randomEngine);

		// Z軸回転
		worldTransform.rotation_.z = angle;

		UpdateWorldTransform(worldTransform);
	}

	// 最初はスプレッド状態
	state_ = State::kSpread;
	SpreadInitialize();
}

// ==============================
// 状態変更
// ==============================
void HitEffect::ChangeState(State newState) {

	state_ = newState;

	switch (state_) {
	case State::kSpread:
		SpreadInitialize();
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
void HitEffect::Update() {

	switch (state_) {
	case State::kSpread:
		SpreadUpdate();
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
// スプレッド初期化
// ==============================
void HitEffect::SpreadInitialize() { stateTimer_ = 0.0f; }

// ==============================
// スプレッド更新
// ==============================
void HitEffect::SpreadUpdate() {

	constexpr float kDeltaTime = 1.0f / 60.0f;

	stateTimer_ += kDeltaTime;

	// 0.0～1.0
	float t = stateTimer_ / kSpreadDuration;

	if (t > 1.0f) {
		t = 1.0f;
	}

	// 滑らかな拡大
	const float easedT = t * t * (3.0f - 2.0f * t);

	// 円形エフェクトを拡大
	const float circleScale = 0.05f + (1.0f - 0.05f) * easedT;

	circleWorldTransform_.scale_ = {circleScale, circleScale, circleScale};

	UpdateWorldTransform(circleWorldTransform_);

	// 拡大終了
	if (stateTimer_ >= kSpreadDuration) {
		ChangeState(State::kFade);
	}
}

// ==============================
// フェード初期化
// ==============================
void HitEffect::FadeInitialize() { stateTimer_ = 0.0f; }

// ==============================
// フェード更新
// ==============================
void HitEffect::FadeUpdate() {

	constexpr float kDeltaTime = 1.0f / 60.0f;

	stateTimer_ += kDeltaTime;

	float t = stateTimer_ / kFadeDuration;

	if (t > 1.0f) {
		t = 1.0f;
	}

	// イージング
	float easedT = t * t * (3.0f - 2.0f * t);

	// イージング後の値を使う
	float fadeScale = 1.0f - easedT;

	// 円形を徐々に小さくする
	circleWorldTransform_.scale_ = {
		2.0f * fadeScale, 
		2.0f * fadeScale, 
		2.0f * fadeScale
	};

	UpdateWorldTransform(circleWorldTransform_);
	
	// フェード終了
	if (stateTimer_ >= kFadeDuration) {
		ChangeState(State::kDead);
	}
}

// ==============================
// 描画
// ==============================
void HitEffect::Draw() {

	if (state_ == State::kDead) {
		return;
	}

	if (model_ == nullptr || camera_ == nullptr) {
		return;
	}

	// 円形エフェクト
	model_->Draw(circleWorldTransform_, *camera_);

	// 楕円エフェクト
	for (WorldTransform& worldTransform : ellipseWorldTransforms_) {

		model_->Draw(worldTransform, *camera_);
	}
}