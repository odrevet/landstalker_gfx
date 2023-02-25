#include "Room.h"

#include <cassert>

Room::Room(const std::string& map_name, const std::vector<uint8_t>& params)
    : map(map_name)
{
    assert(params.size() == 4);
    SetParams(params[0], params[1], params[2], params[3]);
}

Room::Room(const std::string& map_name, uint8_t params[4])
    : map(map_name)
{
    SetParams(params[0], params[1], params[2], params[3]);
}

void Room::SetParams(uint8_t param0, uint8_t param1, uint8_t param2, uint8_t param3)
{
    unknown_param1 = param0 >> 6;
    pri_blockset = (param0 >> 5) & 0x01;
    tileset = param0 & 0x1F;
    unknown_param2 = param1 >> 6;
    room_palette = param1 & 0x3F;
    room_z_end = param2 >> 4;
    room_z_begin = param2 & 0x0F;
    sec_blockset = param3 >> 5;
    bgm = param3 & 0x1F;
}

std::array<uint8_t, 4> Room::GetParams() const
{
    std::array<uint8_t, 4> ret;
    ret[0] = ((unknown_param1 & 0x03) << 6) | ((pri_blockset & 0x01) << 5) | (tileset & 0x1F);
    ret[1] = ((unknown_param2 & 0x03) << 6) | (room_palette & 0x3F);
    ret[2] = ((room_z_end & 0x0F) << 4) | (room_z_begin & 0x0F);
    ret[3] = ((sec_blockset & 0x07) << 5) | (bgm & 0x1F);
    return ret;
}

uint8_t Room::GetBlocksetId() const
{
    return pri_blockset << 5 | tileset;
}
