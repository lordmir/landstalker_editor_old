#include "HexEditor.h"

#include <fstream>
#include <sstream>
#include <iomanip>

BEGIN_EVENT_TABLE(HexEditor, wxTextCtrl)
END_EVENT_TABLE()

HexEditor::HexEditor(wxWindow* parent, const std::string& name, const wxTreeItemId& treeItemId, const std::filesystem::path& filename)
	: wxTextCtrl(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxTE_MULTILINE),
	  ObjectEditor(parent, name, treeItemId),
	  m_filename(filename)
{
	Hide();
	SetEditable(false);
	SetFont(wxFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));

	std::ifstream ifs(m_filename, std::ios::binary);

	if(ifs.fail())
	{
		wxMessageBox("Failed to open " + m_filename.string());
	}
	else
	{
		// Stop eating new lines in binary mode
		ifs.unsetf(std::ios::skipws);

		// get its size:
		std::streampos fileSize;

		ifs.seekg(0, std::ios::end);
		fileSize = ifs.tellg();
		ifs.seekg(0, std::ios::beg);

		// reserve capacity
		m_buffer.clear();
		m_buffer.reserve(fileSize);

		// read the data:
		m_buffer.insert(m_buffer.begin(),
				    std::istream_iterator<uint8_t>(ifs),
				    std::istream_iterator<uint8_t>());

		std::ostringstream hex;
		unsigned int counter = 0;
		hex << std::endl << std::uppercase << std::hex << std::setw(6) << std::setfill('0') << counter << " : ";
		for (const auto& byte : m_buffer)
		{
			hex << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned>(byte);
			counter++;
			if (counter % 16 == 0)
			{
				hex << "\n" << std::uppercase << std::hex << std::setw(6) << std::setfill('0') << counter << " : ";
			}
			else if (counter % 8 == 0)
			{
				hex << "   ";
			}
			else
			{
				hex << " ";
			}
		}
		ChangeValue(hex.str());
	}
	Show();
}

bool HexEditor::Save(bool prompt, bool force)
{
	return true;
}

bool HexEditor::IsModified() const
{
	return false;
}

void HexEditor::MarkSaved()
{
}

bool HexEditor::Close()
{
	return true;
}

wxWindow* HexEditor::ToWindow()
{
	return this;
}

ObjectType HexEditor::GetObjectType() const
{
	return ObjectType::BINARY;
}

std::string HexEditor::GetObjectDescription() const
{
	return "Binary File";
}

const std::filesystem::path& HexEditor::GetPath() const
{
	return m_filename;
}

void HexEditor::FireModifiedEvent()
{
	wxCommandEvent evt(OBJECT_EDITOR_MODIFY);
	evt.SetEventObject(this);

	wxPostEvent(this, evt);
}
