#include "DeathParticles.h"
#include "math.h"
#include "KamataEngine.h"
#include <algorithm>

void DeathParticles::Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position) {
	// ワールド変換初期化
	for (KamataEngine::WorldTransform& worldTransform : worldTransforms_) {
		worldTransform.Initialize();
		worldTransform.translation_ = position;
	}
	
	// モデル
	model_ = model;

	// カメラ
	camera_ = camera;

	objectColor_.Initialize();
	color_ = {1.0f, 1.0f, 1.0f, 1.0f}; // 初期色を白に設定
}

void DeathParticles::Update() {
	//終了なら何もしない
	if (isFinished_) {
		return;
	}
	//カウンターを1フレーム分の秒数を進める
	counter_ += 1.0f / 60.0f;

	//存続時間の上限を超えたら
	if (counter_ >= kDuration) {
		counter_ = kDuration;
		//終了扱い
		isFinished_ = true;
	}


	for (uint32_t i = 0; i < kNumParticles; ++i) {
		//基本となる速度ベクトル
		KamataEngine::Vector3 velocity = {kSpeed, 0, 0};
		//回転角を計算する
		float angle = kAngleUnit * i;
		//z軸まわり回転行列
		KamataEngine::Matrix4x4 matrixRotation = KamataEngine::MathUtility::MakeRotateZMatrix(angle);
		//基本ベクトルを回転させて速度ベクトルを得る
		velocity = KamataEngine::MathUtility::Transform(velocity, matrixRotation);
		//移動処理
		worldTransforms_[i].translation_.x += velocity.x;
		worldTransforms_[i].translation_.y += velocity.y;
		worldTransforms_[i].translation_.z += velocity.z;
	}

	color_.w = std::clamp(1.0f - (counter_ / kDuration), 0.0f, 1.0f);
	//色変更オブジェクトに色の数値を設定する
	objectColor_.SetColor(color_);


	// ワールド座標の更新。アフィン変換
	for (KamataEngine::WorldTransform& worldTransform : worldTransforms_) {
		UpdateWorldTransform(worldTransform); 
	}
}

void DeathParticles::Draw() {
	// 終了なら何もしない
	if (isFinished_) {
		return;
	}
	// パーティクル描画
	for (KamataEngine::WorldTransform& worldTransform : worldTransforms_) {
		model_->Draw(worldTransform, *camera_, &objectColor_);
	}
}
