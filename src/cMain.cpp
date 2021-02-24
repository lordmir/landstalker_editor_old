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

#include "Assembler.h"
#include "FileData.h"


wxBEGIN_EVENT_TABLE(cMain, wxFrame)
	EVT_BUTTON(10001, cMain::onLoadButtonClick)
	EVT_BUTTON(10002, cMain::onBuildButtonClick)
	EVT_TREE_ITEM_ACTIVATED(10003, cMain::onFileActivate)
	EVT_AUINOTEBOOK_PAGE_CLOSE(10004, cMain::onAuiNotebookPageClose)
	EVT_STC_STYLENEEDED(wxID_ANY, cMain::OnStcStyleNeeded)
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

	m_mainEditor = new wxAuiNotebook(this, 10004);

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
	auto doc = m_openDocuments.find(evt.GetItem());
	if (doc == m_openDocuments.end())
	{
		const auto& d = *static_cast<Landstalker::FileData*>(m_fileList->GetItemData(evt.GetItem()));
		if (d.IsFile() == true)
		{
			std::ifstream file(d.Path());
			std::string str((std::istreambuf_iterator<char>(file)),
				std::istreambuf_iterator<char>());
			//m_codeEditor = 
			wxStyledTextCtrl* codeEditor = new wxStyledTextCtrl(this, wxID_ANY, wxDefaultPosition, wxSize(200, 150), wxNO_BORDER | wxTE_MULTILINE);
//			codeEditor->SetEditable(false);
			codeEditor->SetFont(wxFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
			codeEditor->AddText(str);
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
	auto it = m_openDocuments.cbegin();
	while (it != m_openDocuments.cend())
	{
		if (it->second == window)
		{
			it = m_openDocuments.erase(it);
		}
		else
		{
			++it;
		}
	}
	evt.Skip();
}

void cMain::OnStcStyleNeeded(wxStyledTextEvent& evt)
{
	wxMessageBox("Passion");
}
