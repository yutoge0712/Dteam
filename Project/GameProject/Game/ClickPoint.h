#pragma once

#include "../Base/Base.h"
class ClickPoint : public Base {
public:
	CImage m_img;
	ClickPoint();
	void Update();
	void Draw();

};