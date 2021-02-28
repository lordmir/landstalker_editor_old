#ifndef _TILEMAP_2D_RLE_
#define _TILEMAP_2D_RLE_

#include <vector>
#include <cstdint>
#include <cstdlib>
#include "Tile.h"

class Tilemap2D
{
public:
	enum MapDataType
	{
		UNCOMPRESSED,
		LZ77,
		RLE,
		SPRITE
	};

	enum MapAutoType
	{
		EMPTY,
		AUTO,
		AUTO_TRANSPOSED,
		AUTO_6x4
	};

	Tilemap2D();
	Tilemap2D(MapAutoType type, size_t width, size_t height, size_t base = 0);
	Tilemap2D(MapDataType type, const uint8_t* data, size_t size, size_t width = 0, size_t height = 0);
	void GenerateMap(MapAutoType type, size_t width, size_t height, size_t base = 0);
	void LoadMap(MapDataType type, const uint8_t* data, size_t size, size_t width = 0, size_t height = 0);
	std::vector<uint8_t> CompressRLE() const;
	void DecompressRLE(const uint8_t* data, size_t size);
	size_t GetHeight() const;
	size_t GetWidth() const;
	Tile GetTile(size_t x, size_t y) const;
	void SetTile(const Tile& tile, size_t x, size_t y);
	Tile* Data();
private:
	size_t m_width;
	size_t m_height;
	std::vector<Tile> m_tiles;
};

#endif // _TILEMAP_2D_RLE_
