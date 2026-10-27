#pragma once

#include "CharacterParts.h"
#include "KamataEngine.h"

#include <string>
#include <unordered_map>

class PlayerAnimationController {

public:
	// ========================================
	// アニメーション状態
	// ========================================
	enum class AnimationState { kIdle, kWalk, kJump, kAttack };

	// ========================================
	// 基本処理
	// ========================================
	void Initialize(CharacterParts* characterParts);

	void Update(float deltaTime);

	void SetAnimationState(AnimationState state);

	AnimationState GetAnimationState() const { return currentState_; }

	// ========================================
	// ImGui
	// ========================================
	void DrawImGui();

	// Idle設定を初期値へ戻す
	void ResetIdleSettings();

	// ========================================
	// プレイヤーの移動速度を設定
	// ========================================
	void SetMovementSpeed(float speed);

	// ========================================
	// ジャンプ状態の設定
	// ========================================
	void SetJumpInfo(bool isGrounded, float velocityY);

private:
	// ========================================
	// 各パーツの姿勢
	// ========================================
	struct PartPose {

		KamataEngine::Vector3 translation = {};
		KamataEngine::Vector3 rotation = {};
		KamataEngine::Vector3 scale = {1.0f, 1.0f, 1.0f};
	};

	// パーツ名と姿勢を対応させる
	using PoseMap = std::unordered_map<std::string, PartPose>;

	// ========================================
	// アニメーション処理
	// ========================================
	void UpdateIdle();
	void UpdateWalk();

	// 現在のパーツ姿勢を取得
	PoseMap CapturePose() const;

	// 保存した姿勢をパーツに反映
	void ApplyPose(const PoseMap& pose);

	// 2つの姿勢を補間
	PoseMap BlendPose(const PoseMap& from, const PoseMap& to, float blendFactor) const;

	// ========================================
	// 管理データ
	// ========================================
	CharacterParts* characterParts_ = nullptr;

	AnimationState currentState_ = AnimationState::kIdle;

	float animationTime_ = 0.0f;

	// ========================================
	// ブレンド設定
	// ========================================

	// 遷移開始時の姿勢
	PoseMap blendStartPose_;

	// ブレンド中かどうか
	bool isBlending_ = false;

	// ブレンド経過時間
	float blendTimer_ = 0.0f;

	// ブレンド時間（秒）
	float blendDuration_ = 0.2f;

	// ========================================
	// Idleアニメーション設定
	// ========================================
	float idleSpeed_ = 2.0f;

	float idleBodyAmplitude_ = 0.04f;

	float idleHeadAmplitude_ = 0.06f;
	float idleHeadRotationAmplitude_ = 0.035f;
	float idleHeadPhaseDelay_ = 0.4f;

	float idleHandAmplitude_ = 0.025f;
	float idleHandRotationAmplitude_ = 0.05f;
	float idleHandPhaseOffset_ = 0.6f;

	float idleFootAmplitude_ = 0.005f;
	float idleFootPhaseOffset_ = 0.5f;

	// ========================================
	// Walkアニメーション設定
	// ========================================
	float walkSpeed_ = 8.0f;

	float walkBodyAmplitude_ = 0.06f;
	float walkHeadAmplitude_ = 0.04f;

	float walkHandAmplitude_ = 0.25f;
	float walkHandVerticalAmplitude_ = 0.06f;
	float walkHandRotationAmplitude_ = 0.15f;
	float walkRightHandSwingRatio_ = 0.7f;

	float walkFootAmplitude_ = 0.25f;
	float walkFootLift_ = 0.08f;

	// ========================================
	// 移動速度に応じたWalk制御
	// ========================================

	// 現在のX方向移動速度の絶対値
	float movementSpeed_ = 0.0f;

	// 歩行速度の基準値
	// PlayerMovementの最高速度と合わせる
	float walkReferenceSpeed_ = 0.15f;

	// 歩行アニメーションの最低再生倍率
	float walkMinSpeedRatio_ = 0.35f;

	// 歩行アニメーションの最大再生倍率
	float walkMaxSpeedRatio_ = 1.0f;

	// 現在の歩行周期
	float walkTime_ = 0.0f;

	// ========================================
	// 歩行アニメーション時間の更新
	// ========================================
	void UpdateWalkTime(float deltaTime);

	// ========================================
	// Walk時のBody・Headの位相
	// ========================================

	// Bodyの上下移動の位相
	float walkBodyPhaseOffset_ = 0.0f;

	// Headの上下移動の位相
	float walkHeadPhaseOffset_ = 0.3f;

	// ========================================
	// Jumpアニメーション
	// ========================================

	// ジャンプ更新
	void UpdateJump();

	// 接地状態
	bool isGrounded_ = true;

	// Y方向の移動速度
	float jumpVelocityY_ = 0.0f;

	// 上昇・落下の判定しきい値
	float jumpVelocityThreshold_ = 0.05f;

	// 上昇中の手の持ち上げ量
	float jumpHandRise_ = 0.35f;

	// 落下中の手の広がり
	float jumpHandSpread_ = 0.25f;

	// 上昇中の足の持ち上げ量
	float jumpFootRise_ = 0.20f;

	// 落下中の足の下げ量
	float jumpFootFall_ = 0.12f;

	// Bodyの上下変化
	float jumpBodyOffset_ = 0.08f;

	// Headの上下変化
	float jumpHeadOffset_ = 0.05f;
};