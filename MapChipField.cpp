#include "MapChipField.h"
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <cassert>

// マップチップ種別テーブル
std::map<char, MapChipType> mapChipTypeTable = {
    {'B', MapChipType::kBlock},
    {'P', MapChipType::kPlayer},
    {'E', MapChipType::kEnemy},

};

void MapChipField::ResetMapChipData() {
	mapChipData_.data.clear();
	mapChipData_.data.resize(kNumBlockVertical);

	for (std::vector<MapChipDataUnit>& mapChipDataLine : mapChipData_.data) {
		mapChipDataLine.resize(kNumBlockHorizontal);
	}
}


void MapChipField::LoadMapChipCsv(const std::string& filePath) {
	// マップチップデータをリセット
	ResetMapChipData();

	std::ifstream file;
	file.open(filePath);
	assert(file.is_open());

	// CSVファイルを開く
	std::stringstream mapChipCsv;
	// ファイルの内容を文字列ストリームにコピー
	mapChipCsv << file.rdbuf();
	// ファイルを閉じる
	file.close();
	// CSVからマップチップデータを読み込む
	for (uint32_t y = 0; y < kNumBlockVertical; ++y) {
		std::string line;
		getline(mapChipCsv, line);

		// 1行分の文字列をストリームに変換して解析しやすくする
		std::istringstream lineStream(line);

		for (uint32_t x = 0; x < kNumBlockHorizontal; ++x) {
			std::string word;
			getline(lineStream, word, ','); // カンマ区切りで読み込む

			// 空白の場合はスキップ
			if (word.empty()) {
				continue;
			}

			// 先頭文字がいずれかのマップチップ種別に該当するか確認
			if (!mapChipTypeTable.contains(word[kChipType])) {
				continue;
			}

			// 先頭文字でマップチップのタイプを利別
			mapChipData_.data[y][x].type = mapChipTypeTable[word[kChipType]];

			// サブIDを含まない場合はスキップ (0番で特定)
			if (word.size() <= kChipSubID) {
				continue;
			}

			// マップチップのサブIDを設定
			mapChipData_.data[y][x].subID = static_cast<uint8_t>(word[kChipSubID] - '0');


		}
	}
}

MapChipType MapChipField::GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex) const {

	if (xIndex >= kNumBlockHorizontal || yIndex >= kNumBlockVertical) {
		return MapChipType::kBlank;
	}

	return mapChipData_.data[yIndex][xIndex].type;
}


KamataEngine::Vector3 MapChipField::GetBlockPositionByIndex(uint32_t xIndex, uint32_t yIndex)const {
	return KamataEngine::Vector3(xIndex * kBlockWidth, kBlockHeight * (kNumBlockVertical - 1 - yIndex), 0.0f);
}

IndexSet MapChipField::GetMapChipIndexSetByPosition(const KamataEngine::Vector3& position) { 
	IndexSet indexSet = {};
	indexSet.xIndex = static_cast<uint32_t>(std::floor((position.x + kBlockWidth / 2.0f) / kBlockWidth));

	indexSet.yIndex = static_cast<uint32_t>(kNumBlockVertical - 1 - std::floor((position.y + kBlockHeight / 2.0f) / kBlockHeight));
	return indexSet;
}

//指定ブロックの中心座標を取得する
Rect MapChipField::GetRectByIndex(uint32_t xIndex, uint32_t yIndex) { 
	KamataEngine::Vector3 center = MapChipField::GetBlockPositionByIndex(xIndex, yIndex);

	Rect rect;
	rect.left = center.x - kBlockWidth / 2.0f;
	rect.right = center.x + kBlockWidth / 2.0f;
	rect.bottom = center.y - kBlockHeight / 2.0f;
	rect.top = center.y + kBlockHeight / 2.0f;

	return rect;
}

uint8_t MapChipField::GetMapChipSubIDByIndex(uint32_t xIndex, uint32_t yIndex) const {

	if (xIndex >= kNumBlockHorizontal || yIndex >= kNumBlockVertical) {
		return 0;
	}

	return mapChipData_.data[yIndex][xIndex].subID;
}