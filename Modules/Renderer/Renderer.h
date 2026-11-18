#pragma once

#include "Core/IModule.h"

#include <memory>

namespace KujataEngine {

// 中身はRenderer.cppだけが知っていればよいので、クラスの名前だけを伝える。
class RenderSystem;

/// <summary>
/// 描画を担当するモジュールで、読む側には工程やGPUの部品を見せない。
/// </summary>
class Renderer : public IModule {
public:
	Renderer();
	~Renderer() override;

	std::vector<std::string> GetProvidedServices() override;
	std::vector<std::string> GetRequiredServices() override;
	void Startup(ServiceRegistry& services, FrameLoop& frameLoop) override;
	void Prepare() override;

private:
	std::unique_ptr<RenderSystem> system_;
};

} // namespace KujataEngine
