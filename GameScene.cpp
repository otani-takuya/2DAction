#include "GameScene.h"
#include "math.h"
#include "StageManager.h"


using namespace KamataEngine;

// ======================
// フィールドオブジェクト生成
// ======================
void GameScene::GenerateFieldObjects() {

	// ブロック配列のサイズ確保
	worldTransformBlocks_.resize(kNumBlockVertical);

	for (uint32_t y = 0; y < kNumBlockVertical; ++y) {
		worldTransformBlocks_[y].resize(kNumBlockHorizontal, nullptr);
	}

	// プレイヤーの二重生成防止
	bool isPlayerCreated = false;

	// CSVの全マスを確認
	for (uint32_t y = 0; y < kNumBlockVertical; ++y) {
		for (uint32_t x = 0; x < kNumBlockHorizontal; ++x) {

			// マップチップ種別
			const MapChipType mapChipType = mapChipField_->GetMapChipTypeByIndex(x, y);

			// マップチップのサブID
			const uint8_t subID = mapChipField_->GetMapChipSubIDByIndex(x, y);

			// マップチップのワールド座標
			KamataEngine::Vector3 position = mapChipField_->GetBlockPositionByIndex(x, y);

			switch (mapChipType) {

			// ======================
			// ブロック生成
			// ======================
			case MapChipType::kBlock: {

				WorldTransform* worldTransform = new WorldTransform();

				worldTransform->Initialize();
				worldTransform->translation_ = position;

				UpdateWorldTransform(*worldTransform);

				worldTransformBlocks_[y][x] = worldTransform;

				break;
			}

			// ======================
			// プレイヤー生成
			// ======================
			case MapChipType::kPlayer: {

				// P0が複数あっても1体だけ生成
				if (isPlayerCreated) {
					break;
				}

				player_ = new Player();

				player_->Initialize(playerModel_, attackModel_, &camera_, position);

				player_->SetMapChipField(mapChipField_);

				isPlayerCreated = true;

				break;
			}

			// ======================
			// 敵生成
			// ======================
			case MapChipType::kEnemy: {

				// 敵をブロック上へ配置するための補正
				position.y += 0.5f;

				switch (subID) {

				// ----------------------
				// E0：通常敵
				// ----------------------
				case 0: {

					Enemy* newEnemy = new Enemy();

					newEnemy->Initialize(enemyModel_, &camera_, position);

					newEnemy->SetMapChipField(mapChipField_);

					newEnemy->SetGameScene(this);

					enemies_.push_back(newEnemy);

					break;
				}

				// ----------------------
				// E1：シールドエネミー
				// ----------------------
				case 1: {

					ShieldEnemy* newShieldEnemy = new ShieldEnemy();

					newShieldEnemy->Initialize(shieldEnemyModel_, &camera_, position);

					newShieldEnemy->SetMapChipField(mapChipField_);

					newShieldEnemy->SetGameScene(this);

					enemies_.push_back(newShieldEnemy);

					break;
				}

				default:
					// 未対応の敵サブID
					break;
				}

				break;
			}

			case MapChipType::kBlank:
			default:
				break;
			}
		}
	}
}

// ======================
// 初期化処理
// ======================
void GameScene::Initialize(StageManager* stageManager) {

	assert(stageManager != nullptr);

	// 引数をメンバ変数へ保存
	stageManager_ = stageManager;

	// リロード要求を解除
	reloadRequested_ = false;


	// ======================
	// テクスチャ読み込み
	// ======================
	// 描画前にGPUにテクスチャを登録しておく必要がある
	textureHandle_ = TextureManager::Load("./Resources/uvChecker.png");
		
	

	// ======================
	// モデル生成
	// ======================
	model_ = Model::Create();
	modelBlock_ = Model::CreateFromOBJ("block", true);
	modelSkydome_ = Model::CreateFromOBJ("skydome", true);
	playerModel_ = Model::CreateFromOBJ("player", true);
	attackModel_ = Model::CreateFromOBJ("hit_effect", true);
	enemyModel_ = Model::CreateFromOBJ("enemy", true);
	shieldEnemyModel_ = Model::CreateFromOBJ("shieldEnemy", true);
	deathParticlesModel_ = Model::CreateFromOBJ("deathparticle", true);
	hitEffectModel_ = Model::CreateFromOBJ("particle", true);
	guardEffectModel_ = Model::CreateFromOBJ("ring", true);

	// ======================
	// カメラ初期化
	// ======================
	// farZを広げる
	camera_.farZ = 1000.0f;

	camera_.Initialize();

	// デバッグ用カメラ（F1で切り替え）
	debugCamera_ = new DebugCamera(1280, 720);

	// マップチップフィールド生成
	mapChipField_ = new MapChipField();

	// 現在のステージデータを取得
	const StageData& stageData = stageManager_->GetCurrentStageData();

	// CSVファイルパスを作成
	const std::string stageFileName = "./Resources/fields/" + stageData.name + ".csv";

	// 現在ステージのCSVを読み込む
	mapChipField_->LoadMapChipCsv(stageFileName);

	GenerateFieldObjects();

	// ======================
	// やられパーティクル生成
	// ======================
	deathParticles_ = new DeathParticles();
	deathParticles_->Initialize(deathParticlesModel_, &camera_, player_->GetWorldPosition());


	// ======================
	// ヒットエフェクト生成
	// ======================
	HitEffect::SetModel(hitEffectModel_);
	HitEffect::SetCamera(&camera_);

	// ======================
	// ガードエフェクト生成
	// ======================
	GuardEffect::SetModel(guardEffectModel_);
	GuardEffect::SetCamera(&camera_);

	// スカイドーム生成
	skydome_ = new Skydome();
	skydome_->Initialize(modelSkydome_, &camera_);

	// カメラコントローラー
	cameraController_ = new CameraController();
	cameraController_->Initialize(&camera_);
	cameraController_->Settarget(player_);

	// 移動範囲の指定
	CameraController::Rect movableArea = {
	    .left = 14.0f,
	    .right = 85.0f,
	    .bottom = 8.0f,
	    .top = 11.0f,
	};

	cameraController_->SetMovableArea(movableArea);
	cameraController_->Reset();

	// 初期カメラを反映
	cameraController_->Update();
	camera_.UpdateMatrix();

	// ======================
	// フェード初期化
	// ======================
	fade_ = new Fade();
	fade_->Initialize();

	// ゲーム開始時はフェードイン
	phase_ = Phase::kFadeIn;
	finished_ = false;

	fade_->Start(Fade::Status::FadeIn, kFadeDuration);


}

// ======================
// デストラクタ（終了処理）
// ======================
GameScene::~GameScene() {

	// モデル解放
	delete model_;
	delete modelBlock_; 
	delete modelSkydome_;

	// プレイヤー解放
	delete player_;

	//敵解放
	for (BaseEnemy* enemy : enemies_) {
		delete enemy;
	}

	enemies_.clear();

	// ======================
	// やあられパーティクル解放
	// ======================
	if (deathParticles_) {
		delete deathParticles_;
		deathParticles_ = nullptr;
	}

	// ======================
	// ブロック解放
	// ======================
	for (auto& line : worldTransformBlocks_) {
		for (auto& wt : line) {
			delete wt; 
			wt = nullptr;
		}
	}
	worldTransformBlocks_.clear();

	// デバッグカメラ解放
	delete debugCamera_;

	// マップチップフィールド解放
	delete mapChipField_;

	// フェード解放
	delete fade_;
	fade_ = nullptr;

	delete playerModel_;
	delete attackModel_;
	delete enemyModel_;
	delete shieldEnemyModel_;
	delete deathParticlesModel_;

	// ヒットエフェクト解放
	for (BaseEffect* effect : effects_) {
		delete effect;
	}

	effects_.clear();

	delete hitEffectModel_;
	delete guardEffectModel_;
}

// ======================
// 更新処理
// ======================
void GameScene::Update() {

	ChangePhase();

	switch (phase_) {

case Phase::kFadeIn:

		// スカイドーム更新
		skydome_->Update();

		// プレイヤー更新
		player_->Update();

		// 敵更新
		for (BaseEnemy* enemy : enemies_) {
			enemy->Update();
		}

		// エフェクト更新
		for (BaseEffect* effect : effects_) {
			effect->Update();
		}


		// ブロックのワールド行列更新
		for (auto& line : worldTransformBlocks_) {
			for (auto& wt : line) {

				if (!wt) {
					continue;
				}

				UpdateWorldTransform(*wt);
			}
		}

		// カメラ更新
		cameraController_->Update();
		camera_.UpdateMatrix();

		// フェード更新
		fade_->Update();

		break;

	case Phase::kPlay:

	// スカイドーム更新
	skydome_->Update();

	// プレイヤー更新
	player_->Update();

	// 敵更新
	for (BaseEnemy* enemy : enemies_) {
		enemy->Update();
	}

	// カメラ更新
	cameraController_->Update();
	camera_.UpdateMatrix();

	// ブロック更新
	for (auto& line : worldTransformBlocks_) {
		for (auto& wt : line) {

			if (!wt) {
				continue;
			}

			UpdateWorldTransform(*wt);
		}
	}

	// すべての当たり判定
	CheckAllCollisions();

	// エフェクト更新
	for (BaseEffect* effect : effects_) {
		effect->Update();
	}

	// 終了したエフェクトを削除
	effects_.remove_if([](BaseEffect* effect) {
		if (effect->IsDead()) {
			delete effect;
			return true;
		}

		return false;
	});

	// 死亡した敵を削除
	enemies_.remove_if([](BaseEnemy* enemy) {
		if (enemy->IsDead()) {
			delete enemy;
			return true;
		}

		return false;
	});

	break;

	case Phase::kDeath:

		// スカイドーム更新
		skydome_->Update();

		// 敵更新
		for (BaseEnemy* enemy : enemies_) {
			enemy->Update();
		}

		// やられパーティクル更新
		if (deathParticles_) {
			deathParticles_->Update();
		}

		// カメラ更新
		camera_.UpdateMatrix();

		// ブロック更新
		for (auto& line : worldTransformBlocks_) {
			for (auto& wt : line) {

				if (!wt) {
					continue;
				}

				UpdateWorldTransform(*wt);
			}
		}

		break;

	case Phase::kFadeOut:

		// フェードアウト更新
		fade_->Update();

		break;
	}

#ifdef _DEBUG
	// ====================== 
	// デバッグカメラ切り替え 
	// ======================
	if (Input::GetInstance()->TriggerKey(DIK_F1)) {
		isDebugCameraActive_ = !isDebugCameraActive_;
	}
#endif

// ====================== 
// カメラ適用 
// ======================
if (isDebugCameraActive_) {
	debugCamera_->Update();
	camera_.matView = debugCamera_->GetCamera().matView;
	camera_.matProjection = debugCamera_->GetCamera().matProjection;
	camera_.TransferMatrix();
	} else {
	camera_.UpdateMatrix();
	}


	#ifdef _DEBUG

	ImGui::Begin("GameScene");

	if (ImGui::Button("Reload Map")) {
		reloadRequested_ = true;
	}

	ImGui::End();

	#endif
}


// ======================
// 描画処理
// ======================
void GameScene::Draw() {

	// 描画開始
	Model::PreDraw();

	// ======================
	// ブロック描画
	// ======================
	for (auto& line : worldTransformBlocks_) {
		for (auto& wt : line) {
			if (!wt)
				continue;
				
			modelBlock_->Draw(*wt, camera_);
		}
	}


	//スカイドーム描画
	skydome_->Draw();

	for (BaseEnemy* enemy : enemies_) {
		enemy->Draw();
	}

	// エフェクト描画
	for (BaseEffect* effect : effects_) {
		effect->Draw();
	}

	player_->Draw();


	// やあられパーティクル描画
	if (deathParticles_) {
		deathParticles_->Draw();
	}

	fade_->Draw();

	// 描画終了
	Model::PostDraw();
}


void GameScene::CheckAllCollisions() {

 AABB playerAABB = player_->GetAABB();

	//自キャラと敵弾全ての当たり判定
	for (BaseEnemy* enemy : enemies_) {

		if (enemy->IsCollisionDisabled()) {
			continue;	//コリジョン無効の敵はスキップ
		}

		//敵弾の座標
		AABB enemyAABB = enemy->GetAABB();

		//AABB同士の交差判定
		if (IsCollision(playerAABB, enemyAABB)) {

			//自キャラの衝突時間判定関数を呼び出す
			player_->OnCollision(enemy);

			// 敵の衝突判定時間関数を呼び出す
			enemy->OnCollision(player_);
		}
	}
}

void GameScene::ChangePhase() {

	switch (phase_) {

	case Phase::kFadeIn:

		// フェードイン終了後にゲーム開始
		if (fade_->IsFinished()) {
			fade_->Stop();
			phase_ = Phase::kPlay;
		}

		break;

	case Phase::kPlay:

		// プレイヤーが死亡したらデス演出へ
		if (player_->IsDead()) {

			phase_ = Phase::kDeath;

			const KamataEngine::Vector3& deathParticlePosition = player_->GetWorldPosition();

			deathParticles_->Initialize(deathParticlesModel_, &camera_, deathParticlePosition);
		}

		break;

	case Phase::kDeath:

		// デス演出終了後にフェードアウト開始
		if (deathParticles_ && deathParticles_->IsFinished()) {

			fade_->Start(Fade::Status::FadeOut, kFadeDuration);

			phase_ = Phase::kFadeOut;
		}

		break;

	case Phase::kFadeOut:

		// フェードアウト終了後にゲームシーンを終了
		if (fade_->IsFinished()) {
			finished_ = true;
		}

		break;
	}
}

void GameScene::CreateHitEffect(const Vector3& position) {

	BaseEffect* effect = HitEffect::Create(position);

	effects_.push_back(effect);
}

void GameScene::CreateGuardEffect(const Vector3& position) {

	BaseEffect* effect = GuardEffect::Create(position);

	effects_.push_back(effect);
}