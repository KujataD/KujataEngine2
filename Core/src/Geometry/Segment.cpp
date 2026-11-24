#include "Core/Geometry/Segment.h"

#include <algorithm>
#include <cmath>

namespace KujataEngine {

Vector3 ClosestPoint(const Vector3& point, const Segment& segment) {
	Vector3 a = point - segment.origin;
	float t = Dot(a, segment.diff) / std::powf(Length(segment.diff), 2);
	t = std::clamp(t, 0.0f, 1.0f);
	Vector3 cp = segment.origin + segment.diff * t;
	return cp;
}

} // namespace KujataEngine
