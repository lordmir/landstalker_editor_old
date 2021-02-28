#ifndef TILESET_EDITOR_H
#define TILESET_EDITOR_H

#include <wx/wx.h>
#include "ObjectEditor.h"

#if defined _WIN32 || defined _WIN64
#include <filesystem>
#else
#if GCC_VERSION < 80000
#include <experimental/filesystem>
namespace std
{
	namespace filesystem = experimental::filesystem;
}
#else
#include <filesystem>  
#endif
#endif
#include <vector>
#include <cstdint>
#include <wx/wx.h>
#include <wx/vscroll.h>
#include "ObjectEditor.h"

class TilesetEditor : public ObjectEditor, public wxHVScrolledWindow
{
public:
	TilesetEditor(wxWindow* parent,
		const std::string& name,
		const wxTreeItemId& treeItemId,
		const ObjectType& type,
		int bpp,
		const std::filesystem::path& filename);

	virtual ~TilesetEditor() = default;

	bool Save(bool prompt = false, bool force = false) override;
	bool IsModified() const override;
	void MarkSaved() override;
	bool Close() override;
	wxWindow* ToWindow() override;

	ObjectType GetObjectType() const override;
	std::string GetObjectDescription() const override;

	const std::filesystem::path& GetPath() const;

	void SetPixelSize(int n);

	DECLARE_EVENT_TABLE()

private:
	void resizeTileDisplay();
	void convertBuffer(const std::vector<uint8_t>& input, size_t bpp);

	virtual wxCoord OnGetRowHeight(size_t row) const override;
	virtual wxCoord OnGetColumnWidth(size_t col) const override;

	void OnDraw(wxDC& dc);
	void OnPaint(wxPaintEvent& evt);
	void OnSize(wxSizeEvent& evt);

	void FireModifiedEvent() override;
	ObjectType m_type;
	std::filesystem::path m_filename;
	std::vector<uint8_t> m_buffer;
	wxColour palette[16];

	int m_bpp = 4;
	int m_pixelSize = 4;
	int m_tileWidth = 8;
	int m_tileHeight = 8;
	int m_tilesPerX = 8;
	int m_tilesPerY = 8;
	int m_totalTiles = 64;
}; // TilesetEditor

#endif // TILESET_EDITOR_H
