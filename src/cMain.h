#ifndef _C_MAIN_H_
#define _C_MAIN_H_

#include <map>

#include "wx/wx.h"
#include "wx/aui/aui.h"
#include "wx/aui/auibook.h"
#include "wx/treectrl.h"
#include <wx/stc/stc.h>

#include "ObjectEditor.h"

#define APP_NAME "Landstalker Editor"
#define APP_DESCR "Editor for Landstalker Disassemblies"

#define APP_MAINT "Lordmir"
#define APP_VENDOR "RCL"
#define APP_COPYRIGHT "(C) 2021 lordmir [Tom Amendt]"
#define APP_LICENCE "GPL"

#define APP_VERSION "0.1.alpha"
#define APP_BUILD __DATE__

#define APP_WEBSITE "http://www.github.com/lordmir"
#define APP_MAIL "tgamendt@gmail.com"
#define APP_MAILTO "mailto://" APP_MAIL


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
	void onMenuAbout(wxCommandEvent& evt);
	void onClose(wxCloseEvent& evt);
	void onObjectEditorModify(wxCommandEvent& evt);

	bool closeTab(std::map<wxTreeItemId, ObjectEditor*>::iterator& it);
	bool closeAll();
	void saveAll();
	void buildRom(bool promptForFilename = false);
	void openProject();
	void loadProject(const std::string& path);
	bool promptSaveAll();
	void showAboutBox();
	void updateMenuState(const std::string& menu, const std::string& submenu, bool enabled = true);
	void updateAllMenuStates();

	wxAuiManager m_mgr;

	wxMenuBar* m_menu;
	wxTextCtrl* m_output;
	wxTreeCtrl* m_fileList;
	wxAuiNotebook* m_mainEditor;

	std::string m_disassemblyPath;
	std::string m_romPath;
	std::map<wxTreeItemId, ObjectEditor*> m_openDocuments;
	bool m_opened{ false };
};

class cAboutDlg : public wxDialog {

public:
	cAboutDlg(wxWindow* parent, long style = 0);
	~cAboutDlg();
}; 

#endif // _C_MAIN_H_