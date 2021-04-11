#pragma once
class c_beam;
class beam_t;
class c_base_entity;
class i_trace_filter;

//---------------------------------------------------------------------------�
// Purpose: Popcorn trail for Beam Follow rendering...
//---------------------------------------------------------------------------�
enum
{
	FBEAM_STARTENTITY = 0x00000001,
	FBEAM_ENDENTITY = 0x00000002,
	FBEAM_FADEIN = 0x00000004,
	FBEAM_FADEOUT = 0x00000008,
	FBEAM_SINENOISE = 0x00000010,
	FBEAM_SOLID = 0x00000020,
	FBEAM_SHADEIN = 0x00000040,
	FBEAM_SHADEOUT = 0x00000080,
	FBEAM_ONLYNOISEONCE = 0x00000100, // Only calculate our noise once
	FBEAM_NOTILE = 0x00000200,
	FBEAM_USE_HITBOXES = 0x00000400, // Attachment indices represent hitbox indices instead when this is set.
	FBEAM_STARTVISIBLE = 0x00000800, // Has this client actually seen this beam's start entity yet?
	FBEAM_ENDVISIBLE = 0x00001000, // Has this client actually seen this beam's end entity yet?
	FBEAM_ISACTIVE = 0x00002000,
	FBEAM_FOREVER = 0x00004000,
	FBEAM_HALOBEAM = 0x00008000, // When drawing a beam with a halo, don't ignore the segments and endwidth
	FBEAM_REVERSED = 0x00010000,
	NUM_BEAM_FLAGS = 17 // KEEP THIS UPDATED!
};
enum
{
	TE_BEAMPOINTS = 0x00, // beam effect between two points
	TE_SPRITE = 0x01, // additive sprite, plays 1 cycle
	TE_BEAMDISK = 0x02, // disk that expands to max radius over lifetime
	TE_BEAMCYLINDER = 0x03, // cylinder that expands to max radius over lifetime
	TE_BEAMFOLLOW = 0x04, // create a line of decaying beam segments until entity stops moving
	TE_BEAMRING = 0x05, // connect a beam ring to two entities
	TE_BEAMSPLINE = 0x06,
	TE_BEAMRINGPOINT = 0x07,
	TE_BEAMLASER = 0x08, // Fades according to viewpoint
	TE_BEAMTESLA = 0x09,
};
struct beam_trail_t
{
	// NOTE: Don't add user defined fields except after these four fields.
	beam_trail_t* next;
	float die;
	Vector org;
	Vector vel;
};

//---------------------------------------------------------------------------�
// Data type for beams.
//---------------------------------------------------------------------------�
struct beam_info_t
{
	int m_nType;

	// Entities
	c_base_entity* m_pStartEnt;
	int m_nStartAttachment;
	c_base_entity* m_pEndEnt;
	int m_nEndAttachment;

	// Points
	Vector m_vecStart;
	Vector m_vecEnd;

	int m_nModelIndex;
	const char* m_pszModelName;

	int m_nHaloIndex;
	const char* m_pszHaloName;
	float m_flHaloScale;

	float m_flLife;
	float m_flWidth;
	float m_flEndWidth;
	float m_flFadeLength;
	float m_flAmplitude;

	float m_flBrightness;
	float m_flSpeed;

	int m_nStartFrame;
	float m_flFrameRate;

	float m_flRed;
	float m_flGreen;
	float m_flBlue;

	bool m_bRenderable;

	int m_nSegments;

	int m_nFlags;

	// Rings
	Vector m_vecCenter;
	float m_flStartRadius;
	float m_flEndRadius;

	beam_info_t()
	{
		m_nType = TE_BEAMPOINTS;
		m_nSegments = -1;
		m_pszModelName = NULL;
		m_pszHaloName = NULL;
		m_nModelIndex = -1;
		m_nHaloIndex = -1;
		m_bRenderable = true;
		m_nFlags = 0;
	}
};

//---------------------------------------------------------------------------�
// Purpose: Declare client .dll beam entity interface
//---------------------------------------------------------------------------�
class i_vieweenderbeams
{
public:
	virtual void init_beams(void) = 0;
	virtual void shutdown_beams(void) = 0;
	virtual void clear_beams(void) = 0;

	// Updates the state of the temp ent beams
	virtual void update_temp_ent_beams() = 0;

	virtual void draw_beam(c_beam* pbeam, i_trace_filter* pEntityBeamTraceFilter = NULL) = 0;
	virtual void draw_beam(beam_t* pbeam) = 0;

	virtual void kill_dead_beams(c_base_entity* pEnt) = 0;

	// New interfaces!
	virtual beam_t* create_beam_ents(beam_info_t& beamInfo) = 0;
	virtual beam_t* create_beam_ent_point(beam_info_t& beamInfo) = 0;
	virtual beam_t* create_beam_points(beam_info_t& beamInfo) = 0;
	virtual beam_t* create_beam_ring(beam_info_t& beamInfo) = 0;
	virtual beam_t* create_beam_ring_point(beam_info_t& beamInfo) = 0;
	virtual beam_t* create_beam_circle_points(beam_info_t& beamInfo) = 0;

	virtual beam_t* create_beam_follow(beam_info_t& beamInfo) = 0;

	virtual void dree_beam(beam_t* pBeam) = 0;
	virtual void update_beam_info(beam_t* pBeam, beam_info_t& beamInfo) = 0;

	// These will go away!
	virtual void create_beam_ents(int startEnt, int endEnt, int modelIndex, int haloIndex, float haloScale,
		float life, float width, float m_nEndWidth, float m_nFadeLength, float amplitude,
		float brightness, float speed, int startFrame,
		float framerate, float r, float g, float b, int type = -1) = 0;
	virtual void create_beam_ent_point(int nStartEntity, const Vector* pStart, int nEndEntity, const Vector* pEnd,
		int modelIndex, int haloIndex, float haloScale,
		float life, float width, float m_nEndWidth, float m_nFadeLength, float amplitude,
		float brightness, float speed, int startFrame,
		float framerate, float r, float g, float b) = 0;
	virtual void create_beam_points(Vector& start, Vector& end, int modelIndex, int haloIndex, float haloScale,
		float life, float width, float m_nEndWidth, float m_nFadeLength, float amplitude,
		float brightness, float speed, int startFrame,
		float framerate, float r, float g, float b) = 0;
	virtual void create_beam_ring(int startEnt, int endEnt, int modelIndex, int haloIndex, float haloScale,
		float life, float width, float m_nEndWidth, float m_nFadeLength, float amplitude,
		float brightness, float speed, int startFrame,
		float framerate, float r, float g, float b, int flags = 0) = 0;
	virtual void create_beam_ring_point(const Vector& center, float start_radius, float end_radius, int modelIndex, int haloIndex, float haloScale,
		float life, float width, float m_nEndWidth, float m_nFadeLength, float amplitude,
		float brightness, float speed, int startFrame,
		float framerate, float r, float g, float b, int flags = 0) = 0;
	virtual void create_beam_circle_points(int type, Vector& start, Vector& end,
		int modelIndex, int haloIndex, float haloScale, float life, float width,
		float m_nEndWidth, float m_nFadeLength, float amplitude, float brightness, float speed,
		int startFrame, float framerate, float r, float g, float b) = 0;
	virtual void create_beam_follow(int startEnt, int modelIndex, int haloIndex, float haloScale,
		float life, float width, float m_nEndWidth, float m_nFadeLength, float r, float g, float b,
		float brightness) = 0;
};