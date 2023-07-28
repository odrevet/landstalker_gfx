#ifndef _DOORS_H_
#define _DOORS_H_

#include <vector>
#include <map>
#include <cstdint>
#include <string>
#include <Literals.h>

struct Door
{
	enum class Size : uint8_t
	{
		DOOR_1X4,
		DOOR_2X4,
		DOOR_2X5,
		DOOR_1X0
	};
	inline static const std::map<Size, std::pair<uint8_t, uint8_t>> SIZES =
	{ {
		{Size::DOOR_1X4, {1_u8, 4_u8}},
		{Size::DOOR_2X4, {2_u8, 4_u8}},
		{Size::DOOR_2X5, {2_u8, 5_u8}},
		{Size::DOOR_1X0, {1_u8, 0_u8}}
	} };
	inline static const std::map<Size, std::string> SIZE_NAMES =
	{ {
		{Size::DOOR_1X4, "1x4 Door"},
		{Size::DOOR_2X4, "2x4 Door"},
		{Size::DOOR_2X5, "2x5 Door"},
		{Size::DOOR_1X0, "1x0 Door"}
	} };
	uint8_t x;
	uint8_t y;
	Size size;

	Door(uint8_t b1, uint8_t b2);
	Door() : x(0), y(0), size(Size::DOOR_1X4) {}

	std::pair<uint8_t, uint8_t> GetBytes() const;

	bool operator==(const Door& rhs) const;
	bool operator!=(const Door& rhs) const;
};

class Doors
{
public:
	Doors(const std::vector<uint8_t>& offsets, const std::vector<uint8_t>& bytes);
	Doors();

	bool operator==(const Doors& rhs) const;
	bool operator!=(const Doors& rhs) const;

	std::pair<std::vector<uint8_t>, std::vector<uint8_t>> GetData(int roomcount) const;
	std::vector<Door> GetDoorsForRoom(uint16_t room) const;
	bool RoomHasDoors(uint16_t room) const;
	void SetRoomDoors(uint16_t room, const std::vector<Door>& swaps);
private:
	std::map<uint16_t, std::vector<Door>> m_doors;
};

#endif // _DOORS_H_
