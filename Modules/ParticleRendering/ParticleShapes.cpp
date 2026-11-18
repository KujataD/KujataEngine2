#include "ParticleShapes.h"

#include "Core/Math/MathUtil.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <numbers>
#include <random>
#include <vector>

namespace KujataEngine {

// 同じ引数なら毎回同じ形になるように、乱数の種を決めておく。
static const uint32_t kRandomSeed = 20240229;

static const float kTwoPi = std::numbers::pi_v<float> * 2.0f;

// sRGBの画像の値を、光の強さに比例する値へ直すときの指数。
static const float kSrgbGamma = 2.2f;

/// <summary>
/// 0から1のUVの位置の色を、いちばん大きい段の画像から光の強さに比例する値で読む。
/// </summary>
static Vector3 ReadImageColor(const ImageData& image, const Vector2& uv) {
	const ImageMip& mip = image.mips[0];

	// 0から1の外のUVは、画像を繰り返し並べたものとして読む。
	const float wrappedU = uv.x - std::floor(uv.x);
	const float wrappedV = uv.y - std::floor(uv.y);
	const uint32_t pixelX = (std::min)(static_cast<uint32_t>(wrappedU * mip.width), mip.width - 1);
	const uint32_t pixelY = (std::min)(static_cast<uint32_t>(wrappedV * mip.height), mip.height - 1);

	// 1画素は赤、緑、青、不透明度の順の4バイト。
	const uint8_t* pixel = &mip.pixels[static_cast<size_t>(pixelY) * mip.rowPitch + static_cast<size_t>(pixelX) * 4];
	Vector3 color;
	color.x = std::pow(pixel[0] / 255.0f, kSrgbGamma);
	color.y = std::pow(pixel[1] / 255.0f, kSrgbGamma);
	color.z = std::pow(pixel[2] / 255.0f, kSrgbGamma);
	return color;
}

ParticleShape CreateSphereParticleShape(uint32_t pointCount, float radius, const Vector3& color) {
	std::mt19937 randomEngine(kRandomSeed);
	std::uniform_real_distribution<float> unitDistribution(0.0f, 1.0f);

	ParticleShape shape;
	shape.targets.reserve(pointCount);
	for (uint32_t i = 0; i < pointCount; i++) {
		// 緯度と経度の角度をそのまま一様に取ると極に固まるので、軸の上の高さを一様に取る。
		const float height = unitDistribution(randomEngine) * 2.0f - 1.0f;
		const float angle = unitDistribution(randomEngine) * kTwoPi;
		const float ringRadius = std::sqrt(1.0f - height * height);

		ParticleTarget target = {};
		target.normal = { ringRadius * std::cos(angle), height, ringRadius * std::sin(angle) };
		target.position = { target.normal.x * radius, target.normal.y * radius, target.normal.z * radius };
		target.color = color;
		shape.targets.push_back(target);
	}
	return shape;
}

ParticleShape CreateTorusParticleShape(uint32_t pointCount, float ringRadius, float tubeRadius, const Vector3& color) {
	std::mt19937 randomEngine(kRandomSeed);
	std::uniform_real_distribution<float> unitDistribution(0.0f, 1.0f);

	ParticleShape shape;
	shape.targets.reserve(pointCount);
	while (shape.targets.size() < pointCount) {
		const float ringAngle = unitDistribution(randomEngine) * kTwoPi;
		const float tubeAngle = unitDistribution(randomEngine) * kTwoPi;

		// 円環の外側は内側より広いので、軸からの距離に比例した確率で残して面積あたりの数をそろえる。
		const float distanceFromAxis = ringRadius + tubeRadius * std::cos(tubeAngle);
		const float keepProbability = distanceFromAxis / (ringRadius + tubeRadius);
		if (unitDistribution(randomEngine) > keepProbability) {
			continue;
		}

		ParticleTarget target = {};
		target.position = { distanceFromAxis * std::cos(ringAngle), tubeRadius * std::sin(tubeAngle), distanceFromAxis * std::sin(ringAngle) };
		target.normal = { std::cos(tubeAngle) * std::cos(ringAngle), std::sin(tubeAngle), std::cos(tubeAngle) * std::sin(ringAngle) };
		target.color = color;
		shape.targets.push_back(target);
	}
	return shape;
}

ParticleShape CreateBoxParticleShape(uint32_t pointCount, const Vector3& size, const Vector3& color) {
	std::mt19937 randomEngine(kRandomSeed);
	std::uniform_real_distribution<float> unitDistribution(0.0f, 1.0f);

	// 向かい合う2面をまとめた面積で、Xに垂直な面、Yに垂直な面、Zに垂直な面の順。
	const float areaX = size.y * size.z * 2.0f;
	const float areaY = size.x * size.z * 2.0f;
	const float areaZ = size.x * size.y * 2.0f;
	const Vector3 halfSize = size * 0.5f;

	ParticleShape shape;
	shape.targets.reserve(pointCount);
	for (uint32_t i = 0; i < pointCount; i++) {
		// 広い面ほど選ばれやすくして、細長い箱でも面積あたりの数をそろえる。
		const float areaPosition = unitDistribution(randomEngine) * (areaX + areaY + areaZ);
		const float side = unitDistribution(randomEngine) < 0.5f ? -1.0f : 1.0f;
		const float first = unitDistribution(randomEngine) * 2.0f - 1.0f;
		const float second = unitDistribution(randomEngine) * 2.0f - 1.0f;

		ParticleTarget target = {};
		if (areaPosition < areaX) {
			target.normal = { side, 0.0f, 0.0f };
			target.position = { side * halfSize.x, first * halfSize.y, second * halfSize.z };
		} else if (areaPosition < areaX + areaY) {
			target.normal = { 0.0f, side, 0.0f };
			target.position = { first * halfSize.x, side * halfSize.y, second * halfSize.z };
		} else {
			target.normal = { 0.0f, 0.0f, side };
			target.position = { first * halfSize.x, second * halfSize.y, side * halfSize.z };
		}
		target.color = color;
		shape.targets.push_back(target);
	}
	return shape;
}

ParticleShape CreateMeshSurfaceParticleShape(const MeshData& mesh, const ImageData& image, uint32_t pointCount) {
	// 頂点だけに置くと平らな面に点が来ないので、三角形の面積に比例した確率で三角形を選ぶ。
	const size_t triangleCount = mesh.indices.size() / 3;
	std::vector<float> cumulativeAreas;
	cumulativeAreas.reserve(triangleCount);
	float totalArea = 0.0f;
	for (size_t i = 0; i < triangleCount; i++) {
		const Vector3& position0 = mesh.vertices[mesh.indices[i * 3]].position;
		const Vector3& position1 = mesh.vertices[mesh.indices[i * 3 + 1]].position;
		const Vector3& position2 = mesh.vertices[mesh.indices[i * 3 + 2]].position;
		totalArea += Length(Cross(position1 - position0, position2 - position0)) * 0.5f;
		cumulativeAreas.push_back(totalArea);
	}

	ParticleShape shape;
	if (totalArea <= 0.0f || image.mips.empty()) {
		printf("面積のないメッシュか空の画像なので、形を作れません。\n");
		return shape;
	}

	std::mt19937 randomEngine(kRandomSeed);
	std::uniform_real_distribution<float> unitDistribution(0.0f, 1.0f);

	shape.targets.reserve(pointCount);
	for (uint32_t i = 0; i < pointCount; i++) {
		// 面積の累積和の中で乱数の位置を二分探索すると、広い三角形ほど選ばれやすくなる。
		const float areaPosition = unitDistribution(randomEngine) * totalArea;
		const size_t foundIndex = std::lower_bound(cumulativeAreas.begin(), cumulativeAreas.end(), areaPosition) - cumulativeAreas.begin();
		const size_t triangleIndex = (std::min)(foundIndex, triangleCount - 1);

		const MeshVertex& vertex0 = mesh.vertices[mesh.indices[triangleIndex * 3]];
		const MeshVertex& vertex1 = mesh.vertices[mesh.indices[triangleIndex * 3 + 1]];
		const MeshVertex& vertex2 = mesh.vertices[mesh.indices[triangleIndex * 3 + 2]];

		// 平方根を挟むと、三角形の中で角に偏らずに一様に取れる。
		const float rootOfRandom = std::sqrt(unitDistribution(randomEngine));
		const float secondRandom = unitDistribution(randomEngine);
		const float weight0 = 1.0f - rootOfRandom;
		const float weight1 = rootOfRandom * (1.0f - secondRandom);
		const float weight2 = rootOfRandom * secondRandom;

		const Vector2 uv = {
			vertex0.uv.x * weight0 + vertex1.uv.x * weight1 + vertex2.uv.x * weight2,
			vertex0.uv.y * weight0 + vertex1.uv.y * weight1 + vertex2.uv.y * weight2,
		};

		ParticleTarget target = {};
		target.position = vertex0.position * weight0 + vertex1.position * weight1 + vertex2.position * weight2;
		target.normal = Normalize(vertex0.normal * weight0 + vertex1.normal * weight1 + vertex2.normal * weight2);
		target.color = ReadImageColor(image, uv);
		shape.targets.push_back(target);
	}
	return shape;
}

} // namespace KujataEngine
