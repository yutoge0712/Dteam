#include "CTextureFrame.h"
#include "CCamera.h"
#include "CShader.h"


CTextureFrame::CTextureFrame(int width,int height, const CVector4D& clear_color)
{
	m_clear_color = clear_color;

	//カラー用テクスチャ生成
	mp_texture = new CTexture(width,height, GL_RGBA);
	//深度バッファ用テクスチャ生成
	mp_depth = new CTexture(width, height, GL_DEPTH_COMPONENT);

	//フレームバッファ作成
	glGenFramebuffersEXT(1, &m_frame);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_frame);

	//テクスチャー接続（カラー）
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, mp_texture->m_bufID, 0);

	//テクスチャー接続（深度）
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, mp_depth->m_bufID, 0);


	//通常描画へ戻す
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

CTextureFrame::~CTextureFrame()
{

	glDeleteFramebuffers(1, &m_frame);
	if (mp_texture) {
		mp_texture->Release();
		delete mp_texture;
	}
	if(mp_depth) {
		mp_depth->Release();
		delete mp_depth;
	}

}

void CTextureFrame::BeginDraw()
{
	//フレームバッファ有効化。以降の描画処理は mp_texture,mp_depthへ書き込まれる。
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_frame);
	//元のビューポートを保存
	glGetIntegerv(GL_VIEWPORT, viewport_old);
	//ビューポートをテクスチャーのサイズに設定
	glViewport(0, 0, mp_texture->m_width, mp_texture->m_height);

	float back_color[4];
	glGetFloatv(GL_COLOR_CLEAR_VALUE,back_color);

	//消去
	glClearColor(m_clear_color.r, m_clear_color.g, m_clear_color.b, m_clear_color.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//クリアカラーを元に戻す
	glClearColor(back_color[0], back_color[1], back_color[2], back_color[3]);
}

void CTextureFrame::EndDraw()
{
	
	//m_blackへの書き込み終了
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	//ここまでの描画命令を一旦処理
	glFlush();
	glViewport(viewport_old[0], viewport_old[1], viewport_old[2], viewport_old[3]);

}

int CTextureFrame::GetWidth()
{
	return mp_texture->m_width;
}

int CTextureFrame::GetHeight()
{
	return mp_texture->m_height;
}

void CTextureFrame::Draw(int x, int y,int w,int h, CTexture* target)
{

	glDisable(GL_CULL_FACE);
	//ライティング無効
	glDisable(GL_LIGHTING);
	//デプステスト無効
	glDisable(GL_DEPTH_TEST);

	//
	//正射投影の行列を作成
	//
	const CMatrix& mProj = CCamera::GetCurrent()->Get2DProjectionMatrix();



	CMatrix pvm = mProj * CMatrix::MTranselate(x, y, 0) * CMatrix::MScale(w, h, 1);

	CShader* s = CShader::GetInstance("Image");



	CVector3D vertices[] = {
		CVector3D(0,0,0),
		CVector3D(1,0,0),
		CVector3D(1,1,0),
		CVector3D(0,1,0)
	};
	CVector2D texcoord[] = {
		CVector2D(0,1),
		CVector2D(1,1),
		CVector2D(1,0),
		CVector2D(0,0)
	};
	CTexture* t = target;


	s->Enable();
	t->MapTexture();

	GLint samplerId = glGetUniformLocation(s->GetProgram(), "sampler");
	glUniform1i(samplerId, 0);//GL_TEXTURE0を適用

	glUniformMatrix4fv(glGetUniformLocation(s->GetProgram(), "PVWMatrix"), 1, GL_FALSE, pvm.f);
	glUniform4fv(glGetUniformLocation(s->GetProgram(), "Color"), 1, CVector4D(1,1,1,1).v);

	glEnableVertexAttribArray(CShader::eVertexLocation);
	glEnableVertexAttribArray(CShader::eTexcoordlLocation);
	glVertexAttribPointer(CShader::eVertexLocation, 3, GL_FLOAT, GL_FALSE, 0, vertices->v);
	glVertexAttribPointer(CShader::eTexcoordlLocation, 2, GL_FLOAT, GL_FALSE, 0, texcoord->v);


	glDrawArrays(GL_QUADS, 0, 4);

	glDisableVertexAttribArray(CShader::eVertexLocation);
	glDisableVertexAttribArray(CShader::eTexcoordlLocation);

	s->Disable();
	t->UnmapTexture();

	//元の設定に戻す

	glEnable(GL_CULL_FACE);
	//ライティング無効
	glEnable(GL_LIGHTING);
	//デプステスト無効
	glEnable(GL_DEPTH_TEST);



}
