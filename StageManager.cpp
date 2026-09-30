#include "StageManager.h"

#include <cassert>
#include <fstream>
#include <sstream>
#include <string>

// ==============================
// ステージデータファイルの読み込み
// ==============================
void StageManager::LoadStageDataCsv() {

	// 再読み込み対策
	stageDatas_.clear();

	// ステージデータCSVのパス
	const std::string filePath = "./Resources/stageDatas.csv";

	// ファイルを開く
	std::ifstream file(filePath);

	assert(file.is_open() && "ステージデータファイルが存在しません");

	// ファイルの内容を文字列ストリームへコピー
	std::stringstream stageDataCsv;
	stageDataCsv << file.rdbuf();

	// ファイルを閉じる
	file.close();

	// 1行ずつ読み込む
	std::string line;

	while (std::getline(stageDataCsv, line)) {

		// 空行は無視
		if (line.empty()) {
			continue;
		}

		std::istringstream lineStream(line);

		StageData stageData{};

		// 1列目：ステージ名
		std::string name;
		std::getline(lineStream, name, ',');

		// 2列目：制限時間
		std::string timeLimitText;
		std::getline(lineStream, timeLimitText, ',');

		stageData.name = name;
		stageData.timeLimit = std::stoi(timeLimitText);

		stageDatas_.push_back(stageData);
	}
}

void StageManager::SetCurrentStageIndexByName(const std::string& name) {
	// 全ステージデータ検索
	for (size_t i = 0; i < stageDatas_.size(); ++i) {
		// ステージ名が一致したら現在ステージ番号を設定する
		if (stageDatas_[i].name == name) {
			currentStageIndex_ = static_cast<int32_t>(i);
			// 目的を達成したので関数を抜ける
			return;
		}
	}
	assert(false && "指定されたステージ名は存在していません");
}