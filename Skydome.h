#pragma once
#include "KamataEngine.h"

class Skydome {
public:
	// ==============================
	// 基本処理
	// ==============================
	/// <summary>
	/// 初期化処理
	/// ゲーム開始時に一度だけ呼ばれる
	/// モデル・カメラ・プレイヤーなどの生成を行う
	/// </summary>
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera);
	/// <summary>
	/// 更新処理
	/// 毎フレーム呼ばれる
	/// 入力処理・座標更新・当たり判定などを行う
	/// </summary>
	void Update();
	/// <summary>
	/// 描画処理
	/// 毎フレーム呼ばれる
	/// 3Dモデルやプレイヤーの描画を行う
	/// </summary>
	void Draw();

private:
	//ワールド変換データ
	KamataEngine::WorldTransform worldTransform_;
	//モデル
	KamataEngine::Model* model_ = nullptr;
	// カメラ
	KamataEngine::Camera* camera_ = nullptr;
};
