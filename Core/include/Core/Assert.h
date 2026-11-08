#pragma once

namespace KujataEngine {

/// <summary>
/// 失敗した条件と場所をコンソールに出す。
/// </summary>
void PrintAssertFailure(const char* condition, const char* file, int line);

/// <summary>
/// デバッガーが付いていればtrueを返す。
/// </summary>
bool IsDebuggerAttached();

/// <summary>
/// アサートの失敗でプログラムを終了する。
/// </summary>
void ExitAfterAssertFailure();

} // namespace KujataEngine

// Releaseでは行ごと消えるので、必ず実行したい処理を条件の中に書かない。
#ifdef NDEBUG

#define KUJATA_ASSERT(condition) ((void)0)

#else

// 関数の中で止めると1つ上の階層になるので、失敗した行で止まるようにマクロで書く。
#define KUJATA_ASSERT(condition)                                                \
	do {                                                                        \
		if (!(condition)) {                                                     \
			KujataEngine::PrintAssertFailure(#condition, __FILE__, __LINE__);   \
			if (KujataEngine::IsDebuggerAttached()) {                           \
				__debugbreak();                                                 \
			}                                                                   \
			KujataEngine::ExitAfterAssertFailure();                             \
		}                                                                       \
	} while (false)

#endif
