#pragma once

#include "Recv.h"

class client_class;
class i_client_networkable;

typedef i_client_networkable* (*Createclient_classFn)(int entnum, int serialNum);
typedef i_client_networkable* (*CreateEventFn)();

class client_class
{
public:
    Createclient_classFn      m_pCreateFn;
    CreateEventFn            m_pCreateEventFn;
    char*                    m_pNetworkName;
    recv_table*               m_pRecvTable;
    client_class*             m_pNext;
    ClassId                  m_ClassID;
};