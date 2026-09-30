#pragma once

#include "BaseEnemy.h"

class ShieldEnemy : public BaseEnemy {
public:
	~ShieldEnemy() override = default;

	// ShieldEnemy専用の更新
	void Update() override;

	// ShieldEnemy専用の衝突応答
	void OnCollision( Player* player) override;

protected:
	virtual void BehaviorWalkUpdate();

private:
	// プレイヤーが正面にいるか
	bool IsPlayerInFront(const Player* player) const;

	// のけぞり開始
	void StartGuardReaction();

	// のけぞり更新
	void UpdateGuardReaction();

private:
	// ガードによるのけぞり中か
	bool isGuardReacting_ = false;

	// のけぞり経過時間
	float guardReactionTimer_ = 0.0f;

	// のけぞり時間
	static inline const float kGuardReactionDuration = 0.3f;

	// のけぞる角度
	static inline const float kGuardReactionAngle = 25.0f;

	// 後ろへ押される速度
	static inline const float kGuardKnockBackSpeed = 0.06f;

	// ガード開始前のZ回転
	float guardStartRotationZ_ = 0.0f;

	// のけぞる方向
	float guardRotationDirection_ = 1.0f;

	// 後退方向
	KamataEngine::Vector3 knockBackDirection_ = {};
};