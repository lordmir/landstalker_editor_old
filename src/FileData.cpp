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
		else if (extension == ".BIN")
		{
			m_type = ObjectType::TILESET_UNCOMPRESSED;
		}
		else if (extension == ".LZ77")
		{
			m_type = ObjectType::TILESET_LZ77;
		}
		else if (extension == ".1BPP")
		{
			m_type = ObjectType::TILESET_1BPP;
		}
		else if (extension == ".2BPP")
		{
			m_type = ObjectType::TILESET_2BPP;
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
