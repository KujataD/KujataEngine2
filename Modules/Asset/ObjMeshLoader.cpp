#include "ObjMeshLoader.h"

#include "AssetPath.h"

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

namespace KujataEngine {

/// <summary>
/// 面の角1つが指す番号で、番号は1から数え、0は指定がないことを表す。
/// </summary>
struct ObjCorner {
	int position = 0;
	int uv = 0;
	int normal = 0;
};

/// <summary>
/// .objのファイルから読み取ったままの内容。
/// </summary>
struct ObjFileData {
	std::vector<Vector3> positions;
	std::vector<Vector2> uvs;
	std::vector<Vector3> normals;
	std::vector<std::vector<ObjCorner>> faces;
	std::string materialFileName;
	std::string firstMaterialName;
	bool hasOtherMaterial = false;
};

/// <summary>
/// 行の残りを前後の空白を除いて返す。
/// </summary>
static std::string ReadRestOfLine(std::istringstream& stream) {
	std::string rest;
	std::getline(stream, rest);

	const size_t firstCharacter = rest.find_first_not_of(" \t");
	if (firstCharacter == std::string::npos) {
		return "";
	}

	const size_t lastCharacter = rest.find_last_not_of(" \t");
	return rest.substr(firstCharacter, lastCharacter - firstCharacter + 1);
}

/// <summary>
/// 負の番号を末尾からの番号に直し、範囲内ならtrueを返す。
/// </summary>
static bool ResolveIndex(int* index, size_t count) {
	if (*index < 0) {
		*index = static_cast<int>(count) + *index + 1;
	}
	return *index >= 1 && *index <= static_cast<int>(count);
}

/// <summary>
/// 面の角の文字列を読み、番号が正しければtrueを返す。
/// </summary>
static bool ParseCorner(const std::string& text, const ObjFileData& objFile, ObjCorner* corner) {
	// 形式はv、v/vt、v//vn、v/vt/vnのどれかで、足りない部分は空になる。
	std::istringstream stream(text);
	std::string parts[3];
	for (std::string& part : parts) {
		std::getline(stream, part, '/');
	}

	corner->position = std::atoi(parts[0].c_str());
	corner->uv = std::atoi(parts[1].c_str());
	corner->normal = std::atoi(parts[2].c_str());

	if (!ResolveIndex(&corner->position, objFile.positions.size())) {
		return false;
	}
	if (corner->uv != 0 && !ResolveIndex(&corner->uv, objFile.uvs.size())) {
		return false;
	}
	if (corner->normal != 0 && !ResolveIndex(&corner->normal, objFile.normals.size())) {
		return false;
	}
	return true;
}

/// <summary>
/// .objのファイルを読み、内容が正しければtrueを返す。
/// </summary>
static bool ReadObjFile(const std::filesystem::path& filePath, ObjFileData* objFile) {
	std::ifstream file(filePath);
	if (!file) {
		return false;
	}

	std::string line;
	while (std::getline(file, line)) {
		std::istringstream stream(line);
		std::string keyword;
		stream >> keyword;

		if (keyword == "v") {
			Vector3 position = {};
			if (!(stream >> position.x >> position.y >> position.z)) {
				return false;
			}
			objFile->positions.push_back(position);
		} else if (keyword == "vt") {
			Vector2 uv;
			if (!(stream >> uv.x >> uv.y)) {
				return false;
			}
			objFile->uvs.push_back(uv);
		} else if (keyword == "vn") {
			Vector3 normal = {};
			if (!(stream >> normal.x >> normal.y >> normal.z)) {
				return false;
			}
			objFile->normals.push_back(normal);
		} else if (keyword == "f") {
			std::vector<ObjCorner> face;
			std::string cornerText;
			while (stream >> cornerText) {
				ObjCorner corner;
				if (!ParseCorner(cornerText, *objFile, &corner)) {
					return false;
				}
				face.push_back(corner);
			}
			objFile->faces.push_back(face);
		} else if (keyword == "mtllib") {
			objFile->materialFileName = ReadRestOfLine(stream);
		} else if (keyword == "usemtl") {
			const std::string materialName = ReadRestOfLine(stream);
			if (objFile->firstMaterialName.empty()) {
				objFile->firstMaterialName = materialName;
			} else if (materialName != objFile->firstMaterialName) {
				objFile->hasOtherMaterial = true;
			}
		}
	}

	return !objFile->faces.empty();
}

/// <summary>
/// .objの右手座標系の角を、このエンジンの左手座標系の頂点に変換する。
/// </summary>
static MeshVertex MakeVertex(const ObjFileData& objFile, const ObjCorner& corner) {
	MeshVertex vertex = {};

	const Vector3& position = objFile.positions[corner.position - 1];
	vertex.position = { -position.x, position.y, position.z };

	if (corner.uv != 0) {
		// .objのVは下が0で、画像のVは上が0になる。
		const Vector2& uv = objFile.uvs[corner.uv - 1];
		vertex.uv = Vector2(uv.x, 1.0f - uv.y);
	}

	if (corner.normal != 0) {
		const Vector3& normal = objFile.normals[corner.normal - 1];
		vertex.normal = { -normal.x, normal.y, normal.z };
	} else {
		vertex.normal = { 0.0f, 1.0f, 0.0f };
	}

	return vertex;
}

/// <summary>
/// 読み取った面を、三角形だけのメッシュに組み立てる。
/// </summary>
static void BuildMesh(const ObjFileData& objFile, MeshData* mesh) {
	// 番号の組が同じ角は、同じ頂点を使い回す。
	std::map<std::tuple<int, int, int>, uint32_t> cornerToVertex;

	for (const std::vector<ObjCorner>& face : objFile.faces) {
		std::vector<uint32_t> faceVertices;

		for (const ObjCorner& corner : face) {
			const std::tuple<int, int, int> key = std::make_tuple(corner.position, corner.uv, corner.normal);
			if (cornerToVertex.count(key) == 0) {
				cornerToVertex[key] = static_cast<uint32_t>(mesh->vertices.size());
				mesh->vertices.push_back(MakeVertex(objFile, corner));
			}
			faceVertices.push_back(cornerToVertex[key]);
		}

		// 多角形を扇形に三角形へ分け、Xの反転で表裏が逆になる分を順番の入れ替えで戻す。
		for (size_t i = 1; i + 1 < faceVertices.size(); i++) {
			mesh->indices.push_back(faceVertices[0]);
			mesh->indices.push_back(faceVertices[i + 1]);
			mesh->indices.push_back(faceVertices[i]);
		}
	}
}

/// <summary>
/// .mtlから指定のマテリアルの画像の名前を探し、なければ空を返す。
/// </summary>
static std::string FindDiffuseTextureName(const std::filesystem::path& materialPath, const std::string& materialName) {
	std::ifstream file(materialPath);
	if (!file) {
		printf("マテリアルのファイルを読めませんでした: %s\n", MakeUtf8FromPath(materialPath).c_str());
		return "";
	}

	std::string currentMaterial;
	std::string line;
	while (std::getline(file, line)) {
		std::istringstream stream(line);
		std::string keyword;
		stream >> keyword;

		if (keyword == "newmtl") {
			currentMaterial = ReadRestOfLine(stream);
		} else if (keyword == "map_Kd" && (materialName.empty() || currentMaterial == materialName)) {
			return ReadRestOfLine(stream);
		}
	}
	return "";
}

bool ObjMeshLoader::Load(const std::filesystem::path& filePath, LoadedModelFile* result) {
	ObjFileData objFile;
	if (!ReadObjFile(filePath, &objFile)) {
		return false;
	}

	BuildMesh(objFile, &result->mesh);

	if (objFile.hasOtherMaterial) {
		printf("マテリアルが複数あるので、最初の1つだけを使います: %s\n", MakeUtf8FromPath(filePath).c_str());
	}

	if (objFile.materialFileName.empty()) {
		return true;
	}

	// .objと.mtlに書かれた名前は、そのファイルからの相対パスになる。
	const std::filesystem::path directory = filePath.parent_path();
	const std::filesystem::path materialPath = directory / MakePathFromUtf8(objFile.materialFileName);
	const std::string textureName = FindDiffuseTextureName(materialPath, objFile.firstMaterialName);
	if (!textureName.empty()) {
		result->texturePath = (directory / MakePathFromUtf8(textureName)).lexically_normal();
	}
	return true;
}

} // namespace KujataEngine
