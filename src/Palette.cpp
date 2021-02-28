#include <Palette.h>
#include <algorithm>
#include <iostream>

static const char* MAGIC_WORD = "TPL";

static const uint32_t BLACK       = 0x000000;
static const uint32_t RED         = 0x0000FF;
static const uint32_t GREEN       = 0x00FF00;
static const uint32_t BLUE        = 0xFF0000;
static const uint32_t YELLOW      = 0x00FFFF;
static const uint32_t MAGENTA     = 0xFF00FF;
static const uint32_t CYAN        = 0xFFFF00;
static const uint32_t WHITE       = 0xFFFFFF;
static const uint32_t DARKGREY    = 0x555555;
static const uint32_t DARKRED     = 0x00007F;
static const uint32_t DARKGREEN   = 0x007F00;
static const uint32_t DARKBLUE    = 0x7F0000;
static const uint32_t DARKYELLOW  = 0x007F7F;
static const uint32_t DARKMAGENTA = 0x7F007F;
static const uint32_t DARKCYAN    = 0x7F7F00;
static const uint32_t GREY        = 0xAAAAAA;

constexpr size_t FileSizeToColours(size_t total_size, Palette::Format fmt)
{
	switch(fmt)
	{
	case Palette::GENESIS:
		return total_size / sizeof(uint16_t);
	case Palette::VARWIDTH_GENESIS:
		return total_size / sizeof(uint16_t) - 1;
	case Palette::TPL:
		return (total_size - strlen(MAGIC_WORD) - 1) / (3 * sizeof(uint8_t));
	case Palette::PNG:
		return total_size / (sizeof(uint8_t) * 3);
	default:
		return 0;
	}
}

constexpr size_t ColoursToFilesize(size_t total_colours, Palette::Format fmt)
{
	switch (fmt)
	{
	case Palette::GENESIS:
		return total_colours * sizeof(uint16_t);
	case Palette::VARWIDTH_GENESIS:
		return (total_colours + 1) * sizeof(uint16_t);
	case Palette::TPL:
		return total_colours * (3 * sizeof(uint8_t)) + strlen(MAGIC_WORD) + 1;
	case Palette::PNG:
		return total_colours * (sizeof(uint8_t) * 3);
	default:
		return 0;
	}
}

constexpr size_t TypeColours(Palette::Type type)
{
	switch (type)
	{
	case Palette::FULL_PALETTE:
		return 16;
	case Palette::ROOM_PALETTE:
		return 13;
	case Palette::SPRITE_LO:
		return 6;
	case Palette::SPRITE_HI:
		return 7;
	case Palette::HUD:
		return 5;
	case Palette::SWORD:
		return 2;
	default:
		return 0;
	}
}

constexpr size_t TypeStart(Palette::Type type)
{
	switch (type)
	{
	case Palette::FULL_PALETTE:
		return 0;
	case Palette::ROOM_PALETTE:
		return 2;
	case Palette::SPRITE_LO:
		return 2;
	case Palette::SPRITE_HI:
		return 8;
	case Palette::HUD:
		return 10;
	case Palette::SWORD:
		return 13;
	default:
		return 0;
	}
}

constexpr size_t TypeEnd(Palette::Type type)
{
	switch (type)
	{
	case Palette::FULL_PALETTE:
		return 16;
	case Palette::ROOM_PALETTE:
		return 15;
	case Palette::SPRITE_LO:
		return 8;
	case Palette::SPRITE_HI:
		return 15;
	case Palette::HUD:
		return 15;
	case Palette::SWORD:
		return 15;
	default:
		return 0;
	}
}

inline uint16_t Word(const uint8_t* src)
{
	return (src[0] << 8) | src[1];
}

Palette::Palette()
{
	m_palettes.push_back({ 0 });
	ClearPalettes();
	SetRGB(2, DARKGREEN);
	SetRGB(3, DARKCYAN);
	SetRGB(4, DARKRED);
	SetRGB(5, DARKMAGENTA);
	SetRGB(6, DARKYELLOW);
	SetRGB(7, WHITE);
	SetRGB(8, DARKGREY);
	SetRGB(9, BLUE);
	SetRGB(10, GREEN);
	SetRGB(11, CYAN);
	SetRGB(12, RED);
	SetRGB(13, MAGENTA);
	SetRGB(14, YELLOW);
}

Palette::Palette(const uint8_t* src, size_t total_size, Format fmt, size_t begin, size_t end, size_t palettes)
{
	Load(src, total_size, fmt, begin, end, palettes);
}

Palette::Palette(const uint8_t* src, size_t total_size, Type type, size_t palettes)
{
	Load(src, total_size, type, palettes);
}

void Palette::Load(const uint8_t* src, size_t size, Format fmt, size_t begin, size_t end, size_t palettes)
{
	size_t total_colours = FileSizeToColours(size, fmt);
	bool wraparound = false;
	if (palettes == 0)
	{
		palettes = (total_colours + (end - begin) - 1) / (end - begin);
	}
	if (fmt == VARWIDTH_GENESIS)
	{
		if (Word(src) > total_colours)
		{
			throw std::runtime_error("Unable to load all requested colours - buffer is too small");
		}
		palettes = std::max(palettes, (total_colours + (end - begin - 1)) / (end - begin));
		total_colours = Word(src);
	}
	else if (fmt == TPL)
	{
		src += strlen(MAGIC_WORD) + 1;
	}
	if (end > 16)
	{
		palettes = (begin + end + 15) / 16;
		end = 16;
		total_colours = end - begin;
		wraparound = true;
	}
	m_palettes.clear();
	m_palettes.assign(palettes, { 0 });
	ClearPalettes();
	size_t c = 0;
	for (size_t p = 0; p < palettes; ++p)
	{
		for (size_t i = begin; i != end; ++i)
		{
			uint8_t i8 = static_cast<uint8_t>(i);
			uint8_t p8 = static_cast<uint8_t>(p);
			if (c++ >= total_colours)
			{
				break;
			}
			if (fmt == GENESIS || fmt == VARWIDTH_GENESIS)
			{
				SetGenColour(i8, Word(src), p8);
				src += 2;
			}
			else if(fmt == TPL || fmt == PNG)
			{
				SetR(i8, *src++, p8);
				SetG(i8, *src++, p8);
				SetB(i8, *src++, p8);
			}
		}
		if (wraparound == true)
		{
			begin = 0;
		}
	}
}

void Palette::Load(const uint8_t* src, size_t size, Type type, size_t palettes)
{
	Load(src, size, GENESIS, TypeStart(type), TypeEnd(type), palettes);
}

void Palette::LoadAlpha(const uint8_t* src, size_t size, Format fmt, size_t begin, size_t end, size_t palettes)
{
	size_t total_colours = FileSizeToColours(size, fmt);
	bool wraparound = false;
	if (end > 16)
	{
		palettes = (begin + end + 15) / 16;
		end = 16;
		total_colours = end - begin;
		wraparound = true;
	}
	size_t c = 0;
	for (size_t p = 0; p < palettes; ++p)
	{
		for (size_t i = begin; i != end; ++i)
		{
			uint8_t i8 = static_cast<uint8_t>(i);
			uint8_t p8 = static_cast<uint8_t>(p);
			if (c++ >= total_colours)
			{
				break;
			}
			SetA(i8, *src++, p8);
		}
		if (wraparound == true)
		{
			begin = 0;
		}
	}
}

std::vector<uint8_t> Palette::GenerateBinary(Format fmt, size_t palette_index, size_t start, size_t end, size_t palettes) const
{
	size_t total_colours = (end - start) * palettes;
	bool wraparound = false;
	size_t pals = palettes;
	if (end > 16)
	{
		pals = (start + end + 15) / 16;
		palettes = (start + end + 15) / 16;
		end = 16;
		total_colours = end - start;
		wraparound = true;
	}
	std::vector<uint8_t> data;
	data.reserve(ColoursToFilesize(total_colours, fmt));
	if (fmt == TPL)
	{
		std::copy(MAGIC_WORD, MAGIC_WORD + strlen(MAGIC_WORD) + 1, std::back_inserter<std::vector<uint8_t>>(data));
	}
	else if (fmt == VARWIDTH_GENESIS)
	{
		data.push_back(static_cast<uint8_t>(total_colours >> 8));
		data.push_back(static_cast<uint8_t>(total_colours & 0xFF));
	}
	size_t c = 0;
	for (size_t p = palette_index; p < (palette_index + pals); ++p)
	{
		for (size_t i = start; i != end; ++i)
		{
			uint8_t i8 = static_cast<uint8_t>(i);
			uint8_t p8 = static_cast<uint8_t>(p);
			if (c++ >= total_colours)
			{
				break;
			}
			if (fmt == GENESIS || fmt == VARWIDTH_GENESIS)
			{
				data.push_back(GetGenColour(i8, p8) >> 8);
				data.push_back(GetGenColour(i8, p8) & 0xFF);
			}
			else if (fmt == TPL || fmt == PNG)
			{
				data.push_back(GetR(i8, p8));
				data.push_back(GetG(i8, p8));
				data.push_back(GetB(i8, p8));
			}
		}
		if (wraparound == true)
		{
			start = 0;
		}
	}
	if (fmt == PNG)
	{
		// PNG has 256 colours
		data.resize(256 * 3);
		// Debug entries
		// 0xFD  RED
		data[253 * 3 + 0] = 0xFF;
		data[253 * 3 + 1] = 0x00;
		data[253 * 3 + 2] = 0x00;
		// 0xFE  BLUE
		data[254 * 3 + 0] = 0x00;
		data[254 * 3 + 1] = 0x00;
		data[254 * 3 + 2] = 0xFF;
		// 0xFF  WHITE
		data[255 * 3 + 0] = 0xFF;
		data[255 * 3 + 1] = 0xFF;
		data[255 * 3 + 2] = 0xFF;

	}
	return data;
}

std::vector<uint8_t> Palette::GenerateBinary(Type type, size_t palettes) const
{
	return GenerateBinary(GENESIS, TypeStart(type), TypeEnd(type), palettes);
}

std::vector<uint8_t> Palette::GenerateAlpha(Format fmt, size_t palette_index, size_t start, size_t end, size_t palettes) const
{
	size_t total_colours = (end - start) * palettes;
	size_t pals = palettes;
	bool wraparound = false;
	if (end > 16)
	{
		pals = (start + end + 15) / 16;
		end = 16;
		total_colours = end - start;
		wraparound = true;
	}
	std::vector<uint8_t> data;
	data.reserve(total_colours);
	size_t c = 0;
	for (size_t p = palette_index; p < (palette_index + pals); ++p)
	{
		for (size_t i = start; i != end; ++i)
		{
			uint8_t i8 = static_cast<uint8_t>(i);
			uint8_t p8 = static_cast<uint8_t>(p);
			if (c++ >= total_colours)
			{
				break;
			}
			data.push_back(GetA(i8, p8));
		}
		if (wraparound == true)
		{
			start = 0;
		}
	}
	if (fmt == PNG)
	{
		// PNG has 256 colours
		data.resize(256);
		// Debug entries
		data[252] = 0xFF;
		data[253] = 0xFF;
		data[254] = 0xFF;
		data[255] = 0xFF;
	}
	return data;
}

uint8_t Palette::GetPalettes() const
{
	return static_cast<uint8_t>(m_palettes.size());
}

uint8_t Palette::GetR(uint8_t index, uint8_t palette) const
{
	return m_palettes[palette][index].r;
}

uint8_t Palette::GetG(uint8_t index, uint8_t palette) const
{
	return m_palettes[palette][index].g;
}

uint8_t Palette::GetB(uint8_t index, uint8_t palette) const
{
	return m_palettes[palette][index].b;
}

uint8_t Palette::GetA(uint8_t index, uint8_t palette) const
{
	return m_palettes[palette][index].a;
}

uint32_t Palette::GetRGBA(uint8_t index, uint8_t palette) const
{
	return m_palettes[palette][index].a << 24 |
	       m_palettes[palette][index].r << 16 |
	       m_palettes[palette][index].g << 8  |
	       m_palettes[palette][index].b;
}

uint32_t Palette::GetRGB(uint8_t index, uint8_t palette) const
{
	return m_palettes[palette][index].r << 16 |
	       m_palettes[palette][index].g << 8  |
	       m_palettes[palette][index].b;
}

uint16_t Palette::GetGenColour(uint8_t index, uint8_t palette) const
{
	return ((m_palettes[palette][index].b / 18) & 0x0E) << 8 |
	       ((m_palettes[palette][index].g / 18) & 0x0E) << 4 |
	       ((m_palettes[palette][index].r / 18) & 0x0E);
}

void Palette::SetR(uint8_t index, uint8_t r, uint8_t palette)
{
	m_palettes[palette][index].r = r;
}

void Palette::SetG(uint8_t index, uint8_t g, uint8_t palette)
{
	m_palettes[palette][index].g = g;
}

void Palette::SetB(uint8_t index, uint8_t b, uint8_t palette)
{
	m_palettes[palette][index].b = b;
}

void Palette::SetA(uint8_t index, uint8_t a, uint8_t palette)
{
	m_palettes[palette][index].a = a;
}

void Palette::SetRGBA(uint8_t index, uint32_t value, uint8_t palette)
{
	m_palettes[palette][index].b = value & 0xFF;
	m_palettes[palette][index].g = (value >> 8) & 0xFF;
	m_palettes[palette][index].r = (value >> 16) & 0xFF;
	m_palettes[palette][index].a = (value >> 24) & 0xFF;
}

void Palette::SetRGB(uint8_t index, uint32_t value, uint8_t palette)
{
	m_palettes[palette][index].b = value & 0xFF;
	m_palettes[palette][index].g = (value >> 8) & 0xFF;
	m_palettes[palette][index].r = (value >> 16) & 0xFF;
}

void Palette::SetGenColour(uint8_t index, uint16_t value, uint8_t palette)
{
	m_palettes[palette][index].r = (value & 0x00E) * 18;
	m_palettes[palette][index].g = ((value >> 4) & 0x00E) * 18;
	m_palettes[palette][index].b = ((value >> 8) & 0x00E) * 18;
}

void Palette::Add(const Palette& pal)
{
	for (size_t p = 0; p < std::min(pal.GetPalettes(), GetPalettes()); ++p)
	{
		for (size_t i = 2; i < 15; ++i)
		{
			uint8_t i8 = static_cast<uint8_t>(i);
			uint8_t p8 = static_cast<uint8_t>(p);
			SetR(i8, std::min(0xFF, pal.GetR(i8, p8) + GetR(i8, p8)),p8);
			SetG(i8, std::min(0xFF, pal.GetG(i8, p8) + GetG(i8, p8)),p8);
			SetB(i8, std::min(0xFF, pal.GetB(i8, p8) + GetB(i8, p8)),p8);
		}
	}
}

void Palette::ClearPalettes()
{
	for (size_t p = 0; p < m_palettes.size(); ++p)
	{
		uint8_t p8 = static_cast<uint8_t>(p);
		SetGenColour(0, 0x0000, p8);
		SetGenColour(1, 0x0CCC, p8);
		SetA(0, 0x00, p8);
		SetA(1, 0xFF, p8);
		for (size_t c = 2; c < 16; ++c)
		{
			uint8_t c8 = static_cast<uint8_t>(c);
			SetGenColour(c8, 0x0000, p8);
			SetA(c8, 0xFF, p8);
		}
	}
}

