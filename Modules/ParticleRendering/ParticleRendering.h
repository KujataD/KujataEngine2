#pragma once

#include "Core/IModule.h"
#include "Interface/IParticleRenderer.h"

#include <cstdint>
#include <memory>
#include <vector>

namespace KujataEngine {

// 中身はParticleRendering.cppだけが知っていればよいので、クラスの名前だけを伝える。
class ParticleShapeTable;
class ParticleInstances;
class ParticleSimulationPass;
class ParticleDrawPass;

// 1フレームに動かして描ける粒子の数で、定数バッファの場所の数になる。
const uint32_t kMaxParticleRequestCount = 64;

/// <summary>
/// 粒子の形を登録し描くように頼む窓口を出し、頼まれた粒子を動かす工程と描く工程を足すモジュール。
/// </summary>
class ParticleRendering : public IModule, public IParticleRenderer {
public:
	ParticleRendering();
	~ParticleRendering() override;

	std::vector<std::string> GetProvidedServices() override;
	std::vector<std::string> GetRequiredServices() override;
	void Startup(ServiceRegistry& services, FrameLoop& frameLoop) override;

	ParticleShapeHandle AddShape(const ParticleShape& shape) override;
	void SubmitParticles(const ParticleRequest& request) override;

private:
	std::vector<ParticleRequest> requests_;
	std::unique_ptr<ParticleShapeTable> shapes_;
	std::unique_ptr<ParticleInstances> instances_;
	std::unique_ptr<ParticleSimulationPass> simulationPass_;
	std::unique_ptr<ParticleDrawPass> drawPass_;
	bool hasWarnedTooManyRequests_ = false;
};

} // namespace KujataEngine
