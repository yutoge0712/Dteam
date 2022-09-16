#pragma once

#include "../Base/Base.h"
class MainObject : public Base {
public:
	CImage m_img;
	MainObject(const CVector2D pos);
	void Update();
	void Draw();
	void Collision();
};