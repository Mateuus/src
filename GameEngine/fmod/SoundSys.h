#ifndef __PWAR_SOUNDSYS_H
#define __PWAR_SOUNDSYS_H

#include "..\..\External\fmod\fmod_event.hpp"
#include "..\..\External\fmod\fmod_event_net.hpp"
#include "..\..\External\fmod\Fmod_errors.h"

#define MAX_SOUND_SAMPLES	2048

struct CSoundSample
{
	int				ID;
	float			minDist;
	float			maxDist;
	FMOD::Sound*	Sound;
	int				CreateFlags;
	char			Name[128];
};


#define SND_ERR_CHK(err) {if((err)!=FMOD_OK) r3dError("Sound error: '%s' in file '%s' at line %d", FMOD_ErrorString(err), __FILE__, __LINE__);}
typedef std::vector<std::string> stringlist_t;

//
//
//
class CSoundSystem
{
private:
	FMOD::EventSystem	*FMODEventSystem;
	FMOD::EventProject	*soundsProject;
	mutable stringlist_t soundsList;

	FMOD::Sound*		m_SoundBank00;
	FMOD::Sound*		m_SoundBank_Streaming;
	FMOD::Sound*		m_SoundBank_Weapons;

#if ENABLE_ZOMBIES
	FMOD::Sound*		m_WarZ_SoundBank;
#endif

	bool				m_isSoundDisabled;
	
public:
	CSoundSystem();
	~CSoundSystem();

	int		Valid() { return FMODEventSystem!=0; }

	int		Open();
	int		Close();

	bool	isSoundDisabled() const { return m_isSoundDisabled; }

	void	Update(const r3dPoint3D &Pos, const r3dPoint3D &dir, const r3dPoint3D& up);
	void	SetSurround(int Ch, int Val){  }
	int		LoadSoundEffects(const char *basedir, const char *fname);

	FMOD::EventReverb*	createPresetReverb(const char* presetName);

	void *		Play(int SampleID, const r3dPoint3D& Pos); // will choose how to play
	void		SetPaused(void* handle, bool paused);
	bool		SetSoundPos(void *handle, const r3dPoint3D& pos);
	void		Release(void *handle);
	float		GetSoundMaxDistance(void *handle) const;
	float		GetSoundMaxDistance(int id) const;
	void		SetProperty(void *handle, FMOD_EVENT_PROPERTY propID, void *value);
	int			GetState(void *handle) const;
	
	void		Stop(void *handle);
	void		Start(void *handle);
	void		KeyOff(void *handle, char* param_name); // stop sound, but make it play it trail part (used for automatic firing weapons)
	void		SetParamValue(void *handle, char* param_name, float value);
	int			GetEventIDByPath(const char *path) const;
	void		Set3DAttributes(void *handle, const r3dVector *pos, const r3dVector *velocity, const r3dVector *orientataion) const;
	bool		IsHandleValid(void *handle) const;

	/**	Check if 3d event inside audible range. */
	bool		IsAudible(void *handle, const r3dPoint3D &pos) const;
	bool		Is3DSound(void *handle) const;
	const stringlist_t & GetSoundsList() const;

private:
	void *		Play2D(int SampleID);
	void *		Play3D(int SampleID, const r3dPoint3D& Pos);
};


extern	CSoundSystem	SoundSys;

extern 	int 		snd_UpdateSoundListener(const r3dPoint3D &Pos, const r3dPoint3D &dir, const r3dPoint3D& up);
extern 	int 		snd_InitSoundSystem();
extern 	int 		snd_CloseSoundSystem();
extern  int 		snd_LoadSoundEffects(char *basedir, char *fname);

enum { SOUND_GROUP_START = 1000};
extern	void		snd_SetGroupVolume(int groupId, int Volume);

// all PlaySound* functions will accept sound group ID as volume.
extern	void *	snd_PlaySound(int SampleID, const r3dPoint3D& pos);
extern	bool	snd_SetSoundPos(void *e, const r3dPoint3D& pos);


#endif	// __PWAR_SOUNDSYS_H
