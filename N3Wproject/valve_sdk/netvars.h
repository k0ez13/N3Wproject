#pragma once
#include <memory>
#include <iostream>
#include <unordered_map>

#include "misc/Recv.h"

namespace netvar_sys
{
    struct netvar_table
    {
        std::string               name;
        recv_prop* prop;
        uint32_t                  offset;
        std::vector<recv_prop*>   child_props;
        std::vector<netvar_table> child_tables;
    };

    void initialize();


    void dump();
    void dump(std::ostream& stream);

    uint32_t      get_offset(const std::string& tableName, const std::string& propName);
    recv_prop* get_netvar_prop(const std::string& tableName, const std::string& propName);

    static netvar_table  load_table(recv_table* client_class);
    static void          dump_table(std::ostream& stream, const netvar_table& table, uint32_t indentation);
    static uint32_t      get_offset(const netvar_table& table, const std::string& propName);
    static recv_prop* get_netvar_prop(const netvar_table& table, const std::string& propName);

    std::vector<netvar_table>  database;
}