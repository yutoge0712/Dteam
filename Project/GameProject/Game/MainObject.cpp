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
	m_img.Draw();
	switch (ShareNum::tori_state) {
		case 0:
			//卵
			tama_img.Draw();
			break;
		case 1:
			//孵化
			huka_img.Draw();
			break;
		case2:
			//ひよこ
			maru_img.Draw();
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
	
	//tama_img.SetSize(352,356);
	//maru_img.SetSize(380,345);
	//huka_imgSetSize(230,383);
	//koza_imgSetSize(180,180);

	tama_img.SetPos(200,500);
	maru_img.SetPos(200,500);
huka_img.SetPos(200,500);
	koza_img.SetPos(200,500);
}
