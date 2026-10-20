#include "AnimationController.h"

#include "CharacterParts.h"

#include <cassert>

namespace {

constexpr float kDeltaTime = 1.0f / 60.0f;

}

// ==============================
// 初期化
// ==============================
void AnimationController::Initialize(CharacterParts* characterParts) {

	assert(characterParts);

	characterParts_ = characterParts;

	currentAnimation_ = AnimationType::kNone;

	animationTime_ = 0.0f;

	duration_ = 0.0f;

	isLoop_ = false;

	isFinished_ = false;

	isPlaying_ = false;
}

// ==============================
// 更新
// ==============================
void AnimationController::Update() {

	if (!isPlaying_) {
		return;
	}

	if (currentAnimation_ == AnimationType::kNone) {

		return;
	}

	// ==============================
	// 時間を進める
	// ==============================

	animationTime_ += kDeltaTime;

	// ==============================
	// アニメーション終了
	// ==============================

	if (duration_ > 0.0f && animationTime_ >= duration_) {

		if (isLoop_) {

			animationTime_ = 0.0f;

		} else {

			animationTime_ = duration_;

			isFinished_ = true;

			isPlaying_ = false;
		}
	}

	// ==============================
	// C-1ではまだポーズを変更しない
	// ==============================

	if (characterParts_ != nullptr) {

		characterParts_->Update();
	}
}

// ==============================
// 再生
// ==============================
void AnimationController::Play(AnimationType animation, bool loop) {

	// ==============================
	// 同じアニメーションなら
	// 最初から再生し直さない
	// ==============================

	if (currentAnimation_ == animation && isPlaying_) {

		return;
	}

	currentAnimation_ = animation;

	animationTime_ = 0.0f;

	isLoop_ = loop;

	isFinished_ = false;

	isPlaying_ = true;

	// ==============================
	// 仮の再生時間
	// ==============================

	switch (animation) {

	case AnimationType::kIdle:

		duration_ = 1.0f;

		break;

	case AnimationType::kRun:

		duration_ = 0.6f;

		break;

	case AnimationType::kJump:

		duration_ = 0.5f;

		break;

	case AnimationType::kFall:

		duration_ = 0.5f;

		break;

	case AnimationType::kAttack:

		duration_ = 0.65f;

		break;

	case AnimationType::kDodge:

		duration_ = 0.4f;

		break;

	case AnimationType::kPerfectDodge:

		duration_ = 0.4f;

		break;

	case AnimationType::kCounter:

		duration_ = 0.5f;

		break;

	case AnimationType::kDamage:

		duration_ = 0.4f;

		break;

	case AnimationType::kDeath:

		duration_ = 1.0f;

		break;

	case AnimationType::kNone:
	default:

		duration_ = 0.0f;

		isPlaying_ = false;

		break;
	}
}

// ==============================
// 停止
// ==============================
void AnimationController::Stop() {

	currentAnimation_ = AnimationType::kNone;

	animationTime_ = 0.0f;

	duration_ = 0.0f;

	isLoop_ = false;

	isFinished_ = false;

	isPlaying_ = false;
}

// ==============================
// 再生中か
// ==============================
bool AnimationController::IsPlaying(AnimationType animation) const { return isPlaying_ && currentAnimation_ == animation; }