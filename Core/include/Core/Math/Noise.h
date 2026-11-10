#pragma once
#include "Core/Math/Vector3.h"

namespace KujataEngine {

/// <summary>
/// 時間や位置を入れると滑らかにつながったおよそ-0.8から0.8の値を返し、yを離して渡すと互いに無関係な揺れになるノイズ。
/// </summary>
float PerlinNoise(float x, float y);

/// <summary>
/// 3次元のパーリンノイズで、整数の座標では必ず0になるので、半端な値を足して渡す。
/// </summary>
float PerlinNoise(const Vector3& position);

/// <summary>
/// 周波数をlacunarity倍、振幅をgain倍にしながらoctaves回重ねたノイズで、雲や地形のように細かさの階層が欲しいものに使う。
/// </summary>
float FractalNoise(float x, float y, int octaves, float lacunarity = 2.0f, float gain = 0.5f);

/// <summary>
/// 3次元版で、octavesは増やすほど細かくなるが重くなるので、3か4で足りることが多い。
/// </summary>
float FractalNoise(const Vector3& position, int octaves, float lacunarity = 2.0f, float gain = 0.5f);

} // namespace KujataEngine
