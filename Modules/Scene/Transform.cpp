#include "Transform.h"

namespace KujataEngine {

Matrix4x4 Transform::MakeWorldMatrix() const {
	const Matrix4x4 scaleMatrix = MakeScaleMatrix(scale.x, scale.y, scale.z);
	const Matrix4x4 rotationMatrix = Multiply(Multiply(MakeRotationXMatrix(rotation.x), MakeRotationYMatrix(rotation.y)), MakeRotationZMatrix(rotation.z));
	const Matrix4x4 translateMatrix = MakeTranslateMatrix(position.x, position.y, position.z);
	return Multiply(Multiply(scaleMatrix, rotationMatrix), translateMatrix);
}

} // namespace KujataEngine
