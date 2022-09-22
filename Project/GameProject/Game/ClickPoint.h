#pragma once

#include "../Base/Base.h"
class ClickPoint : public Base {
public:
	CImage m_img;
	ClickPoint();
	void Update();
	void Draw();

};
class PopupPoint :public Base {
public:
	int m_point;
		int m_cnt;
	PopupPoint(const CVector2D& pos, int p);
	void Update();
	void Draw();
};