#ifndef CODE_EDITOR_H
#define CODE_EDITOR_H

#include "ObjectEditor.h"
#include <wx/wx.h>
#include <wx/stc/stc.h>

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

class CodeEditor : public ObjectEditor, public wxStyledTextCtrl
{
public:
    CodeEditor(wxWindow* parent,
               const std::string& name,
		       const wxTreeItemId& treeItemId,
               const std::filesystem::path& filename);

    virtual ~CodeEditor() = default;

	bool Save(bool prompt = false, bool force = false) override;
	bool IsModified() const override;
	void MarkSaved() override;
	bool Close() override;
	wxWindow* ToWindow() override;

	ObjectType GetObjectType() const override;
	std::string GetObjectDescription() const override;

	const std::filesystem::path& GetPath() const;

	DECLARE_EVENT_TABLE()

private:
	void OnStcModified(wxStyledTextEvent& evt);
	void FireModifiedEvent() override;

	std::filesystem::path m_filename;

};


#endif // CODE_EDITOR_H
