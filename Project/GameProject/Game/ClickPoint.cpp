#include"ClickPoint.h"
#include "../Base/Base.h"
#include"ShareNum.h"

ClickPoint::ClickPoint()
	:Base(eType_ClickPoint)
{

}

void ClickPoint::Update()
{
	if (PUSH(CInput::eMouseL)) {
		ShareNum::score += 1500;
		Base::Add(new PopupPoint(CVector2D(200, 500), 1500));
	}

}

void ClickPoint::Draw()
{
	//m_img.Draw();





}


PopupPoint::PopupPoint(const CVector2D& pos, int p)
	:Base(eType_ClickPoint)
{
	m_point = p;
	m_pos = pos;
	m_cnt = 0;
}

void PopupPoint::Update()
{
	m_pos.y -= 2;
	if (m_cnt++ > 120)SetKill();
}

void PopupPoint::Draw()
{
	FONT_T()->Draw(m_pos.x, m_pos.y,0, 0, 0, "%d", m_point);
}
