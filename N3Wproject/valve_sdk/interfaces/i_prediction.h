#pragma once

#include "../math/QAngle.h"
#include "../misc/c_usercmd.h"
#include "i_move_helper.h"

class c_move_data
{
public:
    bool    m_bFirstRunOfFunctions : 1;
    bool    m_bGameCodeMovedPlayer : 1;
    int     m_nPlayerHandle;        // edict index on server, client entity handle on client=
    int     m_nImpulseCommand;      // Impulse command issued.
    Vector  m_vecViewAngles;        // Command view angles (local space)
    Vector  m_vecAbsViewAngles;     // Command view angles (world space)
    int     m_nButtons;             // Attack buttons.
    int     m_nOldButtons;          // From host_client->oldbuttons;
    float   m_flForwardMove;
    float   m_flSideMove;
    float   m_flUpMove;
    float   m_flMaxSpeed;
    float   m_flClientMaxSpeed;
    Vector  m_vecVelocity;          // edict::velocity        // Current movement direction.
    Vector  m_vecAngles;            // edict::angles
    Vector  m_vecOldAngles;
    float   m_outStepHeight;        // how much you climbed this move
    Vector  m_outWishVel;           // This is where you tried 
    Vector  m_outJumpVel;           // This is your jump velocity
    Vector  m_vecConstraintCenter;
    float   m_flConstraintRadius;
    float   m_flConstraintWidth;
    float   m_flConstraintSpeedFactor;
    float   m_flUnknown[5];
    Vector  m_vecAbsOrigin;
};

class c_base_player;

class i_game_movement
{
public:
    virtual			~i_game_movement(void) {}

    virtual void	          process_movement(c_base_player *pPlayer, c_move_data*pMove) = 0;
    virtual void	          reset(void) = 0;
    virtual void	          start_track_prediction_errors(c_base_player *pPlayer) = 0;
    virtual void	          finish_track_prediction_errors(c_base_player *pPlayer) = 0;
    virtual void	          fiff_print(char const *fmt, ...) = 0;
    virtual Vector const&	  get_player_mins(bool ducked) const = 0;
    virtual Vector const&	  get_player_maxs(bool ducked) const = 0;
    virtual Vector const&     get_player_view_offset(bool ducked) const = 0;
    virtual bool		      is_moving_player_stuck(void) const = 0;
    virtual c_base_player*     get_moving_player(void) const = 0;
    virtual void		      unblock_pusher(c_base_player *pPlayer, c_base_player *pPusher) = 0;
    virtual void              setup_movement_bounds(c_move_data *pMove) = 0;
};

class c_gamemovement
    : public i_game_movement
{
public:
    virtual ~c_gamemovement(void) {}
};

class i_prediction
{
public:
    bool in_prediction()
    {
        typedef bool(__thiscall* oInPrediction)(void*);
        return CallVFunction<oInPrediction>(this, 14)(this);
    }

    void run_command(c_base_player *player, c_usercmd *ucmd, i_move_helper* moveHelper)
    {
        typedef void(__thiscall* oRunCommand)(void*, c_base_player*, c_usercmd*, i_move_helper*);
        return CallVFunction<oRunCommand>(this, 19)(this, player, ucmd, moveHelper);
    }

    void setup_move(c_base_player *player, c_usercmd*ucmd, i_move_helper* moveHelper, void* pMoveData)
    {
        typedef void(__thiscall* oSetupMove)(void*, c_base_player*, c_usercmd*, i_move_helper*, void*);
        return CallVFunction<oSetupMove>(this, 20)(this, player, ucmd, moveHelper, pMoveData);
    }

    void finish_move(c_base_player *player, c_usercmd*ucmd, void*pMoveData)
    {
        typedef void(__thiscall* oFinishMove)(void*, c_base_player*, c_usercmd*, void*);
        return CallVFunction<oFinishMove>(this, 21)(this, player, ucmd, pMoveData);
    }
};