#include "StatusLine.h"
#include "../Base/Base.h"
#include "ShareNum.h"
#include <iostream>

StatusLine::StatusLine() :Base(eType_StatusLine) {
	ImageSet();
}
void StatusLine::ImageSet() {
	bar_img = COPY_RESOURCE("Status", CImage);
	line_img = COPY_RESOURCE("Status", CImage);
	frame_img= COPY_RESOURCE("Status_frame", CImage);
	Egg_icon_img = COPY_RESOURCE("Egg_icon", CImage);

	line_img.SetRect(10, 77, 480, 102);
	frame_img.SetRect(50,50,680,182);
	Egg_icon_img.SetRect(28,152,349,572);

	line_img.SetSize(352, 18);
	frame_img.SetSize(472,99);
	Egg_icon_img.SetSize(62,78);

	bar_img.SetPos(143, 102);
	line_img.SetPos(141, 102);
	frame_img.SetPos(37, 37);
	Egg_icon_img.SetPos(55,45);
}
void StatusLine::Draw() {
	line_img.Draw();
	bar_img.Draw();
	frame_img.Draw();
	Egg_icon_img.Draw();
}
void StatusLine::Update() {
	score_rate = ShareNum::score * 10000 / ShareNum::MaxScore ;
	double ScoreTemp = 467 * score_rate/10000;
	bar_img.SetRect(12, 28, 12+ ScoreTemp, 53);
	bar_img.SetSize( ScoreTemp/4*3, 18);
}