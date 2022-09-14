#include "CSound.h"
#include "CCamera.h"
#include <assert.h>
#include "AL/alut.h"

#pragma comment(lib, "OpenAL32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "alut.lib")
#pragma comment(lib, "common.lib")
CSoundBase::CSoundBase(): m_source(nullptr), m_buffer(0), m_current(0),m_layer_max(0), m_channels(0), m_sample_rate(0){

}
void CSoundBase::Release() {
	Stop();
	// ソースの破棄
	if (m_source) {
		alDeleteSources(m_layer_max, m_source);

		if (m_source) delete m_source;
		m_source = nullptr;
	}
	if (m_buffer) {
		alDeleteBuffers(1, &m_buffer);
		m_buffer = 0;
	}

}
CSoundBase::~CSoundBase() {
	Release();
}
bool CSoundBase::Load(const char* file, int layer, bool is3D)
{
	if (m_buffer) return true;
	m_buffer = alutCreateBufferFromFile(file);
	if (!m_buffer) {
		MessageBox(GL::hWnd, (std::string(file) + "がありません").c_str(), "", MB_OK);
		return false;
	}
	m_layer_max = layer;
	m_source = new ALuint[m_layer_max];
	alGenSources(m_layer_max, m_source);

	m_param.is3D = is3D;


	return true;
}
bool CSoundBase::Create(const void* data, ALsizei size, ALenum format, ALsizei freq)
{

	m_param.is3D = false;
	m_layer_max = 1;
	m_source = new ALuint[m_layer_max];
	alGenBuffers(1, &m_buffer); //曲データ１つにつきバッファ１つ．
	alGenSources(m_layer_max, m_source); //空間に配置する数の分生成する

	alBufferData(m_buffer, format, data, size, freq); // bufferにデータを登録

	// source と buffer の接続
	alSourcei(m_source[0], AL_BUFFER, m_buffer);

	return false;
}
int CSoundBase::Play(bool boLoop) {
	if (m_layer_max == 0) return -1;

	const ALuint &s = m_source[m_current];
	Stop(m_current);
	alSourcei(s, AL_LOOPING, boLoop ? AL_TRUE : AL_FALSE);
	alSourcei(s, AL_BUFFER, m_buffer);
	alSourcef(s, AL_PITCH, m_param.pitch);
	alSourcef(s, AL_GAIN, m_param.vol);
	float position[3] = { m_param.pan, 0.0f, 0.0f };
	alSourcei(s, AL_SOURCE_RELATIVE, AL_TRUE);
	alSourcefv(s, AL_POSITION, position);
	alSourcef(s, AL_MAX_DISTANCE, 1.0f);
	alSourcePlay(s);
	int c = m_current;
	m_current = (m_current + 1) % m_layer_max;
	return c;
}

void CSoundBase::Resume(bool boLoop, int layer) {
	if (m_layer_max == 0) return;

	alSourcePlay(m_source[layer]);

}
int CSoundBase::Play3D(const CVector3D& pos, const CVector3D& velocity,bool boLoop) {
	if (m_layer_max == 0) return -1;

	const ALuint &s = m_source[m_current];
	Stop(m_current);
	alSourcei(s, AL_LOOPING, boLoop ? AL_TRUE : AL_FALSE);
	alSourcei(s, AL_BUFFER, m_buffer);
	alSourcef(s, AL_PITCH, m_param.pitch);
	alSourcef(s, AL_GAIN, m_param.vol);
	alSourcef(s, AL_ROLLOFF_FACTOR, m_param.rolloff_factor);
	alSourcef(s, AL_REFERENCE_DISTANCE, m_param.reference_distance);
	alSourcei(s, AL_SOURCE_RELATIVE, AL_FALSE);
	alSourcefv(s, AL_POSITION, pos.v);
	alSourcefv(s, AL_VELOCITY, velocity.v);
	alSourcef(s, AL_MAX_DISTANCE, m_param.dist);

	alSourcePlay(s);
	int c = m_current;
	m_current = (m_current + 1) % m_layer_max;
	return c;
}

void CSoundBase::SetPos(CVector3D pos, int layer) {
	if (m_layer_max == 0) return;
	alSourcefv(m_source[layer], AL_POSITION, pos.v);
}
void CSoundBase::Stop(int layer){
	if (m_layer_max == 0) return;
	if (layer >= 0) {
		alSourceStop(m_source[layer]);
	} else {
		for (int i=0;i<m_layer_max;i++)
			alSourceStop(m_source[i]);
	}
}
void CSoundBase::Pause(int layer){
	if (m_layer_max == 0) return;
	if (layer >= 0) {
		alSourcePause(m_source[layer]);
	} else {
		for (int i = 0; i<m_layer_max; i++)
			alSourcePause(m_source[i]);
	}

}
void CSoundBase::Pitch(float pitch, int layer) {
	if (m_layer_max == 0) return;
	if (layer == -1) {
		m_param.pitch = pitch;
	}
	else {
		alSourcef(m_source[layer], AL_PITCH, pitch);
	}
}

void CSoundBase::Volume(float volumes, int layer){
	if (m_layer_max == 0) return;
	if (layer == -1) {
		m_param.vol = volumes;
	} else {
		alSourcef(m_source[layer], AL_GAIN, volumes);
	}
}

void CSoundBase::Pan(float pan, int layer) {
	if (m_layer_max == 0) return;
	if (layer == -1) {
		m_param.pan = pan;
	} else {
		float position[3] = { pan, 0.0f, 0.0f };
		alSourcei(m_source[layer], AL_SOURCE_RELATIVE, AL_TRUE);
		alSourcefv(m_source[layer], AL_POSITION, position);
	}


}

void CSoundBase::SetDistance(float max_distance,float reference_distance,float rolloff_factor, int layer){
	if (m_layer_max == 0) return;
	if (layer == -1) {
		m_param.rolloff_factor = rolloff_factor;
		m_param.reference_distance = reference_distance;
		m_param.dist = max_distance;
	}
	else {
		alSourcef(m_source[layer], AL_ROLLOFF_FACTOR, rolloff_factor);
		alSourcef(m_source[layer], AL_MAX_DISTANCE, max_distance);
		alSourcef(m_source[layer], AL_REFERENCE_DISTANCE, reference_distance);
	}
}

bool CSoundBase::CheckEnd(int layer)
{
	if (m_layer_max == 0) return true;
	ALint state = 0;
	alGetSourcei(m_source[layer], AL_SOURCE_STATE, &state);
	return state == AL_STOPPED || state == AL_INITIAL;


}

float CSoundBase::GetOffset(int layer)
{
	float offset;

	alGetSourcef(m_source[layer], AL_SEC_OFFSET, &offset);

	return offset;
}


CSound*	CSound::m_instance=nullptr;

CSound::CSound(){
	
	// COMの初期化
	CoInitialize(NULL);
	alutInit(NULL, NULL);


}
CSound::~CSound() {
	auto it = m_list.begin();
	while (it != m_list.end()) {
		if (it->second) delete it->second;
		it++;
	}
	

	alutExit();
	// COMの終了
	CoUninitialize();
}

CSound* CSound::GetInstance()
{
	if (!m_instance) m_instance = new CSound();
	return m_instance;
}

void CSound::ClearInstance()
{
	if (!m_instance) return;
	delete m_instance;
	m_instance = nullptr;
}

CSoundBase* CSound::GetSound(const char* name)
{
	if (!m_list[name]) m_list[name] = new CSoundBase();
	return m_list[name];
}

void CSound::ReleaseSound(const char * name)
{
	auto it = m_list.find(name);
	if (it == m_list.end()) return;
	if (it->second) delete it->second;
	m_list.erase(it);
}

void CSound::UpdateListener() {
	CVector3D& p = CCamera::GetCurrent()->GetPos();
	CVector3D& f = CCamera::GetCurrent()->GetDir();
	CVector3D& u = CCamera::GetCurrent()->GetUp();
	float orientation[6];
	orientation[0] = f.x;
	orientation[1] = f.y;
	orientation[2] = f.z;
	orientation[3] = u.x;
	orientation[4] = u.y;
	orientation[5] = u.z;

	alListenerfv(AL_POSITION, p.v);
	alListenerfv(AL_ORIENTATION, orientation);
	alListenerfv(AL_VELOCITY, CVector3D(0,0,0).v);
	
}

void CSound::StopAll()
{
	for (auto it = m_list.begin(); it != m_list.end(); it++) {
		it->second->Stop();
	}
}
