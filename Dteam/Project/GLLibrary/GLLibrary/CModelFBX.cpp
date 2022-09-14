#include "CModelFBX.h"
#include "CCollision.h"
#include "Utility.h"
#include "CFPS.h"
#include "CCamera.h"

#define POINT1 pointCnt >= uvCnt

#define NUMREF_MAX 50
class POLY_TABLE
{
public:
	std::vector<int> PolyIndex;
	std::vector<int> Index123;
	//int PolyIndex[NUMREF_MAX];//ポリゴン番号 
	//int Index123[NUMREF_MAX];//3つの頂点のうち、何番目か
	int NumRef;//属しているポリゴン数

	POLY_TABLE()
	{
		NumRef = 0;
	}
};


static void printMatrix(CMatrix &m) {
	printf(
		" %f %f %f %f\n"
		" %f %f %f %f\n"
		" %f %f %f %f\n"
		" %f %f %f %f\n\n",
		m.m00, m.m01, m.m02, m.m03,
		m.m10, m.m11, m.m12, m.m13,
		m.m20, m.m21, m.m22, m.m23,
		m.m30, m.m31, m.m32, m.m33);
}

static void printMatrix(FILE* fp,CMatrix& m) {
	fprintf(fp,
		" %f %f %f %f\n"
		" %f %f %f %f\n"
		" %f %f %f %f\n"
		" %f %f %f %f\n\n",
		m.m00, m.m01, m.m02, m.m03,
		m.m10, m.m11, m.m12, m.m13,
		m.m20, m.m21, m.m22, m.m23,
		m.m30, m.m31, m.m32, m.m33);
}

static void CopyMatrix(CMatrix *d, FbxAMatrix *s) {
	for (int y = 0; y < 4; ++y)
		for (int x = 0; x < 4; ++x)
			d->m[y][x] = (float)s->Get(y, x);
}

CFrameFBX::CFrameFBX(int no, FbxNode* pNode) {
	m_no = no;
	m_pNode = pNode;
	strcpy_s(m_name, 127, pNode->GetName());
	//FbxAMatrix& matA = pNode->EvaluateLocalTransform();
	FbxDouble3 t = pNode->LclTranslation.Get();
	FbxDouble3 r = pNode->LclRotation.Get();
	FbxDouble3 s = pNode->LclScaling.Get();
	localMatrix = CMatrix::MTranselate((float)t[0], (float)t[1], (float)t[2]) *
		CMatrix::MRotationZ(Utility::DgreeToRadian((float)r[2])) * CMatrix::MRotationY(Utility::DgreeToRadian((float)r[1])) * CMatrix::MRotationX(Utility::DgreeToRadian((float)r[0]))*
		CMatrix::MScale((float)s[0], (float)s[1], (float)s[2]);
	//CopyMatrix(&localMatrix, &matA);
	matrix = localMatrix;

	if (pNode->GetParent()) {
		CFrameFBX *p = (CFrameFBX*)pNode->GetParent()->GetUserDataPtr();
		if (p) matrix = p->matrix * matrix;

	}

}
CMatrix CFrameFBX::getWorldMatrix(const FbxTime& pTime) {
	CMatrix mat, mapP;
	FbxAMatrix matA;
	matA = m_pNode->EvaluateLocalTransform(pTime);
	CopyMatrix(&mat, &matA);
	mat = mat*matrixRev;
	if (m_pNode->GetParent()) {
		CFrameFBX *p = (CFrameFBX*)m_pNode->GetParent()->GetUserDataPtr();
		mat = p->getWorldMatrix(pTime)*mat;
	}

	return mat;
}

CModelFBX::AnimationInfo::AnimationInfo(FbxManager* m, FbxScene* s, FbxAnimStack* a, FbxTakeInfo* t)
{
	m_lSdkManager = m;
	m_lScene = s;
	m_AnimationStack = a;
	m_TakeInfo = t;
	FbxTime::EMode m_timeMode = m_lScene->GetGlobalSettings().GetTimeMode();

	FbxLongLong oneFrameValue = FbxTime::GetOneFrameValue(m_timeMode);// EMode pTimeMode=eDefaultMode
	m_start = (float)m_TakeInfo->mLocalTimeSpan.GetStart().Get()/ oneFrameValue;
	m_max = m_end = (float)m_TakeInfo->mLocalTimeSpan.GetStop().Get()/ oneFrameValue;
	
//	m_start = (float)((FbxTime)m_AnimationStack->LocalStart).GetFieldCount(m_timeMode);
//	m_max = m_end = (float)((FbxTime)m_AnimationStack->LocalStop).GetFieldCount(m_timeMode);
}

int CModelFBX::AddMaterial(FbxSurfaceMaterial *pMat) {
	if (!pMat) return 0; 
	int i = 0;
	const char* name = pMat->GetName();
	auto it = m_materialList.begin();
	while (it != m_materialList.end()) { 
		if (strcmp((*it)->m_name, name) == 0) break;
		i++;
		it++;
	}
	if (it != m_materialList.end()) return i;
	CMaterial* mat = new CMaterial();
	mat->mp_shader = CShader::GetInstance("SkinMesh");
	strcpy_s(mat->m_name, NAME_STR_SIZE,pMat->GetName());
	if (pMat->GetClassId().Is(FbxSurfacePhong::ClassId))
	{
		FbxSurfacePhong *m = static_cast<FbxSurfacePhong*>(pMat);

		mat->m_ambient.r = static_cast<float>(m->Ambient.Get()[0]);
		mat->m_ambient.g = static_cast<float>(m->Ambient.Get()[1]);
		mat->m_ambient.b = static_cast<float>(m->Ambient.Get()[2]);
		mat->m_ambient.a = 1.0f - static_cast<float>(m->TransparencyFactor.Get());

		mat->m_diffuse.r = static_cast<float>(m->Diffuse.Get()[0]);
		mat->m_diffuse.g = static_cast<float>(m->Diffuse.Get()[1]);
		mat->m_diffuse.b = static_cast<float>(m->Diffuse.Get()[2]);
		mat->m_diffuse.a = 1.0f - static_cast<float>(m->TransparencyFactor.Get());



		mat->m_specular.r = static_cast<float>(m->Specular.Get()[0]);
		mat->m_specular.g = static_cast<float>(m->Specular.Get()[1]);
		mat->m_specular.b = static_cast<float>(m->Specular.Get()[2]);


		mat->m_emissive.r = static_cast<float>(m->Emissive.Get()[0]);
		mat->m_emissive.g = static_cast<float>(m->Emissive.Get()[1]);
		mat->m_emissive.b = static_cast<float>(m->Emissive.Get()[2]);

		mat->m_shininess = (float)m->Shininess.Get();

	}
	else if (pMat->GetClassId().Is(FbxSurfaceLambert::ClassId))
	{
		FbxSurfaceLambert *m = static_cast<FbxSurfaceLambert*>(pMat);
		mat->m_ambient.r = static_cast<float>(m->Ambient.Get()[0]);
		mat->m_ambient.g = static_cast<float>(m->Ambient.Get()[1]);
		mat->m_ambient.b = static_cast<float>(m->Ambient.Get()[2]);
		mat->m_ambient.a = 1.0f - static_cast<float>(m->TransparencyFactor.Get());

		mat->m_diffuse.r = static_cast<float>(m->Diffuse.Get()[0]);
		mat->m_diffuse.g = static_cast<float>(m->Diffuse.Get()[1]);
		mat->m_diffuse.b = static_cast<float>(m->Diffuse.Get()[2]);
		mat->m_diffuse.a = 1.0f - static_cast<float>(m->TransparencyFactor.Get());

		mat->m_specular = CVector3D(0, 0, 0);
		mat->m_emissive = CVector3D(0, 0, 0);
		mat->m_shininess = 1;
	}

	FbxProperty property = pMat->FindProperty(FbxSurfaceMaterial::sDiffuse);


	int numGeneralTexture = property.GetSrcObjectCount<FbxFileTexture>();
	for (int i = 0; i < numGeneralTexture; ++i) {
		FbxTexture* texture = (FbxTexture*)(property.GetSrcObject<FbxTexture>(i));
		FbxFileTexture * lFileTexture = FbxCast<FbxFileTexture>(texture);
		mat->m_pTex = new CTexture();



		strcpy_s(mat->m_texture_name, NAME_STR_SIZE, (char*)lFileTexture->GetRelativeFileName());
		char* sc = strchr(mat->m_texture_name, ':');
		if (sc) {
			char* p = strrchr(mat->m_texture_name, '/');
			if (!p) p = strrchr(mat->m_texture_name, '\\');
			strcpy_s(mat->m_texture_name,sizeof(mat->m_texture_name), p);
		}
		char str[PATH_SIZE];
		strcpy_s(str, PATH_SIZE, m_filePath);
		strcat_s(str, PATH_SIZE, mat->m_texture_name);

		if (!mat->m_pTex->Load(str)) {
			if (!mat->m_pTex->Load((char*)lFileTexture->GetFileName())) {
				printf("テクスチャー読み込み失敗%s", lFileTexture->GetFileName());
				delete mat->m_pTex;
				mat->m_pTex = NULL;
			}
		}
		break;
	}
	m_materialList.push_back(mat);
	return i;
}
CMeshFBX::CMeshFBX(int no, FbxMesh* pMesh, FbxScene* pScene, CModelFBX* fbx) : CFrameFBX(no, pMesh->GetNode()) {
	int pointCnt = pMesh->GetControlPointsCount();
	int uvCnt = pMesh->GetTextureUVCount();
	int polyCnt = pMesh->GetPolygonCount();
	int* polygonSize = new int[polyCnt];
	int m_vertexCnt = (POINT1) ? pointCnt : uvCnt;
	mp_Vertex = new MY_VERTEX[m_vertexCnt];

	POLY_TABLE* PolyTable = new POLY_TABLE[pointCnt];
	POLY_TABLE *pT = PolyTable;
	
	unsigned int polyCnt_fix = 0;
	

	int* pIdx = pMesh->GetPolygonVertices();
	for (int i = 0; i < polyCnt; i++) {

		polygonSize[i] = pMesh->GetPolygonSize(i);
		int s = polygonSize[i] / 3;
		polyCnt_fix += s+1;

		int idxv1 = pMesh->GetPolygonVertexIndex(i);

		FbxLayerElementUV* uv = pMesh->GetLayer(0)->GetUVs();

		for (int k = 0; k < s; k++, idxv1+=2) {
			int no = k * 2;
			for (int j = 0; j < 3; no++,j++) {

				int idxvb;
				if (POINT1) {
					int idxv2 = idxv1 + (no % 4);
					idxvb = pIdx[idxv2];
				}
				else
					idxvb = pMesh->GetTextureUVIndex(i, no, FbxLayerElement::eTextureDiffuse);
				if (idxvb >= m_vertexCnt) {
					MessageBox(GL::hWnd, "エラー", "頂点数エラー", MB_OK);
				}

				int idx = pMesh->GetPolygonVertex(i, no);

				if (idx >= pointCnt) {
					MessageBox(GL::hWnd, "エラー", "頂点数エラー", MB_OK);
				}



				POLY_TABLE* pT = &PolyTable[idx];
				pT->PolyIndex.push_back(i);
				pT->Index123.push_back(no);
				pT->NumRef++;
				//if (pT->NumRef >= NUMREF_MAX) {
				//	MessageBox(GL::hWnd, "NUMREF_MAXエラー", "頂点数エラー", MB_OK);
				//}

				FbxVector4 point = pMesh->GetControlPointAt(idx);
				mp_Vertex[idxvb].vPos.x = (float)point.mData[0];
				mp_Vertex[idxvb].vPos.y = (float)point.mData[1];
				mp_Vertex[idxvb].vPos.z = (float)point.mData[2];
				//法線
				FbxVector4 normal;
				pMesh->GetPolygonVertexNormal(i, j, normal);
				mp_Vertex[idxvb].vNorm.x = (float)normal.mData[0];
				mp_Vertex[idxvb].vNorm.y = (float)normal.mData[1];
				mp_Vertex[idxvb].vNorm.z = (float)normal.mData[2];


				//テクスチャーコード
				if (uvCnt > 0 && uv->GetMappingMode() == FbxLayerElement::eByPolygonVertex) {
					int idxuv = pMesh->GetTextureUVIndex(i, no, FbxLayerElement::eTextureDiffuse);
					FbxLayerElementUV* pUV = pMesh->GetLayer(0)->GetUVs();
					FbxVector2 texcoord = pUV->GetDirectArray().GetAt(idxuv);
					mp_Vertex[idxvb].vTex.x = (float)texcoord.mData[0];
					mp_Vertex[idxvb].vTex.y = 1.0f - (float)texcoord.mData[1];
				}

			}
		}
	}

	FbxLayerElementUV* uv = pMesh->GetLayer(0)->GetUVs();
	if (uvCnt && uv->GetMappingMode() == FbxLayerElement::eByControlPoint) {
		FbxLayerElementUV* pUV = pMesh->GetLayer(0)->GetUVs();
		for (int k = 0; k<uvCnt; k++) {
			FbxVector2 texcoord = pUV->GetDirectArray().GetAt(k);
			if (k >= m_vertexCnt) {
				MessageBox(GL::hWnd, "エラー", "頂点数エラー", MB_OK);
			}
			mp_Vertex[k].vTex.x = (float)texcoord.mData[0];
			mp_Vertex[k].vTex.y = 1.0f - (float)texcoord.mData[1];
		}
	}

	//マテリアル読み込み
	m_pNode = pMesh->GetNode();
	m_material_cnt = m_pNode->GetMaterialCount();


	m_indexBuffers = new GLuint[m_material_cnt];
	memset(m_indexBuffers, 0, sizeof(GLuint) * m_material_cnt);
	m_numFace = new int[m_material_cnt];
	m_material = new int[m_material_cnt];

	mp_Indexs = new int* [m_material_cnt];
	FbxLayerElementMaterial* mat = pMesh->GetLayer(0)->GetMaterials();//レイヤーが1枚だけを想定
	for (int i = 0; i< m_material_cnt; i++) {
		
		m_material[i] = fbx->AddMaterial(m_pNode->GetMaterial(i));
	
		int matCnt = 0;
		int *pIndex = mp_Indexs[i] = new int[polyCnt_fix * 3];

		for (int k = 0; k< polyCnt; k++) {
			int matId = mat->GetIndexArray().GetAt(k);
			if (matId == i) {
				if (polygonSize[k] == 3) {
					if (POINT1) {
						pIndex[matCnt] = pMesh->GetPolygonVertex(k, 0);
						pIndex[matCnt + 1] = pMesh->GetPolygonVertex(k, 1);
						pIndex[matCnt + 2] = pMesh->GetPolygonVertex(k, 2);
					}
					else {
						pIndex[matCnt] = pMesh->GetTextureUVIndex(k, 0, FbxLayerElement::eTextureDiffuse);
						pIndex[matCnt + 1] = pMesh->GetTextureUVIndex(k, 1, FbxLayerElement::eTextureDiffuse);
						pIndex[matCnt + 2] = pMesh->GetTextureUVIndex(k, 2, FbxLayerElement::eTextureDiffuse);
					}
					matCnt += 3;
				} else {
					if (POINT1) {
						pIndex[matCnt] = pMesh->GetPolygonVertex(k, 0);
						pIndex[matCnt + 1] = pMesh->GetPolygonVertex(k, 1);
						pIndex[matCnt + 2] = pMesh->GetPolygonVertex(k, 2);

						pIndex[matCnt + 3] = pMesh->GetPolygonVertex(k, 2);
						pIndex[matCnt + 4] = pMesh->GetPolygonVertex(k, 3);
						pIndex[matCnt + 5] = pMesh->GetPolygonVertex(k, 0);

					}
					else {
						pIndex[matCnt] = pMesh->GetTextureUVIndex(k, 0, FbxLayerElement::eTextureDiffuse);
						pIndex[matCnt + 1] = pMesh->GetTextureUVIndex(k, 1, FbxLayerElement::eTextureDiffuse);
						pIndex[matCnt + 2] = pMesh->GetTextureUVIndex(k, 2, FbxLayerElement::eTextureDiffuse);

						pIndex[matCnt + 3] = pMesh->GetTextureUVIndex(k, 2, FbxLayerElement::eTextureDiffuse);
						pIndex[matCnt + 4] = pMesh->GetTextureUVIndex(k, 3, FbxLayerElement::eTextureDiffuse);
						pIndex[matCnt + 5] = pMesh->GetTextureUVIndex(k, 0, FbxLayerElement::eTextureDiffuse);
					}

					matCnt += 6;
				}
			}
		}
		m_numFace[i] = matCnt / 3;
	}


	//
	//ボーン
	FbxSkin * pSkinInfo = (FbxSkin *)pMesh->GetDeformer(0, FbxDeformer::eSkin);
	m_numBone = (pSkinInfo) ? pSkinInfo->GetClusterCount() : 0;
	if (m_numBone > 0) {
		m_ppCluster = new  FbxCluster*[m_numBone];
		for (int i = 0; i < m_numBone; i++)
		{
			m_ppCluster[i] = pSkinInfo->GetCluster(i);
		}


		if (POINT1) {
			for (int i = 0; i < m_numBone; i++) {
				int idxCnt = m_ppCluster[i]->GetControlPointIndicesCount();
				int* pIdx = m_ppCluster[i]->GetControlPointIndices();
				double* pWeight = m_ppCluster[i]->GetControlPointWeights();
				CFrameFBX* pBoneNode = (CFrameFBX*)m_ppCluster[i]->GetLink()->GetUserDataPtr();
				int bone_num = pBoneNode->m_no;
				for (int k = 0; k < idxCnt; k++) {
					for (int m = 0; m<4; m++) {
						if (pWeight[k] > mp_Vertex[pIdx[k]].bBoneWeight[m]) {
							if (pIdx[k] >= m_vertexCnt) {
								MessageBox(GL::hWnd, "エラー", "頂点数エラー", MB_OK);
							}
							for (int n = 3; n > m; n--) {
								mp_Vertex[pIdx[k]].bBoneIndex[n] = mp_Vertex[pIdx[k]].bBoneIndex[n - 1];
								mp_Vertex[pIdx[k]].bBoneWeight[n] = mp_Vertex[pIdx[k]].bBoneWeight[n - 1];
							}
							mp_Vertex[pIdx[k]].bBoneIndex[m] = (float)bone_num;
							mp_Vertex[pIdx[k]].bBoneWeight[m] = (float)pWeight[k];
							break;
						}
					}
				}
			}
		}
		else {
			//FILE* fp;
			//fopen_s(&fp,"log.txt", "a");
			int PolyIndex = 0;
			int UVIndex = 0;
			for (int i = 0; i < m_numBone; i++)
			{
				int idxCnt = m_ppCluster[i]->GetControlPointIndicesCount();
				int* pIdx = m_ppCluster[i]->GetControlPointIndices();
				double* pWeight = m_ppCluster[i]->GetControlPointWeights();
				CFrameFBX* pBoneNode = (CFrameFBX*)m_ppCluster[i]->GetLink()->GetUserDataPtr();
				int bone_num = pBoneNode->m_no;
				//std::map<int, int> cnt;

				//fprintf(fp, "\n");
				for (int k = 0; k < idxCnt; k++)
				{

					//fprintf(fp, "pIdx %d Weight %.2f\n", pIdx[k],  pWeight[k]);
					//cnt[pIdx[k]]++;
					if (pIdx[k] >= pointCnt) {
						MessageBox(GL::hWnd, "エラー", "頂点数エラー", MB_OK);
					}


					for (int p = 0; p < PolyTable[pIdx[k]].NumRef; p++)
					{
						PolyIndex = PolyTable[pIdx[k]].PolyIndex[p];
						UVIndex = pMesh->GetTextureUVIndex(PolyIndex, PolyTable[pIdx[k]].Index123[p], FbxLayerElement::eTextureDiffuse);
						if (UVIndex >= m_vertexCnt) {
							MessageBox(GL::hWnd, "エラー", "頂点数エラー", MB_OK);
						}
						int n = PolyTable[pIdx[k]].PolyIndex[p] * 3 + PolyTable[pIdx[k]].Index123[p];
						bool t = false;

						for (int m=0; m < 4; m++) {
							if (mp_Vertex[UVIndex].bBoneWeight[m] == 0.0) break;
							if (mp_Vertex[UVIndex].bBoneIndex[m] == (float)bone_num) {
								t = true;
								break;
							}
						}
						if (t) continue;
						for (int m = 0; m<4; m++) {
							if (pWeight[k] > mp_Vertex[UVIndex].bBoneWeight[m]) {
								for (int n = 3; n > m; n--) {
									mp_Vertex[UVIndex].bBoneIndex[n] = mp_Vertex[UVIndex].bBoneIndex[n - 1];
									mp_Vertex[UVIndex].bBoneWeight[n] = mp_Vertex[UVIndex].bBoneWeight[n - 1];
								}
								mp_Vertex[UVIndex].bBoneIndex[m] = (float)bone_num;
								mp_Vertex[UVIndex].bBoneWeight[m] = (float)pWeight[k];
								break;
							}
						}
					}

				}
			}
			//fclose(fp);

		}

		//
		//ボーンを生成
		m_BoneArray = new SBoneFBX[m_numBone];
		//FILE* fp;
		for (int i = 0; i < m_numBone; i++)
		{
			FbxAMatrix mat, trans;
			m_ppCluster[i]->GetTransformLinkMatrix(mat);
			m_ppCluster[i]->GetTransformMatrix(trans);
			CopyMatrix(&m_BoneArray[i].mTransMatrix, &trans);
			CopyMatrix(&m_BoneArray[i].mBindPose, &mat);
			m_BoneArray[i].mBindPoseInv = m_BoneArray[i].mBindPose.GetInverse();
			CFrameFBX* pFrameFBX = (CFrameFBX*)m_ppCluster[i]->GetLink()->GetUserDataPtr();
			pFrameFBX->matrixOffset = m_BoneArray[i].mBindPoseInv * m_BoneArray[i].mTransMatrix;
			m_BoneArray[i].mOffset = m_BoneArray[i].mBindPoseInv * m_BoneArray[i].mTransMatrix;
			//fopen_s(&fp, "log.txt", "a");
			//fprintf(fp, "-------------\n%s\n", m_ppCluster[i]->GetName());
			//printMatrix(fp, pFrameFBX->matrixOffset);
			//printMatrix(fp, m_BoneArray[i].mBindPoseInv);
			//printMatrix(fp, m_BoneArray[i].mTransMatrix);
			//fclose(fp);
		}
		for (int i = 0; i < m_vertexCnt; i++) {
			float w = 0;
			for (int j = 0; j < 4; j++) {
				w += mp_Vertex[i].bBoneWeight[j];
			}
			if (w > 1.0) {
				for (int j = 0; j < 4; j++) {
					mp_Vertex[i].bBoneWeight[j] /= w;
				}
			}
		}
	}

	delete[] polygonSize;
	delete[] PolyTable;




}
CMeshFBX::~CMeshFBX() {

	if(m_vertexBuffer) glDeleteBuffers(1, &m_vertexBuffer);
	if(m_indexBuffers[0]) glDeleteBuffers(m_material_cnt, m_indexBuffers);
	if (mp_Vertex) {
		delete[] mp_Vertex;
		mp_Vertex = nullptr;
	}
	if (mp_Indexs) {
		for (int i = 0; i < m_material_cnt; i++) {
			delete[] mp_Indexs[i];
		}
		delete[] mp_Indexs;
		mp_Indexs = nullptr;
	}

	delete[] m_indexBuffers;
	delete[] m_numFace;
	delete[] m_material;
	delete[] m_ppCluster;
	delete[] m_BoneArray;
}
void CMeshFBX::CalcBoneMatrix(CMatrix* boneMatrix,float blend) {
	for (int i = 0; i<m_numBone; i++)
	{
		CFrameFBX* pFrame = (CFrameFBX*)m_ppCluster[i]->GetLink()->GetUserDataPtr();
		//	FbxAMatrix mat = m_ppCluster[i]->GetLink()->EvaluateGlobalTransform(pTime);

		//	CopyMatrix(&m_BoneArray[i].mAnimPose, &mat);
		m_BoneArray[i].mAnimPose = pFrame->matrix;
		if (blend < 1) {
			pFrame->boneMatrix = m_BoneArray[i].mAnimPose * blend + m_BoneArray[i].mOldPose*(1 - blend);

		}
		else {
			pFrame->boneMatrix = m_BoneArray[i].mAnimPose;
		}
		boneMatrix[pFrame->m_no] = pFrame->boneMatrix * m_BoneArray[i].mOffset;


	}
}
void CMeshFBX::SaveBoneMatrix() {
	for (int i = 0; i<m_numBone; i++)
	{
		m_BoneArray[i].mOldPose = m_BoneArray[i].mAnimPose;
	}
}
void CMeshFBX::Render(const CMatrix& mv, const CMatrix& m, std::vector<CMaterial*>& materialList,CMatrix* boneMatrix,float blend) {
	BYTE* offset = 0;
	CalcBoneMatrix(boneMatrix,blend);
	if (!m_vertexBuffer) {
		//バーテックスバッファーを作成
		glGenBuffers(1, &m_vertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(MY_VERTEX) * m_vertexCnt, mp_Vertex, GL_STATIC_DRAW);

		glGenBuffers(m_material_cnt, m_indexBuffers);
		for (int i = 0; i < m_material_cnt; i++) {
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffers[i]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<unsigned long long>(m_numFace[i]) * 3 * sizeof(int), mp_Indexs[i], GL_STATIC_DRAW);
			delete[] mp_Indexs[i];
		}


		delete[] mp_Vertex;
		mp_Vertex = nullptr;
		delete[] mp_Indexs;
		mp_Indexs = nullptr;
	}
	glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
	glEnableVertexAttribArray(CShader::eVertexLocation);
	glVertexAttribPointer(CShader::eVertexLocation, 3, GL_FLOAT, GL_FALSE, sizeof(MY_VERTEX), (const void*)offset);
	offset += sizeof(SVector3D);


	glEnableVertexAttribArray(CShader::eNormalLocation);
	glVertexAttribPointer(CShader::eNormalLocation, 3, GL_FLOAT, GL_FALSE, sizeof(MY_VERTEX), (const void*)offset);
	offset += sizeof(SVector3D);

	glEnableVertexAttribArray(CShader::eTexcoordlLocation);
	glVertexAttribPointer(CShader::eTexcoordlLocation, 2, GL_FLOAT, GL_FALSE, sizeof(MY_VERTEX), (const void*)offset);
	offset += sizeof(SVector2D);


	//if (bWeight) {

		glEnableVertexAttribArray(CShader::eWeightsLocation);
		glVertexAttribPointer(CShader::eWeightsLocation, 4, GL_FLOAT, GL_TRUE, sizeof(MY_VERTEX), (void*)offset);
		offset += sizeof(float) * 4;


		glEnableVertexAttribArray(CShader::eIndicesLocation);
		glVertexAttribPointer(CShader::eIndicesLocation, 4, GL_FLOAT, GL_FALSE, sizeof(MY_VERTEX), (void*)offset);
	//}




	for (int i = 0; i < m_material_cnt; i++) {
		CMaterial* mat = materialList[m_material[i]];
		CShader* s = mat->mp_shader;
		s->Enable();
		CModel::SendShaderParam(s,m, CCamera::GetCurrent()->GetViewMatrix(), CCamera::GetCurrent()->GetProjectionMatrix());
		if (m_numBone > 0) {
			int MatrixLocation = glGetUniformLocation(s->GetProgram(), "Transforms");
			glUniformMatrix4fv(MatrixLocation, m_numBone, GL_FALSE, boneMatrix->f);
			glUniform1i(glGetUniformLocation(s->GetProgram(), "useSkin"), 1);
		}
		else {
			glUniform1i(glGetUniformLocation(s->GetProgram(), "useSkin"), 0);
		}

		mat->Map();

		glUniformMatrix4fv(glGetUniformLocation(s->GetProgram(), "LocalMatrix"), 1, GL_FALSE, CMatrix::indentity.f);
		glUniformMatrix4fv(glGetUniformLocation(s->GetProgram(), "ModelViewMatrix"), 1, GL_FALSE, mv.f);

		glUniformMatrix4fv(glGetUniformLocation(s->GetProgram(), "WorldMatrix"), 1, false, m.f);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffers[i]);
		glDrawElements(GL_TRIANGLES, m_numFace[i] * 3, GL_UNSIGNED_INT, NULL);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		mat->Unmap();
		s->Disable();
	}


	glDisableVertexAttribArray(CShader::eVertexLocation);
	glDisableVertexAttribArray(CShader::eNormalLocation);
	glDisableVertexAttribArray(CShader::eTexcoordlLocation);
	glDisableVertexAttribArray(CShader::eWeightsLocation);
	glDisableVertexAttribArray(CShader::eIndicesLocation);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

}
bool CMeshFBX::CollisionRay(CVector3D *pCorss, CVector3D *pNormal, const CVector3D &s, const CVector3D &e, float *pLength) {
	MY_VERTEX* v = nullptr;
	if (m_vertexBuffer) {
		glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
		v=(MY_VERTEX*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
	}
	else {
		v = mp_Vertex;
	}
	bool hit = false;
	for (int i = 0; i < m_material_cnt; i++)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffers[i]);
		int *idx = (int*)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_READ_ONLY);
		for (int j = 0; j < m_numFace[i]; j++) {
			if (CCollision::IntersectTriangleRay(pCorss, s, e, v[idx[0]].vPos, v[idx[1]].vPos, v[idx[2]].vPos, pLength)) {

				CVector3D e1 = (CVector3D)v[idx[1]].vPos - (CVector3D)v[idx[0]].vPos;
				CVector3D e2 = (CVector3D)v[idx[2]].vPos - (CVector3D)v[idx[0]].vPos;

				*pNormal = CVector3D::Cross(e1, e2).GetNormalize();
				hit = true;
			}
			idx += 3;
		}
		glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	return hit;
}

int CMeshFBX::CollisionSphere(CCollTriangle *out, const  CVector3D &center, float radius, int maxcnt) {
	glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
	MY_VERTEX *v = (MY_VERTEX*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
	int cnt = 0;
	for (int i = 0; i < m_material_cnt; i++)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffers[i]);
		int *idx = (int*)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_READ_ONLY);
		for (int j = 0; j < m_numFace[i]; j++) {
			float dist;
			if (CCollision::CollisionTriangleSphere(v[idx[0]].vPos, v[idx[1]].vPos, v[idx[2]].vPos, center, radius, NULL, &dist)) {
				out->m_dist = dist;
				out->m_material_id = i;
				CVector3D e1 = (CVector3D)v[idx[1]].vPos - (CVector3D)v[idx[0]].vPos;
				CVector3D e2 = (CVector3D)v[idx[2]].vPos - (CVector3D)v[idx[0]].vPos;

				out->m_normal = CVector3D::Cross(e1, e2).GetNormalize();
				out->m_vertex[0] = v[idx[0]].vPos;
				out->m_vertex[1] = v[idx[1]].vPos;
				out->m_vertex[2] = v[idx[2]].vPos;
				out++;
				cnt++;
				if (cnt > maxcnt) break;
			}
			idx += 3;
			if (cnt > maxcnt) break;
		}
		glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	return cnt;
}

int CMeshFBX::CollisionCupsel(CCollTriangle *out, const CVector3D &top, const CVector3D &bottom, float radius, int maxcnt) {

	glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
	MY_VERTEX *v = (MY_VERTEX*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
	int cnt = 0;
	for (int i = 0; i < m_material_cnt; i++)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffers[i]);
		int *idx = (int*)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_READ_ONLY);
		for (int j = 0; j < m_numFace[i]; j++) {
			float dist;
			if (CCollision::CollisionTriangleCapsule(v[idx[0]].vPos, v[idx[1]].vPos, v[idx[2]].vPos, top, bottom, radius, NULL, &dist)) {
				out->m_dist = dist;
				out->m_material_id = i;
				CVector3D e1 = (CVector3D)v[idx[1]].vPos - (CVector3D)v[idx[0]].vPos;
				CVector3D e2 = (CVector3D)v[idx[2]].vPos - (CVector3D)v[idx[0]].vPos;
				out->m_normal = CVector3D::Cross(e1, e2).GetNormalize();
				out->m_vertex[0] = v[idx[0]].vPos;
				out->m_vertex[1] = v[idx[1]].vPos;
				out->m_vertex[2] = v[idx[2]].vPos;
				out++;
				cnt++;
				if (cnt > maxcnt) break;
			}
			idx += 3;
			if (cnt > maxcnt) break;
		}
		glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	return cnt;
}

void CModelFBX::createNode(FbxNode* pNode, int lv, int &no, bool preCreate) {
	for (int i = 0; i < lv; i++) printf(" ");
	printf("%s ", pNode->GetName());
	CFrameFBX* pFrameFBX;
	if (preCreate) {
		pFrameFBX = new CFrameFBX(no, pNode);
		pNode->SetUserDataPtr(pFrameFBX);
	}
	else {
		pFrameFBX = (CFrameFBX*)pNode->GetUserDataPtr();
	}
	if (pNode->GetNodeAttribute()) {
		FbxNodeAttribute::EType lAttributeType;
		lAttributeType = pNode->GetNodeAttribute()->GetAttributeType();
		switch (lAttributeType) {
		case FbxNodeAttribute::eSkeleton:
			printf("Skeleton");
			break;
		case FbxNodeAttribute::eMesh:
			printf("Mesh");
			if (!preCreate) {
				CMeshFBX* pMeshFBX = new CMeshFBX(pFrameFBX->m_no, pNode->GetMesh(), m_lScene,this);
				pNode->SetUserDataPtr(pMeshFBX);
				delete pFrameFBX;
				pFrameFBX = pMeshFBX;
			}
			break;
		default:
			break;
		}
	}
	printf("\n");
	for (int i = 0; i < pNode->GetChildCount(); i++) {
		no++;
		createNode(pNode->GetChild(i), lv + 1, no, preCreate);
	}
}
void CModelFBX::drawNode(FbxNode* pNode, CMatrix mv, CMatrix m) {
	CFrameFBX *pFrameFBX = (CFrameFBX*)pNode->GetUserDataPtr();
	//glPushMatrix();
	if (m_animJam < 0) {
		//glMultMatrixf(pFrameFBX->localMatrix.f);
		mv = mv * pFrameFBX->localMatrix;
		m = m * pFrameFBX->localMatrix;
	}
	if (pNode->GetNodeAttribute()) {
		FbxNodeAttribute::EType lAttributeType;
		lAttributeType = pNode->GetNodeAttribute()->GetAttributeType();

		switch (lAttributeType) {
		case FbxNodeAttribute::eSkeleton:
//			glMultMatrixf(pFrameFBX->localMatrix.f);
//			m = m * pFrameFBX->localMatrix;

			break;
		case FbxNodeAttribute::eMesh:
		{

	

		//	CMatrix mModelView;
		//	glGetFloatv(GL_MODELVIEW_MATRIX, mModelView.f);

			CMeshFBX* pMeshFBX = (CMeshFBX*)pNode->GetUserDataPtr();
			pMeshFBX->Render(mv,m, m_materialList, m_boneMatrix,m_blend);

		}
		break;
		}
	}
	for (int i = 0; i < pNode->GetChildCount(); i++) {
		drawNode(pNode->GetChild(i), mv,m);
	}
	//glPopMatrix();
}

void CModelFBX::updateBoneMatrix(FbxNode* pNode, const FbxTime& pTime) {
	CFrameFBX* f = (CMeshFBX*)pNode->GetUserDataPtr();
	CMatrix mat;
	FbxAMatrix matA;
	FbxNode* p = m_lCurrentAnimationStack->m_lScene->FindNodeByName(pNode->GetName());
	if (p) {
		matA = p->EvaluateLocalTransform(pTime);
		CopyMatrix(&f->localMatrix, &matA);
		f->matrix = f->localMatrix * f->matrixRev;
	}
	if (pNode->GetParent()) {
		CFrameFBX *p = (CFrameFBX*)pNode->GetParent()->GetUserDataPtr();
		f->matrix = p->matrix*f->matrix;
	}


	for (int i = 0; i < pNode->GetChildCount(); i++) {
		updateBoneMatrix(pNode->GetChild(i), pTime);
	}
}
void CModelFBX::destroyNode(FbxNode* pNode, int lv) {
	if (pNode->GetNodeAttribute() && pNode->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eMesh) {
		CMeshFBX* pMeshFBX = (CMeshFBX*)pNode->GetUserDataPtr();
		delete pMeshFBX;
		pNode->SetUserDataPtr(NULL);
	}
	else {
		CFrameFBX* pFrameFBX = (CFrameFBX*)pNode->GetUserDataPtr();
		delete pFrameFBX;
		pNode->SetUserDataPtr(NULL);

	}
	for (int i = 0; i < pNode->GetChildCount(); i++) {
		destroyNode(pNode->GetChild(i), lv + 1);
	}
}

CModelFBX::CModelFBX() :m_lSdkManager(NULL), m_lScene(NULL), m_AnimStackNameArray(NULL), m_lCurrentAnimationStack(NULL), m_pAnimData(NULL)
{

}

CModelFBX::CModelFBX(CModelFBX &m) {
	memcpy(this, &m, sizeof(CModelFBX));
}



bool CModelFBX::Load(const char* filePath) {


	MakePath(filePath);
	m_lSdkManager = FbxManager::Create();


	FbxIOSettings* ios = FbxIOSettings::Create(m_lSdkManager, IOSROOT);
	m_lSdkManager->SetIOSettings(ios);


	FbxImporter* lImporter = FbxImporter::Create(m_lSdkManager, "");



	bool lImportStatus = lImporter->Initialize(filePath, -1, m_lSdkManager->GetIOSettings());

	int lFileMajor, lFileMinor, lFileRevision;
	lImporter->GetFileVersion(lFileMajor, lFileMinor, lFileRevision);
	if (!lImportStatus) {
		printf("Call to FbxImporter::Initialize() failed.\n");
		printf("Error returned: %s\n\n", lImporter->GetStatus().GetErrorString());
		return false;
	}



	printf("%d.%d.%d\n", lFileMajor, lFileMinor, lFileRevision);



	m_lScene = FbxScene::Create(m_lSdkManager, "myScene");


	lImporter->Import(m_lScene);

	int anim_size = m_anim_size = lImporter->GetAnimStackCount();




	m_AnimStackNameArray = new FbxArray < FbxString* >;
	m_lScene->FillAnimStackNameArray(*m_AnimStackNameArray);
	//m_lCurrentAnimationStack = m_lScene->GetSrcObject<FbxAnimStack>();
	if (anim_size > 0) {
		for (int i = 0; i < anim_size; i++) {
			m_animation_list.push_back(new AnimationInfo(m_lSdkManager, m_lScene,
				m_lScene->GetSrcObject<FbxAnimStack>(i), lImporter->GetTakeInfo(i)));
		}

		m_lCurrentAnimationStack = m_animation_list[0];
	}
	lImporter->Destroy();
	ios->Destroy();


	m_timeMode = m_lScene->GetGlobalSettings().GetTimeMode();


	
	m_animJam = -1;
	m_animSpeed = 1.0f;
	m_animTime = m_animStartFrame = m_animEndFrame = 0;
	m_pAnimData = NULL;




	FbxNode* lRootNode = m_lScene->GetRootNode();
	int no = 0;


	//１回目階層構築
	createNode(lRootNode, 0, no, true);
	m_bone_num = no;
	m_boneMatrix = new CMatrix[m_bone_num];
	//２回めメッシュの読み込み
	createNode(lRootNode, 0, no, false);


	return true;


}

void CModelFBX::Render()
{
	UpdateMatrix();
	Render(m_matrix);
}

void CModelFBX::UpdateMatrix()
{
	CModel::UpdateMatrix();
	UpdateMatrix(m_matrix);
}



void CModelFBX::Render(CMatrix &m) {


	m_matrix = m;
	//glPushMatrix();
	//glMultMatrixf(m.f);

//	SendShaderParam(m);
	
	
	UpdateMatrix(m_matrix);
	drawNode(m_lScene->GetRootNode(),CCamera::GetCurrent()->GetViewMatrix()* m_matrix,m_matrix);

	//glPopMatrix();

}
void CModelFBX::UpdateMatrix(CMatrix& m)
{
	m_matrix = m;
	if (m_animJam >= 0) {
		float time;
		if (m_pAnimData) {
			time = m_pAnimData[m_animJam].start + m_animTime;
		}
		else {
			if (m_lCurrentAnimationStack)
				m_lCurrentAnimationStack->m_lScene->SetCurrentAnimationStack(m_lCurrentAnimationStack->m_AnimationStack);
			time = m_animTime;
		}


		FbxTime::SetGlobalTimeMode(m_timeMode);
		FbxTime f_time;
		f_time.SetTime(0, 0, 0, (int)time, 0, 0, FbxTime::eFrames60);
		//	Time.SetTime(0, 0, 0, (int)time, 0 , 0, m_timeMode);

		updateBoneMatrix(m_lScene->GetRootNode(), f_time);
	}
}
void CModelFBX::Release() {


	for (unsigned int i = 0; i < m_materialList.size(); i++) {
		m_materialList[i]->Release();
		delete m_materialList[i];
	}

	if (m_AnimStackNameArray) {
		FbxArrayDelete(*m_AnimStackNameArray);
		delete m_AnimStackNameArray;
		m_AnimStackNameArray = NULL;
	}
	if (m_lScene) {
		destroyNode(m_lScene->GetRootNode(), 0);
		m_lScene->Destroy(); m_lScene = NULL;
	}
	if (m_lSdkManager) { m_lSdkManager->Destroy(); m_lSdkManager = NULL; }
	delete[] m_boneMatrix;
	//if (m_shader) { delete m_shader; m_shader = NULL; }
}
void CModelFBX::ChangeAnimation(int jam, bool loop, bool check, float time, float blendtime) {
	if (!m_pAnimData && jam >=m_anim_size) return;
	if (check && m_animJam == jam) return;
	m_animJam = jam;
	m_animLoop = loop;
	m_blendvec = (blendtime >0) ? 1.0f / blendtime : 1.0f;
	m_blend = 0;
	int meshCnt = m_lScene->GetSrcObjectCount<FbxMesh>();
	for (int i = 0; i < meshCnt; i++) {
		FbxMesh *pMesh = m_lScene->GetSrcObject<FbxMesh>(i);
		CMeshFBX *m = (CMeshFBX*)pMesh->GetNode()->GetUserDataPtr();
		m->SaveBoneMatrix();
	}

	if (m_pAnimData) {
		m_animEndFrame = (float)(m_pAnimData[m_animJam].end - m_pAnimData[m_animJam].start);
		m_animTime = m_animStartFrame = 0;
	}
	else {
		//current =  m_lScene->FindMember<FbxAnimStack>((*m_AnimStackNameArray)[jam]->Buffer());
		m_lCurrentAnimationStack = m_animation_list[jam];

		FbxAnimStack* current = m_lCurrentAnimationStack->m_AnimationStack;
		m_animEndFrame = m_lCurrentAnimationStack->m_end;

		m_animTime = m_animStartFrame = m_lCurrentAnimationStack->m_start;
	}

}

float CModelFBX::GetAnimationFrame() {
	return m_animTime;
}
int CModelFBX::GetAnimationSize() const
{
	return m_anim_size;
}
void CModelFBX::UpdateAnimation() {
	m_blend += m_blendvec;
	if (m_blend > 1) m_blend = 1;
	m_animTime += m_animSpeed * 60*CFPS::GetDeltaTime();
	if (m_animTime >= m_lCurrentAnimationStack->m_end) {
		if (m_animLoop) {
			m_animTime = m_animStartFrame;
		}
		else {
			m_animTime = m_animEndFrame;
		}
	}
}
bool CModelFBX::isAnimationEnd() {
	return (m_animTime >= m_animEndFrame);
}

int CModelFBX::GetAnimationJam() {
	return m_animJam;
}

void CModelFBX::SetAnimationSpeed(float s) {
	m_animSpeed = s;
}

void CModelFBX::AttachAnimData(SAnimData *p) {
	m_pAnimData = p;
}

CMatrix CModelFBX::getFrameMatrix(FbxNode *pFrame, bool local) {
	CFrameFBX *CFrame = (CFrameFBX*)pFrame->GetUserDataPtr();
	if (!CFrame) return m_matrix;

	return (local) ? CFrame->matrix : m_matrix * CFrame->matrix;

}

void CModelFBX::setFrameRevMatrix(FbxNode *pFrame, CMatrix &m) {
	CFrameFBX *CFrame = (CFrameFBX*)pFrame->GetUserDataPtr();
	if (CFrame) {
		CFrame->matrixRev = m;
	}

}
CMatrix CModelFBX::GetFrameMatrix(const char *name, bool local) {

	FbxString str(name);
	FbxNode *pFrame = m_lScene->FindMember<FbxNode>(name);
	if (!pFrame) return m_matrix;
	return getFrameMatrix(pFrame, local);
}
CMatrix CModelFBX::GetFrameMatrix(int no, bool local) {
	FbxNode *pFrame = m_lScene->GetSrcObject<FbxNode>(no);
	if (!pFrame) return m_matrix;
	return getFrameMatrix(pFrame, local);
}

void CModelFBX::SetFrameRevMatrix(const char *name, CMatrix &m) {
	FbxString str(name);
	FbxNode *pFrame = m_lScene->FindMember<FbxNode>(name);
	if (!pFrame) return;
	setFrameRevMatrix(pFrame, m);

}
void CModelFBX::SetFrameRevMatrix(int no, CMatrix &m) {
	FbxNode *pFrame = m_lScene->GetSrcObject<FbxNode>(no);
	if (!pFrame) return;
	setFrameRevMatrix(pFrame, m);

}

CMaterial *CModelFBX::GetMaterial(int no)const {
	return m_materialList[no];
}

void CModelFBX::SetMaterial(int no, CMaterial *m) {
	m_materialList[no]=m;
}
/*
bool CModelFBX::CollisionRay(CVector3D *pCross, CVector3D *pNormal, const CVector3D &s, const CVector3D &e) {
	CVector3D cross, normal;
	float lengh = 1e10;
	bool hit = false;
	CVector3D c, n;

	CMatrix inv = m_matrix.GetInverse();
	CVector3D s2 = inv * CVector4D(s, 1);
	CVector3D e2 = inv * CVector4D(e, 1);

	int meshCnt = m_lScene->GetSrcObjectCount<FbxMesh>();
	for (int i = 0; i < meshCnt; i++) {
		FbxNode *pNode = m_lScene->GetSrcObject<FbxMesh>(i)->GetNode();
		CMeshFBX *pMeshFrame = (CMeshFBX*)pNode->GetUserDataPtr();
		if (!pMeshFrame) continue;

		CMatrix mat_mesh = pMeshFrame->matrix;
		CMatrix inv_mesh = mat_mesh.GetInverse();
		CVector3D s2_mesh = inv_mesh * CVector4D(s2, 1);
		CVector3D e2_mesh = inv_mesh * CVector4D(e2, 1);

		if (pMeshFrame->CollisionRay(&c, &n, s2_mesh, e2_mesh, &lengh)) {
			c = mat_mesh*CVector4D(c, 1);
			n = CVector3D(mat_mesh*CVector4D(n, 0)).GetNormalize();
			hit = true;
		}
	}
	if (hit) {
		if (pCross) {
			*pCross = m_matrix*CVector4D(c, 1);
		}
		if (pNormal) {
			*pNormal = CVector3D(m_matrix*CVector4D(n, 0)).GetNormalize();
		}
	}
	return hit;

}

int CModelFBX::CollisionSphere(CCollTriangle *out, const CVector3D &center, float radius, int maxcnt) {
	float scale = CVector3D(m_matrix.m00, m_matrix.m10, m_matrix.m20).Length();
	int cnt = 0;
	CCollTriangle *o = out;
	CVector3D p = m_matrix.GetInverse() * CVector4D(center, 1);
	float r = radius / scale;

	int meshCnt = m_lScene->GetSrcObjectCount<FbxMesh>();
	for (int i = 0; i < meshCnt; i++) {
		FbxNode *pNode = m_lScene->GetSrcObject<FbxMesh>(i)->GetNode();
		CMeshFBX *pMeshFrame = (CMeshFBX*)pNode->GetUserDataPtr();
		if (!pMeshFrame) continue;

		CMatrix mat_mesh = pMeshFrame->matrix;
		CMatrix inv_mesh = mat_mesh.GetInverse();
		CVector3D p_mesh = inv_mesh * CVector4D(p, 1);
		float scale_mesh = CVector3D(mat_mesh.m00, mat_mesh.m10, mat_mesh.m20).Length();
		float r_mesh = r / scale_mesh;


		int c = pMeshFrame->CollisionSphere(o, p_mesh, r_mesh, maxcnt);
		for (int k = 0; k < c; k++) {
			o[k].m_dist *= scale_mesh;
			o[k].Transform(mat_mesh);
		}

		o += c;
		cnt += c;
		if (cnt > maxcnt) break;

	}
	o = out;
	for (int i = 0; i < cnt; i++, o++) {
		o->m_dist *= scale;
		o->Transform(m_matrix);

	}
	return cnt;
}

int CModelFBX::CollisionCupsel(CCollTriangle *out, const CVector3D &top, const CVector3D &bottom, float radius, int maxcnt) {
	float scale = CVector3D(m_matrix.m00, m_matrix.m10, m_matrix.m20).Length();
	int cnt = 0;
	CCollTriangle *o = out;
	CMatrix inv = m_matrix.GetInverse();
	CVector3D t = inv * CVector4D(top, 1);
	CVector3D b = inv * CVector4D(bottom, 1);
	float r = radius / scale;

	int meshCnt = m_lScene->GetSrcObjectCount<FbxMesh>();
	for (int i = 0; i < meshCnt; i++) {
		FbxNode *pNode = m_lScene->GetSrcObject<FbxMesh>(i)->GetNode();
		CMeshFBX *pMeshFrame = (CMeshFBX*)pNode->GetUserDataPtr();
		if (!pMeshFrame) continue;

		CMatrix mat_mesh = pMeshFrame->matrix;
		CMatrix inv_mesh = mat_mesh.GetInverse();
		CVector3D t_mesh = inv_mesh * CVector4D(t, 1);
		CVector3D b_mesh = inv_mesh * CVector4D(b, 1);
		float scale_mesh = CVector3D(mat_mesh.m00, mat_mesh.m10, mat_mesh.m20).Length();
		float r_mesh = r / scale_mesh;

		int c = pMeshFrame->CollisionCupsel(o, t_mesh, b_mesh, r_mesh, maxcnt);
		for (int k = 0; k < c; k++) {
			o[k].m_dist *= scale_mesh;
			o[k].Transform(mat_mesh);
		}
		o += c;
		cnt += c;
		if (cnt > maxcnt) break;

	}
	o = out;
	for (int i = 0; i < cnt; i++, o++) {
		o->m_dist *= scale;
		o->Transform(m_matrix);

	}
	return cnt;

}
*/

bool CModelFBX::AddAnimation(const char* filePath) {
	CModelFBX::FBXInfo*a = new CModelFBX::FBXInfo();
	a->m_lSdkManager = FbxManager::Create();
	FbxIOSettings * ios = FbxIOSettings::Create(a->m_lSdkManager, IOSROOT);
	a->m_lSdkManager->SetIOSettings(ios);


	FbxImporter* lImporter = FbxImporter::Create(a->m_lSdkManager, "");



	bool lImportStatus = lImporter->Initialize(filePath, -1, a->m_lSdkManager->GetIOSettings());
	if (!lImportStatus) {
		printf("Call to FbxImporter::Initialize() failed.\n");
		printf("Error returned: %s\n\n", lImporter->GetStatus().GetErrorString());
		return false;
	}

	int lFileMajor, lFileMinor, lFileRevision;


	lImporter->GetFileVersion(lFileMajor, lFileMinor, lFileRevision);
	printf("%d.%d.%d\n", lFileMajor, lFileMinor, lFileRevision);



	a->m_lScene = FbxScene::Create(a->m_lSdkManager, "myScene");

	int anim_size = lImporter->GetAnimStackCount();

	lImporter->Import(a->m_lScene);

	
	a->m_AnimStackNameArray = new FbxArray < FbxString* >;
	a->m_lScene->FillAnimStackNameArray(*a->m_AnimStackNameArray);

	for (int i = 0; i < anim_size; i++, m_anim_size++) {
		m_animation_list.push_back(new AnimationInfo( a->m_lSdkManager,a->m_lScene,
			a->m_lScene->GetSrcObject<FbxAnimStack>(i), lImporter->GetTakeInfo(i)));
	}

	lImporter->Destroy();
	ios->Destroy();
	
	return true;



}
