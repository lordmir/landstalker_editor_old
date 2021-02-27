#include "FileData.h"
#include <algorithm>

Landstalker::FileData::FileData(const std::string& path, bool isFile)
    : m_path(path), m_isFile(isFile)
{
	if (m_isFile == true)
	{
		std::string extension = m_path.extension().generic_string();
		std::transform(extension.begin(), extension.end(), extension.begin(), [](char c) {return toupper(c);});
		if (extension == ".ASM" || extension == ".INC")
		{
			m_type = ObjectType::ASSEMBLY_SOURCE;
		}
		else
		{
			m_type = ObjectType::BINARY;
		}
	}
	else
	{
		m_type = ObjectType::DIRECTORY;
	}
}
