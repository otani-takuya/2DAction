#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include "KamataEngine.h"

enum class MapChipType {
	kBlank, // 空白
	kBlock, // ブロック
	kPlayer, // プレイヤー
	kEnemy, // 敵
};


// 1マスの分のデータ
struct MapChipDataUnit {
	MapChipType type;	// マップチップの種別
	uint8_t subID;		// 種類ごとのサブID
};

// マップチップCSVの文字番号
enum MapChipCharIndex {
	kChipType = 0,  // マップチップタイプ 
	kChipSubID = 1  // タイプごとのサブID
};


struct MapChipData {
	std::vector<std::vector<MapChipDataUnit>> data;
};

struct IndexSet {
	uint32_t xIndex;
	uint32_t yIndex;
};

//範囲矩形
struct Rect {
	float left;			//左端
	float right;		//右端
	float bottom;		//下端
	float top;			//上端
};

// ブロックの数
static inline const uint32_t kNumBlockVertical = 20;
static inline const uint32_t kNumBlockHorizontal = 100;

// 1ブロックのサイズ
static inline const float kBlockWidth = 1.0f;
static inline const float kBlockHeight = 1.0f;

class MapChipField {
private:
	MapChipData mapChipData_;

	void ResetMapChipData();


public:

	void LoadMapChipCsv(const std::string& filePath);

	MapChipType GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex) const;

	KamataEngine::Vector3 GetBlockPositionByIndex(uint32_t xIndex, uint32_t yIndex) const;
	
	IndexSet GetMapChipIndexSetByPosition(const KamataEngine::Vector3& position);

	Rect GetRectByIndex(uint32_t xIndex, uint32_t yIndex);

	uint8_t GetMapChipSubIDByIndex(uint32_t xIndex, uint32_t yIndex) const;
};