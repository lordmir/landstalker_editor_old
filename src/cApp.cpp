#include "cApp.h"

wxIMPLEMENT_APP(cApp);

cApp::cApp()
{

}

cApp::~cApp()
{

}

bool cApp::onInit()
{
	m_mainframe = new cMain();
	m_mainframe->Show();

	return true;
}