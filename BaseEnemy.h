#pragma once

#include "Collision.h"
#include "KamataEngine.h"
#include "math.h"

class MapChipField;
class Player;
class GameScene;

class BaseEnemy {
public:
	// BaseEnemyポインタから子クラスをdeleteするために必須
	virtual ~BaseEnemy() = default;

	// 初期化
	virtual void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position);

	// 更新
	virtual void Update();

	// 描画
	virtual void Draw();

	void SetMapChipField(MapChipField* mapChipField) { mapChipField_ = mapChipField; }

	void SetGameScene(GameScene* gameScene) { gameScene_ = gameScene; }

	KamataEngine::Vector3 GetWorldPosition() const;

	AABB GetAABB() const;

	virtual void OnCollision(Player* player);

	bool IsDead() const { return isDead_; }

	bool IsCollisionDisabled() const { return isCollisionDisabled_; }

protected:
	enum class Behavior {
		kWalk,
		kDeath,
		kUnknown,
	};

	virtual void BehaviorWalkInitialize();
	virtual void BehaviorDeathInitialize();

	virtual void BehaviorWalkUpdate();
	virtual void BehaviorDeathUpdate();

	KamataEngine::WorldTransform worldTransform_;

	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Camera* camera_ = nullptr;

	MapChipField* mapChipField_ = nullptr;
	GameScene* gameScene_ = nullptr;

	static inline const float kEnemyWidth = 0.8f;
	static inline const float kEnemyHeight = 0.8f;

	static inline const float kWalkSpeed = 0.05f;

	KamataEngine::Vector3 velocity_ = {};

	float walkTimer_ = 0.0f;
	float deathTimer_ = 0.0f;

	static inline const float kWalkMotionAngleStart = -45.0f;
	static inline const float kWalkMotionAngleEnd = 100.0f;
	static inline const float kWalkMotionTime = 6.0f;

	bool isDead_ = false;
	bool isCollisionDisabled_ = false;

	Behavior behavior_ = Behavior::kWalk;
	Behavior behaviorRequest_ = Behavior::kUnknown;
};