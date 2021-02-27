#include "cMain.h"

#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <sstream>
#include <fstream>
#include <algorithm>
#if defined _WIN32 || defined _WIN64
#include <filesystem>
#else
#if GCC_VERSION < 80000
#include <experimental/filesystem>
namespace std
{
namespace filesystem = experimental::filesystem;
}
#else
#include <filesystem>
#endif
#endif

#include <wx/artprov.h>
#include <wx/image.h>
#include <wx/imaglist.h>
#include <wx/hyperlink.h>

#include "Assembler.h"
#include "FileData.h"
#include "CodeEditor.h"
#include "HexEditor.h"

#include "img/chest.xpm"
#include "img/unk.xpm"
#include "img/dir.xpm"
#include "img/asm.xpm"
#include "img/bin.xpm"


wxBEGIN_EVENT_TABLE(cMain, wxFrame)
	EVT_MENU(wxID_OPEN, cMain::onMenuOpen)
	EVT_MENU(wxID_SAVE, cMain::onMenuSave)
	EVT_MENU(wxID_SAVEAS, cMain::onMenuSaveAs)
	EVT_MENU(10001, cMain::onMenuSaveAll)
	EVT_MENU(10002, cMain::onMenuBuildRom)
	EVT_MENU(10003, cMain::onMenuBuildRomAs)
	EVT_MENU(wxID_CLOSE, cMain::onMenuClose)
	EVT_MENU(10004, cMain::onMenuCloseTab)
	EVT_MENU(10005, cMain::onMenuCloseAll)
	EVT_MENU(10006, cMain::onMenuCloseAllButThis)
	EVT_MENU(wxID_ABOUT, cMain::onMenuAbout)
	EVT_TREE_ITEM_ACTIVATED(10003, cMain::onFileActivate)
	EVT_AUINOTEBOOK_PAGE_CLOSE(10004, cMain::onAuiNotebookPageClose)
	EVT_COMMAND(wxID_ANY, OBJECT_EDITOR_MODIFY, cMain::onObjectEditorModify)
	EVT_CLOSE(cMain::onClose)
wxEND_EVENT_TABLE()

cMain::cMain() : wxFrame(nullptr, wxID_ANY, "Landstalker Disassembly Editor",
                         wxDefaultPosition, wxSize(800, 600), wxDEFAULT_FRAME_STYLE)
{
	SetIcon(chest_xpm);
	populateMenus();
	initAuiMgr();
	initImageList();
}

cMain::~cMain()
{
	m_mgr.UnInit();
}

void cMain::onLoadButtonClick(wxCommandEvent& evt)
{
	evt.Skip();
}

void cMain::onBuildButtonClick(wxCommandEvent& evt)
{
	evt.Skip();
}

void cMain::onFileActivate(wxTreeEvent& evt)
{
	auto doc = m_openDocuments.find(evt.GetItem());
	if (doc == m_openDocuments.end())
	{
		const auto& d = *static_cast<Landstalker::FileData*>(m_fileList->GetItemData(evt.GetItem()));
		if (d.IsFile() == true)
		{
			ObjectEditor* editor = nullptr;
			switch(d.Type())
			{
			case ObjectType::ASSEMBLY_SOURCE:
				editor = new CodeEditor(this, m_fileList->GetItemText(evt.GetItem()).ToStdString(), evt.GetItem(), d.Path());
				break;
			case ObjectType::BINARY:
			case ObjectType::UNKNOWN:
				editor = new HexEditor(this, m_fileList->GetItemText(evt.GetItem()).ToStdString(), evt.GetItem(), d.Path());
				break;
			case ObjectType::DIRECTORY:
			default:
				break;
			}
			if (editor != nullptr)
			{
				m_mainEditor->InsertPage(m_mainEditor->GetSelection() + 1, editor->ToWindow(), m_fileList->GetItemText(evt.GetItem()), true, m_fileList->GetItemImage(evt.GetItem()));
				m_openDocuments.insert({ evt.GetItem(), editor });
			}
		}
	}
	else
	{
		auto pageId = m_mainEditor->GetPageIndex(doc->second->ToWindow());
		if (pageId == wxNOT_FOUND)
		{
			m_openDocuments.erase(evt.GetItem());
		}
		else
		{
			m_mainEditor->ChangeSelection(pageId);
		}
	}
	updateAllMenuStates();
	evt.Skip();
}

void cMain::onAuiNotebookPageClose(wxAuiNotebookEvent& evt)
{
	auto* editor = dynamic_cast<ObjectEditor*>(m_mainEditor->GetPage(evt.GetSelection()));
	auto it = m_openDocuments.find(editor->GetTreeItemId());
	if (it != m_openDocuments.end())
	{
		closeTab(it);
	}
	// Always veto, as we close tabs programatically
	evt.Veto();
	evt.Skip();
}

void cMain::onMenuOpen(wxCommandEvent& evt)
{
	openProject();
	evt.Skip();
}

void cMain::onMenuSave(wxCommandEvent& evt)
{
	if (m_openDocuments.empty() == false)
	{
		dynamic_cast<ObjectEditor*>(m_mainEditor->GetPage(m_mainEditor->GetSelection()))->Save(false);
	}
	evt.Skip();
}

void cMain::onMenuSaveAs(wxCommandEvent& evt)
{
	if (m_openDocuments.empty() == false)
	{
		dynamic_cast<ObjectEditor*>(m_mainEditor->GetPage(m_mainEditor->GetSelection()))->Save(true);
	}
	evt.Skip();
}

void cMain::onMenuSaveAll(wxCommandEvent& evt)
{
	saveAll();
	evt.Skip();
}

void cMain::onMenuBuildRom(wxCommandEvent& evt)
{
	buildRom(false);
	evt.Skip();
}

void cMain::onMenuBuildRomAs(wxCommandEvent& evt)
{
	buildRom(true);
	evt.Skip();
}

void cMain::onMenuClose(wxCommandEvent& evt)
{
	Close();
	evt.Skip();
}

void cMain::onMenuCloseTab(wxCommandEvent& evt)
{
	auto* editor = dynamic_cast<ObjectEditor*>(m_mainEditor->GetPage(m_mainEditor->GetSelection()));
	auto it = m_openDocuments.find(editor->GetTreeItemId());
	if (it != m_openDocuments.end())
	{
		closeTab(it);
	}
	evt.Skip();
}

void cMain::onMenuCloseAll(wxCommandEvent& evt)
{
	closeAll();
}

void cMain::onMenuCloseAllButThis(wxCommandEvent& evt)
{
	auto it = m_openDocuments.begin();
	auto* editor = dynamic_cast<ObjectEditor*>(m_mainEditor->GetPage(m_mainEditor->GetSelection()));
	auto id = editor->GetTreeItemId();
	while (it != m_openDocuments.end())
	{
		if (it->first != id)
		{
			if (closeTab(it) == false)
			{
				break;
			}
		}
		else
		{
			it++;
		}
	}
	evt.Skip();
}

void cMain::onMenuAbout(wxCommandEvent& evt)
{
	showAboutBox();
}

void cMain::onClose(wxCloseEvent& evt)
{
	auto it = m_openDocuments.begin();
	while (it != m_openDocuments.end())
	{
		if (closeTab(it) == false)
		{
			if (evt.CanVeto() == true)
			{
				evt.Veto();
				return;
			}
			break;
		}
	}
	evt.Skip();
}

void cMain::onObjectEditorModify(wxCommandEvent& evt)
{
	auto* tab = dynamic_cast<ObjectEditor*>(evt.GetEventObject());
	auto id = m_mainEditor->GetPageIndex(static_cast<wxWindow*>(evt.GetEventObject()));
	if (id != wxNOT_FOUND)
	{
		m_mainEditor->SetPageText(id, tab->GetDisplayTitle());
	}
	evt.Skip();
}

void cMain::populateMenus()
{
	m_menu = new wxMenuBar();
	wxMenu* fileMenu = new wxMenu();
	fileMenu->Append(wxID_OPEN, "&Open Disassembly...\tCtrl+O");
	fileMenu->Append(wxID_SAVE, "&Save\tCtrl+S");
	fileMenu->Append(wxID_SAVEAS, "Save &As...\tCtrl+Shift+S");
	fileMenu->Append(10001, "Save A&ll\tCtrl+Alt+S");
	fileMenu->Append(10002, "&Build ROM\tCtrl+B");
	fileMenu->Append(10003, "B&uild ROM As...\tCtrl+Shift+B");
	fileMenu->Append(wxID_CLOSE, "&Close\tAlt+F4");
	m_menu->Append(fileMenu, "&File");
	wxMenu* windowMenu = new wxMenu();
	windowMenu->Append(10004, "&Close\tCtrl+W");
	windowMenu->Append(10005, "Close &All\tCtrl+Shift+W");
	windowMenu->Append(10006, "Close All &But This\tCtrl+Alt+W");
	m_menu->Append(windowMenu, "&Window");
	wxMenu* helpMenu = new wxMenu();
	helpMenu->Append(wxID_ABOUT, "&About...");
	m_menu->Append(helpMenu, "&Help");
	SetMenuBar(m_menu);
	updateAllMenuStates();
}

void cMain::initImageList()
{
	wxImageList* im = new wxImageList(16, 16, true);
	im->Add(wxBitmap(unk_xpm));
	im->Add(wxBitmap(dir_xpm));
	im->Add(wxBitmap(asm_xpm));
	im->Add(wxBitmap(bin_xpm));
	m_fileList->SetImageList(im);
	m_mainEditor->AssignImageList(im);
}

void cMain::initAuiMgr()
{
	m_mgr.SetManagedWindow(this);

	m_fileList = new wxTreeCtrl(this, 10003);
	m_fileList->SetWindowStyle(wxTR_HAS_BUTTONS | wxTR_NO_LINES | wxTR_HIDE_ROOT | wxTR_SINGLE);

	m_output = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(200, 150),
		wxNO_BORDER | wxTE_MULTILINE);
	m_output->SetEditable(false);
	m_output->SetFont(wxFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));

	m_mainEditor = new wxAuiNotebook(this, 10004);

	m_mgr.AddPane(m_fileList, wxAuiPaneInfo().Direction(wxAUI_DOCK_LEFT).Layer(0).Row(0).Position(0).MinSize(100, 100).BestSize(200, 100).CaptionVisible(true).MaximizeButton(false).CloseButton(false).MinimizeButton(false).PinButton(false).Caption("Files"));
	m_mgr.AddPane(m_output, wxAuiPaneInfo().Direction(wxAUI_DOCK_BOTTOM).Layer(0).Row(0).Position(0).MinSize(100, 100).BestSize(100, 150).CaptionVisible(true).MaximizeButton(false).CloseButton(false).MinimizeButton(false).PinButton(false).Caption("Output"));
	m_mgr.AddPane(m_mainEditor, wxAuiPaneInfo().Direction(wxAUI_DOCK_CENTER).Layer(0).Row(0).Position(0).MinSize(100, 100).CaptionVisible(false).MaximizeButton(false).CloseButton(false).MinimizeButton(false).PinButton(false));

	m_mgr.Update();
}

bool cMain::closeTab(std::map<wxTreeItemId, ObjectEditor*>::iterator& it)
{
	if (it->second->Close() == true)
	{
		m_mainEditor->DeletePage(m_mainEditor->GetPageIndex(it->second->ToWindow()));
		it = m_openDocuments.erase(it);
		updateAllMenuStates();
		return true;
	}
	return false;
}

bool cMain::closeAll()
{
	auto it = m_openDocuments.begin();
	while (it != m_openDocuments.end())
	{
		if (closeTab(it) == false)
		{
			return false;
		}
	}
	updateAllMenuStates();
	return true;
}

void cMain::saveAll()
{
	for (auto d : m_openDocuments)
	{
		d.second->Save(false);
	}
}

void cMain::buildRom(bool promptForFilename)
{
	if (promptSaveAll() == false)
	{
		return;
	}
	if (promptForFilename == true)
	{
		wxFileDialog fileDlg(this, "Save built ROM As", m_disassemblyPath, "landstalker.bin", "Sega Genesis ROM (*.bin)|*.bin", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
		if (fileDlg.ShowModal() == wxID_OK)
		{
			m_romPath = fileDlg.GetPath().ToStdString();
		}
		else
		{
			return;
		}
	}
	Landstalker::Assembler::Assemble(m_disassemblyPath, "landstalker.asm", m_romPath, "/p /o ae-,e+,w+,c+,op+,os+,ow+,oz+,l_ /e EXPANDED=0", m_output);
}

void cMain::openProject()
{
	if (closeAll() == false)
	{
		return;
	}
	wxDirDialog dir(this, "Select the disassembly directory", "", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
	if (dir.ShowModal() == wxID_OK)
	{
		loadProject(dir.GetPath().ToStdString());
	}
}

void cMain::loadProject(const std::string& path)
{
	m_mainEditor->DeleteAllPages();
	m_openDocuments.clear();
	m_disassemblyPath = path;
	m_romPath = path + "/landstalker.bin";

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
			static_cast<int>(d->Type()),
			static_cast<int>(d->Type()),
			d);
		depth = it.depth();
	}
	m_opened = true;
	updateAllMenuStates();
}

bool cMain::promptSaveAll()
{
	bool modified = false;
	for (auto d : m_openDocuments)
	{
		if (static_cast<ObjectEditor*>(d.second)->IsModified())
		{
			modified = true;
			break;
		}
	}
	if (modified == true)
	{
		auto result = wxMessageBox("Some files have not been saved. Save them now?", "Build ROM", wxYES_NO, this);
		if (result == wxYES)
		{
			saveAll();
		}
		else if (result == wxCANCEL)
		{
			return false;
		}
	}
	return true;
}

void cMain::showAboutBox()
{
	cAboutDlg dlg(this);
}

void cMain::updateMenuState(const std::string& menu, const std::string& submenu, bool enabled)
{
	auto id = m_menu->FindMenuItem(menu, submenu);
	if (id != wxNOT_FOUND)
	{
		m_menu->Enable(id, enabled);
	}
}

void cMain::updateAllMenuStates()
{
	if (m_opened == false)
	{
		updateMenuState("File", "Save", false);
		updateMenuState("File", "Save As...", false);
		updateMenuState("File", "Save All", false);
		updateMenuState("File", "Build ROM", false);
		updateMenuState("File", "Build ROM As...", false);
		updateMenuState("Window", "Close", false);
		updateMenuState("Window", "Close All", false);
		updateMenuState("Window", "Close All But This", false);
	}
	else
	{
		updateMenuState("File", "Build ROM", true);
		updateMenuState("File", "Build ROM As...", true);
		if (m_openDocuments.empty() == true)
		{
			updateMenuState("File", "Save", false);
			updateMenuState("File", "Save As...", false);
			updateMenuState("File", "Save All", false);
			updateMenuState("Window", "Close", false);
			updateMenuState("Window", "Close All", false);
			updateMenuState("Window", "Close All But This", false);
		}
		else
		{
			updateMenuState("File", "Save", true);
			updateMenuState("File", "Save As...", true);
			updateMenuState("File", "Save All", true);
			updateMenuState("Window", "Close", true);
			updateMenuState("Window", "Close All", true);
			updateMenuState("Window", "Close All But This", true);
		}
	}
}

cAboutDlg::cAboutDlg(wxWindow* parent, long style)
	: wxDialog(parent, wxID_ANY, wxEmptyString,
		wxDefaultPosition, wxDefaultSize,
		style | wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{

	// Get version of Scintilla
	wxVersionInfo vi = wxStyledTextCtrl::GetLibraryVersionInfo();

	// sets the application title
	SetTitle(_("About " APP_NAME));

	// about info
	wxGridSizer* aboutinfo = new wxGridSizer(2, 0, 2);
	aboutinfo->Add(new wxStaticText(this, wxID_ANY, _("Written by: ")),
		0, wxALIGN_LEFT);
	aboutinfo->Add(new wxStaticText(this, wxID_ANY, APP_MAINT),
		1, wxEXPAND | wxALIGN_LEFT);
	aboutinfo->Add(new wxStaticText(this, wxID_ANY, _("Version: ")),
		0, wxALIGN_LEFT);
	aboutinfo->Add(new wxStaticText(this, wxID_ANY, wxString::Format("%s (%s)", APP_VERSION, vi.GetVersionString())),
		1, wxEXPAND | wxALIGN_LEFT);
	aboutinfo->Add(new wxStaticText(this, wxID_ANY, _("Licence type: ")),
		0, wxALIGN_LEFT);
	aboutinfo->Add(new wxStaticText(this, wxID_ANY, APP_LICENCE),
		1, wxEXPAND | wxALIGN_LEFT);
	aboutinfo->Add(new wxStaticText(this, wxID_ANY, _("Copyright: ")),
		0, wxALIGN_LEFT);
	aboutinfo->Add(new wxStaticText(this, wxID_ANY, APP_COPYRIGHT),
		1, wxEXPAND | wxALIGN_LEFT);
	aboutinfo->Add(new wxStaticText(this, wxID_ANY, _("Github: ")),
		0, wxALIGN_LEFT);
	aboutinfo->Add(new wxHyperlinkCtrl(this, wxID_ANY, APP_WEBSITE, APP_WEBSITE),
		1, wxEXPAND | wxALIGN_LEFT);
	aboutinfo->Add(new wxStaticText(this, wxID_ANY, _("Email: ")),
		0, wxALIGN_LEFT);
	aboutinfo->Add(new wxHyperlinkCtrl(this, wxID_ANY, APP_MAIL, APP_MAILTO),
		1, wxEXPAND | wxALIGN_LEFT);

	// about icontitle//info
	wxBoxSizer* aboutpane = new wxBoxSizer(wxHORIZONTAL);
	wxBitmap bitmap = wxBitmap(chest_xpm);
	aboutpane->Add(new wxStaticBitmap(this, wxID_ANY, bitmap),
		0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 20);
	aboutpane->Add(aboutinfo, 1, wxEXPAND);
	aboutpane->Add(60, 0);

	// about complete
	wxBoxSizer* totalpane = new wxBoxSizer(wxVERTICAL);
	totalpane->Add(0, 20);
	wxStaticText* appname = new wxStaticText(this, wxID_ANY, APP_NAME);
	appname->SetFont(wxFontInfo(24).Bold());
	totalpane->Add(appname, 0, wxALIGN_CENTER | wxLEFT | wxRIGHT, 40);
	totalpane->Add(0, 10);
	totalpane->Add(aboutpane, 0, wxEXPAND | wxALL, 4);
	totalpane->Add(new wxStaticText(this, wxID_ANY, APP_DESCR),
		0, wxALIGN_CENTER | wxALL, 10);
	wxButton* okButton = new wxButton(this, wxID_OK, _("OK"));
	okButton->SetDefault();
	totalpane->Add(okButton, 0, wxALIGN_CENTER | wxLEFT | wxRIGHT | wxBOTTOM, 10);

	SetSizerAndFit(totalpane);

	CenterOnScreen();
	ShowModal();
}

cAboutDlg::~cAboutDlg()
{
}
