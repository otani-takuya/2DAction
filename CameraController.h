#include <KamataEngine.h>

#pragma once

// Playerクラスの宣言
class Player;

class CameraController {
public:
	// 矩形
	struct Rect {
		float left = 0.0f;
		float right = 1.0f;
		float bottom = 0.0f;
		float top = 1.0f;
	};


	//初期化
	void Initialize(KamataEngine::Camera* camera);
	//更新
	void Update();

	// 追従ターゲットの設定するセッター
	void Settarget(Player* target) { target_ = target; }

	//リセット
	void Reset();

	void SetMovableArea(const Rect& area) { movableArea_ = area; }

	// ==============================
	// ImGuiでカメラを調整
	// ==============================
	void DrawImGui();


private:
	//カメラ
	KamataEngine::Camera* camera_ = nullptr;

	//追従ターゲット
	Player* target_ = nullptr;

	//オフセット
	//  ========================================
	// プレイヤーに対するカメラの位置
	// ========================================
	KamataEngine::Vector3 targetoffset_ = {

	    2.0f,  // X：プレイヤーを画面左に配置
	    3.5f,  // Y：プレイヤーより少し上
	    -12.0f // Z：カメラとプレイヤーの距離
	};

	// カメラ移動範囲
	Rect movableArea_ = {
	    -100.0f, // X座標の最小値
	    100.0f,  // X座標の最大値
	    -100.0f, // Y座標の最小値
	    100.0f   // Y座標の最大値
	};

	//カメラ目標座標
	KamataEngine::Vector3 cameraTargetPosition_ = {};

	// ========================================
	// カメラ追従パラメータ
	// ========================================

	// 追従の補間率
	static inline const float kInterpolationRate = 0.05f;

	// 移動速度による先読み補正
	static inline const float kVelocityBias = 0.0f;

	//追従対象の各方向へのカメラ移動範囲
	static inline const Rect kCameraMovableArea = {
	    .left = -14.0f,
	    .right = 85.0f,
	    .bottom = -100.0f,
	    .top = 100.0f,
	};

	// カメラの移動範囲制限を使用するか
	bool isCameraLimitEnabled_ = true;
};
