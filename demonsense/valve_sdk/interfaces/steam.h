#pragma once

typedef int32_t HSteamPipe;
typedef int32_t HSteamUser;

enum EGCResults
{
	k_EGCResultOK = 0,
	k_EGCResultNoMessage = 1,           // There is no message in the queue
	k_EGCResultBufferTooSmall = 2,      // The buffer is too small for the requested message
	k_EGCResultNotLoggedOn = 3,         // The client is not logged onto Steam
	k_EGCResultInvalidMessage = 4,      // Something was wrong with the message being sent with SendMessage
};

class i_steam_game_coordinator
{
public:
	virtual EGCResults gc_send_message(int unMsgType, const void* pubData, int cubData) = 0;
	virtual bool is_message_available(int* pcubMsgSize) = 0;
	virtual EGCResults retrieve_message(int* punMsgType, void* pubDest, int cubDest, int* pcubMsgSize) = 0;

};

class CSteamID
{
public:
	CSteamID()
	{
		m_steamid.m_comp.m_unAccountID = 0;
		m_steamid.m_comp.m_EAccountType = 0;
		m_steamid.m_comp.m_EUniverse = 0;
		m_steamid.m_comp.m_unAccountInstance = 0;
	}
	uint32_t GetAccountID() const { return m_steamid.m_comp.m_unAccountID; }

private:
	union SteamID_t
	{
		struct SteamIDComponent_t
		{
			uint32_t			m_unAccountID : 32;			// unique account identifier
			unsigned int		m_unAccountInstance : 20;	// dynamic instance ID (used for multiseat type accounts only)
			unsigned int		m_EAccountType : 4;			// type of account - can't show as EAccountType, due to signed / unsigned difference
			int					m_EUniverse : 8;	// universe this account belongs to
		} m_comp;

		uint64_t m_unAll64Bits;
	} m_steamid;
};

class i_steam_user
{
public:
	virtual uint32_t get_hsteam_user() = 0;
	virtual bool b_logged_on() = 0;
	virtual CSteamID get_steam_id() = 0;
};

using HTTPRequestHandle = std::uint32_t;
enum : HTTPRequestHandle { INVALID_HTTPREQUEST_HANDLE = 0 };

using SteamAPICall_t = std::uint64_t;
enum : SteamAPICall_t { k_uAPICallInvalid = 0 };

enum class EHTTPMethod
{
	Invalid = 0,
	GET,
	HEAD,
	POST
};

class i_steam_http
{
public:
	virtual HTTPRequestHandle     create_http_request(EHTTPMethod eHTTPRequestMethod, const char* pchAbsoluteURL) = 0;
	virtual bool                  set_http_request_context_value(HTTPRequestHandle hRequest, std::uint64_t ulContextValue) = 0;
	virtual bool                  set_http_request_network_activity_timeout(HTTPRequestHandle hRequest, std::uint32_t unTimeoutSeconds) = 0;
	virtual bool                  set_http_request_header_value(HTTPRequestHandle hRequest, const char* pchHeaderName, const char* pchHeaderValue) = 0;
	virtual bool                  set_http_request_get_or_post_parameter(HTTPRequestHandle hRequest, const char* pchParamName, const char* pchParamValue) = 0;
	virtual bool                  send_http_request(HTTPRequestHandle hRequest, SteamAPICall_t* pCallHandle) = 0;
	virtual bool                  send_http_request_and_stream_response(HTTPRequestHandle hRequest, SteamAPICall_t* pCallHandle) = 0;
	virtual bool                  defer_http_request(HTTPRequestHandle hRequest) = 0;
	virtual bool                  prioritize_http_request(HTTPRequestHandle hRequest) = 0;
	virtual bool                  get_http_response_header_size(HTTPRequestHandle hRequest, const char* pchHeaderName, std::uint32_t* unResponseHeaderSize) = 0;
	virtual bool                  get_http_response_header_value(HTTPRequestHandle hRequest, const char* pchHeaderName, std::uint8_t* pHeaderValueBuffer, std::uint32_t unBufferSize) = 0;
	virtual bool                  get_http_response_body_size(HTTPRequestHandle hRequest, std::uint32_t* unBodySize) = 0;
	virtual bool                  get_http_response_body_data(HTTPRequestHandle hRequest, std::uint8_t* pBodyDataBuffer, std::uint32_t unBufferSize) = 0;
	virtual bool                  get_http_streaming_response_body_data(HTTPRequestHandle hRequest, std::uint32_t cOffset, std::uint8_t* pBodyDataBuffer, std::uint32_t unBufferSize) = 0;
	virtual bool                  release_http_request(HTTPRequestHandle hRequest) = 0;
	virtual bool                  get_http_download_progress_pct(HTTPRequestHandle hRequest, float* pflPercentOut) = 0;
	virtual bool                  set_http_request_raw_post_body(HTTPRequestHandle hRequest, const char* pchContentType, std::uint8_t* pubBody, std::uint32_t unBodyLen) = 0;
};

class i_steam_friends;

class i_steam_client
{
public:
	i_steam_user* get_i_steam_user(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char* pchVersion)
	{
		typedef i_steam_user* (__stdcall * func)(HSteamUser, HSteamPipe, const char*);
		return CallVFunction<func>(this, 5)(hSteamUser, hSteamPipe, pchVersion);
	}

	i_steam_friends* get_i_steam_friends(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char* pchVersion)
	{
		typedef i_steam_friends* (__stdcall * func)(HSteamUser, HSteamPipe, const char*);
		return CallVFunction<func>(this, 8)(hSteamUser, hSteamPipe, pchVersion);
	}

	i_steam_game_coordinator* get_i_steamgeneric_interface(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char* pchVersion)
	{
		typedef i_steam_game_coordinator* (__stdcall * func)(HSteamUser, HSteamPipe, const char*);
		return CallVFunction<func>(this, 12)(hSteamUser, hSteamPipe, pchVersion);
	}

	i_steam_http* get_i_steam_http(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char* pchVersion)
	{
		typedef i_steam_http* (__stdcall * func)(HSteamUser, HSteamPipe, const char*);
		return CallVFunction<func>(this, 23)(hSteamUser, hSteamPipe, pchVersion);
	}
};