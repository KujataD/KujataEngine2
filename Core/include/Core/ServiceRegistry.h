#pragma once

#include <map>
#include <string>

namespace KujataEngine {

/// <summary>
/// モジュールが他のモジュールを名前で受け取るための登録簿。
/// </summary>
class ServiceRegistry {
public:
	/// <summary>
	/// 名前を付けてサービスを登録する。
	/// </summary>
	void Register(const std::string& name, void* service);

	/// <summary>
	/// 登録されていなければnullptrを返す。
	/// </summary>
	void* Get(const std::string& name);

private:
	// void*はどの型のポインタでも入る。受け取る側で元の型に戻して使う。
	std::map<std::string, void*> services_;
};

} // namespace KujataEngine
