#include "cApp.h"

wxDECLARE_APP(cApp);
wxIMPLEMENT_APP(cApp);

cApp::cApp()
{

}

cApp::~cApp()
{

}

bool cApp::OnInit()
{
	m_mainframe = new cMain();
	SetTopWindow(m_mainframe);
	m_mainframe->Show();

	return true;
}
