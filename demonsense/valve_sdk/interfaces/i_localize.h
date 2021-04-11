#pragma once
using localize_string_index_t = unsigned;
class i_localize_text_query
{
public:
	virtual int compute_text_width(const wchar_t* pString) = 0;
};
class i_localization_change_callback
{
public:
	virtual void on_localization_changed() = 0;
};
class i_localize : public i_app_system
{
public:
	virtual bool                    add_file(const char* fileName, const char* pPathID = nullptr, bool bIncludeFallbackSearchPaths = false) = 0;
	virtual void                    remove_all() = 0;
	virtual wchar_t*                find(const char* tokenName) = 0;
	virtual const wchar_t*          find_safe(const char* tokenName) = 0;
	virtual int                     convert_ansi_to_unicode(const char* ansi, wchar_t* unicode, int unicodeBufferSizeInBytes) = 0;
	virtual int                     convert_unicode_to_ansi(const wchar_t* unicode, char* ansi, int ansiBufferSize) = 0;
	virtual localize_string_index_t find_index(const char* tokenName) = 0;
	virtual void                    construct_string(wchar_t* unicodeOuput, int unicodeBufferSizeInBytes, const wchar_t* formatString, int numFormatParameters, ...) = 0;
	virtual const char*             get_name_by_index(localize_string_index_t index) = 0;
	virtual wchar_t*                get_value_by_index(localize_string_index_t index) = 0;
};