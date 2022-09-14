#pragma once


#include "GL.h"
#include "CModel.h"
#include "Vertex.h"
#include "CTexture.h"
#include "CShader.h"
#include "fbxsdk.h"
#include <vector>
#include <list>


#ifdef _DEBUG
#pragma comment(lib,"libfbxsdk-mtd.lib")
#pragma comment(lib,"libxml2-mtd.lib")
#pragma comment(lib,"zlib-mtd.lib")
#else
#pragma comment(lib,"libfbxsdk-mt.lib")
#pragma comment(lib,"libxml2-mt.lib")
#pragma comment(lib,"zlib-mt.lib")
#endif
class CModelFBX;
class CFrameFBX{
public:

	char m_name[128];
	int	 m_no;			//�t���[���ԍ�
	FbxNode* m_pNode;

	CMatrix animMatrix;
	CMatrix boneMatrix;
	CMatrix matrix;
	CMatrix matrixOffset;
	CMatrix matrixRev;
	CMatrix localMatrix;
public:
	CFrameFBX() {
	}
	CFrameFBX(int no, FbxNode* pNode);
	CMatrix getWorldMatrix(const FbxTime& pTime);
	friend class CModelA3M;
	
};


//�{�[���\����
struct SBoneFBX
{
	CMatrix mBindPose;
	CMatrix mBindPoseInv;
	CMatrix mTransMatrix;
	CMatrix mAnimPose;
	CMatrix mOldPose;
	CMatrix mOffset;

	SBoneFBX()
	{
		ZeroMemory(this, sizeof(SBoneFBX));
	}
};
class CMeshFBX : public CFrameFBX{
public:
	GLuint* m_indexBuffers;
	GLuint m_vertexBuffer;
	int* m_numFace;
	int* m_material;
	int m_material_cnt;
	int		m_numBone;
	FbxCluster **m_ppCluster;
	SBoneFBX *m_BoneArray;

	FbxNode *m_pNode;
	MY_VERTEX* mp_Vertex;
	unsigned int m_vertexCnt;
	int** mp_Indexs;



	void CalcBoneMatrix(CMatrix* boneMatrix,float blend);
public:
	CMeshFBX(int no, FbxMesh* pMesh, FbxScene* pScene, CModelFBX* fbx);
	~CMeshFBX();
	void Render(const CMatrix& mv, const CMatrix& m, std::vector<CMaterial*>& materialList, CMatrix* boneMatrix, float blend);
	bool CollisionRay(CVector3D *c, CVector3D *n, const CVector3D &s, const CVector3D &e,float *lengh);

	int CollisionSphere(CCollTriangle *out, const CVector3D &center, float radius,int maxcnt);


	int CollisionCupsel(CCollTriangle *out, const CVector3D &top, const CVector3D &bottom, float radius, int maxcnt);


	void SaveBoneMatrix();
};
class CModelFBX : public CModel {
public:
	struct AnimationInfo {
		FbxManager* m_lSdkManager;
		FbxScene* m_lScene;
		FbxAnimStack* m_AnimationStack;
		FbxTakeInfo* m_TakeInfo;
		union {
			struct {
				float			m_start;
				float			m_end;
			};
			float time[2];
		};
		float			m_max;
		AnimationInfo(FbxManager* m, FbxScene* s, FbxAnimStack* a, FbxTakeInfo* t);
	};
private:

	int AddMaterial(FbxSurfaceMaterial* pMat);
	struct FBXInfo {
		FbxManager* m_lSdkManager;
		FbxScene* m_lScene;
		FbxArray<FbxString*>* m_AnimStackNameArray;
	};
	std::vector<AnimationInfo*> m_animation_list;

	std::vector<FBXInfo*> m_info_list;
	FbxManager* m_lSdkManager;
	FbxScene* m_lScene;
	FbxArray<FbxString*> *m_AnimStackNameArray;
	AnimationInfo* m_lCurrentAnimationStack;
	FbxTime::EMode m_timeMode;
	std::vector<CMaterial*> m_materialList;
	int m_bone_num;
	CMatrix* m_boneMatrix;
public:
	int m_animJam;
	float m_animSpeed;
	bool m_animLoop;
	float m_animEndFrame;
	float m_animTime;
	float m_animStartFrame;
	float m_blend;
	float m_blendvec;
	int m_anim_size;
	SAnimData*	m_pAnimData;

	void updateBoneMatrix(FbxNode* pNode, const FbxTime& pTime);
	void drawNode(FbxNode* pNode,CMatrix mv,CMatrix m);
	void createNode(FbxNode* pNode, int lv, int &no, bool preCreate);
	void destroyNode(FbxNode* pNode, int lv);
	CMatrix getFrameMatrix(FbxNode *pFrame, bool local);
	void setFrameRevMatrix(FbxNode *pFrame, CMatrix &m);


public:
	CModelFBX();
	CModelFBX(CModelFBX &m);
	bool AddAnimation(const char* filePath);
	AnimationInfo* GetAnimationInfo(int jam) {
		return m_animation_list[jam];
	}
	/*!
		@brief	���f���ǂݍ���
		@param	filepath		[in] ���f���t�@�C����
		@retval	true:�����@false:���s
	**/
	bool Load(const char* filePath);
	/*!
		@brief	���f���̕\��
		@param	m				[in] ���f���s��
		@retval	����
	**/
	void Render(CMatrix& m);


	void Render();

	void UpdateMatrix();

	/*!
		@brief	�w��̍s��Ŋe�{�[�����X�V����
		@retval	����
	**/
	void UpdateMatrix(CMatrix& m);
	
	/*!
		@brief	���f���J��
		@retval	����
	**/
	void Release();
	/*!
		@brief	�A�j���[�V�����X�V
		@retval	����
	**/
	void UpdateAnimation();
	/*!
		@brief�@�A�j���[�V�����I������
		@retval	true:�I��
	**/
	bool isAnimationEnd();
	/*!
		@brief	�A�j���[�V�����̔ԍ����擾
		@retval	�Đ����ԍ��i0�`�j
	**/
	int GetAnimationJam();
	/*!
		@brief	�A�j���[�V�����̑��x�Đ����x�ύX
		@retval	����
	**/

	void SetAnimationSpeed(float s);
	/*!
		@brief	�A�j���[�V�����؂�ւ�
		@param	jam				[in] �A�j���[�V�����ԍ�
		@param	loop			[in] �A�j���[�V�������[�v�t���O
		@param	check			[in] ���ɍĐ������`�F�b�N
		@param	frame			[in] �؂�ւ���Ԏ���
		@retval	����
	**/

	void ChangeAnimation(int jam, bool loop = true, bool check = true, float time = 0,float blendtime = 0.1);
	/*!
		@brief	�A�j���[�V�����̌��݃t���[�����擾
		@retval	�t���[��
	**/

	float GetAnimationFrame();

	/*!
		@brief	�A�j���[�V�����̐����擾
		@retval	�A�j���[�V�����̐�
	**/
	int GetAnimationSize() const;
	
	/*!
		@brief	�A�j���[�V�����f�[�^��ݒ�
				��̃��[�V�����f�[�^�𕪊�����ꍇ�Ɏg�p
		@param	p				[in] �A�j���[�V�����f�[�^
		@param	loop			[in] �A�j���[�V�������[�v�t���O
		@param	check			[in] ���ɍĐ������`�F�b�N
		@param	frame			[in] �؂�ւ���Ԏ���
		@retval	����
	**/
	void AttachAnimData(SAnimData *p);
	/*!
		@brief	�{�[���̍s����擾
		@param	name			[in] �{�[����
		@param	local			[in] ���[�J���s��Ŏ擾�t���O
		@retval	����
	**/
	CMatrix GetFrameMatrix(const char *name, bool local = false);


	/*!
		@brief	�{�[���̍s����擾
		@param	name			[in] �t���[���ԍ�
		@param	local			[in] ���[�J���s��Ŏ擾�t���O
		@retval	����
	**/
	CMatrix GetFrameMatrix(int no, bool local = false);


	/*!
	@brief	�{�[���̕␳�s���ݒ�
	@param	name			[in] �{�[����
	@param	m				[in] �s��
	@retval	����
	**/
	void SetFrameRevMatrix(const char *name, CMatrix &m);


	/*!
	@brief	�{�[���̕␳�s���ݒ�
	@param	name			[in] �t���[���ԍ�
	@param	m				[in] �s��
	@retval	����
	**/
	void SetFrameRevMatrix(int no, CMatrix &m);
	/*!
		@brief	�}�e���A�����擾
		@param	no				[in]�}�e���A���ԍ�
		@param	mesh			[in] ���b�V���ԍ�
		@retval	����
	**/
	CMaterial *GetMaterial(int no)const;
	/*!
	@brief	�}�e���A����ݒ�
	@param	no				[n]�}�e���A���ԍ�
	@param	mesh			[in] ���b�V���ԍ�
	@retval	����
	**/
	virtual void SetMaterial(int no, CMaterial* m);

	/*!
	@brief	�����ƃ��f���̔���
	@param	c				[out] �ڐG�n�_
	@param	n				[out] �ڐG�ʂ̖@��
	@param	s				[in] �����̊J�n�n�_
	@param	e				[in] �����̏I���n�_
	@retval	true:�ڐG�@false:��ڐG
	**/
	//bool CollisionRay(CVector3D* c, CVector3D* n, const CVector3D& s, const CVector3D& e)const;

	/*!
	@brief	�����ƃ��f���̔���
	@param	s				[in] �����̊J�n�n�_
	@param	e				[in] �����̏I���n�_
	@retval	CCollTriangle	�Փ˂����ʃf�[�^
	**/
	//std::vector<CCollTriangle> CollisionRay(const CVector3D& s, const CVector3D& e)const;


	/*!
	@brief	���ƃ��f���̔���
	@param	center			[in] ���̒��S
	@param	radius			[in] ���̔��a
	@retval	CCollTriangle	�Փ˂����ʃf�[�^
	**/
	//std::vector<CCollTriangle> CollisionSphere(const CVector3D& center, float radius)const;



	/*!
	@brief	�J�v�Z���ƃ��f���̔���
	@param	top				[in] �J�v�Z���̓V��
	@param	bottom			[in] �J�v�Z���̒�
	@param	radius			[in] ���̔��a
	@retval	CCollTriangle	�Փ˂����ʃf�[�^
	**/
	//std::vector<CCollTriangle> CollisionCupsel(const CVector3D& top, const CVector3D& bottom, float radius)const;

	
	friend class CModelA3M;
	friend class CMeshFBX;

};

