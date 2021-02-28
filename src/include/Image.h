#ifndef _IMAGE_H_
#define _IMAGE_H_

#include <string>
#include <Tilemap2DRLE.h>
#include <Palette.h>
#include <Tileset.h>

class Image
{
	Image(size_t x, size_t y);
	Image(const std::string& filename, size_t tile_width, size_t tile_height);
	Image(const Tileset& tileset, const Palette& palette, const Tilemap2D& tilemap);

	void ExportToPng(const std::string& filename);
	void ImportFromPng(const std::string& filename);
	void RemoveDuplicateTiles();
	Tileset GetTileset() const;
	Palette GetPalette() const;
	Tilemap2D GetTilemap() const;
	void SetPalette(const Palette& palette);

};

#endif _IMAGE_H_