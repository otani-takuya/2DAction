#include "BaseEnemy.h"

#include "GameScene.h"
#include "Player.h"

#include <cmath>
#include <numbers>

using namespace KamataEngine;

// ==============================
// 初期化
// ==============================
void BaseEnemy::Initialize(Model* model, Camera* camera, const Vector3& position) {

	// ワールド変換初期化
	worldTransform_.Initialize();

	// モデル
	model_ = model;

	// カメラ
	camera_ = camera;

	// 初期回転
	worldTransform_.rotation_.y = -std::numbers::pi_v<float> / 2.0f;

	// 初期座標
	worldTransform_.translation_ = position;

	// 初期サイズ
	worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};

	// 初期速度
	velocity_ = {-kWalkSpeed, 0.0f, 0.0f};

	// タイマー初期化
	walkTimer_ = 0.0f;
	deathTimer_ = 0.0f;

	// フラグ初期化
	isDead_ = false;
	isCollisionDisabled_ = false;

	// 最初は歩行状態
	behavior_ = Behavior::kWalk;
	behaviorRequest_ = Behavior::kUnknown;

	// ワールド行列更新
	UpdateWorldTransform(worldTransform_);
}

// ==============================
// 更新
// ==============================
void BaseEnemy::Update() {

	// ==============================
	// 状態変更
	// ==============================
	if (behaviorRequest_ != Behavior::kUnknown) {

		// リクエストされた状態に変更
		behavior_ = behaviorRequest_;

		// 状態ごとの初期化
		switch (behavior_) {

		case Behavior::kWalk:
			BehaviorWalkInitialize();
			break;

		case Behavior::kDeath:
			BehaviorDeathInitialize();
			break;

		case Behavior::kUnknown:
		default:
			break;
		}

		// リクエストをリセット
		behaviorRequest_ = Behavior::kUnknown;
	}

	// ==============================
	// 現在の状態を更新
	// ==============================
	switch (behavior_) {

	case Behavior::kWalk:
		BehaviorWalkUpdate();
		break;

	case Behavior::kDeath:
		BehaviorDeathUpdate();
		break;

	case Behavior::kUnknown:
	default:
		break;
	}
}

// ==============================
// 描画
// ==============================
void BaseEnemy::Draw() {

	// モデルまたはカメラがない場合は描画しない
	if (model_ == nullptr || camera_ == nullptr) {
		return;
	}

	// 死亡後は描画しない
	if (isDead_) {
		return;
	}

	model_->Draw(worldTransform_, *camera_);
}

// ==============================
// ワールド座標取得
// ==============================
Vector3 BaseEnemy::GetWorldPosition() const {

	Vector3 worldPosition{};

	// ワールド行列の平行移動成分を取得
	worldPosition.x = worldTransform_.matWorld_.m[3][0];
	worldPosition.y = worldTransform_.matWorld_.m[3][1];
	worldPosition.z = worldTransform_.matWorld_.m[3][2];

	return worldPosition;
}

// ==============================
// AABB取得
// ==============================
AABB BaseEnemy::GetAABB() const {

	const Vector3 worldPosition = GetWorldPosition();

	AABB aabb{};

	aabb.min = {worldPosition.x - kEnemyWidth / 2.0f, worldPosition.y - kEnemyHeight / 2.0f, worldPosition.z - kEnemyWidth / 2.0f};

	aabb.max = {worldPosition.x + kEnemyWidth / 2.0f, worldPosition.y + kEnemyHeight / 2.0f, worldPosition.z + kEnemyWidth / 2.0f};

	return aabb;
}

// ==============================
// プレイヤーとの衝突
// ==============================
void BaseEnemy::OnCollision( Player* player) {

	// プレイヤーが設定されていない場合
	if (player == nullptr) {
		return;
	}

	// すでに死亡状態なら何もしない
	if (behavior_ == Behavior::kDeath || behaviorRequest_ == Behavior::kDeath) {
		return;
	}

	// プレイヤーが攻撃中の場合
	if (player->IsAttack()) {

		// 死亡状態への変更をリクエスト
		behaviorRequest_ = Behavior::kDeath;

		// 敵とプレイヤーの中間位置を求める
		const Vector3 enemyPosition = GetWorldPosition();
		const Vector3 playerPosition = player->GetWorldPosition();

		Vector3 effectPosition{};

		effectPosition.x = (enemyPosition.x + playerPosition.x) / 2.0f;

		effectPosition.y = (enemyPosition.y + playerPosition.y) / 2.0f;

		effectPosition.z = (enemyPosition.z + playerPosition.z) / 2.0f;

		// ヒットエフェクト生成
		if (gameScene_ != nullptr) {
			gameScene_->CreateHitEffect(effectPosition);
		}
	}
}

// ==============================
// 歩行状態の初期化
// ==============================
void BaseEnemy::BehaviorWalkInitialize() {

	// 歩行タイマー初期化
	walkTimer_ = 0.0f;

	// 歩行速度を設定
	velocity_ = {-kWalkSpeed, 0.0f, 0.0f};

	// 当たり判定を有効化
	isCollisionDisabled_ = false;
}

// ==============================
// 死亡状態の初期化
// ==============================
void BaseEnemy::BehaviorDeathInitialize() {

	// 死亡演出タイマー初期化
	deathTimer_ = 0.0f;

	// 移動停止
	velocity_ = {0.0f, 0.0f, 0.0f};

	// 当たり判定を無効化
	isCollisionDisabled_ = true;
}

// ==============================
// 歩行状態の更新
// ==============================
void BaseEnemy::BehaviorWalkUpdate() {

	// 毎フレーム時間を進める
	walkTimer_ += 1.0f / 60.0f;

	// 周期でループ
	if (walkTimer_ >= kWalkMotionTime) {
		walkTimer_ = 0.0f;
	}

	// 回転アニメーション
	const float parameter = std::sin(walkTimer_);

	const float degree = kWalkMotionAngleStart + kWalkMotionAngleEnd * (parameter + 1.0f) / 2.0f;

	worldTransform_.rotation_.x = degree * std::numbers::pi_v<float> / 180.0f;

	// 移動
	worldTransform_.translation_.x += velocity_.x;
	worldTransform_.translation_.y += velocity_.y;
	worldTransform_.translation_.z += velocity_.z;

	// ワールド行列更新
	UpdateWorldTransform(worldTransform_);
}

// ==============================
// 死亡状態の更新
// ==============================
void BaseEnemy::BehaviorDeathUpdate() {

	// 死亡演出タイマー加算
	deathTimer_ += 1.0f / 30.0f;

	// Y軸回転
	worldTransform_.rotation_.y += 0.4f;

	// X軸回転
	worldTransform_.rotation_.x = -0.8f;

	// ワールド行列更新
	UpdateWorldTransform(worldTransform_);

	// 一定時間経過後に削除可能にする
	if (deathTimer_ >= 1.0f) {
		isDead_ = true;
	}
}