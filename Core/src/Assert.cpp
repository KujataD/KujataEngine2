#include "Core/Assert.h"

#include <Windows.h>
#include <cstdio>
#include <cstdlib>

namespace KujataEngine {

void PrintAssertFailure(const char* condition, const char* file, int line) {
	printf("アサート失敗: %s\n場所: %s(%d)\n", condition, file, line);
}

bool IsDebuggerAttached() {
	return IsDebuggerPresent() != FALSE;
}

void ExitAfterAssertFailure() {
	exit(EXIT_FAILURE);
}

} // namespace KujataEngine
