#include "QuitAfterFrames.h"

QuitAfterFrames::QuitAfterFrames(int frameCount) {
	remainingFrames_ = frameCount;
}

void QuitAfterFrames::Startup(ServiceRegistry& services, FrameLoop& frameLoop) {
	frameLoop_ = &frameLoop;
	frameLoop.Register(kUpdate, [this]() {
		Update();
	});
}

void QuitAfterFrames::Update() {
	remainingFrames_--;
	if (remainingFrames_ <= 0) {
		frameLoop_->RequestQuit();
	}
}
