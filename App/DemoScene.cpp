#include "DemoScene.h"

#include "Asset/PointCloudShapes.h"
#include "Interface/ICollisionListener.h"
#include "Physics/ColliderComponent.h"
#include "Scene/Camera.h"
#include "Scene/DebugCamera.h"
#include "Scene/DirectionalLight.h"
#include "Scene/PointCloudRenderer.h"

#include <json.hpp>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <random>
#include <system_error>
#include <vector>

#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI

using namespace KujataEngine;

// アクションの名前。
static const char* const kMoveAction = "Move";
static const char* const kFireAction = "Fire";
static const char* const kLookAction = "Look";
static const char* const kLookEnabledAction = "LookEnabled";

// 衝突の絞り込みに使うLayerの番号。
static const uint32_t kBallLayer = 2;
static const uint32_t kCubeLayer = 3;

// 目の高さと、1秒に歩く距離。
static const float kEyeHeight = 1.6f;
static const float kWalkSpeed = 4.0f;

// マウスを1ピクセル動かしたときに回る角度で、単位はラジアン。
static const float kLookRadiansPerPixel = 0.003f;

// 真上と真下を越えて裏返らないようにする、上下の向きの限界で、単位はラジアン。
static const float kMaxPitch = 1.4f;

/// <summary>
/// 点群の見た目の調整項目で、調整ウィンドウで書き換えてPointCloudRendererへ写す。
/// </summary>
struct PointCloudLook {
	// 点の板の中心から端までの長さ。
	float pointSize;

	// 点1つが足す明るさ。
	float intensity;

	// 点が揺れ続ける幅。
	float floatDistance;

	// 形になっていない点の明るさの倍率。
	float scatteredBrightness;

	// 描く点の割合。
	float density;
};

// 球の見た目の初期値で、崩れた点が残らないように形でなくなった点は消す。
static const PointCloudLook kBallLook = {0.01f, 0.026f, 0.05f, 0.0f, 1.0f};

// 立方体の見た目の初期値で、点が密なので明るさを球より小さくする。
static const PointCloudLook kCubeLook = {0.01f, 0.013f, 0.05f, 1.0f, 1.0f};

/// <summary>
/// 調整項目の値を、点群を描くコンポーネントへ写す。
/// </summary>
static void ApplyLook(PointCloudRenderer* renderer, const PointCloudLook& look) {
	renderer->pointSize = look.pointSize;
	renderer->intensity = look.intensity;
	renderer->floatDistance = look.floatDistance;
	renderer->scatteredBrightness = look.scatteredBrightness;
	renderer->density = look.density;
}

// 調整ウィンドウの値を保存するファイルの名前で、データのフォルダに置く。
static const char* const kTuningFileName = "PointCloudTuning.json";

// 見た目の調整項目とJSONを、メンバーと同じ名前の項目で変換できるようにする。
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PointCloudLook, pointSize, intensity, floatDistance, scatteredBrightness, density)

/// <summary>
/// マウスが調整ウィンドウの上にあるかを返し、imguiがない構成では常にfalseを返す。
/// </summary>
static bool IsMouseOnDebugWindow() {
#ifdef USE_IMGUI
	return ImGui::GetIO().WantCaptureMouse;
#else
	return false;
#endif // USE_IMGUI
}

// 球の半径と点の数と、1秒に進む距離。
static const float kBallRadius = 0.25f;
static const uint32_t kBallPointCount = 30000;
static const float kBallSpeed = 12.0f;

// 球と立方体の点の色。
static const Vector3 kBallColor = {1.0f, 0.75f, 0.4f};
static const Vector3 kCubeColor = {0.35f, 0.75f, 1.0f};

// 球の中に目が入らないように、目からこの距離だけ前に球を出す。
static const float kBallSpawnDistance = 1.0f;

// 何にも当たらなかった球が崩れ始めるまでの秒数。
static const float kBallFlySeconds = 2.5f;

// 崩れた球の点が消えきって、球を消すまでの秒数。
static const float kBallCrumbleSeconds = 3.0f;

// 撃つアクションを押し続けている間に、次の球を撃つまでの秒数。
static const float kFireIntervalSeconds = 0.2f;

// 立方体の数と一辺の長さと、1個の点の数。
static const size_t kCubeCount = 5;
static const float kCubeSize = 1.2f;
static const uint32_t kCubePointCount = 150000;

// 立方体を置く範囲の、最小の角と最大の角。
static const Vector3 kCubeAreaMin = {-3.75f, 1.875f, 5.75f};
static const Vector3 kCubeAreaMax = {3.75f, 4.125f, 10.25f};

// 立方体どうしが重ならないように空ける、中心の間の距離。
static const float kCubeMinDistance = 2.5f;

// 重ならない場所を探し直す回数の上限。
static const int kCubePlaceTryCount = 30;

// 全部崩れてから、別の場所に集まり直すまでの秒数。
static const float kRegatherDelaySeconds = 1.5f;

// フィールドを覆う塵の球の中心と半径と、密度を上げられるように多めに作る点の数。
static const Vector3 kDustCenter = {0.0f, 3.0f, 8.0f};
static const float kDustRadius = 30.0f;
static const uint32_t kDustPointCount = 400000;

// 塵の見た目の初期値で、遠くてまばらなので点を大きく明るくし、作った点の一部だけを描く。
static const PointCloudLook kDustLook = {0.03f, 0.6f, 0.3f, 1.0f, 0.2f};

// 塵の色の範囲の両端で、点ごとにこの間の色をランダムに選ぶ。
static const Vector3 kDustBlue = {0.1f, 0.25f, 1.0f};
static const Vector3 kDustPurple = {0.6f, 0.15f, 1.0f};

/// <summary>
/// 点ごとに青から紫の間の色を付けた、塵の球の点群を作る。
/// </summary>
static PointCloudData CreateDustPointCloud() {
	PointCloudData dust = CreateSpherePointCloud(kDustPointCount, kDustRadius, kDustBlue);

	std::mt19937 randomEngine(std::random_device{}());
	std::uniform_real_distribution<float> purpleAmount(0.0f, 1.0f);
	for (PointCloudTarget& target : dust.targets) {
		target.color = kDustBlue + (kDustPurple - kDustBlue) * purpleAmount(randomEngine);
	}
	return dust;
}

/// <summary>
/// 点でできた球で、まっすぐ飛び、崩れると点がその場に散りながらばらばらに消える。
/// </summary>
class PointBall : public Component {
public:
	PointBall(ITime* time, PointCloudRenderer* renderer, ColliderComponent* collider, const Vector3& velocity) {
		time_ = time;
		renderer_ = renderer;
		collider_ = collider;
		velocity_ = velocity;
	}

	void Update() override {
		const float deltaTime = time_->GetDeltaTime();

		if (isCrumbling_) {
			// 点が全部消えるのを待ってから、球を消す。
			crumbleSeconds_ += deltaTime;
			if (crumbleSeconds_ >= kBallCrumbleSeconds) {
				GetOwner()->Destroy();
			}
			return;
		}

		GetOwner()->GetTransform().translation_ += velocity_ * deltaTime;

		// 何にも当たらなかった球が飛び続けないように、時間で崩す。
		flySeconds_ += deltaTime;
		if (flySeconds_ >= kBallFlySeconds) {
			Crumble();
		}
	}

	/// <summary>
	/// 球を止めて崩し、もう何にも当たらないようにする。
	/// </summary>
	void Crumble() {
		if (isCrumbling_) {
			return;
		}
		isCrumbling_ = true;
		renderer_->Crumble();
		collider_->SetEnabled(false);
	}

	bool IsCrumbling() const { return isCrumbling_; }

private:
	ITime* time_;
	PointCloudRenderer* renderer_;
	ColliderComponent* collider_;
	Vector3 velocity_;
	bool isCrumbling_ = false;
	float flySeconds_ = 0.0f;
	float crumbleSeconds_ = 0.0f;
};

/// <summary>
/// 点でできた立方体で、球が当たると球と一緒に崩れる。
/// </summary>
class PointCube : public Component, public ICollisionListener {
public:
	PointCube(PointCloudRenderer* renderer, ColliderComponent* collider) {
		renderer_ = renderer;
		collider_ = collider;
	}

	void OnTriggerEnter(GameObject* other) override {
		// 球のほかの物や、もう崩れている球には反応しない。
		PointBall* ball = other->GetComponent<PointBall>();
		if (ball == nullptr || ball->IsCrumbling() || isCrumbled_) {
			return;
		}

		ball->Crumble();
		isCrumbled_ = true;
		renderer_->Crumble();

		// 崩れている間は、次の球が当たらないようにする。
		collider_->SetEnabled(false);
	}

	/// <summary>
	/// 指定の位置へ移し、点を集め直して、また球が当たるようにする。
	/// </summary>
	void Regather(const Vector3& position) {
		GetOwner()->GetTransform().translation_ = position;
		renderer_->GatherAll();
		collider_->SetEnabled(true);
		isCrumbled_ = false;
	}

	bool IsCrumbled() const { return isCrumbled_; }

	PointCloudRenderer* GetRenderer() { return renderer_; }

private:
	PointCloudRenderer* renderer_;
	ColliderComponent* collider_;
	bool isCrumbled_ = false;
};

/// <summary>
/// 球と立方体と塵の点群の見た目を調整するウィンドウを出し、保存したファイルが書き換わると読み直す。
/// </summary>
class PointCloudTuning : public Component {
public:
	PointCloudTuning(const std::vector<PointCube*>& cubes, PointCloudRenderer* dustRenderer, const std::filesystem::path& filePath) {
		cubes_ = cubes;
		dustRenderer_ = dustRenderer;
		filePath_ = filePath;
		ReloadIfFileChanged();
	}

	void Update() override {
		ReloadIfFileChanged();
		DrawWindow();

		// 立方体と塵は作り直さないので、変えた値をすぐ見られるように毎フレーム写す。
		for (PointCube* cube : cubes_) {
			ApplyLook(cube->GetRenderer(), cubeLook_);
		}
		ApplyLook(dustRenderer_, dustLook_);
	}

	/// <summary>
	/// 次に撃つ球から使う見た目を返す。
	/// </summary>
	const PointCloudLook& GetBallLook() const { return ballLook_; }

private:
	/// <summary>
	/// ファイルが前に見たときから書き換わっていたら、値を読む。
	/// </summary>
	void ReloadIfFileChanged() {
		// ファイルがないのは普通のことなので、例外を投げない読み方にする。
		std::error_code errorCode;
		const std::filesystem::file_time_type writeTime = std::filesystem::last_write_time(filePath_, errorCode);
		if (writeTime == loadedWriteTime_) {
			return;
		}
		loadedWriteTime_ = writeTime;
		Load();
	}

	/// <summary>
	/// 保存したファイルから値を読む。
	/// </summary>
	void Load() {
		std::ifstream file(filePath_);
		if (!file.is_open()) {
			return;
		}

		// 手で書き換えている途中の壊れたファイルで止まらないようにする。
		try {
			const nlohmann::json json = nlohmann::json::parse(file);
			ballLook_ = json.at("Ball");
			cubeLook_ = json.at("Cube");
			dustLook_ = json.at("Dust");
			printf("点群の調整のファイルを読みました: %s\n", kTuningFileName);
		} catch (const nlohmann::json::exception&) {
			printf("点群の調整のファイルを読めませんでした: %s\n", kTuningFileName);
		}
	}

	/// <summary>
	/// 今の値をファイルへ書き出す。
	/// </summary>
	void Save() {
		const nlohmann::json json = {{"Ball", ballLook_}, {"Cube", cubeLook_}, {"Dust", dustLook_}};
		std::ofstream file(filePath_);
		file << json.dump(4);
	}

	/// <summary>
	/// 調整ウィンドウを組み、imguiがない構成では何もしない。
	/// </summary>
	void DrawWindow() {
#ifdef USE_IMGUI
		// 初めて出すときだけ、項目の名前が切れない幅にする。
		ImGui::SetNextWindowSize({380.0f, 0.0f}, ImGuiCond_FirstUseEver);
		ImGui::Begin("Point Cloud");
		DrawLookSliders("Ball", &ballLook_);
		DrawLookSliders("Cube", &cubeLook_);
		DrawLookSliders("Dust", &dustLook_);

		ImGui::Separator();
		if (ImGui::Button("Save")) {
			Save();
		}
		ImGui::End();
#endif // USE_IMGUI
	}

#ifdef USE_IMGUI
	/// <summary>
	/// 見出しの下に、見た目の調整項目のスライダーを並べる。
	/// </summary>
	void DrawLookSliders(const char* title, PointCloudLook* look) {
		ImGui::SeparatorText(title);

		// 見出しが違う同じ名前のスライダーが、別の物として扱われるようにする。
		ImGui::PushID(title);
		ImGui::SliderFloat("Density", &look->density, 0.0f, 1.0f, "%.2f");
		ImGui::SliderFloat("Point Size", &look->pointSize, 0.001f, 0.05f, "%.3f");
		ImGui::SliderFloat("Intensity", &look->intensity, 0.001f, 1.0f, "%.3f", ImGuiSliderFlags_Logarithmic);
		ImGui::SliderFloat("Float Distance", &look->floatDistance, 0.0f, 0.3f, "%.3f");
		ImGui::SliderFloat("Scattered Brightness", &look->scatteredBrightness, 0.0f, 1.0f, "%.2f");
		ImGui::PopID();
	}
#endif // USE_IMGUI

	std::vector<PointCube*> cubes_;
	PointCloudRenderer* dustRenderer_;
	std::filesystem::path filePath_;

	// 前に見たときのファイルの時刻で、今の時刻と違えば書き換わったと見る。
	std::filesystem::file_time_type loadedWriteTime_;
	PointCloudLook ballLook_ = kBallLook;
	PointCloudLook cubeLook_ = kCubeLook;
	PointCloudLook dustLook_ = kDustLook;
};

/// <summary>
/// 立方体をランダムな位置に置き、全部崩れたら少し待ってから別の位置に集め直す。
/// </summary>
class CubeField : public Component {
public:
	CubeField(ITime* time, const std::vector<PointCube*>& cubes) {
		time_ = time;
		cubes_ = cubes;

		// 起動のたびに違う並びになるように、乱数の種を実行のたびに変える。
		std::random_device seedSource;
		randomEngine_.seed(seedSource());
	}

	void Start() override {
		PlaceAllCubes();
	}

	void Update() override {
		if (!AreAllCubesCrumbled()) {
			return;
		}

		// 崩れる様子が見えるように、少し待ってから集め直す。
		waitSeconds_ += time_->GetDeltaTime();
		if (waitSeconds_ >= kRegatherDelaySeconds) {
			PlaceAllCubes();
			waitSeconds_ = 0.0f;
		}
	}

private:
	bool AreAllCubesCrumbled() const {
		for (const PointCube* cube : cubes_) {
			if (!cube->IsCrumbled()) {
				return false;
			}
		}
		return true;
	}

	/// <summary>
	/// 全部の立方体を、互いに重ならないランダムな位置で集め直す。
	/// </summary>
	void PlaceAllCubes() {
		std::vector<Vector3> placedPositions;
		for (PointCube* cube : cubes_) {
			const Vector3 position = PickFreePosition(placedPositions);
			placedPositions.push_back(position);
			cube->Regather(position);
		}
	}

	/// <summary>
	/// 置いた立方体から離れた位置を探し、決めた回数で見つからなければ最後の候補を返す。
	/// </summary>
	Vector3 PickFreePosition(const std::vector<Vector3>& placedPositions) {
		Vector3 candidate = {};
		for (int tryIndex = 0; tryIndex < kCubePlaceTryCount; tryIndex++) {
			candidate = PickRandomPosition();
			if (IsFarFromAll(candidate, placedPositions)) {
				break;
			}
		}
		return candidate;
	}

	Vector3 PickRandomPosition() {
		std::uniform_real_distribution<float> unitDistribution(0.0f, 1.0f);
		Vector3 position;
		position.x = Lerp(kCubeAreaMin.x, kCubeAreaMax.x, unitDistribution(randomEngine_));
		position.y = Lerp(kCubeAreaMin.y, kCubeAreaMax.y, unitDistribution(randomEngine_));
		position.z = Lerp(kCubeAreaMin.z, kCubeAreaMax.z, unitDistribution(randomEngine_));
		return position;
	}

	bool IsFarFromAll(const Vector3& position, const std::vector<Vector3>& placedPositions) const {
		for (const Vector3& placedPosition : placedPositions) {
			if (Length(position - placedPosition) < kCubeMinDistance) {
				return false;
			}
		}
		return true;
	}

	ITime* time_;
	std::vector<PointCube*> cubes_;
	std::mt19937 randomEngine_;
	float waitSeconds_ = 0.0f;
};

/// <summary>
/// 目になるカメラを歩かせて見回し、撃つアクションを押している間、マウスが指す方向へ点の球を撃ち続ける。
/// </summary>
class EyeController : public Component {
public:
	EyeController(Scene* scene, IInputActions* inputActions, ITime* time, IWindow* window, Camera* camera, PointCloudHandle ballShape, const PointCloudTuning* tuning) {
		tuning_ = tuning;
		scene_ = scene;
		inputActions_ = inputActions;
		time_ = time;
		window_ = window;
		camera_ = camera;
		ballShape_ = ballShape;
	}

	void Update() override {
		// デバッグカメラの接続中はカメラが無効になるので、その間は同じキーで目が動かないようにする。
		if (!camera_->IsEnabled()) {
			return;
		}

		WorldTransform& transform = GetOwner()->GetTransform();

		// 調整ウィンドウを触っている間は、同じマウスの操作で見回したり撃ったりしない。
		const bool canUseMouse = !IsMouseOnDebugWindow();

		// マウスの右を押している間だけ、マウスの動きで見回す。
		if (canUseMouse && inputActions_->IsHeld(kLookEnabledAction)) {
			const Vector2 look = inputActions_->GetAxis2D(kLookAction);
			transform.rotation_.y += look.x * kLookRadiansPerPixel;
			transform.rotation_.x = std::clamp(transform.rotation_.x + look.y * kLookRadiansPerPixel, -kMaxPitch, kMaxPitch);
		}

		// 上や下を向いていても同じ高さを歩くように、Y軸まわりの向きだけで前と右を決める。
		const Vector3 forward = {std::sin(transform.rotation_.y), 0.0f, std::cos(transform.rotation_.y)};
		const Vector3 right = {forward.z, 0.0f, -forward.x};
		const Vector2 move = inputActions_->GetAxis2D(kMoveAction);
		const float distance = kWalkSpeed * time_->GetDeltaTime();
		transform.translation_ += forward * (move.y * distance);
		transform.translation_ += right * (move.x * distance);

		secondsSinceFire_ += time_->GetDeltaTime();
		// 押している間は、決めた間隔で撃ち続ける。
		if (canUseMouse && inputActions_->IsHeld(kFireAction) && secondsSinceFire_ >= kFireIntervalSeconds) {
			SpawnBall();
			secondsSinceFire_ = 0.0f;
		}
	}

private:
	/// <summary>
	/// 目の少し前に点の球を作り、マウスが指す方向へ飛ばす。
	/// </summary>
	void SpawnBall() {
		const WorldTransform& eye = GetOwner()->GetTransform();
		const Vector3 direction = ComputeAimDirection();

		GameObject* ball = scene_->CreateGameObject();
		ball->SetLayer(kBallLayer);
		ball->GetTransform().translation_ = eye.translation_ + direction * kBallSpawnDistance;

		PointCloudRenderer* renderer = new PointCloudRenderer(ballShape_);
		ApplyLook(renderer, tuning_->GetBallLook());
		ball->AddComponent(renderer);

		// 押し出しは要らず重なりだけを知りたいので、トリガーにする。
		SphereColliderComponent* collider = new SphereColliderComponent();
		collider->SetRadius(kBallRadius);
		collider->SetTrigger(true);
		collider->SetCollisionMask(1u << kCubeLayer);
		ball->AddComponent(collider);

		ball->AddComponent(new PointBall(time_, renderer, collider, direction * kBallSpeed));
	}

	/// <summary>
	/// 目から、画面のマウスが指す点の奥へ向かう向きを返し、決まらないときは目の正面を返す。
	/// </summary>
	Vector3 ComputeAimDirection() {
		const WorldTransform& eye = GetOwner()->GetTransform();
		const Vector3 eyeForward = TransformNormal({0.0f, 0.0f, 1.0f}, MakeRotateMatrix(eye.rotation_));

		const float width = static_cast<float>(window_->GetClientWidth());
		const float height = static_cast<float>(window_->GetClientHeight());
		// 最小化中は大きさが0になり、縦横比が求まらない。
		if (width <= 0.0f || height <= 0.0f) {
			return eyeForward;
		}

		// カメラの行列は描画の直前に更新されるので、ここでは今映っている絵のものになる。
		const Matrix4x4 view = Inverse(eye.matWorld_);
		const Matrix4x4 projection = MakePerspectiveFovMatrix(camera_->fovYRadians, width / height, camera_->nearZ, camera_->farZ);
		const Matrix4x4 viewport = MakeViewportMatrix(0.0f, 0.0f, width, height, 0.0f, 1.0f);
		// 描画と同じ行列の逆で、画面の点を世界へ戻す。
		const Matrix4x4 screenToWorld = Inverse(view * projection * viewport);

		// 深さ0が写す範囲の手前の面に、1が奥の面に当たる。
		const Vector2 mousePosition = inputActions_->GetMousePosition();
		const Vector3 nearPoint = Transform({mousePosition.x, mousePosition.y, 0.0f}, screenToWorld);
		const Vector3 farPoint = Transform({mousePosition.x, mousePosition.y, 1.0f}, screenToWorld);
		return Normalize(farPoint - nearPoint);
	}

	Scene* scene_;
	IInputActions* inputActions_;
	ITime* time_;
	IWindow* window_;
	Camera* camera_;
	PointCloudHandle ballShape_;
	const PointCloudTuning* tuning_;

	// 起動してすぐ撃てるように、間隔より長い値から始める。
	float secondsSinceFire_ = kFireIntervalSeconds;
};

/// <summary>
/// 歩く、見回す、撃つ操作のアクションを登録する。
/// </summary>
static void RegisterEyeActions(IInputActions* inputActions) {
	inputActions->AddAxis2D(kMoveAction);
	inputActions->BindKeys2D(kMoveAction, Key::W, Key::S, Key::A, Key::D);
	inputActions->BindKeys2D(kMoveAction, Key::Up, Key::Down, Key::Left, Key::Right);
	inputActions->BindPadStick(kMoveAction, PadStick::Left);

	inputActions->AddButton(kFireAction);
	inputActions->BindKey(kFireAction, Key::Space);
	inputActions->BindPadButton(kFireAction, PadButton::A);
	inputActions->BindMouseButton(kFireAction, MouseButton::Left);

	inputActions->AddAxis2D(kLookAction);
	inputActions->BindMouseMove(kLookAction);

	inputActions->AddButton(kLookEnabledAction);
	inputActions->BindMouseButton(kLookEnabledAction, MouseButton::Right);
}

/// <summary>
/// 光がないというログを出さないための光を置く。
/// </summary>
static void CreateLight(Scene* scene) {
	GameObject* light = scene->CreateGameObject();
	light->GetTransform().rotation_ = {0.9f, -0.5f, 0.0f};
	light->AddComponent(new DirectionalLight());
}

/// <summary>
/// フィールドを覆う塵の球を置く。
/// </summary>
static PointCloudRenderer* CreateDust(Scene* scene, IAssetLibrary* assets) {
	PointCloudRenderer* renderer = new PointCloudRenderer(assets->AddPointCloud(CreateDustPointCloud()));
	ApplyLook(renderer, kDustLook);

	// 散ったままにして塵に見せる。
	renderer->initialSettle = 0.0f;

	GameObject* dust = scene->CreateGameObject();
	dust->GetTransform().translation_ = kDustCenter;
	dust->AddComponent(renderer);
	return renderer;
}

/// <summary>
/// 点の立方体を1個置き、位置はCubeFieldが決める。
/// </summary>
static PointCube* CreatePointCube(Scene* scene, PointCloudHandle shape) {
	PointCloudRenderer* renderer = new PointCloudRenderer(shape);
	ApplyLook(renderer, kCubeLook);

	// 散った点が集まる所から見せる。
	renderer->initialSettle = 0.0f;

	BoxColliderComponent* collider = new BoxColliderComponent();
	collider->SetSize({kCubeSize, kCubeSize, kCubeSize});
	collider->SetCollisionMask(1u << kBallLayer);

	PointCube* cube = new PointCube(renderer, collider);

	GameObject* cubeObject = scene->CreateGameObject();
	cubeObject->SetLayer(kCubeLayer);
	cubeObject->AddComponent(renderer);
	cubeObject->AddComponent(collider);
	cubeObject->AddComponent(cube);
	return cube;
}

/// <summary>
/// 同じ形の点の立方体を、決めた数だけ置く。
/// </summary>
static std::vector<PointCube*> CreatePointCubes(Scene* scene, IAssetLibrary* assets) {
	const PointCloudHandle shape = assets->AddPointCloud(CreateBoxPointCloud(kCubePointCount, {kCubeSize, kCubeSize, kCubeSize}, kCubeColor));

	std::vector<PointCube*> cubes;
	for (size_t i = 0; i < kCubeCount; i++) {
		cubes.push_back(CreatePointCube(scene, shape));
	}
	return cubes;
}

/// <summary>
/// 目になるカメラを置き、球を撃てるようにする。
/// </summary>
static Camera* CreateEye(Scene* scene, IAssetLibrary* assets, IInputActions* inputActions, ITime* time, IWindow* window, const PointCloudTuning* tuning) {
	const PointCloudHandle ballShape = assets->AddPointCloud(CreateSpherePointCloud(kBallPointCount, kBallRadius, kBallColor));
	Camera* camera = new Camera();

	GameObject* eye = scene->CreateGameObject();
	eye->GetTransform().translation_ = {0.0f, kEyeHeight, 0.0f};
	eye->AddComponent(camera);
	eye->AddComponent(new EyeController(scene, inputActions, time, window, camera, ballShape, tuning));
	return camera;
}

/// <summary>
/// ターミナルのdebugcameraで切り替わるデバッグカメラを置く。
/// </summary>
static void CreateDebugCamera(Scene* scene, IInputActions* inputActions, ITime* time, IConsoleCommands* consoleCommands, Camera* mainCamera) {
	DebugCamera* debugCamera = new DebugCamera(inputActions, time, mainCamera);
	scene->CreateGameObject()->AddComponent(debugCamera);

	consoleCommands->AddCommand("debugcamera", [debugCamera]() {
		debugCamera->Toggle();
	});
}

void BuildDemoScene(Scene* scene, IAssetLibrary* assets, IInputActions* inputActions, ITime* time, IWindow* window, IConsoleCommands* consoleCommands) {
	RegisterEyeActions(inputActions);
	CreateLight(scene);

	PointCloudRenderer* dustRenderer = CreateDust(scene, assets);
	const std::vector<PointCube*> cubes = CreatePointCubes(scene, assets);
	scene->CreateGameObject()->AddComponent(new CubeField(time, cubes));

	PointCloudTuning* tuning = new PointCloudTuning(cubes, dustRenderer, assets->GetDataDirectory() / kTuningFileName);
	scene->CreateGameObject()->AddComponent(tuning);

	Camera* camera = CreateEye(scene, assets, inputActions, time, window, tuning);
	CreateDebugCamera(scene, inputActions, time, consoleCommands, camera);
}
