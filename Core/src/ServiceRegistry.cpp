#include "Core/ServiceRegistry.h"

#include <cstdio>

void ServiceRegistry::Register(const std::string& name, void* service) {
	if (services_.count(name) > 0) {
		printf("Service already registered: %s\n", name.c_str());
		return;
	}
	services_[name] = service;
}

void* ServiceRegistry::Get(const std::string& name) {
	if (services_.count(name) == 0) {
		printf("Service not registered: %s\n", name.c_str());
		return nullptr;
	}
	return services_[name];
}
