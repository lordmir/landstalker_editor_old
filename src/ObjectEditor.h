#ifndef _I_OBJECT_EDITOR_H_
#define _I_OBJECT_EDITOR_H_

#include <string>
#include <vector>
#include <wx/wx.h>
#include <wx/treectrl.h>

wxDECLARE_EVENT(OBJECT_EDITOR_MODIFY, wxCommandEvent);

enum class ObjectType
{
	UNKNOWN,
	ASSEMBLY_SOURCE,
	BINARY
};

class ObjectEditor
{
public:
	ObjectEditor(wxWindow* parent, const std::string& name, const wxTreeItemId& treeId)
		: m_parent(parent), m_name(name), m_treeId(treeId) {}
	virtual ~ObjectEditor() = default;
	
	virtual bool Save(bool prompt = false, bool force = false) = 0;
	virtual bool IsModified() const = 0;
	virtual void MarkSaved() = 0;
	virtual bool Close() = 0;
	virtual wxWindow* ToWindow() = 0;
	
	virtual ObjectType GetObjectType() const = 0;
	virtual std::string GetObjectDescription() const = 0;
	std::string GetObjectName() const { return m_name; }
	std::string GetDisplayTitle() const { return (m_name + (IsModified() ? "*" : "")); }
	wxTreeItemId GetTreeItemId() const { return m_treeId; }
protected:
	wxWindow* GetParent() { return m_parent; }
	virtual void FireModifiedEvent() = 0;

private:
	wxWindow* m_parent;
	std::string m_name;
	wxTreeItemId m_treeId;
}; // ObjectEditor

#endif // _I_OBJECT_EDITOR_H_
