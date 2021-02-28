#ifndef PALETTE_H
#define PALETTE_H

#include <array>
#include <cstdint>
#include <vector>

class Palette
{
public:

    enum Format
    {
        GENESIS,
        VARWIDTH_GENESIS,
        PNG,
        TPL
    };

    enum Type
    {
        FULL_PALETTE,
        ROOM_PALETTE,
        SPRITE_LO,
        SPRITE_HI,
        HUD,
        SWORD
    };

    Palette();
    Palette(const uint8_t* src, size_t total_size, Format fmt = GENESIS, size_t begin = 0, size_t end = 16, size_t palettes = 1);
    Palette(const uint8_t* src, size_t total_size, Type type = FULL_PALETTE, size_t palettes = 1);

    void Load(const uint8_t* src, size_t size = 16, Format fmt = GENESIS, size_t begin = 0, size_t end = 16, size_t palettes = 1);
    void Load(const uint8_t* src, size_t size = 16, Type type = FULL_PALETTE, size_t palettes = 1);
    void LoadAlpha(const uint8_t* src, size_t size = 16, Format fmt = GENESIS, size_t begin = 0, size_t end = 16, size_t palettes = 1);
    std::vector<uint8_t> GenerateBinary(Type type = FULL_PALETTE, size_t palettes = 1) const;
    std::vector<uint8_t> GenerateBinary(Format fmt = GENESIS, size_t palette_index = 0, size_t start = 0, size_t end = 16, size_t palettes = 1) const;
    std::vector<uint8_t> GenerateAlpha(Format fmt = GENESIS, size_t palette_index = 0, size_t start = 0, size_t end = 16,  size_t palettes = 1) const;

    uint8_t GetPalettes() const;
    uint8_t GetR(uint8_t index, uint8_t palette = 0) const;
    uint8_t GetG(uint8_t index, uint8_t palette = 0) const;
    uint8_t GetB(uint8_t index, uint8_t palette = 0) const;
    uint8_t GetA(uint8_t index, uint8_t palette = 0) const;
    uint32_t GetRGBA(uint8_t index, uint8_t palette = 0) const;
    uint32_t GetRGB(uint8_t index, uint8_t palette = 0) const;
    uint16_t GetGenColour(uint8_t index, uint8_t palette = 0) const;
    void SetR(uint8_t index, uint8_t r, uint8_t palette = 0);
    void SetG(uint8_t index, uint8_t g, uint8_t palette = 0);
    void SetB(uint8_t index, uint8_t b, uint8_t palette = 0);
    void SetA(uint8_t index, uint8_t a, uint8_t palette = 0);
    void SetRGBA(uint8_t index, uint32_t value, uint8_t palette = 0);
    void SetRGB(uint8_t index, uint32_t value, uint8_t palette = 0);
    void SetGenColour(uint8_t index, uint16_t value, uint8_t palette = 0);
    void Add(const Palette& pal);
    void ClearPalettes();
private:
    struct PaletteEntry
    {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
    };

    std::vector<std::array<PaletteEntry, 16>> m_palettes;
};

#endif // PALETTE_H
