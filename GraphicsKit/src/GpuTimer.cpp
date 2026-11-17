#include "GraphicsKit/GpuTimer.h"

#include "Core/Assert.h"

namespace KujataEngine {

void GpuTimer::Initialize(IGpu* gpu, UINT markCount) {
	KUJATA_ASSERT(markCount > 0);
	markCount_ = markCount;
	timestampFrequency_ = gpu->GetTimestampFrequency();

	ID3D12Device* device = gpu->GetDevice();

	D3D12_QUERY_HEAP_DESC heapDescription = {};
	heapDescription.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
	heapDescription.Count = markCount;
	HRESULT result = device->CreateQueryHeap(&heapDescription, IID_PPV_ARGS(&queryHeap_));
	KUJATA_ASSERT(SUCCEEDED(result));

	// タイムスタンプは1つ8バイトで、CPUから読めるメモリに写してから読む。
	D3D12_HEAP_PROPERTIES heapProperties = {};
	heapProperties.Type = D3D12_HEAP_TYPE_READBACK;

	D3D12_RESOURCE_DESC description = {};
	description.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	description.Width = sizeof(uint64_t) * markCount;
	description.Height = 1;
	description.DepthOrArraySize = 1;
	description.MipLevels = 1;
	description.SampleDesc.Count = 1;
	description.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	result = device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &description, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&readbackBuffer_));
	KUJATA_ASSERT(SUCCEEDED(result));
}

void GpuTimer::Mark(ID3D12GraphicsCommandList* list, UINT markIndex) {
	KUJATA_ASSERT(markIndex < markCount_);

	// タイムスタンプには始まりがなく、EndQueryだけでその時点の時刻が記録される。
	list->EndQuery(queryHeap_.Get(), D3D12_QUERY_TYPE_TIMESTAMP, markIndex);
}

void GpuTimer::Resolve(ID3D12GraphicsCommandList* list) {
	list->ResolveQueryData(queryHeap_.Get(), D3D12_QUERY_TYPE_TIMESTAMP, 0, markCount_, readbackBuffer_.Get(), 0);

	// Gpuが毎フレーム完了を待つので、次のフレームにはもう読める。
	hasResult_ = true;
}

double GpuTimer::ReadMilliseconds(UINT fromMarkIndex, UINT toMarkIndex) {
	KUJATA_ASSERT(hasResult_);
	KUJATA_ASSERT(fromMarkIndex < markCount_ && toMarkIndex < markCount_);

	const D3D12_RANGE readRange = { 0, sizeof(uint64_t) * markCount_ };
	uint64_t* timestamps = nullptr;
	const HRESULT result = readbackBuffer_->Map(0, &readRange, reinterpret_cast<void**>(&timestamps));
	KUJATA_ASSERT(SUCCEEDED(result));

	const uint64_t elapsedTicks = timestamps[toMarkIndex] - timestamps[fromMarkIndex];

	// 何も書いていないことを、空の範囲で伝える。
	const D3D12_RANGE writtenRange = { 0, 0 };
	readbackBuffer_->Unmap(0, &writtenRange);

	return static_cast<double>(elapsedTicks) / static_cast<double>(timestampFrequency_) * 1000.0;
}

} // namespace KujataEngine
