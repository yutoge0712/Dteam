#pragma once

#include "../Base/Base.h"
class MainObject : public Base {
public:
	//�ۂ���
	CImage maru_img;
	//�z�������Ă̒�
	CImage huka_img;
	//�R�U�N���C���R
	CImage koza_img;
	MainObject(const CVector2D pos);
	void Update();
	void Draw();
	void Collision();
	void ImageSet();
};