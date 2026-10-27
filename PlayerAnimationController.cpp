#include "PlayerAnimationController.h"

#include <algorithm>
#include <cassert>
#include <cmath>

#ifdef USE_IMGUI
#include <imgui.h>
#endif

// ========================================
// 初期化
// ========================================
void PlayerAnimationController::Initialize(CharacterParts* characterParts) {

	// NULLチェック
	assert(characterParts != nullptr);

	characterParts_ = characterParts;

	// 初期状態はIdle
	currentState_ = AnimationState::kIdle;

	// アニメーション時間を初期化
	animationTime_ = 0.0f;
}

// ========================================
// アニメーション状態変更
// ========================================
void PlayerAnimationController::SetAnimationState(AnimationState state) {

	// 同じ状態なら変更しない
	if (currentState_ == state) {
		return;
	}

	// ========================================
	// 遷移開始時の姿勢を保存
	// ========================================

	blendStartPose_ = CapturePose();

	// ========================================
	// 状態を変更
	// ========================================

	currentState_ = state;

	// 新しいアニメーションの時間をリセット
	animationTime_ = 0.0f;

	// ========================================
	// ブレンド開始
	// ========================================

	blendTimer_ = 0.0f;

	isBlending_ = true;
}

// ========================================
// アニメーション更新
// ========================================
void PlayerAnimationController::Update(float deltaTime) {

	if (characterParts_ == nullptr) {
		return;
	}

	// ========================================
	// アニメーション時間更新
	// ========================================

	animationTime_ += deltaTime;

	// ========================================
	// Walkアニメーション時間更新
	// ========================================
	if (currentState_ == AnimationState::kWalk) {

		UpdateWalkTime(deltaTime);
	}

	// ========================================
	// 基準姿勢へ戻す
	// ========================================

	characterParts_->ResetPose();

	// ========================================
	// 現在のアニメーションを計算
	// ========================================

	switch (currentState_) {

	case AnimationState::kIdle:

		UpdateIdle();
		break;

	case AnimationState::kWalk:

		UpdateWalk();
		break;

	case AnimationState::kJump:

		UpdateJump();
		break;

	case AnimationState::kAttack:
		break;
	}

	// ========================================
	// アニメーション計算後の姿勢を取得
	// ========================================

	const PoseMap targetPose = CapturePose();

	// ========================================
	// ブレンド処理
	// ========================================

	if (isBlending_) {

		blendTimer_ += deltaTime;

		// ブレンド率
		float blendFactor = 1.0f;

		if (blendDuration_ > 0.0f) {

			blendFactor = blendTimer_ / blendDuration_;
		}

		blendFactor = (std::clamp)(blendFactor, 0.0f, 1.0f);

		// 滑らかな補間（SmoothStep）
		const float smoothFactor = blendFactor * blendFactor * (3.0f - 2.0f * blendFactor);

		// ========================================
		// 姿勢を補間
		// ========================================

		const PoseMap blendedPose = BlendPose(blendStartPose_, targetPose, smoothFactor);

		// 最終姿勢を反映
		ApplyPose(blendedPose);

		// ========================================
		// ブレンド終了判定
		// ========================================

		if (blendFactor >= 1.0f) {

			isBlending_ = false;
		}
	}
}

// ========================================
// Idleアニメーション
// BodyとHeadを連動させる
// ========================================
void PlayerAnimationController::UpdateIdle() {

	// ==============================
	// パーツを取得
	// ==============================

	CharacterParts::Part* body = characterParts_->GetPart("Body");

	CharacterParts::Part* head = characterParts_->GetPart("Head");

	// ==============================
	// アニメーション時間
	// ==============================

	// sin波に使用する時間
	const float time = animationTime_ * idleSpeed_;

	// ==============================
	// Bodyの上下移動
	// ==============================

	if (body != nullptr) {

		// Bodyの上下移動量
		const float bodyOffsetY = std::sin(time) * idleBodyAmplitude_;

		// 初期位置を基準に上下移動
		body->worldTransform.translation_.y = body->baseTranslation.y + bodyOffsetY;
	}

	// ==============================
	// Headの上下移動・回転
	// ==============================

	if (head != nullptr) {

		// Bodyより少し遅れた時間
		const float headTime = time - idleHeadPhaseDelay_;

		// Headの上下移動量
		const float headOffsetY = std::sin(headTime) * idleHeadAmplitude_;

		// Headの傾き
		const float headRotationZ = std::sin(headTime) * idleHeadRotationAmplitude_;

		// 初期位置を基準に上下移動
		head->worldTransform.translation_.y = head->baseTranslation.y + headOffsetY;

		// 初期角度を基準に回転
		head->worldTransform.rotation_.z = head->baseRotation.z + headRotationZ;
	}

	// ========================================
	// 左右の手のIdleアニメーション
	// ========================================

	// 左右の手を取得
	CharacterParts::Part* leftHand = characterParts_->GetPart("LeftHand");

	CharacterParts::Part* rightHand = characterParts_->GetPart("RightHand");

	// 左手のアニメーション
	if (leftHand != nullptr) {

		// 左手用の時間
		const float leftHandTime = time + idleHandPhaseOffset_;

		// 上下移動
		const float offsetY = std::sin(leftHandTime) * idleHandAmplitude_;

		// 回転
		const float rotationZ = std::sin(leftHandTime) * idleHandRotationAmplitude_;

		// 初期姿勢を基準に移動
		leftHand->worldTransform.translation_.y = leftHand->baseTranslation.y + offsetY;

		// 初期角度を基準に回転
		leftHand->worldTransform.rotation_.z = leftHand->baseRotation.z + rotationZ;
	}

	// 右手のアニメーション
	if (rightHand != nullptr) {

		// 左手とは少し時間をずらす
		const float rightHandTime = time - idleHandPhaseOffset_;

		// 上下移動
		const float offsetY = std::sin(rightHandTime) * idleHandAmplitude_;

		// 回転
		const float rotationZ = std::sin(rightHandTime) * idleHandRotationAmplitude_;

		// 初期姿勢を基準に移動
		rightHand->worldTransform.translation_.y = rightHand->baseTranslation.y + offsetY;

		// 初期角度を基準に回転
		rightHand->worldTransform.rotation_.z = rightHand->baseRotation.z - rotationZ;
	}

	// ========================================
	// 左右の足のIdleアニメーション
	// ========================================

	// 左右の足を取得
	CharacterParts::Part* leftFoot = characterParts_->GetPart("LeftFoot");

	CharacterParts::Part* rightFoot = characterParts_->GetPart("RightFoot");

	// 左足のアニメーション
	if (leftFoot != nullptr) {

		// 左足用の時間
		const float leftFootTime = time + idleFootPhaseOffset_;

		// 上下移動
		const float offsetY = std::sin(leftFootTime) * idleFootAmplitude_;

		// 初期位置を基準に移動
		leftFoot->worldTransform.translation_.y = leftFoot->baseTranslation.y + offsetY;
	}

	// 右足のアニメーション
	if (rightFoot != nullptr) {

		// 右足用の時間
		const float rightFootTime = time - idleFootPhaseOffset_;

		// 上下移動
		const float offsetY = std::sin(rightFootTime) * idleFootAmplitude_;

		// 初期位置を基準に移動
		rightFoot->worldTransform.translation_.y = rightFoot->baseTranslation.y + offsetY;
	}
}

// ========================================
// IdleアニメーションのImGui調整
// ========================================
void PlayerAnimationController::DrawImGui() {

#ifdef USE_IMGUI

	ImGui::Begin("Player Animation");

	// ========================================
	// 現在のアニメーション状態
	// ========================================

	const char* stateName = "Unknown";

	switch (currentState_) {

	case AnimationState::kIdle:
		stateName = "Idle";
		break;

	case AnimationState::kWalk:
		stateName = "Walk";
		break;

	case AnimationState::kJump:
		stateName = "Jump";
		break;

	case AnimationState::kAttack:
		stateName = "Attack";
		break;
	}

	ImGui::Text("Current State : %s", stateName);

	ImGui::Separator();

	// ========================================
	// Idleアニメーションの設定
	// ========================================

	if (ImGui::CollapsingHeader("Idle Animation", ImGuiTreeNodeFlags_DefaultOpen)) {

		// アニメーション速度
		ImGui::DragFloat("Idle Speed", &idleSpeed_, 0.01f, 0.0f, 10.0f);

		ImGui::SeparatorText("Body");

		// Bodyの上下移動量
		ImGui::DragFloat("Body Amplitude", &idleBodyAmplitude_, 0.001f, 0.0f, 1.0f);

		ImGui::SeparatorText("Head");

		// Headの上下移動量
		ImGui::DragFloat("Head Amplitude", &idleHeadAmplitude_, 0.001f, 0.0f, 1.0f);

		// Headの回転量
		ImGui::DragFloat("Head Rotation", &idleHeadRotationAmplitude_, 0.001f, 0.0f, 1.0f);

		// Headの動きの遅れ
		ImGui::DragFloat("Head Phase Delay", &idleHeadPhaseDelay_, 0.01f, 0.0f, 3.14f);

		ImGui::SeparatorText("Hands");

		// 手の上下移動量
		ImGui::DragFloat("Hand Amplitude", &idleHandAmplitude_, 0.001f, 0.0f, 1.0f);

		// 手の回転量
		ImGui::DragFloat("Hand Rotation", &idleHandRotationAmplitude_, 0.001f, 0.0f, 1.0f);

		// 手の動きの時間差
		ImGui::DragFloat("Hand Phase Offset", &idleHandPhaseOffset_, 0.01f, 0.0f, 3.14f);

		ImGui::SeparatorText("Feet");

		// 足の上下移動量
		ImGui::DragFloat("Foot Amplitude", &idleFootAmplitude_, 0.001f, 0.0f, 1.0f);

		// 足の動きの時間差
		ImGui::DragFloat("Foot Phase Offset", &idleFootPhaseOffset_, 0.01f, 0.0f, 3.14f);
	}

	// ========================================
	// Idle設定を初期値に戻す
	// ========================================

	ImGui::Separator();

	if (ImGui::Button("Reset Idle Settings")) {

		// Idleアニメーションの設定を初期値へ戻す
		ResetIdleSettings();
	}

	// ========================================
	// Walkアニメーション設定
	// ========================================
	if (ImGui::CollapsingHeader("Walk Animation", ImGuiTreeNodeFlags_DefaultOpen)) {

		// アニメーション速度
		ImGui::DragFloat("Walk Speed", &walkSpeed_, 0.01f, 0.0f, 20.0f);

		ImGui::SeparatorText("Body");

		// Bodyの上下移動量
		ImGui::DragFloat("Walk Body Amplitude", &walkBodyAmplitude_, 0.001f, 0.0f, 1.0f);

		ImGui::SeparatorText("Head");

		// Headの上下移動量
		ImGui::DragFloat("Walk Head Amplitude", &walkHeadAmplitude_, 0.001f, 0.0f, 1.0f);

		ImGui::SeparatorText("Hands");

		// 手の前後移動量
		ImGui::DragFloat("Walk Hand Amplitude", &walkHandAmplitude_, 0.001f, 0.0f, 1.0f);

		// 手の回転量
		ImGui::DragFloat("Walk Hand Rotation", &walkHandRotationAmplitude_, 0.001f, 0.0f, 1.0f);

		// ========================================
		// 手の楕円軌道設定
		// ========================================

		ImGui::SeparatorText("Hand Ellipse");

		// 手の上下移動量
		ImGui::DragFloat("Hand Vertical Amplitude", &walkHandVerticalAmplitude_, 0.001f, 0.0f, 0.3f);

		// 右手の振り幅倍率
		ImGui::DragFloat("Right Hand Swing Ratio", &walkRightHandSwingRatio_, 0.01f, 0.0f, 1.5f);

		ImGui::SeparatorText("Feet");

		// 足の前後移動量
		ImGui::DragFloat("Walk Foot Amplitude", &walkFootAmplitude_, 0.001f, 0.0f, 1.0f);

		// 足を持ち上げる高さ
		ImGui::DragFloat("Walk Foot Lift", &walkFootLift_, 0.001f, 0.0f, 1.0f);
	}

	// ========================================
	// アニメーションブレンド設定
	// ========================================
	if (ImGui::CollapsingHeader("Animation Blend", ImGuiTreeNodeFlags_DefaultOpen)) {

		// ブレンド時間
		ImGui::DragFloat("Blend Duration", &blendDuration_, 0.01f, 0.0f, 1.0f);

		// ブレンド中かどうか
		ImGui::Text("Is Blending : %s", isBlending_ ? "Yes" : "No");

		// ブレンド進行率
		float blendProgress = 1.0f;

		if (isBlending_ && blendDuration_ > 0.0f) {

			blendProgress = (std::clamp)(blendTimer_ / blendDuration_, 0.0f, 1.0f);
		}

		ImGui::ProgressBar(blendProgress, ImVec2(-1.0f, 0.0f));
	}

	// ========================================
	// Walkの速度連動設定
	// ========================================
	ImGui::SeparatorText("Walk Speed Control");

	// 基準移動速度
	ImGui::DragFloat("Walk Reference Speed", &walkReferenceSpeed_, 0.001f, 0.01f, 1.0f);

	// 最低再生倍率
	ImGui::DragFloat("Walk Min Speed Ratio", &walkMinSpeedRatio_, 0.01f, 0.0f, 1.0f);

	// 最大再生倍率
	ImGui::DragFloat("Walk Max Speed Ratio", &walkMaxSpeedRatio_, 0.01f, 0.0f, 3.0f);

	// ========================================
	// Body・Headの位相調整
	// ========================================
	ImGui::SeparatorText("Walk Phase");

	// Bodyの位相
	ImGui::DragFloat("Walk Body Phase", &walkBodyPhaseOffset_, 0.01f, -3.14f, 3.14f);

	// Headの位相
	ImGui::DragFloat("Walk Head Phase", &walkHeadPhaseOffset_, 0.01f, -3.14f, 3.14f);

	// ========================================
	// 現在の移動速度表示
	// ========================================
	ImGui::Separator();

	ImGui::Text("Movement Speed : %.3f", movementSpeed_);

	ImGui::Text("Walk Time : %.3f", walkTime_);

	// ========================================
	// Jumpアニメーション設定
	// ========================================
	if (ImGui::CollapsingHeader("Jump Animation", ImGuiTreeNodeFlags_DefaultOpen)) {

		ImGui::Text("Grounded : %s", isGrounded_ ? "Yes" : "No");

		ImGui::Text("Velocity Y : %.3f", jumpVelocityY_);

		ImGui::Separator();

		ImGui::DragFloat("Jump Velocity Threshold", &jumpVelocityThreshold_, 0.001f, 0.0f, 0.5f);

		ImGui::DragFloat("Jump Hand Rise", &jumpHandRise_, 0.01f, 0.0f, 1.0f);

		ImGui::DragFloat("Jump Hand Spread", &jumpHandSpread_, 0.01f, 0.0f, 1.0f);

		ImGui::DragFloat("Jump Foot Rise", &jumpFootRise_, 0.01f, 0.0f, 1.0f);

		ImGui::DragFloat("Jump Foot Fall", &jumpFootFall_, 0.01f, 0.0f, 1.0f);

		ImGui::DragFloat("Jump Body Offset", &jumpBodyOffset_, 0.01f, 0.0f, 0.5f);

		ImGui::DragFloat("Jump Head Offset", &jumpHeadOffset_, 0.01f, 0.0f, 0.5f);
	}

	ImGui::End();

#endif
}

// ========================================
// Idleアニメーション設定のリセット
// ========================================
void PlayerAnimationController::ResetIdleSettings() {

	// アニメーション速度
	idleSpeed_ = 2.0f;

	// Body
	idleBodyAmplitude_ = 0.04f;

	// Head
	idleHeadAmplitude_ = 0.06f;
	idleHeadRotationAmplitude_ = 0.035f;
	idleHeadPhaseDelay_ = 0.4f;

	// Hands
	idleHandAmplitude_ = 0.025f;
	idleHandRotationAmplitude_ = 0.05f;
	idleHandPhaseOffset_ = 0.6f;

	// Feet
	idleFootAmplitude_ = 0.005f;
	idleFootPhaseOffset_ = 0.5f;
}

// ========================================
// Walkアニメーション更新
// ========================================
void PlayerAnimationController::UpdateWalk() {

	// CharacterPartsが設定されていない場合
	if (characterParts_ == nullptr) {
		return;
	}

	// ========================================
	// 1. 歩行アニメーションの時間
	// ========================================

	// 移動速度に応じて更新された歩行時間
	const float time = walkTime_;

	// 左右の足の前後移動に使用
	const float footSwing = std::sin(time);

	// 足を持ち上げるタイミングに使用
	const float footLiftWave = std::cos(time);

	// ========================================
	// 2. Bodyの上下移動
	// ========================================

	CharacterParts::Part* body = characterParts_->GetPart("Body");

	if (body != nullptr) {

		// Bodyの上下移動タイミング
		const float bodyTime = time + walkBodyPhaseOffset_;

		// 1歩ごとに小さく上下する
		const float offsetY = std::abs(std::sin(bodyTime)) * walkBodyAmplitude_;

		// 基準位置から上下移動
		body->worldTransform.translation_.y = body->baseTranslation.y + offsetY;
	}

	// ========================================
	// 3. Headの上下移動
	// ========================================

	CharacterParts::Part* head = characterParts_->GetPart("Head");

	if (head != nullptr) {

		// Bodyより少し遅れて動かす
		const float headTime = time - walkHeadPhaseOffset_;

		// Headの上下移動
		const float offsetY = std::abs(std::sin(headTime)) * walkHeadAmplitude_;

		head->worldTransform.translation_.y = head->baseTranslation.y + offsetY;
	}

	// ========================================
	// 4. LeftFootの歩行モーション
	// ========================================

	CharacterParts::Part* leftFoot = characterParts_->GetPart("LeftFoot");

	if (leftFoot != nullptr) {

		// 左足の前後移動
		const float offsetX = footSwing * walkFootAmplitude_;

		// 左足を前へ運ぶ半周期だけ持ち上げる
		const float liftPhase = footLiftWave > 0.0f ? footLiftWave : 0.0f;

		// 足の上下移動
		const float offsetY = liftPhase * walkFootLift_;

		// PlayerがY軸に90度回転しているため、
		// ローカルZ方向を歩行の前後方向として使用
		leftFoot->worldTransform.translation_.z = leftFoot->baseTranslation.z + offsetX;

		// Y方向：持ち上げ
		leftFoot->worldTransform.translation_.y = leftFoot->baseTranslation.y + offsetY;
	}

	// ========================================
	// 5. RightFootの歩行モーション
	// ========================================

	CharacterParts::Part* rightFoot = characterParts_->GetPart("RightFoot");

	if (rightFoot != nullptr) {

		// 右足は左足と逆方向へ動かす
		const float offsetX = -footSwing * walkFootAmplitude_;

		// 右足は左足と逆の半周期で持ち上げる
		const float liftPhase = footLiftWave < 0.0f ? -footLiftWave : 0.0f;

		// 足の上下移動
		const float offsetY = liftPhase * walkFootLift_;

		rightFoot->worldTransform.translation_.z = rightFoot->baseTranslation.z + offsetX;

		// Y方向：持ち上げ
		rightFoot->worldTransform.translation_.y = rightFoot->baseTranslation.y + offsetY;
	}

	// ========================================
	// 6. LeftHandの歩行モーション
	// ========================================

	CharacterParts::Part* leftHand = characterParts_->GetPart("LeftHand");

	if (leftHand != nullptr) {

		// ========================================
		// 左手の楕円軌道
		// ========================================

		// 左足と逆方向に振る
		const float handPhase = time + 3.14159265f;

		// 前後移動（ローカルZ方向）
		const float offsetZ = std::sin(handPhase) * walkHandAmplitude_;

		// 上下移動（ローカルY方向）
		const float offsetY = std::cos(handPhase) * walkHandVerticalAmplitude_;

		// 手の回転（ローカルX軸）
		const float rotationX = std::sin(handPhase) * walkHandRotationAmplitude_;

		// ========================================
		// 位置・回転を反映
		// ========================================

		leftHand->worldTransform.translation_.z = leftHand->baseTranslation.z + offsetZ;

		leftHand->worldTransform.translation_.y = leftHand->baseTranslation.y + offsetY;

		leftHand->worldTransform.rotation_.x = leftHand->baseRotation.x + rotationX;
	}

	// ========================================
	// 7. RightHandの歩行モーション
	// ========================================

	CharacterParts::Part* rightHand = characterParts_->GetPart("RightHand");

	if (rightHand != nullptr) {

		// ========================================
		// 右手の楕円軌道
		// ========================================

		// 左手と反対のタイミング
		const float handPhase = time;

		// 武器を持つ右手の振り幅倍率
		const float swingRatio = walkRightHandSwingRatio_;

		// 前後移動（ローカルZ方向）
		const float offsetZ = std::sin(handPhase) * walkHandAmplitude_ * swingRatio;

		// 上下移動（ローカルY方向）
		const float offsetY = std::cos(handPhase) * walkHandVerticalAmplitude_ * swingRatio;

		// 手の回転（ローカルX軸）
		const float rotationX = std::sin(handPhase) * walkHandRotationAmplitude_ * swingRatio;

		// ========================================
		// 位置・回転を反映
		// ========================================

		rightHand->worldTransform.translation_.z = rightHand->baseTranslation.z + offsetZ;

		rightHand->worldTransform.translation_.y = rightHand->baseTranslation.y + offsetY;

		rightHand->worldTransform.rotation_.x = rightHand->baseRotation.x + rotationX;
	}

	// ========================================
	// 8. Weaponについて
	// ========================================

	// WeaponはRightHandの子パーツなので、
	// ここで個別に移動させる必要はない。
	//
	// CharacterParts::Update()によって
	// RightHandの位置・回転に自動追従する。
}

// ========================================
// 現在のパーツ姿勢を保存
// ========================================
PlayerAnimationController::PoseMap PlayerAnimationController::CapturePose() const {

	PoseMap pose;

	if (characterParts_ == nullptr) {
		return pose;
	}

	// 全パーツを取得
	const auto& parts = characterParts_->GetParts();

	for (const auto& [name, part] : parts) {

		PartPose partPose;

		partPose.translation = part.worldTransform.translation_;

		partPose.rotation = part.worldTransform.rotation_;

		partPose.scale = part.worldTransform.scale_;

		pose[name] = partPose;
	}

	return pose;
}

// ========================================
// 保存した姿勢をパーツへ反映
// ========================================
void PlayerAnimationController::ApplyPose(const PoseMap& pose) {

	if (characterParts_ == nullptr) {
		return;
	}

	for (const auto& [name, partPose] : pose) {

		CharacterParts::Part* part = characterParts_->GetPart(name);

		if (part == nullptr) {
			continue;
		}

		part->worldTransform.translation_ = partPose.translation;

		part->worldTransform.rotation_ = partPose.rotation;

		part->worldTransform.scale_ = partPose.scale;
	}
}

// ========================================
// 2つの姿勢を補間
// ========================================
PlayerAnimationController::PoseMap PlayerAnimationController::BlendPose(const PoseMap& from, const PoseMap& to, float blendFactor) const {

	PoseMap result;

	// 0.0～1.0に制限
	const float t = (std::clamp)(blendFactor, 0.0f, 1.0f);

	// Vector3の線形補間
	auto lerpVector3 = [t](const KamataEngine::Vector3& a, const KamataEngine::Vector3& b) -> KamataEngine::Vector3 { return {a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t}; };

	// 遷移先のパーツを基準に補間
	for (const auto& [name, toPose] : to) {

		const auto it = from.find(name);

		// 遷移元に存在しないパーツは
		// 遷移先の姿勢をそのまま使用
		if (it == from.end()) {

			result[name] = toPose;
			continue;
		}

		const PartPose& fromPose = it->second;

		PartPose blendedPose;

		// Translation
		blendedPose.translation = lerpVector3(fromPose.translation, toPose.translation);

		// Rotation
		blendedPose.rotation = lerpVector3(fromPose.rotation, toPose.rotation);

		// Scale
		blendedPose.scale = lerpVector3(fromPose.scale, toPose.scale);

		result[name] = blendedPose;
	}

	return result;
}

// ========================================
// プレイヤーの移動速度を受け取る
// ========================================
void PlayerAnimationController::SetMovementSpeed(float speed) {

	// 移動方向に関係なく速度の大きさを保存
	movementSpeed_ = std::abs(speed);
}

// ========================================
// 移動速度に応じた歩行周期の更新
// ========================================
void PlayerAnimationController::UpdateWalkTime(float deltaTime) {

	// 0除算防止
	if (walkReferenceSpeed_ <= 0.0001f) {
		return;
	}

	// ========================================
	// 現在の移動速度を0～1へ正規化
	// ========================================

	const float speedRatio = (std::clamp)(movementSpeed_ / walkReferenceSpeed_, 0.0f, 1.0f);

	// ========================================
	// 歩行アニメーションの再生倍率
	// ========================================

	const float playbackRatio = walkMinSpeedRatio_ + (walkMaxSpeedRatio_ - walkMinSpeedRatio_) * speedRatio;

	// ========================================
	// 歩行時間を進める
	// ========================================

	walkTime_ += deltaTime * walkSpeed_ * playbackRatio;
}

// ========================================
// ジャンプ情報を設定
// ========================================
void PlayerAnimationController::SetJumpInfo(bool isGrounded, float velocityY) {

	isGrounded_ = isGrounded;
	jumpVelocityY_ = velocityY;
}

// ========================================
// Jumpアニメーション更新
// ========================================
void PlayerAnimationController::UpdateJump() {

	if (characterParts_ == nullptr) {
		return;
	}

	// ========================================
	// ジャンプの状態を判定
	// ========================================

	const bool isRising = jumpVelocityY_ > jumpVelocityThreshold_;

	const bool isFalling = jumpVelocityY_ < -jumpVelocityThreshold_;

	// ========================================
	// Body
	// ========================================

	CharacterParts::Part* body = characterParts_->GetPart("Body");

	if (body != nullptr) {

		float offsetY = 0.0f;

		if (isRising) {

			// 上昇中はBodyを少し持ち上げる
			offsetY = jumpBodyOffset_;

		} else if (isFalling) {

			// 落下中はBodyを少し下げる
			offsetY = -jumpBodyOffset_ * 0.5f;
		}

		body->worldTransform.translation_.y = body->baseTranslation.y + offsetY;
	}

	// ========================================
	// Head
	// ========================================

	CharacterParts::Part* head = characterParts_->GetPart("Head");

	if (head != nullptr) {

		float offsetY = 0.0f;

		if (isRising) {

			offsetY = jumpHeadOffset_;

		} else if (isFalling) {

			offsetY = -jumpHeadOffset_ * 0.5f;
		}

		head->worldTransform.translation_.y = head->baseTranslation.y + offsetY;
	}

	// ========================================
	// LeftHand
	// ========================================

	CharacterParts::Part* leftHand = characterParts_->GetPart("LeftHand");

	if (leftHand != nullptr) {

		float offsetY = 0.0f;
		float offsetZ = 0.0f;

		if (isRising) {

			// 上昇中は手を持ち上げる
			offsetY = jumpHandRise_;

		} else if (isFalling) {

			// 落下中は手を少し広げる
			offsetZ = -jumpHandSpread_;
		}

		leftHand->worldTransform.translation_.y = leftHand->baseTranslation.y + offsetY;

		leftHand->worldTransform.translation_.z = leftHand->baseTranslation.z + offsetZ;
	}

	// ========================================
	// RightHand
	// ========================================

	CharacterParts::Part* rightHand = characterParts_->GetPart("RightHand");

	if (rightHand != nullptr) {

		float offsetY = 0.0f;
		float offsetZ = 0.0f;

		if (isRising) {

			// 武器を持つ右手も持ち上げる
			offsetY = jumpHandRise_ * 0.7f;

		} else if (isFalling) {

			// 右手は左手と逆方向へ
			offsetZ = jumpHandSpread_ * 0.7f;
		}

		rightHand->worldTransform.translation_.y = rightHand->baseTranslation.y + offsetY;

		rightHand->worldTransform.translation_.z = rightHand->baseTranslation.z + offsetZ;
	}

	// ========================================
	// LeftFoot
	// ========================================

	CharacterParts::Part* leftFoot = characterParts_->GetPart("LeftFoot");

	if (leftFoot != nullptr) {

		float offsetY = 0.0f;

		if (isRising) {

			// 上昇中は足を縮める
			offsetY = jumpFootRise_;

		} else if (isFalling) {

			// 落下中は足を伸ばす
			offsetY = -jumpFootFall_;
		}

		leftFoot->worldTransform.translation_.y = leftFoot->baseTranslation.y + offsetY;
	}

	// ========================================
	// RightFoot
	// ========================================

	CharacterParts::Part* rightFoot = characterParts_->GetPart("RightFoot");

	if (rightFoot != nullptr) {

		float offsetY = 0.0f;

		if (isRising) {

			offsetY = jumpFootRise_;

		} else if (isFalling) {

			offsetY = -jumpFootFall_;
		}

		rightFoot->worldTransform.translation_.y = rightFoot->baseTranslation.y + offsetY;
	}

	// WeaponはRightHandに親子付けされているため
	// 個別に更新しない
}