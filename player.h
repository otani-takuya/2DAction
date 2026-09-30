#pragma once
#include "KamataEngine.h"
#include "MapChipField.h"
#include <algorithm>
#include "Collision.h"


enum class LRDirection {
	kRight,
	kLeft,
};

// キャラクターの当たり判定サイズ
static inline const float kPlayerWidth = 0.8f;
static inline const float kPlayerHeight = 0.8f;


//角
enum Corner {
	kRightBottom, // 右下
	kLeftBottom,  // 左下
	kRightTop,    // 右上
	kLeftTop,     // 左上

	kNumCorner // 角の数
};

struct CollisionMapInfo {
	bool isCollided = false;
	bool isGrounded = false;
	bool isCeiling = false;
	bool hitWall = false;

	KamataEngine::Vector3 blockPosition = {0.0f, 0.0f, 0.0f};
};



class MapChipField;
class BaseEnemy;
class Player {
public:
	// ==============================
	// 基本処理
	// ==============================

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(KamataEngine::Model* model, KamataEngine::Model* modelAttack,KamataEngine::Camera* camera, const KamataEngine::Vector3& position);

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	void SetMapChipField(MapChipField* mapChipField) 
	{ 
		mapChipField_ = mapChipField; 
	}

	//マップ衝突判定
	void MapChipCollision(CollisionMapInfo& info);

	//関数
	void InputMove();

	KamataEngine::Vector3 CornerPosition(const KamataEngine::Vector3& center, Corner corner);

	//マップ衝突判定上方向
	void MapChipCollisionUp(CollisionMapInfo& info);
	// マップ衝突判定下方向
	void MapChipCollisionDown(CollisionMapInfo& info);
	// マップ衝突判定右方向
	void MapChipCollisionRight(CollisionMapInfo& info);
	// マップ衝突判定左方向
	void MapChipCollisionLeft(CollisionMapInfo& info);

	void Turn();

	void Move(const CollisionMapInfo& info);

	void CeilingCollision(CollisionMapInfo& info);

	void OnGround(const CollisionMapInfo& info);

	void WallCollision(const CollisionMapInfo& info);

	LRDirection lrDirection_ = LRDirection::kRight;

	const KamataEngine::Vector3& GetVelocity() const { return velocity_; }

	// ワールドトランスフォームを取得
	const KamataEngine::WorldTransform* GetWorldTransform() const { return &worldTransform_; }

	//ワールド座標を取得
	KamataEngine::Vector3 GetWorldPosition() const;

	AABB GetAABB();

	//衝突応答
	void OnCollision(const BaseEnemy* enemy);

	// 死亡フラグの取得
	bool IsDead() const { return isDead_; }

	//通常行動更新
	void BehaviorRootUpdate();

	//攻撃行動更新
	void BehaviorAttackUpdate();

	// 振る舞い
	enum class Behavior {
		kRoot,   // 通常攻撃
		kAttack, // 攻撃中
		kKnockback, // ノックバック
		kUnknown,// 空
	};

	//振る舞い
	Behavior behavior_ = Behavior::kRoot;

	//次の振る舞いリクエスト
	Behavior behaviorRequest_ = Behavior::kUnknown;

	// 攻撃クラス(型)
	enum class AttackPhase { 
		ChargeUp, 
		Tackle, 
		After,
	};

	//現在の攻撃フェーズ
	AttackPhase nowAttackPhase_ = AttackPhase::ChargeUp;

	//通常行動初期化
	void BehaviorRootInitialize();

	// 攻撃行動初期化
	void BehaviorAttackInitialize();

	// ノックバックを要求
	void RequestKnockback(const KamataEngine::Vector3& direction);

	// ノックバック初期化
	void BehaviorKnockbackInitialize();

	// ノックバック更新
	void BehaviorKnockbackUpdate();

	//攻撃中
	bool IsAttack() const ;

private:
	// ==============================
	// ワールド変換
	// ==============================
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::WorldTransform worldTransformAttack_;

	// ==============================
	// モデル
	// ==============================
	KamataEngine::Model* model_ = nullptr;

	KamataEngine::Model* modelAttack_ = nullptr;

	// ==============================
	// カメラ
	// ==============================
	KamataEngine::Camera* camera_ = nullptr;

	KamataEngine::Vector3 velocity_ = {
	    0.0f, // x
	    0.0f, // y
	    0.0f, // z

	};

	static inline const float kAcceleration = 0.01f;

	static inline const float kAttenuation = 0.05f;

	static inline const float kLimitRumSpeed = 0.2f;

	// 旋回開始時の角度
	float trunFirstRotationY_ = 0.0f;
	//旋回タイマー
	float turnTimer_ = 0.0f;

	static inline const float kTimeTurn = 0.3f;

	//接地状態フラグ
	bool onGrounded_ = true;
	//重力加速度(下)
	static inline const float kGravityAcceleration = 0.02f;
	// 最大落下速度(下)
	static inline const float kLimitFallSpeed = -1.5f;
	// ジャンプ初速度(上)
	static inline const float kJumpAcceleration = 0.5f;

	static inline const float kBlank = 0.0f;

	//マップチップによるフィールド
	MapChipField* mapChipField_ = nullptr;

	static inline const float kAttenuationLanding = 0.2f;

	static inline const float kAttenuationWall = 0.1f;

	//死亡フラグ
	bool isDead_ = false;

	//攻撃ギミックの経過時間カウンター
	uint32_t attackParameter_ = 0;

	KamataEngine::Vector3 attackVelocity_ = {0.5f, 0.0f, 0.0f};


	// ノックバック要求が来ているか
	bool isKnockbackRequested_ = false;

	// ノックバックする方向
	KamataEngine::Vector3 knockbackRequestDirection_ = {};

	// 実際のノックバック速度
	KamataEngine::Vector3 knockbackVelocity_ = {};

	// ノックバック時間
	float knockbackTimer_ = 0.0f;

	// ノックバック継続時間
	static inline const float kKnockbackDuration = 0.25f;

	// ノックバックの強さ
	static inline const float kKnockbackSpeed = 0.5f;
};