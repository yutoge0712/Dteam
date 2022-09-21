#include "MainObject.h"
#include "ShareNum.h"
MainObject::MainObject(const CVector2D pos)
	:Base(eType_MObject)
{
	ImageSet();
}

void MainObject::Update()
{
	//Ç±Ç±Ç…âÊëúïœçXÇ∆Ç©ÇÃèëÇ≠
	
}

void MainObject::Draw()
{
	maru_img.Draw();
	huka_img.Draw();
	koza_img.Draw();
	tama_img.Draw();
}

void MainObject::Collision()
{

}

void MainObject::ImageSet()
{
	tama_img = COPY_RESOURCE("tama", CImage);
	maru_img = COPY_RESOURCE("huka", CImage);
	huka_img = COPY_RESOURCE("maru", CImage);
	koza_img = COPY_RESOURCE("koza", CImage);

	//tama_img.SetRect();
	//maru_img.SetRect();
	//huka_img.SetRect();
	//koza_img.SetRect();
	
	//tama_img.SetSize(352,356);
	//maru_img.SetSize(380,345);
	//huka_imgSetSize(230,383);
	//koza_imgSetSize(180,180);

	//tama_img.SetPos(500,500);
	//maru_img.SetPos(500,500);
	//huka_img.SetPos(500,500);
	//koza_img.SetPos(500,500);
}
