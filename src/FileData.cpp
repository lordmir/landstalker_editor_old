#include "FileData.h"
#include "CodeEditor.h"
#include "HexEditor.h"
#include "TilesetEditor.h"
#include <algorithm>

Landstalker::FileData::FileData(wxWindow* parent, wxXmlNode* node, const std::filesystem::path& basedir)
    : m_parent(parent),
	  m_node(node),
	  m_type(StrToObj(m_node->GetName().ToStdString())),
	  m_name(m_node->GetAttribute("name")),
	  m_description(m_node->GetAttribute("description")),
	  m_basedir(basedir)
{
		updateFilenames();
}

void Landstalker::FileData::SetTreeId(wxTreeItemId* id)
{
	m_id = id;
}

ObjectEditor* Landstalker::FileData::MakeEditor() const
{
	ObjectEditor* editor = nullptr;
	switch (m_type)
	{
	case ObjectType::ASSEMBLY_SOURCE:
		if (m_files.at(m_type).size() == 1)
		{
			editor = new CodeEditor(m_parent, m_name, m_id, m_files.at(m_type)[0]);
		}
		break;
	case ObjectType::TILESET_UNCOMPRESSED:
		if (m_files.at(m_type).size() == 1)
		{
			int bpp = m_node->HasAttribute("bpp") ? std::stoi(m_node->GetAttribute("bpp").ToStdString()) : 4;
			int width = m_node->HasAttribute("width") ? std::stoi(m_node->GetAttribute("width").ToStdString()) : 8;
			int height = m_node->HasAttribute("height") ? std::stoi(m_node->GetAttribute("height").ToStdString()) : 8;
			bool compressed = m_node->HasAttribute("compressed") ? (m_node->GetAttribute("compressed").Lower() == "true") : false;
			editor = new TilesetEditor(m_parent, m_name, m_id, m_files.at(m_type)[0], compressed, bpp, width, height);
		}
		break;
	case ObjectType::BINARY:
		if (m_files.at(m_type).size() == 1)
		{
			editor = new HexEditor(m_parent, m_name, m_id, m_files.at(m_type)[0]);
		}
		break;
	default:
		break;
	}
	return editor;
}                       

void Landstalker::FileData::updateFilenames()
{
	if (m_node->HasAttribute("filename"))
	{
		m_files[m_type].push_back(m_basedir / m_node->GetAttribute("filename").ToStdString());
	}
	auto* node = m_node->GetChildren();
	while (node != nullptr)
	{
		if (node->GetName().Lower() == "file")
		{
			auto type = node->HasAttribute("type") ? StrToObj(node->GetAttribute("type").ToStdString()) : m_type;
			auto path = node->GetAttribute("filename");
			m_files[type].push_back(m_basedir / path.ToStdString());
		}
		node = node->GetNext();
	}
}

