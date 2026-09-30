#pragma once

#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

// ==============================
// 1ステージ分のデータ
// ==============================
struct StageData {
	std::string name;	// ステージ名(フィールドCSVファイル名)
	int32_t timeLimit;	// 制限時間(秒)
};

// ==============================
// ステージ管理クラス
// ==============================
class StageManager {
public:
	/// <summary>
	/// ステージデータファイルの読み込み
	/// </summary>
	void LoadStageDataCsv();

	/// <summary>
	/// ステージデータの取得
	/// </summary>
	/// <param name="index">ステージ番号</param>
	/// <returns>ステージデータ</returns>
	const StageData& GetStageData(uint32_t index) const {
		assert(index < stageDatas_.size());
		return stageDatas_[index];
	}

	/// <summary>
	/// ステージ数を取得
	/// </summary>
	uint32_t GetStageCount() const { return static_cast<uint32_t>(stageDatas_.size()); }


	// 現在のステージ番号を変更
	void SetCurrentStageIndex(int32_t index) {

		assert(index >= 0 && index < static_cast<int32_t>(stageDatas_.size()));

		currentStageIndex_ = index;
	}

	// 現在のステージ番号を取得
	int32_t GetCurrentStageIndex() const { return currentStageIndex_; }

	// 現在のステージデータを取得
	const StageData& GetCurrentStageData() const { return GetStageData(static_cast<uint32_t>(currentStageIndex_)); }

	/// <summary>
	/// ステージ名指定で現在ステージ番号設定
	/// </summary>
	/// <param name="stageName">ステージ名</param>
	void SetCurrentStageIndexByName(const std::string& name); 

private:
	// 全ステージデータ
	std::vector<StageData> stageDatas_;

	// 現在のステージ番号
	int32_t currentStageIndex_ = 0;
};