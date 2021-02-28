#include <stdexcept>
#include <iostream>
#include <numeric>
#include <algorithm>

#include "Tilemap2DRLE.h"
#include "LZ77.h"


Tilemap2D::Tilemap2D()
{
}

Tilemap2D::Tilemap2D(MapAutoType type, size_t width, size_t height, size_t base)
{
	GenerateMap(type, width, height, base);
}

Tilemap2D::Tilemap2D(MapDataType type, const uint8_t* data, size_t size, size_t width, size_t height)
{
	LoadMap(type, data, size, width, height);
}

void Tilemap2D::GenerateMap(MapAutoType type, size_t width, size_t height, size_t base)
{
	m_tiles.clear();
	m_width = width;
	m_height = height;
	m_tiles.resize(m_width * m_height);
	switch (type)
	{
	case EMPTY:
		std::fill(m_tiles.begin(), m_tiles.end(), base);
		break;
	case AUTO:
		std::iota(m_tiles.begin(), m_tiles.end(), base);
		break;
	case AUTO_TRANSPOSED:
		for (size_t y = 0; y < height; y++)
		{
			for (size_t x = 0; x < width; x++)
			{
				m_tiles[x + y * width] = y + x * height + base;
			}
		}
		break;
	case AUTO_6x4:
		size_t cc = base;
		for (size_t yy = 0; yy < height; yy += 6)
		{
			for (size_t xx = 0; xx < width; xx += 4)
			{
				for (size_t x = 0; x < 4; x++)
				{
					for (size_t y = 0; y < 4; y++)
					{
						if ((x + xx < width) && (y + yy < height))
						{
							m_tiles[(x + xx) + (y + yy) * width] = cc++;
						}
					}
				}
				for (size_t x = 0; x < 4; x++)
				{
					for (size_t y = 4; y < 6; y++)
					{
						if ((x + xx < width) && (y + yy < height))
						{
							m_tiles[(x + xx) + (y + yy) * width] = cc++;
						}
					}
				}
			}
		}
		break;
	}
}

void Tilemap2D::LoadMap(MapDataType type, const uint8_t* data, size_t size, size_t width, size_t height)
{
	m_tiles.clear();
	m_width = width;
	m_height = height;
	switch (type)
	{
	case UNCOMPRESSED:
		m_tiles.resize(m_width * m_height);
		for (auto& tile : m_tiles)
		{
			tile.SetTileValue((data[0] << 8) | data[1]);
			data += 2;
		}
		break;
	case RLE:
		DecompressRLE(data, size);
		break;
	case LZ77:
		{
			std::vector<uint8_t> decoded(65536);
			size_t elen;
			decoded.resize(LZ77::Decode(data, size, decoded.data(), elen));
			m_width = decoded[0] / 2;
			m_height = decoded[3];
			m_tiles.resize(m_width * m_height);
			auto it = decoded.begin() + 4;
			for (auto& tile : m_tiles)
			{
				if (it != decoded.end())
				{
					tile.SetTileValue((it[0] << 8) | it[1]);
					it += 2;
				}
			}
		}
		break;
	case SPRITE:
		break;
	}
}

void Tilemap2D::DecompressRLE(const uint8_t* compressed_data, size_t size)
{
	const uint8_t* data = compressed_data;
	if (size <= 6)
	{
		throw std::runtime_error("Compressed data size is too small");
	}
	m_width = *data++;
	m_height = *data++;
	m_tiles.reserve(m_width * m_height);

	// First, the tile attributes (priority, palette, HFLIP, VFLIP)
	// These are stored using simple RLE
	bool done = false;
	size_t idx = 0;
	while (done == false)
	{
		uint16_t attrs = (*data & 0xF8) << 8;
		uint16_t length = *data++ & 0x03;
		if (data >= (compressed_data + size))
		{
			throw std::runtime_error("Unexpected end of compressed data.");
		}
		if ((*(data - 1) & 0x04) == 0)
		{
			length <<= 8;
			length |= *data++;
			if (length == 0)
			{
				// End of attributes
				done = true;
				break;
			}
			else if (data >= (compressed_data + size))
			{
				throw std::runtime_error("Unexpected end of compressed data.");
			}
		}
		if (length + idx >= m_width * m_height)
		{
			throw std::runtime_error("Compressed data is bad or corrupted.");
		}
		do
		{
			m_tiles.push_back(attrs);
		} while (length--);
	}
	if (m_tiles.size() != m_width * m_height)
	{
		throw std::runtime_error("Compressed data is bad or corrupted.");
	}
	// Next, we load the tile indicies
	idx = 0;
	done = false;
	uint16_t value = 0;
	uint16_t count = 0;
	uint16_t last_value = 0xBEEF;
	uint16_t incr_value = 0xBEEF;
	uint8_t cmd = 0;
	while (done == false)
	{
		cmd = *data >> 6;
		switch (cmd)
		{
		case 0: // Single tile copy
			if (data >= (compressed_data + size - 1))
			{
				throw std::runtime_error("Unexpected end of compressed data.");
			}
			value = *data++ << 8;
			value |= *data++;
			value &= 0x07FF;
			if (value == 0x7FF)
			{
				done = true;
			}
			else
			{
				if (idx >= m_tiles.size())
				{
					throw std::runtime_error("Compressed data is bad or corrupted.");
				}
				m_tiles[idx++].SetIndex(value);
			}
			break;
		case 1: // RLE tile copy
			if (data >= (compressed_data + size - 1))
			{
				throw std::runtime_error("Unexpected end of compressed data.");
			}
			count = ((*data & 0x38) >> 3);
			value = *data++ << 8;
			value |= *data++;
			value &= 0x07FF;
			if (idx + count >= m_tiles.size())
			{
				throw std::runtime_error("Compressed data is bad or corrupted.");
			}
			do
			{
				m_tiles[idx++].SetIndex(value);
			} while (count--);
			last_value = value;
			if (incr_value == 0xBEEF)
			{
				incr_value = value;
			}
			break;
		case 2: // RLE last tile copy
			count = *data++ & 0x3F;
			if (data >= (compressed_data + size))
			{
				throw std::runtime_error("Unexpected end of compressed data.");
			}
			if (last_value == 0xBEEF)
			{
				throw std::runtime_error("Compressed data is bad or corrupted.");
			}
			else if (idx + count >= m_tiles.size())
			{
				throw std::runtime_error("Compressed data is bad or corrupted.");
			}
			do
			{
				m_tiles[idx++].SetIndex(last_value);
			} while (count--);
			break;
		case 3: // RLE increment
			count = *data++ & 0x3F;
			if (data >= (compressed_data + size))
			{
				throw std::runtime_error("Unexpected end of compressed data.");
			}
			if (incr_value == 0xBEEF)
			{
				throw std::runtime_error("Compressed data is bad or corrupted.");
			}
			else if (idx + count >= m_tiles.size())
			{
				throw std::runtime_error("Compressed data is bad or corrupted.");
			}
			do
			{
				m_tiles[idx++].SetIndex(++incr_value);
			} while (count--);
			break;
		}
	}
}

std::vector<uint8_t> Tilemap2D::CompressRLE() const
{
	std::vector<uint8_t> cmp;

	cmp.push_back(m_width & 0xFF);
	cmp.push_back(m_height & 0xFF);
	
	// First, encode tile parameters as RLE
	size_t idx = 0;
	size_t count;
	while (idx < m_tiles.size())
	{
		for (count = 0; idx + count + 1 < m_tiles.size() && count < 0x400; ++count)
		{
			if ((m_tiles[idx].GetTileValue() & 0xF800) != (m_tiles[idx + count + 1].GetTileValue() & 0xF800))
			{
				break;
			}
		}
		uint8_t byte1 = (m_tiles[idx].GetTileValue() >> 8) & 0xF8;
		idx += count + 1;
		if (count > 0x04)
		{
			byte1 |= (count) >> 8;
			cmp.push_back(byte1);
			cmp.push_back(count & 0xFF);
		}
		else
		{
			byte1 |= (count) & 0x03;
			byte1 |= 0x04;
			cmp.push_back(byte1);
		}
	}
	cmp.push_back(0x00);
	cmp.push_back(0x00);
	// Next, encode RLE
	uint16_t last = m_tiles.front().GetIndex();
	uint16_t incr = m_tiles.front().GetIndex();
	// First, we need to write out a single RLE run in order to set up our registers
	count = 0;
	for (auto it = m_tiles.cbegin() + 1; it != m_tiles.cend(); ++it)
	{
		if (it->GetIndex() != m_tiles.cbegin()->GetIndex())
		{
			break;
		}
		count++;
		if (count >= 7)
		{
			break;
		}
	}
	uint8_t byte = 0x40 | ((count & 0x07) << 3) | ((last >> 8) & 0x07);
	cmp.push_back(byte);
	cmp.push_back(last & 0xFF);
	// Now we can proceed with the RLE encoding
	auto it = m_tiles.cbegin() + count + 1;
	while (it != m_tiles.cend())
	{
		if (it->GetIndex() == last)
		{
			// Best option : RLE Fill last
			count = 0;
			for (auto jt = it + 1; jt != m_tiles.cend(); ++jt)
			{
				if (jt->GetIndex() != last)
				{
					break;
				}
				count++;
				if (count >= 0x3F)
				{
					break;
				}
			}
			cmp.push_back(0x80 | (count & 0x3F));
			it += count + 1;
		}
		else if (it->GetIndex() == incr + 1)
		{
			// Best option : RLE Increment
			count = 0;
			incr++;
			for (auto jt = it + 1; jt != m_tiles.cend(); ++jt)
			{
				if (jt->GetIndex() != ++incr)
				{
					incr--;
					break;
				}
				count++;
				if (count >= 0x3F)
				{
					break;
				}
			}
			cmp.push_back(0xC0 | (count & 0x3F));
			it += count + 1;
		}
		else
		{
			if (std::next(it) != m_tiles.cend() && std::next(it)->GetIndex() == it->GetIndex())
			{
				// Best option : RLE Fill
				count = 0;
				for (auto jt = it + 1; jt != m_tiles.cend(); ++jt)
				{
					if (it->GetIndex() != jt->GetIndex())
					{
						break;
					}
					count++;
					if (count >= 7)
					{
						break;
					}
				}
				cmp.push_back(0x40 | ((it->GetIndex() >> 8) & 0x07) | ((count & 0x07) << 3));
				cmp.push_back(it->GetIndex() & 0xFF);
				last = it->GetIndex();
				it += count + 1;
			}
			else
			{
				// Best option : Copy
				cmp.push_back(it->GetIndex() >> 8);
				cmp.push_back(it->GetIndex() & 0xFF);
				it++;
			}
		}
	}


	cmp.push_back(0x07);
	cmp.push_back(0xFF);

	return cmp;
}

size_t Tilemap2D::GetHeight() const
{
	return m_height;
}

size_t Tilemap2D::GetWidth() const
{
	return m_width;
}

Tile Tilemap2D::GetTile(size_t x, size_t y) const
{
	return m_tiles[y * m_width + x];
}

void Tilemap2D::SetTile(const Tile& tile, size_t x, size_t y)
{
	m_tiles[y * m_width + x] = tile;
}

Tile* Tilemap2D::Data()
{
	return m_tiles.data();
}
