#include "cMain.h"

#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <sstream>
#include <fstream>
#if defined _WIN32 || defined _WIN64
#include <filesystem>
#else
#if GCC_VERSION < 80000
#include <experimental/filesystem>
#define filesystem experimental::filesystem
#else
#include <filesystem>
#endif
#endif

#include "wx/artprov.h"
#include "wx/image.h"
#include "wx/imaglist.h"

#include "Assembler.h"
#include "FileData.h"


wxBEGIN_EVENT_TABLE(cMain, wxFrame)
	EVT_BUTTON(10001, cMain::onLoadButtonClick)
	EVT_BUTTON(10002, cMain::onBuildButtonClick)
	EVT_TREE_ITEM_ACTIVATED(10003, cMain::onFileActivate)
wxEND_EVENT_TABLE()

cMain::cMain() : wxFrame(nullptr, wxID_ANY, "Landstalker Disassembly Editor",
                         wxDefaultPosition, wxSize(800, 600), wxDEFAULT_FRAME_STYLE)
{

	m_mgr.SetManagedWindow(this);

	m_fileList = new wxTreeCtrl(this, 10003);
	m_fileList->SetWindowStyle(wxTR_HAS_BUTTONS | wxTR_NO_LINES | wxTR_HIDE_ROOT | wxTR_SINGLE);
	wxImageList* im = new wxImageList(16, 16, true);
	im->Add(wxArtProvider::GetIcon(wxART_NORMAL_FILE, wxART_LIST, { 16,16 }));
	im->Add(wxArtProvider::GetIcon(wxART_FOLDER, wxART_LIST, { 16,16 }));
	m_fileList->AssignImageList(im);

	m_loadButton = new wxButton(this, 10001, "Load");
	m_buildButton = new wxButton(this, 10002, "Build");
	m_buildButton->Enable(false);
	m_output = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(200, 150),
		wxNO_BORDER | wxTE_MULTILINE);
	m_output->SetEditable(false);
	m_output->SetFont(wxFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
	m_mainEditor = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(200, 150),
		wxNO_BORDER | wxTE_MULTILINE);
	m_mainEditor->SetEditable(false);
	m_mainEditor->SetFont(wxFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));

	m_mgr.AddPane(m_fileList, wxAuiPaneInfo().Direction(wxAUI_DOCK_LEFT).Layer(0).Row(0).Position(0).MinSize(100, 100).BestSize(200,100).CaptionVisible(true).MaximizeButton(false).CloseButton(false).MinimizeButton(false).PinButton(false).Caption("Files"));
	m_mgr.AddPane(m_loadButton, wxRIGHT, "Load");
	m_mgr.AddPane(m_buildButton, wxRIGHT, "Build");
	m_mgr.AddPane(m_output, wxAuiPaneInfo().Direction(wxAUI_DOCK_BOTTOM).Layer(0).Row(0).Position(0).MinSize(100, 100).BestSize(100, 150).CaptionVisible(true).MaximizeButton(false).CloseButton(false).MinimizeButton(false).PinButton(false).Caption("Output"));
	m_mgr.AddPane(m_mainEditor, wxAuiPaneInfo().Direction(wxAUI_DOCK_CENTER).Layer(0).Row(0).Position(0).MinSize(100, 100).CaptionVisible(false).MaximizeButton(false).CloseButton(false).MinimizeButton(false).PinButton(false));

	m_mgr.Update();
}

cMain::~cMain()
{
	m_mgr.UnInit();
}

void cMain::onLoadButtonClick(wxCommandEvent& evt)
{
	wxDirDialog dir(this, "Select the disassembly directory", "", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
	if (dir.ShowModal() == wxID_OK)
	{
		m_disassemblyPath = dir.GetPath();
		m_buildButton->Enable();

		m_fileList->DeleteAllItems();
		
		auto parent = m_fileList->AddRoot(".");
		auto last = parent;
		int depth = 0;
		for (auto it = std::filesystem::recursive_directory_iterator(m_disassemblyPath);
			 it != std::filesystem::recursive_directory_iterator(); ++it)
		{
			if (depth < it.depth())
			{
				// last item was a directory
				parent = last;
			}
			else
			{
				while (depth > it.depth())
				{
					// finished for this directory
					parent = m_fileList->GetItemParent(parent);
					depth--;
				}
			}
			auto* d = new Landstalker::FileData(it->path().generic_string(), std::filesystem::is_regular_file(*it));
			last = m_fileList->AppendItem(parent,
				                          it->path().filename().c_str(),
				                          std::filesystem::is_directory(*it) ? 1 : 0,
				                          std::filesystem::is_directory(*it) ? 1 : 0,
				                          d);
			depth = it.depth();
		}
	}
	evt.Skip();
}

void cMain::onBuildButtonClick(wxCommandEvent& evt)
{
	wxFileDialog fileDlg(this, "Save built ROM As", m_disassemblyPath, "landstalker.bin", "Sega Genesis ROM (*.bin)|*.bin", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if (fileDlg.ShowModal() == wxID_OK)
	{
		m_buildButton->Enable(false);
		m_loadButton->Enable(false);
		Landstalker::Assembler::Assemble(m_disassemblyPath, "landstalker.asm", fileDlg.GetPath().ToStdString(), "/p /o ae-,e+,w+,c+,op+,os+,ow+,oz+,l_ /e EXPANDED=0", m_output);
		m_loadButton->Enable(true);
		m_buildButton->Enable(true);
	}
	evt.Skip();
}

void cMain::onFileActivate(wxTreeEvent& evt)
{
	const auto& d = *static_cast<Landstalker::FileData*>(m_fileList->GetItemData(evt.GetItem()));
	if (d.IsFile() == true)
	{
		std::ifstream file(d.Path());
		std::string str((std::istreambuf_iterator<char>(file)),
		                 std::istreambuf_iterator<char>());
		m_mainEditor->ChangeValue(str);
	}
	evt.Skip();
}
