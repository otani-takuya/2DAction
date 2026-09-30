#include "Skydome.h"

void Skydome::Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera) {
	// ワールド変換初期化
	worldTransform_.Initialize();
	// モデル生成
	model_ = model;
	// カメラ設定
	camera_ = camera;
	
}

void Skydome::Update() {
	// ワールド行列の更新
	// 本来はここで position / rotation / scale を変更する
	// 例：移動・回転・入力処理など
	// 行列をGPUに転送
	// DirectXではCPU側の行列を明示的にGPUへ送る必要がある
	worldTransform_.TransferMatrix();


}



void Skydome::Draw() {
	// ==============================
	// 安全チェック
	// ==============================
	// 初期化漏れや外部破棄に備える
	if (model_ == nullptr) {
		return;
	}

	model_->Draw(worldTransform_, *camera_);

}