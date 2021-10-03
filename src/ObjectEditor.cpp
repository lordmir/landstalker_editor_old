#include "ObjectEditor.h"
#include <algorithm>
#include <wx/wx.h>

wxDEFINE_EVENT(OBJECT_EDITOR_MODIFY, wxCommandEvent);

ObjectType StrToObj(const std::string& name)
{
	ObjectType ret = ObjectType::UNKNOWN;
	std::string type = name;
	std::transform(type.begin(), type.end(), type.begin(), [](char c) {return tolower(c);});
	if (name == "dir")
	{
		ret = ObjectType::DIRECTORY;
	}
	else if (name == "asm")
	{
		ret = ObjectType::ASSEMBLY_SOURCE;
	}
	else if (name == "binary")
	{
		ret = ObjectType::BINARY;
	}
	else if (name == "tileset")
	{
		ret = ObjectType::TILESET_UNCOMPRESSED;
	}
	else
	{
		ret = ObjectType::UNKNOWN;
	}
	return ret;
}
