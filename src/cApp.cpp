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
	if(!wxApp::OnInit())
	{
		return false;
	}

	wxInitAllImageHandlers();

	m_mainframe = new cMain();
	SetTopWindow(m_mainframe);
	m_mainframe->Show();
	if(!m_filename.empty())
	{
		m_mainframe->loadProject(m_filename);
	}

	return true;
}

int cApp::OnExit()
{
	return wxApp::OnExit();
}

void cApp::OnInitCmdLine(wxCmdLineParser& parser)
{
	wxApp::OnInitCmdLine(parser);

	parser.AddParam("projectfile", wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL);
	parser.SetSwitchChars("-");
}

bool cApp::OnCmdLineParsed(wxCmdLineParser& parser)
{
	int paramcount = parser.GetParamCount();

	if(paramcount == 1)
	{
		m_filename = parser.GetParam(0);
	}
	else if(paramcount > 1)
	{
		return false;
	}

	return wxApp::OnCmdLineParsed(parser);
}



