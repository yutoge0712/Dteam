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

	}
}

void ClickPoint::Draw()
{
	m_img.Draw();





}