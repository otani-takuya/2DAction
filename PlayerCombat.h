#pragma once

#include "KamataEngine.h"
#include "math.h"

class PlayerMovement;

class PlayerCombat {
public:
	// 攻撃フェーズ
	enum class AttackPhase {
		kChargeUp,
		kTackle,
		kAfter,
	};

public:
	void Initialize(KamataEngine::WorldTransform* playerWorldTransform, KamataEngine::Model* attackModel, KamataEngine::Camera* camera, PlayerMovement* movement);

	// 攻撃開始
	void StartAttack();

	// 攻撃更新
	void Update();

	// 攻撃描画
	void Draw();

	// 攻撃終了
	void Reset();

	// 攻撃が最後まで終了したか
	bool IsFinished() const { return isFinished_; }

	AttackPhase GetAttackPhase() const { return attackPhase_; }

private:
	// Player本体のTransform
	KamataEngine::WorldTransform* playerWorldTransform_ = nullptr;

	// 移動システム
	PlayerMovement* movement_ = nullptr;

	// 攻撃モデル
	KamataEngine::Model* attackModel_ = nullptr;

	// カメラ
	KamataEngine::Camera* camera_ = nullptr;

	// 攻撃モデルTransform
	KamataEngine::WorldTransform attackWorldTransform_;

	// 現在の攻撃フェーズ
	AttackPhase attackPhase_ = AttackPhase::kChargeUp;

	// 攻撃経過フレーム
	uint32_t attackParameter_ = 0;

	// 攻撃突進速度
	KamataEngine::Vector3 attackVelocity_ = {0.5f, 0.0f, 0.0f};

	// 攻撃終了
	bool isFinished_ = false;
};