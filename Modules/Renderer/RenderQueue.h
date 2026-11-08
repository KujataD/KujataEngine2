#pragma once

#include "Interface/IRenderQueue.h"

#include <vector>

namespace KujataEngine {

/// <summary>
/// Sceneから届いた描画の依頼を、描くまで預かるクラス。
/// </summary>
class RenderQueue : public IRenderQueue {
public:
	void SetCamera(const CameraData& camera) override;
	void SubmitCube(const Matrix4x4& world) override;

	bool HasCamera() const;
	const CameraData& GetCamera() const;
	const std::vector<Matrix4x4>& GetCubeWorlds() const;

	/// <summary>
	/// 次のフレームのために預かった依頼を空にする。
	/// </summary>
	void Clear();

private:
	CameraData camera_ = {};
	bool hasCamera_ = false;
	std::vector<Matrix4x4> cubeWorlds_;
};

} // namespace KujataEngine
