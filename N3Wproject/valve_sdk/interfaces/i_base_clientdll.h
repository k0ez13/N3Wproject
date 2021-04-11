#pragma once

#include "../misc/global_vars.h"
#include "../misc/client_class.h"

enum class frame_stage {
	UNDEFINED = -1,
	START,
	NET_UPDATE_START,
	NET_UPDATE_POSTDATAUPDATE_START,
	NET_UPDATE_POSTDATAUPDATE_END,
	NET_UPDATE_END,
	RENDER_START,
	RENDER_END
};

// Used by RenderView
enum render_view_info_t
{
	RENDERVIEW_UNSPECIFIED = 0,
	RENDERVIEW_DRAWVIEWMODEL = (1 << 0),
	RENDERVIEW_DRAWHUD = (1 << 1),
	RENDERVIEW_SUPPRESSMONITORRENDERING = (1 << 2),
};

class i_base_clientdll
{
public:
	virtual int              connect(create_interface_fn appSystemFactory, c_global_varsbase*pGlobals) = 0;
	virtual int              disconnect(void) = 0;
	virtual int              init(create_interface_fn appSystemFactory, c_global_varsbase*pGlobals) = 0;
	virtual void             post_init() = 0;
	virtual void             shutdown(void) = 0;
	virtual void             level_init_pre_entity(char const* pMapName) = 0;
	virtual void             level_init_post_entity() = 0;
	virtual void             level_shutdown(void) = 0;
	virtual client_class*    get_all_classes(void) = 0;

	bool dispatch_user_message(int messageType, int arg, int arg1, void* data)
	{
		using DispatchUserMessage_t = bool* (__thiscall*)(void*, int, int, int, void*);
		return CallVFunction<DispatchUserMessage_t>(this, 38)(this, messageType, arg, arg1, data);
	}
};