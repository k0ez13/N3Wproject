#pragma once

#include <cstdint>
#include "../interfaces/i_cvar.h"
#include "UtlVector.h"
#include "UtlString.h"

#define FORCEINLINE_CVAR inline
//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------
class convar;
class c_command;
class con_command;
class con_command_base;
struct characterset_t;

class c_command
{
public:
    c_command();
    c_command(int nArgC, const char **ppArgV);
    bool tokenize(const char *pCommand, characterset_t *pBreakSet = NULL);
    void reset();

    int arg_c() const;
    const char** arg_v() const;
    const char*  arg_s() const;					        // All args that occur after the 0th arg, in string form
    const char*  get_command_string() const;		  // The entire command in string form, including the 0th arg
    const char*  operator[](int nIndex) const;	// Gets at arguments
    const char*  arg(int nIndex) const;		      // Gets at arguments

                                                  // Helper functions to parse arguments to commands.
    const char* find_arg(const char *pName) const;
    int find_arg_int(const char *pName, int nDefaultVal) const;

    static int max_command_length();
    static characterset_t* default_break_set();

private:
    enum
    {
        COMMAND_MAX_ARGC = 64,
        COMMAND_MAX_LENGTH = 512,
    };

    int		m_nArgc;
    int		m_nArgv0Size;
    char	m_pArgSBuffer[COMMAND_MAX_LENGTH];
    char	m_pArgvBuffer[COMMAND_MAX_LENGTH];
    const char*	m_ppArgv[COMMAND_MAX_ARGC];
};

int c_command::max_command_length()
{
    return COMMAND_MAX_LENGTH - 1;
}

int c_command::arg_c() const
{
    return m_nArgc;
}

const char ** c_command::arg_v() const
{
    return m_nArgc ? (const char**)m_ppArgv : NULL;
}

const char * c_command::arg_s() const
{
    return m_nArgv0Size ? &m_pArgSBuffer[m_nArgv0Size] : "";
}

const char * c_command::get_command_string() const
{
    return m_nArgc ? m_pArgSBuffer : "";
}

const char * c_command::arg(int nIndex) const
{
    // FIXME: Many command handlers appear to not be particularly careful
    // about checking for valid argc range. For now, we're going to
    // do the extra check and return an empty string if it's out of range
    if(nIndex < 0 || nIndex >= m_nArgc)
        return "";
    return m_ppArgv[nIndex];
}

const char * c_command::operator[](int nIndex) const
{
    return arg(nIndex);
}

//-----------------------------------------------------------------------------
// Any executable that wants to use convars need to implement one of
// these to hook up access to console variables.
//-----------------------------------------------------------------------------
class i_con_command_base_accessor
{
public:
    // Flags is a combination of FCVAR flags in cvar.h.
    // hOut is filled in with a handle to the variable.
    virtual bool register_con_command_base(con_command_base *pVar) = 0;
};

//-----------------------------------------------------------------------------
// Called when a ConCommand needs to execute
//-----------------------------------------------------------------------------
typedef void(*FnCommandCallbackV1_t)(void);
typedef void(*FnCommandCallback_t)(const c_command &command);

#define COMMAND_COMPLETION_MAXITEMS       64
#define COMMAND_COMPLETION_ITEM_LENGTH    64

//-----------------------------------------------------------------------------
// Returns 0 to COMMAND_COMPLETION_MAXITEMS worth of completion strings
//-----------------------------------------------------------------------------
typedef int(*FnCommandCompletionCallback)(const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH]);


//-----------------------------------------------------------------------------
// Interface version
//-----------------------------------------------------------------------------
class i_command_callback
{
public:
    virtual void command_callback(const c_command &command) = 0;
};

class i_command_completion_callback
{
public:
    virtual int  command_completion_callback(const char *pPartial, CUtlVector<CUtlString> &commands) = 0;
};

//-----------------------------------------------------------------------------
// Purpose: The base console invoked command/cvar interface
//-----------------------------------------------------------------------------
class con_command_base
{
    friend class c_cvar;
    friend class c_convar;
    friend class con_command;
    friend void convar_register(int nCVarFlag, i_con_command_base_accessor *pAccessor);

    // FIXME: Remove when convar changes are done
    friend class CDefaultCvar;

public:
    con_command_base(void);
    con_command_base(const char *pName, const char *pHelpString = 0, int flags = 0);

    virtual                     ~con_command_base(void);
    virtual bool                is_command(void) const;
    virtual bool                is_flag_set(int flag) const;
    virtual void                add_flags(int flags);
    virtual void                remove_flags(int flags);
    virtual int                 get_flags() const;
    virtual const char*         get_name(void) const;
    virtual const char*         get_help_text(void) const;
    const con_command_base*       get_next(void) const;
    con_command_base*             get_next(void);
    virtual bool                is_registered(void) const;
    virtual c_var_dll_identifier_t get_dll_identifier() const;

    //protected:
    virtual void                create(const char *pName, const char *pHelpString = 0, int flags = 0);
    virtual void                init();
    void                        shutdown();
    char*                       copy_string(const char *from);

    //private:
    // Next convar in chain
    // Prior to register, it points to the next convar in the DLL.
    // Once registered, though, m_pNext is reset to point to the next
    // convar in the global list
    con_command_base*             m_pNext;
    bool                        m_bRegistered;
    const char*                 m_pszName;
    const char*                 m_pszHelpString;
    int                         m_nFlags;

protected:
    // convars add themselves to this list for the executable. 
    // Then convar_Register runs through  all the console variables 
    // and registers them into a global list stored in vstdlib.dll
    static con_command_base* s_pConCommandBases;

    // convars in this executable use this 'global' to access values.
    static i_con_command_base_accessor* s_pAccessor;

public:
    // This list will hold all the registered commands.
    // It is not from the official SDK. I've added this so that
    // we can parse all convars we have created if we want to
    // save them to a file later on, for example.
    static con_command_base* s_pRegisteredCommands;
};

//-----------------------------------------------------------------------------
// Purpose: The console invoked command
//-----------------------------------------------------------------------------
class con_command : public con_command_base
{
    friend class c_cvar;

public:
    typedef con_command_base base_class;

    con_command(const char *pName, FnCommandCallbackV1_t callback,
        const char *pHelpString = 0, int flags = 0, FnCommandCompletionCallback completionFunc = 0);
    con_command(const char *pName, FnCommandCallback_t callback,
        const char *pHelpString = 0, int flags = 0, FnCommandCompletionCallback completionFunc = 0);
    con_command(const char *pName, i_command_callback *pCallback,
        const char *pHelpString = 0, int flags = 0, i_command_completion_callback *pCommandCompletionCallback = 0);

    virtual         ~con_command(void);
    virtual bool    is_command(void) const;
    virtual int     auto_complete_suggest(const char *partial, CUtlVector<CUtlString> &commands);
    virtual bool    can_auto_complete(void);
    virtual void    dispatch(const c_command &command);

    //private:
    // NOTE: To maintain backward compat, we have to be very careful:
    // All public virtual methods must appear in the same order always
    // since engine code will be calling into this code, which *does not match*
    // in the mod code; it's using slightly different, but compatible versions
    // of this class. Also: Be very careful about adding new fields to this class.
    // Those fields will not exist in the version of this class that is instanced
    // in mod code.

    // Call this function when executing the command
    union
    {
        FnCommandCallbackV1_t       m_fnCommandCallbackV1;
        FnCommandCallback_t         m_fnCommandCallback;
        i_command_callback*           m_pCommandCallback;
    };

    union
    {
        FnCommandCompletionCallback m_fnCompletionCallback;
        i_command_completion_callback* m_pCommandCompletionCallback;
    };

    bool m_bHasCompletionCallback : 1;
    bool m_bUsingNewCommandCallback : 1;
    bool m_bUsingCommandCallbackInterface : 1;
};


//-----------------------------------------------------------------------------
// Purpose: A console variable
//-----------------------------------------------------------------------------
class convar : public con_command_base, public i_convar
{
    friend class CCvar;
    friend class convarRef;
    friend class SplitScreenconvarRef;

public:
    typedef con_command_base base_class;

    convar(const char *pName, const char *pDefaultValue, int flags = 0);

    convar(const char *pName, const char *pDefaultValue, int flags, const char *pHelpString);
    convar(const char *pName, const char *pDefaultValue, int flags, const char *pHelpString, bool bMin, float fMin, bool bMax, float fMax);
    convar(const char *pName, const char *pDefaultValue, int flags, const char *pHelpString, FnChangeCallback_t callback);
    convar(const char *pName, const char *pDefaultValue, int flags, const char *pHelpString, bool bMin, float fMin, bool bMax, float fMax, FnChangeCallback_t callback);

    virtual                     ~convar(void);
    virtual bool                is_flag_set(int flag) const;
    virtual const char*         get_help_text(void) const;
    virtual bool                is_registered(void) const;
    virtual const char*         get_name(void) const;
    virtual const char*         get_base_name(void) const;
    virtual int                 get_split_screen_player_slot() const;

    virtual void                add_flags(int flags);
    virtual int                 get_flags() const;
    virtual bool                is_command(void) const;

    // Install a change callback (there shouldn't already be one....)
    void install_change_callback(FnChangeCallback_t callback, bool bInvoke = true);
    void remove_change_callback(FnChangeCallback_t callbackToRemove);

    int get_change_callback_count() const { return m_pParent->m_fnChangeCallbacks.Count(); }
    FnChangeCallback_t get_change_callback(int slot) const { return m_pParent->m_fnChangeCallbacks[slot]; }

    // Retrieve value
    virtual float                   get_float(void) const;
    virtual int                     get_int(void) const;
    FORCEINLINE_CVAR Color          get_color(void) const;
    FORCEINLINE_CVAR bool           get_bool() const { return !!get_int(); }
    FORCEINLINE_CVAR char const*    get_string(void) const;

    // Compiler driven selection for template use
    template <typename T> T get(void) const;
    template <typename T> T get(T *) const;

    // Any function that allocates/frees memory needs to be virtual or else you'll have crashes
    //  from alloc/free across dll/exe boundaries.

    // These just call into the IConCommandBaseAccessor to check flags and Set the var (which ends up calling InternalSetValue).
    virtual void                    set_value(const char *value);
    virtual void                    set_value(float value);
    virtual void                    set_value(int value);
    virtual void                    set_value(Color value);

    // Reset to default value
    void                            revert(void);
    bool                            has_min() const;
    bool                            has_max() const;
    bool                            get_min(float& minVal) const;
    bool                            get_max(float& maxVal) const;
    float                           get_min_value() const;
    float                           get_max_value() const;
    const char*                     get_default(void) const;

    struct c_vvalue_t
    {
        char*   m_pszString;
        int     m_StringLength;
        float   m_fValue;
        int     m_nValue;
    };

    FORCEINLINE_CVAR c_vvalue_t &get_raw_value()
    {
        return m_Value;
    }
    FORCEINLINE_CVAR const c_vvalue_t &get_raw_value() const
    {
        return m_Value;
    }

    //private:
    bool                        internal_set_color_from_string(const char *value);
    virtual void                internal_set_value(const char *value);
    virtual void                internal_set_float_value(float fNewValue);
    virtual void                internal_set_int_value(int nValue);
    virtual void                internal_set_color_value(Color value);
    virtual bool                clamp_value(float& value);
    virtual void                change_string_value(const char *tempVal, float flOldValue);
    virtual void                create(const char *pName, const char *pDefaultValue, int flags = 0, const char *pHelpString = 0, bool bMin = false, float fMin = 0.0, bool bMax = false, float fMax = false, FnChangeCallback_t callback = 0);

    // Used internally by OneTimeInit to Initialize.
    virtual void                Init();

    //protected:
    convar*                     m_pParent;
    const char*                 m_pszDefaultValue;
    c_vvalue_t                   m_Value;
    bool                        m_bHasMin;
    float                       m_fMinVal;
    bool                        m_bHasMax;
    float                       m_fMaxVal;

    // Call this function when convar changes
    CUtlVector<FnChangeCallback_t> m_fnChangeCallbacks;
};


//-----------------------------------------------------------------------------
// Purpose: Return convar value as a float
// Output : float
//-----------------------------------------------------------------------------
FORCEINLINE_CVAR float convar::get_float(void) const
{
    uint32_t xored = *(uint32_t*)&m_pParent->m_Value.m_fValue ^ (uint32_t)this;
    return *(float*)&xored;
}

//-----------------------------------------------------------------------------
// Purpose: Return convar value as an int
// Output : int
//-----------------------------------------------------------------------------
FORCEINLINE_CVAR int convar::get_int(void) const
{
    return (int)(m_pParent->m_Value.m_nValue ^ (int)this);
}

//-----------------------------------------------------------------------------
// Purpose: Return convar value as a color
// Output : Color
//-----------------------------------------------------------------------------
FORCEINLINE_CVAR Color convar::get_color(void) const
{
    int value = get_int();
    unsigned char *pColorElement = ((unsigned char *)&value);
    return Color(pColorElement[0], pColorElement[1], pColorElement[2], pColorElement[3]);
}


//-----------------------------------------------------------------------------

template <> FORCEINLINE_CVAR float          convar::get<float>(void) const { return get_float(); }
template <> FORCEINLINE_CVAR int            convar::get<int>(void) const { return get_int(); }
template <> FORCEINLINE_CVAR bool           convar::get<bool>(void) const { return get_bool(); }
template <> FORCEINLINE_CVAR const char*    convar::get<const char *>(void) const { return get_string(); }
template <> FORCEINLINE_CVAR float          convar::get<float>(float *p) const { return (*p = get_float()); }
template <> FORCEINLINE_CVAR int            convar::get<int>(int *p) const { return (*p = get_int()); }
template <> FORCEINLINE_CVAR bool           convar::get<bool>(bool *p) const { return (*p = get_bool()); }
template <> FORCEINLINE_CVAR const char*    convar::get<const char *>(char const **p) const { return (*p = get_string()); }

//-----------------------------------------------------------------------------
// Purpose: Return convar value as a string, return "" for bogus string pointer, etc.
// Output : const char *
//-----------------------------------------------------------------------------
FORCEINLINE_CVAR const char * convar::get_string(void) const
{
    if(m_nFlags & FCVAR_NEVER_AS_STRING)
        return "FCVAR_NEVER_AS_STRING";
    char const *str = m_pParent->m_Value.m_pszString;
    return str ? str : "";
}

//-----------------------------------------------------------------------------
// Called by the framework to register ConCommands with the ICVar
//-----------------------------------------------------------------------------
void convar_register(int nCVarFlag = 0, i_con_command_base_accessor *pAccessor = NULL);
void convar_unregister();