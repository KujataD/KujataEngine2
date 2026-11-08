#include "Core/FrameLoop.h"

void FrameLoop::Register(FramePhase phase, std::function<void()> callback) {
	callbacks_[phase].push_back(callback);
}

void FrameLoop::RequestQuit() {
	isQuitRequested_ = true;
}

void FrameLoop::Run() {
	if (!HasCallback()) {
		return;
	}

	while (!isQuitRequested_) {
		RunFrame();
	}
}

bool FrameLoop::HasCallback() {
	for (int phase = 0; phase < kFramePhaseCount; phase++) {
		if (!callbacks_[phase].empty()) {
			return true;
		}
	}
	return false;
}

void FrameLoop::RunFrame() {
	for (int phase = 0; phase < kFramePhaseCount; phase++) {
		for (std::function<void()>& callback : callbacks_[phase]) {
			callback();
		}
	}
}
