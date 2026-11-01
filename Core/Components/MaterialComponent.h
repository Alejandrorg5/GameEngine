#include "Component.h"

class MaterialComponent : public Component 
{

public:
	
	MaterialComponent();

	~MaterialComponent();

	void Enable() { active = true; };
	void Disable() { active = false; };
	void Update(float dt) override;
};