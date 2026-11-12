#include "PointCloudShapes.h"

#include <cmath>
#include <numbers>
#include <random>

namespace KujataEngine {

// 同じ引数なら毎回同じ点群になるように、乱数の種を決めておく。
static const uint32_t kRandomSeed = 20240229;

static const float kTwoPi = std::numbers::pi_v<float> * 2.0f;

PointCloudData CreateSpherePointCloud(uint32_t pointCount, float radius, const Vector3& color) {
	std::mt19937 randomEngine(kRandomSeed);
	std::uniform_real_distribution<float> unitDistribution(0.0f, 1.0f);

	PointCloudData pointCloud;
	pointCloud.targets.reserve(pointCount);
	for (uint32_t i = 0; i < pointCount; i++) {
		// 緯度と経度の角度をそのまま一様に取ると極に固まるので、軸の上の高さを一様に取る。
		const float height = unitDistribution(randomEngine) * 2.0f - 1.0f;
		const float angle = unitDistribution(randomEngine) * kTwoPi;
		const float ringRadius = std::sqrt(1.0f - height * height);

		PointCloudTarget target = {};
		target.normal = { ringRadius * std::cos(angle), height, ringRadius * std::sin(angle) };
		target.position = { target.normal.x * radius, target.normal.y * radius, target.normal.z * radius };
		target.color = color;
		pointCloud.targets.push_back(target);
	}
	return pointCloud;
}

PointCloudData CreateTorusPointCloud(uint32_t pointCount, float ringRadius, float tubeRadius, const Vector3& color) {
	std::mt19937 randomEngine(kRandomSeed);
	std::uniform_real_distribution<float> unitDistribution(0.0f, 1.0f);

	PointCloudData pointCloud;
	pointCloud.targets.reserve(pointCount);
	while (pointCloud.targets.size() < pointCount) {
		const float ringAngle = unitDistribution(randomEngine) * kTwoPi;
		const float tubeAngle = unitDistribution(randomEngine) * kTwoPi;

		// 円環の外側は内側より広いので、軸からの距離に比例した確率で残して面積あたりの数をそろえる。
		const float distanceFromAxis = ringRadius + tubeRadius * std::cos(tubeAngle);
		const float keepProbability = distanceFromAxis / (ringRadius + tubeRadius);
		if (unitDistribution(randomEngine) > keepProbability) {
			continue;
		}

		PointCloudTarget target = {};
		target.position = { distanceFromAxis * std::cos(ringAngle), tubeRadius * std::sin(tubeAngle), distanceFromAxis * std::sin(ringAngle) };
		target.normal = { std::cos(tubeAngle) * std::cos(ringAngle), std::sin(tubeAngle), std::cos(tubeAngle) * std::sin(ringAngle) };
		target.color = color;
		pointCloud.targets.push_back(target);
	}
	return pointCloud;
}

} // namespace KujataEngine
