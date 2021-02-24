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
	void OnStcStyleNeeded(wxStyledTextEvent& evt);

	wxAuiManager m_mgr;

	wxButton* m_loadButton;
	wxButton* m_buildButton;
	wxTextCtrl* m_output;
	wxTreeCtrl* m_fileList;
	wxAuiNotebook* m_mainEditor;

	std::string m_disassemblyPath;
	std::map<wxTreeItemId, wxWindow*> m_openDocuments;
};

#endif // _C_MAIN_H_
