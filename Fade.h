#pragma once
#include "KamataEngine.h"

using namespace KamataEngine;

class Fade {
public:
	// ==============================
	// 基本処理
	// ==============================

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();



	// フェードの状態
	enum class Status {
		None,    // フェードなし
		FadeIn,  // フェードイン中
		FadeOut, // フェードアウト中
	};

	//フェード開始
	void Start(Status status, float duration);

	//フェード停止
	void Stop();

	//フェード修了判定
	bool IsFinished() const;

private:
	KamataEngine::Sprite* sprite_ = nullptr;

	//現在のフェードの状態
	Status status_ = Status::None;

	//フェードの持続時間
	float duration_ = 0.0f;
	//経過時間カウンター
	float counter_ = 0.0f;

};
