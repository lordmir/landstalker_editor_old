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

#include "img/chest.xpm"


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
	EVT_STC_MODIFIED(wxID_ANY, cMain::OnStcModified)
	EVT_CLOSE(cMain::onClose)
wxEND_EVENT_TABLE()

cMain::cMain() : wxFrame(nullptr, wxID_ANY, "Landstalker Disassembly Editor",
                         wxDefaultPosition, wxSize(800, 600), wxDEFAULT_FRAME_STYLE)
{
	SetIcon(chest_xpm);

	m_menu = new wxMenuBar();
	wxMenu* fileMenu = new wxMenu();
	fileMenu->Append(wxID_OPEN, "&Open...\tCtrl+O");
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

	m_mgr.SetManagedWindow(this);

	m_fileList = new wxTreeCtrl(this, 10003);
	m_fileList->SetWindowStyle(wxTR_HAS_BUTTONS | wxTR_NO_LINES | wxTR_HIDE_ROOT | wxTR_SINGLE);
	wxImageList* im = new wxImageList(16, 16, true);
	im->Add(wxArtProvider::GetIcon(wxART_NORMAL_FILE, wxART_LIST, { 16,16 }));
	im->Add(wxArtProvider::GetIcon(wxART_FOLDER, wxART_LIST, { 16,16 }));
	m_fileList->AssignImageList(im);

	m_output = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(200, 150),
		wxNO_BORDER | wxTE_MULTILINE);
	m_output->SetEditable(false);
	m_output->SetFont(wxFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));

	m_mainEditor = new wxAuiNotebook(this, 10004);

	m_mgr.AddPane(m_fileList, wxAuiPaneInfo().Direction(wxAUI_DOCK_LEFT).Layer(0).Row(0).Position(0).MinSize(100, 100).BestSize(200,100).CaptionVisible(true).MaximizeButton(false).CloseButton(false).MinimizeButton(false).PinButton(false).Caption("Files"));
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
			wxStyledTextCtrl* codeEditor = new wxStyledTextCtrl(this, wxID_ANY, wxDefaultPosition, wxSize(200, 150), wxNO_BORDER | wxTE_MULTILINE);
			codeEditor->SetFont(wxFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
			codeEditor->SetLexer(wxSTC_LEX_A68K);
			for (int i = 0; i < wxSTC_STYLE_LASTPREDEFINED; i++) {
				wxFont font(wxFontInfo(10).Family(wxFONTFAMILY_TELETYPE));
				codeEditor->StyleSetFont(i, font);
			}
			codeEditor->SetMarginType(0, wxSTC_MARGIN_NUMBER);
			codeEditor->StyleSetForeground(wxSTC_STYLE_LINENUMBER, wxColour("DARK GREY"));
			codeEditor->StyleSetBackground(wxSTC_STYLE_LINENUMBER, *wxWHITE);
			codeEditor->SetMarginWidth(0, codeEditor->TextWidth(wxSTC_STYLE_LINENUMBER, "_999999"));
			// set common styles
			codeEditor->StyleSetForeground(wxSTC_STYLE_DEFAULT, wxColour("DARK GREY"));
			codeEditor->StyleSetForeground(wxSTC_STYLE_INDENTGUIDE, wxColour("DARK GREY"));
			codeEditor->StyleSetForeground(wxSTC_A68K_COMMENT, wxColour("FOREST GREEN"));
			codeEditor->StyleSetForeground(wxSTC_A68K_COMMENT_DOXYGEN, wxColour("FOREST GREEN"));
			codeEditor->StyleSetForeground(wxSTC_A68K_COMMENT_SPECIAL, wxColour("FOREST GREEN"));
			codeEditor->StyleSetForeground(wxSTC_A68K_COMMENT_WORD, wxColour("FOREST GREEN"));
			codeEditor->StyleSetForeground(wxSTC_A68K_CPUINSTRUCTION, wxColour("NAVY"));
			codeEditor->StyleSetBold(wxSTC_A68K_CPUINSTRUCTION, true);
			codeEditor->StyleSetForeground(wxSTC_A68K_DIRECTIVE, wxColour("BLUE"));
			codeEditor->StyleSetForeground(wxSTC_A68K_EXTINSTRUCTION, wxColour("MAGENTA"));
			//codeEditor->StyleSetForeground(wxSTC_A68K_IDENTIFIER, wxColour("BLACK"));
			codeEditor->StyleSetForeground(wxSTC_A68K_LABEL, wxColour(0, 127, 127));
			codeEditor->StyleSetForeground(wxSTC_A68K_MACRO_ARG, wxColour("PURPLE"));
			codeEditor->StyleSetForeground(wxSTC_A68K_MACRO_DECLARATION, wxColour("PURPLE"));
			codeEditor->StyleSetForeground(wxSTC_A68K_NUMBER_BIN, wxColour("BROWN"));
			codeEditor->StyleSetForeground(wxSTC_A68K_NUMBER_DEC, wxColour("BROWN"));
			codeEditor->StyleSetForeground(wxSTC_A68K_NUMBER_HEX, wxColour("BROWN"));
			codeEditor->StyleSetForeground(wxSTC_A68K_OPERATOR, wxColour("NAVY"));
			codeEditor->StyleSetBold(wxSTC_A68K_OPERATOR, true);
			codeEditor->StyleSetForeground(wxSTC_A68K_REGISTER, wxColour("BLUE"));
			codeEditor->StyleSetBold(wxSTC_A68K_REGISTER, true);
			codeEditor->StyleSetForeground(wxSTC_A68K_STRING1, wxColour("BROWN"));
			codeEditor->StyleSetForeground(wxSTC_A68K_STRING2, wxColour("BROWN"));
			codeEditor->SetKeyWords(0, "abcd abcd.b add add.b add.w add.l adda adda.w adda.l "
			                           "addi addi.b addi.w addi.l addq addq.b addq.w addq.l "
			                           "addx addx.b addx.w addx.l and and.b and.w and.l "
			                           "andi andi.b andi.w andi.l asl asl.b asl.w asl.l "
			                           "asr asr.b asr.w asr.l bcc bcc.s bcc.w bcs bcs.s bcs.w "
			                           "beq beq.s beq.w bge bge.s bge.w bgt bgt.s bgt.w "
				                       "bhi bhi.s bhi.w ble ble.s ble.w bls bls.s bls.w "
			                           "blt blt.s blt.w bmi bmi.s bmi.w bne bne.s bne.w "
			                           "bpl bpl.s bpl.w bvc bvc.s bvc.w bvs bvs.s bvs.w "
			                           "bchg bchg.b bchg.l bclr bclr.b bclr.l bra.s bra.w "
			                           "bset bset.b bset.l bsr bsr.s bsr.w btst btst.b btst.w "
			                           "chk chk.w clr clr.b clr.w clr.l cmp cmp.b cmp.w cmp.l "
			                           "cmpa cmpa.w cmpa.l cmpi cmpi.b cmpi.w cmpi.l "
				                       "cmpm cmpm.b cmpm.w cmpm.l dbcc dbcc.w dbcs dbcs.w "
				                       "dbeq dbeq.w dbge dbge.w dbgt dbgt.w dbhi dbhi.w "
				                       "dble dble.w dbls dbls.w dblt dblt.w dbmi dbmi.w "
				                       "dbne dbne.w dbpl dbpl.w dbvc dbvc.w dbvs dbvs.w "
				                       "dbf dbf.w dbt dbt.w dbra dbra.w divs divs.w divs.l "
				                       "divu divu.w divu.l eor eor.b eor.w eor.l eori eori.b "
                                       "eori.w eori.l exg exg.l ext ext.w ext.l illegal jmp "
				                       "jsr lea lea.l link link lsl lsl.b lsl.w lsl.l "
				                       "lsr lsr.b lsr.w lsr.l move move.b move.w move.l "
				                       "movea movea.w movea.l movem movem.w movem.l movep "
				                       "movep.w movep.l moveq moveq.l muls muls.w mulu mulu.w "
				                       "nbcd nbcd.b neg neg.b neg.w neg.l negx negx.b negx.w "
				                       "negx.l nop not not.b not.w not.l or or.b or.w or.l "
				                       "ori ori.b ori.w ori.l pea pea.l reset rol rol.b rol.w "
				                       "rol.l ror ror.b ror.w ror.l roxl roxl.b roxl.w roxl.l "
				                       "roxr roxr.b roxr.w roxr.l rte rtr rts sbcd sbcd.b "
				                       "scc scc.b scs scs.b seq seq.b sge sge.b sgt sgt.b "
				                       "shi shi.b sle sle.b sls sls.b slt slt.b smi smi.b "
				                       "sne sne.b spl spl.b svc svc.b svs svs.b stop sub "
				                       "sub.b sub.w sub.l suba suba.w suba.l subi subi.b "
				                       "subi.w subi.l subq subq.b subq.w subq.l subx subx.b "
				                       "subx.w subx.l swap swap.w tas tas.b trap trapv "
				                       "tst tst.b tst.w tst.l ulnk"); // Instructions
			codeEditor->SetKeyWords(1, "d0 d1 d2 d3 d4 d5 d6 d7 a0 a1 a2 a3 a4 a5 a6 a7 sp sr usp ssp ccr"); // Registers
			codeEditor->SetKeyWords(2, "dc dc.b dc.w dc.l dcb dcb.b dcb.w dcb.l ds ds.b ds.w ds.l "
				                       "_year _month _day _weekday _hours _minutes _seconds narg __rs "
				                       "_filename def ref type sqrt strlen strcmp instr sect offset sectoff "
			                           "group groupoff filesize groupsize grouporg groupend sectend sectsize "
			                           "alignment radix alias disable equ set equs equr reg rs rs.b rs.w rs.l "
			                           "rsset rsreset hex data datasize ieee32 ieee64 org even cnop obj objend "
			                           "include incbin end if else elseif endif case endcase rept endr while "
			                           "endw do until regs unit macro endm mexit shift macros pushp popp purge "
			                           "type substr module modend local section group pushs pops word bss size "
			                           "over opt pusho popo list nolist inform fail xref xdef public global "); // Directives
			codeEditor->SetKeyWords(3, "ScriptID ScriptJump Align PadTo ROM_End UnlockSRAM LockSRAM ExpandBsr "); // External Instructions
			codeEditor->LoadFile(d.Path());
			codeEditor->SetSavePoint();
			
			m_mainEditor->InsertPage(m_mainEditor->GetSelection() + 1, codeEditor, m_fileList->GetItemText(evt.GetItem()), true);
			m_openDocuments.insert({ evt.GetItem(), codeEditor });
		}
	}
	else
	{
		auto pageId = m_mainEditor->GetPageIndex(doc->second);
		if (pageId == wxNOT_FOUND)
		{
			m_openDocuments.erase(evt.GetItem());
		}
		else
		{
			m_mainEditor->ChangeSelection(pageId);
		}
	}
	evt.Skip();
}

void cMain::onAuiNotebookPageClose(wxAuiNotebookEvent& evt)
{
	auto* window = m_mainEditor->GetPage(evt.GetSelection());
	auto it = m_openDocuments.begin();
	while (it != m_openDocuments.end())
	{
		if (it->second == window)
		{
			if (closeTab(it) == false)
			{
				break;
			}
		}
		else
		{
			++it;
		}
	}
	// Always veto, as we close tabs programatically
	evt.Veto();
	evt.Skip();
}

void cMain::OnStcModified(wxStyledTextEvent& evt)
{
	auto* tab = static_cast<wxStyledTextCtrl*>(evt.GetEventObject());
	auto id = m_mainEditor->GetPageIndex(tab);
	if (id != wxNOT_FOUND)
	{
		wxTreeItemId treeItem;
		for (auto d : m_openDocuments)
		{
			if (d.second == tab)
				treeItem = d.first;
		}
		std::string path = static_cast<Landstalker::FileData*>(m_fileList->GetItemData(treeItem))->Path();
		std::string fname = m_fileList->GetItemText(treeItem).ToStdString();
		if (tab->IsModified() == true)
		{
			m_mainEditor->SetPageText(id, fname + "*");
		}
		else
		{
			m_mainEditor->SetPageText(id, fname);
		}
	}
	evt.Skip();
}

void cMain::onMenuOpen(wxCommandEvent& evt)
{
	openProject();
	evt.Skip();
}

void cMain::onMenuSave(wxCommandEvent& evt)
{
	saveFile(m_mainEditor->GetPage(m_mainEditor->GetSelection()));
	evt.Skip();
}

void cMain::onMenuSaveAs(wxCommandEvent& evt)
{
	saveFile(m_mainEditor->GetPage(m_mainEditor->GetSelection()), true);
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
	auto it = m_openDocuments.begin();
	auto* currentPage = m_mainEditor->GetPage(m_mainEditor->GetSelection());
	while (it != m_openDocuments.end())
	{
		if (it->second == currentPage)
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

void cMain::onMenuCloseAll(wxCommandEvent& evt)
{
	closeAll();
}

void cMain::onMenuCloseAllButThis(wxCommandEvent& evt)
{
	auto it = m_openDocuments.begin();
	auto* currentPage = m_mainEditor->GetPage(m_mainEditor->GetSelection());
	while (it != m_openDocuments.end())
	{
		if (it->second != currentPage)
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

bool cMain::closeTab(std::map<wxTreeItemId, wxWindow*>::iterator& it)
{
	bool ok = true;
	auto* tab = static_cast<wxStyledTextCtrl*>(it->second);
	if (tab->IsModified())
	{
		auto path = static_cast<Landstalker::FileData*>(m_fileList->GetItemData(it->first))->Path();
		std::string msg("Do you want to save changes to " + m_fileList->GetItemText(it->first) + "?");
		auto result = wxMessageBox(msg, "Save Changes", wxYES_NO | wxCANCEL, this);
		if (result == wxYES)
		{
			saveFile(tab);
		}
		else if (result == wxCANCEL)
		{
			ok = false;
		}
	}
	if (ok == true)
	{
		m_mainEditor->DeletePage(m_mainEditor->GetPageIndex(tab));
		it = m_openDocuments.erase(it);
	}
	return ok;
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
	return true;
}

void cMain::saveAll()
{
	for (auto d : m_openDocuments)
	{
		saveFile(d.second);
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

void cMain::saveFile(wxWindow* file, bool promptForFilename, bool force)
{
	wxStyledTextCtrl* tab = static_cast<wxStyledTextCtrl*>(m_mainEditor->GetPage(m_mainEditor->GetSelection()));
	wxTreeItemId treeItem;
	for (auto d : m_openDocuments)
	{
		if (d.second == tab)
			treeItem = d.first;
	}
	std::string path = static_cast<Landstalker::FileData*>(m_fileList->GetItemData(treeItem))->Path();
	std::string fname = m_fileList->GetItemText(treeItem).ToStdString();
	if (promptForFilename == true)
	{
		wxFileDialog fileDlg(this, "Save file as...", path, fname, "All Files (*.*)|*.*", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
		if (fileDlg.ShowModal() == wxID_OK)
		{
			path = fileDlg.GetPath().ToStdString();
		}
		else
		{
			return;
		}
	}
	if (tab->IsModified() || promptForFilename || force)
	{
		tab->SaveFile(path);
		m_mainEditor->SetPageText(m_mainEditor->GetSelection(), fname);
		tab->SetSavePoint();
	}

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
			std::filesystem::is_directory(*it) ? 1 : 0,
			std::filesystem::is_directory(*it) ? 1 : 0,
			d);
		depth = it.depth();
	}
}

bool cMain::promptSaveAll()
{
	bool modified = false;
	for (auto d : m_openDocuments)
	{
		if (static_cast<wxStyledTextCtrl*>(d.second)->IsModified())
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
