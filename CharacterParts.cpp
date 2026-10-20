#include "CharacterParts.h"

#include "math.h"

#include <cassert>

#ifdef USE_IMGUI
#include <imgui.h>
#endif

using namespace KamataEngine;

// ==============================
// パーツ追加
// ==============================
void CharacterParts::AddPart(const std::string& name, Model* model, const Vector3& translation, const Vector3& rotation, const Vector3& scale) {

	assert(model);

	// ==============================
	// unordered_map内に直接Partを作成
	// ==============================
	auto [iterator, inserted] = parts_.try_emplace(name);

	Part& part = iterator->second;

	// ==============================
	// 基本情報
	// ==============================
	part.name = name;
	part.model = model;

	// ==============================
	// WorldTransform初期化
	// ==============================
	part.worldTransform.Initialize();

	part.worldTransform.translation_ = translation;

	part.worldTransform.rotation_ = rotation;

	part.worldTransform.scale_ = scale;

	// ==============================
	// 初期姿勢を保存
	// ==============================
	part.baseTranslation = translation;

	part.baseRotation = rotation;

	part.baseScale = scale;

	// ==============================
	// 行列更新
	// ==============================
	UpdateWorldTransform(part.worldTransform);

	part.worldTransform.TransferMatrix();
}

// ==============================
// パーツ取得
// ==============================
CharacterParts::Part* CharacterParts::GetPart(const std::string& name) {

	auto iterator = parts_.find(name);

	if (iterator == parts_.end()) {

		return nullptr;
	}

	return &iterator->second;
}

// ==============================
// キャラクター全体の親を設定
// ==============================
void CharacterParts::SetRootTransform(WorldTransform* rootTransform) { rootTransform_ = rootTransform; }

// ==============================
// パーツ同士の親子関係を設定
// ==============================
bool CharacterParts::SetParent(const std::string& childName, const std::string& parentName) {

	// 自分自身を親にはできない
	if (childName == parentName) {
		return false;
	}

	// 両方のパーツが存在するか確認
	if (!GetPart(childName) || !GetPart(parentName)) {
		return false;
	}

	// 親子関係の循環を防止
	std::string current = parentName;

	while (true) {

		if (current == childName) {
			return false;
		}

		auto iterator = parentNames_.find(current);

		if (iterator == parentNames_.end()) {
			break;
		}

		current = iterator->second;
	}

	parentNames_[childName] = parentName;

	return true;
}

// ==============================
// パーツ取得 const版
// ==============================
const CharacterParts::Part* CharacterParts::GetPart(const std::string& name) const {

	auto iterator = parts_.find(name);

	if (iterator == parts_.end()) {

		return nullptr;
	}

	return &iterator->second;
}

// ==============================
// 初期姿勢へ戻す
// ==============================
void CharacterParts::ResetPose() {

	for (auto& [name, part] : parts_) {

		(void)name;

		part.worldTransform.translation_ = part.baseTranslation;

		part.worldTransform.rotation_ = part.baseRotation;

		part.worldTransform.scale_ = part.baseScale;
	}
}

// ==============================
// 全パーツの行列更新
// ==============================
void CharacterParts::Update() {

	// 親のワールド行列が先に更新されていることが前提

	std::unordered_map<std::string, bool> updated;

	for (const auto& [name, part] : parts_) {

		(void)part;

		UpdatePart(name, updated);
	}
}

// ==============================
// 親から順番にパーツの行列を更新
// ==============================
void CharacterParts::UpdatePart(const std::string& name, std::unordered_map<std::string, bool>& updated) {

	if (updated[name]) {
		return;
	}

	Part* part = GetPart(name);

	if (!part) {
		return;
	}

	// 基本の親はキャラクター全体のRoot
	WorldTransform* parentTransform = rootTransform_;

	auto iterator = parentNames_.find(name);

	// パーツ同士の親子関係がある場合
	if (iterator != parentNames_.end()) {

		const std::string& parentName = iterator->second;

		// 先に親パーツを更新する
		UpdatePart(parentName, updated);

		Part* parentPart = GetPart(parentName);

		if (parentPart) {
			parentTransform = &parentPart->worldTransform;
		}
	}

	// ローカル行列を生成
	UpdateWorldTransform(part->worldTransform);

	// 親がある場合はワールド行列を合成
	if (parentTransform) {

		part->worldTransform.matWorld_ = MathUtility::operator*(part->worldTransform.matWorld_, parentTransform->matWorld_);
	}

	// GPUへ転送
	part->worldTransform.TransferMatrix();

	updated[name] = true;
}

// ==============================
// 描画
// ==============================
void CharacterParts::Draw(Camera* camera) {

	if (camera == nullptr) {
		return;
	}

	for (auto& [name, part] : parts_) {

		(void)name;

		if (part.model == nullptr) {
			continue;
		}

		part.model->Draw(part.worldTransform, *camera);
	}
}

// ==============================
// パーツ調整用ImGui
// ==============================
void CharacterParts::DrawImGui() {

#ifdef USE_IMGUI

	ImGui::Begin("Character Parts");

	// 調整するパーツの名前
	const char* partNames[] = {"Body", "Head", "LeftHand", "RightHand", "LeftFoot", "RightFoot", "Weapon"};

	// 各パーツの位置を調整
	for (const char* name : partNames) {

		Part* part = GetPart(name);

		if (part == nullptr) {
			continue;
		}

		// パーツごとに折りたたみ表示
		if (ImGui::TreeNode(name)) {

			// ==============================
			// パーツの位置
			// ==============================
			ImGui::DragFloat3("Translation", &part->worldTransform.translation_.x, 0.05f);

			// ==============================
			// パーツの回転
			// ==============================
			// rotation_ はラジアンで管理しているため、
			// ImGuiでは角度（度）として表示する
			ImGui::DragFloat3("Rotation", &part->worldTransform.rotation_.x, 0.01f);

			// ==============================
			// パーツの拡大縮小
			// ==============================
			ImGui::DragFloat3("Scale", &part->worldTransform.scale_.x, 0.01f, 0.01f, 10.0f);

			// ==============================
			// 現在の姿勢を初期姿勢として保存
			// ==============================
			if (ImGui::Button("Save Base Pose")) {

				part->baseTranslation = part->worldTransform.translation_;

				part->baseRotation = part->worldTransform.rotation_;

				part->baseScale = part->worldTransform.scale_;
			}

			// ==============================
			// 保存した初期姿勢に戻す
			// ==============================
			if (ImGui::Button("Reset Pose")) {

				part->worldTransform.translation_ = part->baseTranslation;

				part->worldTransform.rotation_ = part->baseRotation;

				part->worldTransform.scale_ = part->baseScale;
			}

			ImGui::TreePop();
		}
	}

	ImGui::End();

#endif
}