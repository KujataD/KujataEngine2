#include "Core/ServiceRegistry.h"

#include <cstdio>

namespace KujataEngine {

void ServiceRegistry::Register(const std::string& name, void* service) {
	if (services_.count(name) > 0) {
		printf("サービスはすでに登録されています: %s\n", name.c_str());
		return;
	}
	services_[name] = service;
}

void* ServiceRegistry::Get(const std::string& name) {
	if (services_.count(name) == 0) {
		printf("サービスが登録されていません: %s\n", name.c_str());
		return nullptr;
	}
	return services_[name];
}

} // namespace KujataEngine
