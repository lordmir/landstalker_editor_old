#include "cMain.h"

#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <sstream>

#include "Assembler.h"


wxBEGIN_EVENT_TABLE(cMain, wxFrame)
	EVT_BUTTON(10001, cMain::onLoadButtonClick)
	EVT_BUTTON(10002, cMain::onBuildButtonClick)
wxEND_EVENT_TABLE()

cMain::cMain() : wxFrame(nullptr, wxID_ANY, "Landstalker Disassembly Editor")
{

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

	sizer->Add(m_loadButton = new wxButton(this, 10001, "Load"));
	sizer->Add(m_buildButton = new wxButton(this, 10002, "Build"));
	SetSizerAndFit(sizer);
	m_buildButton->Enable(false);
}

cMain::~cMain()
{
}

void cMain::onLoadButtonClick(wxCommandEvent& evt)
{
	wxDirDialog dir(this, "Select the disassembly directory", "", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
	if (dir.ShowModal() == wxID_OK)
	{
		m_disassemblyPath = dir.GetPath();
		m_buildButton->Enable();
	}
	evt.Skip();
}

void cMain::onBuildButtonClick(wxCommandEvent& evt)
{
	wxFileDialog fileDlg(this, "Save built ROM As", m_disassemblyPath, "landstalker.bin", "Sega Genesis ROM (*.bin)|*.bin", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if (fileDlg.ShowModal() == wxID_OK)
	{
		std::string output;
		auto ret = Landstalker::Assembler::Assemble(m_disassemblyPath, "landstalker.asm", fileDlg.GetPath().ToStdString(), "/p /o ae-,e+,w+,c+,op+,os+,ow+,oz+,l_ /e EXPANDED=0", &output);
		std::ostringstream msg;
		msg << "Assembler returned " << ret << ":\n" << output;
		wxMessageBox(msg.str());
	}
	evt.Skip();
}


