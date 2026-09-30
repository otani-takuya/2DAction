#include "ShieldEnemy.h"

#include "GameScene.h"
#include "Player.h"

#include <algorithm>
#include <cmath>
#include <numbers>

using namespace KamataEngine;

// ==============================
// 更新
// ==============================
void ShieldEnemy::Update() {

	// のけぞり中
	if (isGuardReacting_) {

		UpdateGuardReaction();

		// のけぞり中は通常の歩行処理を実行しない
		return;
	}

	// 通常処理はBaseEnemyに任せる
	BaseEnemy::Update();
}

// ==============================
// プレイヤーとの衝突
// ==============================
void ShieldEnemy::OnCollision(Player* player) {

	if (player == nullptr) {
		return;
	}

	// 攻撃中でなければ何もしない
	if (!player->IsAttack()) {
		return;
	}

	// すでに死亡状態なら何もしない
	if (behavior_ == Behavior::kDeath || behaviorRequest_ == Behavior::kDeath) {
		return;
	}

	// 正面から攻撃された
	if (IsPlayerInFront(player)) {

		// ガードリアクション中は再発動しない
		if (isGuardReacting_) {
			return;
		}

		const Vector3 enemyPosition = GetWorldPosition();
		const Vector3 playerPosition = player->GetWorldPosition();

		// ヒットエフェクト
		if (gameScene_ != nullptr) {

			Vector3 effectPosition{};

			effectPosition.x = (enemyPosition.x + playerPosition.x) / 2.0f;

			effectPosition.y = (enemyPosition.y + playerPosition.y) / 2.0f;

			effectPosition.z = (enemyPosition.z + playerPosition.z) / 2.0f;

			gameScene_->CreateGuardEffect(effectPosition);
		}

		// ShieldEnemyからプレイヤーを遠ざける方向
		Vector3 knockbackDirection{};

		knockbackDirection.x = playerPosition.x - enemyPosition.x;

		// X方向だけ正規化
		if (knockbackDirection.x < 0.0f) {
			knockbackDirection.x = -1.0f;
		} else {
			knockbackDirection.x = 1.0f;
		}

		// プレイヤーへノックバック要求
		player->RequestKnockback(knockbackDirection);

		// ShieldEnemyのガードリアクション
		StartGuardReaction();

		return;
	}

	// 背後からの攻撃は通常どおり死亡
	BaseEnemy::OnCollision(player);
}

// ==============================
// プレイヤーが正面にいるか
// ==============================
bool ShieldEnemy::IsPlayerInFront(const Player* player) const {

	if (player == nullptr) {
		return false;
	}

	const Vector3 enemyPosition = GetWorldPosition();
	const Vector3 playerPosition = player->GetWorldPosition();

	// 敵からプレイヤーへの方向
	Vector3 toPlayer{playerPosition.x - enemyPosition.x, 0.0f, 0.0f};

	// 横スクロールなのでX方向だけで判定
	//
	// ShieldEnemyは-X方向を向いている
	// プレイヤーが敵より左にいれば正面
	return toPlayer.x < 0.0f;
}

// ==============================
// ガードリアクション開始
// ==============================
void ShieldEnemy::StartGuardReaction() {

	isGuardReacting_ = true;
	guardReactionTimer_ = 0.0f;

	// 現在の角度を保存
	guardStartRotationZ_ = worldTransform_.rotation_.z;

	// ShieldEnemyの正面方向
	Vector3 enemyForward{std::sin(worldTransform_.rotation_.y), 0.0f, std::cos(worldTransform_.rotation_.y)};

	// 後退方向は正面方向の逆
	knockBackDirection_ = {-enemyForward.x, 0.0f, -enemyForward.z};

	// 正面が左向きなら右側へ傾けるなど、
	// 敵の向きに応じて傾く方向を変更
	if (enemyForward.x < 0.0f) {
		guardRotationDirection_ = 1.0f;
	} else {
		guardRotationDirection_ = -1.0f;
	}

	// 通常移動を止める
	velocity_ = {0.0f, 0.0f, 0.0f};
}

// ==============================
// ガードリアクション更新
// ==============================
void ShieldEnemy::UpdateGuardReaction() {

	constexpr float kDeltaTime = 1.0f / 60.0f;

	guardReactionTimer_ += kDeltaTime;

	float t = guardReactionTimer_ / kGuardReactionDuration;

	t = std::clamp(t, 0.0f, 1.0f);

	// 0 → 1 → 0になる値
	// 前半でのけぞり、後半で元に戻る
	const float reaction = std::sin(t * std::numbers::pi_v<float>);

	// 角度をラジアンへ変換
	const float reactionAngle = kGuardReactionAngle * std::numbers::pi_v<float> / 180.0f;

	// 後ろにのけぞる
	worldTransform_.rotation_.z = guardStartRotationZ_ + reactionAngle * reaction * guardRotationDirection_;

	// 攻撃された直後ほど大きく後退させる
	const float movePower = 1.0f - t;

	worldTransform_.translation_.x += knockBackDirection_.x * kGuardKnockBackSpeed * movePower;

	UpdateWorldTransform(worldTransform_);

	// のけぞり終了
	if (t >= 1.0f) {

		isGuardReacting_ = false;
		guardReactionTimer_ = 0.0f;

		// 傾きを完全に元へ戻す
		worldTransform_.rotation_.z = guardStartRotationZ_;

		// 再び歩行状態にする
		BehaviorWalkInitialize();

		UpdateWorldTransform(worldTransform_);
	}
}

// ==============================
// ShieldEnemy専用の歩行更新
// ==============================
void ShieldEnemy::BehaviorWalkUpdate() {

	walkTimer_ += 1.0f / 60.0f;

	if (walkTimer_ >= kWalkMotionTime) {
		walkTimer_ = 0.0f;
	}

	const float param = std::sin(walkTimer_);

	// Y軸で揺らす角度
	const float degree = kWalkMotionAngleStart + kWalkMotionAngleEnd * (param + 1.0f) / 2.0f;

	const float baseRotationY = -std::numbers::pi_v<float> / 2.0f;

	const float motionRotationY = degree * std::numbers::pi_v<float> / 180.0f;

	// 左向きの基準角度 + 歩行アニメーション
	worldTransform_.rotation_.y = baseRotationY + motionRotationY;

	worldTransform_.translation_.x += velocity_.x;
	worldTransform_.translation_.y += velocity_.y;

	UpdateWorldTransform(worldTransform_);
}