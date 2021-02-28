#ifndef _FILE_DATA_H_
#define _FILE_DATA_H_

#include <wx/treebase.h>
#include <string>
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
#include "ObjectEditor.h"

namespace Landstalker
{

class FileData : public wxTreeItemData
{
public:
	FileData(const std::string& path, bool isFile);
	std::string Path() const { return m_path.generic_string(); }
	bool IsFile() const { return m_isFile; }
	ObjectType Type() const { return m_type; };

private:
	ObjectType m_type;
	std::filesystem::path m_path;
	bool m_isFile;
};

} // namespace Landstalker

#endif // _FILE_DATA_H_
