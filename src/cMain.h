#ifndef _C_MAIN_H_
#define _C_MAIN_H_

#include <wx/wx.h>

class cMain : public wxFrame
{
public:
	cMain();
	~cMain();

	wxDECLARE_EVENT_TABLE();
private:
	void onLoadButtonClick(wxCommandEvent& evt);
	void onBuildButtonClick(wxCommandEvent& evt);

	wxButton* m_loadButton;
	wxButton* m_buildButton;

	std::string m_disassemblyPath;
};

#endif // _C_MAIN_H_
