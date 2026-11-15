#pragma once

#include "Camera.h"
#include "Framework/Component.h"
#include "Interface/IInputActions.h"
#include "Interface/IRenderable.h"
#include "Interface/ITime.h"

namespace KujataEngine {

/// <summary>
/// 接続すると元のカメラの位置から始まり、ゲームの時間を止めて自由に見て回れるデバッグ用のカメラ。
/// </summary>
class DebugCamera : public Component, public IRenderable {
public:
	/// <summary>
	/// 操作に使うアクションを登録するので、準備の段階で作る。
	/// </summary>
	DebugCamera(IInputActions* inputActions, ITime* time, Camera* mainCamera);

	/// <summary>
	/// 接続していなければ接続し、接続していれば解除して元のカメラに戻す。
	/// </summary>
	void Toggle();

	void Update() override;
	void SubmitRender(IRenderQueue* renderQueue) override;

private:
	/// <summary>
	/// 元のカメラの位置と向きを写し取り、元のカメラを止めてゲームの時間も止める。
	/// </summary>
	void Attach();

	/// <summary>
	/// 元のカメラとゲームの時間を、接続する前の状態に戻す。
	/// </summary>
	void Detach();

	IInputActions* inputActions_;
	ITime* time_;
	Camera* mainCamera_;

	bool isAttached_ = false;

	// 解除のときに戻すための、接続する前の時間の速さ。
	float timeScaleBeforeAttach_ = 1.0f;
};

} // namespace KujataEngine
