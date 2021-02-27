#ifndef _C_MAIN_H_
#define _C_MAIN_H_

#include <map>

#include "wx/wx.h"
#include "wx/aui/aui.h"
#include "wx/aui/auibook.h"
#include "wx/treectrl.h"
#include <wx/stc/stc.h>

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
	void onAuiNotebookPageClose(wxAuiNotebookEvent& evt);
	void OnStcModified(wxStyledTextEvent& evt);
	void onMenuOpen(wxCommandEvent& evt);
	void onMenuSave(wxCommandEvent& evt);
	void onMenuSaveAs(wxCommandEvent& evt);
	void onMenuSaveAll(wxCommandEvent& evt);
	void onMenuBuildRom(wxCommandEvent& evt);
	void onMenuBuildRomAs(wxCommandEvent& evt);
	void onMenuClose(wxCommandEvent& evt);
	void onMenuCloseTab(wxCommandEvent& evt);
	void onMenuCloseAll(wxCommandEvent& evt);
	void onMenuCloseAllButThis(wxCommandEvent& evt);
	void onClose(wxCloseEvent& evt);

	bool closeTab(std::map<wxTreeItemId, wxWindow*>::iterator& it);

	wxAuiManager m_mgr;

	wxMenuBar* m_menu;
	wxTextCtrl* m_output;
	wxTreeCtrl* m_fileList;
	wxAuiNotebook* m_mainEditor;

	std::string m_disassemblyPath;
	std::string m_romPath;
	std::map<wxTreeItemId, wxWindow*> m_openDocuments;
};

#endif // _C_MAIN_H_
