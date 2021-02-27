#ifndef CODE_EDITOR_H
#define CODE_EDITOR_H

#include "ObjectEditor.h"
#include <wx/wx.h>
#include <wx/stc/stc.h>
#include <filesystem>

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