#include "Component.h"

class TransformComponent : public Component {

private:
	

public:

	struct Position {
		float x = 0;
		float y = 0;
		float z = 0;
	};

	struct Rotation {
		float x = 0;
		float y = 0;
		float z = 0;
	};

	struct Scale {
		float x = 0;
		float y = 0;
		float z = 0;
	};

	TransformComponent();

	~TransformComponent();

	void Enable() { active = true; };
	void Disable() { active = false; };
	void Update(float dt) override;

	Position position;
	Rotation rotation;
	Scale scale;
};