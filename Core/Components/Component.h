#pragma once


class GameObject;

class Component 
{

public:
	enum Type {
		TRANSFORM,
		MATERIAL,
		MESH
	};
	bool active;
	GameObject* owner = nullptr;
	
	Component() : active(false) {}

	virtual void Enable() {};
	virtual void Disable() {};
	virtual void Update(float dt) {};
	

};