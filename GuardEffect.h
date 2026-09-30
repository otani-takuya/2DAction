#pragma once

#include "BaseEffect.h"
#include "KamataEngine.h"

class GuardEffect : public BaseEffect {
public:
	enum class State {
		kAppear,
		kFade,
		kDead,
	};

	static GuardEffect* Create(const KamataEngine::Vector3& position);

	void Initialize(const KamataEngine::Vector3& position);

	void Update() override;
	void Draw() override;

	bool IsDead() const override { return state_ == State::kDead; }

	static void SetModel(KamataEngine::Model* model) { model_ = model; }

	static void SetCamera(KamataEngine::Camera* camera) { camera_ = camera; }

private:
	void ChangeState(State newState);

	void AppearInitialize();
	void AppearUpdate();

	void FadeInitialize();
	void FadeUpdate();

private:
	KamataEngine::WorldTransform worldTransform_;

	State state_ = State::kAppear;

	float stateTimer_ = 0.0f;

	static inline constexpr float kAppearDuration = 0.12f;
	static inline constexpr float kFadeDuration = 0.25f;

	static KamataEngine::Model* model_;
	static KamataEngine::Camera* camera_;
};