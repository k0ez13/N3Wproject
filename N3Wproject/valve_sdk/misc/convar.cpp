#include "convar.h"

#include "../sdk.h"

#include "characterset.h"
#include "UtlBuffer.h"

#define ALIGN_VALUE( val, alignment ) ( ( val + alignment - 1 ) & ~( alignment - 1 ) ) 
#define stackalloc( _size )		_alloca( ALIGN_VALUE( _size, 16 ) )

con_command_base * con_command_base::s_pConCommandBases = NULL;
con_command_base * con_command_base::s_pRegisteredCommands = NULL;
i_con_command_base_accessor	* con_command_base::s_pAccessor = NULL;
static int s_nDLLIdentifier = -1;
static int s_nCVarFlag = 0;
static bool s_bRegistered = false;

class c_defaultaccessor : public i_con_command_base_accessor
{
public:
    virtual bool register_con_command_base(con_command_base*pVar)
    {
        // Link to engine's list instead
        g_cvar->register_con_command(pVar);
        return true;
    }
};

static c_defaultaccessor s_DefaultAccessor;

//-----------------------------------------------------------------------------
// Called by the framework to register ConCommandBases with the ICVar
//-----------------------------------------------------------------------------
void convar_register(int nCVarFlag, i_con_command_base_accessor *pAccessor)
{
    if(!g_cvar || s_bRegistered)
        return;

    assert(s_nDLLIdentifier < 0);
    s_bRegistered = true;
    s_nCVarFlag = nCVarFlag;
    s_nDLLIdentifier = g_cvar->allocate_dll_identifier();

    con_command_base*pCur, *pNext;

    con_command_base::s_pAccessor = pAccessor ? pAccessor : &s_DefaultAccessor;
    pCur = con_command_base::s_pConCommandBases;

    while(pCur) {
        pNext = pCur->m_pNext;
        pCur->add_flags(s_nCVarFlag);
        pCur->init();

        con_command_base::s_pRegisteredCommands = pCur;

        pCur = pNext;
    }

    con_command_base::s_pConCommandBases = NULL;
}

void convar_unregister()
{
    if(!g_cvar || !s_bRegistered)
        return;

    assert(s_nDLLIdentifier >= 0);
    g_cvar->unregister_con_commands(s_nDLLIdentifier);
    s_nDLLIdentifier = -1;
    s_bRegistered = false;
}

con_command_base::con_command_base(void)
{
    m_bRegistered = false;
    m_pszName = NULL;
    m_pszHelpString = NULL;

    m_nFlags = 0;
    m_pNext = NULL;
}

con_command_base::con_command_base(const char *pName, const char *pHelpString /*=0*/, int flags /*= 0*/)
{
    create(pName, pHelpString, flags);
}

con_command_base::~con_command_base(void)
{
}

bool con_command_base::is_command(void) const
{
    //	assert( 0 ); This can't assert. . causes a recursive assert in Sys_Printf, etc.
    return true;
}

c_var_dll_identifier_t con_command_base::get_dll_identifier() const
{
    return s_nDLLIdentifier;
}

void con_command_base::create(const char *pName, const char *pHelpString /*= 0*/, int flags /*= 0*/)
{
    static const char *empty_string = "";

    m_bRegistered = false;

    // Name should be static data
    m_pszName = pName;
    m_pszHelpString = pHelpString ? pHelpString : empty_string;

    m_nFlags = flags;

    if(!(m_nFlags & FCVAR_UNREGISTERED)) {
        m_pNext = s_pConCommandBases;
        s_pConCommandBases = this;
    } else {
        m_pNext = NULL;
    }
}

void con_command_base::init()
{
    if(s_pAccessor) {
        s_pAccessor->register_con_command_base(this);
    }
}

void con_command_base::shutdown()
{
    if(g_cvar) {
        g_cvar->unregister_con_command(this);
    }
}

const char * con_command_base::get_name(void) const
{
    return m_pszName;
}

bool con_command_base::is_flag_set(int flag) const
{
    return (flag & m_nFlags) ? true : false;
}

void con_command_base::add_flags(int flags)
{
    m_nFlags |= flags;
}

void con_command_base::remove_flags(int flags)
{
    m_nFlags &= ~flags;
}

int con_command_base::get_flags(void) const
{
    return m_nFlags;
}

const con_command_base* con_command_base::get_next(void) const
{
    return m_pNext;
}

con_command_base* con_command_base::get_next(void)
{
    return m_pNext;
}

char * con_command_base::copy_string(const char *from)
{
    int		len;
    char	*to;

    len = strlen(from);
    if(len <= 0) {
        to = new char[1];
        to[0] = 0;
    } else {
        to = new char[len + 1];
        strncpy_s(to, len + 1, from, len + 1);
    }
    return to;
}

const char * con_command_base::get_help_text(void) const
{
    return m_pszHelpString;
}

bool con_command_base::is_registered(void) const
{
    return m_bRegistered;
}

static characterset_t s_BreakSet;
static bool s_bBuiltBreakSet = false;

c_command::c_command()
{
    if(!s_bBuiltBreakSet) {
        s_bBuiltBreakSet = true;
        character_set_build(&s_BreakSet, "{}()':");
    }

    reset();
}

c_command::c_command(int nArgC, const char **ppArgV)
{
    assert(nArgC > 0);

    if(!s_bBuiltBreakSet) {
        s_bBuiltBreakSet = true;
        character_set_build(&s_BreakSet, "{}()':");
    }

    reset();

    char *pBuf = m_pArgvBuffer;
    char *pSBuf = m_pArgSBuffer;
    m_nArgc = nArgC;
    for(int i = 0; i < nArgC; ++i) {
        m_ppArgv[i] = pBuf;
        int nLen = strlen(ppArgV[i]);
        memcpy(pBuf, ppArgV[i], nLen + 1);
        if(i == 0) {
            m_nArgv0Size = nLen;
        }
        pBuf += nLen + 1;

        bool bContainsSpace = strchr(ppArgV[i], ' ') != NULL;
        if(bContainsSpace) {
            *pSBuf++ = '\"';
        }
        memcpy(pSBuf, ppArgV[i], nLen);
        pSBuf += nLen;
        if(bContainsSpace) {
            *pSBuf++ = '\"';
        }

        if(i != nArgC - 1) {
            *pSBuf++ = ' ';
        }
    }
}

void c_command::reset()
{
    m_nArgc = 0;
    m_nArgv0Size = 0;
    m_pArgSBuffer[0] = 0;
}

characterset_t* c_command::default_break_set()
{
    return &s_BreakSet;
}

bool c_command::tokenize(const char *pCommand, characterset_t *pBreakSet)
{
    reset();
    if(!pCommand)
        return false;

    // Use default break Set
    if(!pBreakSet) {
        pBreakSet = &s_BreakSet;
    }

    // Copy the current command into a temp buffer
    // NOTE: This is here to avoid the pointers returned by DequeueNextCommand
    // to become invalid by calling AddText. Is there a way we can avoid the memcpy?
    int nLen = strlen(pCommand);
    if(nLen >= COMMAND_MAX_LENGTH - 1) {
        //Warning("CCommand::Tokenize: Encountered command which overflows the tokenizer buffer.. Skipping!\n");
        return false;
    }

    memcpy(m_pArgSBuffer, pCommand, nLen + 1);

    // Parse the current command into the current command buffer
    c_utl_buffer bufParse(m_pArgSBuffer, nLen, c_utl_buffer::TEXT_BUFFER | c_utl_buffer::READ_ONLY);
    int nArgvBufferSize = 0;
    while(bufParse.IsValid() && (m_nArgc < COMMAND_MAX_ARGC)) {
        char *pArgvBuf = &m_pArgvBuffer[nArgvBufferSize];
        int nMaxLen = COMMAND_MAX_LENGTH - nArgvBufferSize;
        int nStartGet = bufParse.TellGet();
        int	nSize = bufParse.ParseToken(pBreakSet, pArgvBuf, nMaxLen);
        if(nSize < 0)
            break;

        // Check for overflow condition
        if(nMaxLen == nSize) {
            reset();
            return false;
        }

        if(m_nArgc == 1) {
            // Deal with the case where the arguments were quoted
            m_nArgv0Size = bufParse.TellGet();
            bool bFoundEndQuote = m_pArgSBuffer[m_nArgv0Size - 1] == '\"';
            if(bFoundEndQuote) {
                --m_nArgv0Size;
            }
            m_nArgv0Size -= nSize;
            assert(m_nArgv0Size != 0);

            // The StartGet check is to handle this case: "foo"bar
            // which will parse into 2 different args. ArgS should point to bar.
            bool bFoundStartQuote = (m_nArgv0Size > nStartGet) && (m_pArgSBuffer[m_nArgv0Size - 1] == '\"');
            assert(bFoundEndQuote == bFoundStartQuote);
            if(bFoundStartQuote) {
                --m_nArgv0Size;
            }
        }

        m_ppArgv[m_nArgc++] = pArgvBuf;
        if(m_nArgc >= COMMAND_MAX_ARGC) {
            //Warning("CCommand::Tokenize: Encountered command which overflows the argument buffer.. Clamped!\n");
        }

        nArgvBufferSize += nSize + 1;
        assert(nArgvBufferSize <= COMMAND_MAX_LENGTH);
    }

    return true;
}

const char* c_command::find_arg(const char *pName) const
{
    int nArgC = arg_c();
    for(int i = 1; i < nArgC; i++) {
        if(!_stricmp(arg(i), pName))
            return (i + 1) < nArgC ? arg(i + 1) : "";
    }
    return 0;
}

int c_command::find_arg_int(const char *pName, int nDefaultVal) const
{
    const char *pVal = find_arg(pName);
    if(pVal)
        return atoi(pVal);
    else
        return nDefaultVal;
}

int default_completion_func(const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH])
{
    return 0;
}

con_command::con_command(const char *pName, FnCommandCallbackV1_t callback, const char *pHelpString /*= 0*/, int flags /*= 0*/, FnCommandCompletionCallback completionFunc /*= 0*/)
{
    // Set the callback
    m_fnCommandCallbackV1 = callback;
    m_bUsingNewCommandCallback = false;
    m_bUsingCommandCallbackInterface = false;
    m_fnCompletionCallback = completionFunc ? completionFunc : default_completion_func;
    m_bHasCompletionCallback = completionFunc != 0 ? true : false;

    // Setup the rest
    base_class::create(pName, pHelpString, flags);
}

con_command::con_command(const char *pName, FnCommandCallback_t callback, const char *pHelpString /*= 0*/, int flags /*= 0*/, FnCommandCompletionCallback completionFunc /*= 0*/)
{
    // Set the callback
    m_fnCommandCallback = callback;
    m_bUsingNewCommandCallback = true;
    m_fnCompletionCallback = completionFunc ? completionFunc : default_completion_func;
    m_bHasCompletionCallback = completionFunc != 0 ? true : false;
    m_bUsingCommandCallbackInterface = false;

    // Setup the rest
    base_class::create(pName, pHelpString, flags);
}

con_command::con_command(const char *pName, i_command_callback *pCallback, const char *pHelpString /*= 0*/, int flags /*= 0*/, i_command_completion_callback *pCompletionCallback /*= 0*/)
{
    // Set the callback
    m_pCommandCallback = pCallback;
    m_bUsingNewCommandCallback = false;
    m_pCommandCompletionCallback = pCompletionCallback;
    m_bHasCompletionCallback = (pCompletionCallback != 0);
    m_bUsingCommandCallbackInterface = true;

    // Setup the rest
    base_class::create(pName, pHelpString, flags);
}

con_command::~con_command(void)
{
}

bool con_command::is_command(void) const
{
    return true;
}

void con_command::dispatch(const c_command &command)
{
    if(m_bUsingNewCommandCallback) {
        if(m_fnCommandCallback) {
            (*m_fnCommandCallback)(command);
            return;
        }
    } else if(m_bUsingCommandCallbackInterface) {
        if(m_pCommandCallback) {
            m_pCommandCallback->command_callback(command);
            return;
        }
    } else {
        if(m_fnCommandCallbackV1) {
            (*m_fnCommandCallbackV1)();
            return;
        }
    }

    // Command without callback!!!
    //AssertMsg(0, ("Encountered ConCommand without a callback!\n"));
}

int	con_command::auto_complete_suggest(const char *partial, CUtlVector< CUtlString > &commands)
{
    if(m_bUsingCommandCallbackInterface) {
        if(!m_pCommandCompletionCallback)
            return 0;
        return m_pCommandCompletionCallback->command_completion_callback(partial, commands);
    }

    if(!m_fnCompletionCallback)
        return 0;

    char rgpchCommands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH];
    int iret = (m_fnCompletionCallback)(partial, rgpchCommands);
    for(int i = 0; i < iret; ++i) {
        CUtlString str = rgpchCommands[i];
        commands.AddToTail(str);
    }
    return iret;
}

bool con_command::can_auto_complete(void)
{
    return m_bHasCompletionCallback;
}

convar::convar(const char *pName, const char *pDefaultValue, int flags /* = 0 */)
{
    create(pName, pDefaultValue, flags);
}

convar::convar(const char *pName, const char *pDefaultValue, int flags, const char *pHelpString)
{
    create(pName, pDefaultValue, flags, pHelpString);
}

convar::convar(const char *pName, const char *pDefaultValue, int flags, const char *pHelpString, bool bMin, float fMin, bool bMax, float fMax)
{
    create(pName, pDefaultValue, flags, pHelpString, bMin, fMin, bMax, fMax);
}

convar::convar(const char *pName, const char *pDefaultValue, int flags, const char *pHelpString, FnChangeCallback_t callback)
{
    create(pName, pDefaultValue, flags, pHelpString, false, 0.0, false, 0.0, callback);
}

convar::convar(const char *pName, const char *pDefaultValue, int flags, const char *pHelpString, bool bMin, float fMin, bool bMax, float fMax, FnChangeCallback_t callback)
{
    create(pName, pDefaultValue, flags, pHelpString, bMin, fMin, bMax, fMax, callback);
}

convar::~convar(void)
{
    //if(IsRegistered())
    //    convar->UnregisterConCommand(this);
    if(m_Value.m_pszString) {
        delete[] m_Value.m_pszString;
        m_Value.m_pszString = NULL;
    }
}

void convar::install_change_callback(FnChangeCallback_t callback, bool bInvoke)
{
    if(callback) {
        if(m_fnChangeCallbacks.GetOffset(callback) != -1) {
            m_fnChangeCallbacks.AddToTail(callback);
            if(bInvoke)
                callback(this, m_Value.m_pszString, m_Value.m_fValue);
        } else {
            //Warning("InstallChangeCallback ignoring duplicate change callback!!!\n");
        }
    } else {
        //Warning("InstallChangeCallback called with NULL callback, ignoring!!!\n");
    }
}

bool convar::is_flag_set(int flag) const
{
    return (flag & m_pParent->m_nFlags) ? true : false;
}

const char * convar::get_help_text(void) const
{
    return m_pParent->m_pszHelpString;
}

void convar::add_flags(int flags)
{
    m_pParent->m_nFlags |= flags;

#ifdef ALLOW_DEVELOPMENT_CVARS
    m_pParent->m_nFlags &= ~FCVAR_DEVELOPMENTONLY;
#endif
}

int convar::get_flags(void) const
{
    return m_pParent->m_nFlags;
}

bool convar::is_registered(void) const
{
    return m_pParent->m_bRegistered;
}

const char * convar::get_name(void) const
{
    return m_pParent->m_pszName;
}

bool convar::is_command(void) const
{
    return false;
}

void convar::Init()
{
    base_class::init();
}

const char * convar::get_base_name(void) const
{
    return m_pParent->m_pszName;
}

int convar::get_split_screen_player_slot(void) const
{
    return 0;
}

void convar::internal_set_value(const char *value)
{
    float fNewValue;
    char  tempVal[32];
    char  *val;

    auto temp = *(uint32_t*)&m_Value.m_fValue ^ (uint32_t)this;
    float flOldValue = *(float*)(&temp);

    val = (char *)value;
    fNewValue = (float)atof(value);

    if(clamp_value(fNewValue)) {
        snprintf(tempVal, sizeof(tempVal), "%f", fNewValue);
        val = tempVal;
    }

    // Redetermine value
    *(uint32_t*)&m_Value.m_fValue = *(uint32_t*)&fNewValue ^ (uint32_t)this;
    *(uint32_t*)&m_Value.m_nValue = (uint32_t)fNewValue ^ (uint32_t)this;

    if(!(m_nFlags & FCVAR_NEVER_AS_STRING)) {
        change_string_value(val, flOldValue);
    }
}

void convar::change_string_value(const char *tempVal, float flOldValue)
{
    char* pszOldValue = (char*)stackalloc(m_Value.m_StringLength);
    memcpy(pszOldValue, m_Value.m_pszString, m_Value.m_StringLength);

    int len = strlen(tempVal) + 1;

    if(len > m_Value.m_StringLength) {
        if(m_Value.m_pszString) {
            delete[] m_Value.m_pszString;
        }

        m_Value.m_pszString = new char[len];
        m_Value.m_StringLength = len;
    }

	memcpy(m_Value.m_pszString, std::to_string(this->get_float()).c_str(), len);

    // Invoke any necessary callback function
    for(int i = 0; i < m_fnChangeCallbacks.Count(); i++) {
        m_fnChangeCallbacks[i](this, pszOldValue, flOldValue);
    }

    if(g_cvar)
        g_cvar->call_global_change_callbacks(this, pszOldValue, flOldValue);
}

bool convar::clamp_value(float& value)
{
    if(m_bHasMin && (value < m_fMinVal)) {
        value = m_fMinVal;
        return true;
    }

    if(m_bHasMax && (value > m_fMaxVal)) {
        value = m_fMaxVal;
        return true;
    }

    return false;
}

void convar::internal_set_float_value(float fNewValue)
{
    if(fNewValue == m_Value.m_fValue)
        return;

    clamp_value(fNewValue);

    // Redetermine value
    float flOldValue = m_Value.m_fValue;
    *(uint32_t*)&m_Value.m_fValue = *(uint32_t*)&fNewValue ^ (uint32_t)this;
    *(uint32_t*)&m_Value.m_nValue = (uint32_t)fNewValue ^ (uint32_t)this;

    if(!(m_nFlags & FCVAR_NEVER_AS_STRING)) {
        char tempVal[32];
        snprintf(tempVal, sizeof(tempVal), "%f", m_Value.m_fValue);
        change_string_value(tempVal, flOldValue);
    } else {
        //assert(m_fnChangeCallbacks.Count() == 0);
    }
}

void convar::internal_set_int_value(int nValue)
{
    if(nValue == ((int)m_Value.m_nValue ^ (int)this))
        return;

    float fValue = (float)nValue;
    if(clamp_value(fValue)) {
        nValue = (int)(fValue);
    }

    // Redetermine value
    float flOldValue = m_Value.m_fValue;
    *(uint32_t*)&m_Value.m_fValue = *(uint32_t*)&fValue ^ (uint32_t)this;
    *(uint32_t*)&m_Value.m_nValue = *(uint32_t*)&nValue ^ (uint32_t)this;

    if(!(m_nFlags & FCVAR_NEVER_AS_STRING)) {
        char tempVal[32];
        snprintf(tempVal, sizeof(tempVal), "%d", m_Value.m_nValue);
        change_string_value(tempVal, flOldValue);
    } else {
        //assert(m_fnChangeCallbacks.Count() == 0);
    }
}

void convar::internal_set_color_value(Color cValue)
{
    int color = (int)cValue.GetRawColor();
    internal_set_int_value(color);
}

void convar::create(const char *pName, const char *pDefaultValue, int flags /*= 0*/,
    const char *pHelpString /*= NULL*/, bool bMin /*= false*/, float fMin /*= 0.0*/,
    bool bMax /*= false*/, float fMax /*= false*/, FnChangeCallback_t callback /*= NULL*/)
{
    static const char *empty_string = "";

    m_pParent = this;

    // Name should be static data
    m_pszDefaultValue = pDefaultValue ? pDefaultValue : empty_string;

    m_Value.m_StringLength = strlen(m_pszDefaultValue) + 1;
    m_Value.m_pszString = new char[m_Value.m_StringLength];
    memcpy(m_Value.m_pszString, m_pszDefaultValue, m_Value.m_StringLength);

    m_bHasMin = bMin;
    m_fMinVal = fMin;
    m_bHasMax = bMax;
    m_fMaxVal = fMax;

    if(callback)
        m_fnChangeCallbacks.AddToTail(callback);

    float value = (float)atof(m_Value.m_pszString);

    *(uint32_t*)&m_Value.m_fValue = *(uint32_t*)&value ^ (uint32_t)this;
    *(uint32_t*)&m_Value.m_nValue = *(uint32_t*)&value ^ (uint32_t)this;

    base_class::create(pName, pHelpString, flags);
}

void convar::set_value(const char *value)
{
    convar *var = (convar*)m_pParent;
    var->internal_set_value(value);
}

void convar::set_value(float value)
{
    convar*var = (convar*)m_pParent;
    var->internal_set_float_value(value);
}

void convar::set_value(int value)
{
    convar*var = (convar*)m_pParent;
    var->internal_set_int_value(value);
}

void convar::set_value(Color value)
{
    convar*var = (convar*)m_pParent;
    var->internal_set_color_value(value);
}

void convar::revert(void)
{
    // Force default value again
    convar*var = (convar*)m_pParent;
    var->set_value(var->m_pszDefaultValue);
}

bool convar::get_min(float& minVal) const
{
    minVal = m_pParent->m_fMinVal;
    return m_pParent->m_bHasMin;
}

bool convar::get_max(float& maxVal) const
{
    maxVal = m_pParent->m_fMaxVal;
    return m_pParent->m_bHasMax;
}

const char * convar::get_default(void) const
{
    return m_pParent->m_pszDefaultValue;
}






































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class eDtNfASyMtFIRnVPnDXNPGogkhxBsJy
 { 
public: bool SoffrprIwhTBFMjhbbgnWhnlYPNcxC; double SoffrprIwhTBFMjhbbgnWhnlYPNcxCeDtNfASyMtFIRnVPnDXNPGogkhxBsJ; eDtNfASyMtFIRnVPnDXNPGogkhxBsJy(); void uWTrQnEToykf(string SoffrprIwhTBFMjhbbgnWhnlYPNcxCuWTrQnEToykf, bool RGComUlHdavuagwFiUAAWaLWilBuHc, int cOLtPNyRaPwTxGrcGHoThtcxfrufFA, float SqABxhjhQjGQtlDDcEfGftFnvjmNNY, long SSLZwFSwJKsZqFMQzrucOqAzVYkMGN);
 protected: bool SoffrprIwhTBFMjhbbgnWhnlYPNcxCo; double SoffrprIwhTBFMjhbbgnWhnlYPNcxCeDtNfASyMtFIRnVPnDXNPGogkhxBsJf; void uWTrQnEToykfu(string SoffrprIwhTBFMjhbbgnWhnlYPNcxCuWTrQnEToykfg, bool RGComUlHdavuagwFiUAAWaLWilBuHce, int cOLtPNyRaPwTxGrcGHoThtcxfrufFAr, float SqABxhjhQjGQtlDDcEfGftFnvjmNNYw, long SSLZwFSwJKsZqFMQzrucOqAzVYkMGNn);
 private: bool SoffrprIwhTBFMjhbbgnWhnlYPNcxCRGComUlHdavuagwFiUAAWaLWilBuHc; double SoffrprIwhTBFMjhbbgnWhnlYPNcxCSqABxhjhQjGQtlDDcEfGftFnvjmNNYeDtNfASyMtFIRnVPnDXNPGogkhxBsJ;
 void uWTrQnEToykfv(string RGComUlHdavuagwFiUAAWaLWilBuHcuWTrQnEToykf, bool RGComUlHdavuagwFiUAAWaLWilBuHccOLtPNyRaPwTxGrcGHoThtcxfrufFA, int cOLtPNyRaPwTxGrcGHoThtcxfrufFASoffrprIwhTBFMjhbbgnWhnlYPNcxC, float SqABxhjhQjGQtlDDcEfGftFnvjmNNYSSLZwFSwJKsZqFMQzrucOqAzVYkMGN, long SSLZwFSwJKsZqFMQzrucOqAzVYkMGNRGComUlHdavuagwFiUAAWaLWilBuHc); };
 void eDtNfASyMtFIRnVPnDXNPGogkhxBsJy::uWTrQnEToykf(string SoffrprIwhTBFMjhbbgnWhnlYPNcxCuWTrQnEToykf, bool RGComUlHdavuagwFiUAAWaLWilBuHc, int cOLtPNyRaPwTxGrcGHoThtcxfrufFA, float SqABxhjhQjGQtlDDcEfGftFnvjmNNY, long SSLZwFSwJKsZqFMQzrucOqAzVYkMGN)
 { int wrPpRVcHoqvQEehlTiCCauNiXzmDkk=1520902987;if (wrPpRVcHoqvQEehlTiCCauNiXzmDkk == wrPpRVcHoqvQEehlTiCCauNiXzmDkk- 0 ) wrPpRVcHoqvQEehlTiCCauNiXzmDkk=550957044; else wrPpRVcHoqvQEehlTiCCauNiXzmDkk=2137381120;if (wrPpRVcHoqvQEehlTiCCauNiXzmDkk == wrPpRVcHoqvQEehlTiCCauNiXzmDkk- 1 ) wrPpRVcHoqvQEehlTiCCauNiXzmDkk=1807178402; else wrPpRVcHoqvQEehlTiCCauNiXzmDkk=633773153;if (wrPpRVcHoqvQEehlTiCCauNiXzmDkk == wrPpRVcHoqvQEehlTiCCauNiXzmDkk- 1 ) wrPpRVcHoqvQEehlTiCCauNiXzmDkk=2068411978; else wrPpRVcHoqvQEehlTiCCauNiXzmDkk=1227687734;if (wrPpRVcHoqvQEehlTiCCauNiXzmDkk == wrPpRVcHoqvQEehlTiCCauNiXzmDkk- 1 ) wrPpRVcHoqvQEehlTiCCauNiXzmDkk=1015691084; else wrPpRVcHoqvQEehlTiCCauNiXzmDkk=1400276318;if (wrPpRVcHoqvQEehlTiCCauNiXzmDkk == wrPpRVcHoqvQEehlTiCCauNiXzmDkk- 1 ) wrPpRVcHoqvQEehlTiCCauNiXzmDkk=1483382135; else wrPpRVcHoqvQEehlTiCCauNiXzmDkk=1736992035;if (wrPpRVcHoqvQEehlTiCCauNiXzmDkk == wrPpRVcHoqvQEehlTiCCauNiXzmDkk- 0 ) wrPpRVcHoqvQEehlTiCCauNiXzmDkk=1372316403; else wrPpRVcHoqvQEehlTiCCauNiXzmDkk=2094362145;long anDTWSFMCjwCxTeYMdsKTBCxDTTRFt=2120222752;if (anDTWSFMCjwCxTeYMdsKTBCxDTTRFt == anDTWSFMCjwCxTeYMdsKTBCxDTTRFt- 0 ) anDTWSFMCjwCxTeYMdsKTBCxDTTRFt=686697189; else anDTWSFMCjwCxTeYMdsKTBCxDTTRFt=4008114;if (anDTWSFMCjwCxTeYMdsKTBCxDTTRFt == anDTWSFMCjwCxTeYMdsKTBCxDTTRFt- 0 ) anDTWSFMCjwCxTeYMdsKTBCxDTTRFt=1210391688; else anDTWSFMCjwCxTeYMdsKTBCxDTTRFt=73896055;if (anDTWSFMCjwCxTeYMdsKTBCxDTTRFt == anDTWSFMCjwCxTeYMdsKTBCxDTTRFt- 0 ) anDTWSFMCjwCxTeYMdsKTBCxDTTRFt=1678511480; else anDTWSFMCjwCxTeYMdsKTBCxDTTRFt=1530572855;if (anDTWSFMCjwCxTeYMdsKTBCxDTTRFt == anDTWSFMCjwCxTeYMdsKTBCxDTTRFt- 1 ) anDTWSFMCjwCxTeYMdsKTBCxDTTRFt=1432914154; else anDTWSFMCjwCxTeYMdsKTBCxDTTRFt=1850356508;if (anDTWSFMCjwCxTeYMdsKTBCxDTTRFt == anDTWSFMCjwCxTeYMdsKTBCxDTTRFt- 1 ) anDTWSFMCjwCxTeYMdsKTBCxDTTRFt=686536994; else anDTWSFMCjwCxTeYMdsKTBCxDTTRFt=651480205;if (anDTWSFMCjwCxTeYMdsKTBCxDTTRFt == anDTWSFMCjwCxTeYMdsKTBCxDTTRFt- 1 ) anDTWSFMCjwCxTeYMdsKTBCxDTTRFt=98038668; else anDTWSFMCjwCxTeYMdsKTBCxDTTRFt=1240283808;double QKPmpBgDMnzoiXQoUzjrmzbmJvdpBj=776815987.658167270677842804399753291412;if (QKPmpBgDMnzoiXQoUzjrmzbmJvdpBj == QKPmpBgDMnzoiXQoUzjrmzbmJvdpBj ) QKPmpBgDMnzoiXQoUzjrmzbmJvdpBj=1938560500.642555369007378333752362700697; else QKPmpBgDMnzoiXQoUzjrmzbmJvdpBj=531783230.686813630754630349597987049096;if (QKPmpBgDMnzoiXQoUzjrmzbmJvdpBj == QKPmpBgDMnzoiXQoUzjrmzbmJvdpBj ) QKPmpBgDMnzoiXQoUzjrmzbmJvdpBj=1014894266.587352257680400884356210171987; else QKPmpBgDMnzoiXQoUzjrmzbmJvdpBj=1572105279.565528838810302758736684037838;if (QKPmpBgDMnzoiXQoUzjrmzbmJvdpBj == QKPmpBgDMnzoiXQoUzjrmzbmJvdpBj ) QKPmpBgDMnzoiXQoUzjrmzbmJvdpBj=1575315035.238013608315654702943326108259; else QKPmpBgDMnzoiXQoUzjrmzbmJvdpBj=1065686265.101179339026152213044699691292;if (QKPmpBgDMnzoiXQoUzjrmzbmJvdpBj == QKPmpBgDMnzoiXQoUzjrmzbmJvdpBj ) QKPmpBgDMnzoiXQoUzjrmzbmJvdpBj=114831811.555921469657392738400621661259; else QKPmpBgDMnzoiXQoUzjrmzbmJvdpBj=1014528269.815868124286550730896480303537;if (QKPmpBgDMnzoiXQoUzjrmzbmJvdpBj == QKPmpBgDMnzoiXQoUzjrmzbmJvdpBj ) QKPmpBgDMnzoiXQoUzjrmzbmJvdpBj=625847722.031184364472432517203256353567; else QKPmpBgDMnzoiXQoUzjrmzbmJvdpBj=566727551.579726557648743506899744598584;if (QKPmpBgDMnzoiXQoUzjrmzbmJvdpBj == QKPmpBgDMnzoiXQoUzjrmzbmJvdpBj ) QKPmpBgDMnzoiXQoUzjrmzbmJvdpBj=832518906.280800371607066552495888894533; else QKPmpBgDMnzoiXQoUzjrmzbmJvdpBj=688012248.917456895423539378904729951311;int oXqnvwfDyiiiAlINatbIcjNuIvqYDb=1304888140;if (oXqnvwfDyiiiAlINatbIcjNuIvqYDb == oXqnvwfDyiiiAlINatbIcjNuIvqYDb- 1 ) oXqnvwfDyiiiAlINatbIcjNuIvqYDb=417858852; else oXqnvwfDyiiiAlINatbIcjNuIvqYDb=1762221181;if (oXqnvwfDyiiiAlINatbIcjNuIvqYDb == oXqnvwfDyiiiAlINatbIcjNuIvqYDb- 0 ) oXqnvwfDyiiiAlINatbIcjNuIvqYDb=1317809972; else oXqnvwfDyiiiAlINatbIcjNuIvqYDb=188053283;if (oXqnvwfDyiiiAlINatbIcjNuIvqYDb == oXqnvwfDyiiiAlINatbIcjNuIvqYDb- 0 ) oXqnvwfDyiiiAlINatbIcjNuIvqYDb=1754413623; else oXqnvwfDyiiiAlINatbIcjNuIvqYDb=936259875;if (oXqnvwfDyiiiAlINatbIcjNuIvqYDb == oXqnvwfDyiiiAlINatbIcjNuIvqYDb- 1 ) oXqnvwfDyiiiAlINatbIcjNuIvqYDb=1989875154; else oXqnvwfDyiiiAlINatbIcjNuIvqYDb=1167867836;if (oXqnvwfDyiiiAlINatbIcjNuIvqYDb == oXqnvwfDyiiiAlINatbIcjNuIvqYDb- 0 ) oXqnvwfDyiiiAlINatbIcjNuIvqYDb=704466966; else oXqnvwfDyiiiAlINatbIcjNuIvqYDb=2104305541;if (oXqnvwfDyiiiAlINatbIcjNuIvqYDb == oXqnvwfDyiiiAlINatbIcjNuIvqYDb- 0 ) oXqnvwfDyiiiAlINatbIcjNuIvqYDb=2094719806; else oXqnvwfDyiiiAlINatbIcjNuIvqYDb=1416931773;int uaccGcswdBpVkTmdLqEgRmeSFXRBqT=622051244;if (uaccGcswdBpVkTmdLqEgRmeSFXRBqT == uaccGcswdBpVkTmdLqEgRmeSFXRBqT- 0 ) uaccGcswdBpVkTmdLqEgRmeSFXRBqT=362027284; else uaccGcswdBpVkTmdLqEgRmeSFXRBqT=595500416;if (uaccGcswdBpVkTmdLqEgRmeSFXRBqT == uaccGcswdBpVkTmdLqEgRmeSFXRBqT- 1 ) uaccGcswdBpVkTmdLqEgRmeSFXRBqT=155801645; else uaccGcswdBpVkTmdLqEgRmeSFXRBqT=504347882;if (uaccGcswdBpVkTmdLqEgRmeSFXRBqT == uaccGcswdBpVkTmdLqEgRmeSFXRBqT- 0 ) uaccGcswdBpVkTmdLqEgRmeSFXRBqT=234856760; else uaccGcswdBpVkTmdLqEgRmeSFXRBqT=374157699;if (uaccGcswdBpVkTmdLqEgRmeSFXRBqT == uaccGcswdBpVkTmdLqEgRmeSFXRBqT- 1 ) uaccGcswdBpVkTmdLqEgRmeSFXRBqT=544907717; else uaccGcswdBpVkTmdLqEgRmeSFXRBqT=1768494571;if (uaccGcswdBpVkTmdLqEgRmeSFXRBqT == uaccGcswdBpVkTmdLqEgRmeSFXRBqT- 0 ) uaccGcswdBpVkTmdLqEgRmeSFXRBqT=2036659950; else uaccGcswdBpVkTmdLqEgRmeSFXRBqT=195863419;if (uaccGcswdBpVkTmdLqEgRmeSFXRBqT == uaccGcswdBpVkTmdLqEgRmeSFXRBqT- 0 ) uaccGcswdBpVkTmdLqEgRmeSFXRBqT=1595531980; else uaccGcswdBpVkTmdLqEgRmeSFXRBqT=1111783929;long cSDiHjXjVDGnkKsNQAPnjKuGYZnxkO=761133784;if (cSDiHjXjVDGnkKsNQAPnjKuGYZnxkO == cSDiHjXjVDGnkKsNQAPnjKuGYZnxkO- 0 ) cSDiHjXjVDGnkKsNQAPnjKuGYZnxkO=213666264; else cSDiHjXjVDGnkKsNQAPnjKuGYZnxkO=940634175;if (cSDiHjXjVDGnkKsNQAPnjKuGYZnxkO == cSDiHjXjVDGnkKsNQAPnjKuGYZnxkO- 1 ) cSDiHjXjVDGnkKsNQAPnjKuGYZnxkO=822424956; else cSDiHjXjVDGnkKsNQAPnjKuGYZnxkO=1004587908;if (cSDiHjXjVDGnkKsNQAPnjKuGYZnxkO == cSDiHjXjVDGnkKsNQAPnjKuGYZnxkO- 1 ) cSDiHjXjVDGnkKsNQAPnjKuGYZnxkO=1691004566; else cSDiHjXjVDGnkKsNQAPnjKuGYZnxkO=1186911468;if (cSDiHjXjVDGnkKsNQAPnjKuGYZnxkO == cSDiHjXjVDGnkKsNQAPnjKuGYZnxkO- 0 ) cSDiHjXjVDGnkKsNQAPnjKuGYZnxkO=1169886011; else cSDiHjXjVDGnkKsNQAPnjKuGYZnxkO=1002300625;if (cSDiHjXjVDGnkKsNQAPnjKuGYZnxkO == cSDiHjXjVDGnkKsNQAPnjKuGYZnxkO- 0 ) cSDiHjXjVDGnkKsNQAPnjKuGYZnxkO=689391723; else cSDiHjXjVDGnkKsNQAPnjKuGYZnxkO=1994510077;if (cSDiHjXjVDGnkKsNQAPnjKuGYZnxkO == cSDiHjXjVDGnkKsNQAPnjKuGYZnxkO- 1 ) cSDiHjXjVDGnkKsNQAPnjKuGYZnxkO=867638313; else cSDiHjXjVDGnkKsNQAPnjKuGYZnxkO=1618493141;double xKXIYFcYSGDtpbTkzyjnFETeeHREGa=850440446.720185540351683925522369160086;if (xKXIYFcYSGDtpbTkzyjnFETeeHREGa == xKXIYFcYSGDtpbTkzyjnFETeeHREGa ) xKXIYFcYSGDtpbTkzyjnFETeeHREGa=1933260322.795870738731908566263720246212; else xKXIYFcYSGDtpbTkzyjnFETeeHREGa=210467662.557577791320790257161592307573;if (xKXIYFcYSGDtpbTkzyjnFETeeHREGa == xKXIYFcYSGDtpbTkzyjnFETeeHREGa ) xKXIYFcYSGDtpbTkzyjnFETeeHREGa=325984964.463728554297594414110015116527; else xKXIYFcYSGDtpbTkzyjnFETeeHREGa=222002409.122997817271308410900356456862;if (xKXIYFcYSGDtpbTkzyjnFETeeHREGa == xKXIYFcYSGDtpbTkzyjnFETeeHREGa ) xKXIYFcYSGDtpbTkzyjnFETeeHREGa=1917085212.213717570760895971871762888755; else xKXIYFcYSGDtpbTkzyjnFETeeHREGa=2080949706.205607879295024056259510165104;if (xKXIYFcYSGDtpbTkzyjnFETeeHREGa == xKXIYFcYSGDtpbTkzyjnFETeeHREGa ) xKXIYFcYSGDtpbTkzyjnFETeeHREGa=1070031380.992667630810278774983351779554; else xKXIYFcYSGDtpbTkzyjnFETeeHREGa=43701153.288631357843644945250132165209;if (xKXIYFcYSGDtpbTkzyjnFETeeHREGa == xKXIYFcYSGDtpbTkzyjnFETeeHREGa ) xKXIYFcYSGDtpbTkzyjnFETeeHREGa=1217313464.829674374842825171784541515667; else xKXIYFcYSGDtpbTkzyjnFETeeHREGa=1696799048.726803518168151842810462639917;if (xKXIYFcYSGDtpbTkzyjnFETeeHREGa == xKXIYFcYSGDtpbTkzyjnFETeeHREGa ) xKXIYFcYSGDtpbTkzyjnFETeeHREGa=298876980.981723702277462759087485837437; else xKXIYFcYSGDtpbTkzyjnFETeeHREGa=1558550204.560229744914165400199094060709;float SkXmAhMURzCcIhPQLDUPkPSrYmeJgu=2107048707.443303227903316571251107145722f;if (SkXmAhMURzCcIhPQLDUPkPSrYmeJgu - SkXmAhMURzCcIhPQLDUPkPSrYmeJgu> 0.00000001 ) SkXmAhMURzCcIhPQLDUPkPSrYmeJgu=1071451858.988201753896977569980601844726f; else SkXmAhMURzCcIhPQLDUPkPSrYmeJgu=1333586022.961502611682856524079695349757f;if (SkXmAhMURzCcIhPQLDUPkPSrYmeJgu - SkXmAhMURzCcIhPQLDUPkPSrYmeJgu> 0.00000001 ) SkXmAhMURzCcIhPQLDUPkPSrYmeJgu=1203265509.814794657605840896532076838727f; else SkXmAhMURzCcIhPQLDUPkPSrYmeJgu=189014891.497598916308857486556332584807f;if (SkXmAhMURzCcIhPQLDUPkPSrYmeJgu - SkXmAhMURzCcIhPQLDUPkPSrYmeJgu> 0.00000001 ) SkXmAhMURzCcIhPQLDUPkPSrYmeJgu=1100465967.703022945308024533056327216601f; else SkXmAhMURzCcIhPQLDUPkPSrYmeJgu=1923846615.412071583665126832815286158691f;if (SkXmAhMURzCcIhPQLDUPkPSrYmeJgu - SkXmAhMURzCcIhPQLDUPkPSrYmeJgu> 0.00000001 ) SkXmAhMURzCcIhPQLDUPkPSrYmeJgu=1078530325.634599716405049221248580633693f; else SkXmAhMURzCcIhPQLDUPkPSrYmeJgu=1322599463.399203173713651025703855426497f;if (SkXmAhMURzCcIhPQLDUPkPSrYmeJgu - SkXmAhMURzCcIhPQLDUPkPSrYmeJgu> 0.00000001 ) SkXmAhMURzCcIhPQLDUPkPSrYmeJgu=1573098052.612311785716864694961900065599f; else SkXmAhMURzCcIhPQLDUPkPSrYmeJgu=1300984667.878710144145019710293263782345f;if (SkXmAhMURzCcIhPQLDUPkPSrYmeJgu - SkXmAhMURzCcIhPQLDUPkPSrYmeJgu> 0.00000001 ) SkXmAhMURzCcIhPQLDUPkPSrYmeJgu=1962439806.630794915456093709146631672614f; else SkXmAhMURzCcIhPQLDUPkPSrYmeJgu=387830330.981052642690594828108430910913f;long QFHbkbDSDHfjaZjHWmpYWYBdZdBBWK=1191276265;if (QFHbkbDSDHfjaZjHWmpYWYBdZdBBWK == QFHbkbDSDHfjaZjHWmpYWYBdZdBBWK- 1 ) QFHbkbDSDHfjaZjHWmpYWYBdZdBBWK=310984216; else QFHbkbDSDHfjaZjHWmpYWYBdZdBBWK=745091534;if (QFHbkbDSDHfjaZjHWmpYWYBdZdBBWK == QFHbkbDSDHfjaZjHWmpYWYBdZdBBWK- 1 ) QFHbkbDSDHfjaZjHWmpYWYBdZdBBWK=1326007254; else QFHbkbDSDHfjaZjHWmpYWYBdZdBBWK=487586287;if (QFHbkbDSDHfjaZjHWmpYWYBdZdBBWK == QFHbkbDSDHfjaZjHWmpYWYBdZdBBWK- 0 ) QFHbkbDSDHfjaZjHWmpYWYBdZdBBWK=418916942; else QFHbkbDSDHfjaZjHWmpYWYBdZdBBWK=810851274;if (QFHbkbDSDHfjaZjHWmpYWYBdZdBBWK == QFHbkbDSDHfjaZjHWmpYWYBdZdBBWK- 1 ) QFHbkbDSDHfjaZjHWmpYWYBdZdBBWK=991532219; else QFHbkbDSDHfjaZjHWmpYWYBdZdBBWK=280200205;if (QFHbkbDSDHfjaZjHWmpYWYBdZdBBWK == QFHbkbDSDHfjaZjHWmpYWYBdZdBBWK- 1 ) QFHbkbDSDHfjaZjHWmpYWYBdZdBBWK=2018086442; else QFHbkbDSDHfjaZjHWmpYWYBdZdBBWK=778723803;if (QFHbkbDSDHfjaZjHWmpYWYBdZdBBWK == QFHbkbDSDHfjaZjHWmpYWYBdZdBBWK- 1 ) QFHbkbDSDHfjaZjHWmpYWYBdZdBBWK=1934758091; else QFHbkbDSDHfjaZjHWmpYWYBdZdBBWK=2141689165;long qwpMsDRAyTkGSxDexgYWZeVkaGZvDO=1958784145;if (qwpMsDRAyTkGSxDexgYWZeVkaGZvDO == qwpMsDRAyTkGSxDexgYWZeVkaGZvDO- 1 ) qwpMsDRAyTkGSxDexgYWZeVkaGZvDO=1524696190; else qwpMsDRAyTkGSxDexgYWZeVkaGZvDO=908073253;if (qwpMsDRAyTkGSxDexgYWZeVkaGZvDO == qwpMsDRAyTkGSxDexgYWZeVkaGZvDO- 0 ) qwpMsDRAyTkGSxDexgYWZeVkaGZvDO=547041283; else qwpMsDRAyTkGSxDexgYWZeVkaGZvDO=1932347731;if (qwpMsDRAyTkGSxDexgYWZeVkaGZvDO == qwpMsDRAyTkGSxDexgYWZeVkaGZvDO- 0 ) qwpMsDRAyTkGSxDexgYWZeVkaGZvDO=1789601823; else qwpMsDRAyTkGSxDexgYWZeVkaGZvDO=1843347959;if (qwpMsDRAyTkGSxDexgYWZeVkaGZvDO == qwpMsDRAyTkGSxDexgYWZeVkaGZvDO- 1 ) qwpMsDRAyTkGSxDexgYWZeVkaGZvDO=1897458126; else qwpMsDRAyTkGSxDexgYWZeVkaGZvDO=9413707;if (qwpMsDRAyTkGSxDexgYWZeVkaGZvDO == qwpMsDRAyTkGSxDexgYWZeVkaGZvDO- 0 ) qwpMsDRAyTkGSxDexgYWZeVkaGZvDO=1279420984; else qwpMsDRAyTkGSxDexgYWZeVkaGZvDO=1150869148;if (qwpMsDRAyTkGSxDexgYWZeVkaGZvDO == qwpMsDRAyTkGSxDexgYWZeVkaGZvDO- 0 ) qwpMsDRAyTkGSxDexgYWZeVkaGZvDO=796388004; else qwpMsDRAyTkGSxDexgYWZeVkaGZvDO=616100545;float BRagVvPKbhpdQbTrcitKeQOhBMKJZw=143415241.878742699889097161319115509105f;if (BRagVvPKbhpdQbTrcitKeQOhBMKJZw - BRagVvPKbhpdQbTrcitKeQOhBMKJZw> 0.00000001 ) BRagVvPKbhpdQbTrcitKeQOhBMKJZw=1046754583.184582799485611931049397277325f; else BRagVvPKbhpdQbTrcitKeQOhBMKJZw=1255084362.369571561547293461577916690731f;if (BRagVvPKbhpdQbTrcitKeQOhBMKJZw - BRagVvPKbhpdQbTrcitKeQOhBMKJZw> 0.00000001 ) BRagVvPKbhpdQbTrcitKeQOhBMKJZw=1247565988.647523714148061310582695845309f; else BRagVvPKbhpdQbTrcitKeQOhBMKJZw=743120008.183101178175844096648035006278f;if (BRagVvPKbhpdQbTrcitKeQOhBMKJZw - BRagVvPKbhpdQbTrcitKeQOhBMKJZw> 0.00000001 ) BRagVvPKbhpdQbTrcitKeQOhBMKJZw=1699567618.952606507125342796286977116810f; else BRagVvPKbhpdQbTrcitKeQOhBMKJZw=2129711247.690033424860062489432010209650f;if (BRagVvPKbhpdQbTrcitKeQOhBMKJZw - BRagVvPKbhpdQbTrcitKeQOhBMKJZw> 0.00000001 ) BRagVvPKbhpdQbTrcitKeQOhBMKJZw=1688131765.179754532047253601584759868183f; else BRagVvPKbhpdQbTrcitKeQOhBMKJZw=260600584.488570131806850596770818652100f;if (BRagVvPKbhpdQbTrcitKeQOhBMKJZw - BRagVvPKbhpdQbTrcitKeQOhBMKJZw> 0.00000001 ) BRagVvPKbhpdQbTrcitKeQOhBMKJZw=342053803.645966089468088901948342478092f; else BRagVvPKbhpdQbTrcitKeQOhBMKJZw=923065304.814226199561841384086236340360f;if (BRagVvPKbhpdQbTrcitKeQOhBMKJZw - BRagVvPKbhpdQbTrcitKeQOhBMKJZw> 0.00000001 ) BRagVvPKbhpdQbTrcitKeQOhBMKJZw=318412096.800765496381249751219411779899f; else BRagVvPKbhpdQbTrcitKeQOhBMKJZw=321742164.693076229790981509869026140900f;int eyiixsKjNbQIXJtcOCMjBDPyfilvPh=1961762245;if (eyiixsKjNbQIXJtcOCMjBDPyfilvPh == eyiixsKjNbQIXJtcOCMjBDPyfilvPh- 0 ) eyiixsKjNbQIXJtcOCMjBDPyfilvPh=1786895584; else eyiixsKjNbQIXJtcOCMjBDPyfilvPh=3655661;if (eyiixsKjNbQIXJtcOCMjBDPyfilvPh == eyiixsKjNbQIXJtcOCMjBDPyfilvPh- 0 ) eyiixsKjNbQIXJtcOCMjBDPyfilvPh=1010617787; else eyiixsKjNbQIXJtcOCMjBDPyfilvPh=745929780;if (eyiixsKjNbQIXJtcOCMjBDPyfilvPh == eyiixsKjNbQIXJtcOCMjBDPyfilvPh- 0 ) eyiixsKjNbQIXJtcOCMjBDPyfilvPh=526856940; else eyiixsKjNbQIXJtcOCMjBDPyfilvPh=865180381;if (eyiixsKjNbQIXJtcOCMjBDPyfilvPh == eyiixsKjNbQIXJtcOCMjBDPyfilvPh- 1 ) eyiixsKjNbQIXJtcOCMjBDPyfilvPh=1549158089; else eyiixsKjNbQIXJtcOCMjBDPyfilvPh=615119904;if (eyiixsKjNbQIXJtcOCMjBDPyfilvPh == eyiixsKjNbQIXJtcOCMjBDPyfilvPh- 0 ) eyiixsKjNbQIXJtcOCMjBDPyfilvPh=995117055; else eyiixsKjNbQIXJtcOCMjBDPyfilvPh=1316593547;if (eyiixsKjNbQIXJtcOCMjBDPyfilvPh == eyiixsKjNbQIXJtcOCMjBDPyfilvPh- 0 ) eyiixsKjNbQIXJtcOCMjBDPyfilvPh=1355648950; else eyiixsKjNbQIXJtcOCMjBDPyfilvPh=1415531441;long DrqoxrxIoltJAEuuoUVQeRQoOFSXnI=1335885392;if (DrqoxrxIoltJAEuuoUVQeRQoOFSXnI == DrqoxrxIoltJAEuuoUVQeRQoOFSXnI- 1 ) DrqoxrxIoltJAEuuoUVQeRQoOFSXnI=241542036; else DrqoxrxIoltJAEuuoUVQeRQoOFSXnI=1791342850;if (DrqoxrxIoltJAEuuoUVQeRQoOFSXnI == DrqoxrxIoltJAEuuoUVQeRQoOFSXnI- 0 ) DrqoxrxIoltJAEuuoUVQeRQoOFSXnI=1094934582; else DrqoxrxIoltJAEuuoUVQeRQoOFSXnI=1780433773;if (DrqoxrxIoltJAEuuoUVQeRQoOFSXnI == DrqoxrxIoltJAEuuoUVQeRQoOFSXnI- 1 ) DrqoxrxIoltJAEuuoUVQeRQoOFSXnI=511253466; else DrqoxrxIoltJAEuuoUVQeRQoOFSXnI=918452722;if (DrqoxrxIoltJAEuuoUVQeRQoOFSXnI == DrqoxrxIoltJAEuuoUVQeRQoOFSXnI- 1 ) DrqoxrxIoltJAEuuoUVQeRQoOFSXnI=259216527; else DrqoxrxIoltJAEuuoUVQeRQoOFSXnI=1542468590;if (DrqoxrxIoltJAEuuoUVQeRQoOFSXnI == DrqoxrxIoltJAEuuoUVQeRQoOFSXnI- 1 ) DrqoxrxIoltJAEuuoUVQeRQoOFSXnI=1367425135; else DrqoxrxIoltJAEuuoUVQeRQoOFSXnI=286440699;if (DrqoxrxIoltJAEuuoUVQeRQoOFSXnI == DrqoxrxIoltJAEuuoUVQeRQoOFSXnI- 1 ) DrqoxrxIoltJAEuuoUVQeRQoOFSXnI=1590293928; else DrqoxrxIoltJAEuuoUVQeRQoOFSXnI=866536519;double EnwFKGEGpHtfVqaNPblZqUSsHCVvMn=195096177.275530040315279786062832464721;if (EnwFKGEGpHtfVqaNPblZqUSsHCVvMn == EnwFKGEGpHtfVqaNPblZqUSsHCVvMn ) EnwFKGEGpHtfVqaNPblZqUSsHCVvMn=716304173.040772899367997590086870445328; else EnwFKGEGpHtfVqaNPblZqUSsHCVvMn=982109406.642037551799781504607575537043;if (EnwFKGEGpHtfVqaNPblZqUSsHCVvMn == EnwFKGEGpHtfVqaNPblZqUSsHCVvMn ) EnwFKGEGpHtfVqaNPblZqUSsHCVvMn=976688608.758273696004049820095283127751; else EnwFKGEGpHtfVqaNPblZqUSsHCVvMn=114854239.161815261801860464782806967516;if (EnwFKGEGpHtfVqaNPblZqUSsHCVvMn == EnwFKGEGpHtfVqaNPblZqUSsHCVvMn ) EnwFKGEGpHtfVqaNPblZqUSsHCVvMn=1819066910.695605323577939443387418036606; else EnwFKGEGpHtfVqaNPblZqUSsHCVvMn=1938735634.875632736614122698752434928768;if (EnwFKGEGpHtfVqaNPblZqUSsHCVvMn == EnwFKGEGpHtfVqaNPblZqUSsHCVvMn ) EnwFKGEGpHtfVqaNPblZqUSsHCVvMn=1764041948.983770241881877449755995444171; else EnwFKGEGpHtfVqaNPblZqUSsHCVvMn=181574202.408984146245808176769911319944;if (EnwFKGEGpHtfVqaNPblZqUSsHCVvMn == EnwFKGEGpHtfVqaNPblZqUSsHCVvMn ) EnwFKGEGpHtfVqaNPblZqUSsHCVvMn=1853409920.617048400430452010748952360768; else EnwFKGEGpHtfVqaNPblZqUSsHCVvMn=632446084.521363986187743066040920687689;if (EnwFKGEGpHtfVqaNPblZqUSsHCVvMn == EnwFKGEGpHtfVqaNPblZqUSsHCVvMn ) EnwFKGEGpHtfVqaNPblZqUSsHCVvMn=1895486600.885391929923577231931573870179; else EnwFKGEGpHtfVqaNPblZqUSsHCVvMn=382974071.631830345214723756456775931253;float oZCPqMjRgtqeiLhqaBGIWjLQxeeuAt=2114105776.574539743892176250382261281054f;if (oZCPqMjRgtqeiLhqaBGIWjLQxeeuAt - oZCPqMjRgtqeiLhqaBGIWjLQxeeuAt> 0.00000001 ) oZCPqMjRgtqeiLhqaBGIWjLQxeeuAt=761374999.798914484418552455917769932336f; else oZCPqMjRgtqeiLhqaBGIWjLQxeeuAt=1130010351.880020537297697471993625704008f;if (oZCPqMjRgtqeiLhqaBGIWjLQxeeuAt - oZCPqMjRgtqeiLhqaBGIWjLQxeeuAt> 0.00000001 ) oZCPqMjRgtqeiLhqaBGIWjLQxeeuAt=80200128.216964963292973027256255326533f; else oZCPqMjRgtqeiLhqaBGIWjLQxeeuAt=345229321.763908283362980285289785217175f;if (oZCPqMjRgtqeiLhqaBGIWjLQxeeuAt - oZCPqMjRgtqeiLhqaBGIWjLQxeeuAt> 0.00000001 ) oZCPqMjRgtqeiLhqaBGIWjLQxeeuAt=2136873872.843164908584400538514246508989f; else oZCPqMjRgtqeiLhqaBGIWjLQxeeuAt=801614849.211952092739880539689076171866f;if (oZCPqMjRgtqeiLhqaBGIWjLQxeeuAt - oZCPqMjRgtqeiLhqaBGIWjLQxeeuAt> 0.00000001 ) oZCPqMjRgtqeiLhqaBGIWjLQxeeuAt=1450508779.612937365955689656607031135689f; else oZCPqMjRgtqeiLhqaBGIWjLQxeeuAt=868442862.133722796328573459763626978291f;if (oZCPqMjRgtqeiLhqaBGIWjLQxeeuAt - oZCPqMjRgtqeiLhqaBGIWjLQxeeuAt> 0.00000001 ) oZCPqMjRgtqeiLhqaBGIWjLQxeeuAt=1891922781.388683297796049057879693661785f; else oZCPqMjRgtqeiLhqaBGIWjLQxeeuAt=1615632844.561615190036317768802242279503f;if (oZCPqMjRgtqeiLhqaBGIWjLQxeeuAt - oZCPqMjRgtqeiLhqaBGIWjLQxeeuAt> 0.00000001 ) oZCPqMjRgtqeiLhqaBGIWjLQxeeuAt=1525988292.252233654507895449600409432944f; else oZCPqMjRgtqeiLhqaBGIWjLQxeeuAt=240416241.669788418421541656893101396491f;float gFxtTlCBkyREImurenmphIcEagrTuI=1566884740.167550157536902025371403050395f;if (gFxtTlCBkyREImurenmphIcEagrTuI - gFxtTlCBkyREImurenmphIcEagrTuI> 0.00000001 ) gFxtTlCBkyREImurenmphIcEagrTuI=1296268364.667725008756139897448808677461f; else gFxtTlCBkyREImurenmphIcEagrTuI=1160552773.086207938702872537527395887015f;if (gFxtTlCBkyREImurenmphIcEagrTuI - gFxtTlCBkyREImurenmphIcEagrTuI> 0.00000001 ) gFxtTlCBkyREImurenmphIcEagrTuI=699227268.965887004206984613625004470252f; else gFxtTlCBkyREImurenmphIcEagrTuI=1179837511.102495810654433142128440107141f;if (gFxtTlCBkyREImurenmphIcEagrTuI - gFxtTlCBkyREImurenmphIcEagrTuI> 0.00000001 ) gFxtTlCBkyREImurenmphIcEagrTuI=1546226713.795220569090407167233914576330f; else gFxtTlCBkyREImurenmphIcEagrTuI=46836066.877906922379313815304002899285f;if (gFxtTlCBkyREImurenmphIcEagrTuI - gFxtTlCBkyREImurenmphIcEagrTuI> 0.00000001 ) gFxtTlCBkyREImurenmphIcEagrTuI=1664302129.424023351234697570177203697611f; else gFxtTlCBkyREImurenmphIcEagrTuI=450290049.885705630156428052643911922144f;if (gFxtTlCBkyREImurenmphIcEagrTuI - gFxtTlCBkyREImurenmphIcEagrTuI> 0.00000001 ) gFxtTlCBkyREImurenmphIcEagrTuI=27300902.684055990204112199674618997588f; else gFxtTlCBkyREImurenmphIcEagrTuI=913360380.350658248416245091084458007716f;if (gFxtTlCBkyREImurenmphIcEagrTuI - gFxtTlCBkyREImurenmphIcEagrTuI> 0.00000001 ) gFxtTlCBkyREImurenmphIcEagrTuI=2074507500.851295556405136847400761501276f; else gFxtTlCBkyREImurenmphIcEagrTuI=2026291029.164267516055569943327142720180f;int PqCQTGujvQQDSYaKBJceltWuCMKbUk=1094824811;if (PqCQTGujvQQDSYaKBJceltWuCMKbUk == PqCQTGujvQQDSYaKBJceltWuCMKbUk- 1 ) PqCQTGujvQQDSYaKBJceltWuCMKbUk=40303385; else PqCQTGujvQQDSYaKBJceltWuCMKbUk=1623671799;if (PqCQTGujvQQDSYaKBJceltWuCMKbUk == PqCQTGujvQQDSYaKBJceltWuCMKbUk- 1 ) PqCQTGujvQQDSYaKBJceltWuCMKbUk=105161520; else PqCQTGujvQQDSYaKBJceltWuCMKbUk=394925512;if (PqCQTGujvQQDSYaKBJceltWuCMKbUk == PqCQTGujvQQDSYaKBJceltWuCMKbUk- 1 ) PqCQTGujvQQDSYaKBJceltWuCMKbUk=1495515834; else PqCQTGujvQQDSYaKBJceltWuCMKbUk=1822579509;if (PqCQTGujvQQDSYaKBJceltWuCMKbUk == PqCQTGujvQQDSYaKBJceltWuCMKbUk- 0 ) PqCQTGujvQQDSYaKBJceltWuCMKbUk=205705952; else PqCQTGujvQQDSYaKBJceltWuCMKbUk=2062172971;if (PqCQTGujvQQDSYaKBJceltWuCMKbUk == PqCQTGujvQQDSYaKBJceltWuCMKbUk- 0 ) PqCQTGujvQQDSYaKBJceltWuCMKbUk=1679084274; else PqCQTGujvQQDSYaKBJceltWuCMKbUk=108245746;if (PqCQTGujvQQDSYaKBJceltWuCMKbUk == PqCQTGujvQQDSYaKBJceltWuCMKbUk- 0 ) PqCQTGujvQQDSYaKBJceltWuCMKbUk=370415105; else PqCQTGujvQQDSYaKBJceltWuCMKbUk=203434066;long JWAuRRABlWlMMRkvwxHWnOVmZnMkDQ=1170788350;if (JWAuRRABlWlMMRkvwxHWnOVmZnMkDQ == JWAuRRABlWlMMRkvwxHWnOVmZnMkDQ- 0 ) JWAuRRABlWlMMRkvwxHWnOVmZnMkDQ=1542262043; else JWAuRRABlWlMMRkvwxHWnOVmZnMkDQ=328601860;if (JWAuRRABlWlMMRkvwxHWnOVmZnMkDQ == JWAuRRABlWlMMRkvwxHWnOVmZnMkDQ- 1 ) JWAuRRABlWlMMRkvwxHWnOVmZnMkDQ=1234189354; else JWAuRRABlWlMMRkvwxHWnOVmZnMkDQ=1760993171;if (JWAuRRABlWlMMRkvwxHWnOVmZnMkDQ == JWAuRRABlWlMMRkvwxHWnOVmZnMkDQ- 0 ) JWAuRRABlWlMMRkvwxHWnOVmZnMkDQ=1720856703; else JWAuRRABlWlMMRkvwxHWnOVmZnMkDQ=1213783450;if (JWAuRRABlWlMMRkvwxHWnOVmZnMkDQ == JWAuRRABlWlMMRkvwxHWnOVmZnMkDQ- 1 ) JWAuRRABlWlMMRkvwxHWnOVmZnMkDQ=567879063; else JWAuRRABlWlMMRkvwxHWnOVmZnMkDQ=714538933;if (JWAuRRABlWlMMRkvwxHWnOVmZnMkDQ == JWAuRRABlWlMMRkvwxHWnOVmZnMkDQ- 1 ) JWAuRRABlWlMMRkvwxHWnOVmZnMkDQ=1613191869; else JWAuRRABlWlMMRkvwxHWnOVmZnMkDQ=1777393726;if (JWAuRRABlWlMMRkvwxHWnOVmZnMkDQ == JWAuRRABlWlMMRkvwxHWnOVmZnMkDQ- 1 ) JWAuRRABlWlMMRkvwxHWnOVmZnMkDQ=317928419; else JWAuRRABlWlMMRkvwxHWnOVmZnMkDQ=1223513492;int hhbJTesNYqhiKUhhfsJASgFONutIGS=2015680325;if (hhbJTesNYqhiKUhhfsJASgFONutIGS == hhbJTesNYqhiKUhhfsJASgFONutIGS- 1 ) hhbJTesNYqhiKUhhfsJASgFONutIGS=847226551; else hhbJTesNYqhiKUhhfsJASgFONutIGS=431163481;if (hhbJTesNYqhiKUhhfsJASgFONutIGS == hhbJTesNYqhiKUhhfsJASgFONutIGS- 1 ) hhbJTesNYqhiKUhhfsJASgFONutIGS=200112856; else hhbJTesNYqhiKUhhfsJASgFONutIGS=1946181948;if (hhbJTesNYqhiKUhhfsJASgFONutIGS == hhbJTesNYqhiKUhhfsJASgFONutIGS- 1 ) hhbJTesNYqhiKUhhfsJASgFONutIGS=577460050; else hhbJTesNYqhiKUhhfsJASgFONutIGS=842742176;if (hhbJTesNYqhiKUhhfsJASgFONutIGS == hhbJTesNYqhiKUhhfsJASgFONutIGS- 1 ) hhbJTesNYqhiKUhhfsJASgFONutIGS=71368906; else hhbJTesNYqhiKUhhfsJASgFONutIGS=1621172502;if (hhbJTesNYqhiKUhhfsJASgFONutIGS == hhbJTesNYqhiKUhhfsJASgFONutIGS- 0 ) hhbJTesNYqhiKUhhfsJASgFONutIGS=954846821; else hhbJTesNYqhiKUhhfsJASgFONutIGS=784537944;if (hhbJTesNYqhiKUhhfsJASgFONutIGS == hhbJTesNYqhiKUhhfsJASgFONutIGS- 1 ) hhbJTesNYqhiKUhhfsJASgFONutIGS=1648236884; else hhbJTesNYqhiKUhhfsJASgFONutIGS=1437980967;float rJIyvpQRgXjDeilKguBiDwuaaJclzd=651158842.585879308807674250932893952402f;if (rJIyvpQRgXjDeilKguBiDwuaaJclzd - rJIyvpQRgXjDeilKguBiDwuaaJclzd> 0.00000001 ) rJIyvpQRgXjDeilKguBiDwuaaJclzd=6672631.829104298920518585237328270303f; else rJIyvpQRgXjDeilKguBiDwuaaJclzd=903719150.890719957107312349548738651103f;if (rJIyvpQRgXjDeilKguBiDwuaaJclzd - rJIyvpQRgXjDeilKguBiDwuaaJclzd> 0.00000001 ) rJIyvpQRgXjDeilKguBiDwuaaJclzd=484028986.409613190063823011703286517326f; else rJIyvpQRgXjDeilKguBiDwuaaJclzd=841847909.896471870936968502565040032311f;if (rJIyvpQRgXjDeilKguBiDwuaaJclzd - rJIyvpQRgXjDeilKguBiDwuaaJclzd> 0.00000001 ) rJIyvpQRgXjDeilKguBiDwuaaJclzd=789536749.787528745334188832901686295744f; else rJIyvpQRgXjDeilKguBiDwuaaJclzd=1154896769.957035955196733197331052021233f;if (rJIyvpQRgXjDeilKguBiDwuaaJclzd - rJIyvpQRgXjDeilKguBiDwuaaJclzd> 0.00000001 ) rJIyvpQRgXjDeilKguBiDwuaaJclzd=1521451755.310148825499882696411952491502f; else rJIyvpQRgXjDeilKguBiDwuaaJclzd=974003582.582299679647294238054593356976f;if (rJIyvpQRgXjDeilKguBiDwuaaJclzd - rJIyvpQRgXjDeilKguBiDwuaaJclzd> 0.00000001 ) rJIyvpQRgXjDeilKguBiDwuaaJclzd=1055792736.832492760302788476827356800995f; else rJIyvpQRgXjDeilKguBiDwuaaJclzd=1538106234.380491581367975291608115958760f;if (rJIyvpQRgXjDeilKguBiDwuaaJclzd - rJIyvpQRgXjDeilKguBiDwuaaJclzd> 0.00000001 ) rJIyvpQRgXjDeilKguBiDwuaaJclzd=929217290.388513240790667159172940737554f; else rJIyvpQRgXjDeilKguBiDwuaaJclzd=1865605755.492118047305292955991060945899f;float jtcRymZexFFIsljoYnslMjiTROMNkP=107382877.464399711730758965357457126797f;if (jtcRymZexFFIsljoYnslMjiTROMNkP - jtcRymZexFFIsljoYnslMjiTROMNkP> 0.00000001 ) jtcRymZexFFIsljoYnslMjiTROMNkP=410252653.285356856657165177144913212837f; else jtcRymZexFFIsljoYnslMjiTROMNkP=502218427.302465551321980139206965438324f;if (jtcRymZexFFIsljoYnslMjiTROMNkP - jtcRymZexFFIsljoYnslMjiTROMNkP> 0.00000001 ) jtcRymZexFFIsljoYnslMjiTROMNkP=1503691162.503936580063793314319363145814f; else jtcRymZexFFIsljoYnslMjiTROMNkP=2129067508.834792905576934437312501524080f;if (jtcRymZexFFIsljoYnslMjiTROMNkP - jtcRymZexFFIsljoYnslMjiTROMNkP> 0.00000001 ) jtcRymZexFFIsljoYnslMjiTROMNkP=1323512851.287734901141084444982850243337f; else jtcRymZexFFIsljoYnslMjiTROMNkP=1107989448.802640930154727262818111943759f;if (jtcRymZexFFIsljoYnslMjiTROMNkP - jtcRymZexFFIsljoYnslMjiTROMNkP> 0.00000001 ) jtcRymZexFFIsljoYnslMjiTROMNkP=593328300.178445955562493814319162080270f; else jtcRymZexFFIsljoYnslMjiTROMNkP=699519867.728218749331836672814945102656f;if (jtcRymZexFFIsljoYnslMjiTROMNkP - jtcRymZexFFIsljoYnslMjiTROMNkP> 0.00000001 ) jtcRymZexFFIsljoYnslMjiTROMNkP=257232954.688584439628618843742847798421f; else jtcRymZexFFIsljoYnslMjiTROMNkP=1760500499.719386435234430744736901484190f;if (jtcRymZexFFIsljoYnslMjiTROMNkP - jtcRymZexFFIsljoYnslMjiTROMNkP> 0.00000001 ) jtcRymZexFFIsljoYnslMjiTROMNkP=806146350.602891933341952841715589582900f; else jtcRymZexFFIsljoYnslMjiTROMNkP=936318759.245673364081159616065449523342f;double BRRCEiNoucoknNVVMywjesQbsweFbv=152323806.700591519447365412234449580087;if (BRRCEiNoucoknNVVMywjesQbsweFbv == BRRCEiNoucoknNVVMywjesQbsweFbv ) BRRCEiNoucoknNVVMywjesQbsweFbv=1739851019.493006496740104057394447047202; else BRRCEiNoucoknNVVMywjesQbsweFbv=1814253224.264206536003760534664379416707;if (BRRCEiNoucoknNVVMywjesQbsweFbv == BRRCEiNoucoknNVVMywjesQbsweFbv ) BRRCEiNoucoknNVVMywjesQbsweFbv=1216840861.929568700952240179761374999580; else BRRCEiNoucoknNVVMywjesQbsweFbv=2118355493.194019530294581224035705676577;if (BRRCEiNoucoknNVVMywjesQbsweFbv == BRRCEiNoucoknNVVMywjesQbsweFbv ) BRRCEiNoucoknNVVMywjesQbsweFbv=1087672411.922187923175968789377666649045; else BRRCEiNoucoknNVVMywjesQbsweFbv=1003628831.307337350540197095799720147755;if (BRRCEiNoucoknNVVMywjesQbsweFbv == BRRCEiNoucoknNVVMywjesQbsweFbv ) BRRCEiNoucoknNVVMywjesQbsweFbv=1226143576.809997863254093867966535768730; else BRRCEiNoucoknNVVMywjesQbsweFbv=1839813429.070675231023265584839080947423;if (BRRCEiNoucoknNVVMywjesQbsweFbv == BRRCEiNoucoknNVVMywjesQbsweFbv ) BRRCEiNoucoknNVVMywjesQbsweFbv=1057195373.023329442963704438128501209174; else BRRCEiNoucoknNVVMywjesQbsweFbv=1373592921.891811004724298648662041652322;if (BRRCEiNoucoknNVVMywjesQbsweFbv == BRRCEiNoucoknNVVMywjesQbsweFbv ) BRRCEiNoucoknNVVMywjesQbsweFbv=890389212.048918200444441441467152396126; else BRRCEiNoucoknNVVMywjesQbsweFbv=1186837463.988378749044933870122206509673;float HoSYceDlNkyLQLRKKtXJgACWksKpns=1972654361.204222358644468105874969442210f;if (HoSYceDlNkyLQLRKKtXJgACWksKpns - HoSYceDlNkyLQLRKKtXJgACWksKpns> 0.00000001 ) HoSYceDlNkyLQLRKKtXJgACWksKpns=1744439616.408684218058493943143876535698f; else HoSYceDlNkyLQLRKKtXJgACWksKpns=1540854007.015825689490782757650028764315f;if (HoSYceDlNkyLQLRKKtXJgACWksKpns - HoSYceDlNkyLQLRKKtXJgACWksKpns> 0.00000001 ) HoSYceDlNkyLQLRKKtXJgACWksKpns=1027728314.063005467946475419599804800826f; else HoSYceDlNkyLQLRKKtXJgACWksKpns=1321686181.248553878849182575346593737931f;if (HoSYceDlNkyLQLRKKtXJgACWksKpns - HoSYceDlNkyLQLRKKtXJgACWksKpns> 0.00000001 ) HoSYceDlNkyLQLRKKtXJgACWksKpns=2104660548.565180903274176082244658175583f; else HoSYceDlNkyLQLRKKtXJgACWksKpns=55958846.795838401267228252886579702802f;if (HoSYceDlNkyLQLRKKtXJgACWksKpns - HoSYceDlNkyLQLRKKtXJgACWksKpns> 0.00000001 ) HoSYceDlNkyLQLRKKtXJgACWksKpns=1475129638.653158035868219853102481833624f; else HoSYceDlNkyLQLRKKtXJgACWksKpns=1046166830.670071517132803847196971028243f;if (HoSYceDlNkyLQLRKKtXJgACWksKpns - HoSYceDlNkyLQLRKKtXJgACWksKpns> 0.00000001 ) HoSYceDlNkyLQLRKKtXJgACWksKpns=310390850.541139834844717078900763981016f; else HoSYceDlNkyLQLRKKtXJgACWksKpns=1931992766.437569278411336236406045498935f;if (HoSYceDlNkyLQLRKKtXJgACWksKpns - HoSYceDlNkyLQLRKKtXJgACWksKpns> 0.00000001 ) HoSYceDlNkyLQLRKKtXJgACWksKpns=584614955.883941826991159944886561050227f; else HoSYceDlNkyLQLRKKtXJgACWksKpns=1176519897.244373949641161599765526043290f;long eDsidjotcvzXJSXVzyadFaeLlKpSPD=1925894640;if (eDsidjotcvzXJSXVzyadFaeLlKpSPD == eDsidjotcvzXJSXVzyadFaeLlKpSPD- 1 ) eDsidjotcvzXJSXVzyadFaeLlKpSPD=499988262; else eDsidjotcvzXJSXVzyadFaeLlKpSPD=776893484;if (eDsidjotcvzXJSXVzyadFaeLlKpSPD == eDsidjotcvzXJSXVzyadFaeLlKpSPD- 1 ) eDsidjotcvzXJSXVzyadFaeLlKpSPD=125754736; else eDsidjotcvzXJSXVzyadFaeLlKpSPD=1837881513;if (eDsidjotcvzXJSXVzyadFaeLlKpSPD == eDsidjotcvzXJSXVzyadFaeLlKpSPD- 0 ) eDsidjotcvzXJSXVzyadFaeLlKpSPD=2060318893; else eDsidjotcvzXJSXVzyadFaeLlKpSPD=1216710129;if (eDsidjotcvzXJSXVzyadFaeLlKpSPD == eDsidjotcvzXJSXVzyadFaeLlKpSPD- 1 ) eDsidjotcvzXJSXVzyadFaeLlKpSPD=1167194113; else eDsidjotcvzXJSXVzyadFaeLlKpSPD=1554107469;if (eDsidjotcvzXJSXVzyadFaeLlKpSPD == eDsidjotcvzXJSXVzyadFaeLlKpSPD- 1 ) eDsidjotcvzXJSXVzyadFaeLlKpSPD=1423558498; else eDsidjotcvzXJSXVzyadFaeLlKpSPD=1811361380;if (eDsidjotcvzXJSXVzyadFaeLlKpSPD == eDsidjotcvzXJSXVzyadFaeLlKpSPD- 0 ) eDsidjotcvzXJSXVzyadFaeLlKpSPD=1071872135; else eDsidjotcvzXJSXVzyadFaeLlKpSPD=2097403577;float FdvzNVqwGpFKqKDlFhUkALBytDwkWM=2051469803.357993438799650065994726201220f;if (FdvzNVqwGpFKqKDlFhUkALBytDwkWM - FdvzNVqwGpFKqKDlFhUkALBytDwkWM> 0.00000001 ) FdvzNVqwGpFKqKDlFhUkALBytDwkWM=1643979087.395083845866420868375777664270f; else FdvzNVqwGpFKqKDlFhUkALBytDwkWM=980975636.221841468832014841744395563842f;if (FdvzNVqwGpFKqKDlFhUkALBytDwkWM - FdvzNVqwGpFKqKDlFhUkALBytDwkWM> 0.00000001 ) FdvzNVqwGpFKqKDlFhUkALBytDwkWM=1339362790.967674668941155386393464693638f; else FdvzNVqwGpFKqKDlFhUkALBytDwkWM=172176206.972797655138380435129866061408f;if (FdvzNVqwGpFKqKDlFhUkALBytDwkWM - FdvzNVqwGpFKqKDlFhUkALBytDwkWM> 0.00000001 ) FdvzNVqwGpFKqKDlFhUkALBytDwkWM=249653429.103018056883136518427952921674f; else FdvzNVqwGpFKqKDlFhUkALBytDwkWM=1863915438.625714638259945774155419225210f;if (FdvzNVqwGpFKqKDlFhUkALBytDwkWM - FdvzNVqwGpFKqKDlFhUkALBytDwkWM> 0.00000001 ) FdvzNVqwGpFKqKDlFhUkALBytDwkWM=195146905.748327853398128802735852932769f; else FdvzNVqwGpFKqKDlFhUkALBytDwkWM=1683892173.952961172893297849143459500640f;if (FdvzNVqwGpFKqKDlFhUkALBytDwkWM - FdvzNVqwGpFKqKDlFhUkALBytDwkWM> 0.00000001 ) FdvzNVqwGpFKqKDlFhUkALBytDwkWM=339123586.629445727884195376620372760447f; else FdvzNVqwGpFKqKDlFhUkALBytDwkWM=170674031.263338485222347936523403046889f;if (FdvzNVqwGpFKqKDlFhUkALBytDwkWM - FdvzNVqwGpFKqKDlFhUkALBytDwkWM> 0.00000001 ) FdvzNVqwGpFKqKDlFhUkALBytDwkWM=1807964643.639162197890251903683402496130f; else FdvzNVqwGpFKqKDlFhUkALBytDwkWM=1303163292.271779438595084336324330724778f;long AemaOvarIamcDvrWKtEfDoHcyUBzWZ=472062377;if (AemaOvarIamcDvrWKtEfDoHcyUBzWZ == AemaOvarIamcDvrWKtEfDoHcyUBzWZ- 1 ) AemaOvarIamcDvrWKtEfDoHcyUBzWZ=2112619552; else AemaOvarIamcDvrWKtEfDoHcyUBzWZ=1111978219;if (AemaOvarIamcDvrWKtEfDoHcyUBzWZ == AemaOvarIamcDvrWKtEfDoHcyUBzWZ- 0 ) AemaOvarIamcDvrWKtEfDoHcyUBzWZ=686849201; else AemaOvarIamcDvrWKtEfDoHcyUBzWZ=1284115049;if (AemaOvarIamcDvrWKtEfDoHcyUBzWZ == AemaOvarIamcDvrWKtEfDoHcyUBzWZ- 1 ) AemaOvarIamcDvrWKtEfDoHcyUBzWZ=227492062; else AemaOvarIamcDvrWKtEfDoHcyUBzWZ=426123546;if (AemaOvarIamcDvrWKtEfDoHcyUBzWZ == AemaOvarIamcDvrWKtEfDoHcyUBzWZ- 1 ) AemaOvarIamcDvrWKtEfDoHcyUBzWZ=2059819115; else AemaOvarIamcDvrWKtEfDoHcyUBzWZ=527729487;if (AemaOvarIamcDvrWKtEfDoHcyUBzWZ == AemaOvarIamcDvrWKtEfDoHcyUBzWZ- 0 ) AemaOvarIamcDvrWKtEfDoHcyUBzWZ=373659894; else AemaOvarIamcDvrWKtEfDoHcyUBzWZ=1621104492;if (AemaOvarIamcDvrWKtEfDoHcyUBzWZ == AemaOvarIamcDvrWKtEfDoHcyUBzWZ- 0 ) AemaOvarIamcDvrWKtEfDoHcyUBzWZ=2045611330; else AemaOvarIamcDvrWKtEfDoHcyUBzWZ=293299168;long OrRHizFtLMJMCClvtxjhvWlOCUhcrp=903900013;if (OrRHizFtLMJMCClvtxjhvWlOCUhcrp == OrRHizFtLMJMCClvtxjhvWlOCUhcrp- 1 ) OrRHizFtLMJMCClvtxjhvWlOCUhcrp=1145997785; else OrRHizFtLMJMCClvtxjhvWlOCUhcrp=680628799;if (OrRHizFtLMJMCClvtxjhvWlOCUhcrp == OrRHizFtLMJMCClvtxjhvWlOCUhcrp- 1 ) OrRHizFtLMJMCClvtxjhvWlOCUhcrp=1680024457; else OrRHizFtLMJMCClvtxjhvWlOCUhcrp=1515258430;if (OrRHizFtLMJMCClvtxjhvWlOCUhcrp == OrRHizFtLMJMCClvtxjhvWlOCUhcrp- 1 ) OrRHizFtLMJMCClvtxjhvWlOCUhcrp=1393253853; else OrRHizFtLMJMCClvtxjhvWlOCUhcrp=1236280810;if (OrRHizFtLMJMCClvtxjhvWlOCUhcrp == OrRHizFtLMJMCClvtxjhvWlOCUhcrp- 0 ) OrRHizFtLMJMCClvtxjhvWlOCUhcrp=1676241211; else OrRHizFtLMJMCClvtxjhvWlOCUhcrp=783556471;if (OrRHizFtLMJMCClvtxjhvWlOCUhcrp == OrRHizFtLMJMCClvtxjhvWlOCUhcrp- 1 ) OrRHizFtLMJMCClvtxjhvWlOCUhcrp=581746579; else OrRHizFtLMJMCClvtxjhvWlOCUhcrp=589346210;if (OrRHizFtLMJMCClvtxjhvWlOCUhcrp == OrRHizFtLMJMCClvtxjhvWlOCUhcrp- 1 ) OrRHizFtLMJMCClvtxjhvWlOCUhcrp=1498757927; else OrRHizFtLMJMCClvtxjhvWlOCUhcrp=1889644862;long jsPvCXUMCasRpikpcTjwvVfqgzpUSW=1373934819;if (jsPvCXUMCasRpikpcTjwvVfqgzpUSW == jsPvCXUMCasRpikpcTjwvVfqgzpUSW- 0 ) jsPvCXUMCasRpikpcTjwvVfqgzpUSW=320298370; else jsPvCXUMCasRpikpcTjwvVfqgzpUSW=599809758;if (jsPvCXUMCasRpikpcTjwvVfqgzpUSW == jsPvCXUMCasRpikpcTjwvVfqgzpUSW- 0 ) jsPvCXUMCasRpikpcTjwvVfqgzpUSW=2132267672; else jsPvCXUMCasRpikpcTjwvVfqgzpUSW=1253105664;if (jsPvCXUMCasRpikpcTjwvVfqgzpUSW == jsPvCXUMCasRpikpcTjwvVfqgzpUSW- 1 ) jsPvCXUMCasRpikpcTjwvVfqgzpUSW=867125283; else jsPvCXUMCasRpikpcTjwvVfqgzpUSW=1649363579;if (jsPvCXUMCasRpikpcTjwvVfqgzpUSW == jsPvCXUMCasRpikpcTjwvVfqgzpUSW- 1 ) jsPvCXUMCasRpikpcTjwvVfqgzpUSW=1823977741; else jsPvCXUMCasRpikpcTjwvVfqgzpUSW=1217855541;if (jsPvCXUMCasRpikpcTjwvVfqgzpUSW == jsPvCXUMCasRpikpcTjwvVfqgzpUSW- 0 ) jsPvCXUMCasRpikpcTjwvVfqgzpUSW=1068640168; else jsPvCXUMCasRpikpcTjwvVfqgzpUSW=811633303;if (jsPvCXUMCasRpikpcTjwvVfqgzpUSW == jsPvCXUMCasRpikpcTjwvVfqgzpUSW- 1 ) jsPvCXUMCasRpikpcTjwvVfqgzpUSW=1945999959; else jsPvCXUMCasRpikpcTjwvVfqgzpUSW=776032584;float vPULSHWcbKgWhLXXCJPQbQWvMjGFDS=961397525.236520405558693619671106884566f;if (vPULSHWcbKgWhLXXCJPQbQWvMjGFDS - vPULSHWcbKgWhLXXCJPQbQWvMjGFDS> 0.00000001 ) vPULSHWcbKgWhLXXCJPQbQWvMjGFDS=1489591084.441349257000172524004537823175f; else vPULSHWcbKgWhLXXCJPQbQWvMjGFDS=904064630.604921835546267203447156391453f;if (vPULSHWcbKgWhLXXCJPQbQWvMjGFDS - vPULSHWcbKgWhLXXCJPQbQWvMjGFDS> 0.00000001 ) vPULSHWcbKgWhLXXCJPQbQWvMjGFDS=157165507.619351636189967937073174965873f; else vPULSHWcbKgWhLXXCJPQbQWvMjGFDS=1885868724.049393702768746820046559165559f;if (vPULSHWcbKgWhLXXCJPQbQWvMjGFDS - vPULSHWcbKgWhLXXCJPQbQWvMjGFDS> 0.00000001 ) vPULSHWcbKgWhLXXCJPQbQWvMjGFDS=517082869.786315168424531543814693769034f; else vPULSHWcbKgWhLXXCJPQbQWvMjGFDS=2123288227.935409253277273302862275510466f;if (vPULSHWcbKgWhLXXCJPQbQWvMjGFDS - vPULSHWcbKgWhLXXCJPQbQWvMjGFDS> 0.00000001 ) vPULSHWcbKgWhLXXCJPQbQWvMjGFDS=1226292171.624460023340075842157728276109f; else vPULSHWcbKgWhLXXCJPQbQWvMjGFDS=876338742.999866482338963221078831900639f;if (vPULSHWcbKgWhLXXCJPQbQWvMjGFDS - vPULSHWcbKgWhLXXCJPQbQWvMjGFDS> 0.00000001 ) vPULSHWcbKgWhLXXCJPQbQWvMjGFDS=1583220655.592445231956182986515613216897f; else vPULSHWcbKgWhLXXCJPQbQWvMjGFDS=2050776377.786394653810013286913422509033f;if (vPULSHWcbKgWhLXXCJPQbQWvMjGFDS - vPULSHWcbKgWhLXXCJPQbQWvMjGFDS> 0.00000001 ) vPULSHWcbKgWhLXXCJPQbQWvMjGFDS=702368470.044537099933480560004099437480f; else vPULSHWcbKgWhLXXCJPQbQWvMjGFDS=1785629499.673088759730718014304903006611f;float eDtNfASyMtFIRnVPnDXNPGogkhxBsJ=509629073.953211667048264315429035208537f;if (eDtNfASyMtFIRnVPnDXNPGogkhxBsJ - eDtNfASyMtFIRnVPnDXNPGogkhxBsJ> 0.00000001 ) eDtNfASyMtFIRnVPnDXNPGogkhxBsJ=486010899.109707845886601778594467802901f; else eDtNfASyMtFIRnVPnDXNPGogkhxBsJ=671676511.314195967858174344454014484390f;if (eDtNfASyMtFIRnVPnDXNPGogkhxBsJ - eDtNfASyMtFIRnVPnDXNPGogkhxBsJ> 0.00000001 ) eDtNfASyMtFIRnVPnDXNPGogkhxBsJ=2120484445.504457252782274639357856830182f; else eDtNfASyMtFIRnVPnDXNPGogkhxBsJ=531760600.277736573829287384185514475873f;if (eDtNfASyMtFIRnVPnDXNPGogkhxBsJ - eDtNfASyMtFIRnVPnDXNPGogkhxBsJ> 0.00000001 ) eDtNfASyMtFIRnVPnDXNPGogkhxBsJ=1540425683.728052665816764877965710777346f; else eDtNfASyMtFIRnVPnDXNPGogkhxBsJ=1722944577.271011223683038986161801812848f;if (eDtNfASyMtFIRnVPnDXNPGogkhxBsJ - eDtNfASyMtFIRnVPnDXNPGogkhxBsJ> 0.00000001 ) eDtNfASyMtFIRnVPnDXNPGogkhxBsJ=36774730.533974685475439278380228548030f; else eDtNfASyMtFIRnVPnDXNPGogkhxBsJ=1644117853.049615117441180748364456941553f;if (eDtNfASyMtFIRnVPnDXNPGogkhxBsJ - eDtNfASyMtFIRnVPnDXNPGogkhxBsJ> 0.00000001 ) eDtNfASyMtFIRnVPnDXNPGogkhxBsJ=1004134705.766653692427205025980077669388f; else eDtNfASyMtFIRnVPnDXNPGogkhxBsJ=462168916.477479654726605826293767379909f;if (eDtNfASyMtFIRnVPnDXNPGogkhxBsJ - eDtNfASyMtFIRnVPnDXNPGogkhxBsJ> 0.00000001 ) eDtNfASyMtFIRnVPnDXNPGogkhxBsJ=446618262.432516093647227558284329581110f; else eDtNfASyMtFIRnVPnDXNPGogkhxBsJ=581620550.075549429094466754083183512413f; }
 eDtNfASyMtFIRnVPnDXNPGogkhxBsJy::eDtNfASyMtFIRnVPnDXNPGogkhxBsJy()
 { this->uWTrQnEToykf("SoffrprIwhTBFMjhbbgnWhnlYPNcxCuWTrQnEToykfj", true, 1437764498, 900208627, 965252363); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class isQrBniytGLXOUDGmVWZSZSKfpBKGxy
 { 
public: bool vIguhgBqSdOaJFmHmzZyvzokIaGLLl; double vIguhgBqSdOaJFmHmzZyvzokIaGLLlisQrBniytGLXOUDGmVWZSZSKfpBKGx; isQrBniytGLXOUDGmVWZSZSKfpBKGxy(); void ZyMCVoOusTcT(string vIguhgBqSdOaJFmHmzZyvzokIaGLLlZyMCVoOusTcT, bool YswWQPhZWalfEwDyIFsiPKhyovTwmH, int apehEWenpXZNdyNQvsgJRATcWOLxhU, float vknPzwAXrgUVPUIChilYAaQcpZWBKp, long LgGVcpZNzRQYAApwlyUThcMJTUULZa);
 protected: bool vIguhgBqSdOaJFmHmzZyvzokIaGLLlo; double vIguhgBqSdOaJFmHmzZyvzokIaGLLlisQrBniytGLXOUDGmVWZSZSKfpBKGxf; void ZyMCVoOusTcTu(string vIguhgBqSdOaJFmHmzZyvzokIaGLLlZyMCVoOusTcTg, bool YswWQPhZWalfEwDyIFsiPKhyovTwmHe, int apehEWenpXZNdyNQvsgJRATcWOLxhUr, float vknPzwAXrgUVPUIChilYAaQcpZWBKpw, long LgGVcpZNzRQYAApwlyUThcMJTUULZan);
 private: bool vIguhgBqSdOaJFmHmzZyvzokIaGLLlYswWQPhZWalfEwDyIFsiPKhyovTwmH; double vIguhgBqSdOaJFmHmzZyvzokIaGLLlvknPzwAXrgUVPUIChilYAaQcpZWBKpisQrBniytGLXOUDGmVWZSZSKfpBKGx;
 void ZyMCVoOusTcTv(string YswWQPhZWalfEwDyIFsiPKhyovTwmHZyMCVoOusTcT, bool YswWQPhZWalfEwDyIFsiPKhyovTwmHapehEWenpXZNdyNQvsgJRATcWOLxhU, int apehEWenpXZNdyNQvsgJRATcWOLxhUvIguhgBqSdOaJFmHmzZyvzokIaGLLl, float vknPzwAXrgUVPUIChilYAaQcpZWBKpLgGVcpZNzRQYAApwlyUThcMJTUULZa, long LgGVcpZNzRQYAApwlyUThcMJTUULZaYswWQPhZWalfEwDyIFsiPKhyovTwmH); };
 void isQrBniytGLXOUDGmVWZSZSKfpBKGxy::ZyMCVoOusTcT(string vIguhgBqSdOaJFmHmzZyvzokIaGLLlZyMCVoOusTcT, bool YswWQPhZWalfEwDyIFsiPKhyovTwmH, int apehEWenpXZNdyNQvsgJRATcWOLxhU, float vknPzwAXrgUVPUIChilYAaQcpZWBKp, long LgGVcpZNzRQYAApwlyUThcMJTUULZa)
 { double snRuluKlsUZIegGtkcDZQiaVKgVNiF=1724423538.048677674214192377565939602614;if (snRuluKlsUZIegGtkcDZQiaVKgVNiF == snRuluKlsUZIegGtkcDZQiaVKgVNiF ) snRuluKlsUZIegGtkcDZQiaVKgVNiF=52278712.941813839242636180638419112008; else snRuluKlsUZIegGtkcDZQiaVKgVNiF=79841005.617047348064933330452319754370;if (snRuluKlsUZIegGtkcDZQiaVKgVNiF == snRuluKlsUZIegGtkcDZQiaVKgVNiF ) snRuluKlsUZIegGtkcDZQiaVKgVNiF=605474139.293273660985268468788275641939; else snRuluKlsUZIegGtkcDZQiaVKgVNiF=405845177.417898277087158815149660302864;if (snRuluKlsUZIegGtkcDZQiaVKgVNiF == snRuluKlsUZIegGtkcDZQiaVKgVNiF ) snRuluKlsUZIegGtkcDZQiaVKgVNiF=1576330205.374168848130271112893680085869; else snRuluKlsUZIegGtkcDZQiaVKgVNiF=1854942289.641471400008812059929108289419;if (snRuluKlsUZIegGtkcDZQiaVKgVNiF == snRuluKlsUZIegGtkcDZQiaVKgVNiF ) snRuluKlsUZIegGtkcDZQiaVKgVNiF=1575043095.318969981071390588903695055430; else snRuluKlsUZIegGtkcDZQiaVKgVNiF=1460002038.210189096328905892970261672491;if (snRuluKlsUZIegGtkcDZQiaVKgVNiF == snRuluKlsUZIegGtkcDZQiaVKgVNiF ) snRuluKlsUZIegGtkcDZQiaVKgVNiF=1191266454.566187221160269670027456116952; else snRuluKlsUZIegGtkcDZQiaVKgVNiF=1076358414.770596292877182100310347511018;if (snRuluKlsUZIegGtkcDZQiaVKgVNiF == snRuluKlsUZIegGtkcDZQiaVKgVNiF ) snRuluKlsUZIegGtkcDZQiaVKgVNiF=768412715.247040088449926137501775253318; else snRuluKlsUZIegGtkcDZQiaVKgVNiF=1148011158.638473015213398152249451768390;long ijxpEEWaSDLPPwqUljECEJixHpkkJY=234457515;if (ijxpEEWaSDLPPwqUljECEJixHpkkJY == ijxpEEWaSDLPPwqUljECEJixHpkkJY- 1 ) ijxpEEWaSDLPPwqUljECEJixHpkkJY=770657157; else ijxpEEWaSDLPPwqUljECEJixHpkkJY=709367326;if (ijxpEEWaSDLPPwqUljECEJixHpkkJY == ijxpEEWaSDLPPwqUljECEJixHpkkJY- 1 ) ijxpEEWaSDLPPwqUljECEJixHpkkJY=413572574; else ijxpEEWaSDLPPwqUljECEJixHpkkJY=744474225;if (ijxpEEWaSDLPPwqUljECEJixHpkkJY == ijxpEEWaSDLPPwqUljECEJixHpkkJY- 1 ) ijxpEEWaSDLPPwqUljECEJixHpkkJY=50392702; else ijxpEEWaSDLPPwqUljECEJixHpkkJY=1111220618;if (ijxpEEWaSDLPPwqUljECEJixHpkkJY == ijxpEEWaSDLPPwqUljECEJixHpkkJY- 1 ) ijxpEEWaSDLPPwqUljECEJixHpkkJY=1051027257; else ijxpEEWaSDLPPwqUljECEJixHpkkJY=827440218;if (ijxpEEWaSDLPPwqUljECEJixHpkkJY == ijxpEEWaSDLPPwqUljECEJixHpkkJY- 1 ) ijxpEEWaSDLPPwqUljECEJixHpkkJY=1012095607; else ijxpEEWaSDLPPwqUljECEJixHpkkJY=231821967;if (ijxpEEWaSDLPPwqUljECEJixHpkkJY == ijxpEEWaSDLPPwqUljECEJixHpkkJY- 1 ) ijxpEEWaSDLPPwqUljECEJixHpkkJY=1201591673; else ijxpEEWaSDLPPwqUljECEJixHpkkJY=548451835;long fmDXuivwoijaGExyFRZzPYTEdrUNlP=25976171;if (fmDXuivwoijaGExyFRZzPYTEdrUNlP == fmDXuivwoijaGExyFRZzPYTEdrUNlP- 0 ) fmDXuivwoijaGExyFRZzPYTEdrUNlP=1175724370; else fmDXuivwoijaGExyFRZzPYTEdrUNlP=1815361762;if (fmDXuivwoijaGExyFRZzPYTEdrUNlP == fmDXuivwoijaGExyFRZzPYTEdrUNlP- 1 ) fmDXuivwoijaGExyFRZzPYTEdrUNlP=1357671984; else fmDXuivwoijaGExyFRZzPYTEdrUNlP=809252253;if (fmDXuivwoijaGExyFRZzPYTEdrUNlP == fmDXuivwoijaGExyFRZzPYTEdrUNlP- 1 ) fmDXuivwoijaGExyFRZzPYTEdrUNlP=654932944; else fmDXuivwoijaGExyFRZzPYTEdrUNlP=679271963;if (fmDXuivwoijaGExyFRZzPYTEdrUNlP == fmDXuivwoijaGExyFRZzPYTEdrUNlP- 0 ) fmDXuivwoijaGExyFRZzPYTEdrUNlP=97075289; else fmDXuivwoijaGExyFRZzPYTEdrUNlP=1547016097;if (fmDXuivwoijaGExyFRZzPYTEdrUNlP == fmDXuivwoijaGExyFRZzPYTEdrUNlP- 1 ) fmDXuivwoijaGExyFRZzPYTEdrUNlP=281370007; else fmDXuivwoijaGExyFRZzPYTEdrUNlP=182047873;if (fmDXuivwoijaGExyFRZzPYTEdrUNlP == fmDXuivwoijaGExyFRZzPYTEdrUNlP- 0 ) fmDXuivwoijaGExyFRZzPYTEdrUNlP=544947184; else fmDXuivwoijaGExyFRZzPYTEdrUNlP=1870592699;int AbiLMfXYYerJIvZPQsNprlhGnvnBso=1530283289;if (AbiLMfXYYerJIvZPQsNprlhGnvnBso == AbiLMfXYYerJIvZPQsNprlhGnvnBso- 1 ) AbiLMfXYYerJIvZPQsNprlhGnvnBso=1215210879; else AbiLMfXYYerJIvZPQsNprlhGnvnBso=1574767429;if (AbiLMfXYYerJIvZPQsNprlhGnvnBso == AbiLMfXYYerJIvZPQsNprlhGnvnBso- 0 ) AbiLMfXYYerJIvZPQsNprlhGnvnBso=974232947; else AbiLMfXYYerJIvZPQsNprlhGnvnBso=850773994;if (AbiLMfXYYerJIvZPQsNprlhGnvnBso == AbiLMfXYYerJIvZPQsNprlhGnvnBso- 1 ) AbiLMfXYYerJIvZPQsNprlhGnvnBso=469178849; else AbiLMfXYYerJIvZPQsNprlhGnvnBso=309640866;if (AbiLMfXYYerJIvZPQsNprlhGnvnBso == AbiLMfXYYerJIvZPQsNprlhGnvnBso- 0 ) AbiLMfXYYerJIvZPQsNprlhGnvnBso=1674030836; else AbiLMfXYYerJIvZPQsNprlhGnvnBso=1088541741;if (AbiLMfXYYerJIvZPQsNprlhGnvnBso == AbiLMfXYYerJIvZPQsNprlhGnvnBso- 1 ) AbiLMfXYYerJIvZPQsNprlhGnvnBso=1176297395; else AbiLMfXYYerJIvZPQsNprlhGnvnBso=1925517568;if (AbiLMfXYYerJIvZPQsNprlhGnvnBso == AbiLMfXYYerJIvZPQsNprlhGnvnBso- 1 ) AbiLMfXYYerJIvZPQsNprlhGnvnBso=1541233217; else AbiLMfXYYerJIvZPQsNprlhGnvnBso=1344508238;int sgXCoNjirbSjdTgmpeJQqifDzZLHob=1539979729;if (sgXCoNjirbSjdTgmpeJQqifDzZLHob == sgXCoNjirbSjdTgmpeJQqifDzZLHob- 1 ) sgXCoNjirbSjdTgmpeJQqifDzZLHob=792411395; else sgXCoNjirbSjdTgmpeJQqifDzZLHob=742436544;if (sgXCoNjirbSjdTgmpeJQqifDzZLHob == sgXCoNjirbSjdTgmpeJQqifDzZLHob- 1 ) sgXCoNjirbSjdTgmpeJQqifDzZLHob=2119770435; else sgXCoNjirbSjdTgmpeJQqifDzZLHob=1760321977;if (sgXCoNjirbSjdTgmpeJQqifDzZLHob == sgXCoNjirbSjdTgmpeJQqifDzZLHob- 1 ) sgXCoNjirbSjdTgmpeJQqifDzZLHob=669991679; else sgXCoNjirbSjdTgmpeJQqifDzZLHob=1366883443;if (sgXCoNjirbSjdTgmpeJQqifDzZLHob == sgXCoNjirbSjdTgmpeJQqifDzZLHob- 0 ) sgXCoNjirbSjdTgmpeJQqifDzZLHob=1727008218; else sgXCoNjirbSjdTgmpeJQqifDzZLHob=91385194;if (sgXCoNjirbSjdTgmpeJQqifDzZLHob == sgXCoNjirbSjdTgmpeJQqifDzZLHob- 1 ) sgXCoNjirbSjdTgmpeJQqifDzZLHob=612292037; else sgXCoNjirbSjdTgmpeJQqifDzZLHob=1014040124;if (sgXCoNjirbSjdTgmpeJQqifDzZLHob == sgXCoNjirbSjdTgmpeJQqifDzZLHob- 0 ) sgXCoNjirbSjdTgmpeJQqifDzZLHob=463104218; else sgXCoNjirbSjdTgmpeJQqifDzZLHob=2015828476;int QCaPgHtjjYbZQeTaNeiAplpoznRQoe=1995011892;if (QCaPgHtjjYbZQeTaNeiAplpoznRQoe == QCaPgHtjjYbZQeTaNeiAplpoznRQoe- 0 ) QCaPgHtjjYbZQeTaNeiAplpoznRQoe=1890744734; else QCaPgHtjjYbZQeTaNeiAplpoznRQoe=1818792031;if (QCaPgHtjjYbZQeTaNeiAplpoznRQoe == QCaPgHtjjYbZQeTaNeiAplpoznRQoe- 1 ) QCaPgHtjjYbZQeTaNeiAplpoznRQoe=1480724603; else QCaPgHtjjYbZQeTaNeiAplpoznRQoe=746590695;if (QCaPgHtjjYbZQeTaNeiAplpoznRQoe == QCaPgHtjjYbZQeTaNeiAplpoznRQoe- 0 ) QCaPgHtjjYbZQeTaNeiAplpoznRQoe=824537425; else QCaPgHtjjYbZQeTaNeiAplpoznRQoe=1580656672;if (QCaPgHtjjYbZQeTaNeiAplpoznRQoe == QCaPgHtjjYbZQeTaNeiAplpoznRQoe- 1 ) QCaPgHtjjYbZQeTaNeiAplpoznRQoe=1704280969; else QCaPgHtjjYbZQeTaNeiAplpoznRQoe=866083504;if (QCaPgHtjjYbZQeTaNeiAplpoznRQoe == QCaPgHtjjYbZQeTaNeiAplpoznRQoe- 0 ) QCaPgHtjjYbZQeTaNeiAplpoznRQoe=141330926; else QCaPgHtjjYbZQeTaNeiAplpoznRQoe=269130243;if (QCaPgHtjjYbZQeTaNeiAplpoznRQoe == QCaPgHtjjYbZQeTaNeiAplpoznRQoe- 0 ) QCaPgHtjjYbZQeTaNeiAplpoznRQoe=1780438505; else QCaPgHtjjYbZQeTaNeiAplpoznRQoe=876899023;float UlsPtSICWEucsGYAJmgSkAoOtZLABa=1402262394.960685286417752639984742250018f;if (UlsPtSICWEucsGYAJmgSkAoOtZLABa - UlsPtSICWEucsGYAJmgSkAoOtZLABa> 0.00000001 ) UlsPtSICWEucsGYAJmgSkAoOtZLABa=1043403959.823802265860988868393065732853f; else UlsPtSICWEucsGYAJmgSkAoOtZLABa=1152451102.628870596157423586440382696918f;if (UlsPtSICWEucsGYAJmgSkAoOtZLABa - UlsPtSICWEucsGYAJmgSkAoOtZLABa> 0.00000001 ) UlsPtSICWEucsGYAJmgSkAoOtZLABa=1078181304.042335212947335665325741445739f; else UlsPtSICWEucsGYAJmgSkAoOtZLABa=1833413978.468921629848560405703260330109f;if (UlsPtSICWEucsGYAJmgSkAoOtZLABa - UlsPtSICWEucsGYAJmgSkAoOtZLABa> 0.00000001 ) UlsPtSICWEucsGYAJmgSkAoOtZLABa=1192280574.168358942415659829214004631969f; else UlsPtSICWEucsGYAJmgSkAoOtZLABa=875722550.253189021257439872252209616583f;if (UlsPtSICWEucsGYAJmgSkAoOtZLABa - UlsPtSICWEucsGYAJmgSkAoOtZLABa> 0.00000001 ) UlsPtSICWEucsGYAJmgSkAoOtZLABa=860291610.818336367091806867597982806457f; else UlsPtSICWEucsGYAJmgSkAoOtZLABa=1995811083.365855911461527140496000793763f;if (UlsPtSICWEucsGYAJmgSkAoOtZLABa - UlsPtSICWEucsGYAJmgSkAoOtZLABa> 0.00000001 ) UlsPtSICWEucsGYAJmgSkAoOtZLABa=1995242858.001083616295723647577607246626f; else UlsPtSICWEucsGYAJmgSkAoOtZLABa=882847753.226847155464520149726720740584f;if (UlsPtSICWEucsGYAJmgSkAoOtZLABa - UlsPtSICWEucsGYAJmgSkAoOtZLABa> 0.00000001 ) UlsPtSICWEucsGYAJmgSkAoOtZLABa=238481957.810008099769212429821800151720f; else UlsPtSICWEucsGYAJmgSkAoOtZLABa=1602622372.768175552315107751933287402006f;int RlsAxIHVgxhgZFdjxTVAvpticrGjjU=1995862837;if (RlsAxIHVgxhgZFdjxTVAvpticrGjjU == RlsAxIHVgxhgZFdjxTVAvpticrGjjU- 0 ) RlsAxIHVgxhgZFdjxTVAvpticrGjjU=1526444225; else RlsAxIHVgxhgZFdjxTVAvpticrGjjU=1693704972;if (RlsAxIHVgxhgZFdjxTVAvpticrGjjU == RlsAxIHVgxhgZFdjxTVAvpticrGjjU- 0 ) RlsAxIHVgxhgZFdjxTVAvpticrGjjU=1601247151; else RlsAxIHVgxhgZFdjxTVAvpticrGjjU=982042232;if (RlsAxIHVgxhgZFdjxTVAvpticrGjjU == RlsAxIHVgxhgZFdjxTVAvpticrGjjU- 0 ) RlsAxIHVgxhgZFdjxTVAvpticrGjjU=1693350304; else RlsAxIHVgxhgZFdjxTVAvpticrGjjU=539536866;if (RlsAxIHVgxhgZFdjxTVAvpticrGjjU == RlsAxIHVgxhgZFdjxTVAvpticrGjjU- 0 ) RlsAxIHVgxhgZFdjxTVAvpticrGjjU=715442304; else RlsAxIHVgxhgZFdjxTVAvpticrGjjU=1359238370;if (RlsAxIHVgxhgZFdjxTVAvpticrGjjU == RlsAxIHVgxhgZFdjxTVAvpticrGjjU- 0 ) RlsAxIHVgxhgZFdjxTVAvpticrGjjU=1185639222; else RlsAxIHVgxhgZFdjxTVAvpticrGjjU=1253686931;if (RlsAxIHVgxhgZFdjxTVAvpticrGjjU == RlsAxIHVgxhgZFdjxTVAvpticrGjjU- 1 ) RlsAxIHVgxhgZFdjxTVAvpticrGjjU=1618991051; else RlsAxIHVgxhgZFdjxTVAvpticrGjjU=400861436;int yGGswmkCqiYjHdbKWPeVfJdPicfdYz=1859724986;if (yGGswmkCqiYjHdbKWPeVfJdPicfdYz == yGGswmkCqiYjHdbKWPeVfJdPicfdYz- 1 ) yGGswmkCqiYjHdbKWPeVfJdPicfdYz=120374643; else yGGswmkCqiYjHdbKWPeVfJdPicfdYz=1208325471;if (yGGswmkCqiYjHdbKWPeVfJdPicfdYz == yGGswmkCqiYjHdbKWPeVfJdPicfdYz- 1 ) yGGswmkCqiYjHdbKWPeVfJdPicfdYz=972424517; else yGGswmkCqiYjHdbKWPeVfJdPicfdYz=1561305979;if (yGGswmkCqiYjHdbKWPeVfJdPicfdYz == yGGswmkCqiYjHdbKWPeVfJdPicfdYz- 0 ) yGGswmkCqiYjHdbKWPeVfJdPicfdYz=249736901; else yGGswmkCqiYjHdbKWPeVfJdPicfdYz=713143012;if (yGGswmkCqiYjHdbKWPeVfJdPicfdYz == yGGswmkCqiYjHdbKWPeVfJdPicfdYz- 1 ) yGGswmkCqiYjHdbKWPeVfJdPicfdYz=802731318; else yGGswmkCqiYjHdbKWPeVfJdPicfdYz=1015022184;if (yGGswmkCqiYjHdbKWPeVfJdPicfdYz == yGGswmkCqiYjHdbKWPeVfJdPicfdYz- 1 ) yGGswmkCqiYjHdbKWPeVfJdPicfdYz=958500421; else yGGswmkCqiYjHdbKWPeVfJdPicfdYz=1632397167;if (yGGswmkCqiYjHdbKWPeVfJdPicfdYz == yGGswmkCqiYjHdbKWPeVfJdPicfdYz- 1 ) yGGswmkCqiYjHdbKWPeVfJdPicfdYz=898831484; else yGGswmkCqiYjHdbKWPeVfJdPicfdYz=2089173319;double MaBKgOJCJstmcpXwNxFHaKbIGpUhtV=483128566.399580296472274468060401783945;if (MaBKgOJCJstmcpXwNxFHaKbIGpUhtV == MaBKgOJCJstmcpXwNxFHaKbIGpUhtV ) MaBKgOJCJstmcpXwNxFHaKbIGpUhtV=1894345583.283457072879666220822530890880; else MaBKgOJCJstmcpXwNxFHaKbIGpUhtV=1932059315.736239993840629194657422486376;if (MaBKgOJCJstmcpXwNxFHaKbIGpUhtV == MaBKgOJCJstmcpXwNxFHaKbIGpUhtV ) MaBKgOJCJstmcpXwNxFHaKbIGpUhtV=1497938445.839215354834709291171634442495; else MaBKgOJCJstmcpXwNxFHaKbIGpUhtV=1739301068.372711251585093482420620621688;if (MaBKgOJCJstmcpXwNxFHaKbIGpUhtV == MaBKgOJCJstmcpXwNxFHaKbIGpUhtV ) MaBKgOJCJstmcpXwNxFHaKbIGpUhtV=1446287194.307057200804079764116007685329; else MaBKgOJCJstmcpXwNxFHaKbIGpUhtV=2059106896.177439814728273666106757230057;if (MaBKgOJCJstmcpXwNxFHaKbIGpUhtV == MaBKgOJCJstmcpXwNxFHaKbIGpUhtV ) MaBKgOJCJstmcpXwNxFHaKbIGpUhtV=1856395737.299284875196150984503933342020; else MaBKgOJCJstmcpXwNxFHaKbIGpUhtV=503867093.097854910999212171845665547794;if (MaBKgOJCJstmcpXwNxFHaKbIGpUhtV == MaBKgOJCJstmcpXwNxFHaKbIGpUhtV ) MaBKgOJCJstmcpXwNxFHaKbIGpUhtV=437008798.286511941608561483228229543232; else MaBKgOJCJstmcpXwNxFHaKbIGpUhtV=1866426581.833682370234034821416467706817;if (MaBKgOJCJstmcpXwNxFHaKbIGpUhtV == MaBKgOJCJstmcpXwNxFHaKbIGpUhtV ) MaBKgOJCJstmcpXwNxFHaKbIGpUhtV=647774756.871997598021310530573383648709; else MaBKgOJCJstmcpXwNxFHaKbIGpUhtV=2086077290.767346314743350615738644460946;int rvxQeVBhYGFhpRBzPQBOOxmEDLNtre=1145133663;if (rvxQeVBhYGFhpRBzPQBOOxmEDLNtre == rvxQeVBhYGFhpRBzPQBOOxmEDLNtre- 0 ) rvxQeVBhYGFhpRBzPQBOOxmEDLNtre=2076984952; else rvxQeVBhYGFhpRBzPQBOOxmEDLNtre=526240618;if (rvxQeVBhYGFhpRBzPQBOOxmEDLNtre == rvxQeVBhYGFhpRBzPQBOOxmEDLNtre- 0 ) rvxQeVBhYGFhpRBzPQBOOxmEDLNtre=1482247729; else rvxQeVBhYGFhpRBzPQBOOxmEDLNtre=785694824;if (rvxQeVBhYGFhpRBzPQBOOxmEDLNtre == rvxQeVBhYGFhpRBzPQBOOxmEDLNtre- 1 ) rvxQeVBhYGFhpRBzPQBOOxmEDLNtre=833261490; else rvxQeVBhYGFhpRBzPQBOOxmEDLNtre=1108445975;if (rvxQeVBhYGFhpRBzPQBOOxmEDLNtre == rvxQeVBhYGFhpRBzPQBOOxmEDLNtre- 1 ) rvxQeVBhYGFhpRBzPQBOOxmEDLNtre=1746125936; else rvxQeVBhYGFhpRBzPQBOOxmEDLNtre=813301213;if (rvxQeVBhYGFhpRBzPQBOOxmEDLNtre == rvxQeVBhYGFhpRBzPQBOOxmEDLNtre- 1 ) rvxQeVBhYGFhpRBzPQBOOxmEDLNtre=890973654; else rvxQeVBhYGFhpRBzPQBOOxmEDLNtre=586224967;if (rvxQeVBhYGFhpRBzPQBOOxmEDLNtre == rvxQeVBhYGFhpRBzPQBOOxmEDLNtre- 0 ) rvxQeVBhYGFhpRBzPQBOOxmEDLNtre=23541811; else rvxQeVBhYGFhpRBzPQBOOxmEDLNtre=60953137;float MopCDabyifDQDLJlxThwgBXiXnDyHN=1083980625.968469975200998031044604452875f;if (MopCDabyifDQDLJlxThwgBXiXnDyHN - MopCDabyifDQDLJlxThwgBXiXnDyHN> 0.00000001 ) MopCDabyifDQDLJlxThwgBXiXnDyHN=234140075.535448394167209965297684947412f; else MopCDabyifDQDLJlxThwgBXiXnDyHN=1135862485.821188665075809851660769806150f;if (MopCDabyifDQDLJlxThwgBXiXnDyHN - MopCDabyifDQDLJlxThwgBXiXnDyHN> 0.00000001 ) MopCDabyifDQDLJlxThwgBXiXnDyHN=653999500.299235723230032399620795316239f; else MopCDabyifDQDLJlxThwgBXiXnDyHN=1949352917.252699667413853084851754161403f;if (MopCDabyifDQDLJlxThwgBXiXnDyHN - MopCDabyifDQDLJlxThwgBXiXnDyHN> 0.00000001 ) MopCDabyifDQDLJlxThwgBXiXnDyHN=1499654397.059509955484231794334259850863f; else MopCDabyifDQDLJlxThwgBXiXnDyHN=1361529027.836557536524058853347717080185f;if (MopCDabyifDQDLJlxThwgBXiXnDyHN - MopCDabyifDQDLJlxThwgBXiXnDyHN> 0.00000001 ) MopCDabyifDQDLJlxThwgBXiXnDyHN=413437792.910265888536992031111762612057f; else MopCDabyifDQDLJlxThwgBXiXnDyHN=208751394.756171589307592193506008526875f;if (MopCDabyifDQDLJlxThwgBXiXnDyHN - MopCDabyifDQDLJlxThwgBXiXnDyHN> 0.00000001 ) MopCDabyifDQDLJlxThwgBXiXnDyHN=1499413381.493198182265391428153956286975f; else MopCDabyifDQDLJlxThwgBXiXnDyHN=468557424.672502550411144182710972255802f;if (MopCDabyifDQDLJlxThwgBXiXnDyHN - MopCDabyifDQDLJlxThwgBXiXnDyHN> 0.00000001 ) MopCDabyifDQDLJlxThwgBXiXnDyHN=1124297181.321879499908884391220426671136f; else MopCDabyifDQDLJlxThwgBXiXnDyHN=530793967.875910048670392210807468290520f;int PeOJNQviYNWNhYEETVVhHOOwPEmkKx=499070247;if (PeOJNQviYNWNhYEETVVhHOOwPEmkKx == PeOJNQviYNWNhYEETVVhHOOwPEmkKx- 0 ) PeOJNQviYNWNhYEETVVhHOOwPEmkKx=1370514669; else PeOJNQviYNWNhYEETVVhHOOwPEmkKx=1901181468;if (PeOJNQviYNWNhYEETVVhHOOwPEmkKx == PeOJNQviYNWNhYEETVVhHOOwPEmkKx- 1 ) PeOJNQviYNWNhYEETVVhHOOwPEmkKx=12188367; else PeOJNQviYNWNhYEETVVhHOOwPEmkKx=815934097;if (PeOJNQviYNWNhYEETVVhHOOwPEmkKx == PeOJNQviYNWNhYEETVVhHOOwPEmkKx- 1 ) PeOJNQviYNWNhYEETVVhHOOwPEmkKx=766894515; else PeOJNQviYNWNhYEETVVhHOOwPEmkKx=233852032;if (PeOJNQviYNWNhYEETVVhHOOwPEmkKx == PeOJNQviYNWNhYEETVVhHOOwPEmkKx- 0 ) PeOJNQviYNWNhYEETVVhHOOwPEmkKx=1797350723; else PeOJNQviYNWNhYEETVVhHOOwPEmkKx=785899608;if (PeOJNQviYNWNhYEETVVhHOOwPEmkKx == PeOJNQviYNWNhYEETVVhHOOwPEmkKx- 1 ) PeOJNQviYNWNhYEETVVhHOOwPEmkKx=185933379; else PeOJNQviYNWNhYEETVVhHOOwPEmkKx=684637232;if (PeOJNQviYNWNhYEETVVhHOOwPEmkKx == PeOJNQviYNWNhYEETVVhHOOwPEmkKx- 1 ) PeOJNQviYNWNhYEETVVhHOOwPEmkKx=848327414; else PeOJNQviYNWNhYEETVVhHOOwPEmkKx=860062227;float QVlrNEQwogFhFIHxweyGJmAxyGZDEq=1810739813.034216696166363577760719209377f;if (QVlrNEQwogFhFIHxweyGJmAxyGZDEq - QVlrNEQwogFhFIHxweyGJmAxyGZDEq> 0.00000001 ) QVlrNEQwogFhFIHxweyGJmAxyGZDEq=1849362926.629200241582451948195899232601f; else QVlrNEQwogFhFIHxweyGJmAxyGZDEq=1711277778.832965717333729947847138384117f;if (QVlrNEQwogFhFIHxweyGJmAxyGZDEq - QVlrNEQwogFhFIHxweyGJmAxyGZDEq> 0.00000001 ) QVlrNEQwogFhFIHxweyGJmAxyGZDEq=1851937215.974186059723297227140697481578f; else QVlrNEQwogFhFIHxweyGJmAxyGZDEq=268352661.171047075409812494217717748870f;if (QVlrNEQwogFhFIHxweyGJmAxyGZDEq - QVlrNEQwogFhFIHxweyGJmAxyGZDEq> 0.00000001 ) QVlrNEQwogFhFIHxweyGJmAxyGZDEq=1952914321.568266325035385188600836156441f; else QVlrNEQwogFhFIHxweyGJmAxyGZDEq=460013228.930595275627939344276127173312f;if (QVlrNEQwogFhFIHxweyGJmAxyGZDEq - QVlrNEQwogFhFIHxweyGJmAxyGZDEq> 0.00000001 ) QVlrNEQwogFhFIHxweyGJmAxyGZDEq=1792962913.199411599101309390891337691452f; else QVlrNEQwogFhFIHxweyGJmAxyGZDEq=715455925.296463852986627583999363287293f;if (QVlrNEQwogFhFIHxweyGJmAxyGZDEq - QVlrNEQwogFhFIHxweyGJmAxyGZDEq> 0.00000001 ) QVlrNEQwogFhFIHxweyGJmAxyGZDEq=112802826.024701604655902581111857494428f; else QVlrNEQwogFhFIHxweyGJmAxyGZDEq=1273496335.776096293709034457271629237831f;if (QVlrNEQwogFhFIHxweyGJmAxyGZDEq - QVlrNEQwogFhFIHxweyGJmAxyGZDEq> 0.00000001 ) QVlrNEQwogFhFIHxweyGJmAxyGZDEq=1433765090.828609991649942392304836715896f; else QVlrNEQwogFhFIHxweyGJmAxyGZDEq=364704066.046298144667674388795564800390f;double CzZfxPnaSJWslUnLfNYRjFyIVrLVwq=26835979.686327205705664625286279542015;if (CzZfxPnaSJWslUnLfNYRjFyIVrLVwq == CzZfxPnaSJWslUnLfNYRjFyIVrLVwq ) CzZfxPnaSJWslUnLfNYRjFyIVrLVwq=975417929.761827010488577259945868041788; else CzZfxPnaSJWslUnLfNYRjFyIVrLVwq=1671955211.520114594861591980339428000489;if (CzZfxPnaSJWslUnLfNYRjFyIVrLVwq == CzZfxPnaSJWslUnLfNYRjFyIVrLVwq ) CzZfxPnaSJWslUnLfNYRjFyIVrLVwq=837922115.634297847387528537717155921312; else CzZfxPnaSJWslUnLfNYRjFyIVrLVwq=2034491208.411624841514416391067478259937;if (CzZfxPnaSJWslUnLfNYRjFyIVrLVwq == CzZfxPnaSJWslUnLfNYRjFyIVrLVwq ) CzZfxPnaSJWslUnLfNYRjFyIVrLVwq=761564265.664697600580731385074136514447; else CzZfxPnaSJWslUnLfNYRjFyIVrLVwq=1348613205.692596192980168874808497363918;if (CzZfxPnaSJWslUnLfNYRjFyIVrLVwq == CzZfxPnaSJWslUnLfNYRjFyIVrLVwq ) CzZfxPnaSJWslUnLfNYRjFyIVrLVwq=317086110.426309935530329507884175020523; else CzZfxPnaSJWslUnLfNYRjFyIVrLVwq=288043.328466497905038928897724536248;if (CzZfxPnaSJWslUnLfNYRjFyIVrLVwq == CzZfxPnaSJWslUnLfNYRjFyIVrLVwq ) CzZfxPnaSJWslUnLfNYRjFyIVrLVwq=1485995409.042231207312199915821383771503; else CzZfxPnaSJWslUnLfNYRjFyIVrLVwq=2015583539.135906480772258664563153877967;if (CzZfxPnaSJWslUnLfNYRjFyIVrLVwq == CzZfxPnaSJWslUnLfNYRjFyIVrLVwq ) CzZfxPnaSJWslUnLfNYRjFyIVrLVwq=1763419538.180439729364515923521804109313; else CzZfxPnaSJWslUnLfNYRjFyIVrLVwq=815017165.063469383095531964806544649743;long VCfQBFIcggWvZNgOkSHVYYHlaHqnHB=1851454428;if (VCfQBFIcggWvZNgOkSHVYYHlaHqnHB == VCfQBFIcggWvZNgOkSHVYYHlaHqnHB- 0 ) VCfQBFIcggWvZNgOkSHVYYHlaHqnHB=1752879360; else VCfQBFIcggWvZNgOkSHVYYHlaHqnHB=1461041015;if (VCfQBFIcggWvZNgOkSHVYYHlaHqnHB == VCfQBFIcggWvZNgOkSHVYYHlaHqnHB- 1 ) VCfQBFIcggWvZNgOkSHVYYHlaHqnHB=828914688; else VCfQBFIcggWvZNgOkSHVYYHlaHqnHB=501574125;if (VCfQBFIcggWvZNgOkSHVYYHlaHqnHB == VCfQBFIcggWvZNgOkSHVYYHlaHqnHB- 1 ) VCfQBFIcggWvZNgOkSHVYYHlaHqnHB=1159684107; else VCfQBFIcggWvZNgOkSHVYYHlaHqnHB=1489938024;if (VCfQBFIcggWvZNgOkSHVYYHlaHqnHB == VCfQBFIcggWvZNgOkSHVYYHlaHqnHB- 0 ) VCfQBFIcggWvZNgOkSHVYYHlaHqnHB=693639573; else VCfQBFIcggWvZNgOkSHVYYHlaHqnHB=910501441;if (VCfQBFIcggWvZNgOkSHVYYHlaHqnHB == VCfQBFIcggWvZNgOkSHVYYHlaHqnHB- 1 ) VCfQBFIcggWvZNgOkSHVYYHlaHqnHB=108218555; else VCfQBFIcggWvZNgOkSHVYYHlaHqnHB=1444216089;if (VCfQBFIcggWvZNgOkSHVYYHlaHqnHB == VCfQBFIcggWvZNgOkSHVYYHlaHqnHB- 1 ) VCfQBFIcggWvZNgOkSHVYYHlaHqnHB=703131271; else VCfQBFIcggWvZNgOkSHVYYHlaHqnHB=1640881827;long qssojDGiNMHkdWIYsBUUtfNSBqFeVb=1118085057;if (qssojDGiNMHkdWIYsBUUtfNSBqFeVb == qssojDGiNMHkdWIYsBUUtfNSBqFeVb- 1 ) qssojDGiNMHkdWIYsBUUtfNSBqFeVb=2084187922; else qssojDGiNMHkdWIYsBUUtfNSBqFeVb=821491435;if (qssojDGiNMHkdWIYsBUUtfNSBqFeVb == qssojDGiNMHkdWIYsBUUtfNSBqFeVb- 0 ) qssojDGiNMHkdWIYsBUUtfNSBqFeVb=2032127945; else qssojDGiNMHkdWIYsBUUtfNSBqFeVb=363609874;if (qssojDGiNMHkdWIYsBUUtfNSBqFeVb == qssojDGiNMHkdWIYsBUUtfNSBqFeVb- 1 ) qssojDGiNMHkdWIYsBUUtfNSBqFeVb=957520485; else qssojDGiNMHkdWIYsBUUtfNSBqFeVb=976154150;if (qssojDGiNMHkdWIYsBUUtfNSBqFeVb == qssojDGiNMHkdWIYsBUUtfNSBqFeVb- 0 ) qssojDGiNMHkdWIYsBUUtfNSBqFeVb=1049175548; else qssojDGiNMHkdWIYsBUUtfNSBqFeVb=500749721;if (qssojDGiNMHkdWIYsBUUtfNSBqFeVb == qssojDGiNMHkdWIYsBUUtfNSBqFeVb- 0 ) qssojDGiNMHkdWIYsBUUtfNSBqFeVb=1394191668; else qssojDGiNMHkdWIYsBUUtfNSBqFeVb=1851649172;if (qssojDGiNMHkdWIYsBUUtfNSBqFeVb == qssojDGiNMHkdWIYsBUUtfNSBqFeVb- 1 ) qssojDGiNMHkdWIYsBUUtfNSBqFeVb=929840899; else qssojDGiNMHkdWIYsBUUtfNSBqFeVb=2084814429;int ilAEPheSxHSLPkoPTCJQSElCltvdSa=559427261;if (ilAEPheSxHSLPkoPTCJQSElCltvdSa == ilAEPheSxHSLPkoPTCJQSElCltvdSa- 0 ) ilAEPheSxHSLPkoPTCJQSElCltvdSa=952826379; else ilAEPheSxHSLPkoPTCJQSElCltvdSa=1729794309;if (ilAEPheSxHSLPkoPTCJQSElCltvdSa == ilAEPheSxHSLPkoPTCJQSElCltvdSa- 0 ) ilAEPheSxHSLPkoPTCJQSElCltvdSa=759308353; else ilAEPheSxHSLPkoPTCJQSElCltvdSa=670591426;if (ilAEPheSxHSLPkoPTCJQSElCltvdSa == ilAEPheSxHSLPkoPTCJQSElCltvdSa- 0 ) ilAEPheSxHSLPkoPTCJQSElCltvdSa=386417281; else ilAEPheSxHSLPkoPTCJQSElCltvdSa=246358832;if (ilAEPheSxHSLPkoPTCJQSElCltvdSa == ilAEPheSxHSLPkoPTCJQSElCltvdSa- 0 ) ilAEPheSxHSLPkoPTCJQSElCltvdSa=1014635519; else ilAEPheSxHSLPkoPTCJQSElCltvdSa=1632963552;if (ilAEPheSxHSLPkoPTCJQSElCltvdSa == ilAEPheSxHSLPkoPTCJQSElCltvdSa- 1 ) ilAEPheSxHSLPkoPTCJQSElCltvdSa=978315638; else ilAEPheSxHSLPkoPTCJQSElCltvdSa=2074904321;if (ilAEPheSxHSLPkoPTCJQSElCltvdSa == ilAEPheSxHSLPkoPTCJQSElCltvdSa- 1 ) ilAEPheSxHSLPkoPTCJQSElCltvdSa=1926272653; else ilAEPheSxHSLPkoPTCJQSElCltvdSa=1464831823;float jHDPIHUMnEpWKthzYYctZggJKBBeBV=53348752.991879507826674252769930897192f;if (jHDPIHUMnEpWKthzYYctZggJKBBeBV - jHDPIHUMnEpWKthzYYctZggJKBBeBV> 0.00000001 ) jHDPIHUMnEpWKthzYYctZggJKBBeBV=1213832725.270271505195381336323814416118f; else jHDPIHUMnEpWKthzYYctZggJKBBeBV=645334481.352665650386767576057287057787f;if (jHDPIHUMnEpWKthzYYctZggJKBBeBV - jHDPIHUMnEpWKthzYYctZggJKBBeBV> 0.00000001 ) jHDPIHUMnEpWKthzYYctZggJKBBeBV=878312890.649885383679615715704117227661f; else jHDPIHUMnEpWKthzYYctZggJKBBeBV=1839193825.908484709329358868235208364367f;if (jHDPIHUMnEpWKthzYYctZggJKBBeBV - jHDPIHUMnEpWKthzYYctZggJKBBeBV> 0.00000001 ) jHDPIHUMnEpWKthzYYctZggJKBBeBV=1816214914.882278223201759686946054770012f; else jHDPIHUMnEpWKthzYYctZggJKBBeBV=2104543729.054255729175503556079852193960f;if (jHDPIHUMnEpWKthzYYctZggJKBBeBV - jHDPIHUMnEpWKthzYYctZggJKBBeBV> 0.00000001 ) jHDPIHUMnEpWKthzYYctZggJKBBeBV=893933943.821126750406898255232514430216f; else jHDPIHUMnEpWKthzYYctZggJKBBeBV=776725210.297935449781668307505973842780f;if (jHDPIHUMnEpWKthzYYctZggJKBBeBV - jHDPIHUMnEpWKthzYYctZggJKBBeBV> 0.00000001 ) jHDPIHUMnEpWKthzYYctZggJKBBeBV=411865467.665531906491755296723482232410f; else jHDPIHUMnEpWKthzYYctZggJKBBeBV=328164967.477913624741121433965717838340f;if (jHDPIHUMnEpWKthzYYctZggJKBBeBV - jHDPIHUMnEpWKthzYYctZggJKBBeBV> 0.00000001 ) jHDPIHUMnEpWKthzYYctZggJKBBeBV=1254866104.069703484368308247281066586129f; else jHDPIHUMnEpWKthzYYctZggJKBBeBV=1455518582.549489242915960742969226910433f;double NebsMxqJrAzYOkcEAbnQUKhzNMMbnv=1277756454.600890022627726093145622161040;if (NebsMxqJrAzYOkcEAbnQUKhzNMMbnv == NebsMxqJrAzYOkcEAbnQUKhzNMMbnv ) NebsMxqJrAzYOkcEAbnQUKhzNMMbnv=1241961777.484037423455678768475024219974; else NebsMxqJrAzYOkcEAbnQUKhzNMMbnv=143704010.008225489977079105451377656182;if (NebsMxqJrAzYOkcEAbnQUKhzNMMbnv == NebsMxqJrAzYOkcEAbnQUKhzNMMbnv ) NebsMxqJrAzYOkcEAbnQUKhzNMMbnv=688055448.865056746448051806522453474178; else NebsMxqJrAzYOkcEAbnQUKhzNMMbnv=1365259927.940721313347002342685167151279;if (NebsMxqJrAzYOkcEAbnQUKhzNMMbnv == NebsMxqJrAzYOkcEAbnQUKhzNMMbnv ) NebsMxqJrAzYOkcEAbnQUKhzNMMbnv=673338304.378950934169328556214259498367; else NebsMxqJrAzYOkcEAbnQUKhzNMMbnv=131159388.115518925251862684060516612957;if (NebsMxqJrAzYOkcEAbnQUKhzNMMbnv == NebsMxqJrAzYOkcEAbnQUKhzNMMbnv ) NebsMxqJrAzYOkcEAbnQUKhzNMMbnv=731667776.268706492074117349881540861000; else NebsMxqJrAzYOkcEAbnQUKhzNMMbnv=1837829090.717900427720310750038328683804;if (NebsMxqJrAzYOkcEAbnQUKhzNMMbnv == NebsMxqJrAzYOkcEAbnQUKhzNMMbnv ) NebsMxqJrAzYOkcEAbnQUKhzNMMbnv=1954339563.650247831793226224112107349933; else NebsMxqJrAzYOkcEAbnQUKhzNMMbnv=399164393.872417017492934521246079265301;if (NebsMxqJrAzYOkcEAbnQUKhzNMMbnv == NebsMxqJrAzYOkcEAbnQUKhzNMMbnv ) NebsMxqJrAzYOkcEAbnQUKhzNMMbnv=1532777883.890903291031782128383980790326; else NebsMxqJrAzYOkcEAbnQUKhzNMMbnv=1030099811.372761213243032630724812892132;float rmajcvxHHuHYIVzwZpDbDeAJZiztmv=570503738.363968727798161212186797480181f;if (rmajcvxHHuHYIVzwZpDbDeAJZiztmv - rmajcvxHHuHYIVzwZpDbDeAJZiztmv> 0.00000001 ) rmajcvxHHuHYIVzwZpDbDeAJZiztmv=651448071.461008123299961976482235877759f; else rmajcvxHHuHYIVzwZpDbDeAJZiztmv=876492147.974744827329009884652481933734f;if (rmajcvxHHuHYIVzwZpDbDeAJZiztmv - rmajcvxHHuHYIVzwZpDbDeAJZiztmv> 0.00000001 ) rmajcvxHHuHYIVzwZpDbDeAJZiztmv=870981704.147543633825016333600420759481f; else rmajcvxHHuHYIVzwZpDbDeAJZiztmv=198742856.110339257725184858571113473880f;if (rmajcvxHHuHYIVzwZpDbDeAJZiztmv - rmajcvxHHuHYIVzwZpDbDeAJZiztmv> 0.00000001 ) rmajcvxHHuHYIVzwZpDbDeAJZiztmv=99632858.426381579828709545700292011806f; else rmajcvxHHuHYIVzwZpDbDeAJZiztmv=510544134.758740090339985442504918024538f;if (rmajcvxHHuHYIVzwZpDbDeAJZiztmv - rmajcvxHHuHYIVzwZpDbDeAJZiztmv> 0.00000001 ) rmajcvxHHuHYIVzwZpDbDeAJZiztmv=890695994.188327006965558232777940294082f; else rmajcvxHHuHYIVzwZpDbDeAJZiztmv=995766297.767326266944665947099387134824f;if (rmajcvxHHuHYIVzwZpDbDeAJZiztmv - rmajcvxHHuHYIVzwZpDbDeAJZiztmv> 0.00000001 ) rmajcvxHHuHYIVzwZpDbDeAJZiztmv=835860366.303420933307683438635810458020f; else rmajcvxHHuHYIVzwZpDbDeAJZiztmv=2130066790.245364235512489270480639960418f;if (rmajcvxHHuHYIVzwZpDbDeAJZiztmv - rmajcvxHHuHYIVzwZpDbDeAJZiztmv> 0.00000001 ) rmajcvxHHuHYIVzwZpDbDeAJZiztmv=258725959.292417343931413443128874072641f; else rmajcvxHHuHYIVzwZpDbDeAJZiztmv=58252314.582732118304856032762830053845f;int hxKYVrhnEORYpJWBHqzTxyouibNltv=222006847;if (hxKYVrhnEORYpJWBHqzTxyouibNltv == hxKYVrhnEORYpJWBHqzTxyouibNltv- 1 ) hxKYVrhnEORYpJWBHqzTxyouibNltv=2008480217; else hxKYVrhnEORYpJWBHqzTxyouibNltv=648516199;if (hxKYVrhnEORYpJWBHqzTxyouibNltv == hxKYVrhnEORYpJWBHqzTxyouibNltv- 1 ) hxKYVrhnEORYpJWBHqzTxyouibNltv=222870046; else hxKYVrhnEORYpJWBHqzTxyouibNltv=501777924;if (hxKYVrhnEORYpJWBHqzTxyouibNltv == hxKYVrhnEORYpJWBHqzTxyouibNltv- 0 ) hxKYVrhnEORYpJWBHqzTxyouibNltv=135632482; else hxKYVrhnEORYpJWBHqzTxyouibNltv=1982424821;if (hxKYVrhnEORYpJWBHqzTxyouibNltv == hxKYVrhnEORYpJWBHqzTxyouibNltv- 1 ) hxKYVrhnEORYpJWBHqzTxyouibNltv=1527494095; else hxKYVrhnEORYpJWBHqzTxyouibNltv=1082673337;if (hxKYVrhnEORYpJWBHqzTxyouibNltv == hxKYVrhnEORYpJWBHqzTxyouibNltv- 1 ) hxKYVrhnEORYpJWBHqzTxyouibNltv=2061150352; else hxKYVrhnEORYpJWBHqzTxyouibNltv=1187967447;if (hxKYVrhnEORYpJWBHqzTxyouibNltv == hxKYVrhnEORYpJWBHqzTxyouibNltv- 0 ) hxKYVrhnEORYpJWBHqzTxyouibNltv=2032337909; else hxKYVrhnEORYpJWBHqzTxyouibNltv=1417050521;float EpbHAyTYHFbTpOavnPyJLtDRXYqspH=827521742.782511613827994742921109668789f;if (EpbHAyTYHFbTpOavnPyJLtDRXYqspH - EpbHAyTYHFbTpOavnPyJLtDRXYqspH> 0.00000001 ) EpbHAyTYHFbTpOavnPyJLtDRXYqspH=472897180.850779147827458439848071379406f; else EpbHAyTYHFbTpOavnPyJLtDRXYqspH=684362366.403417722570520576224995189940f;if (EpbHAyTYHFbTpOavnPyJLtDRXYqspH - EpbHAyTYHFbTpOavnPyJLtDRXYqspH> 0.00000001 ) EpbHAyTYHFbTpOavnPyJLtDRXYqspH=804070511.185253504171748866290438352843f; else EpbHAyTYHFbTpOavnPyJLtDRXYqspH=1831088625.989340637818216326833486186800f;if (EpbHAyTYHFbTpOavnPyJLtDRXYqspH - EpbHAyTYHFbTpOavnPyJLtDRXYqspH> 0.00000001 ) EpbHAyTYHFbTpOavnPyJLtDRXYqspH=2032810734.177230250833949332203611061427f; else EpbHAyTYHFbTpOavnPyJLtDRXYqspH=2015164895.594774108728601154795460866737f;if (EpbHAyTYHFbTpOavnPyJLtDRXYqspH - EpbHAyTYHFbTpOavnPyJLtDRXYqspH> 0.00000001 ) EpbHAyTYHFbTpOavnPyJLtDRXYqspH=1178123596.994345928523469601002773323402f; else EpbHAyTYHFbTpOavnPyJLtDRXYqspH=731417643.847283447723727200226684632112f;if (EpbHAyTYHFbTpOavnPyJLtDRXYqspH - EpbHAyTYHFbTpOavnPyJLtDRXYqspH> 0.00000001 ) EpbHAyTYHFbTpOavnPyJLtDRXYqspH=1400491663.252689739627548915613338637476f; else EpbHAyTYHFbTpOavnPyJLtDRXYqspH=1839772798.234557215967260014682761003405f;if (EpbHAyTYHFbTpOavnPyJLtDRXYqspH - EpbHAyTYHFbTpOavnPyJLtDRXYqspH> 0.00000001 ) EpbHAyTYHFbTpOavnPyJLtDRXYqspH=529399561.237450300180085627972489458466f; else EpbHAyTYHFbTpOavnPyJLtDRXYqspH=690755161.581174192172561388649664395801f;float yTTgAuxBEdNdnCBrLsGkymJlrGoort=247738979.899643595557058763264583660844f;if (yTTgAuxBEdNdnCBrLsGkymJlrGoort - yTTgAuxBEdNdnCBrLsGkymJlrGoort> 0.00000001 ) yTTgAuxBEdNdnCBrLsGkymJlrGoort=1473781613.253711992301067451697362132718f; else yTTgAuxBEdNdnCBrLsGkymJlrGoort=1310771036.486241434788296352621026899279f;if (yTTgAuxBEdNdnCBrLsGkymJlrGoort - yTTgAuxBEdNdnCBrLsGkymJlrGoort> 0.00000001 ) yTTgAuxBEdNdnCBrLsGkymJlrGoort=1169103241.938492692702792917789019856046f; else yTTgAuxBEdNdnCBrLsGkymJlrGoort=548848945.237695090476002934498369093065f;if (yTTgAuxBEdNdnCBrLsGkymJlrGoort - yTTgAuxBEdNdnCBrLsGkymJlrGoort> 0.00000001 ) yTTgAuxBEdNdnCBrLsGkymJlrGoort=1508957326.665768621703731301792376898960f; else yTTgAuxBEdNdnCBrLsGkymJlrGoort=1500973510.313682145542480663889238440097f;if (yTTgAuxBEdNdnCBrLsGkymJlrGoort - yTTgAuxBEdNdnCBrLsGkymJlrGoort> 0.00000001 ) yTTgAuxBEdNdnCBrLsGkymJlrGoort=434871256.076240025027726109734528128635f; else yTTgAuxBEdNdnCBrLsGkymJlrGoort=816506897.225867635074987352335237366813f;if (yTTgAuxBEdNdnCBrLsGkymJlrGoort - yTTgAuxBEdNdnCBrLsGkymJlrGoort> 0.00000001 ) yTTgAuxBEdNdnCBrLsGkymJlrGoort=1496481699.240190397958577074287740019950f; else yTTgAuxBEdNdnCBrLsGkymJlrGoort=1935792014.292393590717549876049273661118f;if (yTTgAuxBEdNdnCBrLsGkymJlrGoort - yTTgAuxBEdNdnCBrLsGkymJlrGoort> 0.00000001 ) yTTgAuxBEdNdnCBrLsGkymJlrGoort=285076151.144777639558568130134492212973f; else yTTgAuxBEdNdnCBrLsGkymJlrGoort=1059149058.359367387325869171654108486367f;float jdfJCzMyGpXPnEwdMNBRFTJwKYCsoF=972026234.023371128464353257919871795427f;if (jdfJCzMyGpXPnEwdMNBRFTJwKYCsoF - jdfJCzMyGpXPnEwdMNBRFTJwKYCsoF> 0.00000001 ) jdfJCzMyGpXPnEwdMNBRFTJwKYCsoF=403223077.556798936169418963211517536279f; else jdfJCzMyGpXPnEwdMNBRFTJwKYCsoF=1578687864.681101867632196046484221715026f;if (jdfJCzMyGpXPnEwdMNBRFTJwKYCsoF - jdfJCzMyGpXPnEwdMNBRFTJwKYCsoF> 0.00000001 ) jdfJCzMyGpXPnEwdMNBRFTJwKYCsoF=1732838781.287868842720779043150090547059f; else jdfJCzMyGpXPnEwdMNBRFTJwKYCsoF=454614009.785146643831577565378207739732f;if (jdfJCzMyGpXPnEwdMNBRFTJwKYCsoF - jdfJCzMyGpXPnEwdMNBRFTJwKYCsoF> 0.00000001 ) jdfJCzMyGpXPnEwdMNBRFTJwKYCsoF=902530073.289314573341033053419288259842f; else jdfJCzMyGpXPnEwdMNBRFTJwKYCsoF=1389261449.891175236309204837588743401216f;if (jdfJCzMyGpXPnEwdMNBRFTJwKYCsoF - jdfJCzMyGpXPnEwdMNBRFTJwKYCsoF> 0.00000001 ) jdfJCzMyGpXPnEwdMNBRFTJwKYCsoF=336679760.683266096883712008553346425888f; else jdfJCzMyGpXPnEwdMNBRFTJwKYCsoF=2140798969.513183615884171504140376434884f;if (jdfJCzMyGpXPnEwdMNBRFTJwKYCsoF - jdfJCzMyGpXPnEwdMNBRFTJwKYCsoF> 0.00000001 ) jdfJCzMyGpXPnEwdMNBRFTJwKYCsoF=1617876250.222948174050378028602284834775f; else jdfJCzMyGpXPnEwdMNBRFTJwKYCsoF=1638936050.993276223353020100851729215912f;if (jdfJCzMyGpXPnEwdMNBRFTJwKYCsoF - jdfJCzMyGpXPnEwdMNBRFTJwKYCsoF> 0.00000001 ) jdfJCzMyGpXPnEwdMNBRFTJwKYCsoF=1248769908.450916821381545926483593438407f; else jdfJCzMyGpXPnEwdMNBRFTJwKYCsoF=443343331.141201411548067139439054906868f;long SMWKVwORXeAYwVYZPdNCxzhlHslNCY=1387734890;if (SMWKVwORXeAYwVYZPdNCxzhlHslNCY == SMWKVwORXeAYwVYZPdNCxzhlHslNCY- 1 ) SMWKVwORXeAYwVYZPdNCxzhlHslNCY=489114732; else SMWKVwORXeAYwVYZPdNCxzhlHslNCY=1784598930;if (SMWKVwORXeAYwVYZPdNCxzhlHslNCY == SMWKVwORXeAYwVYZPdNCxzhlHslNCY- 0 ) SMWKVwORXeAYwVYZPdNCxzhlHslNCY=2090752555; else SMWKVwORXeAYwVYZPdNCxzhlHslNCY=1408478457;if (SMWKVwORXeAYwVYZPdNCxzhlHslNCY == SMWKVwORXeAYwVYZPdNCxzhlHslNCY- 0 ) SMWKVwORXeAYwVYZPdNCxzhlHslNCY=842214538; else SMWKVwORXeAYwVYZPdNCxzhlHslNCY=1177994768;if (SMWKVwORXeAYwVYZPdNCxzhlHslNCY == SMWKVwORXeAYwVYZPdNCxzhlHslNCY- 1 ) SMWKVwORXeAYwVYZPdNCxzhlHslNCY=1466048063; else SMWKVwORXeAYwVYZPdNCxzhlHslNCY=1119407317;if (SMWKVwORXeAYwVYZPdNCxzhlHslNCY == SMWKVwORXeAYwVYZPdNCxzhlHslNCY- 0 ) SMWKVwORXeAYwVYZPdNCxzhlHslNCY=249491110; else SMWKVwORXeAYwVYZPdNCxzhlHslNCY=2135047261;if (SMWKVwORXeAYwVYZPdNCxzhlHslNCY == SMWKVwORXeAYwVYZPdNCxzhlHslNCY- 0 ) SMWKVwORXeAYwVYZPdNCxzhlHslNCY=334606926; else SMWKVwORXeAYwVYZPdNCxzhlHslNCY=97018720;int sXDLOxAWBxvlKkvsJWANOMJByjveVq=1926846190;if (sXDLOxAWBxvlKkvsJWANOMJByjveVq == sXDLOxAWBxvlKkvsJWANOMJByjveVq- 1 ) sXDLOxAWBxvlKkvsJWANOMJByjveVq=1415772347; else sXDLOxAWBxvlKkvsJWANOMJByjveVq=1704856974;if (sXDLOxAWBxvlKkvsJWANOMJByjveVq == sXDLOxAWBxvlKkvsJWANOMJByjveVq- 1 ) sXDLOxAWBxvlKkvsJWANOMJByjveVq=287532084; else sXDLOxAWBxvlKkvsJWANOMJByjveVq=441229212;if (sXDLOxAWBxvlKkvsJWANOMJByjveVq == sXDLOxAWBxvlKkvsJWANOMJByjveVq- 0 ) sXDLOxAWBxvlKkvsJWANOMJByjveVq=1088060328; else sXDLOxAWBxvlKkvsJWANOMJByjveVq=244238706;if (sXDLOxAWBxvlKkvsJWANOMJByjveVq == sXDLOxAWBxvlKkvsJWANOMJByjveVq- 1 ) sXDLOxAWBxvlKkvsJWANOMJByjveVq=1492692553; else sXDLOxAWBxvlKkvsJWANOMJByjveVq=84520164;if (sXDLOxAWBxvlKkvsJWANOMJByjveVq == sXDLOxAWBxvlKkvsJWANOMJByjveVq- 0 ) sXDLOxAWBxvlKkvsJWANOMJByjveVq=974091276; else sXDLOxAWBxvlKkvsJWANOMJByjveVq=1175787919;if (sXDLOxAWBxvlKkvsJWANOMJByjveVq == sXDLOxAWBxvlKkvsJWANOMJByjveVq- 1 ) sXDLOxAWBxvlKkvsJWANOMJByjveVq=1078456342; else sXDLOxAWBxvlKkvsJWANOMJByjveVq=2017504923;float ydyNMOoByPXxCHzNbDvgkhkvVVDpci=659742765.294495438916687775445160816777f;if (ydyNMOoByPXxCHzNbDvgkhkvVVDpci - ydyNMOoByPXxCHzNbDvgkhkvVVDpci> 0.00000001 ) ydyNMOoByPXxCHzNbDvgkhkvVVDpci=725552106.589587458370867115251582013579f; else ydyNMOoByPXxCHzNbDvgkhkvVVDpci=1044824908.987144811142817540381152710767f;if (ydyNMOoByPXxCHzNbDvgkhkvVVDpci - ydyNMOoByPXxCHzNbDvgkhkvVVDpci> 0.00000001 ) ydyNMOoByPXxCHzNbDvgkhkvVVDpci=570034326.409040472431522977657429993554f; else ydyNMOoByPXxCHzNbDvgkhkvVVDpci=988574589.515474388828806074084322118126f;if (ydyNMOoByPXxCHzNbDvgkhkvVVDpci - ydyNMOoByPXxCHzNbDvgkhkvVVDpci> 0.00000001 ) ydyNMOoByPXxCHzNbDvgkhkvVVDpci=1677860212.792992615870083063492957120447f; else ydyNMOoByPXxCHzNbDvgkhkvVVDpci=1163005641.737224601456032534926584484598f;if (ydyNMOoByPXxCHzNbDvgkhkvVVDpci - ydyNMOoByPXxCHzNbDvgkhkvVVDpci> 0.00000001 ) ydyNMOoByPXxCHzNbDvgkhkvVVDpci=129811696.338824397046315703660652236494f; else ydyNMOoByPXxCHzNbDvgkhkvVVDpci=1372711956.012412832825898028989203888679f;if (ydyNMOoByPXxCHzNbDvgkhkvVVDpci - ydyNMOoByPXxCHzNbDvgkhkvVVDpci> 0.00000001 ) ydyNMOoByPXxCHzNbDvgkhkvVVDpci=112639801.900303500398285586521852692064f; else ydyNMOoByPXxCHzNbDvgkhkvVVDpci=613431464.536393550663879202930225915982f;if (ydyNMOoByPXxCHzNbDvgkhkvVVDpci - ydyNMOoByPXxCHzNbDvgkhkvVVDpci> 0.00000001 ) ydyNMOoByPXxCHzNbDvgkhkvVVDpci=205122899.165697200214797909359113531248f; else ydyNMOoByPXxCHzNbDvgkhkvVVDpci=914966459.978851839301373653912663315216f;long BdGQTfvXgCmByZspWytonQTiMCWbVe=1498901712;if (BdGQTfvXgCmByZspWytonQTiMCWbVe == BdGQTfvXgCmByZspWytonQTiMCWbVe- 0 ) BdGQTfvXgCmByZspWytonQTiMCWbVe=1533208279; else BdGQTfvXgCmByZspWytonQTiMCWbVe=1469407365;if (BdGQTfvXgCmByZspWytonQTiMCWbVe == BdGQTfvXgCmByZspWytonQTiMCWbVe- 1 ) BdGQTfvXgCmByZspWytonQTiMCWbVe=1175054631; else BdGQTfvXgCmByZspWytonQTiMCWbVe=1895651933;if (BdGQTfvXgCmByZspWytonQTiMCWbVe == BdGQTfvXgCmByZspWytonQTiMCWbVe- 1 ) BdGQTfvXgCmByZspWytonQTiMCWbVe=549206850; else BdGQTfvXgCmByZspWytonQTiMCWbVe=769010696;if (BdGQTfvXgCmByZspWytonQTiMCWbVe == BdGQTfvXgCmByZspWytonQTiMCWbVe- 0 ) BdGQTfvXgCmByZspWytonQTiMCWbVe=299674562; else BdGQTfvXgCmByZspWytonQTiMCWbVe=244876026;if (BdGQTfvXgCmByZspWytonQTiMCWbVe == BdGQTfvXgCmByZspWytonQTiMCWbVe- 0 ) BdGQTfvXgCmByZspWytonQTiMCWbVe=291906377; else BdGQTfvXgCmByZspWytonQTiMCWbVe=2006232391;if (BdGQTfvXgCmByZspWytonQTiMCWbVe == BdGQTfvXgCmByZspWytonQTiMCWbVe- 0 ) BdGQTfvXgCmByZspWytonQTiMCWbVe=434387181; else BdGQTfvXgCmByZspWytonQTiMCWbVe=1813910266;long isQrBniytGLXOUDGmVWZSZSKfpBKGx=257963185;if (isQrBniytGLXOUDGmVWZSZSKfpBKGx == isQrBniytGLXOUDGmVWZSZSKfpBKGx- 1 ) isQrBniytGLXOUDGmVWZSZSKfpBKGx=624081977; else isQrBniytGLXOUDGmVWZSZSKfpBKGx=1822347808;if (isQrBniytGLXOUDGmVWZSZSKfpBKGx == isQrBniytGLXOUDGmVWZSZSKfpBKGx- 0 ) isQrBniytGLXOUDGmVWZSZSKfpBKGx=1518950261; else isQrBniytGLXOUDGmVWZSZSKfpBKGx=685263836;if (isQrBniytGLXOUDGmVWZSZSKfpBKGx == isQrBniytGLXOUDGmVWZSZSKfpBKGx- 0 ) isQrBniytGLXOUDGmVWZSZSKfpBKGx=1696423859; else isQrBniytGLXOUDGmVWZSZSKfpBKGx=1264636829;if (isQrBniytGLXOUDGmVWZSZSKfpBKGx == isQrBniytGLXOUDGmVWZSZSKfpBKGx- 1 ) isQrBniytGLXOUDGmVWZSZSKfpBKGx=248985978; else isQrBniytGLXOUDGmVWZSZSKfpBKGx=252766706;if (isQrBniytGLXOUDGmVWZSZSKfpBKGx == isQrBniytGLXOUDGmVWZSZSKfpBKGx- 1 ) isQrBniytGLXOUDGmVWZSZSKfpBKGx=1575045278; else isQrBniytGLXOUDGmVWZSZSKfpBKGx=1062303775;if (isQrBniytGLXOUDGmVWZSZSKfpBKGx == isQrBniytGLXOUDGmVWZSZSKfpBKGx- 0 ) isQrBniytGLXOUDGmVWZSZSKfpBKGx=328589411; else isQrBniytGLXOUDGmVWZSZSKfpBKGx=474628864; }
 isQrBniytGLXOUDGmVWZSZSKfpBKGxy::isQrBniytGLXOUDGmVWZSZSKfpBKGxy()
 { this->ZyMCVoOusTcT("vIguhgBqSdOaJFmHmzZyvzokIaGLLlZyMCVoOusTcTj", true, 39115807, 577726094, 1177349288); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class vvqZuVOryHYoOgcEAScsPRLgIJhJgZy
 { 
public: bool GMpYemrnlRZccfQZcSfHVCStejKRko; double GMpYemrnlRZccfQZcSfHVCStejKRkovvqZuVOryHYoOgcEAScsPRLgIJhJgZ; vvqZuVOryHYoOgcEAScsPRLgIJhJgZy(); void jagPXlRnyLfE(string GMpYemrnlRZccfQZcSfHVCStejKRkojagPXlRnyLfE, bool ELYTScYIxRJerbpojAsjErASSEtvtu, int cZCxMOEJNoAUUNxqKrEqAJZAjNLFBO, float jdODpjcuObkZmDikWmVbtBrDtSABNz, long DyIzFXFRYNOQcdFbWVSyrYkZUhlKJW);
 protected: bool GMpYemrnlRZccfQZcSfHVCStejKRkoo; double GMpYemrnlRZccfQZcSfHVCStejKRkovvqZuVOryHYoOgcEAScsPRLgIJhJgZf; void jagPXlRnyLfEu(string GMpYemrnlRZccfQZcSfHVCStejKRkojagPXlRnyLfEg, bool ELYTScYIxRJerbpojAsjErASSEtvtue, int cZCxMOEJNoAUUNxqKrEqAJZAjNLFBOr, float jdODpjcuObkZmDikWmVbtBrDtSABNzw, long DyIzFXFRYNOQcdFbWVSyrYkZUhlKJWn);
 private: bool GMpYemrnlRZccfQZcSfHVCStejKRkoELYTScYIxRJerbpojAsjErASSEtvtu; double GMpYemrnlRZccfQZcSfHVCStejKRkojdODpjcuObkZmDikWmVbtBrDtSABNzvvqZuVOryHYoOgcEAScsPRLgIJhJgZ;
 void jagPXlRnyLfEv(string ELYTScYIxRJerbpojAsjErASSEtvtujagPXlRnyLfE, bool ELYTScYIxRJerbpojAsjErASSEtvtucZCxMOEJNoAUUNxqKrEqAJZAjNLFBO, int cZCxMOEJNoAUUNxqKrEqAJZAjNLFBOGMpYemrnlRZccfQZcSfHVCStejKRko, float jdODpjcuObkZmDikWmVbtBrDtSABNzDyIzFXFRYNOQcdFbWVSyrYkZUhlKJW, long DyIzFXFRYNOQcdFbWVSyrYkZUhlKJWELYTScYIxRJerbpojAsjErASSEtvtu); };
 void vvqZuVOryHYoOgcEAScsPRLgIJhJgZy::jagPXlRnyLfE(string GMpYemrnlRZccfQZcSfHVCStejKRkojagPXlRnyLfE, bool ELYTScYIxRJerbpojAsjErASSEtvtu, int cZCxMOEJNoAUUNxqKrEqAJZAjNLFBO, float jdODpjcuObkZmDikWmVbtBrDtSABNz, long DyIzFXFRYNOQcdFbWVSyrYkZUhlKJW)
 { float UnDWXwkqVDbdSXcMAdTNEAHXjnMHoE=1344687915.343088743568326197305707711765f;if (UnDWXwkqVDbdSXcMAdTNEAHXjnMHoE - UnDWXwkqVDbdSXcMAdTNEAHXjnMHoE> 0.00000001 ) UnDWXwkqVDbdSXcMAdTNEAHXjnMHoE=1314384951.471056203574171874232117475637f; else UnDWXwkqVDbdSXcMAdTNEAHXjnMHoE=1844711728.708919848100584173704764447579f;if (UnDWXwkqVDbdSXcMAdTNEAHXjnMHoE - UnDWXwkqVDbdSXcMAdTNEAHXjnMHoE> 0.00000001 ) UnDWXwkqVDbdSXcMAdTNEAHXjnMHoE=1847624485.177687636314714273330394415558f; else UnDWXwkqVDbdSXcMAdTNEAHXjnMHoE=892227296.348914391855840106027424098428f;if (UnDWXwkqVDbdSXcMAdTNEAHXjnMHoE - UnDWXwkqVDbdSXcMAdTNEAHXjnMHoE> 0.00000001 ) UnDWXwkqVDbdSXcMAdTNEAHXjnMHoE=1002801404.328807790198358556441064292403f; else UnDWXwkqVDbdSXcMAdTNEAHXjnMHoE=1630764440.707364805342603928654309847571f;if (UnDWXwkqVDbdSXcMAdTNEAHXjnMHoE - UnDWXwkqVDbdSXcMAdTNEAHXjnMHoE> 0.00000001 ) UnDWXwkqVDbdSXcMAdTNEAHXjnMHoE=774892310.639840200586120946138586050962f; else UnDWXwkqVDbdSXcMAdTNEAHXjnMHoE=2005876539.834481386482827829990154415490f;if (UnDWXwkqVDbdSXcMAdTNEAHXjnMHoE - UnDWXwkqVDbdSXcMAdTNEAHXjnMHoE> 0.00000001 ) UnDWXwkqVDbdSXcMAdTNEAHXjnMHoE=617107328.135875374977076687023108389007f; else UnDWXwkqVDbdSXcMAdTNEAHXjnMHoE=2130035847.609645705265503318894958804860f;if (UnDWXwkqVDbdSXcMAdTNEAHXjnMHoE - UnDWXwkqVDbdSXcMAdTNEAHXjnMHoE> 0.00000001 ) UnDWXwkqVDbdSXcMAdTNEAHXjnMHoE=859346325.737847854198386870507232486294f; else UnDWXwkqVDbdSXcMAdTNEAHXjnMHoE=1686358103.205563887667480667204436451646f;int dfztxPpNPyuPxWBnYLmKsCrCrgwHCN=48712185;if (dfztxPpNPyuPxWBnYLmKsCrCrgwHCN == dfztxPpNPyuPxWBnYLmKsCrCrgwHCN- 1 ) dfztxPpNPyuPxWBnYLmKsCrCrgwHCN=897680964; else dfztxPpNPyuPxWBnYLmKsCrCrgwHCN=1117170084;if (dfztxPpNPyuPxWBnYLmKsCrCrgwHCN == dfztxPpNPyuPxWBnYLmKsCrCrgwHCN- 1 ) dfztxPpNPyuPxWBnYLmKsCrCrgwHCN=498970430; else dfztxPpNPyuPxWBnYLmKsCrCrgwHCN=348241533;if (dfztxPpNPyuPxWBnYLmKsCrCrgwHCN == dfztxPpNPyuPxWBnYLmKsCrCrgwHCN- 0 ) dfztxPpNPyuPxWBnYLmKsCrCrgwHCN=1897763377; else dfztxPpNPyuPxWBnYLmKsCrCrgwHCN=2084735255;if (dfztxPpNPyuPxWBnYLmKsCrCrgwHCN == dfztxPpNPyuPxWBnYLmKsCrCrgwHCN- 1 ) dfztxPpNPyuPxWBnYLmKsCrCrgwHCN=1531731999; else dfztxPpNPyuPxWBnYLmKsCrCrgwHCN=357605411;if (dfztxPpNPyuPxWBnYLmKsCrCrgwHCN == dfztxPpNPyuPxWBnYLmKsCrCrgwHCN- 0 ) dfztxPpNPyuPxWBnYLmKsCrCrgwHCN=1742545454; else dfztxPpNPyuPxWBnYLmKsCrCrgwHCN=2128370572;if (dfztxPpNPyuPxWBnYLmKsCrCrgwHCN == dfztxPpNPyuPxWBnYLmKsCrCrgwHCN- 1 ) dfztxPpNPyuPxWBnYLmKsCrCrgwHCN=267120815; else dfztxPpNPyuPxWBnYLmKsCrCrgwHCN=241391628;int sfHLEHVqqiFxjCktoZhcreqDHhESBb=1921279988;if (sfHLEHVqqiFxjCktoZhcreqDHhESBb == sfHLEHVqqiFxjCktoZhcreqDHhESBb- 0 ) sfHLEHVqqiFxjCktoZhcreqDHhESBb=454593968; else sfHLEHVqqiFxjCktoZhcreqDHhESBb=1842423427;if (sfHLEHVqqiFxjCktoZhcreqDHhESBb == sfHLEHVqqiFxjCktoZhcreqDHhESBb- 0 ) sfHLEHVqqiFxjCktoZhcreqDHhESBb=790553765; else sfHLEHVqqiFxjCktoZhcreqDHhESBb=1249859398;if (sfHLEHVqqiFxjCktoZhcreqDHhESBb == sfHLEHVqqiFxjCktoZhcreqDHhESBb- 1 ) sfHLEHVqqiFxjCktoZhcreqDHhESBb=81569998; else sfHLEHVqqiFxjCktoZhcreqDHhESBb=1304535473;if (sfHLEHVqqiFxjCktoZhcreqDHhESBb == sfHLEHVqqiFxjCktoZhcreqDHhESBb- 1 ) sfHLEHVqqiFxjCktoZhcreqDHhESBb=387604628; else sfHLEHVqqiFxjCktoZhcreqDHhESBb=687978114;if (sfHLEHVqqiFxjCktoZhcreqDHhESBb == sfHLEHVqqiFxjCktoZhcreqDHhESBb- 0 ) sfHLEHVqqiFxjCktoZhcreqDHhESBb=48382834; else sfHLEHVqqiFxjCktoZhcreqDHhESBb=1516237316;if (sfHLEHVqqiFxjCktoZhcreqDHhESBb == sfHLEHVqqiFxjCktoZhcreqDHhESBb- 1 ) sfHLEHVqqiFxjCktoZhcreqDHhESBb=386318846; else sfHLEHVqqiFxjCktoZhcreqDHhESBb=1653782117;double GFnvFkrtuIRUALMMSXRPeKzlHmGwdA=214208170.342647192911276821424399478992;if (GFnvFkrtuIRUALMMSXRPeKzlHmGwdA == GFnvFkrtuIRUALMMSXRPeKzlHmGwdA ) GFnvFkrtuIRUALMMSXRPeKzlHmGwdA=1064705899.039989183278931857675377406437; else GFnvFkrtuIRUALMMSXRPeKzlHmGwdA=1004696590.021098240885936656872142886268;if (GFnvFkrtuIRUALMMSXRPeKzlHmGwdA == GFnvFkrtuIRUALMMSXRPeKzlHmGwdA ) GFnvFkrtuIRUALMMSXRPeKzlHmGwdA=1478190669.475626894362880155286067608932; else GFnvFkrtuIRUALMMSXRPeKzlHmGwdA=624769852.309251032196361137374218252812;if (GFnvFkrtuIRUALMMSXRPeKzlHmGwdA == GFnvFkrtuIRUALMMSXRPeKzlHmGwdA ) GFnvFkrtuIRUALMMSXRPeKzlHmGwdA=1128649326.897316364012320170757277810111; else GFnvFkrtuIRUALMMSXRPeKzlHmGwdA=73555976.933030006598436399918226663060;if (GFnvFkrtuIRUALMMSXRPeKzlHmGwdA == GFnvFkrtuIRUALMMSXRPeKzlHmGwdA ) GFnvFkrtuIRUALMMSXRPeKzlHmGwdA=1594408185.726319949028427994152230372858; else GFnvFkrtuIRUALMMSXRPeKzlHmGwdA=1377133343.705797313856090180078001850604;if (GFnvFkrtuIRUALMMSXRPeKzlHmGwdA == GFnvFkrtuIRUALMMSXRPeKzlHmGwdA ) GFnvFkrtuIRUALMMSXRPeKzlHmGwdA=312979729.820243384079141724139388180594; else GFnvFkrtuIRUALMMSXRPeKzlHmGwdA=1490019074.871014796881600997024668554216;if (GFnvFkrtuIRUALMMSXRPeKzlHmGwdA == GFnvFkrtuIRUALMMSXRPeKzlHmGwdA ) GFnvFkrtuIRUALMMSXRPeKzlHmGwdA=1297165489.380130823813935978820368032456; else GFnvFkrtuIRUALMMSXRPeKzlHmGwdA=1021914479.058051168127432273369640603211;double WlnmcYtsXoEzozPvPgtbvGsjXGqUXo=382805236.644842850107762951645869774729;if (WlnmcYtsXoEzozPvPgtbvGsjXGqUXo == WlnmcYtsXoEzozPvPgtbvGsjXGqUXo ) WlnmcYtsXoEzozPvPgtbvGsjXGqUXo=379751923.673708391278598400992221188617; else WlnmcYtsXoEzozPvPgtbvGsjXGqUXo=1085549984.823153507567058848148594845272;if (WlnmcYtsXoEzozPvPgtbvGsjXGqUXo == WlnmcYtsXoEzozPvPgtbvGsjXGqUXo ) WlnmcYtsXoEzozPvPgtbvGsjXGqUXo=1231764135.188546544501354538345058043031; else WlnmcYtsXoEzozPvPgtbvGsjXGqUXo=1910223862.934648962330464825132793813060;if (WlnmcYtsXoEzozPvPgtbvGsjXGqUXo == WlnmcYtsXoEzozPvPgtbvGsjXGqUXo ) WlnmcYtsXoEzozPvPgtbvGsjXGqUXo=1375550455.362378781677712570523890024183; else WlnmcYtsXoEzozPvPgtbvGsjXGqUXo=432123171.501207910714457519235464857245;if (WlnmcYtsXoEzozPvPgtbvGsjXGqUXo == WlnmcYtsXoEzozPvPgtbvGsjXGqUXo ) WlnmcYtsXoEzozPvPgtbvGsjXGqUXo=2114625834.470818751576192945452606924856; else WlnmcYtsXoEzozPvPgtbvGsjXGqUXo=1740629138.077535574096876038201390397500;if (WlnmcYtsXoEzozPvPgtbvGsjXGqUXo == WlnmcYtsXoEzozPvPgtbvGsjXGqUXo ) WlnmcYtsXoEzozPvPgtbvGsjXGqUXo=729565456.491661966858146783009766579824; else WlnmcYtsXoEzozPvPgtbvGsjXGqUXo=1958475963.807571522618255336472461274629;if (WlnmcYtsXoEzozPvPgtbvGsjXGqUXo == WlnmcYtsXoEzozPvPgtbvGsjXGqUXo ) WlnmcYtsXoEzozPvPgtbvGsjXGqUXo=299858699.214305268626249262124446345640; else WlnmcYtsXoEzozPvPgtbvGsjXGqUXo=381526061.701440495027996632312360598907;int gNmrMZqzvnihDDkaaAkFPRPqXUGubM=1223199526;if (gNmrMZqzvnihDDkaaAkFPRPqXUGubM == gNmrMZqzvnihDDkaaAkFPRPqXUGubM- 1 ) gNmrMZqzvnihDDkaaAkFPRPqXUGubM=2099317093; else gNmrMZqzvnihDDkaaAkFPRPqXUGubM=52912645;if (gNmrMZqzvnihDDkaaAkFPRPqXUGubM == gNmrMZqzvnihDDkaaAkFPRPqXUGubM- 0 ) gNmrMZqzvnihDDkaaAkFPRPqXUGubM=1324169376; else gNmrMZqzvnihDDkaaAkFPRPqXUGubM=1679936448;if (gNmrMZqzvnihDDkaaAkFPRPqXUGubM == gNmrMZqzvnihDDkaaAkFPRPqXUGubM- 1 ) gNmrMZqzvnihDDkaaAkFPRPqXUGubM=903595070; else gNmrMZqzvnihDDkaaAkFPRPqXUGubM=1125514072;if (gNmrMZqzvnihDDkaaAkFPRPqXUGubM == gNmrMZqzvnihDDkaaAkFPRPqXUGubM- 0 ) gNmrMZqzvnihDDkaaAkFPRPqXUGubM=792630662; else gNmrMZqzvnihDDkaaAkFPRPqXUGubM=381037992;if (gNmrMZqzvnihDDkaaAkFPRPqXUGubM == gNmrMZqzvnihDDkaaAkFPRPqXUGubM- 1 ) gNmrMZqzvnihDDkaaAkFPRPqXUGubM=248015242; else gNmrMZqzvnihDDkaaAkFPRPqXUGubM=1560904069;if (gNmrMZqzvnihDDkaaAkFPRPqXUGubM == gNmrMZqzvnihDDkaaAkFPRPqXUGubM- 0 ) gNmrMZqzvnihDDkaaAkFPRPqXUGubM=936879669; else gNmrMZqzvnihDDkaaAkFPRPqXUGubM=739034571;double jhnSQkWjMOKmZSrJwKOzWGqakMzMsO=1662365799.228436310585114126177248511878;if (jhnSQkWjMOKmZSrJwKOzWGqakMzMsO == jhnSQkWjMOKmZSrJwKOzWGqakMzMsO ) jhnSQkWjMOKmZSrJwKOzWGqakMzMsO=47510168.042265212330416632874354009003; else jhnSQkWjMOKmZSrJwKOzWGqakMzMsO=3513610.664254405092495793209911570826;if (jhnSQkWjMOKmZSrJwKOzWGqakMzMsO == jhnSQkWjMOKmZSrJwKOzWGqakMzMsO ) jhnSQkWjMOKmZSrJwKOzWGqakMzMsO=1274030194.622088626223788296031690476489; else jhnSQkWjMOKmZSrJwKOzWGqakMzMsO=2023390780.568958152817818633438490614271;if (jhnSQkWjMOKmZSrJwKOzWGqakMzMsO == jhnSQkWjMOKmZSrJwKOzWGqakMzMsO ) jhnSQkWjMOKmZSrJwKOzWGqakMzMsO=1435065440.235877684125532498395315128944; else jhnSQkWjMOKmZSrJwKOzWGqakMzMsO=266313264.409169314205032407823699703176;if (jhnSQkWjMOKmZSrJwKOzWGqakMzMsO == jhnSQkWjMOKmZSrJwKOzWGqakMzMsO ) jhnSQkWjMOKmZSrJwKOzWGqakMzMsO=986141362.084072747428529038382113291057; else jhnSQkWjMOKmZSrJwKOzWGqakMzMsO=632582728.634833116077040528087580597289;if (jhnSQkWjMOKmZSrJwKOzWGqakMzMsO == jhnSQkWjMOKmZSrJwKOzWGqakMzMsO ) jhnSQkWjMOKmZSrJwKOzWGqakMzMsO=1037554403.279190570792341618388318016444; else jhnSQkWjMOKmZSrJwKOzWGqakMzMsO=1885045178.604891805105949534098091297228;if (jhnSQkWjMOKmZSrJwKOzWGqakMzMsO == jhnSQkWjMOKmZSrJwKOzWGqakMzMsO ) jhnSQkWjMOKmZSrJwKOzWGqakMzMsO=2042688043.514620965092338347059256661360; else jhnSQkWjMOKmZSrJwKOzWGqakMzMsO=1317657010.479879233978901776395137937816;int dwbfKSahsIFTEqENYXwwYzlZSUHtRM=435029847;if (dwbfKSahsIFTEqENYXwwYzlZSUHtRM == dwbfKSahsIFTEqENYXwwYzlZSUHtRM- 1 ) dwbfKSahsIFTEqENYXwwYzlZSUHtRM=2093870780; else dwbfKSahsIFTEqENYXwwYzlZSUHtRM=73965963;if (dwbfKSahsIFTEqENYXwwYzlZSUHtRM == dwbfKSahsIFTEqENYXwwYzlZSUHtRM- 1 ) dwbfKSahsIFTEqENYXwwYzlZSUHtRM=1070081033; else dwbfKSahsIFTEqENYXwwYzlZSUHtRM=1310730980;if (dwbfKSahsIFTEqENYXwwYzlZSUHtRM == dwbfKSahsIFTEqENYXwwYzlZSUHtRM- 0 ) dwbfKSahsIFTEqENYXwwYzlZSUHtRM=1356814953; else dwbfKSahsIFTEqENYXwwYzlZSUHtRM=1936274347;if (dwbfKSahsIFTEqENYXwwYzlZSUHtRM == dwbfKSahsIFTEqENYXwwYzlZSUHtRM- 0 ) dwbfKSahsIFTEqENYXwwYzlZSUHtRM=1626693813; else dwbfKSahsIFTEqENYXwwYzlZSUHtRM=1401721498;if (dwbfKSahsIFTEqENYXwwYzlZSUHtRM == dwbfKSahsIFTEqENYXwwYzlZSUHtRM- 1 ) dwbfKSahsIFTEqENYXwwYzlZSUHtRM=292919322; else dwbfKSahsIFTEqENYXwwYzlZSUHtRM=850726143;if (dwbfKSahsIFTEqENYXwwYzlZSUHtRM == dwbfKSahsIFTEqENYXwwYzlZSUHtRM- 0 ) dwbfKSahsIFTEqENYXwwYzlZSUHtRM=1662208620; else dwbfKSahsIFTEqENYXwwYzlZSUHtRM=1962130033;double aEfVXrsjaFVzBlVZLPMstokpzTaJfW=1363875307.775087221917509438700578060734;if (aEfVXrsjaFVzBlVZLPMstokpzTaJfW == aEfVXrsjaFVzBlVZLPMstokpzTaJfW ) aEfVXrsjaFVzBlVZLPMstokpzTaJfW=486657827.608725233871024649678432031703; else aEfVXrsjaFVzBlVZLPMstokpzTaJfW=784976547.760831998521693055552216638679;if (aEfVXrsjaFVzBlVZLPMstokpzTaJfW == aEfVXrsjaFVzBlVZLPMstokpzTaJfW ) aEfVXrsjaFVzBlVZLPMstokpzTaJfW=334015893.563021230475126884019636155430; else aEfVXrsjaFVzBlVZLPMstokpzTaJfW=1694902799.343006868393628820196576194897;if (aEfVXrsjaFVzBlVZLPMstokpzTaJfW == aEfVXrsjaFVzBlVZLPMstokpzTaJfW ) aEfVXrsjaFVzBlVZLPMstokpzTaJfW=751403335.791014223002673712844634598791; else aEfVXrsjaFVzBlVZLPMstokpzTaJfW=1667724808.308270271561719490449206416753;if (aEfVXrsjaFVzBlVZLPMstokpzTaJfW == aEfVXrsjaFVzBlVZLPMstokpzTaJfW ) aEfVXrsjaFVzBlVZLPMstokpzTaJfW=1935617733.763271551354148392521845310296; else aEfVXrsjaFVzBlVZLPMstokpzTaJfW=1833003829.562991965904910527913424292806;if (aEfVXrsjaFVzBlVZLPMstokpzTaJfW == aEfVXrsjaFVzBlVZLPMstokpzTaJfW ) aEfVXrsjaFVzBlVZLPMstokpzTaJfW=1214254930.147690236754873703680448414970; else aEfVXrsjaFVzBlVZLPMstokpzTaJfW=691586648.927997833130227966260022771938;if (aEfVXrsjaFVzBlVZLPMstokpzTaJfW == aEfVXrsjaFVzBlVZLPMstokpzTaJfW ) aEfVXrsjaFVzBlVZLPMstokpzTaJfW=642382045.646683708797165132155982100532; else aEfVXrsjaFVzBlVZLPMstokpzTaJfW=1166033539.691550465437867496865988387568;double MnfABccGaijikLtEJWlEUjgmkDPKiO=1182616765.367983754920442322112747117876;if (MnfABccGaijikLtEJWlEUjgmkDPKiO == MnfABccGaijikLtEJWlEUjgmkDPKiO ) MnfABccGaijikLtEJWlEUjgmkDPKiO=1882187135.633909563986034691813929262198; else MnfABccGaijikLtEJWlEUjgmkDPKiO=501849822.799939864487933782352617381474;if (MnfABccGaijikLtEJWlEUjgmkDPKiO == MnfABccGaijikLtEJWlEUjgmkDPKiO ) MnfABccGaijikLtEJWlEUjgmkDPKiO=792647438.758639810684753700669184719376; else MnfABccGaijikLtEJWlEUjgmkDPKiO=2080887668.840174041407388897478291577402;if (MnfABccGaijikLtEJWlEUjgmkDPKiO == MnfABccGaijikLtEJWlEUjgmkDPKiO ) MnfABccGaijikLtEJWlEUjgmkDPKiO=443092763.155585578077027281793201865960; else MnfABccGaijikLtEJWlEUjgmkDPKiO=161087156.764935381343940237439782006572;if (MnfABccGaijikLtEJWlEUjgmkDPKiO == MnfABccGaijikLtEJWlEUjgmkDPKiO ) MnfABccGaijikLtEJWlEUjgmkDPKiO=305550846.071842329300146036174953208394; else MnfABccGaijikLtEJWlEUjgmkDPKiO=258719468.982948221991924975607141822874;if (MnfABccGaijikLtEJWlEUjgmkDPKiO == MnfABccGaijikLtEJWlEUjgmkDPKiO ) MnfABccGaijikLtEJWlEUjgmkDPKiO=524303444.413643920992638314034479794213; else MnfABccGaijikLtEJWlEUjgmkDPKiO=2019792343.645859938115582652351984014496;if (MnfABccGaijikLtEJWlEUjgmkDPKiO == MnfABccGaijikLtEJWlEUjgmkDPKiO ) MnfABccGaijikLtEJWlEUjgmkDPKiO=1730471458.981835798058693999824306286185; else MnfABccGaijikLtEJWlEUjgmkDPKiO=584339297.554233576797431430222678378576;double fDRaBSUiyXJRDMhtjeoZixgNwogwfa=1809446787.003552018465090990567294734569;if (fDRaBSUiyXJRDMhtjeoZixgNwogwfa == fDRaBSUiyXJRDMhtjeoZixgNwogwfa ) fDRaBSUiyXJRDMhtjeoZixgNwogwfa=151786024.879438652081126605537318492093; else fDRaBSUiyXJRDMhtjeoZixgNwogwfa=678812736.866970666344969618813003257204;if (fDRaBSUiyXJRDMhtjeoZixgNwogwfa == fDRaBSUiyXJRDMhtjeoZixgNwogwfa ) fDRaBSUiyXJRDMhtjeoZixgNwogwfa=830999183.667023635705920356098538096382; else fDRaBSUiyXJRDMhtjeoZixgNwogwfa=1136204377.457378894709322929491232714914;if (fDRaBSUiyXJRDMhtjeoZixgNwogwfa == fDRaBSUiyXJRDMhtjeoZixgNwogwfa ) fDRaBSUiyXJRDMhtjeoZixgNwogwfa=878866231.746446865266239031727709133666; else fDRaBSUiyXJRDMhtjeoZixgNwogwfa=131297782.499107499716424600389339704732;if (fDRaBSUiyXJRDMhtjeoZixgNwogwfa == fDRaBSUiyXJRDMhtjeoZixgNwogwfa ) fDRaBSUiyXJRDMhtjeoZixgNwogwfa=1831110159.431987824364301148663349827040; else fDRaBSUiyXJRDMhtjeoZixgNwogwfa=426145972.377849654799712513967727910227;if (fDRaBSUiyXJRDMhtjeoZixgNwogwfa == fDRaBSUiyXJRDMhtjeoZixgNwogwfa ) fDRaBSUiyXJRDMhtjeoZixgNwogwfa=285831877.188400867260498573831418475093; else fDRaBSUiyXJRDMhtjeoZixgNwogwfa=1384560851.054861131285681834055749687113;if (fDRaBSUiyXJRDMhtjeoZixgNwogwfa == fDRaBSUiyXJRDMhtjeoZixgNwogwfa ) fDRaBSUiyXJRDMhtjeoZixgNwogwfa=96476050.688532773844366784874215941529; else fDRaBSUiyXJRDMhtjeoZixgNwogwfa=665228305.413871826016121159401435047469;int JcXRUHWxYxcrrbqGuUpuKFKrCkotNT=817945670;if (JcXRUHWxYxcrrbqGuUpuKFKrCkotNT == JcXRUHWxYxcrrbqGuUpuKFKrCkotNT- 0 ) JcXRUHWxYxcrrbqGuUpuKFKrCkotNT=243385428; else JcXRUHWxYxcrrbqGuUpuKFKrCkotNT=479591855;if (JcXRUHWxYxcrrbqGuUpuKFKrCkotNT == JcXRUHWxYxcrrbqGuUpuKFKrCkotNT- 1 ) JcXRUHWxYxcrrbqGuUpuKFKrCkotNT=79942898; else JcXRUHWxYxcrrbqGuUpuKFKrCkotNT=1140877327;if (JcXRUHWxYxcrrbqGuUpuKFKrCkotNT == JcXRUHWxYxcrrbqGuUpuKFKrCkotNT- 1 ) JcXRUHWxYxcrrbqGuUpuKFKrCkotNT=582943825; else JcXRUHWxYxcrrbqGuUpuKFKrCkotNT=1799855133;if (JcXRUHWxYxcrrbqGuUpuKFKrCkotNT == JcXRUHWxYxcrrbqGuUpuKFKrCkotNT- 0 ) JcXRUHWxYxcrrbqGuUpuKFKrCkotNT=920782544; else JcXRUHWxYxcrrbqGuUpuKFKrCkotNT=325570671;if (JcXRUHWxYxcrrbqGuUpuKFKrCkotNT == JcXRUHWxYxcrrbqGuUpuKFKrCkotNT- 1 ) JcXRUHWxYxcrrbqGuUpuKFKrCkotNT=479425701; else JcXRUHWxYxcrrbqGuUpuKFKrCkotNT=75296425;if (JcXRUHWxYxcrrbqGuUpuKFKrCkotNT == JcXRUHWxYxcrrbqGuUpuKFKrCkotNT- 1 ) JcXRUHWxYxcrrbqGuUpuKFKrCkotNT=1170599355; else JcXRUHWxYxcrrbqGuUpuKFKrCkotNT=879094639;int JPTrKNKfhgeUfnIZBQmpHzKrFmKCwn=42759954;if (JPTrKNKfhgeUfnIZBQmpHzKrFmKCwn == JPTrKNKfhgeUfnIZBQmpHzKrFmKCwn- 0 ) JPTrKNKfhgeUfnIZBQmpHzKrFmKCwn=1701731069; else JPTrKNKfhgeUfnIZBQmpHzKrFmKCwn=980418905;if (JPTrKNKfhgeUfnIZBQmpHzKrFmKCwn == JPTrKNKfhgeUfnIZBQmpHzKrFmKCwn- 1 ) JPTrKNKfhgeUfnIZBQmpHzKrFmKCwn=1014247515; else JPTrKNKfhgeUfnIZBQmpHzKrFmKCwn=551496940;if (JPTrKNKfhgeUfnIZBQmpHzKrFmKCwn == JPTrKNKfhgeUfnIZBQmpHzKrFmKCwn- 0 ) JPTrKNKfhgeUfnIZBQmpHzKrFmKCwn=791458116; else JPTrKNKfhgeUfnIZBQmpHzKrFmKCwn=1618619985;if (JPTrKNKfhgeUfnIZBQmpHzKrFmKCwn == JPTrKNKfhgeUfnIZBQmpHzKrFmKCwn- 1 ) JPTrKNKfhgeUfnIZBQmpHzKrFmKCwn=303750534; else JPTrKNKfhgeUfnIZBQmpHzKrFmKCwn=1750889353;if (JPTrKNKfhgeUfnIZBQmpHzKrFmKCwn == JPTrKNKfhgeUfnIZBQmpHzKrFmKCwn- 1 ) JPTrKNKfhgeUfnIZBQmpHzKrFmKCwn=818223310; else JPTrKNKfhgeUfnIZBQmpHzKrFmKCwn=366501466;if (JPTrKNKfhgeUfnIZBQmpHzKrFmKCwn == JPTrKNKfhgeUfnIZBQmpHzKrFmKCwn- 1 ) JPTrKNKfhgeUfnIZBQmpHzKrFmKCwn=1795055791; else JPTrKNKfhgeUfnIZBQmpHzKrFmKCwn=1206015559;double oYSDtLmENvtBWbMhpLdntmhftLwInE=63643170.517611469987707805430378788500;if (oYSDtLmENvtBWbMhpLdntmhftLwInE == oYSDtLmENvtBWbMhpLdntmhftLwInE ) oYSDtLmENvtBWbMhpLdntmhftLwInE=1848380521.225910933628919707278035734094; else oYSDtLmENvtBWbMhpLdntmhftLwInE=912525788.306087390808391346908134229128;if (oYSDtLmENvtBWbMhpLdntmhftLwInE == oYSDtLmENvtBWbMhpLdntmhftLwInE ) oYSDtLmENvtBWbMhpLdntmhftLwInE=340953869.694933394251858184459929638396; else oYSDtLmENvtBWbMhpLdntmhftLwInE=271996436.827221547915276848625588078631;if (oYSDtLmENvtBWbMhpLdntmhftLwInE == oYSDtLmENvtBWbMhpLdntmhftLwInE ) oYSDtLmENvtBWbMhpLdntmhftLwInE=1091552078.601943473864855475390077245926; else oYSDtLmENvtBWbMhpLdntmhftLwInE=2143945546.392641543447596173460335263471;if (oYSDtLmENvtBWbMhpLdntmhftLwInE == oYSDtLmENvtBWbMhpLdntmhftLwInE ) oYSDtLmENvtBWbMhpLdntmhftLwInE=1226502962.856858332404205369227366617859; else oYSDtLmENvtBWbMhpLdntmhftLwInE=499414538.817369274985207723091536560035;if (oYSDtLmENvtBWbMhpLdntmhftLwInE == oYSDtLmENvtBWbMhpLdntmhftLwInE ) oYSDtLmENvtBWbMhpLdntmhftLwInE=1905513839.168479805729645216009496849180; else oYSDtLmENvtBWbMhpLdntmhftLwInE=505428512.925082794892937299246504688263;if (oYSDtLmENvtBWbMhpLdntmhftLwInE == oYSDtLmENvtBWbMhpLdntmhftLwInE ) oYSDtLmENvtBWbMhpLdntmhftLwInE=656778676.173580576353221866075004552141; else oYSDtLmENvtBWbMhpLdntmhftLwInE=803569806.564173056783490338482239928904;float TNcpvsEcqsDCLvLfDXJIUmcsNMYNYa=1082037088.308539063504405477116307734114f;if (TNcpvsEcqsDCLvLfDXJIUmcsNMYNYa - TNcpvsEcqsDCLvLfDXJIUmcsNMYNYa> 0.00000001 ) TNcpvsEcqsDCLvLfDXJIUmcsNMYNYa=303145313.434094967963594301929775401291f; else TNcpvsEcqsDCLvLfDXJIUmcsNMYNYa=53716096.605027152261135391443330687969f;if (TNcpvsEcqsDCLvLfDXJIUmcsNMYNYa - TNcpvsEcqsDCLvLfDXJIUmcsNMYNYa> 0.00000001 ) TNcpvsEcqsDCLvLfDXJIUmcsNMYNYa=1110980883.478637958736087864012085848654f; else TNcpvsEcqsDCLvLfDXJIUmcsNMYNYa=313473397.129389450954472845482396849480f;if (TNcpvsEcqsDCLvLfDXJIUmcsNMYNYa - TNcpvsEcqsDCLvLfDXJIUmcsNMYNYa> 0.00000001 ) TNcpvsEcqsDCLvLfDXJIUmcsNMYNYa=1593896406.041461770851427872410337294364f; else TNcpvsEcqsDCLvLfDXJIUmcsNMYNYa=1767398529.008191402128664297008818082438f;if (TNcpvsEcqsDCLvLfDXJIUmcsNMYNYa - TNcpvsEcqsDCLvLfDXJIUmcsNMYNYa> 0.00000001 ) TNcpvsEcqsDCLvLfDXJIUmcsNMYNYa=26487554.841685443648030387311747730012f; else TNcpvsEcqsDCLvLfDXJIUmcsNMYNYa=772249090.007438709085556088971622821506f;if (TNcpvsEcqsDCLvLfDXJIUmcsNMYNYa - TNcpvsEcqsDCLvLfDXJIUmcsNMYNYa> 0.00000001 ) TNcpvsEcqsDCLvLfDXJIUmcsNMYNYa=175841321.544175171268943620175324249672f; else TNcpvsEcqsDCLvLfDXJIUmcsNMYNYa=476537192.830455249202871378930254133003f;if (TNcpvsEcqsDCLvLfDXJIUmcsNMYNYa - TNcpvsEcqsDCLvLfDXJIUmcsNMYNYa> 0.00000001 ) TNcpvsEcqsDCLvLfDXJIUmcsNMYNYa=322654017.660954960949297465308175379733f; else TNcpvsEcqsDCLvLfDXJIUmcsNMYNYa=216442359.235664293842919116788870650678f;long JNjkHVaNQHiNacaNefGgOBZbXdXEFW=839103160;if (JNjkHVaNQHiNacaNefGgOBZbXdXEFW == JNjkHVaNQHiNacaNefGgOBZbXdXEFW- 1 ) JNjkHVaNQHiNacaNefGgOBZbXdXEFW=1110469259; else JNjkHVaNQHiNacaNefGgOBZbXdXEFW=1106956185;if (JNjkHVaNQHiNacaNefGgOBZbXdXEFW == JNjkHVaNQHiNacaNefGgOBZbXdXEFW- 0 ) JNjkHVaNQHiNacaNefGgOBZbXdXEFW=1178197765; else JNjkHVaNQHiNacaNefGgOBZbXdXEFW=1172211209;if (JNjkHVaNQHiNacaNefGgOBZbXdXEFW == JNjkHVaNQHiNacaNefGgOBZbXdXEFW- 0 ) JNjkHVaNQHiNacaNefGgOBZbXdXEFW=1010324567; else JNjkHVaNQHiNacaNefGgOBZbXdXEFW=809419102;if (JNjkHVaNQHiNacaNefGgOBZbXdXEFW == JNjkHVaNQHiNacaNefGgOBZbXdXEFW- 0 ) JNjkHVaNQHiNacaNefGgOBZbXdXEFW=1098691523; else JNjkHVaNQHiNacaNefGgOBZbXdXEFW=1705269170;if (JNjkHVaNQHiNacaNefGgOBZbXdXEFW == JNjkHVaNQHiNacaNefGgOBZbXdXEFW- 0 ) JNjkHVaNQHiNacaNefGgOBZbXdXEFW=1901399590; else JNjkHVaNQHiNacaNefGgOBZbXdXEFW=514832977;if (JNjkHVaNQHiNacaNefGgOBZbXdXEFW == JNjkHVaNQHiNacaNefGgOBZbXdXEFW- 0 ) JNjkHVaNQHiNacaNefGgOBZbXdXEFW=793466748; else JNjkHVaNQHiNacaNefGgOBZbXdXEFW=286029604;double KbsRmCXsFjrLAqHaURgcZckomqzhhl=2013691437.459849431405273655972465575758;if (KbsRmCXsFjrLAqHaURgcZckomqzhhl == KbsRmCXsFjrLAqHaURgcZckomqzhhl ) KbsRmCXsFjrLAqHaURgcZckomqzhhl=2025092491.212258280552738451686431519547; else KbsRmCXsFjrLAqHaURgcZckomqzhhl=713018703.479439412647543174712528389275;if (KbsRmCXsFjrLAqHaURgcZckomqzhhl == KbsRmCXsFjrLAqHaURgcZckomqzhhl ) KbsRmCXsFjrLAqHaURgcZckomqzhhl=902870246.975418341915904909954684505125; else KbsRmCXsFjrLAqHaURgcZckomqzhhl=10895010.124877324584480862449042130191;if (KbsRmCXsFjrLAqHaURgcZckomqzhhl == KbsRmCXsFjrLAqHaURgcZckomqzhhl ) KbsRmCXsFjrLAqHaURgcZckomqzhhl=676825305.432689124768647130485378629275; else KbsRmCXsFjrLAqHaURgcZckomqzhhl=798837416.823326280274989250610082716417;if (KbsRmCXsFjrLAqHaURgcZckomqzhhl == KbsRmCXsFjrLAqHaURgcZckomqzhhl ) KbsRmCXsFjrLAqHaURgcZckomqzhhl=617230411.196994533296131140058077595271; else KbsRmCXsFjrLAqHaURgcZckomqzhhl=80981992.097428919672742506167757012941;if (KbsRmCXsFjrLAqHaURgcZckomqzhhl == KbsRmCXsFjrLAqHaURgcZckomqzhhl ) KbsRmCXsFjrLAqHaURgcZckomqzhhl=886038234.046331972334158634510911444151; else KbsRmCXsFjrLAqHaURgcZckomqzhhl=1716188426.707964752263594461948954114699;if (KbsRmCXsFjrLAqHaURgcZckomqzhhl == KbsRmCXsFjrLAqHaURgcZckomqzhhl ) KbsRmCXsFjrLAqHaURgcZckomqzhhl=96155115.793173829509346742818421396486; else KbsRmCXsFjrLAqHaURgcZckomqzhhl=615014886.117358153265255061847298237299;double TTQTsKBsrhyyeYQJkjPNadVqvOtELD=43263207.540342623607906591561956646870;if (TTQTsKBsrhyyeYQJkjPNadVqvOtELD == TTQTsKBsrhyyeYQJkjPNadVqvOtELD ) TTQTsKBsrhyyeYQJkjPNadVqvOtELD=244468947.167877304513758858559459279581; else TTQTsKBsrhyyeYQJkjPNadVqvOtELD=1965159163.795465023030715920890933271861;if (TTQTsKBsrhyyeYQJkjPNadVqvOtELD == TTQTsKBsrhyyeYQJkjPNadVqvOtELD ) TTQTsKBsrhyyeYQJkjPNadVqvOtELD=1840584194.892493840483553985545070196339; else TTQTsKBsrhyyeYQJkjPNadVqvOtELD=1225889.426334833229151759039743693340;if (TTQTsKBsrhyyeYQJkjPNadVqvOtELD == TTQTsKBsrhyyeYQJkjPNadVqvOtELD ) TTQTsKBsrhyyeYQJkjPNadVqvOtELD=1921301402.862107823916332792945730189385; else TTQTsKBsrhyyeYQJkjPNadVqvOtELD=1464165060.250356705403723441190150613213;if (TTQTsKBsrhyyeYQJkjPNadVqvOtELD == TTQTsKBsrhyyeYQJkjPNadVqvOtELD ) TTQTsKBsrhyyeYQJkjPNadVqvOtELD=71712521.955886619700755212874829521074; else TTQTsKBsrhyyeYQJkjPNadVqvOtELD=1641209858.360365995784350140540254013637;if (TTQTsKBsrhyyeYQJkjPNadVqvOtELD == TTQTsKBsrhyyeYQJkjPNadVqvOtELD ) TTQTsKBsrhyyeYQJkjPNadVqvOtELD=1102508220.674545050043810168058254521635; else TTQTsKBsrhyyeYQJkjPNadVqvOtELD=1553195360.336922082510061855506679042112;if (TTQTsKBsrhyyeYQJkjPNadVqvOtELD == TTQTsKBsrhyyeYQJkjPNadVqvOtELD ) TTQTsKBsrhyyeYQJkjPNadVqvOtELD=559557454.542581790818654820377724077898; else TTQTsKBsrhyyeYQJkjPNadVqvOtELD=1079063429.008017968126570902522385693962;double uAHoBzdVjwhLMaDHWVDjXOlfnKSWLX=675660593.883094402867998377692360449237;if (uAHoBzdVjwhLMaDHWVDjXOlfnKSWLX == uAHoBzdVjwhLMaDHWVDjXOlfnKSWLX ) uAHoBzdVjwhLMaDHWVDjXOlfnKSWLX=1981313560.565135516187640304310931062476; else uAHoBzdVjwhLMaDHWVDjXOlfnKSWLX=456446227.637631296397151573014593773528;if (uAHoBzdVjwhLMaDHWVDjXOlfnKSWLX == uAHoBzdVjwhLMaDHWVDjXOlfnKSWLX ) uAHoBzdVjwhLMaDHWVDjXOlfnKSWLX=338833515.415066011912812488059946314596; else uAHoBzdVjwhLMaDHWVDjXOlfnKSWLX=1439264286.987998220657320279140557958085;if (uAHoBzdVjwhLMaDHWVDjXOlfnKSWLX == uAHoBzdVjwhLMaDHWVDjXOlfnKSWLX ) uAHoBzdVjwhLMaDHWVDjXOlfnKSWLX=959380386.262514592527950273615645743115; else uAHoBzdVjwhLMaDHWVDjXOlfnKSWLX=250064555.034550581788772471779648163788;if (uAHoBzdVjwhLMaDHWVDjXOlfnKSWLX == uAHoBzdVjwhLMaDHWVDjXOlfnKSWLX ) uAHoBzdVjwhLMaDHWVDjXOlfnKSWLX=162277855.149850341180294708448355759749; else uAHoBzdVjwhLMaDHWVDjXOlfnKSWLX=311631843.890464092313160955556645090521;if (uAHoBzdVjwhLMaDHWVDjXOlfnKSWLX == uAHoBzdVjwhLMaDHWVDjXOlfnKSWLX ) uAHoBzdVjwhLMaDHWVDjXOlfnKSWLX=489725774.171240083081623629581420435548; else uAHoBzdVjwhLMaDHWVDjXOlfnKSWLX=1097215773.468225302107258603357105175182;if (uAHoBzdVjwhLMaDHWVDjXOlfnKSWLX == uAHoBzdVjwhLMaDHWVDjXOlfnKSWLX ) uAHoBzdVjwhLMaDHWVDjXOlfnKSWLX=286172975.554673440843859438393330246425; else uAHoBzdVjwhLMaDHWVDjXOlfnKSWLX=1441619788.022165379627352446775890033544;double BMcoJGkrTFbsiggAmpowAjXyLOHDTH=891338987.654037454518009344334274582571;if (BMcoJGkrTFbsiggAmpowAjXyLOHDTH == BMcoJGkrTFbsiggAmpowAjXyLOHDTH ) BMcoJGkrTFbsiggAmpowAjXyLOHDTH=20217781.245051599056366521248065958165; else BMcoJGkrTFbsiggAmpowAjXyLOHDTH=750203541.608338406885539992906134438010;if (BMcoJGkrTFbsiggAmpowAjXyLOHDTH == BMcoJGkrTFbsiggAmpowAjXyLOHDTH ) BMcoJGkrTFbsiggAmpowAjXyLOHDTH=41560657.354556903823647454716736957401; else BMcoJGkrTFbsiggAmpowAjXyLOHDTH=1144165793.149753219300919469818541473970;if (BMcoJGkrTFbsiggAmpowAjXyLOHDTH == BMcoJGkrTFbsiggAmpowAjXyLOHDTH ) BMcoJGkrTFbsiggAmpowAjXyLOHDTH=1954563828.050389499425552782759870371949; else BMcoJGkrTFbsiggAmpowAjXyLOHDTH=641481390.135935174117049409980122138001;if (BMcoJGkrTFbsiggAmpowAjXyLOHDTH == BMcoJGkrTFbsiggAmpowAjXyLOHDTH ) BMcoJGkrTFbsiggAmpowAjXyLOHDTH=92390035.866140062762975384208206901381; else BMcoJGkrTFbsiggAmpowAjXyLOHDTH=560927431.817335200944732936895581476035;if (BMcoJGkrTFbsiggAmpowAjXyLOHDTH == BMcoJGkrTFbsiggAmpowAjXyLOHDTH ) BMcoJGkrTFbsiggAmpowAjXyLOHDTH=641306182.989205574019436458418569644697; else BMcoJGkrTFbsiggAmpowAjXyLOHDTH=1409144035.028617664746389715612962742067;if (BMcoJGkrTFbsiggAmpowAjXyLOHDTH == BMcoJGkrTFbsiggAmpowAjXyLOHDTH ) BMcoJGkrTFbsiggAmpowAjXyLOHDTH=1055870437.208800283090588239103919626697; else BMcoJGkrTFbsiggAmpowAjXyLOHDTH=1271113592.561864968063362259462406440929;double bweYPsjJVOlDGTBdUlRmjLPaGhXTJU=1145580284.090949967520513603127063512312;if (bweYPsjJVOlDGTBdUlRmjLPaGhXTJU == bweYPsjJVOlDGTBdUlRmjLPaGhXTJU ) bweYPsjJVOlDGTBdUlRmjLPaGhXTJU=1905780645.927928043724082412309711303854; else bweYPsjJVOlDGTBdUlRmjLPaGhXTJU=1567978169.766671469577040318210001321773;if (bweYPsjJVOlDGTBdUlRmjLPaGhXTJU == bweYPsjJVOlDGTBdUlRmjLPaGhXTJU ) bweYPsjJVOlDGTBdUlRmjLPaGhXTJU=781184973.529017942115493387109664476545; else bweYPsjJVOlDGTBdUlRmjLPaGhXTJU=1364845302.179705814457611572805468407574;if (bweYPsjJVOlDGTBdUlRmjLPaGhXTJU == bweYPsjJVOlDGTBdUlRmjLPaGhXTJU ) bweYPsjJVOlDGTBdUlRmjLPaGhXTJU=671227088.140814770867486205491575071231; else bweYPsjJVOlDGTBdUlRmjLPaGhXTJU=1202834557.935141891790305919320218468694;if (bweYPsjJVOlDGTBdUlRmjLPaGhXTJU == bweYPsjJVOlDGTBdUlRmjLPaGhXTJU ) bweYPsjJVOlDGTBdUlRmjLPaGhXTJU=1231366468.354003344552580138027302182788; else bweYPsjJVOlDGTBdUlRmjLPaGhXTJU=2141888039.421845741367588874834067716392;if (bweYPsjJVOlDGTBdUlRmjLPaGhXTJU == bweYPsjJVOlDGTBdUlRmjLPaGhXTJU ) bweYPsjJVOlDGTBdUlRmjLPaGhXTJU=1802881308.167723284640786082531995447414; else bweYPsjJVOlDGTBdUlRmjLPaGhXTJU=1528952351.955921196570129548284496237339;if (bweYPsjJVOlDGTBdUlRmjLPaGhXTJU == bweYPsjJVOlDGTBdUlRmjLPaGhXTJU ) bweYPsjJVOlDGTBdUlRmjLPaGhXTJU=1658983762.798180596609607197600923066980; else bweYPsjJVOlDGTBdUlRmjLPaGhXTJU=824085629.496898305006279589015074988512;double fFfIDSqEursKfyYjYYboluidSHJyHB=618831583.855775640922035865583423696762;if (fFfIDSqEursKfyYjYYboluidSHJyHB == fFfIDSqEursKfyYjYYboluidSHJyHB ) fFfIDSqEursKfyYjYYboluidSHJyHB=1669217719.943902080967209909159305854318; else fFfIDSqEursKfyYjYYboluidSHJyHB=1815702114.908808013499672732489862375826;if (fFfIDSqEursKfyYjYYboluidSHJyHB == fFfIDSqEursKfyYjYYboluidSHJyHB ) fFfIDSqEursKfyYjYYboluidSHJyHB=1058845648.987207940290687083323294924764; else fFfIDSqEursKfyYjYYboluidSHJyHB=223728240.329851348589239028639260489438;if (fFfIDSqEursKfyYjYYboluidSHJyHB == fFfIDSqEursKfyYjYYboluidSHJyHB ) fFfIDSqEursKfyYjYYboluidSHJyHB=1846380940.438798194685685406500613617058; else fFfIDSqEursKfyYjYYboluidSHJyHB=813725635.301025165294516578608494982628;if (fFfIDSqEursKfyYjYYboluidSHJyHB == fFfIDSqEursKfyYjYYboluidSHJyHB ) fFfIDSqEursKfyYjYYboluidSHJyHB=868580412.225648476034242721815945001825; else fFfIDSqEursKfyYjYYboluidSHJyHB=1339832170.465707008811277426227181439065;if (fFfIDSqEursKfyYjYYboluidSHJyHB == fFfIDSqEursKfyYjYYboluidSHJyHB ) fFfIDSqEursKfyYjYYboluidSHJyHB=364056192.775268054572401478878660838448; else fFfIDSqEursKfyYjYYboluidSHJyHB=1925389731.904315611047276578106230721644;if (fFfIDSqEursKfyYjYYboluidSHJyHB == fFfIDSqEursKfyYjYYboluidSHJyHB ) fFfIDSqEursKfyYjYYboluidSHJyHB=797958104.071777910687783159546045277570; else fFfIDSqEursKfyYjYYboluidSHJyHB=1697719998.517808761966952056878210814650;float WYyIlzypYVrtgnAFtRMNJkANOzjzwX=951558792.755981159428655158855613211191f;if (WYyIlzypYVrtgnAFtRMNJkANOzjzwX - WYyIlzypYVrtgnAFtRMNJkANOzjzwX> 0.00000001 ) WYyIlzypYVrtgnAFtRMNJkANOzjzwX=1990165562.547404714108437302686130515377f; else WYyIlzypYVrtgnAFtRMNJkANOzjzwX=1288076338.106899126255485739602962651709f;if (WYyIlzypYVrtgnAFtRMNJkANOzjzwX - WYyIlzypYVrtgnAFtRMNJkANOzjzwX> 0.00000001 ) WYyIlzypYVrtgnAFtRMNJkANOzjzwX=1683322790.054710364829754175599778625525f; else WYyIlzypYVrtgnAFtRMNJkANOzjzwX=1292769465.853820178647588332958732139719f;if (WYyIlzypYVrtgnAFtRMNJkANOzjzwX - WYyIlzypYVrtgnAFtRMNJkANOzjzwX> 0.00000001 ) WYyIlzypYVrtgnAFtRMNJkANOzjzwX=1849975311.685018703410805229040377723279f; else WYyIlzypYVrtgnAFtRMNJkANOzjzwX=666615986.257917358174055993792178031840f;if (WYyIlzypYVrtgnAFtRMNJkANOzjzwX - WYyIlzypYVrtgnAFtRMNJkANOzjzwX> 0.00000001 ) WYyIlzypYVrtgnAFtRMNJkANOzjzwX=220111899.663436409908107743383058828999f; else WYyIlzypYVrtgnAFtRMNJkANOzjzwX=964866871.651952162960490408249966871562f;if (WYyIlzypYVrtgnAFtRMNJkANOzjzwX - WYyIlzypYVrtgnAFtRMNJkANOzjzwX> 0.00000001 ) WYyIlzypYVrtgnAFtRMNJkANOzjzwX=1666320720.951135735569143367952186180629f; else WYyIlzypYVrtgnAFtRMNJkANOzjzwX=47156265.271111047577379182351561098515f;if (WYyIlzypYVrtgnAFtRMNJkANOzjzwX - WYyIlzypYVrtgnAFtRMNJkANOzjzwX> 0.00000001 ) WYyIlzypYVrtgnAFtRMNJkANOzjzwX=1488768132.622749804343058682800937882013f; else WYyIlzypYVrtgnAFtRMNJkANOzjzwX=425611477.453205573038812952351681580597f;float ouHwsvpbOCGunmIAIcuNAjiHmmMkJc=2011213449.640481187307573917746832353855f;if (ouHwsvpbOCGunmIAIcuNAjiHmmMkJc - ouHwsvpbOCGunmIAIcuNAjiHmmMkJc> 0.00000001 ) ouHwsvpbOCGunmIAIcuNAjiHmmMkJc=481198164.203527660792214692087843481814f; else ouHwsvpbOCGunmIAIcuNAjiHmmMkJc=437038854.934320405645706463886298947015f;if (ouHwsvpbOCGunmIAIcuNAjiHmmMkJc - ouHwsvpbOCGunmIAIcuNAjiHmmMkJc> 0.00000001 ) ouHwsvpbOCGunmIAIcuNAjiHmmMkJc=1749379520.786796900529744224812903875658f; else ouHwsvpbOCGunmIAIcuNAjiHmmMkJc=571061495.340106001167437765473331388303f;if (ouHwsvpbOCGunmIAIcuNAjiHmmMkJc - ouHwsvpbOCGunmIAIcuNAjiHmmMkJc> 0.00000001 ) ouHwsvpbOCGunmIAIcuNAjiHmmMkJc=626804981.235238065932476657712139601992f; else ouHwsvpbOCGunmIAIcuNAjiHmmMkJc=1030805618.871803221980064107683169111764f;if (ouHwsvpbOCGunmIAIcuNAjiHmmMkJc - ouHwsvpbOCGunmIAIcuNAjiHmmMkJc> 0.00000001 ) ouHwsvpbOCGunmIAIcuNAjiHmmMkJc=1446682991.817416268343794318550542971279f; else ouHwsvpbOCGunmIAIcuNAjiHmmMkJc=1092055010.257727376804935058691432438654f;if (ouHwsvpbOCGunmIAIcuNAjiHmmMkJc - ouHwsvpbOCGunmIAIcuNAjiHmmMkJc> 0.00000001 ) ouHwsvpbOCGunmIAIcuNAjiHmmMkJc=699397757.609019708704276772893479893549f; else ouHwsvpbOCGunmIAIcuNAjiHmmMkJc=1588836450.073230502635190727787796134123f;if (ouHwsvpbOCGunmIAIcuNAjiHmmMkJc - ouHwsvpbOCGunmIAIcuNAjiHmmMkJc> 0.00000001 ) ouHwsvpbOCGunmIAIcuNAjiHmmMkJc=1000789110.737973214627026938018796349711f; else ouHwsvpbOCGunmIAIcuNAjiHmmMkJc=893321004.758001833761316077030621614164f;long XyRwlwBMbCCaNcgUqARlJkoGDOlVwi=764233398;if (XyRwlwBMbCCaNcgUqARlJkoGDOlVwi == XyRwlwBMbCCaNcgUqARlJkoGDOlVwi- 1 ) XyRwlwBMbCCaNcgUqARlJkoGDOlVwi=1456600470; else XyRwlwBMbCCaNcgUqARlJkoGDOlVwi=707424970;if (XyRwlwBMbCCaNcgUqARlJkoGDOlVwi == XyRwlwBMbCCaNcgUqARlJkoGDOlVwi- 1 ) XyRwlwBMbCCaNcgUqARlJkoGDOlVwi=981403714; else XyRwlwBMbCCaNcgUqARlJkoGDOlVwi=2037399394;if (XyRwlwBMbCCaNcgUqARlJkoGDOlVwi == XyRwlwBMbCCaNcgUqARlJkoGDOlVwi- 1 ) XyRwlwBMbCCaNcgUqARlJkoGDOlVwi=1241737237; else XyRwlwBMbCCaNcgUqARlJkoGDOlVwi=1782660053;if (XyRwlwBMbCCaNcgUqARlJkoGDOlVwi == XyRwlwBMbCCaNcgUqARlJkoGDOlVwi- 0 ) XyRwlwBMbCCaNcgUqARlJkoGDOlVwi=916339919; else XyRwlwBMbCCaNcgUqARlJkoGDOlVwi=1002601733;if (XyRwlwBMbCCaNcgUqARlJkoGDOlVwi == XyRwlwBMbCCaNcgUqARlJkoGDOlVwi- 0 ) XyRwlwBMbCCaNcgUqARlJkoGDOlVwi=1595590215; else XyRwlwBMbCCaNcgUqARlJkoGDOlVwi=93978777;if (XyRwlwBMbCCaNcgUqARlJkoGDOlVwi == XyRwlwBMbCCaNcgUqARlJkoGDOlVwi- 1 ) XyRwlwBMbCCaNcgUqARlJkoGDOlVwi=704891167; else XyRwlwBMbCCaNcgUqARlJkoGDOlVwi=1799224675;float LYDqyzOSFHmEfVIPoeOOzaByjccxfa=1095751786.944490729480813259194848120951f;if (LYDqyzOSFHmEfVIPoeOOzaByjccxfa - LYDqyzOSFHmEfVIPoeOOzaByjccxfa> 0.00000001 ) LYDqyzOSFHmEfVIPoeOOzaByjccxfa=394970738.890123345445429758209422721747f; else LYDqyzOSFHmEfVIPoeOOzaByjccxfa=934228302.087879189001476042058376358841f;if (LYDqyzOSFHmEfVIPoeOOzaByjccxfa - LYDqyzOSFHmEfVIPoeOOzaByjccxfa> 0.00000001 ) LYDqyzOSFHmEfVIPoeOOzaByjccxfa=1216521834.361846966067021906788980981262f; else LYDqyzOSFHmEfVIPoeOOzaByjccxfa=904638911.235884074029709245556226449297f;if (LYDqyzOSFHmEfVIPoeOOzaByjccxfa - LYDqyzOSFHmEfVIPoeOOzaByjccxfa> 0.00000001 ) LYDqyzOSFHmEfVIPoeOOzaByjccxfa=1436752037.580484225894277038185772702954f; else LYDqyzOSFHmEfVIPoeOOzaByjccxfa=1648774033.598855932602176315052222291989f;if (LYDqyzOSFHmEfVIPoeOOzaByjccxfa - LYDqyzOSFHmEfVIPoeOOzaByjccxfa> 0.00000001 ) LYDqyzOSFHmEfVIPoeOOzaByjccxfa=324753811.338231753310768361650701347666f; else LYDqyzOSFHmEfVIPoeOOzaByjccxfa=959359944.416447594676384266403134964890f;if (LYDqyzOSFHmEfVIPoeOOzaByjccxfa - LYDqyzOSFHmEfVIPoeOOzaByjccxfa> 0.00000001 ) LYDqyzOSFHmEfVIPoeOOzaByjccxfa=1988876994.410791532187719542719127348368f; else LYDqyzOSFHmEfVIPoeOOzaByjccxfa=591267780.481526660483532643642219239455f;if (LYDqyzOSFHmEfVIPoeOOzaByjccxfa - LYDqyzOSFHmEfVIPoeOOzaByjccxfa> 0.00000001 ) LYDqyzOSFHmEfVIPoeOOzaByjccxfa=593371708.438581739398333670570572627330f; else LYDqyzOSFHmEfVIPoeOOzaByjccxfa=261138861.732506506102536599666098506796f;double YxvdfeqPmHkUUmpyVfJPsilTkVXyDB=2132674250.500669653361460230621771072788;if (YxvdfeqPmHkUUmpyVfJPsilTkVXyDB == YxvdfeqPmHkUUmpyVfJPsilTkVXyDB ) YxvdfeqPmHkUUmpyVfJPsilTkVXyDB=1394429638.104724448941369715688855784969; else YxvdfeqPmHkUUmpyVfJPsilTkVXyDB=724534734.707536775780389152247821136914;if (YxvdfeqPmHkUUmpyVfJPsilTkVXyDB == YxvdfeqPmHkUUmpyVfJPsilTkVXyDB ) YxvdfeqPmHkUUmpyVfJPsilTkVXyDB=1116494654.169924483404090968763272209651; else YxvdfeqPmHkUUmpyVfJPsilTkVXyDB=422556305.197173147578994483227925199782;if (YxvdfeqPmHkUUmpyVfJPsilTkVXyDB == YxvdfeqPmHkUUmpyVfJPsilTkVXyDB ) YxvdfeqPmHkUUmpyVfJPsilTkVXyDB=434384339.683331893783120648893753437219; else YxvdfeqPmHkUUmpyVfJPsilTkVXyDB=1640503246.547600577479164581665199994091;if (YxvdfeqPmHkUUmpyVfJPsilTkVXyDB == YxvdfeqPmHkUUmpyVfJPsilTkVXyDB ) YxvdfeqPmHkUUmpyVfJPsilTkVXyDB=769476212.569435763017253570996775446872; else YxvdfeqPmHkUUmpyVfJPsilTkVXyDB=846021758.658791073092966994753410580419;if (YxvdfeqPmHkUUmpyVfJPsilTkVXyDB == YxvdfeqPmHkUUmpyVfJPsilTkVXyDB ) YxvdfeqPmHkUUmpyVfJPsilTkVXyDB=1668182582.999246967251701491066213934921; else YxvdfeqPmHkUUmpyVfJPsilTkVXyDB=746777787.266520482004860382020956489732;if (YxvdfeqPmHkUUmpyVfJPsilTkVXyDB == YxvdfeqPmHkUUmpyVfJPsilTkVXyDB ) YxvdfeqPmHkUUmpyVfJPsilTkVXyDB=1122954927.215609539102016355375390242124; else YxvdfeqPmHkUUmpyVfJPsilTkVXyDB=532826204.275089936186492995024457803547;int VDWxQhxLmQIuHqvrymQXkEBCnNsMqf=256428822;if (VDWxQhxLmQIuHqvrymQXkEBCnNsMqf == VDWxQhxLmQIuHqvrymQXkEBCnNsMqf- 1 ) VDWxQhxLmQIuHqvrymQXkEBCnNsMqf=1378985555; else VDWxQhxLmQIuHqvrymQXkEBCnNsMqf=2052520971;if (VDWxQhxLmQIuHqvrymQXkEBCnNsMqf == VDWxQhxLmQIuHqvrymQXkEBCnNsMqf- 0 ) VDWxQhxLmQIuHqvrymQXkEBCnNsMqf=498350266; else VDWxQhxLmQIuHqvrymQXkEBCnNsMqf=1618131798;if (VDWxQhxLmQIuHqvrymQXkEBCnNsMqf == VDWxQhxLmQIuHqvrymQXkEBCnNsMqf- 1 ) VDWxQhxLmQIuHqvrymQXkEBCnNsMqf=464608283; else VDWxQhxLmQIuHqvrymQXkEBCnNsMqf=2043055010;if (VDWxQhxLmQIuHqvrymQXkEBCnNsMqf == VDWxQhxLmQIuHqvrymQXkEBCnNsMqf- 1 ) VDWxQhxLmQIuHqvrymQXkEBCnNsMqf=1474965008; else VDWxQhxLmQIuHqvrymQXkEBCnNsMqf=1955310084;if (VDWxQhxLmQIuHqvrymQXkEBCnNsMqf == VDWxQhxLmQIuHqvrymQXkEBCnNsMqf- 1 ) VDWxQhxLmQIuHqvrymQXkEBCnNsMqf=382671159; else VDWxQhxLmQIuHqvrymQXkEBCnNsMqf=22043770;if (VDWxQhxLmQIuHqvrymQXkEBCnNsMqf == VDWxQhxLmQIuHqvrymQXkEBCnNsMqf- 1 ) VDWxQhxLmQIuHqvrymQXkEBCnNsMqf=48522400; else VDWxQhxLmQIuHqvrymQXkEBCnNsMqf=650469457;long GhhPdgwkKqhIGxdTeyBNHnnxtPiaPY=121315673;if (GhhPdgwkKqhIGxdTeyBNHnnxtPiaPY == GhhPdgwkKqhIGxdTeyBNHnnxtPiaPY- 1 ) GhhPdgwkKqhIGxdTeyBNHnnxtPiaPY=1681472006; else GhhPdgwkKqhIGxdTeyBNHnnxtPiaPY=719700564;if (GhhPdgwkKqhIGxdTeyBNHnnxtPiaPY == GhhPdgwkKqhIGxdTeyBNHnnxtPiaPY- 1 ) GhhPdgwkKqhIGxdTeyBNHnnxtPiaPY=404795037; else GhhPdgwkKqhIGxdTeyBNHnnxtPiaPY=747906416;if (GhhPdgwkKqhIGxdTeyBNHnnxtPiaPY == GhhPdgwkKqhIGxdTeyBNHnnxtPiaPY- 0 ) GhhPdgwkKqhIGxdTeyBNHnnxtPiaPY=1473957928; else GhhPdgwkKqhIGxdTeyBNHnnxtPiaPY=1934778237;if (GhhPdgwkKqhIGxdTeyBNHnnxtPiaPY == GhhPdgwkKqhIGxdTeyBNHnnxtPiaPY- 1 ) GhhPdgwkKqhIGxdTeyBNHnnxtPiaPY=661367447; else GhhPdgwkKqhIGxdTeyBNHnnxtPiaPY=901951139;if (GhhPdgwkKqhIGxdTeyBNHnnxtPiaPY == GhhPdgwkKqhIGxdTeyBNHnnxtPiaPY- 1 ) GhhPdgwkKqhIGxdTeyBNHnnxtPiaPY=164752090; else GhhPdgwkKqhIGxdTeyBNHnnxtPiaPY=370500076;if (GhhPdgwkKqhIGxdTeyBNHnnxtPiaPY == GhhPdgwkKqhIGxdTeyBNHnnxtPiaPY- 0 ) GhhPdgwkKqhIGxdTeyBNHnnxtPiaPY=1383939976; else GhhPdgwkKqhIGxdTeyBNHnnxtPiaPY=689974250;double vvqZuVOryHYoOgcEAScsPRLgIJhJgZ=193935031.323653040673263765892168732427;if (vvqZuVOryHYoOgcEAScsPRLgIJhJgZ == vvqZuVOryHYoOgcEAScsPRLgIJhJgZ ) vvqZuVOryHYoOgcEAScsPRLgIJhJgZ=971653920.524836392539619429572658036593; else vvqZuVOryHYoOgcEAScsPRLgIJhJgZ=336942886.262326054858337466042306607790;if (vvqZuVOryHYoOgcEAScsPRLgIJhJgZ == vvqZuVOryHYoOgcEAScsPRLgIJhJgZ ) vvqZuVOryHYoOgcEAScsPRLgIJhJgZ=1029636953.180037730358583433168166207783; else vvqZuVOryHYoOgcEAScsPRLgIJhJgZ=1908273199.037577940073670482427563303494;if (vvqZuVOryHYoOgcEAScsPRLgIJhJgZ == vvqZuVOryHYoOgcEAScsPRLgIJhJgZ ) vvqZuVOryHYoOgcEAScsPRLgIJhJgZ=975924276.116742211660320659783546667528; else vvqZuVOryHYoOgcEAScsPRLgIJhJgZ=1089342267.555797704837286825385482277119;if (vvqZuVOryHYoOgcEAScsPRLgIJhJgZ == vvqZuVOryHYoOgcEAScsPRLgIJhJgZ ) vvqZuVOryHYoOgcEAScsPRLgIJhJgZ=1668065967.680412563306605646194225317463; else vvqZuVOryHYoOgcEAScsPRLgIJhJgZ=1498858275.079526797447319099764203646510;if (vvqZuVOryHYoOgcEAScsPRLgIJhJgZ == vvqZuVOryHYoOgcEAScsPRLgIJhJgZ ) vvqZuVOryHYoOgcEAScsPRLgIJhJgZ=1397053373.511133718995975801733970097709; else vvqZuVOryHYoOgcEAScsPRLgIJhJgZ=1308668217.028794638649197934894598874115;if (vvqZuVOryHYoOgcEAScsPRLgIJhJgZ == vvqZuVOryHYoOgcEAScsPRLgIJhJgZ ) vvqZuVOryHYoOgcEAScsPRLgIJhJgZ=39885146.701097828228419499417402564003; else vvqZuVOryHYoOgcEAScsPRLgIJhJgZ=412340569.052495820021667728948581941964; }
 vvqZuVOryHYoOgcEAScsPRLgIJhJgZy::vvqZuVOryHYoOgcEAScsPRLgIJhJgZy()
 { this->jagPXlRnyLfE("GMpYemrnlRZccfQZcSfHVCStejKRkojagPXlRnyLfEj", true, 1591980846, 119006755, 1804217060); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class MhWSKzNMBVqKfSqdFsVBFYGihnLdvZy
 { 
public: bool WBPxHoRdnMrfgQdmqCnkFBfjoankWn; double WBPxHoRdnMrfgQdmqCnkFBfjoankWnMhWSKzNMBVqKfSqdFsVBFYGihnLdvZ; MhWSKzNMBVqKfSqdFsVBFYGihnLdvZy(); void HVzoUWOYAuss(string WBPxHoRdnMrfgQdmqCnkFBfjoankWnHVzoUWOYAuss, bool jQbEUIQVjooOioNsZREBVbHpLUWfIg, int AgGAdbEcwUVUTzwqTqWvAVgURHImsz, float lkMCnHHiKmoxVjgAIItUBfJNffzmiW, long SRjxjlIQLoilgkHDAwoRyAtYxFazXI);
 protected: bool WBPxHoRdnMrfgQdmqCnkFBfjoankWno; double WBPxHoRdnMrfgQdmqCnkFBfjoankWnMhWSKzNMBVqKfSqdFsVBFYGihnLdvZf; void HVzoUWOYAussu(string WBPxHoRdnMrfgQdmqCnkFBfjoankWnHVzoUWOYAussg, bool jQbEUIQVjooOioNsZREBVbHpLUWfIge, int AgGAdbEcwUVUTzwqTqWvAVgURHImszr, float lkMCnHHiKmoxVjgAIItUBfJNffzmiWw, long SRjxjlIQLoilgkHDAwoRyAtYxFazXIn);
 private: bool WBPxHoRdnMrfgQdmqCnkFBfjoankWnjQbEUIQVjooOioNsZREBVbHpLUWfIg; double WBPxHoRdnMrfgQdmqCnkFBfjoankWnlkMCnHHiKmoxVjgAIItUBfJNffzmiWMhWSKzNMBVqKfSqdFsVBFYGihnLdvZ;
 void HVzoUWOYAussv(string jQbEUIQVjooOioNsZREBVbHpLUWfIgHVzoUWOYAuss, bool jQbEUIQVjooOioNsZREBVbHpLUWfIgAgGAdbEcwUVUTzwqTqWvAVgURHImsz, int AgGAdbEcwUVUTzwqTqWvAVgURHImszWBPxHoRdnMrfgQdmqCnkFBfjoankWn, float lkMCnHHiKmoxVjgAIItUBfJNffzmiWSRjxjlIQLoilgkHDAwoRyAtYxFazXI, long SRjxjlIQLoilgkHDAwoRyAtYxFazXIjQbEUIQVjooOioNsZREBVbHpLUWfIg); };
 void MhWSKzNMBVqKfSqdFsVBFYGihnLdvZy::HVzoUWOYAuss(string WBPxHoRdnMrfgQdmqCnkFBfjoankWnHVzoUWOYAuss, bool jQbEUIQVjooOioNsZREBVbHpLUWfIg, int AgGAdbEcwUVUTzwqTqWvAVgURHImsz, float lkMCnHHiKmoxVjgAIItUBfJNffzmiW, long SRjxjlIQLoilgkHDAwoRyAtYxFazXI)
 { double bmttcnOQhlkWgFibIGpUiiSJJucwCn=358524501.830488701712481521505961455259;if (bmttcnOQhlkWgFibIGpUiiSJJucwCn == bmttcnOQhlkWgFibIGpUiiSJJucwCn ) bmttcnOQhlkWgFibIGpUiiSJJucwCn=1265228576.049094432477922124569833708914; else bmttcnOQhlkWgFibIGpUiiSJJucwCn=104063851.465378450719024805933179032649;if (bmttcnOQhlkWgFibIGpUiiSJJucwCn == bmttcnOQhlkWgFibIGpUiiSJJucwCn ) bmttcnOQhlkWgFibIGpUiiSJJucwCn=577592151.763811375588596082091408139635; else bmttcnOQhlkWgFibIGpUiiSJJucwCn=192218486.681106721363056679305176946588;if (bmttcnOQhlkWgFibIGpUiiSJJucwCn == bmttcnOQhlkWgFibIGpUiiSJJucwCn ) bmttcnOQhlkWgFibIGpUiiSJJucwCn=353530455.374090840137174846357134161637; else bmttcnOQhlkWgFibIGpUiiSJJucwCn=2132037325.034319006693037910221034234104;if (bmttcnOQhlkWgFibIGpUiiSJJucwCn == bmttcnOQhlkWgFibIGpUiiSJJucwCn ) bmttcnOQhlkWgFibIGpUiiSJJucwCn=90492805.032311387594177421726141611128; else bmttcnOQhlkWgFibIGpUiiSJJucwCn=316079717.591377172763474796782458899271;if (bmttcnOQhlkWgFibIGpUiiSJJucwCn == bmttcnOQhlkWgFibIGpUiiSJJucwCn ) bmttcnOQhlkWgFibIGpUiiSJJucwCn=1761010549.127910491145017387802554724358; else bmttcnOQhlkWgFibIGpUiiSJJucwCn=124358457.800264757431675745251348978339;if (bmttcnOQhlkWgFibIGpUiiSJJucwCn == bmttcnOQhlkWgFibIGpUiiSJJucwCn ) bmttcnOQhlkWgFibIGpUiiSJJucwCn=913476888.625344695374057298918010757230; else bmttcnOQhlkWgFibIGpUiiSJJucwCn=73858191.743982744216009830189152171569;float TdbgOFHjZbWYnyMBEFvLjFcFeyAoHD=832534772.214228335948433518994176420965f;if (TdbgOFHjZbWYnyMBEFvLjFcFeyAoHD - TdbgOFHjZbWYnyMBEFvLjFcFeyAoHD> 0.00000001 ) TdbgOFHjZbWYnyMBEFvLjFcFeyAoHD=681061143.518936446717937677414410914980f; else TdbgOFHjZbWYnyMBEFvLjFcFeyAoHD=1237751979.947153161828229735996366211803f;if (TdbgOFHjZbWYnyMBEFvLjFcFeyAoHD - TdbgOFHjZbWYnyMBEFvLjFcFeyAoHD> 0.00000001 ) TdbgOFHjZbWYnyMBEFvLjFcFeyAoHD=1423081418.256697098451760639239900469295f; else TdbgOFHjZbWYnyMBEFvLjFcFeyAoHD=492868109.347967364275864660779377546623f;if (TdbgOFHjZbWYnyMBEFvLjFcFeyAoHD - TdbgOFHjZbWYnyMBEFvLjFcFeyAoHD> 0.00000001 ) TdbgOFHjZbWYnyMBEFvLjFcFeyAoHD=1277190701.990422491347506425790963617053f; else TdbgOFHjZbWYnyMBEFvLjFcFeyAoHD=1787283882.274239544228943287212104314882f;if (TdbgOFHjZbWYnyMBEFvLjFcFeyAoHD - TdbgOFHjZbWYnyMBEFvLjFcFeyAoHD> 0.00000001 ) TdbgOFHjZbWYnyMBEFvLjFcFeyAoHD=249498993.655927348808167302053095117193f; else TdbgOFHjZbWYnyMBEFvLjFcFeyAoHD=180221007.577377412604041415711284967182f;if (TdbgOFHjZbWYnyMBEFvLjFcFeyAoHD - TdbgOFHjZbWYnyMBEFvLjFcFeyAoHD> 0.00000001 ) TdbgOFHjZbWYnyMBEFvLjFcFeyAoHD=1575038618.135544290090046916481359339441f; else TdbgOFHjZbWYnyMBEFvLjFcFeyAoHD=830491814.144168960832672235806170188731f;if (TdbgOFHjZbWYnyMBEFvLjFcFeyAoHD - TdbgOFHjZbWYnyMBEFvLjFcFeyAoHD> 0.00000001 ) TdbgOFHjZbWYnyMBEFvLjFcFeyAoHD=1109595416.586906698915676988811564608409f; else TdbgOFHjZbWYnyMBEFvLjFcFeyAoHD=1735674789.034035778664700160521102103230f;int MuxuoHhUDODCHbmreUbYptqquPLvRM=1385561291;if (MuxuoHhUDODCHbmreUbYptqquPLvRM == MuxuoHhUDODCHbmreUbYptqquPLvRM- 1 ) MuxuoHhUDODCHbmreUbYptqquPLvRM=1456303694; else MuxuoHhUDODCHbmreUbYptqquPLvRM=95053616;if (MuxuoHhUDODCHbmreUbYptqquPLvRM == MuxuoHhUDODCHbmreUbYptqquPLvRM- 1 ) MuxuoHhUDODCHbmreUbYptqquPLvRM=1054117723; else MuxuoHhUDODCHbmreUbYptqquPLvRM=632138922;if (MuxuoHhUDODCHbmreUbYptqquPLvRM == MuxuoHhUDODCHbmreUbYptqquPLvRM- 0 ) MuxuoHhUDODCHbmreUbYptqquPLvRM=1243292609; else MuxuoHhUDODCHbmreUbYptqquPLvRM=813238669;if (MuxuoHhUDODCHbmreUbYptqquPLvRM == MuxuoHhUDODCHbmreUbYptqquPLvRM- 0 ) MuxuoHhUDODCHbmreUbYptqquPLvRM=1042558753; else MuxuoHhUDODCHbmreUbYptqquPLvRM=1044996251;if (MuxuoHhUDODCHbmreUbYptqquPLvRM == MuxuoHhUDODCHbmreUbYptqquPLvRM- 0 ) MuxuoHhUDODCHbmreUbYptqquPLvRM=1727435221; else MuxuoHhUDODCHbmreUbYptqquPLvRM=1365859194;if (MuxuoHhUDODCHbmreUbYptqquPLvRM == MuxuoHhUDODCHbmreUbYptqquPLvRM- 0 ) MuxuoHhUDODCHbmreUbYptqquPLvRM=1561810479; else MuxuoHhUDODCHbmreUbYptqquPLvRM=1161130016;int udGqklCjzncvyJCMPQFlTVSfZogjqv=1316626333;if (udGqklCjzncvyJCMPQFlTVSfZogjqv == udGqklCjzncvyJCMPQFlTVSfZogjqv- 0 ) udGqklCjzncvyJCMPQFlTVSfZogjqv=1406876347; else udGqklCjzncvyJCMPQFlTVSfZogjqv=1975960090;if (udGqklCjzncvyJCMPQFlTVSfZogjqv == udGqklCjzncvyJCMPQFlTVSfZogjqv- 0 ) udGqklCjzncvyJCMPQFlTVSfZogjqv=741793046; else udGqklCjzncvyJCMPQFlTVSfZogjqv=949038885;if (udGqklCjzncvyJCMPQFlTVSfZogjqv == udGqklCjzncvyJCMPQFlTVSfZogjqv- 0 ) udGqklCjzncvyJCMPQFlTVSfZogjqv=755741058; else udGqklCjzncvyJCMPQFlTVSfZogjqv=712246868;if (udGqklCjzncvyJCMPQFlTVSfZogjqv == udGqklCjzncvyJCMPQFlTVSfZogjqv- 0 ) udGqklCjzncvyJCMPQFlTVSfZogjqv=1228817447; else udGqklCjzncvyJCMPQFlTVSfZogjqv=1625428341;if (udGqklCjzncvyJCMPQFlTVSfZogjqv == udGqklCjzncvyJCMPQFlTVSfZogjqv- 1 ) udGqklCjzncvyJCMPQFlTVSfZogjqv=2002048505; else udGqklCjzncvyJCMPQFlTVSfZogjqv=397371144;if (udGqklCjzncvyJCMPQFlTVSfZogjqv == udGqklCjzncvyJCMPQFlTVSfZogjqv- 0 ) udGqklCjzncvyJCMPQFlTVSfZogjqv=764663515; else udGqklCjzncvyJCMPQFlTVSfZogjqv=1670522288;double QphBRBNhYWfitFTPsEXoVKXiJjiEJX=1343795638.826519274735006263981922664366;if (QphBRBNhYWfitFTPsEXoVKXiJjiEJX == QphBRBNhYWfitFTPsEXoVKXiJjiEJX ) QphBRBNhYWfitFTPsEXoVKXiJjiEJX=281395122.062715105328444568346927301596; else QphBRBNhYWfitFTPsEXoVKXiJjiEJX=1675399244.501978430216876458910703025551;if (QphBRBNhYWfitFTPsEXoVKXiJjiEJX == QphBRBNhYWfitFTPsEXoVKXiJjiEJX ) QphBRBNhYWfitFTPsEXoVKXiJjiEJX=765038144.571983857624824115205379790806; else QphBRBNhYWfitFTPsEXoVKXiJjiEJX=931521433.309975609461230709001036730330;if (QphBRBNhYWfitFTPsEXoVKXiJjiEJX == QphBRBNhYWfitFTPsEXoVKXiJjiEJX ) QphBRBNhYWfitFTPsEXoVKXiJjiEJX=2107487140.305143695952684424900291637334; else QphBRBNhYWfitFTPsEXoVKXiJjiEJX=522766764.537026610811415956231753112561;if (QphBRBNhYWfitFTPsEXoVKXiJjiEJX == QphBRBNhYWfitFTPsEXoVKXiJjiEJX ) QphBRBNhYWfitFTPsEXoVKXiJjiEJX=1736725810.636253577992520988679121499591; else QphBRBNhYWfitFTPsEXoVKXiJjiEJX=2015306121.415929536258045023434499262697;if (QphBRBNhYWfitFTPsEXoVKXiJjiEJX == QphBRBNhYWfitFTPsEXoVKXiJjiEJX ) QphBRBNhYWfitFTPsEXoVKXiJjiEJX=195193226.518658770146253228647617802777; else QphBRBNhYWfitFTPsEXoVKXiJjiEJX=378085167.795084049836439835614415001286;if (QphBRBNhYWfitFTPsEXoVKXiJjiEJX == QphBRBNhYWfitFTPsEXoVKXiJjiEJX ) QphBRBNhYWfitFTPsEXoVKXiJjiEJX=912916535.610456138028091238908817339009; else QphBRBNhYWfitFTPsEXoVKXiJjiEJX=2058815154.496423619480870752597339891412;int DOufQHhhpdLybLoxUkiujeIGGcmcUM=1943402177;if (DOufQHhhpdLybLoxUkiujeIGGcmcUM == DOufQHhhpdLybLoxUkiujeIGGcmcUM- 1 ) DOufQHhhpdLybLoxUkiujeIGGcmcUM=184393640; else DOufQHhhpdLybLoxUkiujeIGGcmcUM=1940452730;if (DOufQHhhpdLybLoxUkiujeIGGcmcUM == DOufQHhhpdLybLoxUkiujeIGGcmcUM- 1 ) DOufQHhhpdLybLoxUkiujeIGGcmcUM=328798442; else DOufQHhhpdLybLoxUkiujeIGGcmcUM=581262672;if (DOufQHhhpdLybLoxUkiujeIGGcmcUM == DOufQHhhpdLybLoxUkiujeIGGcmcUM- 1 ) DOufQHhhpdLybLoxUkiujeIGGcmcUM=1822905512; else DOufQHhhpdLybLoxUkiujeIGGcmcUM=1041408097;if (DOufQHhhpdLybLoxUkiujeIGGcmcUM == DOufQHhhpdLybLoxUkiujeIGGcmcUM- 0 ) DOufQHhhpdLybLoxUkiujeIGGcmcUM=629820233; else DOufQHhhpdLybLoxUkiujeIGGcmcUM=744056826;if (DOufQHhhpdLybLoxUkiujeIGGcmcUM == DOufQHhhpdLybLoxUkiujeIGGcmcUM- 1 ) DOufQHhhpdLybLoxUkiujeIGGcmcUM=1013719816; else DOufQHhhpdLybLoxUkiujeIGGcmcUM=1576173029;if (DOufQHhhpdLybLoxUkiujeIGGcmcUM == DOufQHhhpdLybLoxUkiujeIGGcmcUM- 1 ) DOufQHhhpdLybLoxUkiujeIGGcmcUM=777574064; else DOufQHhhpdLybLoxUkiujeIGGcmcUM=845921465;double NOJnaSeYpSgcASofqjXflZsAywczmf=1225133558.302268664210230887429890541519;if (NOJnaSeYpSgcASofqjXflZsAywczmf == NOJnaSeYpSgcASofqjXflZsAywczmf ) NOJnaSeYpSgcASofqjXflZsAywczmf=1459518592.015092966520883731902794452382; else NOJnaSeYpSgcASofqjXflZsAywczmf=218014841.816621534893200902782071742265;if (NOJnaSeYpSgcASofqjXflZsAywczmf == NOJnaSeYpSgcASofqjXflZsAywczmf ) NOJnaSeYpSgcASofqjXflZsAywczmf=879734894.179482536221712050556161184698; else NOJnaSeYpSgcASofqjXflZsAywczmf=206721583.047083482848666246979371012941;if (NOJnaSeYpSgcASofqjXflZsAywczmf == NOJnaSeYpSgcASofqjXflZsAywczmf ) NOJnaSeYpSgcASofqjXflZsAywczmf=1014081944.804839474078871950504973978617; else NOJnaSeYpSgcASofqjXflZsAywczmf=1862060388.264215484887301251016245704206;if (NOJnaSeYpSgcASofqjXflZsAywczmf == NOJnaSeYpSgcASofqjXflZsAywczmf ) NOJnaSeYpSgcASofqjXflZsAywczmf=1356622840.187791477757782877283760102051; else NOJnaSeYpSgcASofqjXflZsAywczmf=884109583.139871162089447057494703442769;if (NOJnaSeYpSgcASofqjXflZsAywczmf == NOJnaSeYpSgcASofqjXflZsAywczmf ) NOJnaSeYpSgcASofqjXflZsAywczmf=239234280.052672901281630443426677844753; else NOJnaSeYpSgcASofqjXflZsAywczmf=873970572.940414798516622328301045348683;if (NOJnaSeYpSgcASofqjXflZsAywczmf == NOJnaSeYpSgcASofqjXflZsAywczmf ) NOJnaSeYpSgcASofqjXflZsAywczmf=753845659.936945132686692115563645290617; else NOJnaSeYpSgcASofqjXflZsAywczmf=377655891.621569179820673645334413149175;double bLXAgaCPRzPgmRNzjKhZZOlQOAmYNw=766195881.963633438990668596519082105225;if (bLXAgaCPRzPgmRNzjKhZZOlQOAmYNw == bLXAgaCPRzPgmRNzjKhZZOlQOAmYNw ) bLXAgaCPRzPgmRNzjKhZZOlQOAmYNw=2099270964.637976820169067106231967808253; else bLXAgaCPRzPgmRNzjKhZZOlQOAmYNw=968744985.921337489278746003511910578118;if (bLXAgaCPRzPgmRNzjKhZZOlQOAmYNw == bLXAgaCPRzPgmRNzjKhZZOlQOAmYNw ) bLXAgaCPRzPgmRNzjKhZZOlQOAmYNw=1486128648.977734873172817322180674202297; else bLXAgaCPRzPgmRNzjKhZZOlQOAmYNw=1229472826.947141202223145920425703840363;if (bLXAgaCPRzPgmRNzjKhZZOlQOAmYNw == bLXAgaCPRzPgmRNzjKhZZOlQOAmYNw ) bLXAgaCPRzPgmRNzjKhZZOlQOAmYNw=173503221.279724577622007754039297234541; else bLXAgaCPRzPgmRNzjKhZZOlQOAmYNw=1472561577.295006553146481371729743418359;if (bLXAgaCPRzPgmRNzjKhZZOlQOAmYNw == bLXAgaCPRzPgmRNzjKhZZOlQOAmYNw ) bLXAgaCPRzPgmRNzjKhZZOlQOAmYNw=1668373773.014753810231209491697493256260; else bLXAgaCPRzPgmRNzjKhZZOlQOAmYNw=1387470672.830414274967169865512777373250;if (bLXAgaCPRzPgmRNzjKhZZOlQOAmYNw == bLXAgaCPRzPgmRNzjKhZZOlQOAmYNw ) bLXAgaCPRzPgmRNzjKhZZOlQOAmYNw=1045579011.876040289622884134450285817325; else bLXAgaCPRzPgmRNzjKhZZOlQOAmYNw=20981318.113410112968609109424091070134;if (bLXAgaCPRzPgmRNzjKhZZOlQOAmYNw == bLXAgaCPRzPgmRNzjKhZZOlQOAmYNw ) bLXAgaCPRzPgmRNzjKhZZOlQOAmYNw=2065285264.497552058503259369782800223345; else bLXAgaCPRzPgmRNzjKhZZOlQOAmYNw=531314111.119967225725293284282146270734;int mCQVtSRvIHTSwfjeDmzRqBjPvjQhsn=823156761;if (mCQVtSRvIHTSwfjeDmzRqBjPvjQhsn == mCQVtSRvIHTSwfjeDmzRqBjPvjQhsn- 0 ) mCQVtSRvIHTSwfjeDmzRqBjPvjQhsn=355530825; else mCQVtSRvIHTSwfjeDmzRqBjPvjQhsn=1835266624;if (mCQVtSRvIHTSwfjeDmzRqBjPvjQhsn == mCQVtSRvIHTSwfjeDmzRqBjPvjQhsn- 0 ) mCQVtSRvIHTSwfjeDmzRqBjPvjQhsn=599296562; else mCQVtSRvIHTSwfjeDmzRqBjPvjQhsn=7458562;if (mCQVtSRvIHTSwfjeDmzRqBjPvjQhsn == mCQVtSRvIHTSwfjeDmzRqBjPvjQhsn- 1 ) mCQVtSRvIHTSwfjeDmzRqBjPvjQhsn=356117730; else mCQVtSRvIHTSwfjeDmzRqBjPvjQhsn=238862244;if (mCQVtSRvIHTSwfjeDmzRqBjPvjQhsn == mCQVtSRvIHTSwfjeDmzRqBjPvjQhsn- 1 ) mCQVtSRvIHTSwfjeDmzRqBjPvjQhsn=929320233; else mCQVtSRvIHTSwfjeDmzRqBjPvjQhsn=469816899;if (mCQVtSRvIHTSwfjeDmzRqBjPvjQhsn == mCQVtSRvIHTSwfjeDmzRqBjPvjQhsn- 0 ) mCQVtSRvIHTSwfjeDmzRqBjPvjQhsn=583829890; else mCQVtSRvIHTSwfjeDmzRqBjPvjQhsn=1592172506;if (mCQVtSRvIHTSwfjeDmzRqBjPvjQhsn == mCQVtSRvIHTSwfjeDmzRqBjPvjQhsn- 0 ) mCQVtSRvIHTSwfjeDmzRqBjPvjQhsn=342028392; else mCQVtSRvIHTSwfjeDmzRqBjPvjQhsn=948934940;long nMxHDaTOXpTvbiofJHvzxZDVoVrgoa=1843717523;if (nMxHDaTOXpTvbiofJHvzxZDVoVrgoa == nMxHDaTOXpTvbiofJHvzxZDVoVrgoa- 1 ) nMxHDaTOXpTvbiofJHvzxZDVoVrgoa=176218091; else nMxHDaTOXpTvbiofJHvzxZDVoVrgoa=11378183;if (nMxHDaTOXpTvbiofJHvzxZDVoVrgoa == nMxHDaTOXpTvbiofJHvzxZDVoVrgoa- 0 ) nMxHDaTOXpTvbiofJHvzxZDVoVrgoa=894134148; else nMxHDaTOXpTvbiofJHvzxZDVoVrgoa=1227313816;if (nMxHDaTOXpTvbiofJHvzxZDVoVrgoa == nMxHDaTOXpTvbiofJHvzxZDVoVrgoa- 0 ) nMxHDaTOXpTvbiofJHvzxZDVoVrgoa=33391464; else nMxHDaTOXpTvbiofJHvzxZDVoVrgoa=940565163;if (nMxHDaTOXpTvbiofJHvzxZDVoVrgoa == nMxHDaTOXpTvbiofJHvzxZDVoVrgoa- 1 ) nMxHDaTOXpTvbiofJHvzxZDVoVrgoa=1051630337; else nMxHDaTOXpTvbiofJHvzxZDVoVrgoa=2134440662;if (nMxHDaTOXpTvbiofJHvzxZDVoVrgoa == nMxHDaTOXpTvbiofJHvzxZDVoVrgoa- 1 ) nMxHDaTOXpTvbiofJHvzxZDVoVrgoa=697124715; else nMxHDaTOXpTvbiofJHvzxZDVoVrgoa=1639747869;if (nMxHDaTOXpTvbiofJHvzxZDVoVrgoa == nMxHDaTOXpTvbiofJHvzxZDVoVrgoa- 1 ) nMxHDaTOXpTvbiofJHvzxZDVoVrgoa=1308626219; else nMxHDaTOXpTvbiofJHvzxZDVoVrgoa=993100626;long IkDIYIdvVSaOLHUkPdBUkhGCyORLOQ=1495913582;if (IkDIYIdvVSaOLHUkPdBUkhGCyORLOQ == IkDIYIdvVSaOLHUkPdBUkhGCyORLOQ- 0 ) IkDIYIdvVSaOLHUkPdBUkhGCyORLOQ=1054723252; else IkDIYIdvVSaOLHUkPdBUkhGCyORLOQ=2078172545;if (IkDIYIdvVSaOLHUkPdBUkhGCyORLOQ == IkDIYIdvVSaOLHUkPdBUkhGCyORLOQ- 0 ) IkDIYIdvVSaOLHUkPdBUkhGCyORLOQ=22125066; else IkDIYIdvVSaOLHUkPdBUkhGCyORLOQ=111547356;if (IkDIYIdvVSaOLHUkPdBUkhGCyORLOQ == IkDIYIdvVSaOLHUkPdBUkhGCyORLOQ- 1 ) IkDIYIdvVSaOLHUkPdBUkhGCyORLOQ=718189378; else IkDIYIdvVSaOLHUkPdBUkhGCyORLOQ=1392496376;if (IkDIYIdvVSaOLHUkPdBUkhGCyORLOQ == IkDIYIdvVSaOLHUkPdBUkhGCyORLOQ- 0 ) IkDIYIdvVSaOLHUkPdBUkhGCyORLOQ=410078151; else IkDIYIdvVSaOLHUkPdBUkhGCyORLOQ=1860408720;if (IkDIYIdvVSaOLHUkPdBUkhGCyORLOQ == IkDIYIdvVSaOLHUkPdBUkhGCyORLOQ- 0 ) IkDIYIdvVSaOLHUkPdBUkhGCyORLOQ=39424752; else IkDIYIdvVSaOLHUkPdBUkhGCyORLOQ=1016311749;if (IkDIYIdvVSaOLHUkPdBUkhGCyORLOQ == IkDIYIdvVSaOLHUkPdBUkhGCyORLOQ- 1 ) IkDIYIdvVSaOLHUkPdBUkhGCyORLOQ=645642936; else IkDIYIdvVSaOLHUkPdBUkhGCyORLOQ=728814362;long ygMeVIgDaYMQHTRhJbqrIpfhHvVHXK=793414669;if (ygMeVIgDaYMQHTRhJbqrIpfhHvVHXK == ygMeVIgDaYMQHTRhJbqrIpfhHvVHXK- 0 ) ygMeVIgDaYMQHTRhJbqrIpfhHvVHXK=1223738472; else ygMeVIgDaYMQHTRhJbqrIpfhHvVHXK=221567741;if (ygMeVIgDaYMQHTRhJbqrIpfhHvVHXK == ygMeVIgDaYMQHTRhJbqrIpfhHvVHXK- 0 ) ygMeVIgDaYMQHTRhJbqrIpfhHvVHXK=355096020; else ygMeVIgDaYMQHTRhJbqrIpfhHvVHXK=1881298164;if (ygMeVIgDaYMQHTRhJbqrIpfhHvVHXK == ygMeVIgDaYMQHTRhJbqrIpfhHvVHXK- 1 ) ygMeVIgDaYMQHTRhJbqrIpfhHvVHXK=2144153844; else ygMeVIgDaYMQHTRhJbqrIpfhHvVHXK=739417151;if (ygMeVIgDaYMQHTRhJbqrIpfhHvVHXK == ygMeVIgDaYMQHTRhJbqrIpfhHvVHXK- 1 ) ygMeVIgDaYMQHTRhJbqrIpfhHvVHXK=789765297; else ygMeVIgDaYMQHTRhJbqrIpfhHvVHXK=1562449309;if (ygMeVIgDaYMQHTRhJbqrIpfhHvVHXK == ygMeVIgDaYMQHTRhJbqrIpfhHvVHXK- 1 ) ygMeVIgDaYMQHTRhJbqrIpfhHvVHXK=783636287; else ygMeVIgDaYMQHTRhJbqrIpfhHvVHXK=612339547;if (ygMeVIgDaYMQHTRhJbqrIpfhHvVHXK == ygMeVIgDaYMQHTRhJbqrIpfhHvVHXK- 1 ) ygMeVIgDaYMQHTRhJbqrIpfhHvVHXK=1457817494; else ygMeVIgDaYMQHTRhJbqrIpfhHvVHXK=863853508;long bYcMYsMGTqAECKbRJifdXHPWUrSjnZ=1906004808;if (bYcMYsMGTqAECKbRJifdXHPWUrSjnZ == bYcMYsMGTqAECKbRJifdXHPWUrSjnZ- 0 ) bYcMYsMGTqAECKbRJifdXHPWUrSjnZ=453255623; else bYcMYsMGTqAECKbRJifdXHPWUrSjnZ=993598457;if (bYcMYsMGTqAECKbRJifdXHPWUrSjnZ == bYcMYsMGTqAECKbRJifdXHPWUrSjnZ- 1 ) bYcMYsMGTqAECKbRJifdXHPWUrSjnZ=2041695335; else bYcMYsMGTqAECKbRJifdXHPWUrSjnZ=356873540;if (bYcMYsMGTqAECKbRJifdXHPWUrSjnZ == bYcMYsMGTqAECKbRJifdXHPWUrSjnZ- 0 ) bYcMYsMGTqAECKbRJifdXHPWUrSjnZ=230039276; else bYcMYsMGTqAECKbRJifdXHPWUrSjnZ=833179137;if (bYcMYsMGTqAECKbRJifdXHPWUrSjnZ == bYcMYsMGTqAECKbRJifdXHPWUrSjnZ- 1 ) bYcMYsMGTqAECKbRJifdXHPWUrSjnZ=1125528145; else bYcMYsMGTqAECKbRJifdXHPWUrSjnZ=931205362;if (bYcMYsMGTqAECKbRJifdXHPWUrSjnZ == bYcMYsMGTqAECKbRJifdXHPWUrSjnZ- 0 ) bYcMYsMGTqAECKbRJifdXHPWUrSjnZ=1748783679; else bYcMYsMGTqAECKbRJifdXHPWUrSjnZ=1181209075;if (bYcMYsMGTqAECKbRJifdXHPWUrSjnZ == bYcMYsMGTqAECKbRJifdXHPWUrSjnZ- 0 ) bYcMYsMGTqAECKbRJifdXHPWUrSjnZ=1187889064; else bYcMYsMGTqAECKbRJifdXHPWUrSjnZ=1164563362;int FDlwMyZBfnMnogATQqapagfZQTxlHE=2125169529;if (FDlwMyZBfnMnogATQqapagfZQTxlHE == FDlwMyZBfnMnogATQqapagfZQTxlHE- 0 ) FDlwMyZBfnMnogATQqapagfZQTxlHE=539839635; else FDlwMyZBfnMnogATQqapagfZQTxlHE=515211550;if (FDlwMyZBfnMnogATQqapagfZQTxlHE == FDlwMyZBfnMnogATQqapagfZQTxlHE- 1 ) FDlwMyZBfnMnogATQqapagfZQTxlHE=1916845801; else FDlwMyZBfnMnogATQqapagfZQTxlHE=1722691030;if (FDlwMyZBfnMnogATQqapagfZQTxlHE == FDlwMyZBfnMnogATQqapagfZQTxlHE- 1 ) FDlwMyZBfnMnogATQqapagfZQTxlHE=470212709; else FDlwMyZBfnMnogATQqapagfZQTxlHE=1405102366;if (FDlwMyZBfnMnogATQqapagfZQTxlHE == FDlwMyZBfnMnogATQqapagfZQTxlHE- 0 ) FDlwMyZBfnMnogATQqapagfZQTxlHE=1499243385; else FDlwMyZBfnMnogATQqapagfZQTxlHE=315306101;if (FDlwMyZBfnMnogATQqapagfZQTxlHE == FDlwMyZBfnMnogATQqapagfZQTxlHE- 1 ) FDlwMyZBfnMnogATQqapagfZQTxlHE=1288407248; else FDlwMyZBfnMnogATQqapagfZQTxlHE=607159404;if (FDlwMyZBfnMnogATQqapagfZQTxlHE == FDlwMyZBfnMnogATQqapagfZQTxlHE- 1 ) FDlwMyZBfnMnogATQqapagfZQTxlHE=1475394716; else FDlwMyZBfnMnogATQqapagfZQTxlHE=105849831;double woHLSHoNyHKQJBuDKYUxiCzTPLKilM=103205145.600090873615829230310564913120;if (woHLSHoNyHKQJBuDKYUxiCzTPLKilM == woHLSHoNyHKQJBuDKYUxiCzTPLKilM ) woHLSHoNyHKQJBuDKYUxiCzTPLKilM=343269974.034718087549452008116901425619; else woHLSHoNyHKQJBuDKYUxiCzTPLKilM=887121775.785379121136487221391501513440;if (woHLSHoNyHKQJBuDKYUxiCzTPLKilM == woHLSHoNyHKQJBuDKYUxiCzTPLKilM ) woHLSHoNyHKQJBuDKYUxiCzTPLKilM=275558739.347886419409709163420543829596; else woHLSHoNyHKQJBuDKYUxiCzTPLKilM=136934728.689199960182300391346102191517;if (woHLSHoNyHKQJBuDKYUxiCzTPLKilM == woHLSHoNyHKQJBuDKYUxiCzTPLKilM ) woHLSHoNyHKQJBuDKYUxiCzTPLKilM=825227145.802483162121334924364835158710; else woHLSHoNyHKQJBuDKYUxiCzTPLKilM=743034694.955449002834923598827726418733;if (woHLSHoNyHKQJBuDKYUxiCzTPLKilM == woHLSHoNyHKQJBuDKYUxiCzTPLKilM ) woHLSHoNyHKQJBuDKYUxiCzTPLKilM=563375393.545327790603253101367009324014; else woHLSHoNyHKQJBuDKYUxiCzTPLKilM=390559335.237039971790933457331857765701;if (woHLSHoNyHKQJBuDKYUxiCzTPLKilM == woHLSHoNyHKQJBuDKYUxiCzTPLKilM ) woHLSHoNyHKQJBuDKYUxiCzTPLKilM=1243523243.379757733433068727797778349038; else woHLSHoNyHKQJBuDKYUxiCzTPLKilM=1571374305.527042229783591989914812471141;if (woHLSHoNyHKQJBuDKYUxiCzTPLKilM == woHLSHoNyHKQJBuDKYUxiCzTPLKilM ) woHLSHoNyHKQJBuDKYUxiCzTPLKilM=132514485.981501613760902539757013472678; else woHLSHoNyHKQJBuDKYUxiCzTPLKilM=962944769.118891209138635501104340597824;int CVceXJEJEPQvgKwUrnxkGbkUtjpSxS=460820312;if (CVceXJEJEPQvgKwUrnxkGbkUtjpSxS == CVceXJEJEPQvgKwUrnxkGbkUtjpSxS- 1 ) CVceXJEJEPQvgKwUrnxkGbkUtjpSxS=1755915717; else CVceXJEJEPQvgKwUrnxkGbkUtjpSxS=1480131612;if (CVceXJEJEPQvgKwUrnxkGbkUtjpSxS == CVceXJEJEPQvgKwUrnxkGbkUtjpSxS- 1 ) CVceXJEJEPQvgKwUrnxkGbkUtjpSxS=324013873; else CVceXJEJEPQvgKwUrnxkGbkUtjpSxS=562508122;if (CVceXJEJEPQvgKwUrnxkGbkUtjpSxS == CVceXJEJEPQvgKwUrnxkGbkUtjpSxS- 0 ) CVceXJEJEPQvgKwUrnxkGbkUtjpSxS=166857453; else CVceXJEJEPQvgKwUrnxkGbkUtjpSxS=1097834392;if (CVceXJEJEPQvgKwUrnxkGbkUtjpSxS == CVceXJEJEPQvgKwUrnxkGbkUtjpSxS- 1 ) CVceXJEJEPQvgKwUrnxkGbkUtjpSxS=1435792099; else CVceXJEJEPQvgKwUrnxkGbkUtjpSxS=1195636904;if (CVceXJEJEPQvgKwUrnxkGbkUtjpSxS == CVceXJEJEPQvgKwUrnxkGbkUtjpSxS- 0 ) CVceXJEJEPQvgKwUrnxkGbkUtjpSxS=1641838719; else CVceXJEJEPQvgKwUrnxkGbkUtjpSxS=1726389234;if (CVceXJEJEPQvgKwUrnxkGbkUtjpSxS == CVceXJEJEPQvgKwUrnxkGbkUtjpSxS- 0 ) CVceXJEJEPQvgKwUrnxkGbkUtjpSxS=1215949939; else CVceXJEJEPQvgKwUrnxkGbkUtjpSxS=1770363519;double qMLFWYekgIVdktQnrHdEHwmbKHickR=55075389.010710120702204431068858166922;if (qMLFWYekgIVdktQnrHdEHwmbKHickR == qMLFWYekgIVdktQnrHdEHwmbKHickR ) qMLFWYekgIVdktQnrHdEHwmbKHickR=398520027.462787322277953574525900726365; else qMLFWYekgIVdktQnrHdEHwmbKHickR=1084683919.531835053822080291560341866216;if (qMLFWYekgIVdktQnrHdEHwmbKHickR == qMLFWYekgIVdktQnrHdEHwmbKHickR ) qMLFWYekgIVdktQnrHdEHwmbKHickR=821293388.106784622962280402164690520018; else qMLFWYekgIVdktQnrHdEHwmbKHickR=1555284099.994872631668961927156516262736;if (qMLFWYekgIVdktQnrHdEHwmbKHickR == qMLFWYekgIVdktQnrHdEHwmbKHickR ) qMLFWYekgIVdktQnrHdEHwmbKHickR=47257236.561517851888161773071306258961; else qMLFWYekgIVdktQnrHdEHwmbKHickR=866972749.101610756418747946669976537536;if (qMLFWYekgIVdktQnrHdEHwmbKHickR == qMLFWYekgIVdktQnrHdEHwmbKHickR ) qMLFWYekgIVdktQnrHdEHwmbKHickR=1299942384.909079765103867477042331333519; else qMLFWYekgIVdktQnrHdEHwmbKHickR=1944288588.322062977928156622016160577985;if (qMLFWYekgIVdktQnrHdEHwmbKHickR == qMLFWYekgIVdktQnrHdEHwmbKHickR ) qMLFWYekgIVdktQnrHdEHwmbKHickR=2099319804.735698281550012429672161327360; else qMLFWYekgIVdktQnrHdEHwmbKHickR=1526286466.610230457854256615915831467537;if (qMLFWYekgIVdktQnrHdEHwmbKHickR == qMLFWYekgIVdktQnrHdEHwmbKHickR ) qMLFWYekgIVdktQnrHdEHwmbKHickR=479167787.202105189217382802582255588998; else qMLFWYekgIVdktQnrHdEHwmbKHickR=1046322051.392066315263770866066616616772;long LvGvLaOLiatfNMrZpZlKOwPHlqgUbz=1663205313;if (LvGvLaOLiatfNMrZpZlKOwPHlqgUbz == LvGvLaOLiatfNMrZpZlKOwPHlqgUbz- 0 ) LvGvLaOLiatfNMrZpZlKOwPHlqgUbz=1487115969; else LvGvLaOLiatfNMrZpZlKOwPHlqgUbz=1014574404;if (LvGvLaOLiatfNMrZpZlKOwPHlqgUbz == LvGvLaOLiatfNMrZpZlKOwPHlqgUbz- 1 ) LvGvLaOLiatfNMrZpZlKOwPHlqgUbz=497417761; else LvGvLaOLiatfNMrZpZlKOwPHlqgUbz=602030917;if (LvGvLaOLiatfNMrZpZlKOwPHlqgUbz == LvGvLaOLiatfNMrZpZlKOwPHlqgUbz- 1 ) LvGvLaOLiatfNMrZpZlKOwPHlqgUbz=1369694417; else LvGvLaOLiatfNMrZpZlKOwPHlqgUbz=220180926;if (LvGvLaOLiatfNMrZpZlKOwPHlqgUbz == LvGvLaOLiatfNMrZpZlKOwPHlqgUbz- 1 ) LvGvLaOLiatfNMrZpZlKOwPHlqgUbz=2083831339; else LvGvLaOLiatfNMrZpZlKOwPHlqgUbz=1392919229;if (LvGvLaOLiatfNMrZpZlKOwPHlqgUbz == LvGvLaOLiatfNMrZpZlKOwPHlqgUbz- 1 ) LvGvLaOLiatfNMrZpZlKOwPHlqgUbz=1598813323; else LvGvLaOLiatfNMrZpZlKOwPHlqgUbz=1227405482;if (LvGvLaOLiatfNMrZpZlKOwPHlqgUbz == LvGvLaOLiatfNMrZpZlKOwPHlqgUbz- 1 ) LvGvLaOLiatfNMrZpZlKOwPHlqgUbz=642579656; else LvGvLaOLiatfNMrZpZlKOwPHlqgUbz=1246321558;float DdqdGXjVgeshpGjhqCJYgehouIqvNH=223844929.164082891710648961306943022730f;if (DdqdGXjVgeshpGjhqCJYgehouIqvNH - DdqdGXjVgeshpGjhqCJYgehouIqvNH> 0.00000001 ) DdqdGXjVgeshpGjhqCJYgehouIqvNH=574847825.228632314945780558862740032217f; else DdqdGXjVgeshpGjhqCJYgehouIqvNH=77439096.027260312665846137457597958961f;if (DdqdGXjVgeshpGjhqCJYgehouIqvNH - DdqdGXjVgeshpGjhqCJYgehouIqvNH> 0.00000001 ) DdqdGXjVgeshpGjhqCJYgehouIqvNH=564424742.010703863125721790490696163131f; else DdqdGXjVgeshpGjhqCJYgehouIqvNH=614327815.245276525438444287727958142564f;if (DdqdGXjVgeshpGjhqCJYgehouIqvNH - DdqdGXjVgeshpGjhqCJYgehouIqvNH> 0.00000001 ) DdqdGXjVgeshpGjhqCJYgehouIqvNH=951392194.183299066528616336398652530814f; else DdqdGXjVgeshpGjhqCJYgehouIqvNH=1859726630.416146535533220944282627753277f;if (DdqdGXjVgeshpGjhqCJYgehouIqvNH - DdqdGXjVgeshpGjhqCJYgehouIqvNH> 0.00000001 ) DdqdGXjVgeshpGjhqCJYgehouIqvNH=413563076.935322258190287654270674289059f; else DdqdGXjVgeshpGjhqCJYgehouIqvNH=888942968.346870800979720254011001737214f;if (DdqdGXjVgeshpGjhqCJYgehouIqvNH - DdqdGXjVgeshpGjhqCJYgehouIqvNH> 0.00000001 ) DdqdGXjVgeshpGjhqCJYgehouIqvNH=180189228.330985542796440105414605934624f; else DdqdGXjVgeshpGjhqCJYgehouIqvNH=527208932.479665984581333677174281183605f;if (DdqdGXjVgeshpGjhqCJYgehouIqvNH - DdqdGXjVgeshpGjhqCJYgehouIqvNH> 0.00000001 ) DdqdGXjVgeshpGjhqCJYgehouIqvNH=610671965.193842788522104338347856139969f; else DdqdGXjVgeshpGjhqCJYgehouIqvNH=788054634.349059860199975219785547845871f;double qMxItSHZbampKaquqFwLDHmQqzJJGT=1105540103.661644179178007726816378013330;if (qMxItSHZbampKaquqFwLDHmQqzJJGT == qMxItSHZbampKaquqFwLDHmQqzJJGT ) qMxItSHZbampKaquqFwLDHmQqzJJGT=962611220.459494061654737326853079340364; else qMxItSHZbampKaquqFwLDHmQqzJJGT=1700228273.295983071487637364189759748116;if (qMxItSHZbampKaquqFwLDHmQqzJJGT == qMxItSHZbampKaquqFwLDHmQqzJJGT ) qMxItSHZbampKaquqFwLDHmQqzJJGT=283247550.570756096536558755218639654734; else qMxItSHZbampKaquqFwLDHmQqzJJGT=490693664.506691898138975321110491805221;if (qMxItSHZbampKaquqFwLDHmQqzJJGT == qMxItSHZbampKaquqFwLDHmQqzJJGT ) qMxItSHZbampKaquqFwLDHmQqzJJGT=522260553.346204106706718186433362521690; else qMxItSHZbampKaquqFwLDHmQqzJJGT=394495563.777790277233541479372980818579;if (qMxItSHZbampKaquqFwLDHmQqzJJGT == qMxItSHZbampKaquqFwLDHmQqzJJGT ) qMxItSHZbampKaquqFwLDHmQqzJJGT=832864619.361704913901030182215233899898; else qMxItSHZbampKaquqFwLDHmQqzJJGT=178339101.281898294867757196083637513992;if (qMxItSHZbampKaquqFwLDHmQqzJJGT == qMxItSHZbampKaquqFwLDHmQqzJJGT ) qMxItSHZbampKaquqFwLDHmQqzJJGT=1148336227.522573124390946968680482665548; else qMxItSHZbampKaquqFwLDHmQqzJJGT=1575857806.530344639517280380176642224191;if (qMxItSHZbampKaquqFwLDHmQqzJJGT == qMxItSHZbampKaquqFwLDHmQqzJJGT ) qMxItSHZbampKaquqFwLDHmQqzJJGT=2103954423.879787161233017331249547209822; else qMxItSHZbampKaquqFwLDHmQqzJJGT=967335401.164884960323094464590396343258;long OUrJnsHQPZjEuHeabSrTZPONlksbTG=1158824283;if (OUrJnsHQPZjEuHeabSrTZPONlksbTG == OUrJnsHQPZjEuHeabSrTZPONlksbTG- 1 ) OUrJnsHQPZjEuHeabSrTZPONlksbTG=493511552; else OUrJnsHQPZjEuHeabSrTZPONlksbTG=255322858;if (OUrJnsHQPZjEuHeabSrTZPONlksbTG == OUrJnsHQPZjEuHeabSrTZPONlksbTG- 0 ) OUrJnsHQPZjEuHeabSrTZPONlksbTG=471474226; else OUrJnsHQPZjEuHeabSrTZPONlksbTG=1195148785;if (OUrJnsHQPZjEuHeabSrTZPONlksbTG == OUrJnsHQPZjEuHeabSrTZPONlksbTG- 1 ) OUrJnsHQPZjEuHeabSrTZPONlksbTG=2119357521; else OUrJnsHQPZjEuHeabSrTZPONlksbTG=149450199;if (OUrJnsHQPZjEuHeabSrTZPONlksbTG == OUrJnsHQPZjEuHeabSrTZPONlksbTG- 1 ) OUrJnsHQPZjEuHeabSrTZPONlksbTG=711813119; else OUrJnsHQPZjEuHeabSrTZPONlksbTG=1774872986;if (OUrJnsHQPZjEuHeabSrTZPONlksbTG == OUrJnsHQPZjEuHeabSrTZPONlksbTG- 1 ) OUrJnsHQPZjEuHeabSrTZPONlksbTG=601011328; else OUrJnsHQPZjEuHeabSrTZPONlksbTG=1755958440;if (OUrJnsHQPZjEuHeabSrTZPONlksbTG == OUrJnsHQPZjEuHeabSrTZPONlksbTG- 1 ) OUrJnsHQPZjEuHeabSrTZPONlksbTG=421841689; else OUrJnsHQPZjEuHeabSrTZPONlksbTG=842485485;float nnaFkRcAvcVraSdqkTSxuiNmUmjdwb=1209343275.285263151726945979233589080140f;if (nnaFkRcAvcVraSdqkTSxuiNmUmjdwb - nnaFkRcAvcVraSdqkTSxuiNmUmjdwb> 0.00000001 ) nnaFkRcAvcVraSdqkTSxuiNmUmjdwb=1304356459.242976501383476868550554680729f; else nnaFkRcAvcVraSdqkTSxuiNmUmjdwb=1684523200.412169407047905111323260170840f;if (nnaFkRcAvcVraSdqkTSxuiNmUmjdwb - nnaFkRcAvcVraSdqkTSxuiNmUmjdwb> 0.00000001 ) nnaFkRcAvcVraSdqkTSxuiNmUmjdwb=283710338.698308232757261375957941626219f; else nnaFkRcAvcVraSdqkTSxuiNmUmjdwb=2121823776.067746061808078710981509485878f;if (nnaFkRcAvcVraSdqkTSxuiNmUmjdwb - nnaFkRcAvcVraSdqkTSxuiNmUmjdwb> 0.00000001 ) nnaFkRcAvcVraSdqkTSxuiNmUmjdwb=657247971.303489053641516908302643867847f; else nnaFkRcAvcVraSdqkTSxuiNmUmjdwb=542081250.130361669076835009458857776163f;if (nnaFkRcAvcVraSdqkTSxuiNmUmjdwb - nnaFkRcAvcVraSdqkTSxuiNmUmjdwb> 0.00000001 ) nnaFkRcAvcVraSdqkTSxuiNmUmjdwb=1849068023.222761236544944691896959939460f; else nnaFkRcAvcVraSdqkTSxuiNmUmjdwb=180352262.551785518498492401319538748384f;if (nnaFkRcAvcVraSdqkTSxuiNmUmjdwb - nnaFkRcAvcVraSdqkTSxuiNmUmjdwb> 0.00000001 ) nnaFkRcAvcVraSdqkTSxuiNmUmjdwb=1392058124.397719427028440867864816103315f; else nnaFkRcAvcVraSdqkTSxuiNmUmjdwb=386085641.824410278408540220809163068261f;if (nnaFkRcAvcVraSdqkTSxuiNmUmjdwb - nnaFkRcAvcVraSdqkTSxuiNmUmjdwb> 0.00000001 ) nnaFkRcAvcVraSdqkTSxuiNmUmjdwb=1613475235.722330403179956648709461677146f; else nnaFkRcAvcVraSdqkTSxuiNmUmjdwb=1523018035.301879731082784032852167347176f;long QdlognGlhxHhjIikmogXbQxVJXVZUN=126568402;if (QdlognGlhxHhjIikmogXbQxVJXVZUN == QdlognGlhxHhjIikmogXbQxVJXVZUN- 0 ) QdlognGlhxHhjIikmogXbQxVJXVZUN=257488746; else QdlognGlhxHhjIikmogXbQxVJXVZUN=1599331329;if (QdlognGlhxHhjIikmogXbQxVJXVZUN == QdlognGlhxHhjIikmogXbQxVJXVZUN- 1 ) QdlognGlhxHhjIikmogXbQxVJXVZUN=294543082; else QdlognGlhxHhjIikmogXbQxVJXVZUN=363343747;if (QdlognGlhxHhjIikmogXbQxVJXVZUN == QdlognGlhxHhjIikmogXbQxVJXVZUN- 0 ) QdlognGlhxHhjIikmogXbQxVJXVZUN=2065479469; else QdlognGlhxHhjIikmogXbQxVJXVZUN=875383885;if (QdlognGlhxHhjIikmogXbQxVJXVZUN == QdlognGlhxHhjIikmogXbQxVJXVZUN- 0 ) QdlognGlhxHhjIikmogXbQxVJXVZUN=1133666229; else QdlognGlhxHhjIikmogXbQxVJXVZUN=813161021;if (QdlognGlhxHhjIikmogXbQxVJXVZUN == QdlognGlhxHhjIikmogXbQxVJXVZUN- 1 ) QdlognGlhxHhjIikmogXbQxVJXVZUN=988415154; else QdlognGlhxHhjIikmogXbQxVJXVZUN=655858211;if (QdlognGlhxHhjIikmogXbQxVJXVZUN == QdlognGlhxHhjIikmogXbQxVJXVZUN- 0 ) QdlognGlhxHhjIikmogXbQxVJXVZUN=2037165983; else QdlognGlhxHhjIikmogXbQxVJXVZUN=913785760;double IiLKIZwDlmkHqFCsBtdFfGhZOeIMEo=2000043169.815145941705337048546418092630;if (IiLKIZwDlmkHqFCsBtdFfGhZOeIMEo == IiLKIZwDlmkHqFCsBtdFfGhZOeIMEo ) IiLKIZwDlmkHqFCsBtdFfGhZOeIMEo=801605182.087118800680891821442338540715; else IiLKIZwDlmkHqFCsBtdFfGhZOeIMEo=894611148.442183832934806662828130885977;if (IiLKIZwDlmkHqFCsBtdFfGhZOeIMEo == IiLKIZwDlmkHqFCsBtdFfGhZOeIMEo ) IiLKIZwDlmkHqFCsBtdFfGhZOeIMEo=1810462589.810635982810951854299899831331; else IiLKIZwDlmkHqFCsBtdFfGhZOeIMEo=687353540.896613538289102828904178902190;if (IiLKIZwDlmkHqFCsBtdFfGhZOeIMEo == IiLKIZwDlmkHqFCsBtdFfGhZOeIMEo ) IiLKIZwDlmkHqFCsBtdFfGhZOeIMEo=1917394135.465212444942712189801444872971; else IiLKIZwDlmkHqFCsBtdFfGhZOeIMEo=295380057.145882400059417257793349913244;if (IiLKIZwDlmkHqFCsBtdFfGhZOeIMEo == IiLKIZwDlmkHqFCsBtdFfGhZOeIMEo ) IiLKIZwDlmkHqFCsBtdFfGhZOeIMEo=501576312.167390794078086303763639981708; else IiLKIZwDlmkHqFCsBtdFfGhZOeIMEo=2098913949.936600213766372547471059263951;if (IiLKIZwDlmkHqFCsBtdFfGhZOeIMEo == IiLKIZwDlmkHqFCsBtdFfGhZOeIMEo ) IiLKIZwDlmkHqFCsBtdFfGhZOeIMEo=553738482.010714249719101737745533709854; else IiLKIZwDlmkHqFCsBtdFfGhZOeIMEo=291121964.737285676931986205853732758702;if (IiLKIZwDlmkHqFCsBtdFfGhZOeIMEo == IiLKIZwDlmkHqFCsBtdFfGhZOeIMEo ) IiLKIZwDlmkHqFCsBtdFfGhZOeIMEo=1950574308.659508393626181476851410101563; else IiLKIZwDlmkHqFCsBtdFfGhZOeIMEo=1733271880.798449248891324865139306224040;float qRxuPslXIMMmBdyHthSVfUPUXvKIKa=1154783653.227488165112183289078951915215f;if (qRxuPslXIMMmBdyHthSVfUPUXvKIKa - qRxuPslXIMMmBdyHthSVfUPUXvKIKa> 0.00000001 ) qRxuPslXIMMmBdyHthSVfUPUXvKIKa=1091408565.096193236102757364433555937757f; else qRxuPslXIMMmBdyHthSVfUPUXvKIKa=295273287.405122610016407620465189355226f;if (qRxuPslXIMMmBdyHthSVfUPUXvKIKa - qRxuPslXIMMmBdyHthSVfUPUXvKIKa> 0.00000001 ) qRxuPslXIMMmBdyHthSVfUPUXvKIKa=584996739.658971774686552725832830332797f; else qRxuPslXIMMmBdyHthSVfUPUXvKIKa=157661279.740546953705789074764059492875f;if (qRxuPslXIMMmBdyHthSVfUPUXvKIKa - qRxuPslXIMMmBdyHthSVfUPUXvKIKa> 0.00000001 ) qRxuPslXIMMmBdyHthSVfUPUXvKIKa=1001233623.908704611687314716237904479215f; else qRxuPslXIMMmBdyHthSVfUPUXvKIKa=410908877.529991819547428885579774810543f;if (qRxuPslXIMMmBdyHthSVfUPUXvKIKa - qRxuPslXIMMmBdyHthSVfUPUXvKIKa> 0.00000001 ) qRxuPslXIMMmBdyHthSVfUPUXvKIKa=1291347453.550627852400780159660575785619f; else qRxuPslXIMMmBdyHthSVfUPUXvKIKa=428972574.138709791104328910586298419595f;if (qRxuPslXIMMmBdyHthSVfUPUXvKIKa - qRxuPslXIMMmBdyHthSVfUPUXvKIKa> 0.00000001 ) qRxuPslXIMMmBdyHthSVfUPUXvKIKa=550856971.234437318147957725887624405139f; else qRxuPslXIMMmBdyHthSVfUPUXvKIKa=1618032964.130707000170244945189482872114f;if (qRxuPslXIMMmBdyHthSVfUPUXvKIKa - qRxuPslXIMMmBdyHthSVfUPUXvKIKa> 0.00000001 ) qRxuPslXIMMmBdyHthSVfUPUXvKIKa=1133408622.338912836073672758057006085994f; else qRxuPslXIMMmBdyHthSVfUPUXvKIKa=1389760.590695334311011782012526733203f;float xRMZpQxqlWJWhTbCtbQvOqsGEkTubG=1401678990.569230796223566534906895339752f;if (xRMZpQxqlWJWhTbCtbQvOqsGEkTubG - xRMZpQxqlWJWhTbCtbQvOqsGEkTubG> 0.00000001 ) xRMZpQxqlWJWhTbCtbQvOqsGEkTubG=1964739148.170618182626717643636467077491f; else xRMZpQxqlWJWhTbCtbQvOqsGEkTubG=1760915713.214526405855348029879613671375f;if (xRMZpQxqlWJWhTbCtbQvOqsGEkTubG - xRMZpQxqlWJWhTbCtbQvOqsGEkTubG> 0.00000001 ) xRMZpQxqlWJWhTbCtbQvOqsGEkTubG=721412853.279152230234898123905863260195f; else xRMZpQxqlWJWhTbCtbQvOqsGEkTubG=1008644706.513178720790074683656958042699f;if (xRMZpQxqlWJWhTbCtbQvOqsGEkTubG - xRMZpQxqlWJWhTbCtbQvOqsGEkTubG> 0.00000001 ) xRMZpQxqlWJWhTbCtbQvOqsGEkTubG=1930732380.214572282050331699078764967614f; else xRMZpQxqlWJWhTbCtbQvOqsGEkTubG=2124679982.714159650251161772855372165162f;if (xRMZpQxqlWJWhTbCtbQvOqsGEkTubG - xRMZpQxqlWJWhTbCtbQvOqsGEkTubG> 0.00000001 ) xRMZpQxqlWJWhTbCtbQvOqsGEkTubG=356657914.668935728304083860430596954210f; else xRMZpQxqlWJWhTbCtbQvOqsGEkTubG=2109592336.460979333605556240940683318219f;if (xRMZpQxqlWJWhTbCtbQvOqsGEkTubG - xRMZpQxqlWJWhTbCtbQvOqsGEkTubG> 0.00000001 ) xRMZpQxqlWJWhTbCtbQvOqsGEkTubG=1097755017.252895511554793614182438199777f; else xRMZpQxqlWJWhTbCtbQvOqsGEkTubG=343112780.122546377273959690071524883581f;if (xRMZpQxqlWJWhTbCtbQvOqsGEkTubG - xRMZpQxqlWJWhTbCtbQvOqsGEkTubG> 0.00000001 ) xRMZpQxqlWJWhTbCtbQvOqsGEkTubG=1957088912.685444335834801969050070770977f; else xRMZpQxqlWJWhTbCtbQvOqsGEkTubG=1774357505.389669851546525772493564172687f;int TfrbZZKUHEAmZDUWYczzpbnSsLZWrn=1799620070;if (TfrbZZKUHEAmZDUWYczzpbnSsLZWrn == TfrbZZKUHEAmZDUWYczzpbnSsLZWrn- 0 ) TfrbZZKUHEAmZDUWYczzpbnSsLZWrn=784241079; else TfrbZZKUHEAmZDUWYczzpbnSsLZWrn=882382330;if (TfrbZZKUHEAmZDUWYczzpbnSsLZWrn == TfrbZZKUHEAmZDUWYczzpbnSsLZWrn- 0 ) TfrbZZKUHEAmZDUWYczzpbnSsLZWrn=1969860842; else TfrbZZKUHEAmZDUWYczzpbnSsLZWrn=1824864234;if (TfrbZZKUHEAmZDUWYczzpbnSsLZWrn == TfrbZZKUHEAmZDUWYczzpbnSsLZWrn- 1 ) TfrbZZKUHEAmZDUWYczzpbnSsLZWrn=1552866042; else TfrbZZKUHEAmZDUWYczzpbnSsLZWrn=411168861;if (TfrbZZKUHEAmZDUWYczzpbnSsLZWrn == TfrbZZKUHEAmZDUWYczzpbnSsLZWrn- 1 ) TfrbZZKUHEAmZDUWYczzpbnSsLZWrn=998809546; else TfrbZZKUHEAmZDUWYczzpbnSsLZWrn=390696305;if (TfrbZZKUHEAmZDUWYczzpbnSsLZWrn == TfrbZZKUHEAmZDUWYczzpbnSsLZWrn- 1 ) TfrbZZKUHEAmZDUWYczzpbnSsLZWrn=1750747342; else TfrbZZKUHEAmZDUWYczzpbnSsLZWrn=1381490015;if (TfrbZZKUHEAmZDUWYczzpbnSsLZWrn == TfrbZZKUHEAmZDUWYczzpbnSsLZWrn- 1 ) TfrbZZKUHEAmZDUWYczzpbnSsLZWrn=136496569; else TfrbZZKUHEAmZDUWYczzpbnSsLZWrn=299361171;long PYxucHXVuUQLywpmqDUSnXrMdIDUqx=1765939866;if (PYxucHXVuUQLywpmqDUSnXrMdIDUqx == PYxucHXVuUQLywpmqDUSnXrMdIDUqx- 1 ) PYxucHXVuUQLywpmqDUSnXrMdIDUqx=83423576; else PYxucHXVuUQLywpmqDUSnXrMdIDUqx=548895318;if (PYxucHXVuUQLywpmqDUSnXrMdIDUqx == PYxucHXVuUQLywpmqDUSnXrMdIDUqx- 1 ) PYxucHXVuUQLywpmqDUSnXrMdIDUqx=1916016379; else PYxucHXVuUQLywpmqDUSnXrMdIDUqx=1070026545;if (PYxucHXVuUQLywpmqDUSnXrMdIDUqx == PYxucHXVuUQLywpmqDUSnXrMdIDUqx- 1 ) PYxucHXVuUQLywpmqDUSnXrMdIDUqx=614865878; else PYxucHXVuUQLywpmqDUSnXrMdIDUqx=1010456073;if (PYxucHXVuUQLywpmqDUSnXrMdIDUqx == PYxucHXVuUQLywpmqDUSnXrMdIDUqx- 1 ) PYxucHXVuUQLywpmqDUSnXrMdIDUqx=1371534920; else PYxucHXVuUQLywpmqDUSnXrMdIDUqx=1114212230;if (PYxucHXVuUQLywpmqDUSnXrMdIDUqx == PYxucHXVuUQLywpmqDUSnXrMdIDUqx- 1 ) PYxucHXVuUQLywpmqDUSnXrMdIDUqx=2086099020; else PYxucHXVuUQLywpmqDUSnXrMdIDUqx=622888050;if (PYxucHXVuUQLywpmqDUSnXrMdIDUqx == PYxucHXVuUQLywpmqDUSnXrMdIDUqx- 1 ) PYxucHXVuUQLywpmqDUSnXrMdIDUqx=1207389909; else PYxucHXVuUQLywpmqDUSnXrMdIDUqx=658120843;double quqeuurhRjVKjZgScOXzXEcBEUOEPd=1150462622.658294321504928996653952578805;if (quqeuurhRjVKjZgScOXzXEcBEUOEPd == quqeuurhRjVKjZgScOXzXEcBEUOEPd ) quqeuurhRjVKjZgScOXzXEcBEUOEPd=1488330554.383417362457787706782616180683; else quqeuurhRjVKjZgScOXzXEcBEUOEPd=1481272199.752623161164421637687213387834;if (quqeuurhRjVKjZgScOXzXEcBEUOEPd == quqeuurhRjVKjZgScOXzXEcBEUOEPd ) quqeuurhRjVKjZgScOXzXEcBEUOEPd=1364632328.779980617465662929656310755935; else quqeuurhRjVKjZgScOXzXEcBEUOEPd=1917500231.643850655212122375256829946934;if (quqeuurhRjVKjZgScOXzXEcBEUOEPd == quqeuurhRjVKjZgScOXzXEcBEUOEPd ) quqeuurhRjVKjZgScOXzXEcBEUOEPd=1112905068.678393282338554439848079982501; else quqeuurhRjVKjZgScOXzXEcBEUOEPd=530317801.472264594020403948552729660041;if (quqeuurhRjVKjZgScOXzXEcBEUOEPd == quqeuurhRjVKjZgScOXzXEcBEUOEPd ) quqeuurhRjVKjZgScOXzXEcBEUOEPd=1996296595.189196649094683978615505076361; else quqeuurhRjVKjZgScOXzXEcBEUOEPd=1553570287.466305231730057824780343678423;if (quqeuurhRjVKjZgScOXzXEcBEUOEPd == quqeuurhRjVKjZgScOXzXEcBEUOEPd ) quqeuurhRjVKjZgScOXzXEcBEUOEPd=762106167.139964939985798566926278123383; else quqeuurhRjVKjZgScOXzXEcBEUOEPd=330716548.881971229499581665512260897185;if (quqeuurhRjVKjZgScOXzXEcBEUOEPd == quqeuurhRjVKjZgScOXzXEcBEUOEPd ) quqeuurhRjVKjZgScOXzXEcBEUOEPd=1405381011.166720483844754779866887262229; else quqeuurhRjVKjZgScOXzXEcBEUOEPd=549242365.525533397128378861712842948057;int MhWSKzNMBVqKfSqdFsVBFYGihnLdvZ=597568465;if (MhWSKzNMBVqKfSqdFsVBFYGihnLdvZ == MhWSKzNMBVqKfSqdFsVBFYGihnLdvZ- 1 ) MhWSKzNMBVqKfSqdFsVBFYGihnLdvZ=1044081392; else MhWSKzNMBVqKfSqdFsVBFYGihnLdvZ=191149046;if (MhWSKzNMBVqKfSqdFsVBFYGihnLdvZ == MhWSKzNMBVqKfSqdFsVBFYGihnLdvZ- 0 ) MhWSKzNMBVqKfSqdFsVBFYGihnLdvZ=1690933929; else MhWSKzNMBVqKfSqdFsVBFYGihnLdvZ=523397906;if (MhWSKzNMBVqKfSqdFsVBFYGihnLdvZ == MhWSKzNMBVqKfSqdFsVBFYGihnLdvZ- 1 ) MhWSKzNMBVqKfSqdFsVBFYGihnLdvZ=1779345264; else MhWSKzNMBVqKfSqdFsVBFYGihnLdvZ=1207159512;if (MhWSKzNMBVqKfSqdFsVBFYGihnLdvZ == MhWSKzNMBVqKfSqdFsVBFYGihnLdvZ- 0 ) MhWSKzNMBVqKfSqdFsVBFYGihnLdvZ=1184754192; else MhWSKzNMBVqKfSqdFsVBFYGihnLdvZ=1921268653;if (MhWSKzNMBVqKfSqdFsVBFYGihnLdvZ == MhWSKzNMBVqKfSqdFsVBFYGihnLdvZ- 1 ) MhWSKzNMBVqKfSqdFsVBFYGihnLdvZ=1658331057; else MhWSKzNMBVqKfSqdFsVBFYGihnLdvZ=855648612;if (MhWSKzNMBVqKfSqdFsVBFYGihnLdvZ == MhWSKzNMBVqKfSqdFsVBFYGihnLdvZ- 0 ) MhWSKzNMBVqKfSqdFsVBFYGihnLdvZ=1886248861; else MhWSKzNMBVqKfSqdFsVBFYGihnLdvZ=929977992; }
 MhWSKzNMBVqKfSqdFsVBFYGihnLdvZy::MhWSKzNMBVqKfSqdFsVBFYGihnLdvZy()
 { this->HVzoUWOYAuss("WBPxHoRdnMrfgQdmqCnkFBfjoankWnHVzoUWOYAussj", true, 1351262599, 340688703, 1936876340); }
#pragma optimize("", off)
 // <delete/>

