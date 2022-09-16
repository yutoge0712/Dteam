#pragma once
#include "../Base/Base.h"
class StatusLine :public Base {
public:
	CImage bar_img;
	CImage line_img;
	CImage frame_img;
	double score_rate;
public:
	StatusLine();
	void Draw();
	void ImageSet();
	void Update();
};
