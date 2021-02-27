#ifndef _FILE_DATA_H_
#define _FILE_DATA_H_

#include <wx/treebase.h>
#include <string>

namespace Landstalker
{

class FileData : public wxTreeItemData
{
public:
	FileData(const std::string& path, bool isFile) : m_fullPath(path), m_isFile(isFile) {}
	std::string Path() const { return m_fullPath; }
	bool IsFile() const { return m_isFile; }

private:
	std::string m_fullPath;
	bool m_isFile;
};

} // namespace Landstalker

#endif // _FILE_DATA_H_
