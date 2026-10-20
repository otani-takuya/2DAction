#pragma once

class CharacterParts;

class AnimationController {
public:
	// ==============================
	// アニメーション種類
	// ==============================
	enum class AnimationType {

		kIdle,

		kRun,

		kJump,

		kFall,

		kAttack,

		kDodge,

		kPerfectDodge,

		kCounter,

		kDamage,

		kDeath,

		kNone,
	};

public:
	// ==============================
	// 初期化
	// ==============================
	void Initialize(CharacterParts* characterParts);

	// ==============================
	// 更新
	// ==============================
	void Update();

	// ==============================
	// 再生
	// ==============================
	void Play(AnimationType animation, bool loop = true);

	// ==============================
	// 停止
	// ==============================
	void Stop();

	// ==============================
	// 再生中か
	// ==============================
	bool IsPlaying(AnimationType animation) const;

	// ==============================
	// 終了したか
	// ==============================
	bool IsFinished() const { return isFinished_; }

	AnimationType GetCurrentAnimation() const { return currentAnimation_; }

	float GetTime() const { return animationTime_; }

private:
	// キャラクターパーツ
	CharacterParts* characterParts_ = nullptr;

	// 現在のアニメーション
	AnimationType currentAnimation_ = AnimationType::kNone;

	// 再生時間
	float animationTime_ = 0.0f;

	// アニメーション時間
	float duration_ = 0.0f;

	// ループ
	bool isLoop_ = false;

	// 終了
	bool isFinished_ = false;

	// 再生中
	bool isPlaying_ = false;
};