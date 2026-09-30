#include "CameraController.h"
#include "player.h"
#include <algorithm>
#include "math.h"

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
	cameraTargetPosition_.y = playerWorldTransform->translation_.y + targetoffset_.y + target_->GetVelocity().y * kVelocityBias;
	cameraTargetPosition_.z = playerWorldTransform->translation_.z + targetoffset_.z + target_->GetVelocity().z * kVelocityBias;

	//座標保管によりゆったり追従
	camera_->translation_ = Lerp(camera_->translation_, cameraTargetPosition_, kInterpolationRate);

	//追従対象が画面外に出ないように補正
	camera_->translation_.x = std::max(camera_->translation_.x, playerWorldTransform->translation_.x + kCameraMovableArea.left);
	camera_->translation_.x = std::min(camera_->translation_.x, playerWorldTransform->translation_.x + kCameraMovableArea.right);
	camera_->translation_.y = std::max(camera_->translation_.y, playerWorldTransform->translation_.y + kCameraMovableArea.bottom);
	camera_->translation_.y = std::min(camera_->translation_.y, playerWorldTransform->translation_.y + kCameraMovableArea.top);


	// カメラ移動範囲の制限
	camera_->translation_.x = std::clamp(camera_->translation_.x, movableArea_.left, movableArea_.right);
	camera_->translation_.y = std::clamp(camera_->translation_.y, movableArea_.bottom, movableArea_.top);
	

	camera_->UpdateMatrix();
}

void CameraController::Reset() { 
	const KamataEngine::WorldTransform* playerWorldTransform = target_->GetWorldTransform(); 
	
	camera_->translation_.x = playerWorldTransform->translation_.x + targetoffset_.x;
	camera_->translation_.y = playerWorldTransform->translation_.y + targetoffset_.y;
	camera_->translation_.z = playerWorldTransform->translation_.z + targetoffset_.z;

	camera_->UpdateMatrix();
}