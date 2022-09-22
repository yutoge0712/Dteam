#pragma once

#include "../Base/Base.h"
class MainObject : public Base {
public:
	CImage m_img;
	//卵
	CImage tama_img;
	//丸い鳥
	CImage maru_img;
	//孵化したての鳥
	CImage huka_img;
	//コザクラインコ
	CImage koza_img;
	MainObject(const CVector2D pos);
	void Update();
	void Draw();
	void Collision();
	void ImageSet();
};