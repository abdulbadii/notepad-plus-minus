// This file is part of Notepad++ project
// Copyright (C)2003 Don HO <don.h@free.fr>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// Note that the GPL places important restrictions on "derived works", yet
// it does not provide a detailed definition of that term.  To avoid
// misunderstandings, we consider an application to constitute a
// "derivative work" for the purpose of this license if it does any of the
// following:
// 1. Integrates source code from Notepad++.
// 2. Integrates/includes/aggregates Notepad++ into a proprietary executable
//    installer, such as those produced by InstallShield.
// 3. Links to a library or executes a program that does any of the above.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include <memory>
#include <shlwapi.h>
#include <cinttypes>
#include "ScintillaEditView.h"
#include "Parameters.h"
#include "Sorters.h"
#include "tchar.h"
#include "verifySignedfile.h"

using namespace std;
WcharMbcsConvertor& wmc = WcharMbcsConvertor::getInstance();	

// initialize the static variable

// get full ScinLexer.dll path to avoid hijack
TCHAR * getSciLexerFullPathName(TCHAR * moduleFileName, size_t len)	{

	::GetModuleFileName(NULL, moduleFileName, static_cast<int32_t>(len));
	::PathRemoveFileSpec(moduleFileName);
	::PathAppend(moduleFileName, L"SciLexer.dll");
	return moduleFileName;
};

HINSTANCE ScintillaEditView::_hLib = loadSciLexerDll();
int ScintillaEditView::_refCount = 0;
UserDefineDialog ScintillaEditView::_userDefineDlg;

const int ScintillaEditView::_SC_MARGE_LINENUMBER = 0;
const int ScintillaEditView::_SC_MARGE_SYBOLE = 1;
const int ScintillaEditView::_SC_MARGE_FOLDER = 2;

WNDPROC ScintillaEditView::_scintillaDefaultProc = NULL;
string ScintillaEditView::_defaultCharList = "";

/*
SC_MARKNUM_*     | Arrow               Plus/minus           Circle tree                 Box tree
-------------------------------------------------------------------------------------------------------------
FOLDEROPEN       | SC_MARK_ARROWDOWN   SC_MARK_MINUS     SC_MARK_CIRCLEMINUS            SC_MARK_BOXMINUS
FOLDER           | SC_MARK_ARROW       SC_MARK_PLUS      SC_MARK_CIRCLEPLUS             SC_MARK_BOXPLUS
FOLDERSUB        | SC_MARK_EMPTY       SC_MARK_EMPTY     SC_MARK_VLINE                  SC_MARK_VLINE
FOLDERTAIL       | SC_MARK_EMPTY       SC_MARK_EMPTY     SC_MARK_LCORNERCURVE           SC_MARK_LCORNER
FOLDEREND        | SC_MARK_EMPTY       SC_MARK_EMPTY     SC_MARK_CIRCLEPLUSCONNECTED    SC_MARK_BOXPLUSCONNECTED
FOLDEROPENMID    | SC_MARK_EMPTY       SC_MARK_EMPTY     SC_MARK_CIRCLEMINUSCONNECTED   SC_MARK_BOXMINUSCONNECTED
FOLDERMIDTAIL    | SC_MARK_EMPTY       SC_MARK_EMPTY     SC_MARK_TCORNERCURVE           SC_MARK_TCORNER
*/

const int ScintillaEditView::_markersArray[][NB_FOLDER_STATE] = {
	{SC_MARKNUM_FOLDEROPEN, SC_MARKNUM_FOLDER, SC_MARKNUM_FOLDERSUB, SC_MARKNUM_FOLDERTAIL, SC_MARKNUM_FOLDEREND,        SC_MARKNUM_FOLDEROPENMID,     SC_MARKNUM_FOLDERMIDTAIL},
	{SC_MARK_MINUS,         SC_MARK_PLUS,      SC_MARK_EMPTY,        SC_MARK_EMPTY,         SC_MARK_EMPTY,               SC_MARK_EMPTY,                SC_MARK_EMPTY},
	{SC_MARK_ARROWDOWN,     SC_MARK_ARROW,     SC_MARK_EMPTY,        SC_MARK_EMPTY,         SC_MARK_EMPTY,               SC_MARK_EMPTY,                SC_MARK_EMPTY},
	{SC_MARK_CIRCLEMINUS,   SC_MARK_CIRCLEPLUS,SC_MARK_VLINE,        SC_MARK_LCORNERCURVE,  SC_MARK_CIRCLEPLUSCONNECTED, SC_MARK_CIRCLEMINUSCONNECTED, SC_MARK_TCORNERCURVE},
	{SC_MARK_BOXMINUS,      SC_MARK_BOXPLUS,   SC_MARK_VLINE,        SC_MARK_LCORNER,       SC_MARK_BOXPLUSCONNECTED,    SC_MARK_BOXMINUSCONNECTED,    SC_MARK_TCORNER}
};

// Array with all the names of all languages
// The order of lang type (enum LangType) must be respected
LanguageName ScintillaEditView::langNames[L_EXTERNAL+1] = {
{L"normal",		L"Normal text",		L"Normal text file",								L_TEXT,			SCLEX_NULL},
{L"php",			L"PHP",				L"PHP Hypertext Preprocessor file",				L_PHP,			SCLEX_HTML},
{L"c",				L"C",					L"C source file",									L_C,			SCLEX_CPP},
{L"cpp",			L"C++",				L"C++ source file",								L_CPP,			SCLEX_CPP},
{L"cs",			L"C#",					L"C# source file",									L_CS,			SCLEX_CPP},
{L"objc",			L"Objective-C",		L"Objective-C source file",						L_OBJC,			SCLEX_CPP},
{L"java",			L"Java",				L"Java source file",								L_JAVA,			SCLEX_CPP},
{L"rc",			L"RC",					L"Windows Resource file",							L_RC,			SCLEX_CPP},
{L"html",			L"HTML",				L"Hyper Text Markup Language file",				L_HTML,			SCLEX_HTML},
{L"xml",			L"XML",				L"eXtensible Markup Language file",				L_XML,			SCLEX_XML},
{L"makefile",		L"Makefile",			L"Makefile",										L_MAKEFILE,		SCLEX_MAKEFILE},
{L"pascal",		L"Pascal",				L"Pascal source file",								L_PASCAL,		SCLEX_PASCAL},
{L"batch",			L"Batch",				L"Batch file",										L_BATCH,		SCLEX_BATCH},
{L"ini",			L"ini",				L"MS ini file",									L_INI,			SCLEX_PROPERTIES},
{L"nfo",			L"NFO",				L"MSDOS Style/ASCII Art",							L_ASCII,		SCLEX_NULL},
{L"udf",			L"udf",				L"User Defined language file",						L_USER,			SCLEX_USER},
{L"asp",			L"ASP",				L"Active Server Pages script file",				L_ASP,			SCLEX_HTML},
{L"sql",			L"SQL",				L"Structured Query Language file",					L_SQL,			SCLEX_SQL},
{L"vb",			L"Visual Basic",		L"Visual Basic file",								L_VB,			SCLEX_VB},
{L"javascript",	L"JavaScript",			L"JavaScript file",								L_JS,			SCLEX_CPP},
{L"css",			L"CSS",				L"Cascade Style Sheets File",						L_CSS,			SCLEX_CSS},
{L"perl",			L"Perl",				L"Perl source file",								L_PERL,			SCLEX_PERL},
{L"python",		L"Python",				L"Python file",									L_PYTHON,		SCLEX_PYTHON},
{L"lua",			L"Lua",				L"Lua source File",								L_LUA,			SCLEX_LUA},
{L"tex",			L"TeX",				L"TeX file",										L_TEX,			SCLEX_TEX},
{L"fortran",		L"Fortran free form",	L"Fortran free form source file",					L_FORTRAN,		SCLEX_FORTRAN},
{L"bash",			L"Shell",				L"Unix script file",								L_BASH,			SCLEX_BASH},
{L"actionscript",	L"ActionScript",		L"Flash ActionScript file",						L_FLASH,		SCLEX_CPP},
{L"nsis",			L"NSIS",				L"Nullsoft Scriptable Install System script file",	L_NSIS,			SCLEX_NSIS},
{L"tcl",			L"TCL",				L"Tool Command Language file",						L_TCL,			SCLEX_TCL},
{L"lisp",			L"Lisp",				L"List Processing language file",					L_LISP,			SCLEX_LISP},
{L"scheme",		L"Scheme",				L"Scheme file",									L_SCHEME,		SCLEX_LISP},
{L"asm",			L"Assembly",			L"Assembly language source file",					L_ASM,			SCLEX_ASM},
{L"diff",			L"Diff",				L"Diff file",										L_DIFF,			SCLEX_DIFF},
{L"props",			L"Properties file",	L"Properties file",								L_PROPS,		SCLEX_PROPERTIES},
{L"postscript",	L"PostScript",			L"PostScript file",								L_PS,			SCLEX_PS},
{L"ruby",			L"Ruby",				L"Ruby file",										L_RUBY,			SCLEX_RUBY},
{L"smalltalk",		L"Smalltalk",			L"Smalltalk file",									L_SMALLTALK,	SCLEX_SMALLTALK},
{L"vhdl",			L"VHDL",				L"VHSIC Hardware Description Language file",		L_VHDL,			SCLEX_VHDL},
{L"kix",			L"KiXtart",			L"KiXtart file",									L_KIX,			SCLEX_KIX},
{L"autoit",		L"AutoIt",				L"AutoIt",											L_AU3,			SCLEX_AU3},
{L"caml",			L"CAML",				L"Categorical Abstract Machine Language",			L_CAML,			SCLEX_CAML},
{L"ada",			L"Ada",				L"Ada file",										L_ADA,			SCLEX_ADA},
{L"verilog",		L"Verilog",			L"Verilog file",									L_VERILOG,		SCLEX_VERILOG},
{L"matlab",		L"MATLAB",				L"MATrix LABoratory",								L_MATLAB,		SCLEX_MATLAB},
{L"haskell",		L"Haskell",			L"Haskell",										L_HASKELL,		SCLEX_HASKELL},
{L"inno",			L"Inno Setup",			L"Inno Setup script",								L_INNO,			SCLEX_INNOSETUP},
{L"searchResult",	L"Internal Search",	L"Internal Search",								L_SEARCHRESULT,	SCLEX_SEARCHRESULT},
{L"cmake",			L"CMake",				L"CMake file",										L_CMAKE,		SCLEX_CMAKE},
{L"yaml",			L"YAML",				L"YAML Ain't Markup Language",						L_YAML,			SCLEX_YAML},
{L"cobol",			L"COBOL",				L"COmmon Business Oriented Language",				L_COBOL,		SCLEX_COBOL},
{L"gui4cli",		L"Gui4Cli",			L"Gui4Cli file",									L_GUI4CLI,		SCLEX_GUI4CLI},
{L"d",				L"D",					L"D programming language",							L_D,			SCLEX_D},
{L"powershell",	L"PowerShell",			L"Windows PowerShell",								L_POWERSHELL,	SCLEX_POWERSHELL},
{L"r",				L"R",					L"R programming language",							L_R,			SCLEX_R},
{L"jsp",			L"JSP",				L"JavaServer Pages script file",					L_JSP,			SCLEX_HTML},
{L"coffeescript",	L"CoffeeScript",		L"CoffeeScript file",								L_COFFEESCRIPT,	SCLEX_COFFEESCRIPT},
{L"json",			L"json",				L"JSON file",										L_JSON,			SCLEX_JSON },
{L"javascript.js", L"JavaScript",			L"JavaScript file",								L_JAVASCRIPT,	SCLEX_CPP },
{L"fortran77",		L"Fortran fixed form",	L"Fortran fixed form source file",					L_FORTRAN_77,	SCLEX_F77},
{L"baanc",			L"BaanC",				L"BaanC File",										L_BAANC,		SCLEX_BAAN },
{L"srec",			L"S-Record",			L"Motorola S-Record binary data",					L_SREC,			SCLEX_SREC},
{L"ihex",			L"Intel HEX",			L"Intel HEX binary data",							L_IHEX,			SCLEX_IHEX},
{L"tehex",			L"Tektronix extended HEX",	L"Tektronix extended HEX binary data",			L_TEHEX,		SCLEX_TEHEX},
{L"swift",			L"Swift",              L"Swift file",										L_SWIFT,		SCLEX_CPP},
{L"asn1",			L"ASN.1",				L"Abstract Syntax Notation One file",				L_ASN1,			SCLEX_ASN1},
{L"avs",			L"AviSynth",			L"AviSynth scripts files",							L_AVS,			SCLEX_AVS},
{L"blitzbasic",	L"BlitzBasic",			L"BlitzBasic file",								L_BLITZBASIC,	SCLEX_BLITZBASIC},
{L"purebasic",		L"PureBasic",			L"PureBasic file",									L_PUREBASIC,	SCLEX_PUREBASIC},
{L"freebasic",		L"FreeBasic",			L"FreeBasic file",									L_FREEBASIC,	SCLEX_FREEBASIC},
{L"csound",		L"Csound",				L"Csound file",									L_CSOUND,		SCLEX_CSOUND},
{L"erlang",		L"Erlang",				L"Erlang file",									L_ERLANG,		SCLEX_ERLANG},
{L"escript",		L"ESCRIPT",			L"ESCRIPT file",									L_ESCRIPT,		SCLEX_ESCRIPT},
{L"forth",			L"Forth",				L"Forth file",										L_FORTH,		SCLEX_FORTH},
{L"latex",			L"LaTeX",				L"LaTeX file",										L_LATEX,		SCLEX_LATEX},
{L"mmixal",		L"MMIXAL",				L"MMIXAL file",									L_MMIXAL,		SCLEX_MMIXAL},
{L"nimrod",		L"Nimrod",				L"Nimrod file",									L_NIMROD,		SCLEX_NIMROD},
{L"nncrontab",		L"Nncrontab",			L"extended crontab file",							L_NNCRONTAB,	SCLEX_NNCRONTAB},
{L"oscript",		L"OScript",			L"OScript source file",							L_OSCRIPT,		SCLEX_OSCRIPT},
{L"rebol",			L"REBOL",				L"REBOL file",										L_REBOL,		SCLEX_REBOL},
{L"registry",		L"registry",			L"registry file",									L_REGISTRY,		SCLEX_REGISTRY},
{L"rust",			L"Rust",				L"Rust file",										L_RUST,			SCLEX_RUST},
{L"spice",			L"Spice",				L"spice file",										L_SPICE,		SCLEX_SPICE},
{L"txt2tags",		L"txt2tags",			L"txt2tags file",									L_TXT2TAGS,		SCLEX_TXT2TAGS},
{L"visualprolog",	L"Visual Prolog",		L"Visual Prolog file",								L_VISUALPROLOG,	SCLEX_VISUALPROLOG},
{L"ext",			L"External",			L"External",										L_EXTERNAL,		SCLEX_NULL}
};

//const int MASK_RED   = 0xFF0000;
//const int MASK_GREEN = 0x00FF00;
//const int MASK_BLUE  = 0x0000FF;


int getNbDigits(int aNum, int base)	{

	int nbChiffre = 1;
	int diviseur = base;

	for (;;)	{

		int result = aNum / diviseur;
		if (!result)
			break;
		else	{

			diviseur *= base;
			++nbChiffre;
		}
	}
	if ((base == 16) && (nbChiffre % 2 != 0))
		nbChiffre += 1;

	return nbChiffre;
}

TCHAR moduleFileName[1024];

HMODULE loadSciLexerDll()	{

	generic_string sciLexerPath = getSciLexerFullPathName(moduleFileName, 1024);

	// Do not check dll signature if npp is running in debug mode
	// This is helpful for developers to skip signature checking
	// while analyzing issue or modifying the lexer dll
#ifndef _DEBUG
	SecurityGard securityGard;
	bool isOK = securityGard.checkModule(sciLexerPath, nm_scilexer);

	if (!isOK)	{

		::MessageBox(NULL,
			L"Authenticode check failed:\rsigning certificate or hash is not recognized",
			L"Library verification failed",
			MB_OK | MB_ICONERROR);
		return nullptr;
	}
#endif // !_DEBUG

	return ::LoadLibrary(sciLexerPath.c_str());
}

void ScintillaEditView::init(HINSTANCE hInst, HWND hPere)	{

	if (!_hLib)	{

		throw runtime_error("ScintillaEditView::init : SCINTILLA ERROR - Can not load the dynamic library");
	}

	Window::init(hInst, hPere);
	_hSelf = ::CreateWindowEx(
					WS_EX_CLIENTEDGE,\
					L"Scintilla",\
					L"Notepad++",\
					WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_CLIPCHILDREN | WS_EX_RTLREADING,\
					0, 0, 100, 100,\
					_hParent,\
					NULL,\
					_hInst,\
					NULL);

	if (!_hSelf)	{

		throw runtime_error("ScintillaEditView::init : CreateWindowEx() function return null");
	}

	_pScintillaFunc = (SCINTILLA_FUNC)::SendMessage(_hSelf, SCI_GETDIRECTFUNCTION, 0, 0);
	_pScintillaPtr = (SCINTILLA_PTR)::SendMessage(_hSelf, SCI_GETDIRECTPOINTER, 0, 0);

	_userDefineDlg.init(_hInst, _hParent, this);

	if (!_pScintillaFunc)	{

		throw runtime_error("ScintillaEditView::init : SCI_GETDIRECTFUNCTION message failed");
	}

	if (!_pScintillaPtr)	{

		throw runtime_error("ScintillaEditView::init : SCI_GETDIRECTPOINTER message failed");
	}

	f(SCI_SETMARGINMASKN, _SC_MARGE_FOLDER, SC_MASK_FOLDERS);
	showMargin(_SC_MARGE_FOLDER, true);

	f(SCI_SETMARGINMASKN, _SC_MARGE_SYBOLE, (1<<MARK_BOOKMARK) | (1<<MARK_HIDELINESBEGIN) | (1<<MARK_HIDELINESEND) | (1<<MARK_HIDELINESUNDERLINE));

	f(SCI_MARKERSETALPHA, MARK_BOOKMARK, 70);

	f(SCI_MARKERDEFINE, MARK_HIDELINESUNDERLINE, SC_MARK_UNDERLINE);
	f(SCI_MARKERSETBACK, MARK_HIDELINESUNDERLINE, 0x77CC77);

	if (param._dpiManager.scaleX(100) >= 150)	{

		f(SCI_RGBAIMAGESETWIDTH, 18);
		f(SCI_RGBAIMAGESETHEIGHT, 18);
		f(SCI_MARKERDEFINERGBAIMAGE, MARK_BOOKMARK, reinterpret_cast<LPARAM>(bookmark18));
		f(SCI_MARKERDEFINERGBAIMAGE, MARK_HIDELINESBEGIN, reinterpret_cast<LPARAM>(hidelines_begin18));
		f(SCI_MARKERDEFINERGBAIMAGE, MARK_HIDELINESEND, reinterpret_cast<LPARAM>(hidelines_end18));
	}
	else	{

		f(SCI_RGBAIMAGESETWIDTH, 14);
		f(SCI_RGBAIMAGESETHEIGHT, 14);
		f(SCI_MARKERDEFINERGBAIMAGE, MARK_BOOKMARK, reinterpret_cast<LPARAM>(bookmark14));
		f(SCI_MARKERDEFINERGBAIMAGE, MARK_HIDELINESBEGIN, reinterpret_cast<LPARAM>(hidelines_begin14));
		f(SCI_MARKERDEFINERGBAIMAGE, MARK_HIDELINESEND, reinterpret_cast<LPARAM>(hidelines_end14));
	}

	f(SCI_SETMARGINSENSITIVEN, _SC_MARGE_FOLDER, true);
	f(SCI_SETMARGINSENSITIVEN, _SC_MARGE_SYBOLE, true);

	f(SCI_SETFOLDFLAGS, 16);
	f(SCI_SETSCROLLWIDTHTRACKING, true);
	f(SCI_SETSCROLLWIDTH, 1);	//default empty document: override default width of 2000

	// smart hilighting
	f(SCI_INDICSETSTYLE, SCE_UNIVERSAL_FOUND_STYLE_SMART, INDIC_ROUNDBOX);
	f(SCI_INDICSETSTYLE, SCE_UNIVERSAL_FOUND_STYLE, INDIC_ROUNDBOX);
	f(SCI_INDICSETSTYLE, SCE_UNIVERSAL_FOUND_STYLE_INC, INDIC_ROUNDBOX);
	f(SCI_INDICSETSTYLE, SCE_UNIVERSAL_TAGMATCH, INDIC_ROUNDBOX);
	f(SCI_INDICSETSTYLE, SCE_UNIVERSAL_TAGATTR, INDIC_ROUNDBOX);
	f(SCI_INDICSETSTYLE, SCE_UNIVERSAL_FOUND_STYLE_EXT1, INDIC_ROUNDBOX);
	f(SCI_INDICSETSTYLE, SCE_UNIVERSAL_FOUND_STYLE_EXT2, INDIC_ROUNDBOX);
	f(SCI_INDICSETSTYLE, SCE_UNIVERSAL_FOUND_STYLE_EXT3, INDIC_ROUNDBOX);
	f(SCI_INDICSETSTYLE, SCE_UNIVERSAL_FOUND_STYLE_EXT4, INDIC_ROUNDBOX);
	f(SCI_INDICSETSTYLE, SCE_UNIVERSAL_FOUND_STYLE_EXT5, INDIC_ROUNDBOX);

	f(SCI_INDICSETALPHA, SCE_UNIVERSAL_FOUND_STYLE_SMART, 100);
	f(SCI_INDICSETALPHA, SCE_UNIVERSAL_FOUND_STYLE, 100);
	f(SCI_INDICSETALPHA, SCE_UNIVERSAL_FOUND_STYLE_INC, 100);
	f(SCI_INDICSETALPHA, SCE_UNIVERSAL_TAGMATCH, 100);
	f(SCI_INDICSETALPHA, SCE_UNIVERSAL_TAGATTR, 100);
	f(SCI_INDICSETALPHA, SCE_UNIVERSAL_FOUND_STYLE_EXT1, 100);
	f(SCI_INDICSETALPHA, SCE_UNIVERSAL_FOUND_STYLE_EXT2, 100);
	f(SCI_INDICSETALPHA, SCE_UNIVERSAL_FOUND_STYLE_EXT3, 100);
	f(SCI_INDICSETALPHA, SCE_UNIVERSAL_FOUND_STYLE_EXT4, 100);
	f(SCI_INDICSETALPHA, SCE_UNIVERSAL_FOUND_STYLE_EXT5, 100);

	f(SCI_INDICSETUNDER, SCE_UNIVERSAL_FOUND_STYLE_SMART, true);
	f(SCI_INDICSETUNDER, SCE_UNIVERSAL_FOUND_STYLE, true);
	f(SCI_INDICSETUNDER, SCE_UNIVERSAL_FOUND_STYLE_INC, true);
	f(SCI_INDICSETUNDER, SCE_UNIVERSAL_TAGMATCH, true);
	f(SCI_INDICSETUNDER, SCE_UNIVERSAL_TAGATTR, true);
	f(SCI_INDICSETUNDER, SCE_UNIVERSAL_FOUND_STYLE_EXT1, true);
	f(SCI_INDICSETUNDER, SCE_UNIVERSAL_FOUND_STYLE_EXT2, true);
	f(SCI_INDICSETUNDER, SCE_UNIVERSAL_FOUND_STYLE_EXT3, true);
	f(SCI_INDICSETUNDER, SCE_UNIVERSAL_FOUND_STYLE_EXT4, true);
	f(SCI_INDICSETUNDER, SCE_UNIVERSAL_FOUND_STYLE_EXT5, true);

	_codepage = ::GetACP();

	::SetWindowLongPtr(_hSelf, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
	_callWindowProc = CallWindowProc;
	_scintillaDefaultProc = reinterpret_cast<WNDPROC>(::SetWindowLongPtr(_hSelf, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(scintillaStatic_Proc)));

	if (_defaultCharList.empty())	{

		auto defaultCharListLen = f(SCI_GETWORDCHARS);
		char *defaultCharList = new char[defaultCharListLen + 1];
		f(SCI_GETWORDCHARS, 0, reinterpret_cast<LPARAM>(defaultCharList));
		defaultCharList[defaultCharListLen] = '\0';
		_defaultCharList = defaultCharList;
		delete[] defaultCharList;
	}
	//Get the startup document and make a buffer for it so it can be accessed like a file
	attachDefaultDoc();
}

LRESULT CALLBACK ScintillaEditView::scintillaStatic_Proc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)	{

	ScintillaEditView *pScint = (ScintillaEditView *)(::GetWindowLongPtr(hwnd, GWLP_USERDATA));

	if (Message == WM_MOUSEWHEEL || Message == WM_MOUSEHWHEEL)	{

		POINT pt;
		POINTS pts = MAKEPOINTS(lParam);
		POINTSTOPOINT(pt, pts);
		HWND hwndOnMouse = WindowFromPoint(pt);

		//Hack for Synaptics TouchPad Driver
		char synapticsHack[26];
		GetClassNameA(hwndOnMouse, (LPSTR)&synapticsHack, 26);
		bool isSynpnatic = string(synapticsHack) == "SynTrackCursorWindowClass";
		bool makeTouchPadCompetible = (param.getSVP())._disableAdvancedScrolling;

		if (pScint && (isSynpnatic || makeTouchPadCompetible))
			return (pScint->scintillaNew_Proc(hwnd, Message, wParam, lParam));

		ScintillaEditView *pScintillaOnMouse = (ScintillaEditView *)(::GetWindowLongPtr(hwndOnMouse, GWLP_USERDATA));
		if (pScintillaOnMouse != pScint)
			return ::SendMessage(hwndOnMouse, Message, wParam, lParam);
	}
	if (pScint)
		return (pScint->scintillaNew_Proc(hwnd, Message, wParam, lParam));
	else
		return ::DefWindowProc(hwnd, Message, wParam, lParam);

}
LRESULT ScintillaEditView::scintillaNew_Proc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)	{

	switch (Message)	{

		case WM_MOUSEHWHEEL :	{

			::CallWindowProc(_scintillaDefaultProc, hwnd, WM_HSCROLL, ((short)HIWORD(wParam) > 0)?SB_LINERIGHT:SB_LINELEFT, NULL);
			break;
		}

		case WM_MOUSEWHEEL :	{

			if (LOWORD(wParam) & MK_RBUTTON)	{

				::SendMessage(_hParent, Message, wParam, lParam);
				return TRUE;
			}

			if (LOWORD(wParam) & MK_SHIFT)	{

				// move 3 columns at a time
				::CallWindowProc(_scintillaDefaultProc, hwnd, WM_HSCROLL, ((short)HIWORD(wParam) < 0) ? SB_LINERIGHT : SB_LINELEFT, NULL);
				::CallWindowProc(_scintillaDefaultProc, hwnd, WM_HSCROLL, ((short)HIWORD(wParam) < 0) ? SB_LINERIGHT : SB_LINELEFT, NULL);
				::CallWindowProc(_scintillaDefaultProc, hwnd, WM_HSCROLL, ((short)HIWORD(wParam) < 0) ? SB_LINERIGHT : SB_LINELEFT, NULL);
				return TRUE;
			}

			//Have to perform the scroll first, because the first/last line do not get updated untill after the scroll has been parsed
			LRESULT scrollResult = ::CallWindowProc(_scintillaDefaultProc, hwnd, Message, wParam, lParam);
			return scrollResult;
		}

		case WM_IME_REQUEST:	{


			if (wParam == IMR_RECONVERTSTRING)	{

				int					textLength;
				int					selectSize;
				char				smallTextBuffer[128];
				char			  *	selectedStr = smallTextBuffer;
				RECONVERTSTRING   *	reconvert = (RECONVERTSTRING *)lParam;

				// does nothing with a rectangular selection
				if (f(SCI_SELECTIONISRECTANGLE, 0, 0))
					return 0;

				// get the codepage of the text

				UINT codepage = static_cast<UINT>(f(SCI_GETCODEPAGE));

				// get the current text selection

				Sci_CharacterRange range = getSelection();
				if (range.cpMax == range.cpMin)	{

					// no selection: select the current word instead

					expandWordSelection();
					range = getSelection();
				}
				selectSize = range.cpMax - range.cpMin;

				// does nothing if still no luck with the selection

				if (!selectSize)
					return 0;

				if (selectSize + 1 > sizeof(smallTextBuffer))
					selectedStr = new char[selectSize + 1];
				getText(selectedStr, range.cpMin, range.cpMax);

				if (!reconvert)	{

					// convert the selection to Unicode, and get the number
					// of bytes required for the converted text
					textLength = sizeof(WCHAR) * ::MultiByteToWideChar(codepage, 0, selectedStr, selectSize, NULL, 0);
				}
				else	{

					// convert the selection to Unicode, and store it at the end of the structure.
					// Beware: For a Unicode IME, dwStrLen , dwCompStrLen, and dwTargetStrLen
					// are TCHAR values, that is, character counts. The members dwStrOffset,
					// dwCompStrOffset, and dwTargetStrOffset specify byte counts.

					textLength = ::MultiByteToWideChar(	codepage, 0,
														selectedStr, selectSize,
														(LPWSTR)((LPSTR)reconvert + sizeof(RECONVERTSTRING)),
														reconvert->dwSize - sizeof(RECONVERTSTRING));

					// fill the structure
					reconvert->dwVersion		 = 0;
					reconvert->dwStrLen			 = textLength;
					reconvert->dwStrOffset		 = sizeof(RECONVERTSTRING);
					reconvert->dwCompStrLen		 = textLength;
					reconvert->dwCompStrOffset	 = 0;
					reconvert->dwTargetStrLen	 = reconvert->dwCompStrLen;
					reconvert->dwTargetStrOffset = reconvert->dwCompStrOffset;

					textLength *= sizeof(WCHAR);
				}

				if (selectedStr != smallTextBuffer)
					delete [] selectedStr;

				// return the total length of the structure
				return sizeof(RECONVERTSTRING) + textLength;
			}
			break;
		}

		case WM_KEYUP :	{

			if (wParam == VK_PRIOR || wParam == VK_NEXT)	{

				// find hotspots
				SCNotification notification = {};
				notification.nmhdr.code = SCN_PAINTED;
				notification.nmhdr.hwndFrom = _hSelf;
				notification.nmhdr.idFrom = ::GetDlgCtrlID(_hSelf);
				::SendMessage(_hParent, WM_NOTIFY, LINKTRIGGERED, reinterpret_cast<LPARAM>(&notification));

			}
			break;
		}

		case WM_VSCROLL :	{

			break;
		}
	}
	return _callWindowProc(_scintillaDefaultProc, hwnd, Message, wParam, lParam);
}

#define DEFAULT_FONT_NAME "Courier New"

void ScintillaEditView::setSpecialStyle(const Style & styleToSet)	{

	int styleID = styleToSet._styleID;
	if ( styleToSet._colorStyle & COLORSTYLE_FOREGROUND )
		f(SCI_STYLESETFORE, styleID, styleToSet._fgColor);

	if ( styleToSet._colorStyle & COLORSTYLE_BACKGROUND )
		f(SCI_STYLESETBACK, styleID, styleToSet._bgColor);

	if (styleToSet._fontName && lstrcmp(styleToSet._fontName, L""))	{

		
		if (!param.isInFontList(styleToSet._fontName))	{

			f(SCI_STYLESETFONT, styleID, reinterpret_cast<LPARAM>(DEFAULT_FONT_NAME));
		}
		else	{

			const char * fontNameA = wmc.wchar2char(styleToSet._fontName, CP_UTF8);
			f(SCI_STYLESETFONT, styleID, reinterpret_cast<LPARAM>(fontNameA));
		}
	}
	int fontStyle = styleToSet._fontStyle;
	if (fontStyle != STYLE_NOT_USED)	{

		f(SCI_STYLESETBOLD,		styleID, fontStyle & FONTSTYLE_BOLD);
		f(SCI_STYLESETITALIC,		styleID, fontStyle & FONTSTYLE_ITALIC);
		f(SCI_STYLESETUNDERLINE,	styleID, fontStyle & FONTSTYLE_UNDERLINE);
	}

	if (styleToSet._fontSize > 0)
		f(SCI_STYLESETSIZE, styleID, styleToSet._fontSize);
}

void ScintillaEditView::setHotspotStyle(Style& styleToSet)	{

	StyleMap* styleMap;
	if ( _hotspotStyles.find(_currentBuffer) == _hotspotStyles.end() )	{

		_hotspotStyles[_currentBuffer] = new StyleMap;
	}
	styleMap = _hotspotStyles[_currentBuffer];
	(*styleMap)[styleToSet._styleID] = styleToSet;

	setStyle(styleToSet);
}

void ScintillaEditView::setStyle(Style styleToSet)	{

	GlobalOverride & go = param.getGlobalOverrideStyle();

	if (go.isEnable())	{

		StyleArray & stylers = param.getMiscStylerArray();
		int i = stylers.getStylerIndexByName(L"Global override");
		if (i != -1)	{

			Style & style = stylers.getStyler(i);

			if (go.enableFg)	{

				if (style._colorStyle & COLORSTYLE_FOREGROUND)	{

					styleToSet._colorStyle |= COLORSTYLE_FOREGROUND;
					styleToSet._fgColor = style._fgColor;
				}
				else	{

					if (styleToSet._styleID == STYLE_DEFAULT) //if global is set to transparent, use default style color
						styleToSet._colorStyle |= COLORSTYLE_FOREGROUND;
					else
						styleToSet._colorStyle &= ~COLORSTYLE_FOREGROUND;
				}
			}

			if (go.enableBg)	{

				if (style._colorStyle & COLORSTYLE_BACKGROUND)	{

					styleToSet._colorStyle |= COLORSTYLE_BACKGROUND;
					styleToSet._bgColor = style._bgColor;
				}
				else	{

					if (styleToSet._styleID == STYLE_DEFAULT) 	//if global is set to transparent, use default style color
						styleToSet._colorStyle |= COLORSTYLE_BACKGROUND;
					else
						styleToSet._colorStyle &= ~COLORSTYLE_BACKGROUND;
				}
			}
			if (go.enableFont && style._fontName && style._fontName[0])
				styleToSet._fontName = style._fontName;
			if (go.enableFontSize && (style._fontSize > 0))
				styleToSet._fontSize = style._fontSize;

			if (style._fontStyle != STYLE_NOT_USED)	{

				if (go.enableBold)	{

					if (style._fontStyle & FONTSTYLE_BOLD)
						styleToSet._fontStyle |= FONTSTYLE_BOLD;
					else
						styleToSet._fontStyle &= ~FONTSTYLE_BOLD;
				}
				if (go.enableItalic)	{

					if (style._fontStyle & FONTSTYLE_ITALIC)
						styleToSet._fontStyle |= FONTSTYLE_ITALIC;
					else
						styleToSet._fontStyle &= ~FONTSTYLE_ITALIC;
				}
				if (go.enableUnderLine)	{

					if (style._fontStyle & FONTSTYLE_UNDERLINE)
						styleToSet._fontStyle |= FONTSTYLE_UNDERLINE;
					else
						styleToSet._fontStyle &= ~FONTSTYLE_UNDERLINE;
				}
			}
		}
	}
	setSpecialStyle(styleToSet);
}


void ScintillaEditView::setXmlLexer(LangType type)	{

	if (type == L_XML)	{

		f(SCI_SETLEXER, SCLEX_XML);
		for (int i = 0 ; i < 4 ; ++i)
			f(SCI_SETKEYWORDS, i, reinterpret_cast<LPARAM>(L""));

		makeStyle(type);

		f(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("lexer.xml.allow.scripts"), reinterpret_cast<LPARAM>("0"));
	}
	else if ((type == L_HTML) || (type == L_PHP) || (type == L_ASP) || (type == L_JSP))	{

		f(SCI_SETLEXER, SCLEX_HTML);
		const TCHAR *htmlKeyWords_generic = param.getWordList(L_HTML, LANG_INDEX_INSTR);

				const char *htmlKeyWords = wmc.wchar2char(htmlKeyWords_generic, CP_ACP);
		f(SCI_SETKEYWORDS, 0, reinterpret_cast<LPARAM>(htmlKeyWords?htmlKeyWords:""));
		makeStyle(L_HTML);

		setEmbeddedJSLexer();
		setEmbeddedPhpLexer();
		setEmbeddedAspLexer();
	}
	f(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold"), reinterpret_cast<LPARAM>("1"));
	f(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.compact"), reinterpret_cast<LPARAM>("0"));
	f(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.html"), reinterpret_cast<LPARAM>("1"));
	// This allow to fold comment strem in php/javascript code
	f(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.hypertext.comment"), reinterpret_cast<LPARAM>("1"));
}

void ScintillaEditView::setEmbeddedJSLexer()	{

	const TCHAR *pKwArray[10] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
	makeStyle(L_JS, pKwArray);

	basic_string<char> keywordList("");
	if (pKwArray[LANG_INDEX_INSTR])	{

		basic_string<wchar_t> kwlW = pKwArray[LANG_INDEX_INSTR];
		keywordList = wstring2string(kwlW, CP_ACP);
	}

	f(SCI_SETKEYWORDS, 1, reinterpret_cast<LPARAM>(getCompleteKeywordList(keywordList, L_JS, LANG_INDEX_INSTR)));
	f(SCI_STYLESETEOLFILLED, SCE_HJ_DEFAULT, true);
	f(SCI_STYLESETEOLFILLED, SCE_HJ_COMMENT, true);
	f(SCI_STYLESETEOLFILLED, SCE_HJ_COMMENTDOC, true);
}

void ScintillaEditView::setJsonLexer()	{

	f(SCI_SETLEXER, SCLEX_JSON);

	const TCHAR *pKwArray[10] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };

	makeStyle(L_JSON, pKwArray);

	string keywordList;
	string keywordList2;
	if (pKwArray[LANG_INDEX_INSTR])	{

		wstring kwlW = pKwArray[LANG_INDEX_INSTR];
		keywordList = wstring2string(kwlW, CP_ACP);
	}

	if (pKwArray[LANG_INDEX_INSTR2])	{

		wstring kwlW = pKwArray[LANG_INDEX_INSTR2];
		keywordList2 = wstring2string(kwlW, CP_ACP);
	}

	f(SCI_SETKEYWORDS, 0, reinterpret_cast<LPARAM>(getCompleteKeywordList(keywordList, L_JSON, LANG_INDEX_INSTR)));
	f(SCI_SETKEYWORDS, 1, reinterpret_cast<LPARAM>(getCompleteKeywordList(keywordList2, L_JSON, LANG_INDEX_INSTR2)));

	f(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold"), reinterpret_cast<LPARAM>("1"));
	f(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.compact"), reinterpret_cast<LPARAM>("0"));

	f(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.comment"), reinterpret_cast<LPARAM>("1"));
	f(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.preprocessor"), reinterpret_cast<LPARAM>("1"));
}

void ScintillaEditView::setEmbeddedPhpLexer()	{

	const TCHAR *pKwArray[10] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
	makeStyle(L_PHP, pKwArray);

	basic_string<char> keywordList("");
	if (pKwArray[LANG_INDEX_INSTR])	{

		basic_string<wchar_t> kwlW = pKwArray[LANG_INDEX_INSTR];
		keywordList = wstring2string(kwlW, CP_ACP);
	}

	f(SCI_SETKEYWORDS, 4, reinterpret_cast<LPARAM>(getCompleteKeywordList(keywordList, L_PHP, LANG_INDEX_INSTR)));

	f(SCI_STYLESETEOLFILLED, SCE_HPHP_DEFAULT, true);
	f(SCI_STYLESETEOLFILLED, SCE_HPHP_COMMENT, true);
}

void ScintillaEditView::setEmbeddedAspLexer()	{

	const TCHAR *pKwArray[10] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
	makeStyle(L_ASP, pKwArray);

	basic_string<char> keywordList("");
	if (pKwArray[LANG_INDEX_INSTR])	{

		basic_string<wchar_t> kwlW = pKwArray[LANG_INDEX_INSTR];
		keywordList = wstring2string(kwlW, CP_ACP);
	}

	f(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("asp.default.language"), reinterpret_cast<LPARAM>("2"));

	f(SCI_SETKEYWORDS, 2, reinterpret_cast<LPARAM>(getCompleteKeywordList(keywordList, L_VB, LANG_INDEX_INSTR)));

	f(SCI_STYLESETEOLFILLED, SCE_HBA_DEFAULT, true);
}

void ScintillaEditView::setUserLexer(const TCHAR *userLangName)	{

	int setKeywordsCounter = 0;
	f(SCI_SETLEXER, SCLEX_USER);

	UserLangContainer * userLangContainer = userLangName? param.getULCFromName(userLangName):_userDefineDlg._pCurrentUserLang;

	if (!userLangContainer)
		return;

	UINT codepage = CP_ACP;
	UniMode unicodeMode = _currentBuffer->getUnicodeMode();
	int encoding = _currentBuffer->getEncoding();
	if (encoding == -1)	{

		if (unicodeMode == uniUTF8 || unicodeMode == uniCookie)
			codepage = CP_UTF8;
	}
	else	{

		codepage = CP_OEMCP;	// system OEM code page might not match user selection for character set,
								// but this is the best match WideCharToMultiByte offers
	}

	f(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold"), reinterpret_cast<LPARAM>("1"));
	f(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("userDefine.isCaseIgnored"),		  reinterpret_cast<LPARAM>(userLangContainer->_isCaseIgnored ? "1":"0"));
	f(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("userDefine.allowFoldOfComments"),  reinterpret_cast<LPARAM>(userLangContainer->_allowFoldOfComments ? "1":"0"));
	f(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("userDefine.foldCompact"),		  reinterpret_cast<LPARAM>(userLangContainer->_foldCompact ? "1":"0"));

	char name[] = "userDefine.prefixKeywords0";
	for (int i=0 ; i<SCE_USER_TOTAL_KEYWORD_GROUPS ; ++i)	{

		itoa(i+1, (name+25), 10);
		f(SCI_SETPROPERTY, reinterpret_cast<WPARAM>(name), reinterpret_cast<LPARAM>(userLangContainer->_isPrefix[i] ? "1" : "0"));
	}

	for (int i = 0 ; i < SCE_USER_KWLIST_TOTAL ; ++i)	{

				const char * keyWords_char = wmc.wchar2char(userLangContainer->_keywordLists[i], codepage);

		if (globalMappper().setLexerMapper.find(i) != globalMappper().setLexerMapper.end())	{

			f(SCI_SETPROPERTY, reinterpret_cast<WPARAM>(globalMappper().setLexerMapper[i].c_str()), reinterpret_cast<LPARAM>(keyWords_char));
		}
		else	{ // OPERATORS2, FOLDERS_IN_CODE2, FOLDERS_IN_COMMENT, KEYWORDS1-8

			char temp[max_char];
			bool inDoubleQuote = false;
			bool inSingleQuote = false;
			bool nonWSFound = false;
			int index = 0;
			for (size_t j=0, len = strlen(keyWords_char); j<len && index < (max_char-1); ++j)	{

				if (!inSingleQuote && keyWords_char[j] == '"')	{

					inDoubleQuote = !inDoubleQuote;
					continue;
				}

				if (!inDoubleQuote && keyWords_char[j] == '\'')	{

					inSingleQuote = !inSingleQuote;
					continue;
				}

				if (keyWords_char[j] == '\\' && (keyWords_char[j+1] == '"' || keyWords_char[j+1] == '\'' || keyWords_char[j+1] == '\\'))	{

					++j;
					temp[index++] = keyWords_char[j];
					continue;
				}

				if (inDoubleQuote || inSingleQuote)	{

					if (keyWords_char[j] > ' ')	{		// copy non-whitespace unconditionally

						temp[index++] = keyWords_char[j];
						if (nonWSFound == false)
							nonWSFound = true;
					}
					else if (nonWSFound == true && keyWords_char[j-1] != '"' && keyWords_char[j+1] != '"' && keyWords_char[j+1] > ' ')	{

						temp[index++] = inDoubleQuote ? '\v' : '\b';
					}
					else
						continue;
				}
				else	{

					temp[index++] = keyWords_char[j];
				}

			}
			temp[index++] = 0;
			f(SCI_SETKEYWORDS, setKeywordsCounter++, reinterpret_cast<LPARAM>(temp));
		}
	}

 	char intBuffer[32];

	sprintf(intBuffer, "%d", userLangContainer->_forcePureLC);
	f(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("userDefine.forcePureLC"), reinterpret_cast<LPARAM>(intBuffer));

	sprintf(intBuffer, "%d", userLangContainer->_decimalSeparator);
	f(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("userDefine.decimalSeparator"), reinterpret_cast<LPARAM>(intBuffer));

	// at the end (position SCE_USER_KWLIST_TOTAL) send id values
	sprintf(intBuffer, "%" PRIuPTR, reinterpret_cast<uintptr_t>(userLangContainer->getName())); // use numeric value of TCHAR pointer
	f(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("userDefine.udlName"), reinterpret_cast<LPARAM>(intBuffer));

	sprintf(intBuffer, "%" PRIuPTR, reinterpret_cast<uintptr_t>(_currentBufferID)); // use numeric value of BufferID pointer
	f(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("userDefine.currentBufferID"), reinterpret_cast<LPARAM>(intBuffer));

	for (int i = 0 ; i < SCE_USER_STYLE_TOTAL_STYLES ; ++i)	{

		Style & style = userLangContainer->_styleArray.getStyler(i);

		if (style._styleID == STYLE_NOT_USED)
			continue;

		char nestingBuffer[32];
		sprintf(nestingBuffer, "userDefine.nesting.%02d", i );
		sprintf(intBuffer, "%d", style._nesting);
		f(SCI_SETPROPERTY, reinterpret_cast<WPARAM>(nestingBuffer), reinterpret_cast<LPARAM>(intBuffer));

		setStyle(style);
	}
}

void ScintillaEditView::setExternalLexer(LangType typeDoc)	{

	int id = typeDoc - L_EXTERNAL;
	TCHAR * name = param.getELCFromIndex(id)._name;

		const char *pName = wmc.wchar2char(name, CP_ACP);

	f(SCI_SETLEXERLANGUAGE, 0, reinterpret_cast<LPARAM>(pName));

	LexerStyler *pStyler = (param.getLStylerArray()).getLexerStylerByName(name);
	if (pStyler)	{

		for (int i = 0 ; i < pStyler->getNbStyler() ; ++i)	{

			Style & style = pStyler->getStyler(i);

			setStyle(style);

			if (style._keywordClass >= 0 && style._keywordClass <= KEYWORDSET_MAX)	{

				basic_string<char> keywordList("");
				if (style._keywords)	{

					keywordList = wstring2string(*(style._keywords), CP_ACP);
				}
				f(SCI_SETKEYWORDS, style._keywordClass, reinterpret_cast<LPARAM>(getCompleteKeywordList(keywordList, typeDoc, style._keywordClass)));
			}
		}
	}
}

void ScintillaEditView::setCppLexer(LangType langType)	{

	const char *cppInstrs;
	const char *cppTypes;
	const TCHAR *doxygenKeyWords  = param.getWordList(L_CPP, LANG_INDEX_TYPE2);

	f(SCI_SETLEXER, SCLEX_CPP);

	if (langType != L_RC)	{

		if (doxygenKeyWords)	{

						const char * doxygenKeyWords_char = wmc.wchar2char(doxygenKeyWords, CP_ACP);
			f(SCI_SETKEYWORDS, 2, reinterpret_cast<LPARAM>(doxygenKeyWords_char));
		}
	}

	const TCHAR *pKwArray[10] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
	makeStyle(langType, pKwArray);

	basic_string<char> keywordListInstruction("");
	basic_string<char> keywordListType("");
	if (pKwArray[LANG_INDEX_INSTR])	{

		basic_string<wchar_t> kwlW = pKwArray[LANG_INDEX_INSTR];
		keywordListInstruction = wstring2string(kwlW, CP_ACP);
	}
	cppInstrs = getCompleteKeywordList(keywordListInstruction, langType, LANG_INDEX_INSTR);

	if (pKwArray[LANG_INDEX_TYPE])	{

		basic_string<wchar_t> kwlW = pKwArray[LANG_INDEX_TYPE];
		keywordListType = wstring2string(kwlW, CP_ACP);
	}
	cppTypes = getCompleteKeywordList(keywordListType, langType, LANG_INDEX_TYPE);

	f(SCI_SETKEYWORDS, 0, reinterpret_cast<LPARAM>(cppInstrs));
	f(SCI_SETKEYWORDS, 1, reinterpret_cast<LPARAM>(cppTypes));

	f(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold"), reinterpret_cast<LPARAM>("1"));
	f(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.compact"), reinterpret_cast<LPARAM>("0"));

	f(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.comment"), reinterpret_cast<LPARAM>("1"));
	f(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.preprocessor"), reinterpret_cast<LPARAM>("1"));

	// Disable track preprocessor to avoid incorrect detection.
	// In the most of cases, the symbols are defined outside of file.
	f(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("lexer.cpp.track.preprocessor"), reinterpret_cast<LPARAM>("0"));
}

void ScintillaEditView::setJsLexer()	{

	const TCHAR *doxygenKeyWords = param.getWordList(L_CPP, LANG_INDEX_TYPE2);

	f(SCI_SETLEXER, SCLEX_CPP);
	const TCHAR *pKwArray[10] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
	makeStyle(L_JAVASCRIPT, pKwArray);

	if (doxygenKeyWords)	{

				const char * doxygenKeyWords_char = wmc.wchar2char(doxygenKeyWords, CP_ACP);
		f(SCI_SETKEYWORDS, 2, reinterpret_cast<LPARAM>(doxygenKeyWords_char));
	}

	const TCHAR *newLexerName = ScintillaEditView::langNames[L_JAVASCRIPT].lexerName;
	LexerStyler *pNewStyler = (param.getLStylerArray()).getLexerStylerByName(newLexerName);
	if (pNewStyler)	{ // New js styler is available, so we can use it do more modern styling

		for (int i = 0, nb = pNewStyler->getNbStyler(); i < nb; ++i)	{

			Style & style = pNewStyler->getStyler(i);
			setStyle(style);
		}

		basic_string<char> keywordListInstruction("");
		basic_string<char> keywordListType("");
		basic_string<char> keywordListInstruction2("");

		if (pKwArray[LANG_INDEX_INSTR])	{

			basic_string<wchar_t> kwlW = pKwArray[LANG_INDEX_INSTR];
			keywordListInstruction = wstring2string(kwlW, CP_ACP);
		}
		const char *jsInstrs = getCompleteKeywordList(keywordListInstruction, L_JAVASCRIPT, LANG_INDEX_INSTR);

		if (pKwArray[LANG_INDEX_TYPE])	{

			basic_string<wchar_t> kwlW = pKwArray[LANG_INDEX_TYPE];
			keywordListType = wstring2string(kwlW, CP_ACP);
		}
		const char *jsTypes = getCompleteKeywordList(keywordListType, L_JAVASCRIPT, LANG_INDEX_TYPE);

		if (pKwArray[LANG_INDEX_INSTR2])	{

			basic_string<wchar_t> kwlW = pKwArray[LANG_INDEX_INSTR2];
			keywordListInstruction2 = wstring2string(kwlW, CP_ACP);
		}
		const char *jsInstrs2 = getCompleteKeywordList(keywordListInstruction2, L_JAVASCRIPT, LANG_INDEX_INSTR2);

		f(SCI_SETKEYWORDS, 0, reinterpret_cast<LPARAM>(jsInstrs));
		f(SCI_SETKEYWORDS, 1, reinterpret_cast<LPARAM>(jsTypes));
		f(SCI_SETKEYWORDS, 3, reinterpret_cast<LPARAM>(jsInstrs2));
	}
	else	{ // New js styler is not available, we use the old styling for the sake of retro-compatibility

		const TCHAR *lexerName = ScintillaEditView::langNames[L_JS].lexerName;
		LexerStyler *pOldStyler = (param.getLStylerArray()).getLexerStylerByName(lexerName);

		if (pOldStyler)	{

			for (int i = 0, nb = pOldStyler->getNbStyler(); i < nb; ++i)	{

				Style style = pOldStyler->getStyler(i);	//not by reference, but copy
				int cppID = style._styleID;

				switch (style._styleID)	{

					case SCE_HJ_DEFAULT: cppID = SCE_C_DEFAULT; break;
					case SCE_HJ_WORD: cppID = SCE_C_IDENTIFIER; break;
					case SCE_HJ_SYMBOLS: cppID = SCE_C_OPERATOR; break;
					case SCE_HJ_COMMENT: cppID = SCE_C_COMMENT; break;
					case SCE_HJ_COMMENTLINE: cppID = SCE_C_COMMENTLINE; break;
					case SCE_HJ_COMMENTDOC: cppID = SCE_C_COMMENTDOC; break;
					case SCE_HJ_NUMBER: cppID = SCE_C_NUMBER; break;
					case SCE_HJ_KEYWORD: cppID = SCE_C_WORD; break;
					case SCE_HJ_DOUBLESTRING: cppID = SCE_C_STRING; break;
					case SCE_HJ_SINGLESTRING: cppID = SCE_C_CHARACTER; break;
					case SCE_HJ_REGEX: cppID = SCE_C_REGEX; break;
				}
				style._styleID = cppID;
				setStyle(style);
			}
		}
		f(SCI_STYLESETEOLFILLED, SCE_C_DEFAULT, true);
		f(SCI_STYLESETEOLFILLED, SCE_C_COMMENTLINE, true);
		f(SCI_STYLESETEOLFILLED, SCE_C_COMMENT, true);
		f(SCI_STYLESETEOLFILLED, SCE_C_COMMENTDOC, true);

		makeStyle(L_JS, pKwArray);

		basic_string<char> keywordListInstruction("");
		if (pKwArray[LANG_INDEX_INSTR])	{

			basic_string<wchar_t> kwlW = pKwArray[LANG_INDEX_INSTR];
			keywordListInstruction = wstring2string(kwlW, CP_ACP);
		}
		const char *jsEmbeddedInstrs = getCompleteKeywordList(keywordListInstruction, L_JS, LANG_INDEX_INSTR);
		f(SCI_SETKEYWORDS, 0, reinterpret_cast<LPARAM>(jsEmbeddedInstrs));
	}

	f(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold"), reinterpret_cast<LPARAM>("1"));
	f(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.compact"), reinterpret_cast<LPARAM>("0"));

	f(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.comment"), reinterpret_cast<LPARAM>("1"));
	f(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.preprocessor"), reinterpret_cast<LPARAM>("1"));

	// Disable track preprocessor to avoid incorrect detection.
	// In the most of cases, the symbols are defined outside of file.
	f(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("lexer.cpp.track.preprocessor"), reinterpret_cast<LPARAM>("0"));
	f(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("lexer.cpp.backquoted.strings"), reinterpret_cast<LPARAM>("1"));
}

void ScintillaEditView::setTclLexer()	{

	const char *tclInstrs;
	const char *tclTypes;


	f(SCI_SETLEXER, SCLEX_TCL);

	const TCHAR *pKwArray[10] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
	makeStyle(L_TCL, pKwArray);

	basic_string<char> keywordListInstruction("");
	basic_string<char> keywordListType("");
	if (pKwArray[LANG_INDEX_INSTR])	{

		basic_string<wchar_t> kwlW = pKwArray[LANG_INDEX_INSTR];
		keywordListInstruction = wstring2string(kwlW, CP_ACP);
	}
	tclInstrs = getCompleteKeywordList(keywordListInstruction, L_TCL, LANG_INDEX_INSTR);

	if (pKwArray[LANG_INDEX_TYPE])	{

		basic_string<wchar_t> kwlW = pKwArray[LANG_INDEX_TYPE];
		keywordListType = wstring2string(kwlW, CP_ACP);
	}
	tclTypes = getCompleteKeywordList(keywordListType, L_TCL, LANG_INDEX_TYPE);

	f(SCI_SETKEYWORDS, 0, reinterpret_cast<LPARAM>(tclInstrs));
	f(SCI_SETKEYWORDS, 1, reinterpret_cast<LPARAM>(tclTypes));
}

void ScintillaEditView::setObjCLexer(LangType langType)	{

	f(SCI_SETLEXER, SCLEX_OBJC);

	const TCHAR *pKwArray[10] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

	makeStyle(langType, pKwArray);

	basic_string<char> objcInstr1Kwl("");
	if (pKwArray[LANG_INDEX_INSTR])	{

		objcInstr1Kwl = wstring2string(pKwArray[LANG_INDEX_INSTR], CP_ACP);
	}
	const char *objcInstrs = getCompleteKeywordList(objcInstr1Kwl, langType, LANG_INDEX_INSTR);

	basic_string<char> objcInstr2Kwl("");
	if (pKwArray[LANG_INDEX_INSTR2])	{

		objcInstr2Kwl = wstring2string(pKwArray[LANG_INDEX_INSTR2], CP_ACP);
	}
	const char *objCDirective = getCompleteKeywordList(objcInstr2Kwl, langType, LANG_INDEX_INSTR2);

	basic_string<char> objcTypeKwl("");
	if (pKwArray[LANG_INDEX_TYPE])	{

		objcTypeKwl = wstring2string(pKwArray[LANG_INDEX_TYPE], CP_ACP);
	}
	const char *objcTypes = getCompleteKeywordList(objcTypeKwl, langType, LANG_INDEX_TYPE);


	basic_string<char> objcType2Kwl("");
	if (pKwArray[LANG_INDEX_TYPE2])	{

		objcType2Kwl = wstring2string(pKwArray[LANG_INDEX_TYPE2], CP_ACP);
	}
	const char *objCQualifier = getCompleteKeywordList(objcType2Kwl, langType, LANG_INDEX_TYPE2);



	basic_string<char> doxygenKeyWordsString("");
	const TCHAR *doxygenKeyWordsW = param.getWordList(L_CPP, LANG_INDEX_TYPE2);
	if (doxygenKeyWordsW)	{

		doxygenKeyWordsString = wstring2string(doxygenKeyWordsW, CP_ACP);
	}
	const char *doxygenKeyWords = doxygenKeyWordsString.c_str();

	f(SCI_SETKEYWORDS, 0, reinterpret_cast<LPARAM>(objcInstrs));
	f(SCI_SETKEYWORDS, 1, reinterpret_cast<LPARAM>(objcTypes));
	f(SCI_SETKEYWORDS, 2, reinterpret_cast<LPARAM>(doxygenKeyWords));
	f(SCI_SETKEYWORDS, 3, reinterpret_cast<LPARAM>(objCDirective));
	f(SCI_SETKEYWORDS, 4, reinterpret_cast<LPARAM>(objCQualifier));

	f(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold"), reinterpret_cast<LPARAM>("1"));
	f(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.compact"), reinterpret_cast<LPARAM>("0"));

	f(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.comment"), reinterpret_cast<LPARAM>("1"));
	f(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.preprocessor"), reinterpret_cast<LPARAM>("1"));
}

void ScintillaEditView::setKeywords(LangType langType, const char *keywords, int index)	{

	basic_string<char> wordList;
	wordList = (keywords)?keywords:"";
	f(SCI_SETKEYWORDS, index, reinterpret_cast<LPARAM>(getCompleteKeywordList(wordList, langType, index)));
}

void ScintillaEditView::setLexer(int lexerID, LangType langType, int whichList)	{

	f(SCI_SETLEXER, lexerID);

	const TCHAR *pKwArray[10] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

	makeStyle(langType, pKwArray);

	
	if (whichList & LIST_0)	{

		const char * keyWords_char = wmc.wchar2char(pKwArray[LANG_INDEX_INSTR], CP_ACP);
		setKeywords(langType, keyWords_char, LANG_INDEX_INSTR);
	}

	if (whichList & LIST_1)	{

		const char * keyWords_char = wmc.wchar2char(pKwArray[LANG_INDEX_INSTR2], CP_ACP);
		setKeywords(langType, keyWords_char, LANG_INDEX_INSTR2);
	}

	if (whichList & LIST_2)	{

		const char * keyWords_char = wmc.wchar2char(pKwArray[LANG_INDEX_TYPE], CP_ACP);
		setKeywords(langType, keyWords_char, LANG_INDEX_TYPE);
	}

	if (whichList & LIST_3)	{

		const char * keyWords_char = wmc.wchar2char(pKwArray[LANG_INDEX_TYPE2], CP_ACP);
		setKeywords(langType, keyWords_char, LANG_INDEX_TYPE2);
	}

	if (whichList & LIST_4)	{

		const char * keyWords_char = wmc.wchar2char(pKwArray[LANG_INDEX_TYPE3], CP_ACP);
		setKeywords(langType, keyWords_char, LANG_INDEX_TYPE3);
	}

	if (whichList & LIST_5)	{

		const char * keyWords_char = wmc.wchar2char(pKwArray[LANG_INDEX_TYPE4], CP_ACP);
		setKeywords(langType, keyWords_char, LANG_INDEX_TYPE4);
	}

	if (whichList & LIST_6)	{

		const char * keyWords_char = wmc.wchar2char(pKwArray[LANG_INDEX_TYPE5], CP_ACP);
		setKeywords(langType, keyWords_char, LANG_INDEX_TYPE5);
	}

	if (whichList & LIST_7)	{

		const char * keyWords_char = wmc.wchar2char(pKwArray[LANG_INDEX_TYPE6], CP_ACP);
		setKeywords(langType, keyWords_char, LANG_INDEX_TYPE6);
	}

	if (whichList & LIST_8)	{

		const char * keyWords_char = wmc.wchar2char(pKwArray[LANG_INDEX_TYPE7], CP_ACP);
		setKeywords(langType, keyWords_char, LANG_INDEX_TYPE7);
	}

	f(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold"), reinterpret_cast<LPARAM>("1"));
	f(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.compact"), reinterpret_cast<LPARAM>("0"));
	f(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.comment"), reinterpret_cast<LPARAM>("1"));
}

void ScintillaEditView::makeStyle(LangType language, const TCHAR **keywordArray)	{

	const TCHAR * lexerName = ScintillaEditView::langNames[language].lexerName;
	LexerStyler *pStyler = (param.getLStylerArray()).getLexerStylerByName(lexerName);
	if (pStyler)	{

		for (int i = 0, nb = pStyler->getNbStyler(); i < nb ; ++i)	{

			Style & style = pStyler->getStyler(i);
			setStyle(style);
			if (keywordArray)	{

				if ((style._keywordClass != STYLE_NOT_USED) && (style._keywords))
					keywordArray[style._keywordClass] = style._keywords->c_str();
			}
		}
	}
}

void ScintillaEditView::restoreDefaultWordChars()	{

	f(SCI_SETWORDCHARS, 0, reinterpret_cast<LPARAM>(_defaultCharList.c_str()));
}

void ScintillaEditView::addCustomWordChars()	{

	if (nGUI._customWordChars.empty())
		return;

	string chars2addStr;
	for (size_t i = 0; i < nGUI._customWordChars.length(); ++i)	{

		bool found = false;
		char char2check = nGUI._customWordChars[i];
		for (size_t j = 0; j < _defaultCharList.length(); ++j)	{

			char wordChar = _defaultCharList[j];
			if (char2check == wordChar)	{

				found = true;
				break;
			}
		}
		if (not found)	{

			chars2addStr.push_back(char2check);
		}
	}

	if (not chars2addStr.empty())	{

		string newCharList = _defaultCharList;
		newCharList += chars2addStr;
		f(SCI_SETWORDCHARS, 0, reinterpret_cast<LPARAM>(newCharList.c_str()));
	}
}

void ScintillaEditView::setWordChars()	{

	if (nGUI._isWordCharDefault)
		restoreDefaultWordChars();
	else
		addCustomWordChars();
}

void ScintillaEditView::defineDocType(LangType typeDoc)	{

	StyleArray & stylers = param.getMiscStylerArray();
	int iStyleDefault = stylers.getStylerIndexByID(STYLE_DEFAULT);
	if (iStyleDefault != -1)	{

		Style & styleDefault = stylers.getStyler(iStyleDefault);
		styleDefault._colorStyle = COLORSTYLE_ALL;	//override transparency
		setStyle(styleDefault);
	}

	f(SCI_STYLECLEARALL);

	Style *pStyle;
	Style defaultIndicatorStyle;

	defaultIndicatorStyle._styleID = SCE_UNIVERSAL_FOUND_STYLE;
	defaultIndicatorStyle._bgColor = red;
	pStyle = &defaultIndicatorStyle;
	int iFind = stylers.getStylerIndexByID(SCE_UNIVERSAL_FOUND_STYLE);
	if (iFind != -1)	{

		pStyle = &(stylers.getStyler(iFind));
	}
	setSpecialIndicator(*pStyle);

	defaultIndicatorStyle._styleID = SCE_UNIVERSAL_FOUND_STYLE_SMART;
	defaultIndicatorStyle._bgColor = liteGreen;
	pStyle = &defaultIndicatorStyle;
	iFind = stylers.getStylerIndexByID(SCE_UNIVERSAL_FOUND_STYLE_SMART);
	if (iFind != -1)	{

		pStyle = &(stylers.getStyler(iFind));
	}
	setSpecialIndicator(*pStyle);

	defaultIndicatorStyle._styleID = SCE_UNIVERSAL_FOUND_STYLE_INC;
	defaultIndicatorStyle._bgColor = blue;
	pStyle = &defaultIndicatorStyle;
	iFind = stylers.getStylerIndexByID(SCE_UNIVERSAL_FOUND_STYLE_INC);
	if (iFind != -1)	{

		pStyle = &(stylers.getStyler(iFind));
	}
	setSpecialIndicator(*pStyle);

	defaultIndicatorStyle._styleID = SCE_UNIVERSAL_TAGMATCH;
	defaultIndicatorStyle._bgColor = RGB(0x80, 0x00, 0xFF);
	pStyle = &defaultIndicatorStyle;
	iFind = stylers.getStylerIndexByID(SCE_UNIVERSAL_TAGMATCH);
	if (iFind != -1)	{

		pStyle = &(stylers.getStyler(iFind));
	}
	setSpecialIndicator(*pStyle);

	defaultIndicatorStyle._styleID = SCE_UNIVERSAL_TAGATTR;
	defaultIndicatorStyle._bgColor = yellow;
	pStyle = &defaultIndicatorStyle;
	iFind = stylers.getStylerIndexByID(SCE_UNIVERSAL_TAGATTR);
	if (iFind != -1)	{

		pStyle = &(stylers.getStyler(iFind));
	}
	setSpecialIndicator(*pStyle);


	defaultIndicatorStyle._styleID = SCE_UNIVERSAL_FOUND_STYLE_EXT1;
	defaultIndicatorStyle._bgColor = cyan;
	pStyle = &defaultIndicatorStyle;
	iFind = stylers.getStylerIndexByID(SCE_UNIVERSAL_FOUND_STYLE_EXT1);
	if (iFind != -1)	{

		pStyle = &(stylers.getStyler(iFind));
	}
	setSpecialIndicator(*pStyle);

	defaultIndicatorStyle._styleID = SCE_UNIVERSAL_FOUND_STYLE_EXT2;
	defaultIndicatorStyle._bgColor = orange;
	pStyle = &defaultIndicatorStyle;
	iFind = stylers.getStylerIndexByID(SCE_UNIVERSAL_FOUND_STYLE_EXT2);
	if (iFind != -1)	{

		pStyle = &(stylers.getStyler(iFind));
	}
	setSpecialIndicator(*pStyle);

	defaultIndicatorStyle._styleID = SCE_UNIVERSAL_FOUND_STYLE_EXT3;
	defaultIndicatorStyle._bgColor = yellow;
	pStyle = &defaultIndicatorStyle;
	iFind = stylers.getStylerIndexByID(SCE_UNIVERSAL_FOUND_STYLE_EXT3);
	if (iFind != -1)	{

		pStyle = &(stylers.getStyler(iFind));
	}
	setSpecialIndicator(*pStyle);

	defaultIndicatorStyle._styleID = SCE_UNIVERSAL_FOUND_STYLE_EXT4;
	defaultIndicatorStyle._bgColor = purple;
	pStyle = &defaultIndicatorStyle;
	iFind = stylers.getStylerIndexByID(SCE_UNIVERSAL_FOUND_STYLE_EXT4);
	if (iFind != -1)	{

		pStyle = &(stylers.getStyler(iFind));
	}
	setSpecialIndicator(*pStyle);

	defaultIndicatorStyle._styleID = SCE_UNIVERSAL_FOUND_STYLE_EXT5;
	defaultIndicatorStyle._bgColor = darkGreen;
	pStyle = &defaultIndicatorStyle;
	iFind = stylers.getStylerIndexByID(SCE_UNIVERSAL_FOUND_STYLE_EXT5);
	if (iFind != -1)	{

		pStyle = &(stylers.getStyler(iFind));
	}
	setSpecialIndicator(*pStyle);

	// Il faut surtout faire un test ici avant d'ex�cuter SCI_SETCODEPAGE
	// Sinon y'aura un soucis de performance!
	if (isCJK())	{

		if (getCurrentBuffer()->getUnicodeMode() == uni8Bit)	{

			if (typeDoc == L_CSS || typeDoc == L_CAML || typeDoc == L_ASM || typeDoc == L_MATLAB)
				f(SCI_SETCODEPAGE, CP_ACP);
			else
				f(SCI_SETCODEPAGE, _codepage);
		}
	}

	ScintillaViewParams & svp = (ScintillaViewParams &)param.getSVP();
	if (svp._folderStyle != FOLDER_STYLE_NONE)
		showMargin(_SC_MARGE_FOLDER, isNeededFolderMarge(typeDoc));

	switch (typeDoc)	{

		case L_C :
		case L_CPP :
		case L_JAVA :
		case L_RC :
		case L_CS :
		case L_FLASH :
		case L_SWIFT:
			setCppLexer(typeDoc); break;

		case L_JS:
		case L_JAVASCRIPT:
			setJsLexer(); break;

		case L_TCL :
				setTclLexer(); break;


		case L_OBJC :
				setObjCLexer(typeDoc); break;

		case L_PHP :
		case L_ASP :
		case L_JSP :
		case L_HTML :
		case L_XML :
			setXmlLexer(typeDoc); break;

		case L_JSON:
			setJsonLexer(); break;

		case L_CSS :
			setCssLexer(); break;

		case L_LUA :
			setLuaLexer(); break;

		case L_MAKEFILE :
			setMakefileLexer(); break;

		case L_INI :
			setIniLexer(); break;

		case L_USER : {
			const TCHAR * langExt = _currentBuffer->getUserDefineLangName();
			if (langExt[0])
				setUserLexer(langExt);
			else
				setUserLexer();
			break; }

		case L_ASCII :	{

			LexerStyler *pStyler = (param.getLStylerArray()).getLexerStylerByName(L"nfo");

			Style nfoStyle;
			nfoStyle._styleID = STYLE_DEFAULT;
			nfoStyle._fontName = L"Lucida Console";
			nfoStyle._fontSize = 10;

			if (pStyler)	{

				int i = pStyler->getStylerIndexByName(L"DEFAULT");
				if (i != -1)	{

					Style & style = pStyler->getStyler(i);
					nfoStyle._bgColor = style._bgColor;
					nfoStyle._fgColor = style._fgColor;
					nfoStyle._colorStyle = style._colorStyle;
				}
			}
			setSpecialStyle(nfoStyle);
			f(SCI_STYLECLEARALL);

			Buffer * buf = MainFileManager.getBufferByID(_currentBufferID);

			if (buf->getEncoding() != NPP_CP_DOS_437)	{

				buf->setEncoding(NPP_CP_DOS_437);
				::SendMessage(_hParent, WM_COMMAND, IDM_FILE_RELOAD, 0);
			}
		}
		break;

		case L_SQL :
			setSqlLexer(); break;

		case L_VB :
			setVBLexer(); break;

		case L_PASCAL :
			setPascalLexer(); break;

		case L_PERL :
			setPerlLexer(); break;

		case L_PYTHON :
			setPythonLexer(); break;

		case L_BATCH :
			setBatchLexer(); break;

		case L_TEX :
			setTeXLexer(); break;

		case L_NSIS :
			setNsisLexer(); break;

		case L_BASH :
			setBashLexer(); break;

		case L_FORTRAN :
			setFortranLexer(); break;

		case L_FORTRAN_77 :
			setFortran77Lexer(); break;

		case L_LISP :
				setLispLexer(); break;

		case L_SCHEME :
				setSchemeLexer(); break;

		case L_ASM :
				setAsmLexer(); break;

		case L_DIFF :
				setDiffLexer(); break;

		case L_PROPS :
				setPropsLexer(); break;

		case L_PS :
				setPostscriptLexer(); break;

		case L_RUBY :
				setRubyLexer(); break;

		case L_SMALLTALK :
				setSmalltalkLexer(); break;

		case L_VHDL :
				setVhdlLexer(); break;

		case L_KIX :
				setKixLexer(); break;

		case L_CAML :
				setCamlLexer(); break;

		case L_ADA :
				setAdaLexer(); break;

		case L_VERILOG :
				setVerilogLexer(); break;

		case L_AU3 :
				setAutoItLexer(); break;

		case L_MATLAB :
				setMatlabLexer(); break;

		case L_HASKELL :
				setHaskellLexer(); break;

		case L_INNO :
			setInnoLexer(); break;

		case L_CMAKE :
			setCmakeLexer(); break;

		case L_YAML :
			setYamlLexer(); break;

		case L_COBOL :
			setCobolLexer(); break;

		case L_GUI4CLI :
			setGui4CliLexer(); break;

		case L_D :
			setDLexer(); break;

		case L_POWERSHELL :
			setPowerShellLexer(); break;

		case L_R :
			setRLexer(); break;

		case L_COFFEESCRIPT :
			setCoffeeScriptLexer(); break;

		case L_BAANC:
			setBaanCLexer(); break;

		case L_SREC :
			setSrecLexer(); break;

		case L_IHEX :
			setIHexLexer(); break;

		case L_TEHEX :
			setTEHexLexer(); break;

		case L_ASN1 :
			setAsn1Lexer(); break;

		case L_AVS :
			setAVSLexer(); break;

		case L_BLITZBASIC :
			setBlitzBasicLexer(); break;

		case L_PUREBASIC :
			setPureBasicLexer(); break;

		case L_FREEBASIC :
			setFreeBasicLexer(); break;

		case L_CSOUND :
			setCsoundLexer(); break;

		case L_ERLANG :
			setErlangLexer(); break;

		case L_ESCRIPT :
			setESCRIPTLexer(); break;

		case L_FORTH :
			setForthLexer(); break;

		case L_LATEX :
			setLatexLexer(); break;

		case L_MMIXAL :
			setMMIXALLexer(); break;

		case L_NIMROD :
			setNimrodLexer(); break;

		case L_NNCRONTAB :
			setNncrontabLexer(); break;

		case L_OSCRIPT :
			setOScriptLexer(); break;

		case L_REBOL :
			setREBOLLexer(); break;

		case L_REGISTRY :
			setRegistryLexer(); break;

		case L_RUST :
			setRustLexer(); break;

		case L_SPICE :
			setSpiceLexer(); break;

		case L_TXT2TAGS :
			setTxt2tagsLexer(); break;

		case L_VISUALPROLOG:
			setVisualPrologLexer(); break;

		case L_TEXT :
		default :
			if (typeDoc >= L_EXTERNAL && typeDoc < param.L_END)
				setExternalLexer(typeDoc);
			else
				f(SCI_SETLEXER, (_codepage == CP_CHINESE_TRADITIONAL)?SCLEX_MAKEFILE:SCLEX_NULL);
			break;

	}
	//All the global styles should put here
	int indexOfIndentGuide = stylers.getStylerIndexByID(STYLE_INDENTGUIDE);
	if (indexOfIndentGuide != -1)	{

		Style & styleIG = stylers.getStyler(indexOfIndentGuide);
		setStyle(styleIG);
	}
	int indexOfBraceLight = stylers.getStylerIndexByID(STYLE_BRACELIGHT);
	if (indexOfBraceLight != -1)	{

		Style & styleBL = stylers.getStyler(indexOfBraceLight);
		setStyle(styleBL);
	}
	//setStyle(STYLE_CONTROLCHAR, liteGrey);
	int indexBadBrace = stylers.getStylerIndexByID(STYLE_BRACEBAD);
	if (indexBadBrace != -1)	{

		Style & styleBB = stylers.getStyler(indexBadBrace);
		setStyle(styleBB);
	}
	int indexLineNumber = stylers.getStylerIndexByID(STYLE_LINENUMBER);
	if (indexLineNumber != -1)	{

		Style & styleLN = stylers.getStyler(indexLineNumber);
		setSpecialStyle(styleLN);
	}
	setTabSettings(param.getLangFromID(typeDoc));
	
	if (svp._indentGuideLineShow)	{

		const auto currentIndentMode = f(SCI_GETINDENTATIONGUIDES);
		// Python like indentation, excludes lexers (Nim, VB, YAML, etc.)
		// that includes tailing empty or whitespace only lines in folding block.
		const bool pythonLike = (typeDoc == L_PYTHON || typeDoc == L_COFFEESCRIPT || typeDoc == L_HASKELL);
		const int docIndentMode = pythonLike ? SC_IV_LOOKFORWARD : SC_IV_LOOKBOTH;
		if (currentIndentMode != docIndentMode)
			f(SCI_SETINDENTATIONGUIDES, docIndentMode);
	}
}

BufferID ScintillaEditView::attachDefaultDoc()	{

	// get the doc pointer attached (by default) on the view Scintilla
	Document doc = f(SCI_GETDOCPOINTER, 0, 0);
	f(SCI_ADDREFDOCUMENT, 0, doc);
	BufferID id = MainFileManager.bufferFromDocument(doc, false, true);//true, true);	//keep counter on 1
	Buffer * buf = MainFileManager.getBufferByID(id);

	MainFileManager.addBufferReference(id, this);	//add a reference. Notepad only shows the buffer in tabbar

	_currentBufferID = id;
	_currentBuffer = buf;
	bufferUpdated(buf, BufferChangeMask);	//make sure everything is in sync with the buffer, since no reference exists

	return id;
}

void ScintillaEditView::saveCurrentPos()	{

	//Save data so, that the current topline becomes visible again after restoring.
	int32_t displayedLine = static_cast<int32_t>(f(SCI_GETFIRSTVISIBLELINE));
	int32_t docLine = static_cast<int32_t>(f(SCI_DOCLINEFROMVISIBLE, displayedLine));		//linenumber of the line displayed in the top
	int32_t offset = displayedLine - static_cast<int32_t>(f(SCI_VISIBLEFROMDOCLINE, docLine));		//use this to calc offset of wrap. If no wrap this should be zero
	int wrapCount = static_cast<int32_t>(f(SCI_WRAPCOUNT, docLine));

	Buffer * buf = MainFileManager.getBufferByID(_currentBufferID);

	Position pos;
	// the correct visible line number
	pos._firstVisibleLine = docLine;
	pos._startPos = static_cast<int>(f(SCI_GETANCHOR));
	pos._endPos = static_cast<int>(f(SCI_GETCURRENTPOS));
	pos._xOffset = static_cast<int>(f(SCI_GETXOFFSET));
	pos._selMode = static_cast<int32_t>(f(SCI_GETSELECTIONMODE));
	pos._scrollWidth = static_cast<int32_t>(f(SCI_GETSCROLLWIDTH));
	pos._offset = offset;
	pos._wrapCount = wrapCount;

	buf->setPosition(pos, this);
}

// restore current position is executed in two steps.
// The detection wrap state done in the pre step function:
// if wrap is enabled, then _positionRestoreNeeded is activated
// so post step function will be cakked in the next SCN_PAINTED message
void ScintillaEditView::restoreCurrentPosPreStep()	{

	Buffer * buf = MainFileManager.getBufferByID(_currentBufferID);
	Position & pos = buf->getPosition(this);

	f(SCI_SETSELECTIONMODE, pos._selMode);	//enable
	f(SCI_SETANCHOR, pos._startPos);
	f(SCI_SETCURRENTPOS, pos._endPos);
	f(SCI_CANCEL);							//disable
	if (not isWrap())	{ //only offset if not wrapping, otherwise the offset isnt needed at all

		f(SCI_SETSCROLLWIDTH, pos._scrollWidth);
		f(SCI_SETXOFFSET, pos._xOffset);
	}
	f(SCI_CHOOSECARETX); // choose current x position
	int lineToShow = static_cast<int32_t>(f(SCI_VISIBLEFROMDOCLINE, pos._firstVisibleLine));
	f(SCI_SETFIRSTVISIBLELINE, lineToShow);
	if (isWrap())	{

		// Enable flag 'positionRestoreNeeded' so that function restoreCurrentPosPostStep get called
		// once scintilla send SCN_PAITED notification
		_positionRestoreNeeded = true;
	}
	_restorePositionRetryCount = 0;

}

// If wrap is enabled, the post step function will be called in the next SCN_PAINTED message
// to scroll several lines to set the first visible line to the correct wrapped line.
void ScintillaEditView::restoreCurrentPosPostStep()	{

	if (!_positionRestoreNeeded)
		return;

	static int32_t restoreDone = 0;
	Buffer * buf = MainFileManager.getBufferByID(_currentBufferID);
	Position & pos = buf->getPosition(this);

	++_restorePositionRetryCount;

	// Scintilla can send several SCN_PAINTED notifications before the buffer is ready to be displayed. 
	// this post step function is therefore iterated several times in a maximum of 8 iterations. 
	// 8 is an arbitrary number. 2 is a minimum. Maximum value is unknown.
	if (_restorePositionRetryCount > 8)	{

		// Abort the position restoring  process. Buffer topology may have changed
		_positionRestoreNeeded = false;
		return;
	}
	
	int32_t displayedLine = static_cast<int32_t>(f(SCI_GETFIRSTVISIBLELINE));
	int32_t docLine = static_cast<int32_t>(f(SCI_DOCLINEFROMVISIBLE, displayedLine));		//linenumber of the line displayed in the 
	

	// check docLine must equals saved position
	if (docLine != pos._firstVisibleLine)	{

		
		// Scintilla has paint the buffer but the position is not correct.
		int lineToShow = static_cast<int32_t>(f(SCI_VISIBLEFROMDOCLINE, pos._firstVisibleLine));
		f(SCI_SETFIRSTVISIBLELINE, lineToShow);
	}
	else if (pos._offset > 0)	{

		// don't scroll anything if the wrap count is different than the saved one.
		// Buffer update may be in progress (in case wrap is enabled)
		int wrapCount = static_cast<int32_t>(f(SCI_WRAPCOUNT, docLine));
		if (wrapCount == pos._wrapCount)	{

			scroll(0, pos._offset);
			_positionRestoreNeeded = false;
		}
	}
	else	{

		// Buffer position is correct, and there is no scroll to apply
		_positionRestoreNeeded = false;
	}
}

void ScintillaEditView::restyleBuffer()	{

	f(SCI_CLEARDOCUMENTSTYLE);
	f(SCI_COLOURISE, 0, -1);
	_currentBuffer->setNeedsLexing(false);
}

void ScintillaEditView::styleChange()	{

	defineDocType(_currentBuffer->getLangType());
	restyleBuffer();
}

void ScintillaEditView::activateBuffer(BufferID buffer)	{

	if (buffer == BUFFER_INVALID)
		return;
	if (buffer == _currentBuffer)
		return;
	Buffer * newBuf = MainFileManager.getBufferByID(buffer);

	// before activating another document, we get the current position
	// from the Scintilla view then save it to the current document
	saveCurrentPos();

	// get foldStateInfo of current doc
	vector<size_t> lineStateVector;
	getCurrentFoldStates(lineStateVector);

	// put the state into the future ex buffer
	_currentBuffer->setHeaderLineState(lineStateVector, this);

	_currentBufferID = buffer;	//the magical switch happens here
	_currentBuffer = newBuf;
	// change the doc, this operation will decrease
	// the ref count of old current doc and increase the one of the new doc. FileManager should manage the rest
	// Note that the actual reference in the Buffer itself is NOT decreased, Notepad_plus does that if neccessary
	f(SCI_SETDOCPOINTER, 0, _currentBuffer->getDocument());

	// Due to f(SCI_CLEARDOCUMENTSTYLE); in defineDocType() function
	// defineDocType() function should be called here, but not be after the fold info loop
	defineDocType(_currentBuffer->getLangType());

	setWordChars();

	if (_currentBuffer->getNeedsLexing())	{

		restyleBuffer();
	}

	// restore the collapsed info
	const vector<size_t> & lineStateVectorNew = newBuf->getHeaderLineState(this);
	syncFoldStateWith(lineStateVectorNew);

	restoreCurrentPosPreStep();

	bufferUpdated(_currentBuffer, (BufferChangeMask & ~BufferChangeLanguage));	//everything should be updated, but the language (which undoes some operations done here like folding)

	//setup line number margin
	int numLines = static_cast<int32_t>(f(SCI_GETLINECOUNT));

	char numLineStr[32];
	itoa(numLines, numLineStr, 10);

	runMarkers(true, 0, true, false);
	return;	//all done
}

void ScintillaEditView::getCurrentFoldStates(vector<size_t> & lineStateVector)	{

	// xCodeOptimization1304: For active document get folding state from Scintilla.
	// The code using SCI_CONTRACTEDFOLDNEXT is usually 10%-50% faster than checking each line of the document!!
	size_t contractedFoldHeaderLine = 0;

	do {
		contractedFoldHeaderLine = static_cast<size_t>(f(SCI_CONTRACTEDFOLDNEXT, contractedFoldHeaderLine));
		if (contractedFoldHeaderLine != -1)	{

			//-- Store contracted line
			lineStateVector.push_back(contractedFoldHeaderLine);
			//-- Start next search with next line
			++contractedFoldHeaderLine;
		}
	} while (contractedFoldHeaderLine != -1);
}

void ScintillaEditView::syncFoldStateWith(const vector<size_t> & lineStateVectorNew)	{

	size_t nbLineState = lineStateVectorNew.size();
	for (size_t i = 0 ; i < nbLineState ; ++i)	{

		auto line = lineStateVectorNew.at(i);
		fold(line, false);
	}
}

void ScintillaEditView::bufferUpdated(Buffer * buffer, int mask)	{

	//actually only care about language and lexing etc
	if (buffer == _currentBuffer)	{

		if (mask & BufferChangeLanguage)	{

			defineDocType(buffer->getLangType());
			foldAll(fold_uncollapse);
		}

		if (mask & BufferChangeLexing)	{

			if (buffer->getNeedsLexing())	{

				restyleBuffer();	//sets to false, this will apply to any other view aswell
			}	//else nothing, otherwise infinite loop
		}

		if (mask & BufferChangeFormat)	{

			f(SCI_SETEOLMODE, static_cast<int>(_currentBuffer->getEolFormat()));
		}
		if (mask & BufferChangeReadonly)	{

			f(SCI_SETREADONLY, _currentBuffer->isReadOnly());
		}
		if (mask & BufferChangeUnicode)	{

				int enc = CP_ACP;
			if (buffer->getUnicodeMode() == uni8Bit)	{
	//either 0 or CJK codepage
				LangType typeDoc = buffer->getLangType();
				if (isCJK())	{

					if (typeDoc == L_CSS || typeDoc == L_CAML || typeDoc == L_ASM || typeDoc == L_MATLAB)
						enc = CP_ACP;	//you may also want to set charsets here, not yet implemented
					else
						enc = _codepage;
				}
					else
						enc = CP_ACP;
			}
			else	//CP UTF8 for all unicode
				enc = SC_CP_UTF8;
				f(SCI_SETCODEPAGE, enc);
		}
	}
}

bool ScintillaEditView::isFoldIndentationBased() const
{
	const auto lexer = f(SCI_GETLEXER);
	// search IndentAmount in scintilla\lexers folder
	return lexer == SCLEX_PYTHON
		|| lexer == SCLEX_COFFEESCRIPT
		|| lexer == SCLEX_HASKELL
		|| lexer == SCLEX_NIMROD
		|| lexer == SCLEX_VB
		|| lexer == SCLEX_YAML
	;
}

namespace {

struct FoldLevelStack	{

	int levelCount = 0; // 1-based level number
	int levelStack[MAX_FOLD_COLLAPSE_LEVEL]{};

	void push(int level)	{

		while (levelCount && level <= levelStack[levelCount - 1])	{

			--levelCount;
		}
		levelStack[levelCount++] = level;
	}
};

}

void ScintillaEditView::collapseFoldIndentationBased(int level2Collapse, bool mode)	{

	f(SCI_COLOURISE, 0, -1);

	FoldLevelStack levelStack;
	++level2Collapse; // 1-based level number

	const int maxLine = static_cast<int32_t>(f(SCI_GETLINECOUNT));
	int line = 0;

	while (line < maxLine)	{

		int level = static_cast<int32_t>(f(SCI_GETFOLDLEVEL, line));
		if (level & SC_FOLDLEVELHEADERFLAG)	{

			level &= SC_FOLDLEVELNUMBERMASK;
			// don't need the actually level number, only the relationship.
			levelStack.push(level);
			if (level2Collapse == levelStack.levelCount)	{

				if (bool(f(SCI_GETFOLDEXPANDED, line)) != mode)	{

					fold(line, mode);
				}
				// skip all children lines, required to avoid buffer overrun.
				line = static_cast<int32_t>(f(SCI_GETLASTCHILD, line, -1));
			}
		}
		++line;
	}

	runMarkers(true, 0, true, false);
}

void ScintillaEditView::collapse(int level2Collapse, bool mode)	{

	if (isFoldIndentationBased())	{

		collapseFoldIndentationBased(level2Collapse, mode);
		return;
	}

	f(SCI_COLOURISE, 0, -1);

	int maxLine = static_cast<int32_t>(f(SCI_GETLINECOUNT));

	for (int line = 0; line < maxLine; ++line)	{

		int level = static_cast<int32_t>(f(SCI_GETFOLDLEVEL, line));
		if (level & SC_FOLDLEVELHEADERFLAG)	{

			level -= SC_FOLDLEVELBASE;
			if (level2Collapse == (level & SC_FOLDLEVELNUMBERMASK))
				if (bool(f(SCI_GETFOLDEXPANDED, line)) != mode)	{

					fold(line, mode);
				}
		}
	}
	runMarkers(true, 0, true, false);
}


void ScintillaEditView::getText(char *dest, size_t start, size_t end) const
{
	Sci_TextRange tr;
	tr.chrg.cpMin = static_cast<long>(start);
	tr.chrg.cpMax = static_cast<long>(end);
	tr.lpstrText = dest;
	f(SCI_GETTEXTRANGE, 0, reinterpret_cast<LPARAM>(&tr));
}

generic_string ScintillaEditView::getGenericTextAsString(size_t start, size_t end) const
{
	assert(end > start);
	const size_t bufSize = end - start + 1;
	TCHAR *buf = new TCHAR[bufSize];
	getGenericText(buf, bufSize, start, end);
	generic_string text = buf;
	delete[] buf;
	return text;
}

void ScintillaEditView::getGenericText(TCHAR *dest, size_t destlen, size_t start, size_t end) const
{
	char *destA = new char[end - start + 1];
	getText(destA, start, end);
	UINT cp = static_cast<UINT>(f(SCI_GETCODEPAGE));
	const TCHAR *destW = wmc.char2wchar(destA, cp);
	_tcsncpy_s(dest, destlen, destW, _TRUNCATE);
	delete [] destA;
}

// "mstart" and "mend" are pointers to indexes in the read string,
// which are converted to the corresponding indexes in the returned TCHAR string.

void ScintillaEditView::getGenericText(TCHAR *dest, size_t destlen, int start, int end, int *mstart, int *mend) const
{
		char *destA = new char[end - start + 1];
	getText(destA, start, end);
	UINT cp = static_cast<UINT>(f(SCI_GETCODEPAGE))    ;
	const TCHAR *destW = wmc.char2wchar(destA, cp, mstart, mend);
	_tcsncpy_s(dest, destlen, destW, _TRUNCATE);
	delete [] destA;
}

void ScintillaEditView::insertGenericTextFrom(size_t position, const TCHAR *text2insert) const
{
		UINT cp = static_cast<UINT>(f(SCI_GETCODEPAGE));
	const char *text2insertA = wmc.wchar2char(text2insert, cp);
	f(SCI_INSERTTEXT, position, reinterpret_cast<LPARAM>(text2insertA));
}

void ScintillaEditView::replaceSelWith(const char * replaceText)	{

	f(SCI_REPLACESEL, 0, reinterpret_cast<LPARAM>(replaceText));
}

void ScintillaEditView::getVisibleStartAndEndPosition(int * startPos, int * endPos)	{

	assert(startPos != NULL && endPos);

	auto firstVisibleLine = f(SCI_GETFIRSTVISIBLELINE);
	*startPos = static_cast<int32_t>(f(SCI_POSITIONFROMLINE, f(SCI_DOCLINEFROMVISIBLE, firstVisibleLine)));
	auto linesOnScreen = f(SCI_LINESONSCREEN);
	auto lineCount = f(SCI_GETLINECOUNT);
	auto visibleLine = f(SCI_DOCLINEFROMVISIBLE, firstVisibleLine + min(linesOnScreen, lineCount));
	*endPos = static_cast<int32_t>(f(SCI_POSITIONFROMLINE, visibleLine));
	if (*endPos == -1) 
		*endPos = static_cast<int32_t>(f(SCI_GETLENGTH));
}

char * ScintillaEditView::getWordFromRange(char * txt, int size, int pos1, int pos2)	{

	if (pos1 > pos2)	swap(pos1, pos2);
	// if (!size)	return NULL;// must be check before
	if (size < pos2-pos1)     return NULL;

	getText(txt, pos1, pos2);
	return txt;
}

char * ScintillaEditView::getWordOnCaretPos(char * txt, int size)	{

	if (!size)
		return NULL;

	pair<int,int> range = getWordRange();
	return getWordFromRange(txt, size, range.first, range.second);
}

TCHAR * ScintillaEditView::getGenericWordOnCaretPos(TCHAR * txt, int size)	{

		UINT cp = static_cast<UINT>(f(SCI_GETCODEPAGE));
	char *txtA = new char[size + 1];
	getWordOnCaretPos(txtA, size);

	const TCHAR * txtW = wmc.char2wchar(txtA, cp);
	wcscpy_s(txt, size, txtW);
	delete [] txtA;
	return txt;
}

char * ScintillaEditView::getSelectedText(char * txt, int size, bool expand)	{

	if (!size)
		return NULL;
	Sci_CharacterRange range = getSelection();
	if (expand && range.cpMax == range.cpMin )	{

		expandWordSelection();
		range = getSelection();
	}
	if (size <= (range.cpMax - range.cpMin))	//there must be atleast 1 byte left for zero terminator
		range.cpMax = range.cpMin+size-1;	//keep room for zero terminator
	//getText(txt, range.cpMin, range.cpMax);
	return getWordFromRange(txt, size, range.cpMin, range.cpMax);
}

TCHAR * ScintillaEditView::getGenericSelectedText(TCHAR * txt, int size, bool expand)	{

		UINT cp = static_cast<UINT>(f(SCI_GETCODEPAGE));
	char *txtA = new char[size + 1];
	getSelectedText(txtA, size, expand);

	const TCHAR * txtW = wmc.char2wchar(txtA, cp);
	wcscpy_s(txt, size, txtW);
	delete [] txtA;
	return txt;
}

int ScintillaEditView::searchInTarget(const TCHAR * text2Find, size_t lenOfText2Find, size_t fromPos, size_t toPos) const	{
	f(SCI_SETTARGETRANGE, fromPos, toPos);

	UINT cp = static_cast<UINT>(f(SCI_GETCODEPAGE));
	const char *text2FindA = wmc.wchar2char(text2Find, cp);
	return static_cast<int32_t>(f(SCI_SEARCHINTARGET, max(lenOfText2Find,strlen(text2FindA)), reinterpret_cast<LPARAM>(text2FindA)));
}

void ScintillaEditView::appendGenericText(const TCHAR * text2Append) const
{
		UINT cp = static_cast<UINT>(f(SCI_GETCODEPAGE));
	const char *text2AppendA =wmc.wchar2char(text2Append, cp);
	f(SCI_APPENDTEXT, strlen(text2AppendA), reinterpret_cast<LPARAM>(text2AppendA));
}

void ScintillaEditView::addGenericText(const TCHAR * text2Append) const
{
		UINT cp = static_cast<UINT>(f(SCI_GETCODEPAGE));
	const char *text2AppendA =wmc.wchar2char(text2Append, cp);
	f(SCI_ADDTEXT, strlen(text2AppendA), reinterpret_cast<LPARAM>(text2AppendA));
}

void ScintillaEditView::addGenericText(const TCHAR * text2Append, long *mstart, long *mend) const
{
		UINT cp = static_cast<UINT>(f(SCI_GETCODEPAGE));
	const char *text2AppendA =wmc.wchar2char(text2Append, cp, mstart, mend);
	f(SCI_ADDTEXT, strlen(text2AppendA), reinterpret_cast<LPARAM>(text2AppendA));
}

int32_t ScintillaEditView::replaceTarget(const TCHAR * str2replace, int fromTargetPos, int toTargetPos) const
{
	if (fromTargetPos != -1 || toTargetPos != -1)	{

		f(SCI_SETTARGETRANGE, fromTargetPos, toTargetPos);
	}
		UINT cp = static_cast<UINT>(f(SCI_GETCODEPAGE));
	const char *str2replaceA = wmc.wchar2char(str2replace, cp);
	return static_cast<int32_t>(f(SCI_REPLACETARGET, static_cast<WPARAM>(-1), reinterpret_cast<LPARAM>(str2replaceA)));
}

int ScintillaEditView::replaceTargetRegExMode(const TCHAR * re, int fromTargetPos, int toTargetPos) const
{
	if (fromTargetPos != -1 || toTargetPos != -1)	{

		f(SCI_SETTARGETRANGE, fromTargetPos, toTargetPos);
	}
		UINT cp = static_cast<UINT>(f(SCI_GETCODEPAGE));
	const char *reA = wmc.wchar2char(re, cp);
	return static_cast<int32_t>(f(SCI_REPLACETARGETRE, static_cast<WPARAM>(-1), reinterpret_cast<LPARAM>(reA)));
}

void ScintillaEditView::showCallTip(int startPos, const TCHAR * def)	{

	UINT cp = static_cast<UINT>(f(SCI_GETCODEPAGE));
	const char *defA = wmc.wchar2char(def, cp);
	f(SCI_CALLTIPSHOW, startPos, reinterpret_cast<LPARAM>(defA));
}

generic_string ScintillaEditView::getLine(size_t lineNumber)	{

	int32_t lineLen = static_cast<int32_t>(f(SCI_LINELENGTH, lineNumber));
	const int bufSize = lineLen + 1;
	unique_ptr<TCHAR[]> buf = make_unique<TCHAR[]>(bufSize);
	getLine(lineNumber, buf.get(), bufSize);
	return buf.get();
}

void ScintillaEditView::getLine(size_t lineNumber, TCHAR * line, int lineBufferLen)	{

	// make sure the buffer length is enough to get the whole line
	auto lineLen = f(SCI_LINELENGTH, lineNumber);
	if (lineLen >= lineBufferLen)
		return;

		UINT cp = static_cast<UINT>(f(SCI_GETCODEPAGE));
	char *lineA = new char[lineBufferLen];
	// From Scintilla documentation for SCI_GETLINE: "The buffer is not terminated by a 0 character."
	memset(lineA, 0x0, sizeof(char) * lineBufferLen);
	
	f(SCI_GETLINE, lineNumber, reinterpret_cast<LPARAM>(lineA));
	const TCHAR *lineW = wmc.char2wchar(lineA, cp);
	lstrcpyn(line, lineW, lineBufferLen);
	delete [] lineA;
}

void ScintillaEditView::addText(size_t length, const char *buf)	{

	f(SCI_ADDTEXT, length, reinterpret_cast<LPARAM>(buf));
}

/* void ScintillaEditView::beginOrEndSelect()	{

	if (beginSelectPos == -1)	{

		beginSelectPos = static_cast<int32_t>(f(SCI_GETCURRENTPOS));
	}
	else	{

		f(SCI_SETANCHOR, static_cast<WPARAM>(beginSelectPos));
		beginSelectPos = -1;
	}
} */

void ScintillaEditView::marginClick(Sci_Position position, int modifiers)	{

	size_t lineClick = f(SCI_LINEFROMPOSITION, position, 0);
	int levelClick = static_cast<int32_t>(f(SCI_GETFOLDLEVEL, lineClick, 0));
	if (levelClick & SC_FOLDLEVELHEADERFLAG)	{

		if (modifiers & SCMOD_SHIFT)	{

			// Ensure all children visible
			f(SCI_SETFOLDEXPANDED, lineClick, 1);
			expand(lineClick, true, true, 100, levelClick);
		}
		else if (modifiers & SCMOD_CTRL)	{

			if (bool(f(SCI_GETFOLDEXPANDED, lineClick)))	{

				// Contract this line and all children
				f(SCI_SETFOLDEXPANDED, lineClick, 0);
				expand(lineClick, false, true, 0, levelClick);
			}
				else	{

				// Expand this line and all children
				f(SCI_SETFOLDEXPANDED, lineClick, 1);
				expand(lineClick, true, true, 100, levelClick);
			}
		}
		else	{

			// Toggle this line
			bool mode = bool(f(SCI_GETFOLDEXPANDED, lineClick));
			fold(lineClick, !mode);
			runMarkers(true, lineClick, true, false);
		}
	}
}

void ScintillaEditView::expand(size_t& line, bool doExpand, bool force, int visLevels, int level)	{

	size_t lineMaxSubord = f(SCI_GETLASTCHILD, line, level & SC_FOLDLEVELNUMBERMASK);
	++line;
	while (line <= lineMaxSubord)	{

		if (force)	{

			f(((visLevels > 0) ? SCI_SHOWLINES : SCI_HIDELINES), line, line);
		}
		else	{

			if (doExpand)
				f(SCI_SHOWLINES, line, line);
		}

		int levelLine = level;
		if (levelLine == -1)
			levelLine = static_cast<int32_t>(f(SCI_GETFOLDLEVEL, line, 0));

		if (levelLine & SC_FOLDLEVELHEADERFLAG)	{
			if (force)	{
				if (visLevels > 1)
					f(SCI_SETFOLDEXPANDED, line, 1);
				else
					f(SCI_SETFOLDEXPANDED, line, 0);
				expand(line, doExpand, force, visLevels - 1);
			}
				else
					if (doExpand)	{
						if (!bool(f(SCI_GETFOLDEXPANDED, line)))
							f(SCI_SETFOLDEXPANDED, line, 1);
						expand(line, true, force, visLevels - 1);
					}
					else
						expand(line, false, force, visLevels - 1);
		}
		else
			++line;
	}
	runMarkers(true, 0, true, false);
}


void ScintillaEditView::performCrHiLi()	{
	bool is;
	Style& style = param.getMiscStylerArray().styleOf(L"Current line background colour", is);
	if (is)	{
		if (style._lastColorState || style._colorStyle)	{
			f(SCI_SETCARETLINEBACK, style._lastColorState ? style._lastColorState : style._bgColor);
			f(SCI_SETCARETLINEFRAME, style._colorStyle);
			f(SCI_SETCARETLINEVISIBLE, 1);
		}
		else	{
			f(SCI_SETCARETLINEBACK, style._fgColor ? style._fgColor : style._bgColor ? style._bgColor : 0xCC3371);
			f(SCI_SETCARETLINEFRAME, 0);
			f(SCI_SETCARETLINEVISIBLE, 0);
		}
	}
}

void ScintillaEditView::performGlobalStyles()	{

	StyleArray & stylers = param.getMiscStylerArray();

	int i = stylers.getStylerIndexByName(L"Current line background colour");
	if (i != -1)	{

		Style & style = stylers.getStyler(i);
		f(SCI_SETCARETLINEBACK, style._bgColor);
		f(SCI_SETCARETLINEVISIBLE, 1);
	}

	COLORREF selectColorBack = grey;

	i = stylers.getStylerIndexByName(L"Selected text colour");
	if (i != -1)	{

		Style & style = stylers.getStyler(i);
		selectColorBack = style._bgColor;
	}
	f(SCI_SETSELBACK, 1, selectColorBack);

	COLORREF caretColor = black;
	i = stylers.getStylerIndexByID(SCI_SETCARETFORE);
	if (i != -1)	{

		Style & style = stylers.getStyler(i);
		caretColor = style._fgColor;
	}
	f(SCI_SETCARETFORE, caretColor);

	COLORREF edgeColor = liteGrey;
	i = stylers.getStylerIndexByName(L"Edge colour");
	if (i != -1)	{

		Style & style = stylers.getStyler(i);
		edgeColor = style._fgColor;
	}
	f(SCI_SETEDGECOLOUR, edgeColor);

	COLORREF foldMarginColor = grey;
	COLORREF foldMarginHiColor = white;
	i = stylers.getStylerIndexByName(L"Fold margin");
	if (i != -1)	{

		Style & style = stylers.getStyler(i);
		foldMarginHiColor = style._fgColor;
		foldMarginColor = style._bgColor;
	}
	f(SCI_SETFOLDMARGINCOLOUR, true, foldMarginColor);
	f(SCI_SETFOLDMARGINHICOLOUR, true, foldMarginHiColor);

	COLORREF foldfgColor = white, foldbgColor = grey, activeFoldFgColor = red;
	getFoldColor(foldfgColor, foldbgColor, activeFoldFgColor);

	ScintillaViewParams & svp = (ScintillaViewParams &)param.getSVP();
	for (int j = 0 ; j < NB_FOLDER_STATE ; ++j)
		defineMarker(_markersArray[FOLDER_TYPE][j], _markersArray[svp._folderStyle][j], foldfgColor, foldbgColor, activeFoldFgColor);

	f(SCI_MARKERENABLEHIGHLIGHT, true);

	COLORREF wsSymbolFgColor = black;
	i = stylers.getStylerIndexByName(L"White space symbol");
	if (i != -1)	{

		Style & style = stylers.getStyler(i);
		wsSymbolFgColor = style._fgColor;
	}
	f(SCI_SETWHITESPACEFORE, true, wsSymbolFgColor);
}

void ScintillaEditView::showIndentGuideLine(bool willBeShowed)	{

	auto typeDoc = _currentBuffer->getLangType();
	const bool pythonLike = (typeDoc == L_PYTHON || typeDoc == L_COFFEESCRIPT || typeDoc == L_HASKELL);
	const int docIndentMode = pythonLike ? SC_IV_LOOKFORWARD : SC_IV_LOOKBOTH;
	f(SCI_SETINDENTATIONGUIDES, willBeShowed ? docIndentMode : SC_IV_NONE);
}

void ScintillaEditView::setLineIndent(int line, int indent) const
{
	if (indent < 0)
		return;
	Sci_CharacterRange crange = getSelection();
	int posBefore = static_cast<int32_t>(f(SCI_GETLINEINDENTPOSITION, line));
	f(SCI_SETLINEINDENTATION, line, indent);
	int32_t posAfter = static_cast<int32_t>(f(SCI_GETLINEINDENTPOSITION, line));
	int posDifference = posAfter - posBefore;
	if (posAfter > posBefore)	{

		// Move selection on
		if (crange.cpMin >= posBefore)	{

			crange.cpMin += posDifference;
		}
		if (crange.cpMax >= posBefore)	{

			crange.cpMax += posDifference;
		}
	}
	else if (posAfter < posBefore)	{

		// Move selection back
		if (crange.cpMin >= posAfter)	{

			if (crange.cpMin >= posBefore)
				crange.cpMin += posDifference;
			else
				crange.cpMin = posAfter;
		}

		if (crange.cpMax >= posAfter)	{

			if (crange.cpMax >= posBefore)
				crange.cpMax += posDifference;
			else
				crange.cpMax = posAfter;
		}
	}
	f(SCI_SETSEL, crange.cpMin, crange.cpMax);
}

void ScintillaEditView::updateLineNumberWidth(){
	if (_lineNumbersShown)	{
		if (auto linesVisible = f(SCI_LINESONSCREEN))	{
			auto firstVisibleLineVis = f(SCI_GETFIRSTVISIBLELINE);
			auto lastVisibleLineVis = linesVisible + firstVisibleLineVis + 1;

/* 			if (f(SCI_GETWRAPMODE) != SC_WRAP_NONE)
			{
				auto numLinesDoc = f(SCI_GETLINECOUNT);
				auto prevLineDoc = f(SCI_DOCLINEFROMVISIBLE, firstVisibleLineVis);
				for (auto i = firstVisibleLineVis; i <= lastVisibleLineVis; ++i)	{// + 1

					auto lineDoc = f(SCI_DOCLINEFROMVISIBLE, i);
					if (lineDoc == numLinesDoc)		break;
					if (lineDoc == prevLineDoc)
						lastVisibleLineVis++;
					prevLineDoc = lineDoc;
				}
			} */

			auto lastVisibleLineDoc = f(SCI_DOCLINEFROMVISIBLE, lastVisibleLineVis);
			int i = 1;
			while (lastVisibleLineDoc /= 10)		++i;

			// i = max(i, 3);
			// auto pixelWidth = ;
			f(SCI_SETMARGINWIDTHN, _SC_MARGE_LINENUMBER,  3 + i * f(SCI_TEXTWIDTH, STYLE_LINENUMBER, reinterpret_cast<LPARAM>("6")));
		}
	}
}

const char * ScintillaEditView::getCompleteKeywordList(basic_string<char> & kwl, LangType langType, int keywordIndex)	{

	kwl += " ";
	const TCHAR *defKwl_generic = param.getWordList(langType, keywordIndex);

		const char * defKwl = wmc.wchar2char(defKwl_generic, CP_ACP);
	kwl += defKwl?defKwl:"";

	return kwl.c_str();
}

void ScintillaEditView::setMultiSelections(const ColumnModeInfos & cmi)	{

	for (size_t i = 0, len = cmi.size(); i < len ; ++i)	{

		if (cmi[i].isValid())	{

			int selStart = cmi[i]._direction == L2R?cmi[i]._selLpos:cmi[i]._selRpos;
			int selEnd   = cmi[i]._direction == L2R?cmi[i]._selRpos:cmi[i]._selLpos;
			f(SCI_SETSELECTIONNSTART, i, selStart);
			f(SCI_SETSELECTIONNEND, i, selEnd);
		}
		//if (cmi[i].hasVirtualSpace())
		//{
		if (cmi[i]._nbVirtualAnchorSpc)
			f(SCI_SETSELECTIONNANCHORVIRTUALSPACE, i, cmi[i]._nbVirtualAnchorSpc);
		if (cmi[i]._nbVirtualCaretSpc)
			f(SCI_SETSELECTIONNCARETVIRTUALSPACE, i, cmi[i]._nbVirtualCaretSpc);
		//}
	}
}

// Get selection range : (fromLine, toLine)
// return (-1, -1) if multi-selection
pair<int, int> ScintillaEditView::getSelectionLinesRange() const
{
	pair<int, int> range(-1, -1);
	if (f(SCI_GETSELECTIONS) > 1) // multi-selection
		return range;
	int32_t start = static_cast<int32_t>(f(SCI_GETSELECTIONSTART));
	int32_t end = static_cast<int32_t>(f(SCI_GETSELECTIONEND));

	range.first = static_cast<int32_t>(f(SCI_LINEFROMPOSITION, start));
	range.second = static_cast<int32_t>(f(SCI_LINEFROMPOSITION, end));

	return range;
}

void ScintillaEditView::currentLinesUp() const
{
	f(SCI_MOVESELECTEDLINESUP);
}

void ScintillaEditView::currentLinesDown() const
{
	f(SCI_MOVESELECTEDLINESDOWN);

	// Ensure the selection is within view
	f(SCI_SCROLLRANGE, f(SCI_GETSELECTIONEND), f(SCI_GETSELECTIONSTART));
}

void ScintillaEditView::changeCase(__inout wchar_t * const strWToConvert, const int & nbChars, const TextCase & caseToConvert) const
{
	if (!strWToConvert ||!nbChars)
		return;

	switch (caseToConvert)	{

		case UPPERCASE:	{

			for (int i = 0; i < nbChars; ++i)	{

				strWToConvert[i] = (WCHAR)(UINT_PTR)::CharUpperW((LPWSTR)strWToConvert[i]);
			}
			break; 
		} //case UPPERCASE
		case LOWERCASE:	{

			for (int i = 0; i < nbChars; ++i)	{

				strWToConvert[i] = (WCHAR)(UINT_PTR)::CharLowerW((LPWSTR)strWToConvert[i]);
			}
			break; 
		} //case LOWERCASE
		case TITLECASE_FORCE:
		case TITLECASE_BLEND:	{

			for (int i = 0; i < nbChars; ++i)	{

				if (::IsCharAlphaW(strWToConvert[i]))	{

					if ((i < 1) ? true : not ::IsCharAlphaNumericW(strWToConvert[i - 1]))
						strWToConvert[i] = (WCHAR)(UINT_PTR)::CharUpperW((LPWSTR)strWToConvert[i]);
					else if (caseToConvert == TITLECASE_FORCE)
						strWToConvert[i] = (WCHAR)(UINT_PTR)::CharLowerW((LPWSTR)strWToConvert[i]);
					//An exception
					if ((i < 2) ? false : (strWToConvert[i - 1] == L'\'' && ::IsCharAlphaW(strWToConvert[i - 2])))
						strWToConvert[i] = (WCHAR)(UINT_PTR)::CharLowerW((LPWSTR)strWToConvert[i]);
				}
			}
			break; 
		} //case TITLECASE
		case SENTENCECASE_FORCE:
		case SENTENCECASE_BLEND:	{

			bool isNewSentence = true;
			bool wasEolR = false;
			bool wasEolN = false;
			for (int i = 0; i < nbChars; ++i)	{

				if (::IsCharAlphaW(strWToConvert[i]))	{

					if (isNewSentence)	{

						strWToConvert[i] = (WCHAR)(UINT_PTR)::CharUpperW((LPWSTR)strWToConvert[i]);
						isNewSentence = false;
					}
					else if (caseToConvert == SENTENCECASE_FORCE)	{

						strWToConvert[i] = (WCHAR)(UINT_PTR)::CharLowerW((LPWSTR)strWToConvert[i]);
					}
					wasEolR = false;
					wasEolN = false;
					//An exception
					if (strWToConvert[i] == L'i' &&
						((i < 1) ? false : (::iswspace(strWToConvert[i - 1]) || strWToConvert[i - 1] == L'(' || strWToConvert[i - 1] == L'"')) &&
						((i + 1 == nbChars) ? false : (::iswspace(strWToConvert[i + 1]) || strWToConvert[i + 1] == L'\'')))
					{
						strWToConvert[i] = L'I';
					}
				}
				else if (strWToConvert[i] == L'.' || strWToConvert[i] == L'!' || strWToConvert[i] == L'?')	{

					if ((i + 1 == nbChars) ? true : ::IsCharAlphaNumericW(strWToConvert[i + 1]))
						isNewSentence = false;
					else
						isNewSentence = true;
				}
				else if (strWToConvert[i] == L'\r')	{

					if (wasEolR)
						isNewSentence = true;
					else
						wasEolR = true;
				}
				else if (strWToConvert[i] == L'\n')	{

					if (wasEolN)
						isNewSentence = true;
					else
						wasEolN = true;
				}
			}
			break;
		} //case SENTENCECASE
		case INVERTCASE:	{

			for (int i = 0; i < nbChars; ++i)	{

				if (::IsCharLowerW(strWToConvert[i]))
					strWToConvert[i] = (WCHAR)(UINT_PTR)::CharUpperW((LPWSTR)strWToConvert[i]);
				else
					strWToConvert[i] = (WCHAR)(UINT_PTR)::CharLowerW((LPWSTR)strWToConvert[i]);
			}
			break; 
		} //case INVERTCASE
		case RANDOMCASE:	{

			for (int i = 0; i < nbChars; ++i)	{

				if (::IsCharAlphaW(strWToConvert[i]))	{

					if (rand() & true)
						strWToConvert[i] = (WCHAR)(UINT_PTR)::CharUpperW((LPWSTR)strWToConvert[i]);
					else
						strWToConvert[i] = (WCHAR)(UINT_PTR)::CharLowerW((LPWSTR)strWToConvert[i]);
				}
			}
			break; 
		} //case RANDOMCASE
	} //switch (caseToConvert)
}

void ScintillaEditView::convertSelectedTextTo(const TextCase & caseToConvert)	{

	unsigned int codepage = _codepage;
	UniMode um = getCurrentBuffer()->getUnicodeMode();
	if (um != uni8Bit)
	codepage = CP_UTF8;

	if (f(SCI_GETSELECTIONS) > 1)	{ // Multi-Selection || Column mode

		f(SCI_BEGINUNDOACTION);

		ColumnModeInfos cmi = getColumnModeSelectInfo();

		for (size_t i = 0, cmiLen = cmi.size(); i < cmiLen ; ++i)	{

			const int len = cmi[i]._selRpos - cmi[i]._selLpos;
			char *srcStr = new char[len+1];
			wchar_t *destStr = new wchar_t[len+1];

			int start = cmi[i]._selLpos;
			int end = cmi[i]._selRpos;
			getText(srcStr, start, end);

			int nbChar = ::MultiByteToWideChar(codepage, 0, srcStr, len, destStr, len);

			changeCase(destStr, nbChar, caseToConvert);

			::WideCharToMultiByte(codepage, 0, destStr, len, srcStr, len, NULL, NULL);

			f(SCI_SETTARGETRANGE, start, end);
			f(SCI_REPLACETARGET, static_cast<WPARAM>(-1), reinterpret_cast<LPARAM>(srcStr));

			delete [] srcStr;
			delete [] destStr;
		}

		setMultiSelections(cmi);

		f(SCI_ENDUNDOACTION);
		return;
	}

	size_t selectionStart = f(SCI_GETSELECTIONSTART);
	size_t selectionEnd = f(SCI_GETSELECTIONEND);

	int32_t strLen = static_cast<int32_t>(selectionEnd - selectionStart);
	if (strLen)	{

		int strSize = strLen + 1;
		char *selectedStr = new char[strSize];
		int strWSize = strSize * 2;
		wchar_t *selectedStrW = new wchar_t[strWSize+3];

		f(SCI_GETSELTEXT, 0, reinterpret_cast<LPARAM>(selectedStr));

		int nbChar = ::MultiByteToWideChar(codepage, 0, selectedStr, strSize, selectedStrW, strWSize);

		changeCase(selectedStrW, nbChar, caseToConvert);

		::WideCharToMultiByte(codepage, 0, selectedStrW, strWSize, selectedStr, strSize, NULL, NULL);

		f(SCI_SETTARGETRANGE, selectionStart, selectionEnd);
		f(SCI_REPLACETARGET, strLen, reinterpret_cast<LPARAM>(selectedStr));
		f(SCI_SETSEL, selectionStart, selectionEnd);
		delete [] selectedStr;
		delete [] selectedStrW;
	}
}



pair<int, int> ScintillaEditView::getWordRange()
{
	auto caretPos = f(SCI_GETCURRENTPOS, 0, 0);
	int startPos = static_cast<int>(f(SCI_WORDSTARTPOSITION, caretPos, true));
	int endPos = static_cast<int>(f(SCI_WORDENDPOSITION, caretPos, true));
	return pair<int, int>(startPos, endPos);
}

bool ScintillaEditView::expandWordSelection()	{

	pair<int, int> wordRange = 	getWordRange();
	if (wordRange.first != wordRange.second)	{

		f(SCI_SETSELECTIONSTART, wordRange.first);
		f(SCI_SETSELECTIONEND, wordRange.second);
		return true;
	}
	return false;
}

TCHAR * int2str(TCHAR *str, int strLen, int number, int base, int nbChiffre, bool isZeroLeading)	{

	if (nbChiffre >= strLen) return NULL;
	TCHAR frm[64];
	TCHAR fStr[2] = L"d";
	if (base == 16)
		fStr[0] = 'X';
	else if (base == 8)
		fStr[0] = 'o';
	else if (base == 2)	{

		const unsigned int MASK_ULONG_BITFORT = 0x80000000;
		int nbBits = sizeof(unsigned int) * 8;
		int nbBit2Shift = (nbChiffre >= nbBits)?nbBits:(nbBits - nbChiffre);
		unsigned long mask = MASK_ULONG_BITFORT >> nbBit2Shift;
		int i = 0;
		for (; mask > 0 ; ++i)	{

			str[i] = (mask & number)?'1':'0';
			mask >>= 1;
		}
		str[i] = '\0';
	}

	if (!isZeroLeading)	{

		if (base == 2)	{

			TCHAR *j = str;
			for ( ; *j != '\0' ; ++j)
				if (*j == '1')
					break;
			wcscpy_s(str, strLen, j);
		}
		else	{

			// use sprintf or swprintf instead of wsprintf
			// to make octal format work
			generic_sprintf(frm, L"%%%s", fStr);
			generic_sprintf(str, frm, number);
		}
		int i = lstrlen(str);
		for ( ; i < nbChiffre ; ++i)
			str[i] = ' ';
		str[i] = '\0';
	}
	else	{

		if (base != 2)	{

			// use sprintf or swprintf instead of wsprintf
			// to make octal format work
			generic_sprintf(frm, L"%%.%d%s", nbChiffre, fStr);
			generic_sprintf(str, frm, number);
		}
		// else already done.
	}
	return str;
}

ColumnModeInfos ScintillaEditView::getColumnModeSelectInfo()	{

	ColumnModeInfos columnModeInfos;
	if (f(SCI_GETSELECTIONS) > 1)	{ // Multi-Selection || Column mode

		int nbSel = static_cast<int32_t>(f(SCI_GETSELECTIONS));

		for (int i = 0 ; i < nbSel ; ++i)	{

			int absPosSelStartPerLine = static_cast<int32_t>(f(SCI_GETSELECTIONNANCHOR, i));
			int absPosSelEndPerLine = static_cast<int32_t>(f(SCI_GETSELECTIONNCARET, i));
			int nbVirtualAnchorSpc = static_cast<int32_t>(f(SCI_GETSELECTIONNANCHORVIRTUALSPACE, i));
			int nbVirtualCaretSpc = static_cast<int32_t>(f(SCI_GETSELECTIONNCARETVIRTUALSPACE, i));

			if (absPosSelStartPerLine == absPosSelEndPerLine && f(SCI_SELECTIONISRECTANGLE))	{

				bool dir = nbVirtualAnchorSpc<nbVirtualCaretSpc?L2R:R2L;
				columnModeInfos.push_back(ColumnModeInfo(absPosSelStartPerLine, absPosSelEndPerLine, i, dir, nbVirtualAnchorSpc, nbVirtualCaretSpc));
			}
			else if (absPosSelStartPerLine > absPosSelEndPerLine)
				columnModeInfos.push_back(ColumnModeInfo(absPosSelEndPerLine, absPosSelStartPerLine, i, R2L, nbVirtualAnchorSpc, nbVirtualCaretSpc));
			else
				columnModeInfos.push_back(ColumnModeInfo(absPosSelStartPerLine, absPosSelEndPerLine, i, L2R, nbVirtualAnchorSpc, nbVirtualCaretSpc));
		}
	}
	return columnModeInfos;
}

void ScintillaEditView::columnReplace(ColumnModeInfos & cmi, const TCHAR *str)	{

	int totalDiff = 0;
	for (size_t i = 0, len = cmi.size(); i < len ; ++i)	{

		if (cmi[i].isValid())	{

			int len2beReplace = cmi[i]._selRpos - cmi[i]._selLpos;
			int diff = lstrlen(str) - len2beReplace;

			cmi[i]._selLpos += totalDiff;
			cmi[i]._selRpos += totalDiff;
			bool hasVirtualSpc = cmi[i]._nbVirtualAnchorSpc > 0;

			if (hasVirtualSpc)	{ // if virtual space is present, then insert space

				for (int j = 0, k = cmi[i]._selLpos; j < cmi[i]._nbVirtualCaretSpc ; ++j, ++k)	{

					f(SCI_INSERTTEXT, k, reinterpret_cast<LPARAM>(" "));
				}
				cmi[i]._selLpos += cmi[i]._nbVirtualAnchorSpc;
				cmi[i]._selRpos += cmi[i]._nbVirtualCaretSpc;
			}

			f(SCI_SETTARGETRANGE, cmi[i]._selLpos, cmi[i]._selRpos);

						UINT cp = static_cast<UINT>(f(SCI_GETCODEPAGE));
			const char *strA = wmc.wchar2char(str, cp);
			f(SCI_REPLACETARGET, static_cast<WPARAM>(-1), reinterpret_cast<LPARAM>(strA));

			if (hasVirtualSpc)	{

				totalDiff += cmi[i]._nbVirtualAnchorSpc + lstrlen(str);

				// Now there's no more virtual space
				cmi[i]._nbVirtualAnchorSpc = 0;
				cmi[i]._nbVirtualCaretSpc = 0;
			}
			else	{

				totalDiff += diff;
			}
			cmi[i]._selRpos += diff;
		}
	}
}

void ScintillaEditView::columnReplace(ColumnModeInfos & cmi, int initial, int incr, int repeat, UCHAR format)	{

	assert(repeat > 0);

	// If there is no column mode info available, no need to do anything
	// If required a message can be shown to user, that select column properly or something similar
	// It is just a double check as taken in callee method (in case this method is called from multiple places)
	if (cmi.size() <= 0)
		return;
	// 0000 00 00 : Dec BASE_10
	// 0000 00 01 : Hex BASE_16
	// 0000 00 10 : Oct BASE_08
	// 0000 00 11 : Bin BASE_02

	// 0000 01 00 : 0 leading

	//Defined in ScintillaEditView.h :
	//const UCHAR MASK_FORMAT = 0x03;
	//const UCHAR MASK_ZERO_LEADING = 0x04;

	UCHAR frm = format & MASK_FORMAT;
	bool isZeroLeading = (MASK_ZERO_LEADING & format) != 0;

	int base = 10;
	if (frm == BASE_16)
		base = 16;
	else if (frm == BASE_08)
		base = 8;
	else if (frm == BASE_02)
		base = 2;

	const int stringSize = 512;
	TCHAR str[stringSize];

	// Compute the numbers to be placed at each column.
	vector<int> numbers;
	{
		int curNumber = initial;
		const size_t kiMaxSize = cmi.size();
		while (numbers.size() < kiMaxSize)	{

			for (int i = 0; i < repeat; ++i )	{

				numbers.push_back(curNumber);
				if (numbers.size() >= kiMaxSize)	{

					break;
				}
			}
			curNumber += incr;
		}
	}

	assert(numbers.size()> 0);

	const int kibEnd = getNbDigits(*numbers.rbegin(), base);
	const int kibInit = getNbDigits(initial, base);
	const int kib = max<int>(kibInit, kibEnd);

	int totalDiff = 0;
	const size_t len = cmi.size();
	for (size_t i = 0 ; i < len ; ++i )	{

		if (cmi[i].isValid())	{

			const int len2beReplaced = cmi[i]._selRpos - cmi[i]._selLpos;
			const int diff = kib - len2beReplaced;

			cmi[i]._selLpos += totalDiff;
			cmi[i]._selRpos += totalDiff;

			int2str(str, stringSize, numbers.at(i), base, kib, isZeroLeading);

			const bool hasVirtualSpc = cmi[i]._nbVirtualAnchorSpc > 0;
			if (hasVirtualSpc)	{ // if virtual space is present, then insert space

				for (int j = 0, k = cmi[i]._selLpos; j < cmi[i]._nbVirtualCaretSpc ; ++j, ++k)	{

					f(SCI_INSERTTEXT, k, reinterpret_cast<LPARAM>(" "));
				}
				cmi[i]._selLpos += cmi[i]._nbVirtualAnchorSpc;
				cmi[i]._selRpos += cmi[i]._nbVirtualCaretSpc;
			}
			f(SCI_SETTARGETRANGE, cmi[i]._selLpos, cmi[i]._selRpos);

						UINT cp = static_cast<UINT>(f(SCI_GETCODEPAGE));
			const char *strA = wmc.wchar2char(str, cp);
			f(SCI_REPLACETARGET, static_cast<WPARAM>(-1), reinterpret_cast<LPARAM>(strA));

			if (hasVirtualSpc)	{

				totalDiff += cmi[i]._nbVirtualAnchorSpc + lstrlen(str);
				// Now there's no more virtual space
				cmi[i]._nbVirtualAnchorSpc = 0;
				cmi[i]._nbVirtualCaretSpc = 0;
			}
			else	{

				totalDiff += diff;
			}
			cmi[i]._selRpos += diff;
		}
	}
}


void ScintillaEditView::foldChanged(size_t line, int levelNow, int levelPrev)	{

	if (levelNow & SC_FOLDLEVELHEADERFLAG)	{		//line can be folded

		if (!(levelPrev & SC_FOLDLEVELHEADERFLAG))	{	//but previously couldnt

			// Adding a fold point.
			f(SCI_SETFOLDEXPANDED, line, 1);
			expand(line, true, false, 0, levelPrev);
		}
	}
	else if (levelPrev & SC_FOLDLEVELHEADERFLAG)	{

		if (bool(f(SCI_GETFOLDEXPANDED, line)))	{

			// Removing the fold from one that has been contracted so should expand
			// otherwise lines are left invisible with no way to make them visible
			f(SCI_SETFOLDEXPANDED, line, 1);
			expand(line, true, false, 0, levelPrev);
		}
	}
	else if (!(levelNow & SC_FOLDLEVELWHITEFLAG) &&
	        ((levelPrev & SC_FOLDLEVELNUMBERMASK) > (levelNow & SC_FOLDLEVELNUMBERMASK)))
	{
		// See if should still be hidden
		int parentLine = static_cast<int32_t>(f(SCI_GETFOLDPARENT, line));
		if ((parentLine < 0) || !bool(f(SCI_GETFOLDEXPANDED, parentLine && f(SCI_GETLINEVISIBLE, parentLine))))
			f(SCI_SHOWLINES, line, line);
	}
}


void ScintillaEditView::scrollPosToCenter(size_t pos)	{

	f(SCI_GOTOPOS, pos);
	int line = static_cast<int32_t>(f(SCI_LINEFROMPOSITION, pos));

	int firstVisibleDisplayLine = static_cast<int32_t>(f(SCI_GETFIRSTVISIBLELINE));
	int firstVisibleDocLine = static_cast<int32_t>(f(SCI_DOCLINEFROMVISIBLE, firstVisibleDisplayLine));
	int nbLine = static_cast<int32_t>(f(SCI_LINESONSCREEN));
	int lastVisibleDocLine = static_cast<int32_t>(f(SCI_DOCLINEFROMVISIBLE, firstVisibleDisplayLine + nbLine));

	int middleLine;
	if (line - firstVisibleDocLine < lastVisibleDocLine - line)
		middleLine = firstVisibleDocLine + nbLine/2;
	else
		middleLine = lastVisibleDocLine -  nbLine/2;
	int nbLines2scroll =  line - middleLine;
	scroll(0, nbLines2scroll);
}

void ScintillaEditView::hideLines()	{

	//Folding can screw up hide lines badly if it unfolds a hidden section.
	//Adding runMarkers(hide, foldstart) directly (folding on single document) can help

	//Special func on buffer. If markers are added, create notification with location of start, and hide bool set to true
	int startLine = static_cast<int32_t>(f(SCI_LINEFROMPOSITION, f(SCI_GETSELECTIONSTART)));
	int endLine = static_cast<int32_t>(f(SCI_LINEFROMPOSITION, f(SCI_GETSELECTIONEND)));
	//perform range check: cannot hide very first and very last lines
	//Offset them one off the edges, and then check if they are within the reasonable
	int nbLines = static_cast<int32_t>(f(SCI_GETLINECOUNT));
	if (nbLines < 3)
		return;	//cannot possibly hide anything
	if (!startLine)
		++startLine;
	if (endLine == (nbLines-1))
		--endLine;

	if (startLine > endLine)
		return;	//tried to hide line at edge

	//Hide the lines. We add marks on the outside of the hidden section and hide the lines
	//f(SCI_HIDELINES, startLine, endLine);
	//Add markers
	f(SCI_MARKERADD, startLine-1, MARK_HIDELINESBEGIN);
	f(SCI_MARKERADD, startLine-1, MARK_HIDELINESUNDERLINE);
	f(SCI_MARKERADD, endLine+1, MARK_HIDELINESEND);

	//remove any markers in between
	int scope = 0;
	for (int i = startLine; i <= endLine; ++i)	{

		auto state = f(SCI_MARKERGET, i);
		bool closePresent = ((state & (1 << MARK_HIDELINESEND)) != 0);	//check close first, then open, since close closes scope
		bool openPresent = ((state & (1 << MARK_HIDELINESBEGIN | 1 << MARK_HIDELINESUNDERLINE)) != 0);
		if (closePresent)	{

			f(SCI_MARKERDELETE, i, MARK_HIDELINESEND);
			if (scope > 0) scope--;
		}

		if (openPresent)	{

			f(SCI_MARKERDELETE, i, MARK_HIDELINESBEGIN);
			f(SCI_MARKERDELETE, i, MARK_HIDELINESUNDERLINE);
			++scope;
		}
	}
	if (scope)	{
	//something went wrong
		//Someone managed to make overlapping hidelines sections.
		//We cant do anything since this isnt supposed to happen
	}

	_currentBuffer->setHideLineChanged(true, startLine-1);
}

bool ScintillaEditView::markerMarginClick(int lineNumber)	{

	auto state = f(SCI_MARKERGET, lineNumber);
	bool openPresent = ((state & (1 << MARK_HIDELINESBEGIN | 1 << MARK_HIDELINESUNDERLINE)) != 0);
	bool closePresent = ((state & (1 << MARK_HIDELINESEND)) != 0);

	if (!openPresent && !closePresent)
		return false;

	//Special func on buffer. First call show with location of opening marker. Then remove the marker manually
	if (openPresent)	{

		_currentBuffer->setHideLineChanged(false, lineNumber);
	}

	if (closePresent)	{

		openPresent = false;
		for (lineNumber--; lineNumber >= 0 && !openPresent; --lineNumber )	{

			state = f(SCI_MARKERGET, lineNumber);
			openPresent = ((state & (1 << MARK_HIDELINESBEGIN | 1 << MARK_HIDELINESUNDERLINE)) != 0);
		}

		if (openPresent)	{

			_currentBuffer->setHideLineChanged(false, lineNumber);
		}
	}

	return true;
}

void ScintillaEditView::notifyMarkers(Buffer * buf, bool isHide, int location, bool del)	{

	if (buf != _currentBuffer)	//if not visible buffer dont do a thing
		return;
	runMarkers(isHide, location, false, del);
}

//Run through full document. When switching in or opening folding
//hide is false only when user click on margin
void ScintillaEditView::runMarkers(bool doHide, size_t searchStart, bool endOfDoc, bool doDelete)	{

	//Removes markers if opening
	/*
	AllLines = (start,ENDOFDOCUMENT)
	Hide:
		Run through all lines.
			Find open hiding marker:
				set hiding start
			Find closing:
				if (hiding):
					Hide lines between now and start
					if (endOfDoc = false)
						return
					else
						search for other hidden sections

	Show:
		Run through all lines
			Find open hiding marker
				set last start
			Find closing:
				Show from last start. Stop.
			Find closed folding header:
				Show from last start to folding header
				Skip to LASTCHILD
				Set last start to lastchild
	*/
	size_t maxLines = f(SCI_GETLINECOUNT);
	if (doHide)	{

		auto startHiding = searchStart;
		bool isInSection = false;
		for (auto i = searchStart; i < maxLines; ++i)	{

			auto state = f(SCI_MARKERGET, i);
			if ( ((state & (1 << MARK_HIDELINESEND)) != 0) )	{

				if (isInSection)	{

					f(SCI_HIDELINES, startHiding, i-1);
					if (!endOfDoc)	{

						return;	//done, only single section requested
					}	//otherwise keep going
				}
				isInSection = false;
			}
			if ( ((state & (1 << MARK_HIDELINESBEGIN | 1 << MARK_HIDELINESUNDERLINE)) != 0) )	{

				isInSection = true;
				startHiding = i+1;
			}

		}
	}
	else	{

		auto startShowing = searchStart;
		bool isInSection = false;
		for (auto i = searchStart; i < maxLines; ++i)	{

			auto state = f(SCI_MARKERGET, i);
			if ( ((state & (1 << MARK_HIDELINESEND)) != 0) )	{

				if (doDelete)	{

					f(SCI_MARKERDELETE, i, MARK_HIDELINESEND);
					if (!endOfDoc)	{

						return;	//done, only single section requested
					}	//otherwise keep going
				}
				else if (isInSection)	{

					if (startShowing >= i)	{
	//because of fold skipping, we passed the close tag. In that case we cant do anything
						if (!endOfDoc)	{

							return;
						}
						else	{

							continue;
						}
					}
					f(SCI_SHOWLINES, startShowing, i-1);
					if (!endOfDoc)	{

						return;	//done, only single section requested
					}	//otherwise keep going
					isInSection = false;
				}
			}
			if ( ((state & (1 << MARK_HIDELINESBEGIN | 1 << MARK_HIDELINESUNDERLINE)) != 0) )	{

				if (doDelete)	{

					f(SCI_MARKERDELETE, i, MARK_HIDELINESBEGIN);
					f(SCI_MARKERDELETE, i, MARK_HIDELINESUNDERLINE);
				}
				else	{

					isInSection = true;
					startShowing = i+1;
				}
			}

			auto levelLine = f(SCI_GETFOLDLEVEL, i, 0);
			if (levelLine & SC_FOLDLEVELHEADERFLAG)	{
	//fold section. Dont show lines if fold is closed
				if (isInSection && !bool(f(SCI_GETFOLDEXPANDED, i)))	{

					f(SCI_SHOWLINES, startShowing, i);
					//startShowing = f(SCI_GETLASTCHILD, i, (levelLine & SC_FOLDLEVELNUMBERMASK));
				}
			}
		}
	}
}


void ScintillaEditView::setTabSettings(Lang *lang)	{

	if (lang && lang->_tabSize != -1 && lang->_tabSize != 0)	{

		if (lang->_langID == L_JAVASCRIPT)	{

			Lang *ljs = param.getLangFromID(L_JS);
			f(SCI_SETTABWIDTH, ljs->_tabSize > 0 ? ljs->_tabSize : lang->_tabSize);
			f(SCI_SETUSETABS, !ljs->_isTabReplacedBySpace);
			return;
		}
		f(SCI_SETTABWIDTH, lang->_tabSize);
		f(SCI_SETUSETABS, !lang->_isTabReplacedBySpace);
	}
	else	{

		f(SCI_SETTABWIDTH, nGUI._tabSize  > 0 ? nGUI._tabSize : lang->_tabSize);
		f(SCI_SETUSETABS, !nGUI._tabReplacedBySpace);
	}
}

void ScintillaEditView::insertNewLineAboveCurrentLine()	{

	generic_string newline = getEOLString();
	const auto current_line = getCurrentLineNumber();
	if (!current_line)	{

		// Special handling if caret is at first line.
		insertGenericTextFrom(0, newline.c_str());
	}
	else	{

		const auto eol_length = newline.length();
		const auto position = static_cast<size_t>(f(SCI_POSITIONFROMLINE, current_line)) - eol_length;
		insertGenericTextFrom(position, newline.c_str());
	}
	f(SCI_SETEMPTYSELECTION, f(SCI_POSITIONFROMLINE, current_line));
}


void ScintillaEditView::insertNewLineBelowCurrentLine()	{

	generic_string newline = getEOLString();
	const auto line_count = static_cast<size_t>(f(SCI_GETLINECOUNT));
	const auto current_line = getCurrentLineNumber();
	if (current_line == line_count - 1)	{

		// Special handling if caret is at last line.
		appendGenericText(newline.c_str());
	}
	else	{

		const auto eol_length = newline.length();
		const auto position = eol_length + f(SCI_GETLINEENDPOSITION, current_line);
		insertGenericTextFrom(position, newline.c_str());
	}
	f(SCI_SETEMPTYSELECTION, f(SCI_POSITIONFROMLINE, current_line + 1));
}

void ScintillaEditView::sortLines(size_t fromLine, size_t toLine, ISorter *pSort)	{

	if (fromLine >= toLine)	{

		return;
	}

	const auto startPos = f(SCI_POSITIONFROMLINE, fromLine);
	const auto endPos = f(SCI_POSITIONFROMLINE, toLine) + f(SCI_LINELENGTH, toLine);
	const generic_string text = getGenericTextAsString(startPos, endPos);
	vector<generic_string> splitText = stringSplit(text, getEOLString());
	size_t lineCount = f(SCI_GETLINECOUNT);
	const bool sortEntireDocument = toLine == lineCount - 1;
	if (!sortEntireDocument)	{

		if (splitText.rbegin()->empty())	{

			splitText.pop_back();
		}
	}
	assert(toLine - fromLine + 1 == splitText.size());
	const vector<generic_string> sortedText = pSort->sort(splitText);
	const generic_string joined = stringJoin(sortedText, getEOLString());
	if (sortEntireDocument)	{

		assert(joined.length() == text.length());
		replaceTarget(joined.c_str(), static_cast<int32_t>(startPos), static_cast<int32_t>(endPos));
	}
	else	{

		assert(joined.length() + getEOLString().length() == text.length());
		replaceTarget((joined + getEOLString()).c_str(), static_cast<int32_t>(startPos), static_cast<int32_t>(endPos));
	}
}

bool ScintillaEditView::isTextDirectionRTL() const
{
	long exStyle = static_cast<long>(::GetWindowLongPtr(_hSelf, GWL_EXSTYLE));
	return (exStyle & WS_EX_LAYOUTRTL) != 0;
}

void ScintillaEditView::changeTextDirection(bool isRTL)	{

	long exStyle = static_cast<long>(::GetWindowLongPtr(_hSelf, GWL_EXSTYLE));
	exStyle = isRTL ? exStyle | WS_EX_LAYOUTRTL : exStyle&(~WS_EX_LAYOUTRTL);
	::SetWindowLongPtr(_hSelf, GWL_EXSTYLE, exStyle);
}

generic_string ScintillaEditView::getEOLString()	{

	const int eol_mode = static_cast<int32_t>(f(SCI_GETEOLMODE));
	if (eol_mode == SC_EOL_CRLF)	{

		return L"\r\n";
	}
	else if (eol_mode == SC_EOL_LF)	{

		return L"\n";
	}
	else	{

		return L"\r";
	}
}

void ScintillaEditView::setBorderEdge(bool doWithBorderEdge)	{

	long exStyle = static_cast<long>(::GetWindowLongPtr(_hSelf, GWL_EXSTYLE));

	if (doWithBorderEdge)
		exStyle |= WS_EX_CLIENTEDGE;
	else
		exStyle &= ~WS_EX_CLIENTEDGE;

	::SetWindowLongPtr(_hSelf, GWL_EXSTYLE, exStyle);
	::SetWindowPos(_hSelf, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
}

void ScintillaEditView::getFoldColor(COLORREF& fgColor, COLORREF& bgColor, COLORREF& activeFgColor)	{

	StyleArray & stylers = param.getMiscStylerArray();

	int i = stylers.getStylerIndexByName(L"Fold");
	if (i != -1)	{

		Style & style = stylers.getStyler(i);
		fgColor = style._bgColor;
		bgColor = style._fgColor;
	}
	i = stylers.getStylerIndexByName(L"Fold active");
	if (i != -1)	{
		Style & style = stylers.getStyler(i);
		activeFgColor = style._fgColor;
	}
}

int ScintillaEditView::crUZoption()	{
	if (++nppGUI.caretUZ>7)
		f(SCI_SETYCARETPOLICY, 8,nppGUI.caretUZ=0);
	else
		f(SCI_SETYCARETPOLICY, 13, nGUI.caretUZ);
	f(SCI_SCROLLCARET);
	return nGUI.caretUZ;
}

void ScintillaEditView::foldAll(bool isEXPAND)	{
	SCNotification scnN;
	scnN.nmhdr.code = SCN_FOLDINGSTATECHANGED;
	scnN.nmhdr.hwndFrom = _hSelf;
	scnN.nmhdr.idFrom = 0;
 	for (int line = 0; line< f(SCI_GETLINECOUNT); ++line)
		if (f(SCI_GETFOLDLEVEL, line) & SC_FOLDLEVELHEADERFLAG
		&& bool(f(SCI_GETFOLDEXPANDED, line)) != isEXPAND)	{
			f(SCI_TOGGLEFOLD, line);
			scnN.line = line;
			scnN.foldLevelNow = bool(f(SCI_GETFOLDEXPANDED, line));
			::SendMessage(_hParent, WM_NOTIFY, 0, reinterpret_cast<LPARAM>(&scnN));
		}
}

void ScintillaEditView::putMvmntInView(int p, int e, int prevFound)	{
	auto crVL = f(SCI_VISIBLEFROMDOCLINE, f(SCI_LINEFROMPOSITION,p)),
	preVL = f(SCI_VISIBLEFROMDOCLINE, f(SCI_LINEFROMPOSITION,prevFound)),
	firstVL = f(SCI_GETFIRSTVISIBLELINE),
	lastVL = firstVL + f(SCI_LINESONSCREEN);

	f(SCI_GOTOPOS, p);f(SCI_GOTOPOS, e);
	f(SCI_ENSUREVISIBLE, f(SCI_LINEFROMPOSITION,e));
	f(SCI_SETANCHOR, p);

	if (!prevFound
		|| preVL<firstVL || preVL>lastVL
		|| crVL<firstVL || crVL>lastVL)	{
		f(SCI_SETYCARETPOLICY, 14, 0);f(SCI_SCROLLCARET);
		f(SCI_SETYCARETPOLICY, nGUI.caretUZ? 13: 8, nGUI.caretUZ);
	}
}