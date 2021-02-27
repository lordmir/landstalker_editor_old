#ifndef _HEX_EDITOR_H_
#define _HEX_EDITOR_H_

#include <wx/wx.h>
#include "ObjectEditor.h"

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
#include <vector>
#include <cstdint>

class HexEditor : public ObjectEditor, public wxTextCtrl
{
public:
	HexEditor(wxWindow* parent,
		const std::string& name,
		const wxTreeItemId& treeItemId,
		const std::filesystem::path& filename);

	virtual ~HexEditor() = default;

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
	void FireModifiedEvent() override;

	std::filesystem::path m_filename;
	std::vector<uint8_t> m_buffer;
};

#endif // _HEX_EDITOR_H_