#pragma once
#include <SceneGraph/Component.h>

class Health : public KujataEngine::Component {
public:
	explicit Health(int hp);

	void Damage(int damage) {
		hp_ -= damage > 0 ? damage : 0;

		if (hp_ < 0) {
			hp_ = 0;
		}
	}

	bool IsAlive() const { return hp_ > 0; }
	int GetHP() const { return hp_; }

private:
	int hp_;
};
