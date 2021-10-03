#ifndef _C_APP_
#define _C_APP_

#include <wx/wx.h>
#include <wx/cmdline.h>
#include <string>
#include "cMain.h"

class cApp : public wxApp
{
public:
	cApp();
	~cApp();

private:
	cMain* m_mainframe{ nullptr };

	std::string m_filename;

public:
	virtual bool OnInit() override;
	virtual int  OnExit() override;
	virtual void OnInitCmdLine(wxCmdLineParser& parser) override;
	virtual bool OnCmdLineParsed(wxCmdLineParser& parser) override;
};

#endif // _C_APP_
