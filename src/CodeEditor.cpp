#include "CodeEditor.h"

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

BEGIN_EVENT_TABLE(CodeEditor, wxStyledTextCtrl)
	EVT_STC_MODIFIED(wxID_ANY, CodeEditor::OnStcModified)
END_EVENT_TABLE()

CodeEditor::CodeEditor(wxWindow* parent, const std::string& name,
	                   const wxTreeItemId& treeItemId, const std::filesystem::path& filename)
: wxStyledTextCtrl(parent, wxID_ANY, wxDefaultPosition, wxSize(200, 150), wxNO_BORDER | wxTE_MULTILINE),
  ObjectEditor(parent, name, treeItemId),
  m_filename(filename)
{
	this->SetFont(wxFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
	this->SetLexer(wxSTC_LEX_A68K);
	for (int i = 0; i < wxSTC_STYLE_LASTPREDEFINED; i++) {
		wxFont font(wxFontInfo(10).Family(wxFONTFAMILY_TELETYPE));
		this->StyleSetFont(i, font);
	}
	this->SetMarginType(0, wxSTC_MARGIN_NUMBER);
	this->StyleSetForeground(wxSTC_STYLE_LINENUMBER, wxColour("DARK GREY"));
	this->StyleSetBackground(wxSTC_STYLE_LINENUMBER, *wxWHITE);
	this->SetMarginWidth(0, this->TextWidth(wxSTC_STYLE_LINENUMBER, "_999999"));
	// set common styles
	this->StyleSetForeground(wxSTC_STYLE_DEFAULT, wxColour("DARK GREY"));
	this->StyleSetForeground(wxSTC_STYLE_INDENTGUIDE, wxColour("DARK GREY"));
	this->StyleSetForeground(wxSTC_A68K_COMMENT, wxColour("FOREST GREEN"));
	this->StyleSetForeground(wxSTC_A68K_COMMENT_DOXYGEN, wxColour("FOREST GREEN"));
	this->StyleSetForeground(wxSTC_A68K_COMMENT_SPECIAL, wxColour("FOREST GREEN"));
	this->StyleSetForeground(wxSTC_A68K_COMMENT_WORD, wxColour("FOREST GREEN"));
	this->StyleSetForeground(wxSTC_A68K_CPUINSTRUCTION, wxColour("NAVY"));
	this->StyleSetBold(wxSTC_A68K_CPUINSTRUCTION, true);
	this->StyleSetForeground(wxSTC_A68K_DIRECTIVE, wxColour("BLUE"));
	this->StyleSetForeground(wxSTC_A68K_EXTINSTRUCTION, wxColour("MAGENTA"));
	//codeEditor->StyleSetForeground(wxSTC_A68K_IDENTIFIER, wxColour("BLACK"));
	this->StyleSetForeground(wxSTC_A68K_LABEL, wxColour(0, 127, 127));
	this->StyleSetForeground(wxSTC_A68K_MACRO_ARG, wxColour("PURPLE"));
	this->StyleSetForeground(wxSTC_A68K_MACRO_DECLARATION, wxColour("PURPLE"));
	this->StyleSetForeground(wxSTC_A68K_NUMBER_BIN, wxColour("BROWN"));
	this->StyleSetForeground(wxSTC_A68K_NUMBER_DEC, wxColour("BROWN"));
	this->StyleSetForeground(wxSTC_A68K_NUMBER_HEX, wxColour("BROWN"));
	this->StyleSetForeground(wxSTC_A68K_OPERATOR, wxColour("NAVY"));
	this->StyleSetBold(wxSTC_A68K_OPERATOR, true);
	this->StyleSetForeground(wxSTC_A68K_REGISTER, wxColour("BLUE"));
	this->StyleSetBold(wxSTC_A68K_REGISTER, true);
	this->StyleSetForeground(wxSTC_A68K_STRING1, wxColour("BROWN"));
	this->StyleSetForeground(wxSTC_A68K_STRING2, wxColour("BROWN"));
	this->SetKeyWords(0, "abcd abcd.b add add.b add.w add.l adda adda.w adda.l "
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
	this->SetKeyWords(1, "d0 d1 d2 d3 d4 d5 d6 d7 a0 a1 a2 a3 a4 a5 a6 a7 sp sr usp ssp ccr"); // Registers
	this->SetKeyWords(2, "dc dc.b dc.w dc.l dcb dcb.b dcb.w dcb.l ds ds.b ds.w ds.l "
		"_year _month _day _weekday _hours _minutes _seconds narg __rs "
		"_filename def ref type sqrt strlen strcmp instr sect offset sectoff "
		"group groupoff filesize groupsize grouporg groupend sectend sectsize "
		"alignment radix alias disable equ set equs equr reg rs rs.b rs.w rs.l "
		"rsset rsreset hex data datasize ieee32 ieee64 org even cnop obj objend "
		"include incbin end if else elseif endif case endcase rept endr while "
		"endw do until regs unit macro endm mexit shift macros pushp popp purge "
		"type substr module modend local section group pushs pops word bss size "
		"over opt pusho popo list nolist inform fail xref xdef public global "); // Directives
	this->SetKeyWords(3, "ScriptID ScriptJump Align PadTo ROM_End UnlockSRAM LockSRAM ExpandBsr "); // External Instructions
	this->LoadFile(m_filename.generic_string());
	MarkSaved();
}

bool CodeEditor::Save(bool prompt, bool force)
{
	auto path = m_filename;
	if (prompt == true)
	{
		wxFileDialog fileDlg(this, "Save file as...", m_filename.generic_string(), m_filename.filename().generic_string(), "All Files (*.*)|*.*", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
		if (fileDlg.ShowModal() == wxID_OK)
		{
			path = fileDlg.GetPath().ToStdString();
		}
		else
		{
			return false;
		}
	}
	if (IsModified() || prompt || force)
	{
		wxStyledTextCtrl::SaveFile(path.generic_string());
		MarkSaved();
	}
	return true;
}

bool CodeEditor::IsModified() const
{
	return wxStyledTextCtrl::IsModified();
}

void CodeEditor::MarkSaved()
{
	wxStyledTextCtrl::SetSavePoint();
	FireModifiedEvent();
}

bool CodeEditor::Close()
{
	if (IsModified())
	{
		std::string msg("Do you want to save changes to " + GetObjectName() + "?");
		auto result = wxMessageBox(msg, "Save Changes", wxYES_NO | wxCANCEL, this);
		if (result == wxYES)
		{
			Save(false);
		}
		else if (result == wxCANCEL)
		{
			return false;
		}
	}
	return true;
}

wxWindow* CodeEditor::ToWindow()
{
	return this;
}

ObjectType CodeEditor::GetObjectType() const
{
	return ObjectType::ASSEMBLY_SOURCE;
}

std::string CodeEditor::GetObjectDescription() const
{
	return "Assembly Source File";
}

const std::filesystem::path& CodeEditor::GetPath() const
{
	return m_filename;
}

void CodeEditor::FireModifiedEvent()
{
	wxCommandEvent evt(OBJECT_EDITOR_MODIFY);
	evt.SetEventObject(this);

	wxPostEvent(this, evt);
}

void CodeEditor::OnStcModified(wxStyledTextEvent& evt)
{
	FireModifiedEvent();
	evt.Skip();
}