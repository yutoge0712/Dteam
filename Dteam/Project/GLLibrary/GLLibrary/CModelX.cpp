#include "CModelX.h"
#include "windows.h"
#include "CFPS.h"
#include "CCollision.h"
#include "CCamera.h"

CFrameBone::CFrameBone(CFrameBone &f, CFrameBone *parent, CFrameBone *prev, CModel* r) {
	SetNo(f.GetNo());
	SetMatrix(f.GetMatrix());
	SetType(f.GetType());
	copyToken(f.GetToken());
	copyName(f.GetName());
	pMesh = (f.pMesh) ? new CXMesh(*f.pMesh):nullptr;
	matrixOffset = f.matrixOffset;
	pParent = parent;
	pPrev = prev;
	root = r;
	if (f.pChild) {
		pChild = new CFrameBone(*static_cast<CFrameBone*>(f.pChild), this, nullptr, r);
	}
	if (f.pNext) {
		pNext = new CFrameBone(*static_cast<CFrameBone*>(f.pNext), parent, this, r);
	}

}
void CFrameBone::calcBoneMatrix(CMatrix *out) {
	boneMatrix = sMatrix * matrixOffset;
	uint16_t no = GetNo();
	out[no] = boneMatrix;
}
void CFrameBone::calcMatrix() {

	if (GetParent()) {
		lMatrix = ((CFrameBone*)GetParent())->GetLMatrix() * GetMatrix();
		if (m_bind) {
			//CMatrix pm = ((CFrameBone*)GetParent())->GetSMatrix();

			////親の行列から回転成分を排除した行列
			//CMatrix mm;
			//mm.Scale(pm.GetRight().Length(), pm.GetUp().Length(), pm.GetFront().Length());

			////親の行列から回転行列のみ抽出した行列
			//CMatrix cc = pm * mm.GetInverse();
			//cc.Transelate(0, 0, 0);

			//mm.Transelate(pm.GetPosition());
			////自身のモーション行列の回転成分を排除
			//CMatrix m = GetMatrix();
			////位置は親の行列の影響を受ける
			//m = cc * m;
			//m.m00 = 1; m.m01 = 0; m.m02 = 0;
			//m.m10 = 0; m.m11 = 1; m.m12 = 0;
			//m.m20 = 0; m.m21 = 0; m.m22 = 1;

			////オフセット行列の回転成分を打ち消す逆行列を作成
			//CMatrix ofm = matrixOffset;
			//					ofm.m01 = ofm.m10;		ofm.m02 = ofm.m02;			ofm.m03 = 0;
			//ofm.m10 = ofm.m01;							ofm.m12 = ofm.m21;			ofm.m13 = 0;
			//ofm.m20 = ofm.m02;	ofm.m21 = ofm.m12;									ofm.m23 = 0;

			//ofm = ofm * CMatrix::MScale(ofm.GetRight().Length(), ofm.GetUp().Length(), ofm.GetFront().Length()).GetInverse();

			//回転はbindMatrixのみ適応する計算
			//sMatrix = mm * m*bindMatrix * ofm;

			/*
			//親×自身の行列の平行移動を取り出し、原点へ移動する行列を作成
			CMatrix l = ((CFrameBone*)GetParent())->GetLMatrix()  * GetMatrix();
			l.m00 = 1; l.m01 = 0; l.m02 = 0;
			l.m10 = 0; l.m11 = 1; l.m12 = 0;
			l.m20 = 0; l.m21 = 0; l.m22 = 1;


			//親の行列から回転成分を排除した行列
			CMatrix mm,pm= root->m_matrix;
			mm.Scale(pm.GetRight().Length(), pm.GetUp().Length(), pm.GetFront().Length());
			mm.Transelate(pm.GetPosition());



			sMatrix = mm * l * bindMatrix * l.GetInverse()*((CFrameBone*)GetParent())->GetLMatrix()  * GetMatrix();
			*/



			CMatrix pm = root->m_matrix;

			////親の行列から回転成分を排除した行列
			CMatrix mm;
			mm.Scale(pm.GetLeft().Length(), pm.GetUp().Length(), pm.GetFront().Length());

			////親の行列から回転行列のみ抽出した行列
			CMatrix cc = pm * mm.GetInverse();
			cc.Transelate(0, 0, 0);

			CMatrix ll = ((CFrameBone*)GetParent())->GetSMatrix()*  GetMatrix();
			CMatrix l = ll;
			l.m00 = 1; l.m01 = 0; l.m02 = 0;
			l.m10 = 0; l.m11 = 1; l.m12 = 0;
			l.m20 = 0; l.m21 = 0; l.m22 = 1;




			sMatrix = l * bindMatrix *cc.GetInverse()* l.GetInverse() * ll;
		}
		else {
			sMatrix = ((CFrameBone*)GetParent())->GetSMatrix()*  GetMatrix();

		}


		/*if (m_bind) {
		CMatrix pm = lMatrix;
		CMatrix pmr = CMatrix::MTranselate(-pm.GetPosition())*pm;
		CMatrix pmm = CMatrix::MTranselate(pm.GetPosition());
		CMatrix m = bindMatrix * pmr;
		m.m03 += pmm.m03;
		m.m13 += pmm.m13;
		m.m23 += pmm.m23;
		sMatrix = root->m_matrix * root->m_rotMtx.GetInverse() * m;

		}
		else {
		sMatrix = revMatrix[no] * ((CFrameBone*)GetParent())->GetSMatrix() * GetMatrix();

		}*/
	}
	else {
		lMatrix = GetMatrix();
		sMatrix = root->m_matrix * lMatrix;
	}


	if (GetChild()) ((CFrameBone*)GetChild())->calcMatrix();
	if (GetNext()) ((CFrameBone*)GetNext())->calcMatrix();

}
CSkinWeights::CSkinWeights(CXModelLoader &loader, CXLMesh &mesh) {
	weight = new SSkinWeight[mesh.nVertices];
	memset(weight, 0, sizeof(SSkinWeight)*mesh.nVertices);
	std::vector<CXSkinWeights*> *skin = &mesh.skinweights;
	std::vector<CXSkinWeights*>::iterator it = skin->begin();
	while (it != skin->end()) {
		CXSkinWeights* s = *it;
		int bonenum = loader.GetFrameNum(s->transformNodeName);
		for (int i = 0; i<s->nWeights; i++) {
	
			float w = s->weights[i];
			int j = 0;
			for (j = 0; j<4; j++) {
				if (weight[s->vertexIndices[i]].weight.f[j] < w) {
					for (int k = 3; k > j; k--) {
						weight[s->vertexIndices[i]].bone.f[k] = weight[s->vertexIndices[i]].bone.f[k-1];
						weight[s->vertexIndices[i]].weight.f[k] = weight[s->vertexIndices[i]].weight.f[k-1];
					}
					weight[s->vertexIndices[i]].bone.f[j] = (float)bonenum;
					weight[s->vertexIndices[i]].weight.f[j] = w;
					break;
				}
			}

		}
		it++;

	}

}
CSkinWeights::~CSkinWeights() {
	SAFE_DELETE_ARRAY(weight);
}
CXPoly::CXPoly(CXModelLoader &loader, CXLMesh &mesh, CSkinWeights *skinWeights, int matNo) {
	m_pVertex = NULL;


	material = loader.GetMaterialNum(mesh.meshMaterialList->material[matNo]);
	faceCnt = 0;
	for (int i = 0; i<mesh.meshMaterialList->nFaceIndexes; i++) {
		if (mesh.meshMaterialList->faceIndexes[i] == matNo) faceCnt++;
	}
	vertexCnt = faceCnt * 3;

	bNormal = (mesh.meshNormal) ? true : false;


	bTexCode = (mesh.meshTextureCoords) ? true : false;

	bWeight = (skinWeights) ? true : false;

	MY_VERTEX* v = m_pVertex = new MY_VERTEX[vertexCnt];
	for (int i = 0, j = 0; i<mesh.meshMaterialList->nFaceIndexes; i++) {
		if (mesh.meshMaterialList->faceIndexes[i] == matNo) {

			for (int k = 0; k<3; k++, j++,v++) {
				v->vPos.x = mesh.vertices[mesh.faces[i].idx[k]].x;
				v->vPos.y = mesh.vertices[mesh.faces[i].idx[k]].y;
				v->vPos.z = mesh.vertices[mesh.faces[i].idx[k]].z;
				if (bNormal) {
					v->vNorm.x = mesh.meshNormal->normals[mesh.meshNormal->faceNormals[i].idx[k]].x;
					v->vNorm.y = mesh.meshNormal->normals[mesh.meshNormal->faceNormals[i].idx[k]].y;
					v->vNorm.z = mesh.meshNormal->normals[mesh.meshNormal->faceNormals[i].idx[k]].z;
				}
				if (bTexCode) {
					v->vTex.x = mesh.meshTextureCoords->textureCoords[mesh.faces[i].idx[k]].u;
					v->vTex.y = mesh.meshTextureCoords->textureCoords[mesh.faces[i].idx[k]].v;
				}
				if (bWeight) {
					memcpy(v->bBoneIndex, &skinWeights->weight[mesh.faces[i].idx[k]].bone, sizeof(float) * 4);
					memcpy(v->bBoneWeight, &skinWeights->weight[mesh.faces[i].idx[k]].weight, sizeof(float) * 4);
					/*
					if (v->bBoneWeight[0] + v->bBoneWeight[1] + v->bBoneWeight[2] + v->bBoneWeight[3] > 1.0) {
						printf("over\n");

					}*/


				}
			}
		}
	}



}
CXPoly::~CXPoly() {
	glDeleteBuffers(1, &buffer);
	//	glDeleteVertexArrays(1, &vao);
	SAFE_DELETE_ARRAY(m_pVertex);

}
void CXPoly::draw(CShader *shader) {

	if (!buffer && m_pVertex) {
		//	printWeight();
		glGenBuffers(1, &buffer);

		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(MY_VERTEX) * vertexCnt, m_pVertex, GL_STATIC_DRAW);

		//	glGenVertexArrays(1, &vao);
		//	glBindVertexArray(vao);


		delete[] m_pVertex;
		m_pVertex = NULL;
		//	glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	glBindBuffer(GL_ARRAY_BUFFER, buffer);

	//	glBindVertexArray(vao);

	BYTE* offset = 0;
	glEnableVertexAttribArray(CShader::eVertexLocation);
	glVertexAttribPointer(CShader::eVertexLocation, 3, GL_FLOAT, GL_FALSE, sizeof(MY_VERTEX), reinterpret_cast<const void*>(offset));
	offset += sizeof(SVector3D);


	glEnableVertexAttribArray(CShader::eNormalLocation);
	glVertexAttribPointer(CShader::eNormalLocation, 3, GL_FLOAT, GL_FALSE, sizeof(MY_VERTEX), reinterpret_cast<const void*>(offset));
	offset += sizeof(SVector3D);

	glEnableVertexAttribArray(CShader::eTexcoordlLocation);
	glVertexAttribPointer(CShader::eTexcoordlLocation, 2, GL_FLOAT, GL_FALSE, sizeof(MY_VERTEX), reinterpret_cast<const void*>(offset));
	offset += sizeof(SVector2D);


	if (bWeight) {

		glEnableVertexAttribArray(CShader::eWeightsLocation);
		glVertexAttribPointer(CShader::eWeightsLocation, 4, GL_FLOAT, GL_TRUE, sizeof(MY_VERTEX), (void*)offset);
		offset += sizeof(float) * 4;


		glEnableVertexAttribArray(CShader::eIndicesLocation);
		glVertexAttribPointer(CShader::eIndicesLocation, 4, GL_FLOAT, GL_FALSE, sizeof(MY_VERTEX), (void*)offset);
	}


	glDrawArrays(GL_TRIANGLES, 0, vertexCnt);


	glDisableVertexAttribArray(CShader::eVertexLocation);
	glDisableVertexAttribArray(CShader::eNormalLocation);
	glDisableVertexAttribArray(CShader::eTexcoordlLocation);
	glDisableVertexAttribArray(CShader::eWeightsLocation);
	glDisableVertexAttribArray(CShader::eIndicesLocation);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//	glBindVertexArray(0);
}

bool CXPoly::CollisionRay(CVector3D *c, CVector3D *n, const CVector3D &s, const CVector3D &e, float *pLength) {
	MY_VERTEX* v = nullptr;
	if(buffer) {
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		(MY_VERTEX*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
	} else {
		v = m_pVertex;
	}
	bool hit = false;
	for (int i = 0; i < vertexCnt; i += 3, v += 3) {
		if (CCollision::IntersectTriangleRay(c, s, e, v[0].vPos, v[1].vPos, v[2].vPos, pLength)) {

			CVector3D e1 = (CVector3D)v[1].vPos - (CVector3D)v[0].vPos;
			CVector3D e2 = (CVector3D)v[2].vPos - (CVector3D)v[0].vPos;

			*n = CVector3D::Cross(e1, e2).GetNormalize();
			hit = true;
		}
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	return hit;
}
int CXPoly::CollisionSphere(CCollTriangle *out, const CVector3D &center, float radius) {
	MY_VERTEX* v = nullptr;
	if (buffer) {
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		(MY_VERTEX*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
	}
	else {
		v = m_pVertex;
	}

	int cnt = 0;
	for (int i = 0; i < vertexCnt; i += 3, v += 3) {
		float dist;
		if (CCollision::CollisionTriangleSphere(v[0].vPos, v[1].vPos, v[2].vPos, center, radius, NULL, &dist)) {
			out->m_dist = dist;
			out->m_material_id = material;
			CVector3D e1 = (CVector3D)v[1].vPos - (CVector3D)v[0].vPos;
			CVector3D e2 = (CVector3D)v[2].vPos - (CVector3D)v[0].vPos;

			out->m_normal = CVector3D::Cross(e1, e2).GetNormalize();
			out->m_vertex[0] = v[0].vPos;
			out->m_vertex[1] = v[1].vPos;
			out->m_vertex[2] = v[2].vPos;
			out++;
			cnt++;
		}
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	return cnt;
}

int CXPoly::CollisionCupsel(CCollTriangle *out, const CVector3D &top, const CVector3D &bottom, float radius) {
	MY_VERTEX* v = nullptr;
	if (buffer) {
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		(MY_VERTEX*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
	}
	else {
		v = m_pVertex;
	}

	int cnt = 0;
	for (int i = 0; i < vertexCnt; i += 3, v += 3) {
		float dist;
		if (CCollision::CollisionTriangleCapsule(v[0].vPos, v[1].vPos, v[2].vPos, top, bottom, radius, NULL, &dist)) {
			out->m_dist = dist;
			out->m_material_id = material;
			CVector3D e1 = (CVector3D)v[1].vPos - (CVector3D)v[0].vPos;
			CVector3D e2 = (CVector3D)v[2].vPos - (CVector3D)v[0].vPos;

			out->m_normal = CVector3D::Cross(e1, e2).GetNormalize();
			out->m_vertex[0] = v[0].vPos;
			out->m_vertex[1] = v[1].vPos;
			out->m_vertex[2] = v[2].vPos;
			out++;
			cnt++;
		}
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	return cnt;
}
CXAnimation::CXAnimation() :m_anim_layer(0) {
}
CXAnimation::CXAnimation(std::vector<CXAnimationSet*> &a) {
	pAnimation = new std::vector < CXAnimationSet* >;
	for (unsigned int i = 0; i<a.size(); i++) {
		pAnimation->push_back(new CXAnimationSet(*a[i]));
	}
	jam = -1;
	FPS = 30;
	pAnimData = NULL;
	pBoneRoot = NULL;
	m_anim_layer = 0;
	speed = 1.0f;
}
CXAnimation::~CXAnimation() {
}
void CXAnimation::Release() {
	if (pAnimation) {
		for (std::vector<CXAnimationSet*>::iterator it = pAnimation->begin(); it != pAnimation->end(); it++) {
			SAFE_DELETE(*it);
		}
		pAnimation->clear();
		delete pAnimation;
		pAnimation = NULL;
	}
}
CMatrix CXAnimation::calcMatrix(CXAnimationKey &key, float t) {
	CMatrix m[2], result;
	int i;
	for (i = 0; i<key.nKeys; i++) {
		if (key.keys[i].time >= (int)(t)) break;
	}
	//	if(i==0) return key.keys[0].matrix;

	m[0] = key.keys[i].matrix;
	m[1] = key.keys[i + 1].matrix;
	float r = (float)(key.keys[i + 1].time - (t + 1)) / (key.keys[i + 1].time - key.keys[i].time);

	if (r>1.0) {
		//		printf("over");
	}
	//result = m[0] * r + m[1] * (1.0f - r);
	result = m[0];
	return result;

}
void CXAnimation::changeAnimation(int j, bool l, bool check, float t, int blendFrame) {
	if (check) {
		if (jam == j) return;
	}
	if ((unsigned int)j >= pAnimation->size()) return;
	loop = l;
	jam = j;
	time = t;
	endTime = GetEndTime(jam);
	blend = 0.0f;
	blendS = 1.0f / blendFrame;
	saveBlendMatrix();
}
void CXAnimation::saveBlendMatrix() {
	if (jam < 0) return;
	CXAnimationSet* a = (*pAnimation)[jam];
	for (unsigned int i = 0; i<a->animations.size(); i++) {
		CFrameBone *b = (CFrameBone*)CXFrame::GetFrameByName(a->animations[i]->boneName, pBoneRoot);
		if (b) {
			b->saveBlendMatrix();
		}

	}
}
void CXAnimation::updateMatrix() {
	if (jam == -1 || pBoneRoot == NULL) return;
	CXAnimationSet* a;
	float frame;
	if (pAnimData) {
		a = (*pAnimation)[0];
		frame = time + pAnimData[jam].start;
	}
	else {
		a = (*pAnimation)[jam];
		frame = time;
	}
	for (unsigned int i = 0; i<a->animations.size(); i++) {
		CFrameBone *b = (CFrameBone*)CXFrame::GetFrameByName(a->animations[i]->boneName, pBoneRoot);
		if (b && b->m_anim_layer == m_anim_layer) {
			b->SetMatrix(calcMatrix(*a->animations[i]->animationkey, frame));
			if (blend<1.0f) b->calcBlendMatrix(blend);
		}

	}

	//	CXFrame::printMatrix(pBoneRoot);
}
int CXAnimation::GetEndTime(int j) {
	if (pAnimData) {
		return pAnimData[j].end - pAnimData[j].start;
	}
	if (j >= (int)pAnimation->size()) return -1;
	CXAnimationSet* a = (*pAnimation)[j];
	int e = 0;
	for (unsigned int i = 0; i<a->animations.size(); i++) {
		int m = a->animations[i]->animationkey->keys[a->animations[i]->animationkey->nKeys - 1].time;
		if (e < m) e = m;
	}
	return e;
}
void CXAnimation::upDate() {
	time += speed * FPS*CFPS::GetDeltaTime();
	if (time >= endTime - 1.0f) {
		if (loop) {
			time = 0;
		}
		else {
			time = endTime - 1.0f;
		}
	}
	blend += blendS;
	if (blend>1.0f) blend = 1.0f;
}

CXMesh::CXMesh(CXModelLoader &loader, CXLMesh &mesh, CSkinWeights *bone): visibility(true){
	matrix = mesh.GetMatrix();
	for (int i = 0; i<mesh.meshMaterialList->nMaterials; i++) {
		polyList.push_back(new CXPoly(loader, mesh, bone, i));
	}
	if (bone) {
		for (unsigned int i = 0; i<mesh.skinweights.size(); i++) {
			boneList.push_back(new CBoneOffset(*mesh.skinweights[i]));
		}
	}

}
CXMesh::CXMesh(const CXMesh& mesh) {
	polyList = mesh.polyList;
	boneList = mesh.boneList;
	matrix = mesh.matrix;
	visibility = mesh.visibility;
}
CXMesh CXMesh::operator = (const CXMesh &mesh) {
	polyList = mesh.polyList;
	boneList = mesh.boneList;
	matrix = mesh.matrix;
	visibility = mesh.visibility;
	return *this;
}
void CXMesh::Release() {
	for (std::vector<CXPoly*>::iterator it = polyList.begin(); it != polyList.end(); it++) {
		SAFE_DELETE(*it);
	}

	for (std::vector<CBoneOffset*>::iterator it = boneList.begin(); it != boneList.end(); it++) {
		SAFE_DELETE(*it);
	}
}
CXMesh::~CXMesh() {

	polyList.clear();

	boneList.clear();

}
void CXMesh::draw(std::vector<CMaterial*> &materialList, CShader *shader, CFrameBone *pFrameRoot, CMatrix *boneMatrix, int boneNum) {
	if (shader) {
		if (boneList.size()>0) {
			for (unsigned int i = 0; i<boneList.size(); i++) {
				CFrameBone *b = (CFrameBone*)CXFrame::GetFrameByName(boneList[i]->transformNodeName, pFrameRoot);
				b->seMatrixOffset(boneList[i]->matrixOffset);
				b->calcBoneMatrix(boneMatrix);
			}
			int MatrixLocation = glGetUniformLocation(shader->GetProgram(), "Transforms");
			glUniformMatrix4fv(MatrixLocation, boneNum, GL_FALSE, boneMatrix[0].f);
			glUniform1i(glGetUniformLocation(shader->GetProgram(), "useSkin"), 1);
			
		} else {
			glUniform1i(glGetUniformLocation(shader->GetProgram(), "useSkin"), 0);
		}
	}
	if (!visibility) return;
	//pFrameRoot->calcBoneMatrix();
	std::vector<CXPoly*>::iterator it;
	for (it = polyList.begin(); it != polyList.end(); it++) {
		if ((*it)->material != -1) {
			materialList[(*it)->material]->Map();
		}
		(*it)->draw(shader);
		if ((*it)->material != -1) {
			materialList[(*it)->material]->Unmap();
		}

	}

}

bool CXMesh::CollisionRay(CVector3D *c, CVector3D *n, const CVector3D &s, const CVector3D &e, float *pLength) {
	for (unsigned int i = 0; i < polyList.size(); i++) {
		if (polyList[i]->CollisionRay(c, n, s, e, pLength)) return true;
	}
	return false;
}
int CXMesh::CollisionSphere(CCollTriangle *out, const CVector3D &center, float radius) {
	int cnt = 0;
	for (unsigned int i = 0; i < polyList.size(); i++) {
		cnt += polyList[i]->CollisionSphere(out, center, radius);
		out += cnt;
	}
	return cnt;

}

int CXMesh::CollisionCupsel(CCollTriangle *out, const CVector3D &top, const CVector3D &bottom, float radius) {
	int cnt = 0;
	for (unsigned int i = 0; i < polyList.size(); i++) {
		cnt += polyList[i]->CollisionCupsel(out, top, bottom, radius);
		out += cnt;
	}
	return cnt;

}
CModelX::CModelX() : pMeshList(NULL), pMaterialList(NULL), pFrameRoot(NULL), boneMatrix(NULL), animation(NULL) {

}

CModelX::~CModelX() {
	if (animation) {
		delete[] animation;
		animation = NULL;
	}
	if (boneMatrix) {
		delete[] boneMatrix;
		boneMatrix = NULL;
	}

	if (pMeshList) {
		for (auto it = pMeshList->begin(); it != pMeshList->end(); it++) {
			delete *it;
		}
		delete pMeshList;
	}
	if (pMaterialList) {
		for (auto it = pMaterialList->begin(); it != pMaterialList->end(); it++) {
			delete *it;
		}
		delete pMaterialList;
	}
	if (pFrameRoot) pFrameRoot->release();
}
CModelX::CModelX(const CModelX &m) {
	*this = m;
}

void CModelX::operator = (const CModelX &m) {
	CModel::operator=(m);
	pMeshList = new std::vector < CXMesh* >;
	pMaterialList = new std::vector < CMaterial* >;
	for (auto it = m.pMaterialList->begin(); it != m.pMaterialList->end(); it++) {
		pMaterialList->push_back(new CMaterial(**it));
	}
	pFrameRoot = new CFrameBone(*m.pFrameRoot, nullptr, nullptr, this);
	pFrameRoot->root = this;
	createMeshList(pFrameRoot);
	boneNum = m.boneNum;
	if (m.animation) {
		animation = new CXAnimation[2];
		memcpy(animation, m.animation, sizeof(CXAnimation) * 2);
		boneMatrix = new CMatrix[boneNum];
		animation[0].attachFrame(pFrameRoot);
		animation[1].attachFrame(pFrameRoot);
	}
}
void CModelX::Release() {
	if (pMeshList) {
		for (std::vector<CXMesh*>::iterator it = pMeshList->begin(); it != pMeshList->end(); it++) {
			(*it)->Release();
			SAFE_DELETE(*it);
		}
		pMeshList->clear();
		delete pMeshList;
		pMeshList = NULL;
	}
	if (pMaterialList) {
		for (std::vector<CMaterial*>::iterator it = pMaterialList->begin(); it != pMaterialList->end(); it++) {
			(*it)->Release();
			delete (*it);
		}
		pMaterialList->clear();
		delete pMaterialList;
		pMaterialList = NULL;
	}
	if (pFrameRoot) pFrameRoot->release();
	pFrameRoot = NULL;
	SAFE_DELETE(boneMatrix);
	if (animation) {
		animation[0].Release();
		SAFE_DELETE_ARRAY(animation);
	}
	//	SAFE_DELETE(m_shader);
}
void CModelX::UpdateMatrix() {
	CModel::UpdateMatrix();
	UpdateMatrix(m_matrix);
}

void CModelX::UpdateMatrix(CMatrix& m) {
	if (animation) {
		animation[0].updateMatrix();
		animation[1].updateMatrix();
		pFrameRoot->calcMatrix();
	}
}

bool CModelX::isAnimationEnd(int layer)
{
	if (!animation) return true;
	return animation[layer].isEnd();
}

bool CModelX::isAnimationEnd()
{
	if (!animation) return true;
	return animation[0].isEnd();
}
int CModelX::GetAnimationJam(int layer)
{
	if (!animation) return true;
	return animation[layer].GetJam();
}
int CModelX::GetAnimatioLoop(int layer)
{
	if (!animation) return true;
	return animation[layer].GetJam();
}
void CModelX::SetAnimationSpeed(int layer, float s)
{
	animation[layer].SetSpeed(s);
}
void CModelX::SetAnimationSpeed(float s)
{
	animation[0].SetSpeed(s);
	animation[1].SetSpeed(s);
}
void CModelX::Render() {
	CModel::UpdateMatrix();
	Render(m_matrix);
}
void CModelX::Render(CMatrix &m) {

	m_matrix = m;
	if (!m_shader) {
		if (animation) {
			//アニメーション用
			m_shader = CShader::GetInstance("SkinMesh");
		}
		else {
			//アニメーション無し用
			m_shader = CShader::GetInstance("StaticMesh");
		}
	}
	m_shader->Enable();
	
	SendShaderParam(m_shader,CMatrix::indentity, CCamera::GetCurrent()->GetViewMatrix(), CCamera::GetCurrent()->GetProjectionMatrix());
	
	//pFrameRoot->SetMatrix(matrix);

	if (animation && animation[0].jam >= 0) {
		std::vector<CXMesh*>::iterator it;
		for (it = pMeshList->begin(); it != pMeshList->end(); it++) {
			(*it)->draw(*pMaterialList, m_shader, pFrameRoot, boneMatrix, boneNum);
		}
	}
	else {
		drawMesh(pFrameRoot, m, CCamera::GetCurrent()->GetViewMatrix());
	}
	if (m_shader) m_shader->Disable();

}


void CModelX::drawMesh(CFrameBone *f, const CMatrix& m, const CMatrix& view_matrix) {

	CMatrix mat = m * f->matrix;

	if (f->GetType() == eMesh) {

		glUniformMatrix4fv(glGetUniformLocation(m_shader->GetProgram(), "ModelViewMatrix"), 1, GL_FALSE, (view_matrix* mat).f);
		glUniformMatrix4fv(glGetUniformLocation(m_shader->GetProgram(), "WorldMatrix"), 1, false, mat.f);

		f->pMesh->draw(*pMaterialList, m_shader, pFrameRoot, boneMatrix, boneNum);
	}
	if (f->GetChild()) {
		drawMesh((CFrameBone*)f->GetChild(), mat, view_matrix);
	}
	if (f->GetNext()) {
		drawMesh((CFrameBone*)f->GetNext(), m, view_matrix);
	}
}
void CModelX::createMesh(CFrameBone *f, CXModelLoader &loader) {
	if (f->GetType() == eMesh) {
		CXLMesh *m = (CXLMesh*)f->pMesh;
		CSkinWeights *weight = NULL;
		if (m->nFaces != 0) {
			if (0 < loader.animationSet.size()) {
				if (m->skinweights.size() > 0) {
					weight = new CSkinWeights(loader, *m);
				}
			}
			if (0 < loader.animationSet.size()) {
				for (unsigned int j = 0; j < m->skinweights.size(); j++) {
					CFrameBone *fb = (CFrameBone*)CXFrame::GetFrameByName(m->skinweights[j]->transformNodeName, pFrameRoot);
					fb->root = this;
					fb->seMatrixOffset(m->skinweights[j]->matrixOffset);
				}
			}
			f->pMesh = new CXMesh(loader, *m, weight);
			pMeshList->push_back(f->pMesh);
			if (weight) {
				delete weight;
			}
		}
	}
	if (f->GetChild()) {
		createMesh((CFrameBone*)f->GetChild(), loader);
	}
	if (f->GetNext()) {
		createMesh((CFrameBone*)f->GetNext(), loader);
	}
}
void CModelX::createMeshList(CFrameBone *f) {
	if (f->GetType() == eMesh) {
		pMeshList->push_back(f->pMesh);
	}
	if (f->GetChild()) {
		createMeshList((CFrameBone*)f->GetChild());
	}
	if (f->GetNext()) {
		createMeshList((CFrameBone*)f->GetNext());
	}
}
void CModelX::createFrame(CXFrame *xf, CFrameBone *f, int *num) {
	if (xf->GetType() == eMesh) {
		f->pMesh = (CXMesh*)xf;
	}
	(*num)++;
	printf("%s %s No%d\n", f->GetToken(), f->GetName(), f->GetNo());
	if (xf->GetChild()) {
		CFrameBone *c = new CFrameBone();
		*c = *xf->GetChild();
		c->root = this;
		if (!f->GetChild()) f->SetChild(c);
		c->SetParent(f);
		createFrame(xf->GetChild(), c, num);
	}
	if (xf->GetNext()) {
		CFrameBone *c = new CFrameBone();
		*c = *xf->GetNext();
		c->root = this;
		f->SetNext(c);
		c->SetPrev(f);
		c->SetParent(f->GetParent());
		createFrame(xf->GetNext(), c, num);
	}
}
bool CModelX::Load(CXModelLoader &loader) {
	if (!loader.pRoot) return false;
	MakePath(loader.m_filePath);
	boneNum = 0;
	//	printf("f\n");
	//	CXFrame::printFrame(pFrameRoot,0);


	pMeshList = new std::vector < CXMesh* >;
	pFrameRoot = new CFrameBone();
	*pFrameRoot = *loader.pRoot;
	pFrameRoot->root = this;



	createFrame(loader.pRoot, pFrameRoot, &boneNum);
	createMesh(pFrameRoot, loader);
	if (0<loader.animationSet.size()) {

		boneMatrix = new CMatrix[boneNum];

		CXAnimation *a = new CXAnimation(loader.animationSet);
		a->FPS = loader.fps;
		a->attachFrame(pFrameRoot);

		animation = new CXAnimation[2];

		memcpy(&animation[0], a, sizeof(CXAnimation));
		memcpy(&animation[1], a, sizeof(CXAnimation));
		animation[0].m_anim_layer = 0;
		animation[1].m_anim_layer = 1;
		delete a;
		//		CXFrame::printMatrix(pFrameRoot);

	}
	else {
		animation = NULL;
	}
	pMaterialList = new std::vector < CMaterial* >;
	for (unsigned int i = 0; i < loader.materials.size(); i++) {
		CMaterial* mat = new CMaterial();
		CXMaterial* xm = loader.materials[i];
		memcpy(mat->m_ambient.v, &xm->faceColor, sizeof(float) * 4);
		memcpy(mat->m_diffuse.v, &xm->faceColor, sizeof(float) * 4);


		mat->m_shininess = xm->power;
		memcpy(mat->m_emissive.v, &xm->emissiveColor, sizeof(float) * 3);
		memcpy(mat->m_specular.v, &xm->specularColor, sizeof(float) * 3);

		if (strlen(xm->filename)) {
			strcpy_s(mat->m_texture_name, NAME_STR_SIZE, xm->filename);
			mat->m_pTex = new CTexture();
			char str[256];
			strcpy_s(str, 256, m_filePath);
			strcat_s(str, 256, xm->filename);

			if (!mat->m_pTex->Load(str) && !mat->m_pTex->Load(xm->filename)) {
				delete mat->m_pTex;
				mat->m_pTex = NULL;
			}
		}
		else {
			mat->m_pTex = NULL;
		}
		if (animation) {
			//アニメーション用
			mat->mp_shader = CShader::GetInstance("SkinMesh");
		} else {
				//アニメーション無し用
			mat->mp_shader = CShader::GetInstance("StaticMesh");
		}

		pMaterialList->push_back(mat);
	}
	return true;

}
bool CModelX::Load(const char* filePath) {
	CXModelLoader loader(filePath);
	return Load(loader);

}

void CModelX::UpdateAnimation() {
	if (animation) {
		animation[0].upDate();
		animation[1].upDate();
	}
}

CMatrix CModelX::GetFrameMatrix(const char *name, bool local) {
	if (CFrameBone *f = (CFrameBone*)CFrameBone::GetFrameByName(name, pFrameRoot)) {
		return (local) ? m_matrix.GetInverse()*f->GetSMatrix() : f->GetSMatrix();
	}
	return CMatrix();
}
CMatrix CModelX::GetFrameMatrix(int no, bool local) {
	if (CFrameBone *f = (CFrameBone*)CFrameBone::GetFrameByNo(no, pFrameRoot)) {
		return (local) ? m_matrix.GetInverse()*f->GetSMatrix() : f->GetSMatrix();
	}
	return CMatrix();
}

CXMesh *CModelX::GetMesh(const char *name) {
	CFrameBone *f = (CFrameBone*)CFrameBone::GetFrameByName(name, pFrameRoot);
	return f ? f->pMesh : nullptr;

}
CXMesh *CModelX::GetMesh(int no) {
	CFrameBone *f = (CFrameBone*)CFrameBone::GetFrameByNo(no, pFrameRoot);
	return f ? f->pMesh : nullptr;
}
CFrameBone * CModelX::GetFrameBone(int no)const
{
	return 	(CFrameBone*)CFrameBone::GetFrameByNo(no, pFrameRoot);
}
CFrameBone * CModelX::GetFrameBone(const char* name)const
{
	return 	(CFrameBone*)CFrameBone::GetFrameByName(name, pFrameRoot);
}

void CModelX::ChangeAnimation(int jam, bool loop, bool check, float time, int blendFrame)
{
	if (!animation) return;
	animation[0].changeAnimation(jam, loop, check, time, blendFrame);
}

void CModelX::ChangeAnimation(int layer, int jam, bool loop, bool check, float time, int blendFrame)
{
	if (!animation) return;
	animation[layer].changeAnimation(jam, loop, check, time, blendFrame);
}

void CModelX::SetAnimation(int layer_dst, int layer_src)
{
	if (!animation) return;
	CXAnimation* d = &animation[layer_dst];
	CXAnimation* s = &animation[layer_src];
	d->changeAnimation(s->jam, s->loop, true, s->time);
}
void CModelX::attachAnimData(int layer, SAnimData* p)
{
	animation[layer].attachAnimData(p);
}
void CModelX::attachAnimData(SAnimData* p) {
	animation[0].attachAnimData(p);
	animation[1].attachAnimData(p);
}
void CModelX::BindFrameMatrix(int no, const CMatrix& mat) {

	CFrameBone *f = (CFrameBone*)CFrameBone::GetFrameByNo(no, pFrameRoot);
	if (!f) return;
	f->m_bind = true;
	f->bindMatrix = mat;
}
void CModelX::BindFrameMatrix(const char* name, const CMatrix& mat) {

	CFrameBone *f = (CFrameBone*)CFrameBone::GetFrameByName(name, pFrameRoot);
	if (!f) return;
	f->m_bind = true;
	f->bindMatrix = mat;
}

void CModelX::UnbindFrameMatrix(int no) {
	CFrameBone *f = (CFrameBone*)CFrameBone::GetFrameByNo(no, pFrameRoot);
	if (!f) return;
	f->m_bind = false;
}
void CModelX::UnbindFrameMatrix(const char* name) {
	CFrameBone *f = (CFrameBone*)CFrameBone::GetFrameByName(name, pFrameRoot);
	if (!f) return;
	f->m_bind = false;
}


CMaterial *CModelX::GetMaterial(int no) {
	return (*pMaterialList)[no];
	
}


void CModelX::SetMaterial(int no,CMaterial *m) {
	(*pMaterialList)[no]=m;

}
bool CModelX::CollisionRay(CVector3D *pCross, CVector3D *pNormal, const CVector3D &s, const CVector3D &e)  const {
	CVector3D cross, normal;
	float lengh = 1e10;
	bool hit = false;
	CVector3D c, n;

	CMatrix inv = m_matrix.GetInverse();
	CVector3D s2 = inv * CVector4D(s, 1);
	CVector3D e2 = inv * CVector4D(e, 1);
	for (unsigned int i = 0; i < pMeshList->size(); i++) {
		if ((*pMeshList)[i]->CollisionRay(&c, &n, s2, e2, &lengh)) {
			hit = true;
		}
	}
	if (hit) {
		if (pCross) {
			*pCross = m_matrix * CVector4D(cross, 1);
		}
		if (pNormal) {
			*pNormal = CVector3D(m_matrix*CVector4D(normal, 0)).GetNormalize();
		}
	}
	return hit;

}
/*
int CModelX::CollisionSphere(CCollTriangle *out, const CVector3D &center, float radius, int maxcnt)  const {
	float scale = CVector3D(m_matrix.m00, m_matrix.m10, m_matrix.m20).Length();
	int cnt = 0;
	CCollTriangle *o = out;
	CVector3D p = m_matrix.GetInverse()* CVector4D(center, 1);
	float r = radius / scale;

	for (unsigned int i = 0; i < pMeshList->size(); i++) {
		int c = (*pMeshList)[i]->CollisionSphere(o, p, r);
		o += c;
		cnt += c;

	}
	o = out;
	for (int i = 0; i < cnt; i++, o++) {
		o->m_dist *= scale;
		o->m_normal = (m_matrix*o->m_normal).GetNormalize();

	}
	return cnt;

}

int CModelX::CollisionCupsel(CCollTriangle *out, const CVector3D &top, const CVector3D &bottom, float radius, int maxcnt) const {
	float scale = CVector3D(m_matrix.m00, m_matrix.m10, m_matrix.m20).Length();
	int cnt = 0;
	CCollTriangle *o = out;
	CMatrix inv = m_matrix.GetInverse();
	CVector4D t = inv * CVector4D(top, 1);
	CVector4D b = inv * CVector4D(bottom, 1);
	float r = radius / scale;

	for (unsigned int i = 0; i < pMeshList->size(); i++) {
		int c = (*pMeshList)[i]->CollisionCupsel(o, t, b, r);
		o += c;
		cnt += c;

	}
	o = out;
	for (int i = 0; i < cnt; i++, o++) {
		o->m_dist *= scale;
		o->Transform(m_matrix);

	}
	return cnt;


}
*/
void CModelX::outputFrameInfo(FILE* fp,CXFrame *f,int depth) {
	char space[256]="";
	for (int i = 0; i < depth; ++i) space[i] = ' ';
	space[depth] = '\0';
	fprintf(fp,"%s\"%s\":[%d]\r\n", space,f->GetName(), f->GetNo());
	if (f->GetChild()) {
		outputFrameInfo(fp,f->GetChild(),depth+1);
	}
	if (f->GetNext()) {
		outputFrameInfo(fp,f->GetNext(),depth);
	}
}
void CModelX::OutputFrameInfo(const char * file_name)
{
	FILE *fp;
	fopen_s(&fp, file_name, "w");
	if (!fp) return;

	fprintf(fp, "Frame\r\n");
	outputFrameInfo(fp, pFrameRoot, 0);

	fprintf(fp, "Animation\r\n");
	int size = GetAnimationSize();
	for (int i = 0; i < size; i++) {
		fprintf(fp, "\"%s\":[%d]\r\n",GetAnimationName(i),i);
	}
	fclose(fp);

}
