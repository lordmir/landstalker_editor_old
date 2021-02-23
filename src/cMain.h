#ifndef _C_MAIN_H_
#define _C_MAIN_H_

#include <wx/wx.h>
#include <wx/aui/aui.h>
#include <wx/treectrl.h>

class cMain : public wxFrame
{
public:
	cMain();
	~cMain();

	wxDECLARE_EVENT_TABLE();
private:
	void onLoadButtonClick(wxCommandEvent& evt);
	void onBuildButtonClick(wxCommandEvent& evt);
	void onFileActivate(wxTreeEvent& evt);

	wxAuiManager m_mgr;

	wxButton* m_loadButton;
	wxButton* m_buildButton;
	wxTextCtrl* m_output;
	wxTreeCtrl* m_fileList;
	wxTextCtrl* m_mainEditor;

	std::string m_disassemblyPath;
};

#endif // _C_MAIN_H_
