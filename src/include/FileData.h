#ifndef _FILE_DATA_H_
#define _FILE_DATA_H_

#include <wx/treebase.h>
#include <string>
#include <map>
#include <vector>
#include "filesystem.h"
#include <wx/xml/xml.h>
#include "ObjectEditor.h"

namespace Landstalker
{

class FileData : public wxTreeItemData
{
public:
	FileData(wxWindow* parent, wxXmlNode* node, const std::filesystem::path& basedir);
	void SetTreeId(wxTreeItemId* id);
	bool IsFile() const { return m_type != ObjectType::DIRECTORY; }
	ObjectType Type() const { return m_type; }
	ObjectEditor* MakeEditor() const;
	std::string Name() const { return m_name; }
	std::string Description() const { return m_name; }

private:
	void updateFilenames();

	wxWindow* m_parent;
	wxXmlNode* m_node;
	wxTreeItemId* m_id;
	ObjectType m_type;
	std::string m_name;
	std::string m_description;
	std::filesystem::path m_basedir;
	std::map<ObjectType, std::vector<std::filesystem::path>> m_files;
	std::vector<wxXmlNode*> m_includes;
};

} // namespace Landstalker

#endif // _FILE_DATA_H_
