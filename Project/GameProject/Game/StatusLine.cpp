#include "StatusLine.h"
#include "../Base/Base.h"
#include "ShareNum.h"
#include <iostream>

StatusLine::StatusLine() :Base(eState_StatusLine) {
	ImageSet();
}
void StatusLine::ImageSet() {
	bar_img = COPY_RESOURCE("Status", CImage);
	line_img = COPY_RESOURCE("Status", CImage);
	frame_img= COPY_RESOURCE("Status_frame", CImage);
	line_img.SetRect(10, 77, 480, 102);
	frame_img.SetRect(50,50,680,182);
	line_img.SetSize(480 - 10, 102 - 77);
	frame_img.SetSize(680-50,182-50);
	bar_img.SetPos(191, 137);
	line_img.SetPos(189, 136);
	frame_img.SetPos(50, 50);
}
void StatusLine::Draw() {
	line_img.Draw();
	bar_img.Draw();
	frame_img.Draw();
}
void StatusLine::Update() {
	score_rate = ShareNum::score * 10000 / ShareNum::MaxScore ;
	double ScoreTemp = 467 * score_rate/10000;
	std::cout << ScoreTemp << std::endl;
	bar_img.SetRect(12, 28, 12+ ScoreTemp, 53);
	bar_img.SetSize( ScoreTemp, 53 - 28);
}