#pragma once

class i_client_entity;

class i_move_helper
{
public:
    virtual	void _vpad() = 0;
    virtual void set_host(i_client_entity* host) = 0;
};
