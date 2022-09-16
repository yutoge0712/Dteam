#include "MainObject.h"
#include "ShareNum.h"
MainObject::MainObject(const CVector2D pos)
	:Base(eType_MObject)
{
	
}

void MainObject::Update()
{
	//ここに画像変更とかの書く
}

void MainObject::Draw()
{
	maru_img.Draw();
	huka_img.Draw();
	koza_img.Draw();
}

void MainObject::Collision()
{

}

void MainObject::ImageSet()
{
	maru_img = COPY_RESOURCE("huka", CImage);
	huka_img = COPY_RESOURCE("maru", CImage);
	koza_img = COPY_RESOURCE("koza", CImage);

}
