#ifndef _TILESET_H_
#define _TILESET_H_

#include <vector>
#include <cstdint>

class Tileset
{
	enum Format
	{
		BINARY,
		LZ77
	};
	Tileset(const std::vector<uint8_t>& tiles, Format format = BINARY, size_t bits_per_pixel = 4, size_t tile_height = 8, size_t tile_width = 8);

	std::vector<uint8_t> GetBinaryFile(Format format = BINARY, size_t bits_per_pixel = 4);
};

#endif _TILESET_H_
