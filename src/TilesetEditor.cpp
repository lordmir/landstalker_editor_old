#include "TilesetEditor.h"

#include <fstream>
#include <algorithm>

#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#include <wx/dcbuffer.h>
#include "LZ77.h"

BEGIN_EVENT_TABLE(TilesetEditor, wxHVScrolledWindow)
	EVT_PAINT(TilesetEditor::OnPaint)
	EVT_SIZE(TilesetEditor::OnSize)
END_EVENT_TABLE()

TilesetEditor::TilesetEditor(wxWindow* parent, const std::string& name, const wxTreeItemId& treeItemId, const std::filesystem::path& filename, bool compressed, int bpp, int tileWidth, int tileHeight)
	: wxHVScrolledWindow(parent),
	ObjectEditor(parent, name, treeItemId),
    m_compressed(compressed),
	m_bpp(bpp),
	m_tileWidth(tileWidth),
	m_tileHeight(tileHeight),
	m_filename(filename)
{
	SetBackgroundStyle(wxBG_STYLE_PAINT);

	std::ifstream ifs(m_filename, std::ios::binary);

	// Stop eating new lines in binary mode
	ifs.unsetf(std::ios::skipws);

	// get its size:
	std::streampos fileSize;

	ifs.seekg(0, std::ios::end);
	fileSize = ifs.tellg();
	ifs.seekg(0, std::ios::beg);
	
	std::vector<uint8_t> input;
	input.reserve(fileSize);
	input.insert(input.begin(), std::istream_iterator<uint8_t>(ifs), std::istream_iterator<uint8_t>());

	if (compressed == true)
	{
		std::vector<uint8_t> buffer(65536);
		size_t dsize = 0, esize = 0;
		dsize = LZ77::Decode(input.data(), input.size(), buffer.data(), esize);
		buffer.resize(dsize);
		convertBuffer(buffer, bpp);
	}
	else
	{
		convertBuffer(input, bpp);
	}

	// Tempory values for palette
	palette[0]  = wxColour(0,   0,   0  );
	palette[1]  = wxColour(0,   0,   128);
	palette[2]  = wxColour(0,   128, 0  );
	palette[3]  = wxColour(0,   128, 128);
	palette[4]  = wxColour(128, 0,   0  );
	palette[5]  = wxColour(128, 0,   128);
	palette[6]  = wxColour(128, 128, 0  );
	palette[7]  = wxColour(192, 192, 192);
	palette[8]  = wxColour(64,  64,  64 );
	palette[9]  = wxColour(0,   0,   255);
	palette[10] = wxColour(0,   255, 0  );
	palette[11] = wxColour(0,   255, 255);
	palette[12] = wxColour(255, 0,   0  );
	palette[13] = wxColour(255, 0,   255);
	palette[14] = wxColour(255, 255, 0  );
	palette[15] = wxColour(255, 255, 255);
}

bool TilesetEditor::Save(bool prompt, bool force)
{
	return true;
}

bool TilesetEditor::IsModified() const
{
	return false;
}

void TilesetEditor::MarkSaved()
{
}

bool TilesetEditor::Close()
{
	return true;
}

wxWindow* TilesetEditor::ToWindow()
{
	return this;
}

ObjectType TilesetEditor::GetObjectType() const
{
	return ObjectType();
}

std::string TilesetEditor::GetObjectDescription() const
{
	return std::string();
}

const std::filesystem::path& TilesetEditor::GetPath() const
{
	return m_filename;
}

void TilesetEditor::SetPixelSize(int n)
{
	m_pixelSize = n;
	wxVarHScrollHelper::RefreshAll();
	wxVarVScrollHelper::RefreshAll();
	Refresh();
}

void TilesetEditor::resizeTileDisplay()
{
	int width;
	int height;
	GetSize(&width, &height);

	m_tilesPerX = std::max(width / m_tileWidth / m_pixelSize, 1);
	m_tilesPerY = (m_totalTiles + m_tilesPerX - 1) / m_tilesPerX;

	SetRowColumnCount(m_tileHeight * m_tilesPerY, m_tileWidth * m_tilesPerX );
	wxVarHScrollHelper::RefreshAll();
	wxVarVScrollHelper::RefreshAll();
	Refresh();
}

void TilesetEditor::convertBuffer(const std::vector<uint8_t>& input, size_t bpp)
{
	// reserve capacity
	m_buffer.clear();
	m_buffer.reserve(input.size() * 8/bpp);

	// read the data:
	for(auto byte : input)
	{
		const size_t mask = (1 << bpp) - 1;
		for (uint8_t px = 1; px <= (8/bpp); ++px)
		{
			m_buffer.push_back((byte >> (8 - px * bpp)) & mask);
		}
	}
	m_totalTiles = (m_buffer.size() + (m_tileWidth * m_tileHeight - 1)) / (m_tileWidth * m_tileHeight);

	resizeTileDisplay();
}

wxCoord TilesetEditor::OnGetRowHeight(size_t row) const
{
	return wxCoord(m_pixelSize);
}

wxCoord TilesetEditor::OnGetColumnWidth(size_t col) const
{
	return wxCoord(m_pixelSize);
}

void TilesetEditor::OnDraw(wxDC& dc)
{
	dc.Clear();

	wxBrush brush = dc.GetBrush();
	wxPen pen = dc.GetPen();

	wxPosition s = GetVisibleBegin();
	wxPosition e = GetVisibleEnd();

	pen.SetStyle(wxPENSTYLE_TRANSPARENT);
	pen.SetColour(wxColour(200, 200, 200));
	dc.SetPen(pen);
	dc.SetBrush(brush);

	for (int y = s.GetRow(); y < e.GetRow(); ++y)
	{
		for (int x = s.GetCol(); x < e.GetCol(); ++x)
		{
			size_t tile = (x / m_tileWidth) + (y / m_tileHeight) * m_tilesPerX;
			size_t pixel = (y % m_tileHeight) * m_tileWidth + (x % m_tileWidth);
			size_t index = (tile * m_tileWidth * m_tileHeight) + pixel;
			if (index < m_buffer.size())
			{
				brush.SetColour(palette[m_buffer[index]]);
				dc.SetBrush(brush);
				dc.DrawRectangle(x * m_pixelSize, y * m_pixelSize, m_pixelSize, m_pixelSize);
			}
		}
	}
}

void TilesetEditor::OnPaint(wxPaintEvent& evt)
{
	wxBufferedPaintDC dc(this);
	this->PrepareDC(dc);
	this->OnDraw(dc);
}

void TilesetEditor::OnSize(wxSizeEvent& evt)
{
	resizeTileDisplay();
	evt.Skip();
}

void TilesetEditor::FireModifiedEvent()
{
	wxCommandEvent evt(OBJECT_EDITOR_MODIFY);
	evt.SetEventObject(this);

	wxPostEvent(this, evt);
}
