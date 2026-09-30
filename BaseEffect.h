#pragma once

class BaseEffect {
public:
	// 基底クラスポインタから安全にdeleteするために必要
	virtual ~BaseEffect() = default;

	// 更新
	virtual void Update() = 0;

	// 描画
	virtual void Draw() = 0;

	// 削除可能か
	virtual bool IsDead() const = 0;
};