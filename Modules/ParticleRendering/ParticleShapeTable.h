#pragma once

#include "GraphicsKit/ShaderResourceHeap.h"
#include "Interface/ParticleData.h"

#include <d3d12.h>
#include <vector>
#include <wrl/client.h>

namespace KujataEngine {

/// <summary>
/// GPUに置いた形1つ分の、集まる先のバッファとシェーダーが読む名札。
/// </summary>
struct GpuParticleShape {
	Microsoft::WRL::ComPtr<ID3D12Resource> targetBuffer;
	UINT targetCount = 0;
	D3D12_GPU_DESCRIPTOR_HANDLE targetsGpuHandle = {};
};

/// <summary>
/// 登録された形をGPUのバッファにして番号で引けるようにする表で、登録した次の描画の命令書でGPUへ転送する。
/// </summary>
class ParticleShapeTable {
public:
	/// <summary>
	/// 代わりの1粒を0番に登録する。
	/// </summary>
	void Initialize(ID3D12Device* device, ShaderResourceHeap* shaderResourceHeap);

	/// <summary>
	/// 形のバッファと名札を作って転送を予約し、番号を返す。空の形か棚が満杯なら、代わりの1粒を返す。
	/// </summary>
	ParticleShapeHandle Add(const ParticleShape& shape);

	/// <summary>
	/// 予約された形の転送を命令書に書き、シェーダーが読める状態にする。前に転送した一時バッファはここで捨てる。
	/// </summary>
	void RecordPendingUploads(ID3D12GraphicsCommandList* list);

	/// <summary>
	/// 番号の形を返し、知らない番号なら代わりの1粒を返す。
	/// </summary>
	const GpuParticleShape& Get(ParticleShapeHandle handle) const;

private:
	/// <summary>
	/// 転送を待っている形1つ分で、CPUから書いた一時バッファと、転送先の番号を持つ。
	/// </summary>
	struct PendingUpload {
		Microsoft::WRL::ComPtr<ID3D12Resource> uploadBuffer;
		uint32_t shapeId = 0;
	};

	ID3D12Device* device_ = nullptr;
	ShaderResourceHeap* shaderResourceHeap_ = nullptr;

	std::vector<GpuParticleShape> shapes_;
	std::vector<PendingUpload> pendingUploads_;

	// 命令書に書いた転送の一時バッファで、GPUが使い終わる次のフレームまで残す。
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> recordedUploadBuffers_;

	bool hasWarnedHeapFull_ = false;
	// 引くだけの関数からも一度だけ警告できるように、mutableにする。
	mutable bool hasWarnedUnknownShape_ = false;
};

} // namespace KujataEngine
