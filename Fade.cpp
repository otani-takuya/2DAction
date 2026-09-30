#include "Fade.h"
#include <algorithm>

// ==============================
// 初期化
// ==============================
void Fade::Initialize() {
	// テクスチャを読み込む
	uint32_t fadeSprite = TextureManager::Load("white1x1.png");
	sprite_ = Sprite::Create(fadeSprite, {0.0f, 0.0f});
	sprite_->SetSize(Vector2(1280.0f, 720.0f));
	sprite_->SetColor(Vector4(0.0f, 0.0f, 0.0f, 1.0f));
}

void Fade::Update() {
	//フェード状態による分岐
	switch (status_) {
	case Status::None:
		//何もしない
	    break; 
	case Status::FadeIn:
		// 1フレーム分の秒数をカウントアップ
		counter_ += 1.0f / 60.0f;
		// フェード継続時間に達したら打ち止め
		if (counter_ >= duration_) {
			counter_ = duration_;
		}
		// 1.0fから0.0fの間で、 経過時間がフェード継続時間に近づくほどアルファ値を小さくする
		sprite_->SetColor(Vector4(0, 0, 0, 1.0f - std::clamp(counter_ / duration_, 0.0f, 1.0f)));

		break;
	case Status::FadeOut:
		// 1フレーム分の秒数をカウントアップ 
		counter_ += 1.0f / 60.0f;
		// フェード継続時間に達したら打ち止め
		if (counter_ >= duration_) {
			counter_ = duration_;
		}
		// 0.0fから1.0fの間で、 経過時間がフェード継続時間に近づくほどアルファ値を大きくする 
		sprite_->SetColor(Vector4 (0, 0, 0, std::clamp (counter_/ duration_, 0.0f, 1.0f)));

		break;
	default:
		break;
	}
}

void Fade::Draw() { 
	Sprite::PreDraw();
	if (status_ == Status::None) {
		return;
	}

	sprite_->Draw();
	Sprite::PostDraw();
}

void Fade::Start(Status status, float duration) { 
	status_ = status;
	duration_ = duration;
	counter_ = 0.0f;
}

void Fade::Stop() {
	status_ = Status::None;
}

bool Fade::IsFinished() const {
	// フェード状態による分岐 
    if (status_ == Status::FadeIn || status_ == Status::FadeOut) {
		return counter_ >= duration_;
	}

	return true;
}