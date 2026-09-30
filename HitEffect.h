#pragma once

#include "BaseEffect.h"
#include "KamataEngine.h"

#include <array>

class HitEffect : public BaseEffect {
public:
	// ==============================
	// 状態
	// ==============================
	enum class State {
		kSpread,
		kFade,
		kDead,
	};

	// 生成＋初期化
	static HitEffect* Create(const KamataEngine::Vector3& position);

	void Initialize(const KamataEngine::Vector3& position);

	void Update() override;
	void Draw() override;

	bool IsDead() const override { return state_ == State::kDead; }

	static void SetModel(KamataEngine::Model* model) { model_ = model; }

	static void SetCamera(KamataEngine::Camera* camera) { camera_ = camera; }

private:
	void SpreadInitialize();
	void SpreadUpdate();

	void FadeInitialize();
	void FadeUpdate();

	void ChangeState(State newState);

private:
	static inline constexpr size_t kEllipseCount = 2;

	KamataEngine::WorldTransform circleWorldTransform_;

	std::array<KamataEngine::WorldTransform, kEllipseCount> ellipseWorldTransforms_;

	State state_ = State::kSpread;

	float stateTimer_ = 0.0f;

	static inline constexpr float kSpreadDuration = 0.15f;
	static inline constexpr float kFadeDuration = 0.20f;

	static KamataEngine::Model* model_;
	static KamataEngine::Camera* camera_;
};