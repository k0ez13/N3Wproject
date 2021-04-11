#pragma once

#include <string>

enum send_prop_type
{
    DPT_Int = 0,
    DPT_Float,
    DPT_Vector,
    DPT_VectorXY,
    DPT_String,
    DPT_Array,
    DPT_DataTable,
    DPT_Int64,
    DPT_NUMSendPropTypes
};

class d_variant
{
public:
    union
    {
        float	   m_Float;
        long	   m_Int;
        char*    m_pString;
        void*    m_pData;
        float	   m_Vector[3];
        __int64  m_Int64;
    };
    send_prop_type  m_Type;
};

class recv_table;
class recv_prop;

class c_recv_proxy_data
{
public:
    const recv_prop*     m_pRecvProp;        // The property it's receiving.
    d_variant		    m_Value;            // The value given to you to store.
    int				    m_iElement;         // Which array element you're getting.
    int				    m_ObjectID;         // The object being referred to.
};

//-----------------------------------------------------------------------------
// pStruct = the base structure of the datatable this variable is in (like c_base_entity)
// pOut    = the variable that this this proxy represents (like c_base_entity::m_SomeValue).
//
// Convert the network-standard-type value in m_Value into your own format in pStruct/pOut.
//-----------------------------------------------------------------------------
typedef void(*recv_var_proxy_fn)(const c_recv_proxy_data *pData, void *pStruct, void *pOut);

// ------------------------------------------------------------------------ //
// ArrayLengthRecvProxies are optionally used to Get the length of the 
// incoming array when it changes.
// ------------------------------------------------------------------------ //
typedef void(*array_length_recv_proxy_fn)(void *pStruct, int objectID, int currentArrayLength);

// NOTE: DataTable receive proxies work differently than the other proxies.
// pData points at the object + the recv table's offset.
// pOut should be Set to the location of the object to unpack the data table into.
// If the parent object just contains the child object, the default proxy just does *pOut = pData.
// If the parent object points at the child object, you need to dereference the pointer here.
// NOTE: don't ever return null from a DataTable receive proxy function. Bad things will happen.
typedef void(*data_table_recv_var_proxy_fn)(const recv_prop *pProp, void **pOut, void *pData, int objectID);

class recv_prop
{
public:
    char*                           m_pVarName;
    send_prop_type                  m_RecvType;
    int                             m_Flags;
    int                             m_StringBufferSize;
    int                             m_bInsideArray;
    const void*                     m_pExtraData;
    recv_prop*                      m_pArrayProp;
    array_length_recv_proxy_fn      m_ArrayLengthProxy;
    recv_var_proxy_fn               m_ProxyFn;
    data_table_recv_var_proxy_fn    m_DataTableProxyFn;
    recv_table*                     m_pDataTable;
    int                             m_Offset;
    int                             m_ElementStride;
    int                             m_nElements;
    const char*                     m_pParentArrayPropName;

    recv_var_proxy_fn		        get_proxy_fn() const;
    void					        set_proxy_fn(recv_var_proxy_fn fn);
    data_table_recv_var_proxy_fn	get_data_table_proxy_fn() const;
    void					        set_data_table_proxy_fn(data_table_recv_var_proxy_fn fn);

};

class recv_table
{
public:
    recv_prop*              m_pProps;
    int                     m_nProps;
    void*                   m_pDecoder;
    char*                   m_pNetTableName;
    bool                    m_bInitialized;
    bool                    m_bInMainList;
};

recv_var_proxy_fn recv_prop::get_proxy_fn() const
{
    return m_ProxyFn;
}

void recv_prop::set_proxy_fn(recv_var_proxy_fn fn)
{
    m_ProxyFn = fn;
}

data_table_recv_var_proxy_fn recv_prop::get_data_table_proxy_fn() const
{
    return m_DataTableProxyFn;
}

void recv_prop::set_data_table_proxy_fn(data_table_recv_var_proxy_fn fn)
{
    m_DataTableProxyFn = fn;
}

class recv_prop_hook
{
public:
	recv_prop_hook(recv_prop* prop, const recv_var_proxy_fn proxy_fn) : m_property(prop), m_original_proxy_fn(prop->m_ProxyFn)
	{
		set_proxy_function(proxy_fn);
	}
	~recv_prop_hook()
	{
		m_property->m_ProxyFn = m_original_proxy_fn;
	}
	auto get_original_function() const -> recv_var_proxy_fn
	{
		return m_original_proxy_fn;
	}
	auto set_proxy_function(const recv_var_proxy_fn proxy_fn) const -> void
	{
		m_property->m_ProxyFn = proxy_fn;
	}
private:
	recv_prop* m_property;
	recv_var_proxy_fn m_original_proxy_fn;
};