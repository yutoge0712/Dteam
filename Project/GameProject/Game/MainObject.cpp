#include "MainObject.h"
#include "ShareNum.h"
MainObject::MainObject(const CVector2D pos)
	:Base(eType_MObject)
{
	ImageSet();
}

void MainObject::Update()
{
	
		//状態が成鳥(3)未満　なおかつ　スコアが最大値以上になれば
		if (ShareNum::tori_state < 3 && ShareNum::score >= ShareNum::MaxScore) {
			//スコアリセット
			ShareNum::score -= ShareNum::MaxScore;
			//次の状態へ
			ShareNum::tori_state++;
		}

}

void MainObject::Draw()
{
	//m_img.Draw();
	switch (ShareNum::tori_state) {
		case 0:
			//卵
			tama_img.Draw();
			break;
		case 1:
			//孵化
			huka_img.Draw();
			break;
		case 2:
			//ひよこ
			maru_img.Draw();
			break;
		case 3:
			koza_img.Draw();
			break;
	}

	//maru_img.Draw();
	//huka_img.Draw();
	//koza_img.Draw();
	//tama_img.Draw();
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
	
	tama_img.SetSize(300,300);
	maru_img.SetSize(300,286);
	huka_img.SetSize(245,300);
	koza_img.SetSize(300,300);

	tama_img.SetPos(140,375);
	maru_img.SetPos(140,375);
	huka_img.SetPos(140,375);
	koza_img.SetPos(140,375);
}
