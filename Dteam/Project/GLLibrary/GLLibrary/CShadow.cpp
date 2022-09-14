#include "CShadow.h"

CShadow *CShadow::m_shadow=nullptr;

CShadow::CShadow(float length, float height, int texWidth, int texHeight): m_state(eNone) {
	
	m_depthtexHeight = texHeight;
	m_depthtexWidth = texWidth;
	m_lightViewLength = length;
	m_lightHeight = height;

	mp_render_target = new CTextureFrame(m_depthtexHeight, m_depthtexWidth, CVector4D(1, 1, 1, 1));


}
CShadow::~CShadow() {
	delete mp_render_target;

}
void CShadow::Render(std::function<void()> render) {

	CCamera* back = CCamera::GetCurrent();
	//シャドウマップ描画準備
	CVector3D cam_pos = CCamera::GetCurrent()->GetPos();
	//シャドウマップ用　光源からのカメラに切り替え
	CCamera::SetCurrent(CCamera::GetCamera(CCamera::eShadowCamera));
	//投影行列
	CCamera::GetCurrent()->Ortho(-m_lightViewLength, m_lightViewLength, -m_lightViewLength, m_lightViewLength, 1.0f, 100.0f);
	//0番目のライトの向き
	CVector3D ld = CLight::GetDirPointer()[0];
	//光源の高さ
	const float dist = m_lightHeight;
	//ビュー行列
	CCamera::GetCurrent()->LookAt(cam_pos - ld * dist, cam_pos, CVector3D(0, 1, 0));
	//テクスチャ行列
	static  const CMatrix texMat(0.5f, 0.0f, 0.0f, 0.5f,
		0.0f, 0.5f, 0.0f, 0.5f,
		0.0f, 0.0f, 0.5f, 0.5f,
		0.0f, 0.0f, 0.0f, 1.0f);
	//シャドウマップ変換行列
	CLight::m_shadowMatrix = texMat * CCamera::GetCurrent()->GetProjectionMatrix() * CCamera::GetCurrent()->GetViewMatrix();
	CLight::SetLighting(false);
	//前面を描画しない
	glCullFace(GL_FRONT);
	m_state = eShadow;
	//シャドウマップ用描画
	mp_render_target->BeginDraw();
	render();
	mp_render_target->EndDraw();
	//背面を描画しない
	glCullFace(GL_BACK);
	CLight::SetLighting(true);
	//元のカメラに戻す
	CCamera::SetCurrent(back);
	//7番のテクスチャーをシャドウマップとする
	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, mp_render_target->GetDepthTexture()->m_bufID);
	glEnable(GL_TEXTURE_2D);
	//0番テクスチャー操作に戻す
	glActiveTexture(GL_TEXTURE0);

	m_state = eNone;
	//通常描画
	render();
}

void CShadow::DrawDepthTex(int x,int y,int w, int h)
{
	mp_render_target->Draw(x, y, w, h, mp_render_target->GetDepthTexture());
}
void CShadow::CreateInscance(float length, float height, int texWidth, int texHeight)
{
	if (m_shadow) return;
	m_shadow = new CShadow(length, height,texWidth, texHeight);
}

CShadow* CShadow::GetInstance()
{
	if (!m_shadow) CreateInscance();
	return m_shadow;
}

void CShadow::ClearInstance()
{
	if (!m_shadow) return;
	delete m_shadow;
	m_shadow = nullptr;
}