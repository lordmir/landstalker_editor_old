#ifndef _C_APP_
#define _C_APP_

#include <wx/wx.h>
#include "cMain.h"

class cApp : public wxApp
{
public:
	cApp();
	~cApp();

private:
	cMain* m_mainframe{ nullptr };

public:
	virtual bool OnInit() override;
};

#endif // _C_APP_
