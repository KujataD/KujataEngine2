#pragma once

#include "Interface/ParticleData.h"

namespace KujataEngine {

// ServiceRegistryに登録するときの名前。
const char* const kParticleRendererServiceName = "ParticleRenderer";

/// <summary>
/// 粒子の形を登録し、粒子を動かして描くように頼む窓口で、頼んだフレームにだけ描かれる。
/// </summary>
class IParticleRenderer {
public:
	virtual ~IParticleRenderer() = default;

	/// <summary>
	/// 粒が集まる形を登録して番号を返す。いつ登録しても、次に描くフレームからGPUで使える。空の形なら代わりの1粒を返す。
	/// </summary>
	virtual ParticleShapeHandle AddShape(const ParticleShape& shape) = 0;

	/// <summary>
	/// 粒子を1つ動かして描くように頼む。
	/// </summary>
	virtual void SubmitParticles(const ParticleRequest& request) = 0;
};

} // namespace KujataEngine
