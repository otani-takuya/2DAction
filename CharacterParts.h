#pragma once

#include "KamataEngine.h"

#include <string>
#include <unordered_map>

class CharacterParts {
public:
	// ==============================
	// キャラクターの1パーツ
	// ==============================
	struct Part {

		// パーツ名
		std::string name;

		// モデル
		KamataEngine::Model* model = nullptr;

		// Transform
		KamataEngine::WorldTransform worldTransform;

		// 初期姿勢
		KamataEngine::Vector3 baseScale = {1.0f, 1.0f, 1.0f};

		KamataEngine::Vector3 baseRotation = {0.0f, 0.0f, 0.0f};

		KamataEngine::Vector3 baseTranslation = {0.0f, 0.0f, 0.0f};
	};

public:
	// ==============================
	// パーツ追加
	// ==============================
	void AddPart(
	    const std::string& name, KamataEngine::Model* model, const KamataEngine::Vector3& translation = {0.0f, 0.0f, 0.0f}, const KamataEngine::Vector3& rotation = {0.0f, 0.0f, 0.0f},
	    const KamataEngine::Vector3& scale = {1.0f, 1.0f, 1.0f});

	// ==============================
	// キャラクター全体の親Transformを設定
	// ==============================
	void SetRootTransform(KamataEngine::WorldTransform* rootTransform);

	// ==============================
	// パーツ同士の親子関係を設定
	// ==============================
	bool SetParent(const std::string& childName, const std::string& parentName);


	// ==============================
	// パーツ取得
	// ==============================
	Part* GetPart(const std::string& name);

	const Part* GetPart(const std::string& name) const;

	// ==============================
	// 初期姿勢へ戻す
	// ==============================
	void ResetPose();

	// ==============================
	// 行列更新
	// ==============================
	void Update();

	// ==============================
	// 描画
	// ==============================
	void Draw(KamataEngine::Camera* camera);

	// ==============================
	// パーツ調整用ImGui
	// ==============================
	void DrawImGui();

private:
	std::unordered_map<std::string, Part> parts_;

	// キャラクター全体の親Transform
	KamataEngine::WorldTransform* rootTransform_ = nullptr;

	// パーツの親子関係を記録
	std::unordered_map<std::string, std::string> parentNames_;

	// 指定したパーツと、その子パーツの行列を更新
	void UpdatePart(const std::string& name, std::unordered_map<std::string, bool>& updated);
};