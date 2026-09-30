#pragma once

// ==============================
// インクルード
// ==============================
#include "KamataEngine.h"
#include "Player.h"
#include "BaseEnemy.h"
#include "Enemy.h"
#include "ShieldEnemy.h"
#include "Skydome.h"
#include "MapChipField.h"
#include "CameraController.h"
#include "DeathParticles.h"
#include "Fade.h"
#include "HitEffect.h"
#include "BaseEffect.h"
#include "GuardEffect.h"


// ==============================
// ゲームシーンクラス
// ==============================
class StageManager;
class GameScene {
public:
	// ==============================
	// コンストラクタ / デストラクタ
	// ==============================
	GameScene() = default;
	~GameScene();

	// ==============================
	// 基本処理
	// ==============================

	/// <summary>
	/// 初期化処理
	/// ゲーム開始時に一度だけ呼ばれる
	/// モデル・カメラ・プレイヤーなどの生成を行う
	/// </summary>
	void Initialize(StageManager* stageManager);

	/// <summary>
	/// 更新処理
	/// 毎フレーム呼ばれる
	/// 入力処理・座標更新・当たり判定などを行う
	/// </summary>
	void Update();

	/// <summary>
	/// 描画処理
	/// 毎フレーム呼ばれる
	/// 3Dモデルやプレイヤーの描画を行う
	/// </summary>
	void Draw();

	void GenerateFieldObjects();

	//全ての当たり判定を行う
	void CheckAllCollisions();

	void ChangePhase();

	// ==============================
	// ワールドトランスフォーム（ブロック用）
	// ==============================

	/// <summary>
	/// ブロックのワールド行列を2次元配列で管理
	/// [行][列] の形で配置されることを想定
	///
	/// ※ポインタ管理のため解放漏れに注意
	/// </summary>
	std::vector<std::vector<KamataEngine::WorldTransform*>> worldTransformBlocks_;


	// 敵をポインタリストに
	std::list<BaseEnemy*> enemies_;

	// ヒットエフェクト
	std::list<BaseEffect*> effects_;

	// ゲームのフェーズ(型)
	enum class Phase {
		kFadeIn,  // フェードイン
		kPlay,    // ゲームプレイ
		kDeath,   // デス演出
		kFadeOut, // フェードアウト
	};

	// ゲームのフェーズ(変数)
	Phase phase_;

	// デスフラグのgetter
	bool IsFinished() const { return finished_; }

	// エフェクトを生成
	void CreateHitEffect(const KamataEngine::Vector3& position);

	void CreateGuardEffect(const KamataEngine::Vector3& position);

	// リロード要求を取得
	bool IsReloadRequested() const { return reloadRequested_; }

private:
	// ==============================
	// テクスチャ
	// ==============================

	// UVチェック用テクスチャ
	uint32_t textureHandle_ = 0;

	// ==============================
	// 3Dモデル
	// ==============================

	// 共通モデル（例：プレイヤーなど）
	KamataEngine::Model* model_ = nullptr;

	// ブロック用モデル
	KamataEngine::Model* modelBlock_ = nullptr;

	// スカイドーム用モデル
	KamataEngine::Model* modelSkydome_ = nullptr;

	// プレイヤーモデル
	KamataEngine::Model* playerModel_ = nullptr;

	// アタックモデル
	KamataEngine::Model* attackModel_ = nullptr;

	// 敵モデル
	KamataEngine::Model* enemyModel_ = nullptr;

	KamataEngine::Model* shieldEnemyModel_ = nullptr;

	//やられパーティクル
	KamataEngine::Model* deathParticlesModel_ = nullptr;

	// ヒットエフェクト
	KamataEngine::Model* hitEffectModel_ = nullptr;

	// ガードエフェクト
	KamataEngine::Model* guardEffectModel_ = nullptr;

	// ==============================
	// カメラ
	// ==============================

	// 通常カメラ
	KamataEngine::Camera camera_;

	// デバッグカメラ（自由視点）
	KamataEngine::DebugCamera* debugCamera_ = nullptr;

	// デバッグカメラが有効かどうか
	bool isDebugCameraActive_ = false;



	CameraController* cameraController_ = nullptr;

	// ==============================
	// プレイヤー
	// ==============================

	/// <summary>
	/// 自キャラクラス
	/// ※newしている場合はdelete忘れに注意
	/// </summary>
	Player* player_ = nullptr;

	// ==============================
	// エネミー
	// ==============================
	//Enemy* enemy_ = nullptr;

	// ==============================
	// やられパーティクル
	// ==============================
	DeathParticles* deathParticles_ = nullptr;

	// ==============================
	// skydome
	// ==============================
	Skydome* skydome_ = nullptr;


	// ==============================
	// マップチップフィールド
	// ==============================
	MapChipField *mapChipField_;


	// 終了フラグ
	bool finished_ = false;

	// フェード
	Fade* fade_ = nullptr;

	// フェード時間
	const float kFadeDuration = 1.0f;


	// リロード要求フラグ
	bool reloadRequested_ = false;

	StageManager* stageManager_ = nullptr;

};