#include "Component.h"

class MeshComponent : public Component {

public:
	
	MeshComponent();

	~MeshComponent();

	void Enable() { active = true; };
	void Disable() { active = false; };
	void Update(float dt) override;
};