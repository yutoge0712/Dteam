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
	// 
	//tama_img.SetSize();
	//maru_img.SetSize();
	//huka_imgSetSize();
	//koza_imgSetSize();

	tama_img.SetPos();
	maru_img.
}
