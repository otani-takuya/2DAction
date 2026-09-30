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


private:
	//カメラ
	KamataEngine::Camera* camera_ = nullptr;

	//追従ターゲット
	Player* target_ = nullptr;

	//オフセット
	KamataEngine::Vector3 targetoffset_ = {0.0f, 0.0f, -20.0f};

	//カメラ移動範囲
	Rect movableArea_ = {0.0f, 100.0f, 0.0f, 100.0f};

	//カメラ目標座標
	KamataEngine::Vector3 cameraTargetPosition_ = {};

	//座標保管割合
	static inline const float kInterpolationRate = 0.1f;

	//速度掛け率
	static inline const float kVelocityBias = 30.0f;

	//追従対象の各方向へのカメラ移動範囲
	static inline const Rect kCameraMovableArea = {
	    .left = -14.0f,
	    .right = 85.0f,
	    .bottom = -8.0f,
	    .top = 11.0f,
	};
};
