#include "TitleScene.h"
#include "math.h"
#include <algorithm>
#include <numbers>

using namespace KamataEngine;

// ==============================
// 初期化
// ==============================
void TitleScene::Initialize() {

	// カメラ
	camera_.Initialize();	

	// モデル
	titleModel_ = Model::CreateFromOBJ("titleFont", true);

	playerModel_ = Model::CreateFromOBJ("player", true);

	titleWorldTransform.Initialize();
	titleWorldTransform.translation_ = titlePosition;

	playerWorldTransform.Initialize();
	playerWorldTransform.translation_ = playerPosition;
	// 回転
	playerWorldTransform.rotation_.y = std::numbers::pi_v<float>;



	fade_ = new Fade();
	fade_->Initialize();

	fade_->Start(Fade::Status::FadeIn, kFadeDuration);

}

TitleScene::~TitleScene() {
	delete fade_;
	fade_ = nullptr;
}

void TitleScene::Update() { 

	upDownTimer_ += 0.03f;
	titleWorldTransform.translation_.y = 3.5f + std::sinf(upDownTimer_) * 0.5f;

	 switch (phase_) {

	// フェードイン中
	case Phase::kFadeIn:

		fade_->Update();

		if (fade_->IsFinished()) {
			phase_ = Phase::kMain;
		}

		break;

	// タイトル画面
	case Phase::kMain:

		if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {

			fade_->Start(Fade::Status::FadeOut, kFadeDuration);

			phase_ = Phase::kFadeOut;
		}

		break;

	// フェードアウト中
	case Phase::kFadeOut:

		fade_->Update();

		if (fade_->IsFinished()) {
			finished_ = true;
		}

		break;
	}

	UpdateWorldTransform(titleWorldTransform);
	UpdateWorldTransform(playerWorldTransform);
}

void TitleScene::Draw() { 
	
	Model::PreDraw();
	titleModel_->Draw(titleWorldTransform, camera_);
	playerModel_->Draw(playerWorldTransform, camera_);
	fade_->Draw();

	Model::PostDraw();
}
