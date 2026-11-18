#include "Renderer.h"

#include "RenderSystem.h"

namespace KujataEngine {

Renderer::Renderer() : system_(std::make_unique<RenderSystem>()) {}

// unique_ptrがRenderSystemを消すには中身が見えている必要があるので、ここで定義する。
Renderer::~Renderer() = default;

std::vector<std::string> Renderer::GetProvidedServices() {
	return system_->GetProvidedServices();
}

std::vector<std::string> Renderer::GetRequiredServices() {
	return system_->GetRequiredServices();
}

void Renderer::Startup(ServiceRegistry& services, FrameLoop& frameLoop) {
	system_->Startup(services, frameLoop);
}

void Renderer::Prepare() {
	system_->Prepare();
}

} // namespace KujataEngine
