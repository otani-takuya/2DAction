#pragma once
#include "KamataEngine.h"
#include "Fade.h"


class TitleScene {
public:
	// ==============================
	// 基本処理
	// ==============================

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~TitleScene();


	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	// デスフラグのgetter
	bool IsFinished() const { return finished_; }


private:
	// ==============================
	// モデル
	// ==============================
	KamataEngine::Model* titleModel_ = nullptr;
	KamataEngine::WorldTransform titleWorldTransform;
	static inline const KamataEngine::Vector3 titlePosition = {0.0f, 4.0f, -30.0f};

	KamataEngine::Model* playerModel_ = nullptr;
	KamataEngine::WorldTransform playerWorldTransform;
	static inline const KamataEngine::Vector3 playerPosition = {0.0f, -1.0f, -40.0f};

	// ==============================
	// カメラ
	// ==============================
	KamataEngine::Camera camera_;

	//タイトル上下用タイマー
	float upDownTimer_ = 0.0f ;

	//終了フラグ
	bool finished_ = false;


	//フェード
	Fade* fade_ = nullptr;

	const float kFadeDuration = 1.0f;

	// シーンのフェーズ
	enum class Phase {
		kFadeIn,  // フェードイン
		kMain,    // メイン
		kFadeOut, // フェードアウト
	};

	// 現在のフェーズ
	Phase phase_ = Phase::kFadeIn;
};
