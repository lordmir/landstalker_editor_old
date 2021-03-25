#include "ProjectFile.h"
#include <stdexcept>

ProjectFile::ProjectFile(const std::filesystem::path& path)
	: m_path(path),
	  m_doc(new wxXmlDocument())
{
	if (m_doc->Load(m_path.generic_string()) == false)
	{
		throw std::runtime_error("Unable to load project XML");
	}
}

ProjectFile::~ProjectFile()
{
	if (m_doc != nullptr)
	{
		delete m_doc;
	}
}

bool ProjectFile::Save()
{
	return m_doc->Save(m_path.generic_string());
}

bool ProjectFile::SaveAs(const std::filesystem::path& path)
{
	return m_doc->Save(path.generic_string());
}

wxXmlNode* ProjectFile::Root()
{
	return m_doc->GetRoot();
}
