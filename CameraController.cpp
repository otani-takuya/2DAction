#include "CameraController.h"
#include "player.h"
#include <algorithm>
#include "math.h"
#ifdef USE_IMGUI
#include <imgui.h>
#endif

#undef min
#undef max


void CameraController::Initialize(KamataEngine::Camera* camera) {
	camera_ = camera;
	camera_->Initialize();
}

void CameraController::Update() {
	//追従対象のワールドトランスフォームを参照
	const KamataEngine::WorldTransform* playerWorldTransform = target_->GetWorldTransform();
	//追従対象とオフセットと追従対象の速度からカメラの目標座標を計算
	cameraTargetPosition_.x = playerWorldTransform->translation_.x + targetoffset_.x + target_->GetVelocity().x * kVelocityBias;
	// Y方向はプレイヤーの位置を基準に追従
	// ジャンプ速度による大きなカメラ移動を防ぐ
	cameraTargetPosition_.y = playerWorldTransform->translation_.y + targetoffset_.y;
	cameraTargetPosition_.z = playerWorldTransform->translation_.z + targetoffset_.z + target_->GetVelocity().z * kVelocityBias;

	//座標保管によりゆったり追従
	camera_->translation_ = Lerp(camera_->translation_, cameraTargetPosition_, kInterpolationRate);

	//追従対象が画面外に出ないように補正
	camera_->translation_.x = std::max(camera_->translation_.x, playerWorldTransform->translation_.x + kCameraMovableArea.left);
	camera_->translation_.x = std::min(camera_->translation_.x, playerWorldTransform->translation_.x + kCameraMovableArea.right);
	camera_->translation_.y = std::max(camera_->translation_.y, playerWorldTransform->translation_.y + kCameraMovableArea.bottom);
	camera_->translation_.y = std::min(camera_->translation_.y, playerWorldTransform->translation_.y + kCameraMovableArea.top);


	// ==============================
	// カメラの移動範囲制限
	// ==============================
	if (isCameraLimitEnabled_) {

		camera_->translation_.x = std::clamp(camera_->translation_.x, movableArea_.left, movableArea_.right);

		camera_->translation_.y = std::clamp(camera_->translation_.y, movableArea_.bottom, movableArea_.top);
	}

	camera_->UpdateMatrix();
}

// ========================================
// カメラ位置のリセット
// ========================================
void CameraController::Reset() {

	if (camera_ == nullptr || target_ == nullptr) {
		return;
	}

	// プレイヤーのワールド座標を取得
	const KamataEngine::WorldTransform* playerWorldTransform = target_->GetWorldTransform();

	// ========================================
	// プレイヤーの位置からカメラ位置を計算
	// ========================================

	cameraTargetPosition_.x = playerWorldTransform->translation_.x + targetoffset_.x;

	cameraTargetPosition_.y = playerWorldTransform->translation_.y + targetoffset_.y;

	cameraTargetPosition_.z = playerWorldTransform->translation_.z + targetoffset_.z;

	// ========================================
	// カメラの移動制限
	// ========================================

	if (isCameraLimitEnabled_) {

		cameraTargetPosition_.x = std::clamp(cameraTargetPosition_.x, movableArea_.left, movableArea_.right);

		cameraTargetPosition_.y = std::clamp(cameraTargetPosition_.y, movableArea_.bottom, movableArea_.top);
	}

	// ========================================
	// 初期位置へ即座に移動
	// ========================================

	camera_->translation_ = cameraTargetPosition_;

	// カメラ行列を更新
	camera_->UpdateMatrix();
}

// ==============================
// カメラ調整用ImGui
// ==============================
void CameraController::DrawImGui() {

#ifdef USE_IMGUI

	ImGui::Begin("Camera Controller");

	// カメラの追従位置を調整
	ImGui::Text("Camera Offset");

	ImGui::DragFloat3("Offset XYZ", &targetoffset_.x, 0.1f);

	// ==============================
	// カメラの移動可能範囲を確認
	// ==============================
	ImGui::Separator();

	ImGui::Text("Camera Movable Area");

	ImGui::Text("X: %.2f ~ %.2f", movableArea_.left, movableArea_.right);

	ImGui::Text("Y: %.2f ~ %.2f", movableArea_.bottom, movableArea_.top);

	// 追従カメラの現在座標を表示
	if (camera_) {

		ImGui::Text("Camera Position: %.2f, %.2f, %.2f", camera_->translation_.x, camera_->translation_.y, camera_->translation_.z);
	}

	// Playerの座標を表示
	if (target_) {

		const auto* transform = target_->GetWorldTransform();

		ImGui::Text("Player Position: %.2f, %.2f, %.2f", transform->translation_.x, transform->translation_.y, transform->translation_.z);
	}

	// カメラの移動制限ON/OFF
	ImGui::Checkbox("Enable Camera Limit", &isCameraLimitEnabled_);

	ImGui::End();

#endif
}