#include "GameScene.h"
#include "KamataEngine.h"
#include "StageManager.h"
#include "TitleScene.h"

#include <Windows.h>

#include <cassert>
#include <fstream>
#include <sstream>
#include <string>

using namespace KamataEngine;

// ==============================
// シーンのインスタンス
// ==============================
TitleScene* titleScene = nullptr;
GameScene* gameScene = nullptr;

// ==============================
// ステージマネージャー
// ==============================
StageManager* stageManager = nullptr;

// ==============================
// シーンの種類
// ==============================
enum class Scene {

	kUnknown = 0,

	kTitle,
	kGame,
};

// ==============================
// 現在のシーン
// ==============================
Scene scene = Scene::kUnknown;

// ==============================
// 関数宣言
// ==============================

// シーン切り替え
void ChangeScene();

// 現在シーンの更新
void UpdateScene();

// 現在シーンの描画
void DrawScene();

// デバッグ設定ファイルの読み込み
void LoadDebugSettings();

// ==============================
// Windowsアプリのエントリーポイント
// ==============================
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// ==============================
	// エンジンの初期化
	// ==============================
	KamataEngine::Initialize(L"LC2C_05_オオタニ_タクヤ_AL3_ゲームタイトル");

	// DirectX共通処理の取得
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// ==============================
	// ステージマネージャー生成
	// ==============================
	stageManager = new StageManager();

	// ステージデータファイルを読み込む
	stageManager->LoadStageDataCsv();

#ifdef _DEBUG

	// ==============================
	// デバッグ設定ファイル読み込み
	// ==============================
	LoadDebugSettings();

	// ==============================
	// Debugではゲームシーンから開始
	// ==============================
	scene = Scene::kGame;

	// ゲームシーンを生成
	gameScene = new GameScene();

	// ゲームシーンを初期化
	gameScene->Initialize(stageManager);

#else

	// ==============================
	// Releaseではタイトルシーンから開始
	// ==============================
	scene = Scene::kTitle;

	// タイトルシーンを生成
	titleScene = new TitleScene();

	// タイトルシーンを初期化
	titleScene->Initialize();

#endif

	// ==============================
	// メインループ
	// ==============================
	while (true) {

		// Windowsメッセージ処理
		if (KamataEngine::Update()) {
			break;
		}

#ifdef _DEBUG

		// ImGuiフレーム開始
		ImGuiManager::GetInstance()->Begin();

#endif

		// シーン切り替え
		ChangeScene();

		// 現在シーンの更新
		UpdateScene();

#ifdef _DEBUG

		// ImGuiフレーム終了
		ImGuiManager::GetInstance()->End();

#endif

		// 描画前処理
		dxCommon->PreDraw();

		// 現在シーンの描画
		DrawScene();

#ifdef _DEBUG

		// ImGui描画
		ImGuiManager::GetInstance()->Draw();

#endif

		// 描画後処理
		dxCommon->PostDraw();
	}

	// ==============================
	// シーンの解放
	// ==============================
	delete titleScene;
	titleScene = nullptr;

	delete gameScene;
	gameScene = nullptr;

	// ==============================
	// ステージマネージャーの解放
	// ==============================
	delete stageManager;
	stageManager = nullptr;

	// ==============================
	// エンジンの終了処理
	// ==============================
	KamataEngine::Finalize();

	return 0;
}

// ==============================
// デバッグ設定ファイルの読み込み
// ==============================
void LoadDebugSettings() {

	// StageManagerが未生成の場合は処理しない
	if (stageManager == nullptr) {
		return;
	}

	// ==============================
	// 設定ファイルを開く
	// ==============================
	const std::string filePath = "./DebugSettings.ini";

	std::ifstream file;
	file.open(filePath);

	assert(file.is_open() && "DebugSettings.iniが存在しません");

	// ==============================
	// ファイル内容を文字列ストリームへコピー
	// ==============================
	std::stringstream debugSettingsCsv;
	debugSettingsCsv << file.rdbuf();

	// ファイルを閉じる
	file.close();

	// ==============================
	// 1行ずつ解析
	// ==============================
	std::string line;

	while (std::getline(debugSettingsCsv, line)) {

		// 空行は読み飛ばす
		if (line.empty()) {
			continue;
		}

		// コメント行は読み飛ばす
		if (line[0] == ';' || line[0] == '#') {
			continue;
		}

		// 「=」の位置を取得
		const size_t equalPosition = line.find('=');

		// 「=」が存在しない行は読み飛ばす
		if (equalPosition == std::string::npos) {
			continue;
		}

		// 「=」より左側をキーとして取得
		const std::string key = line.substr(0, equalPosition);

		// 「=」より右側を値として取得
		const std::string value = line.substr(equalPosition + 1);

		// ==============================
		// 初期ステージ設定
		// ==============================
		if (key == "InitialStage") {

			// ステージ名から現在ステージ番号を設定
			stageManager->SetCurrentStageIndexByName(value);
		}
	}
}

// ==============================
// シーン切り替え
// ==============================
void ChangeScene() {

	switch (scene) {

	// ==============================
	// タイトルシーン
	// ==============================
	case Scene::kTitle:

		// タイトルシーンが終了した場合
		if (titleScene != nullptr && titleScene->IsFinished()) {

			// シーンをゲームへ変更
			scene = Scene::kGame;

			// 旧シーンを解放
			delete titleScene;
			titleScene = nullptr;

			// 新しいゲームシーンを生成
			gameScene = new GameScene();

			// ゲームシーンを初期化
			gameScene->Initialize(stageManager);
		}

		break;

	// ==============================
	// ゲームシーン
	// ==============================
	case Scene::kGame:

		if (gameScene == nullptr) {
			break;
		}

		// ゲーム終了後にタイトルへ戻る
		if (gameScene->IsFinished()) {

			// シーンをタイトルへ変更
			scene = Scene::kTitle;

			// 旧シーンを解放
			delete gameScene;
			gameScene = nullptr;

			// 新しいタイトルシーンを生成
			titleScene = new TitleScene();

			// タイトルシーンを初期化
			titleScene->Initialize();
		}

		// ==============================
		// マップのホットリロード
		// ==============================
		else if (gameScene->IsReloadRequested()) {

			// 現在のゲームシーンを解放
			delete gameScene;
			gameScene = nullptr;

			// 新しいゲームシーンを生成
			gameScene = new GameScene();

			// 現在ステージのCSVを再読み込み
			gameScene->Initialize(stageManager);
		}

		break;

	case Scene::kUnknown:
	default:
		break;
	}
}

// ==============================
// 現在シーンの更新
// ==============================
void UpdateScene() {

	switch (scene) {

	case Scene::kTitle:

		if (titleScene != nullptr) {
			titleScene->Update();
		}

		break;

	case Scene::kGame:

		if (gameScene != nullptr) {
			gameScene->Update();
		}

		break;

	case Scene::kUnknown:
	default:
		break;
	}
}

// ==============================
// 現在シーンの描画
// ==============================
void DrawScene() {

	switch (scene) {

	case Scene::kTitle:

		if (titleScene != nullptr) {
			titleScene->Draw();
		}

		break;

	case Scene::kGame:

		if (gameScene != nullptr) {
			gameScene->Draw();
		}

		break;

	case Scene::kUnknown:
	default:
		break;
	}
}