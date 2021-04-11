#pragma once

#include "../math/Vector.h"
#include "../misc/UtlVector.h"

struct snd_info_t;
class i_recipient_filter;

typedef void* file_name_handle_t;

struct snd_info_t {
	int m_nGuid;
	file_name_handle_t m_filenameHandle;
	int m_nSoundSource;
	int m_nChannel;
	int m_nSpeakerEntity;
	float m_flVolume;
	float m_flLastSpatializedVolume;
	float m_flRadius;
	int m_nPitch;
	Vector* m_pOrigin;
	Vector* m_pDirection;
	bool m_bUpdatePositions;
	bool m_bIsSentence;
	bool m_bDryMix;
	bool m_bSpeaker;
	bool m_bSpecialDSP;
	bool m_bFromServer;
};


enum soundlevel_t
{
    SNDLVL_NONE = 0,
    SNDLVL_20dB = 20,		// rustling leaves
    SNDLVL_25dB = 25,		// whispering
    SNDLVL_30dB = 30,		// library
    SNDLVL_35dB = 35,
    SNDLVL_40dB = 40,
    SNDLVL_45dB = 45,		// refrigerator
    SNDLVL_50dB = 50,	    // 3.9    // average home
    SNDLVL_55dB = 55,	    // 3.0
    SNDLVL_IDLE = 60,	    // 2.0	
    SNDLVL_60dB = 60,	    // 2.0	// normal conversation, clothes dryer
    SNDLVL_65dB = 65,	    // 1.5	// washing machine, dishwasher
    SNDLVL_STATIC = 66,	    // 1.25
    SNDLVL_70dB = 70,	    // 1.0	// car, vacuum cleaner, mixer, electric sewing machine
    SNDLVL_NORM = 75,
    SNDLVL_75dB = 75,	    // 0.8	// busy traffic
    SNDLVL_80dB = 80,	    // 0.7	// mini-bike, alarm clock, noisy restaurant, office tabulator, outboard motor, passing snowmobile
    SNDLVL_TALKING = 80,    // 0.7
    SNDLVL_85dB = 85,	    // 0.6	// average factory, electric shaver
    SNDLVL_90dB = 90,	    // 0.5	// screaming child, passing motorcycle, convertible ride on frw
    SNDLVL_95dB = 95,
    SNDLVL_100dB = 100,	    // 0.4	// subway train, diesel truck, woodworking shop, pneumatic drill, boiler shop, jackhammer
    SNDLVL_105dB = 105,     // helicopter, power mower
    SNDLVL_110dB = 110,     // snowmobile drvrs seat, inboard motorboat, sandblasting
    SNDLVL_120dB = 120,     // auto horn, propeller aircraft
    SNDLVL_130dB = 130,     // air raid siren
    SNDLVL_GUNFIRE = 140,	// 0.27	// THRESHOLD OF PAIN, gunshot, jet engine
    SNDLVL_140dB = 140,	    // 0.2
    SNDLVL_150dB = 150,	    // 0.2
    SNDLVL_180dB = 180,		// rocket launching

                            // NOTE: Valid soundlevel_t values are 0-255.
                            //       256-511 are reserved for sounds using goldsrc compatibility attenuation.
};

//-----------------------------------------------------------------------------
// common pitch values
//-----------------------------------------------------------------------------
#define	PITCH_NORM		100			  // non-pitch shifted
#define PITCH_LOW		95			    // other values are possible - 0-255, where 255 is very high
#define PITCH_HIGH		120

class i_enginesound
{
public:
	// Precache a particular sample
	virtual bool precache_sound(const char *pSample, bool bPreload = false, bool bIsUISound = false) = 0;
	virtual bool is_sound_precached(const char *pSample) = 0;
	virtual void prefetch_sound(const char *pSample) = 0;
	virtual bool is_looping_sound(const char *pSample) = 0;

	// Just loads the file header and checks for duration (not hooked up for .mp3's yet)
	// Is accessible to server and client though
	virtual float get_sound_duration(const char *pSample) = 0;

	// Pitch of 100 is no pitch shift.  Pitch > 100 up to 255 is a higher pitch, pitch < 100
	// down to 1 is a lower pitch.   150 to 70 is the realistic range.
	// EmitSound with pitch != 100 should be used sparingly, as it's not quite as
	// fast (the pitchshift mixer is not native coded).

	// NOTE: setting ient_index to -1 will cause the sound to be emitted from the local
	// player (client-side only)
	virtual int emit_sound(i_recipient_filter& filter, int ient_index, int iChannel, const char *pSoundEntry, unsigned int nSoundEntryHash, const char *pSample,
		float flVolume, float flAttenuation, int nSeed, int iFlags = 0, int iPitch = PITCH_NORM,
		const Vector *pOrigin = NULL, const Vector *pDirection = NULL, CUtlVector< Vector >* pUtlVecOrigins = NULL, bool bUpdatePositions = true, float soundtime = 0.0f, int speakerentity = -1) = 0;

	virtual int emit_sound(i_recipient_filter& filter, int ient_index, int iChannel, const char *pSoundEntry, unsigned int nSoundEntryHash, const char *pSample,
		float flVolume, soundlevel_t iSoundlevel, int nSeed, int iFlags = 0, int iPitch = PITCH_NORM,
		const Vector *pOrigin = NULL, const Vector *pDirection = NULL, CUtlVector< Vector >* pUtlVecOrigins = NULL, bool bUpdatePositions = true, float soundtime = 0.0f, int speakerentity = -1) = 0;

	virtual void emit_sentence_by_index(i_recipient_filter& filter, int ient_index, int iChannel, int iSentenceIndex,
		float flVolume, soundlevel_t iSoundlevel, int nSeed, int iFlags = 0, int iPitch = PITCH_NORM,
		const Vector *pOrigin = NULL, const Vector *pDirection = NULL, CUtlVector< Vector >* pUtlVecOrigins = NULL, bool bUpdatePositions = true, float soundtime = 0.0f, int speakerentity = -1) = 0;

	virtual void    stop_sound(int ient_index, int iChannel, const char *pSample, unsigned int nSoundEntryHash) = 0;
	virtual void    stop_all_sounds(bool bClearBuffers) = 0;
	virtual void    set_room_type(i_recipient_filter& filter, int roomType) = 0;
	virtual void    set_player_dsp(i_recipient_filter& filter, int dspType, bool fastReset) = 0;
	virtual int     emit_ambient_sound(const char *pSample, float flVolume, int iPitch = PITCH_NORM, int flags = 0, float soundtime = 0.0f) = 0;
	virtual float   get_dist_gain_from_sound_level(soundlevel_t soundlevel, float dist) = 0;
	virtual int		get_guid_for_last_sound_emitted() = 0;
	virtual bool	is_sound_still_playing(int guid) = 0;
	virtual void	stop_sound_by_guid(int guid, bool bForceSync) = 0;
	virtual void	set_volume_by_guid(int guid, float fvol) = 0;
	virtual void    unk() = 0;
	virtual void	get_active_sounds(CUtlVector<snd_info_t>& sndlist) = 0;
	virtual void	precache_sentence_group(const char *pGroupName) = 0;
	virtual void	notify_begin_movie_playback() = 0;
	virtual void	notify_end_movie_playback() = 0;
	virtual bool	get_sound_channel_volume(const char* sound, float &flVolumeLeft, float &flVolumeRight) = 0;
	virtual float	get_elapsed_time_by_guid(int guid) = 0;
};
