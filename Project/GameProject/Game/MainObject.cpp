#include "MainObject.h"
MainObject::MainObject(const CVector2D pos)
	:Base(eType_MObject)
{

}

void MainObject::Update()
{

}

void MainObject::Draw()
{
	m_img.Draw();
}

void MainObject::Collision()
{

}
