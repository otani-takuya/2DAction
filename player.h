#pragma once

#include "Collision.h"
#include "KamataEngine.h"
#include "MapChipField.h"
#include "PlayerCombat.h"
#include "PlayerMovement.h"
#include "CharacterParts.h"
#include "PlayerAnimationController.h"

class BaseEnemy;

// ==============================
// Playerが使用するモデル一覧
// ==============================
struct PlayerModels {

	KamataEngine::Model* body = nullptr;
	KamataEngine::Model* head = nullptr;

	KamataEngine::Model* leftHand = nullptr;
	KamataEngine::Model* rightHand = nullptr;

	KamataEngine::Model* leftFoot = nullptr;
	KamataEngine::Model* rightFoot = nullptr;

	KamataEngine::Model* weapon = nullptr;
};

class Player {
public:
	// ==============================
	// 基本処理
	// ==============================

	void Initialize(
		const PlayerModels& models,
		KamataEngine::Model* modelAttack,
		KamataEngine::Camera* camera, 
		const KamataEngine::Vector3& position
	);

	void Update();

	void Draw();

	// ==============================
	// パーツ調整用ImGui
	// ==============================
	void DrawPartsImGui();

	// ==============================
	// マップ
	// ==============================

	void SetMapChipField(MapChipField* mapChipField) {

		mapChipField_ = mapChipField;

		// PlayerMovementにも設定
		movement_.SetMapChipField(mapChipField);
	}

	// ==============================
	// Getter
	// ==============================

	const KamataEngine::Vector3& GetVelocity() const { return movement_.GetVelocity(); }

	const KamataEngine::WorldTransform* GetWorldTransform() const { return &worldTransform_; }

	KamataEngine::Vector3 GetWorldPosition() const;

	AABB GetAABB();

	bool IsDead() const { return isDead_; }

	// 攻撃中か
	bool IsAttack() const;

	// ==============================
	// 衝突
	// ==============================

	void OnCollision(const BaseEnemy* enemy);

	// ==============================
	// ノックバック
	// ==============================

	void RequestKnockback(const KamataEngine::Vector3& direction);

	// ========================================
	// アニメーション設定のImGui表示
	// ========================================
	void DrawAnimationImGui() {

#ifdef USE_IMGUI
		animationController_.DrawImGui();
#endif
	}

private:
	// ==============================
	// Playerの状態
	// ==============================

	enum class Behavior {
		kRoot,
		kAttack,
		kKnockback,
		kUnknown,
	};

	Behavior behavior_ = Behavior::kRoot;

	Behavior behaviorRequest_ = Behavior::kUnknown;

	// ==============================
	// Root
	// ==============================

	void BehaviorRootInitialize();
	void BehaviorRootUpdate();

	// ==============================
	// Attack
	// ==============================

	void BehaviorAttackInitialize();
	void BehaviorAttackUpdate();

	// ==============================
	// Knockback
	// ==============================

	void BehaviorKnockbackInitialize();
	void BehaviorKnockbackUpdate();

private:
	// ==============================
	// Player本体
	// ==============================

	// Player全体の位置・回転・拡大縮小
	KamataEngine::WorldTransform worldTransform_;

	// Playerの分割パーツ
	CharacterParts characterParts_;

	// 描画用カメラ
	KamataEngine::Camera* camera_ = nullptr;

	// ==============================
	// 移動
	// ==============================

	PlayerMovement movement_;

	// ==============================
	// 戦闘
	// ==============================

	PlayerCombat combat_;

	// ==============================
	// マップ
	// ==============================

	MapChipField* mapChipField_ = nullptr;

	// ==============================
	// 死亡
	// ==============================

	bool isDead_ = false;

	// ==============================
	// ノックバック
	// ==============================

	bool isKnockbackRequested_ = false;

	KamataEngine::Vector3 knockbackRequestDirection_ = {};

	KamataEngine::Vector3 knockbackVelocity_ = {};

	float knockbackTimer_ = 0.0f;

	static inline constexpr float kKnockbackDuration = 0.25f;

	static inline constexpr float kKnockbackSpeed = 0.5f;

	// ========================================
	// Playerのアニメーション管理
	// ========================================
	PlayerAnimationController animationController_;


};