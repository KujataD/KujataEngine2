#include "Core/ServiceRegistry.h"

#include "Core/Assert.h"

#include <cstdio>

namespace KujataEngine {

void ServiceRegistry::Register(const std::string& name, void* service) {
	KUJATA_ASSERT(service != nullptr);
	KUJATA_ASSERT(services_.count(name) == 0);

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
