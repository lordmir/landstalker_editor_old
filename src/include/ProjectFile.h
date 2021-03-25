#ifndef _PROJECT_FILE_H_
#define _PROJECT_FILE_H_

#include "filesystem.h"
#include <wx/xml/xml.h>

class ProjectFile
{
public:
	ProjectFile(const std::filesystem::path& path);
	virtual ~ProjectFile();
	bool Save();
	bool SaveAs(const std::filesystem::path& path);
	wxXmlNode* Root();
private:
	std::filesystem::path m_path;
	wxXmlDocument* m_doc;
};

#endif // _PROJECT_FILE_H_
