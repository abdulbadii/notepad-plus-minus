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

#include <time.h>
#include <shlwapi.h>
#include "Notepad_plus_Window.h"
#include "FileDialog.h"
#include "Printer.h"
#include "FileNameStringSplitter.h"
#include "lesDlgs.h"
#include "Utf8_16.h"
#include "regExtDlg.h"
#include "RunDlg.h"
#include "ShortcutMapper.h"
#include "TaskListDlg.h"
#include "xmlMatchedTagsHighlighter.h"
#include "EncodingMapper.h"
#include "ansiCharPanel.h"
#include "clipboardHistoryPanel.h"
#include "VerticalFileSwitcher.h"
#include "ProjectPanel.h"
#include "documentMap.h"
#include "functionListPanel.h"
#include "fileBrowser.h"
#include "Common.h"

using namespace std;

// int Notepad_plus::rB = 0;
enum tb_stat {tb_saved, tb_unsaved, tb_ro};
#define DIR_LEFT true
#define DIR_RIGHT false

int docTabIconIDs[] = {IDI_SAVED_ICON, IDI_UNSAVED_ICON, IDI_READONLY_ICON, IDI_MONITORING_ICON};

ToolBarButtonUnit toolBarIcons[] = {
	{IDM_FILE_NEW,		IDI_NEW_OFF_ICON,		IDI_NEW_ON_ICON,		IDI_NEW_OFF_ICON, IDR_FILENEW},
	{IDM_FILE_OPEN,		IDI_OPEN_OFF_ICON,		IDI_OPEN_ON_ICON,		IDI_OPEN_OFF_ICON, IDR_FILEOPEN},
	{IDM_FILE_SAVE,		IDI_SAVE_OFF_ICON,		IDI_SAVE_ON_ICON,		IDI_SAVE_DISABLE_ICON, IDR_FILESAVE},
	{IDM_FILE_SAVEALL,	IDI_SAVEALL_OFF_ICON,	IDI_SAVEALL_ON_ICON,	IDI_SAVEALL_DISABLE_ICON, IDR_SAVEALL},
	{IDM_FILE_CLOSE,	IDI_CLOSE_OFF_ICON,		IDI_CLOSE_ON_ICON,		IDI_CLOSE_OFF_ICON, IDR_CLOSEFILE},
	{IDM_FILE_CLOSEALL,	IDI_CLOSEALL_OFF_ICON,	IDI_CLOSEALL_ON_ICON,	IDI_CLOSEALL_OFF_ICON, IDR_CLOSEALL},
	{IDM_FILE_PRINT,	IDI_PRINT_OFF_ICON,		IDI_PRINT_ON_ICON,		IDI_PRINT_OFF_ICON, IDR_PRINT},

	//-------------------------------------------------------------------------------------//
	{0,					IDI_SEPARATOR_ICON,		IDI_SEPARATOR_ICON,		IDI_SEPARATOR_ICON, IDI_SEPARATOR_ICON},
	//-------------------------------------------------------------------------------------//

	{IDM_EDIT_CUT,		IDI_CUT_OFF_ICON,		IDI_CUT_ON_ICON,		IDI_CUT_DISABLE_ICON, IDR_CUT},
	{IDM_EDIT_COPY,		IDI_COPY_OFF_ICON,		IDI_COPY_ON_ICON,		IDI_COPY_DISABLE_ICON, IDR_COPY},
	{IDM_EDIT_PASTE,	IDI_PASTE_OFF_ICON,		IDI_PASTE_ON_ICON,		IDI_PASTE_DISABLE_ICON, IDR_PASTE},

	//-------------------------------------------------------------------------------------//
	{0,					IDI_SEPARATOR_ICON,		IDI_SEPARATOR_ICON,		IDI_SEPARATOR_ICON, IDI_SEPARATOR_ICON},
	//-------------------------------------------------------------------------------------//

	{IDM_EDIT_UNDO,		IDI_UNDO_OFF_ICON,		IDI_UNDO_ON_ICON,		IDI_UNDO_DISABLE_ICON, IDR_UNDO},
	{IDM_EDIT_REDO,		IDI_REDO_OFF_ICON,		IDI_REDO_ON_ICON,		IDI_REDO_DISABLE_ICON, IDR_REDO},
	//-------------------------------------------------------------------------------------//
	{0,					IDI_SEPARATOR_ICON,		IDI_SEPARATOR_ICON,		IDI_SEPARATOR_ICON, IDI_SEPARATOR_ICON},
	//-------------------------------------------------------------------------------------//

	{IDM_SEARCH_REPLACE,  IDI_REPLACE_OFF_ICON,	IDI_REPLACE_ON_ICON,	IDI_REPLACE_OFF_ICON, IDR_REPLACE},

	//-------------------------------------------------------------------------------------//
	{0,					IDI_SEPARATOR_ICON,		IDI_SEPARATOR_ICON,		IDI_SEPARATOR_ICON, IDI_SEPARATOR_ICON},
	//-------------------------------------------------------------------------------------//
	{IDM_VIEW_ZOOMIN,	IDI_ZOOMIN_OFF_ICON,	IDI_ZOOMIN_ON_ICON,		IDI_ZOOMIN_OFF_ICON, IDR_ZOOMIN},
	{IDM_VIEW_ZOOMOUT,	IDI_ZOOMOUT_OFF_ICON,	IDI_ZOOMOUT_ON_ICON,	IDI_ZOOMOUT_OFF_ICON, IDR_ZOOMOUT},

	//-------------------------------------------------------------------------------------//
	{0,					IDI_SEPARATOR_ICON,		IDI_SEPARATOR_ICON,		IDI_SEPARATOR_ICON, IDI_SEPARATOR_ICON},
	//-------------------------------------------------------------------------------------//
	{IDM_VIEW_SYNSCROLLV,	IDI_SYNCV_OFF_ICON,	IDI_SYNCV_ON_ICON,	IDI_SYNCV_DISABLE_ICON, IDR_SYNCV},
	{IDM_VIEW_SYNSCROLLH,	IDI_SYNCH_OFF_ICON,	IDI_SYNCH_ON_ICON,	IDI_SYNCH_DISABLE_ICON, IDR_SYNCH},

	//-------------------------------------------------------------------------------------//
	{0,					IDI_SEPARATOR_ICON,		IDI_SEPARATOR_ICON,		IDI_SEPARATOR_ICON, IDI_SEPARATOR_ICON},
	//-------------------------------------------------------------------------------------//
	{IDM_VIEW_WRAP,     IDI_VIEW_WRAP_OFF_ICON, IDI_VIEW_WRAP_ON_ICON,  IDI_VIEW_WRAP_OFF_ICON, IDR_WRAP},
	{IDM_VIEW_ALL_CHARACTERS,  IDI_VIEW_ALL_CHAR_OFF_ICON,	IDI_VIEW_ALL_CHAR_ON_ICON,	IDI_VIEW_ALL_CHAR_OFF_ICON, IDR_INVISIBLECHAR},
	{IDM_VIEW_INDENT_GUIDE,  IDI_VIEW_INDENT_OFF_ICON,	IDI_VIEW_INDENT_ON_ICON,	IDI_VIEW_INDENT_OFF_ICON, IDR_INDENTGUIDE},
	{IDM_LANG_USER_DLG,  IDI_VIEW_UD_DLG_OFF_ICON,	IDI_VIEW_UD_DLG_ON_ICON,	IDI_VIEW_UD_DLG_OFF_ICON, IDR_SHOWPANNEL},
	{IDM_VIEW_DOC_MAP,  IDI_VIEW_DOC_MAP_OFF_ICON, IDI_VIEW_DOC_MAP_ON_ICON, IDI_VIEW_DOC_MAP_OFF_ICON, IDR_DOCMAP},
	/* {IDM_VIEW_FUNC_LIST,  IDI_VIEW_UD_DLG_OFF_ICON, IDI_VIEW_UD_DLG_ON_ICON, IDI_VIEW_UD_DLG_OFF_ICON, IDR_FUNC_LIST}, */
	{IDM_VIEW_FUNC_LIST,  IDI_VIEW_FUNCLIST_OFF_ICON, IDI_VIEW_FUNCLIST_ON_ICON, IDI_VIEW_FUNCLIST_OFF_ICON, IDR_FUNC_LIST},
	{IDM_VIEW_FILEBROWSER, IDI_VIEW_FILEBROWSER_OFF_ICON, IDI_VIEW_FILEBROWSER_ON_ICON, IDI_VIEW_FILEBROWSER_OFF_ICON, IDR_FILEBROWSER},
	{IDM_VIEW_MONITORING, IDI_VIEW_MONITORING_OFF_ICON, IDI_VIEW_MONITORING_ON_ICON, IDI_VIEW_MONITORING_OFF_ICON, IDR_FILEMONITORING},

	//-------------------------------------------------------------------------------------//
	{0,					IDI_SEPARATOR_ICON,		IDI_SEPARATOR_ICON,		IDI_SEPARATOR_ICON, IDI_SEPARATOR_ICON},
	//-------------------------------------------------------------------------------------//

	{IDM_MACRO_STARTRECORDINGMACRO,		IDI_STARTRECORD_OFF_ICON,	IDI_STARTRECORD_ON_ICON,	IDI_STARTRECORD_DISABLE_ICON, IDR_STARTRECORD},
	{IDM_MACRO_STOPRECORDINGMACRO,		IDI_STOPRECORD_OFF_ICON,	IDI_STOPRECORD_ON_ICON,		IDI_STOPRECORD_DISABLE_ICON, IDR_STOPRECORD},
	{IDM_MACRO_PLAYBACKRECORDEDMACRO,	IDI_PLAYRECORD_OFF_ICON,	IDI_PLAYRECORD_ON_ICON,		IDI_PLAYRECORD_DISABLE_ICON, IDR_PLAYRECORD},
	{IDM_MACRO_RUNMULTIMACRODLG,			IDI_MMPLAY_OFF_ICON,		IDI_MMPLAY_ON_ICON,			IDI_MMPLAY_DIS_ICON, IDR_M_PLAYRECORD},
	{IDM_MACRO_SAVECURRENTMACRO,			IDI_SAVERECORD_OFF_ICON,	IDI_SAVERECORD_ON_ICON,		IDI_SAVERECORD_DISABLE_ICON, IDR_SAVERECORD}
};

NppParameters& param = NppParameters::getInstance();
NppGUI& nppGUI = const_cast<NppGUI &>(param.getNppGUI());
const NppGUI& nGUI = param.getNppGUI();

Notepad_plus::Notepad_plus()
	: _autoCompleteMain(&_mainEditView)
	, _autoCompleteSub(&_subEditView)
	, _smartHighlighter(&_findReplaceDlg)
{
	ZeroMemory(&_prevSelectedRange, sizeof(_prevSelectedRange));

		TiXmlDocumentA *nativeLangDocRootA = param.getNativeLangA();
	_nativeLangSpeaker.init(nativeLangDocRootA);

	LocalizationSwitcher & localizationSwitcher = param.getLocalizationSwitcher();
	const char *fn = _nativeLangSpeaker.getFileName();
	if (fn)	{

		localizationSwitcher.setFileName(fn);
	}

	param.setNativeLangSpeaker(&_nativeLangSpeaker);

	TiXmlDocument *toolIconsDocRoot = param.getToolIcons();

	if (toolIconsDocRoot)		_toolBar.initTheme(toolIconsDocRoot);

	// Determine if user is administrator.
	BOOL is_admin;
	winVer ver = param.getWinVersion();
	if (ver >= WV_VISTA || ver == WV_UNKNOWN)	{

		SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
		PSID AdministratorsGroup;
		is_admin = AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &AdministratorsGroup);
		if (is_admin)	{

			if (!CheckTokenMembership(NULL, AdministratorsGroup, &is_admin))
				is_admin = FALSE;
			FreeSid(AdministratorsGroup);
		}
	}
	else
		is_admin = false;

	param.setAdminMode(is_admin == TRUE);
	_isAdministrator = is_admin ? true : false;
}


Notepad_plus::~Notepad_plus()
{
	// ATTENTION : the order of the destruction is very important
	// because if the parent's window handle is destroyed before
	// the destruction of its children windows' handles,
	// its children windows' handles will be destroyed automatically!

	param.destroyInstance();

	delete _pTrayIco;
	delete _pAnsiCharPanel;
	delete _pClipboardHistoryPanel;
	delete _pFileSwitcherPanel;
	delete _pProjectPanel_1;
	delete _pProjectPanel_2;
	delete _pProjectPanel_3;
	delete _pDocMap;
	delete _pFuncList;
	delete _pFileBrowser;
}

FileManager& MainFileManager = FileManager::getInstance();

LRESULT Notepad_plus::init(HWND hwnd)	{

	// Menu
	_mainMenuHandle = ::GetMenu(hwnd);
	int langPos2BeRemoved = MENUINDEX_LANGUAGE+1;
	if (nppGUI._isLangMenuCompact)
		langPos2BeRemoved = MENUINDEX_LANGUAGE;
	::RemoveMenu(_mainMenuHandle, langPos2BeRemoved, MF_BYPOSITION);

	//Views
	_pDocTab = &_mainDocTab;
	_pEditView = &_mainEditView;
	_pNonDocTab = &_subDocTab;
	_pNonEditView = &_subEditView;

	_mainEditView.init(_pPublicInterface->getHinst(), hwnd);
	_subEditView.init(_pPublicInterface->getHinst(), hwnd);

	_fileEditView.init(_pPublicInterface->getHinst(), hwnd);
	MainFileManager.init(this, &_fileEditView); //get it up and running asap.

	param.setFontList(hwnd);
	nppGUI.pNpp = this;

	_mainWindowStatus = WindowMainActive;
	// _activeView = MAIN_VIEW;

	const ScintillaViewParams & svp1 = param.getSVP();

	int tabBarStatus = nppGUI._tabStatus;
	_toReduceTabBar = ((tabBarStatus & TAB_REDUCE) != 0);
	int iconDpiDynamicalSize = param._dpiManager.scaleY(_toReduceTabBar?13:20);
	_docTabIconList.create(iconDpiDynamicalSize, _pPublicInterface->getHinst(), docTabIconIDs, sizeof(docTabIconIDs)/sizeof(int));

	_mainDocTab.init(_pPublicInterface->getHinst(), hwnd, &_mainEditView, &_docTabIconList);
	_subDocTab.init(_pPublicInterface->getHinst(), hwnd, &_subEditView, &_docTabIconList);

	_mainEditView.display();

	_invisibleEditView.init(_pPublicInterface->getHinst(), hwnd);
	_invisibleEditView.f(SCI_SETUNDOCOLLECTION);
	_invisibleEditView.f(SCI_EMPTYUNDOBUFFER);
	_invisibleEditView.wrap(false); // Make sure no slow down

	// Configuration of 2 scintilla views
	_mainEditView.showMargin(ScintillaEditView::_SC_MARGE_LINENUMBER, svp1._lineNumberMarginShow);
	_subEditView.showMargin(ScintillaEditView::_SC_MARGE_LINENUMBER, svp1._lineNumberMarginShow);
	_mainEditView.showMargin(ScintillaEditView::_SC_MARGE_SYBOLE, svp1._bookMarkMarginShow);
	_subEditView.showMargin(ScintillaEditView::_SC_MARGE_SYBOLE, svp1._bookMarkMarginShow);

	_mainEditView.showIndentGuideLine(svp1._indentGuideLineShow);
	_subEditView.showIndentGuideLine(svp1._indentGuideLineShow);

	::SendMessage(hwnd, NPPM_INTERNAL_SETCARETWIDTH, 0, 0);
	::SendMessage(hwnd, NPPM_INTERNAL_SETCARETBLINKRATE, 0, 0);

	_configStyleDlg.init(_pPublicInterface->getHinst(), hwnd);
	_preference.init(_pPublicInterface->getHinst(), hwnd);
	_pluginsAdminDlg.init(_pPublicInterface->getHinst(), hwnd);

	//Marker Margin config
	_mainEditView.setMakerStyle(svp1._folderStyle);
	_subEditView.setMakerStyle(svp1._folderStyle);
	_mainEditView.defineDocType(_mainEditView.getCurrentBuffer()->getLangType());
	_subEditView.defineDocType(_subEditView.getCurrentBuffer()->getLangType());

	//Line wrap method
	_mainEditView.setWrapMode(svp1._lineWrapMethod);
	_subEditView.setWrapMode(svp1._lineWrapMethod);

	// _mainEditView.f(SCI_SETCARETLINEVISIBLE, svp1._currentLineHilitingShow);
	// _subEditView.f(SCI_SETCARETLINEVISIBLE, svp1._currentLineHilitingShow);

	_mainEditView.f(SCI_SETENDATLASTLINE, not svp1._scrollBeyondLastLine);
	_subEditView.f(SCI_SETENDATLASTLINE, not svp1._scrollBeyondLastLine);

	if (svp1._doSmoothFont)	{

		_mainEditView.f(SCI_SETFONTQUALITY, SC_EFF_QUALITY_LCD_OPTIMIZED);
		_subEditView.f(SCI_SETFONTQUALITY, SC_EFF_QUALITY_LCD_OPTIMIZED);
	}

	_mainEditView.setBorderEdge(svp1._showBorderEdge);
	_subEditView.setBorderEdge(svp1._showBorderEdge);

	_mainEditView.f(SCI_SETCARETLINEVISIBLEALWAYS, true);
	_subEditView.f(SCI_SETCARETLINEVISIBLEALWAYS, true);

	_mainEditView.wrap(svp1._doWrap);
	_subEditView.wrap(svp1._doWrap);

	_mainEditView.f(SCI_SETEDGECOLUMN, svp1._edgeNbColumn);
	_mainEditView.f(SCI_SETEDGEMODE, svp1._edgeMode);
	_subEditView.f(SCI_SETEDGECOLUMN, svp1._edgeNbColumn);
	_subEditView.f(SCI_SETEDGEMODE, svp1._edgeMode);

	_mainEditView.showEOL(svp1._eolShow);
	_subEditView.showEOL(svp1._eolShow);

	_mainEditView.showWSAndTab(svp1._whiteSpaceShow);
	_subEditView.showWSAndTab(svp1._whiteSpaceShow);

	_mainEditView.showWrapSymbol(svp1._wrapSymbolShow);
	_subEditView.showWrapSymbol(svp1._wrapSymbolShow);

	_mainEditView.performGlobalStyles();
	_subEditView.performGlobalStyles();

	_zoomOriginalValue = int(_pEditView->f(SCI_GETZOOM));
	_mainEditView.f(SCI_SETZOOM, svp1._zoom);
	_subEditView.f(SCI_SETZOOM, svp1._zoom2);

	::SendMessage(hwnd, NPPM_INTERNAL_SETMULTISELCTION, 0, 0);

	// Make backspace or delete work with multiple selections
	_mainEditView.f(SCI_SETADDITIONALSELECTIONTYPING, true);
	_subEditView.f(SCI_SETADDITIONALSELECTIONTYPING, true);

	// Turn virtual space on
	_mainEditView.f(SCI_SETVIRTUALSPACEOPTIONS, SCVS_RECTANGULARSELECTION);
	_subEditView.f(SCI_SETVIRTUALSPACEOPTIONS, SCVS_RECTANGULARSELECTION);

	// Turn multi-paste on
	_mainEditView.f(SCI_SETMULTIPASTE, SC_MULTIPASTE_EACH);
	_subEditView.f(SCI_SETMULTIPASTE, SC_MULTIPASTE_EACH);

	// Let Scintilla deal with some of the folding functionality
	_mainEditView.f(SCI_SETAUTOMATICFOLD, SC_AUTOMATICFOLD_SHOW);
	_subEditView.f(SCI_SETAUTOMATICFOLD, SC_AUTOMATICFOLD_SHOW);

	TabBarPlus::doDragNDrop(true);

	if (_toReduceTabBar)	{

		HFONT hf = static_cast<HFONT>(::GetStockObject(DEFAULT_GUI_FONT));

		if (hf)	{

			::SendMessage(_mainDocTab.getHSelf(), WM_SETFONT, reinterpret_cast<WPARAM>(hf), MAKELPARAM(TRUE, 0));
			::SendMessage(_subDocTab.getHSelf(), WM_SETFONT, reinterpret_cast<WPARAM>(hf), MAKELPARAM(TRUE, 0));
		}
		int tabDpiDynamicalHeight = param._dpiManager.scaleY(22);
		int tabDpiDynamicalWidth = param._dpiManager.scaleX(45);
		TabCtrl_SetItemSize(_mainDocTab.getHSelf(), tabDpiDynamicalWidth, tabDpiDynamicalHeight);
		TabCtrl_SetItemSize(_subDocTab.getHSelf(), tabDpiDynamicalWidth, tabDpiDynamicalHeight);
	}
	_mainDocTab.display();


	TabBarPlus::doDragNDrop((tabBarStatus & TAB_DRAGNDROP) != 0);
	TabBarPlus::setDrawTopBar((tabBarStatus & TAB_DRAWTOPBAR) != 0);
	TabBarPlus::setDrawInactiveTab((tabBarStatus & TAB_DRAWINACTIVETAB) != 0);
	TabBarPlus::setDrawTabCloseButton((tabBarStatus & TAB_CLOSEBUTTON) != 0);
	TabBarPlus::setDbClk2Close((tabBarStatus & TAB_DBCLK2CLOSE) != 0);
	TabBarPlus::setVertical((tabBarStatus & TAB_VERTICAL) != 0);
	drawTabbarColoursFromStylerArray();

	//--Splitter Section--//
	bool isVertical = (nGUI._splitterPos == POS_VERTICAL);

	_subSplitter.init(_pPublicInterface->getHinst(), hwnd);
	_subSplitter.create(&_mainDocTab, &_subDocTab, 5, SplitterMode::DYNAMIC, 50, isVertical);

	//--Status Bar Section--//

	_statusBar.init(_pPublicInterface->getHinst(), hwnd, NB_SB);
	_statusBar.setPartWidth(STATUSBAR_DOC_NAME, param._dpiManager.scaleX(143));
	_statusBar.setPartWidth(STATUSBAR_CUR_POS, param._dpiManager.scaleX(225));
	_statusBar.setPartWidth(STATUSBAR_INFOS, param._dpiManager.scaleX(220));
	_statusBar.setPartWidth(STATUSBAR_DOC_SIZE, param._dpiManager.scaleX(91));
	_statusBar.setPartWidth(STATUSBAR_EOF_FORMAT, param._dpiManager.scaleX(52));
	_statusBar.setPartWidth(STATUSBAR_ENCODING, param._dpiManager.scaleX(89));
	_statusBar.setPartWidth(STATUSBAR_SEL_PASTE, param._dpiManager.scaleX(31));
	_statusBar.setPartWidth(STATUSBAR_SEL_UNDO, param._dpiManager.scaleX(18));
	_statusBar.setPartWidth(STATUSBAR_CR_UZ, param._dpiManager.scaleX(12));
	_statusBar.setPartWidth(STATUSBAR_TYPING_MODE, param._dpiManager.scaleX(26));
	_statusBar.display(nppGUI._statusBarShow);
	
	_statusBar.setText(STATUSBAR_SEL_PASTE,L"LOSE");
	_statusBar.setText(STATUSBAR_SEL_UNDO,L"LS");
	_pEditView->f(SCI_SETOVERTYPE,0);_statusBar.setText(STATUSBAR_TYPING_MODE,L"INS");
	_statusBar.setText(STATUSBAR_CR_UZ,to_wstring(nGUI.caretUZ).c_str());
	_pEditView->f(SCI_SETYCARETPOLICY, 13, nGUI.caretUZ);
	_pEditView->f(SCI_SCROLLCARET);

	_pMainWindow = &_mainDocTab;
	_dockingManager.init(_pPublicInterface->getHinst(), hwnd, &_pMainWindow);

	if (nGUI._isMinimizedToTray &&!_pTrayIco)
		_pTrayIco = new trayIconControler(hwnd, IDI_M30ICON, IDC_MINIMIZED_TRAY,
		::LoadIcon(_pPublicInterface->getHinst(), MAKEINTRESOURCE(IDI_M30ICON)), L"");

	checkSyncState();

	// Plugin Manager
	NppData nppData;
	nppData._nppHandle = hwnd;
	nppData._scintillaMainHandle = _mainEditView.getHSelf();
	nppData._scintillaSecondHandle = _subEditView.getHSelf();

	_scintillaCtrls4Plugins.init(_pPublicInterface->getHinst(), hwnd);
	_pluginsManager.init(nppData);

	_pluginsManager.loadPluginsV2(param.getPluginRootDir());
	_restoreButton.init(_pPublicInterface->getHinst(), hwnd);

	// ------------ //
	// Menu Section //
	// ------------ //

	// Macro Menu
	std::vector<MacroShortcut> & macros  = param.getMacroList();
	HMENU hMacroMenu = ::GetSubMenu(_mainMenuHandle, MENUINDEX_MACRO);
	size_t const posBase = 6;
	size_t nbMacro = macros.size();
	if (nbMacro >= 1)
		::InsertMenu(hMacroMenu, posBase - 1, MF_BYPOSITION, static_cast<UINT>(-1), 0);

	for (size_t i = 0 ; i < nbMacro ; ++i)
		::InsertMenu(hMacroMenu, static_cast<UINT>(posBase + i), MF_BYPOSITION, ID_MACRO + i, macros[i].toMenuItemString().c_str());

	if (nbMacro >= 1)	{

		::InsertMenu(hMacroMenu, static_cast<UINT>(posBase + nbMacro + 1), MF_BYPOSITION, static_cast<UINT>(-1), 0);
		::InsertMenu(hMacroMenu, static_cast<UINT>(posBase + nbMacro + 2), MF_BYCOMMAND, IDM_SETTING_SHORTCUT_MAPPER_MACRO, L"Modify Shortcut/Delete Macro...");
	}

	// Run Menu
	std::vector<UserCommand> & userCommands = param.getUserCommandList();
	HMENU hRunMenu = ::GetSubMenu(_mainMenuHandle, MENUINDEX_RUN);
	int const runPosBase = 2;
	size_t nbUserCommand = userCommands.size();
	if (nbUserCommand >= 1)
		::InsertMenu(hRunMenu, runPosBase - 1, MF_BYPOSITION, static_cast<UINT>(-1), 0);

	for (size_t i = 0 ; i < nbUserCommand ; ++i)	{

		::InsertMenu(hRunMenu, static_cast<UINT>(runPosBase + i), MF_BYPOSITION, ID_USER_CMD + i, userCommands[i].toMenuItemString().c_str());
	}

	if (nbUserCommand >= 1)	{

		::InsertMenu(hRunMenu, static_cast<UINT>(runPosBase + nbUserCommand + 1), MF_BYPOSITION,  static_cast<UINT>(-1), 0);
		::InsertMenu(hRunMenu, static_cast<UINT>(runPosBase + nbUserCommand + 2), MF_BYCOMMAND, IDM_SETTING_SHORTCUT_MAPPER_RUN, L"Modify Shortcut/Delete Command...");
	}

	// Updater menu item
	if (!nppGUI._doesExistUpdater)	{

		::DeleteMenu(_mainMenuHandle, IDM_UPDATE_NPP, MF_BYCOMMAND);
		::DeleteMenu(_mainMenuHandle, IDM_CONFUPDATERPROXY, MF_BYCOMMAND);
		HMENU hHelpMenu = ::GetSubMenu(_mainMenuHandle, MENUINDEX_PLUGINS + 1);
		if (!hHelpMenu)
			hHelpMenu = ::GetSubMenu(_mainMenuHandle, MENUINDEX_PLUGINS);
		if (hHelpMenu)
			::DeleteMenu(hHelpMenu, 7, MF_BYPOSITION); // SEPARATOR
		::DrawMenuBar(hwnd);
	}
	//Languages Menu
	HMENU hLangMenu = ::GetSubMenu(_mainMenuHandle, MENUINDEX_LANGUAGE);

	// Add external languages to menu
	for (int i = 0 ; i < param.getNbExternalLang() ; ++i)	{

		ExternalLangContainer & externalLangContainer = param.getELCFromIndex(i);

		int numLangs = ::GetMenuItemCount(hLangMenu);
		constexpr int bufferSize = 100;
		TCHAR buffer[bufferSize];

		int x;
		for (x = 0; (!x || lstrcmp(externalLangContainer._name, buffer) > 0) && x < numLangs; ++x)	{

			::GetMenuString(hLangMenu, x, buffer, bufferSize, MF_BYPOSITION);
		}

		::InsertMenu(hLangMenu, x-1, MF_BYPOSITION, IDM_LANG_EXTERNAL + i, externalLangContainer._name);
	}

	if (nppGUI._excludedLangList.size() > 0)	{

		for (size_t i = 0, len = nppGUI._excludedLangList.size(); i < len ; ++i)	{

			int cmdID = param.langTypeToCommandID(nppGUI._excludedLangList[i]._langType);
			constexpr int itemSize = 256;
			TCHAR itemName[itemSize];
			::GetMenuString(hLangMenu, cmdID, itemName, itemSize, MF_BYCOMMAND);
			nppGUI._excludedLangList[i]._cmdID = cmdID;
			nppGUI._excludedLangList[i]._langName = itemName;
			::DeleteMenu(hLangMenu, cmdID, MF_BYCOMMAND);
			DrawMenuBar(hwnd);
		}
	}

	// Add User Defined Languages Entry
	int udlpos = ::GetMenuItemCount(hLangMenu) - 1;

	for (int i = 0, len = param.getNbUserLang(); i < len ; ++i)	{

		UserLangContainer & userLangContainer = param.getULCFromIndex(i);
		::InsertMenu(hLangMenu, udlpos + i, MF_BYPOSITION, IDM_LANG_USER + i + 1, userLangContainer.getName());
	}

	//Add recent files
	HMENU hFileMenu = ::GetSubMenu(_mainMenuHandle, MENUINDEX_FILE);
	int nbLRFile = param.getNbLRFile();
	//int pos = IDM_FILEMENU_LASTONE - IDM_FILE + 1 /* +1 : because of  IDM_FILE_PRINTNOW */;

	_lastRecentFileList.initMenu(hFileMenu, IDM_FILEMENU_LASTONE + 1, IDM_FILEMENU_EXISTCMDPOSITION, &_accelerator, param.putRecentFileInSubMenu());
	_lastRecentFileList.setLangEncoding(_nativeLangSpeaker.getLangEncoding());
	for (int i = 0 ; i < nbLRFile ; ++i)	{

		generic_string * stdStr = param.getLRFile(i);
		if (!nppGUI._checkHistoryFiles || PathFileExists(stdStr->c_str()))	{

			_lastRecentFileList.add(stdStr->c_str());
		}
	}

	//Plugin menu
	bool enablePluginAdmin = _pluginsAdminDlg.isValide();
	_pluginsAdminDlg.setPluginsManager(&_pluginsManager);
	_pluginsManager.setMenu(_mainMenuHandle, NULL, enablePluginAdmin);


	//Main menu is loaded, now load context menu items
	param.getContextMenuFromXmlTree(_mainMenuHandle, _pluginsManager.getMenuHandle());

	if (param.hasCustomContextMenu())	{

		_mainEditView.f(SCI_USEPOPUP, FALSE);
		_subEditView.f(SCI_USEPOPUP, FALSE);
	}

	generic_string pluginsTrans, windowTrans;
	_nativeLangSpeaker.changeMenuLang(_mainMenuHandle, pluginsTrans, windowTrans);
	::DrawMenuBar(hwnd);


	if (pluginsTrans != L"")	{

		::ModifyMenu(_mainMenuHandle, MENUINDEX_PLUGINS, MF_BYPOSITION, 0, pluginsTrans.c_str());
	}
	//Windows menu
	_windowsMenu.init(_pPublicInterface->getHinst(), _mainMenuHandle, windowTrans.c_str());

	// Update context menu strings (translated)
	vector<MenuItemUnit> & tmp = param.getContextMenuItems();
	size_t len = tmp.size();
	TCHAR menuName[64];
	for (size_t i = 0 ; i < len ; ++i)	{

		if (tmp[i]._itemName.empty())	{

			::GetMenuString(_mainMenuHandle, tmp[i]._cmdID, menuName, 64, MF_BYCOMMAND);
			tmp[i]._itemName = purgeMenuItemString(menuName);
		}
	}

	//Input all the menu item names into shortcut list
	//This will automatically do all translations, since menu translation has been done already
	vector<CommandShortcut> & shortcuts = param.getUserShortcuts();
	len = shortcuts.size();

	for (size_t i = 0; i < len; ++i)	{

		CommandShortcut & csc = shortcuts[i];
		if (!csc.getName()[0])	{
	//no predefined name, get name from menu and use that
			::GetMenuString(_mainMenuHandle, csc.getID(), menuName, 64, MF_BYCOMMAND);
			csc.setName(purgeMenuItemString(menuName, true).c_str());
		}
		else	{

			// The menu name is already present (e.g. "Restore recent close file")
			// Now get the localized name if possible
			generic_string localizedMenuName = _nativeLangSpeaker.getNativeLangMenuString(csc.getID());
			if(!localizedMenuName.empty())
				csc.setName(purgeMenuItemString(localizedMenuName.c_str(), true).c_str());
		}
	}
	//Translate non-menu shortcuts
	_nativeLangSpeaker.changeShortcutLang();

	//Update plugin shortcuts, all plugin commands should be available now
	param.reloadPluginCmds();

	// Shortcut Accelerator : should be the last one since it will capture all the shortcuts
	_accelerator.init(_mainMenuHandle, hwnd);
	param.setAccelerator(&_accelerator);

	// Scintilla key accelerator
	vector<HWND> scints;
	scints.push_back(_mainEditView.getHSelf());
	scints.push_back(_subEditView.getHSelf());
	_scintaccelerator.init(&scints, _mainMenuHandle, hwnd);

	param.setScintillaAccelerator(&_scintaccelerator);
	_scintaccelerator.updateKeys();

	::DrawMenuBar(hwnd);

	//-- Tool Bar Section --//
	toolBarStatusType tbStatus = nppGUI._toolBarStatus;

	// To notify plugins that toolbar icons can be registered
	SCNotification scnN;
	scnN.nmhdr.code = NPPN_TBMODIFICATION;
	scnN.nmhdr.hwndFrom = hwnd;
	scnN.nmhdr.idFrom = 0;
	_pluginsManager.notify(&scnN);

	_toolBar.init(_pPublicInterface->getHinst(), hwnd, tbStatus, toolBarIcons, sizeof(toolBarIcons)/sizeof(ToolBarButtonUnit));

	changeToolBarIcons();

	_rebarTop.init(_pPublicInterface->getHinst(), hwnd);
	_rebarBottom.init(_pPublicInterface->getHinst(), hwnd);
	_toolBar.addToRebar(&_rebarTop);
	_rebarTop.setIDVisible(REBAR_BAR_TOOLBAR, nppGUI._toolbarShow);

	checkMacroState();

	//--Init dialogs--//
	_findReplaceDlg.init(_pPublicInterface->getHinst(), hwnd, &_pEditView);
	_findInFinderDlg.init(_pPublicInterface->getHinst(), hwnd);
	_incrementFindDlg.init(_pPublicInterface->getHinst(), hwnd, &_findReplaceDlg, _nativeLangSpeaker.isRTL());
	_incrementFindDlg.addToRebar(&_rebarBottom);
	_goToLineDlg.init(_pPublicInterface->getHinst(), hwnd, &_pEditView);
	_findCharsInRangeDlg.init(_pPublicInterface->getHinst(), hwnd, &_pEditView);
	_colEditorDlg.init(_pPublicInterface->getHinst(), hwnd, &_pEditView);
	_aboutDlg.init(_pPublicInterface->getHinst(), hwnd);
	_debugInfoDlg.init(_pPublicInterface->getHinst(), hwnd, _isAdministrator, _pluginsManager.getLoadedPluginNames());
	_runDlg.init(_pPublicInterface->getHinst(), hwnd);
	_runMacroDlg.init(_pPublicInterface->getHinst(), hwnd);
	_documentPeeker.init(_pPublicInterface->getHinst(), hwnd);

	_md5FromFilesDlg.init(_pPublicInterface->getHinst(), hwnd);
	_md5FromFilesDlg.setHashType(hash_md5);
	_md5FromTextDlg.init(_pPublicInterface->getHinst(), hwnd);
	_md5FromTextDlg.setHashType(hash_md5);
	_sha2FromFilesDlg.init(_pPublicInterface->getHinst(), hwnd);
	_sha2FromFilesDlg.setHashType(hash_sha256);
	_sha2FromTextDlg.init(_pPublicInterface->getHinst(), hwnd);
	_sha2FromTextDlg.setHashType(hash_sha256);


	//--User Define Dialog Section--//
	int uddStatus = nppGUI._userDefineDlgStatus;
	UserDefineDialog *udd = _pEditView->getUserDefineDlg();

	bool uddShow = false;
	switch (uddStatus)	{

		case UDD_SHOW:	{ // show & undocked

			udd->doDialog(true, _nativeLangSpeaker.isRTL());
			_nativeLangSpeaker.changeUserDefineLang(udd);
			uddShow = true;
			break;
		}
		case UDD_DOCKED:	{ // hide & docked

			_isUDDocked = true;
				break;
		}
		case (UDD_SHOW | UDD_DOCKED) :    // show & docked
		{
			udd->doDialog(true, _nativeLangSpeaker.isRTL());
			_nativeLangSpeaker.changeUserDefineLang(udd);
			::SendMessage(udd->getHSelf(), WM_COMMAND, IDC_DOCK_BUTTON, 0);
			uddShow = true;
			break;
		}

		default: // hide & undocked
			break;
	}

	// checkMenuItem(IDM_VIEW_LINENUMBER, param.lineNumberMarginOn());

	// Menu & toolbar for UserDefine Dialog
	checkMenuItem(IDM_LANG_USER_DLG, uddShow);
	_toolBar.setCheck(IDM_LANG_USER_DLG, uddShow);

	// Initialize the default foreground & background color
	{
		StyleArray & globalStyles = param.getGlobalStylers();
		int i = globalStyles.getStylerIndexByID(STYLE_DEFAULT);
		if (i != -1)	{

			Style & style = globalStyles.getStyler(i);
			param.setCurrentDefaultFgColor(style._fgColor);
			param.setCurrentDefaultBgColor(style._bgColor);
		}
	}

	//
	// launch the plugin dlg memorized at the last session
	//
	DockingManagerData& dmd = nppGUI._dockingData;

	_dockingManager.setDockedContSize(CONT_LEFT  , nppGUI._dockingData._leftWidth);
	_dockingManager.setDockedContSize(CONT_RIGHT , nppGUI._dockingData._rightWidth);
	_dockingManager.setDockedContSize(CONT_TOP	 , nppGUI._dockingData._topHeight);
	_dockingManager.setDockedContSize(CONT_BOTTOM, nppGUI._dockingData._bottomHight);

	for (size_t i = 0, len = dmd._pluginDockInfo.size(); i < len ; ++i)	{

		PluginDlgDockingInfo& pdi = dmd._pluginDockInfo[i];
		if (pdi._isVisible)	{

			if (pdi._name == NPP_INTERNAL_FUCTION_STR)
				_internalFuncIDs.push_back(pdi._internalID);
			else
				_pluginsManager.runPluginCommand(pdi._name.c_str(), pdi._internalID);
		}
	}

	for (size_t i = 0, len = dmd._containerTabInfo.size(); i < len; ++i)	{

		ContainerTabInfo & cti = dmd._containerTabInfo[i];
		_dockingManager.setActiveTab(cti._cont, cti._activeTab);
	}

	//Load initial docs into doctab
	loadBufferIntoView(_mainEditView.getCurrentBufferID(), MAIN_VIEW);
	loadBufferIntoView(_subEditView.getCurrentBufferID(), SUB_VIEW);
	activateBuffer(_mainEditView.getCurrentBufferID(), MAIN_VIEW);
	activateBuffer(_subEditView.getCurrentBufferID(), SUB_VIEW);
	//::SetFocus(_mainEditView.getHSelf());
	_mainEditView.focus();

	return TRUE;
}

void Notepad_plus::killAllChildren()	{

	_toolBar.destroy();
	_rebarTop.destroy();
	_rebarBottom.destroy();

	if (_pMainSplitter)	{

		_pMainSplitter->destroy();
		delete _pMainSplitter;
	}

	_mainDocTab.destroy();
	_subDocTab.destroy();

	_mainEditView.destroy();
	_subEditView.destroy();
	_invisibleEditView.destroy();

	_subSplitter.destroy();
	_statusBar.destroy();

	_scintillaCtrls4Plugins.destroy();
	_dockingManager.destroy();
}

bool Notepad_plus::saveGUIParams()	{

	
	nppGUI._toolbarShow = _rebarTop.getIDVisible(REBAR_BAR_TOOLBAR);
	nppGUI._toolBarStatus = _toolBar.getState();

	nppGUI._tabStatus = (TabBarPlus::doDragNDropOrNot()?TAB_DRAWTOPBAR:0) | \
						(TabBarPlus::drawTopBar()?TAB_DRAGNDROP:0) | \
						(TabBarPlus::drawInactiveTab()?TAB_DRAWINACTIVETAB:0) | \
						(_toReduceTabBar?TAB_REDUCE:0) | \
						(TabBarPlus::drawTabCloseButton()?TAB_CLOSEBUTTON:0) | \
						(TabBarPlus::isDbClk2Close()?TAB_DBCLK2CLOSE:0) | \
						(TabBarPlus::isVertical() ? TAB_VERTICAL:0) | \
						(TabBarPlus::isMultiLine() ? TAB_MULTILINE:0) |\
						(nppGUI._tabStatus & TAB_HIDE) | \
						(nppGUI._tabStatus & TAB_QUITONEMPTY);
	nppGUI._splitterPos = _subSplitter.isVertical()?POS_VERTICAL:POS_HORIZOTAL;
	UserDefineDialog *udd = _pEditView->getUserDefineDlg();
	bool b = udd->isDocked();
	nppGUI._userDefineDlgStatus = (b?UDD_DOCKED:0) | (udd->isVisible()?UDD_SHOW:0);

	// When window is maximized GetWindowPlacement will return window's last non maximized coordinates.
	// Save them so that those will be used when window is restored next time.
	WINDOWPLACEMENT posInfo;
	posInfo.length = sizeof(WINDOWPLACEMENT);
	::GetWindowPlacement(_pPublicInterface->getHSelf(), &posInfo);

	nppGUI._appPos.left   = posInfo.rcNormalPosition.left;
	nppGUI._appPos.top    = posInfo.rcNormalPosition.top;
	nppGUI._appPos.right  = posInfo.rcNormalPosition.right - posInfo.rcNormalPosition.left;
	nppGUI._appPos.bottom = posInfo.rcNormalPosition.bottom - posInfo.rcNormalPosition.top;
	nppGUI._isMaximized = ((IsZoomed(_pPublicInterface->getHSelf()) != 0) || (posInfo.flags & WPF_RESTORETOMAXIMIZED));

	saveDockingParams();
	param.createXmlTreeFromGUIParams();
	return true;
}

bool Notepad_plus::saveProjectPanelsParams()	{

	if (_pProjectPanel_1)	{

		_pProjectPanel_1->checkIfNeedSave(L"Project Panel 1");
		param.setWorkSpaceFilePath(0, _pProjectPanel_1->getWorkSpaceFilePath());
	}
	if (_pProjectPanel_2)	{

		_pProjectPanel_2->checkIfNeedSave(L"Project Panel 2");
		param.setWorkSpaceFilePath(1, _pProjectPanel_2->getWorkSpaceFilePath());
	}
	if (_pProjectPanel_3)	{

		_pProjectPanel_3->checkIfNeedSave(L"Project Panel 3");
		param.setWorkSpaceFilePath(2, _pProjectPanel_3->getWorkSpaceFilePath());
	}
	return param.writeProjectPanelsSettings();
}

bool Notepad_plus::saveFileBrowserParam()	{

	if (_pFileBrowser)	{

		vector<generic_string> rootPaths = _pFileBrowser->getRoots();
		generic_string selectedItemPath = _pFileBrowser->getSelectedItemPath();
		return param.writeFileBrowserSettings(rootPaths, selectedItemPath);
	}
	return true; // nothing to save so true is returned
}

void Notepad_plus::saveDockingParams()	{

	

	// Save the docking information
	nppGUI._dockingData._leftWidth		= _dockingManager.getDockedContSize(CONT_LEFT);
	nppGUI._dockingData._rightWidth		= _dockingManager.getDockedContSize(CONT_RIGHT);
	nppGUI._dockingData._topHeight		= _dockingManager.getDockedContSize(CONT_TOP);
	nppGUI._dockingData._bottomHight	= _dockingManager.getDockedContSize(CONT_BOTTOM);

	// clear the container tab information (active tab)
	nppGUI._dockingData._containerTabInfo.clear();

	// create a vector to save the current information
	vector<PluginDlgDockingInfo>	vPluginDockInfo;
	vector<FloatingWindowInfo>		vFloatingWindowInfo;

	// save every container
	vector<DockingCont*> vCont = _dockingManager.getContainerInfo();

	for (size_t i = 0, len = vCont.size(); i < len ; ++i)	{

		// save at first the visible Tb's
		vector<tTbData *>	vDataVis	= vCont[i]->getDataOfVisTb();

		for (size_t j = 0, len2 = vDataVis.size(); j < len2 ; ++j)	{

			if (vDataVis[j]->pszName && vDataVis[j]->pszName[0])	{

				PluginDlgDockingInfo pddi(vDataVis[j]->pszModuleName, vDataVis[j]->dlgID, int32_t(i), vDataVis[j]->iPrevCont, true);
				vPluginDockInfo.push_back(pddi);
			}
		}

		// save the hidden Tb's
		vector<tTbData *>	vDataAll	= vCont[i]->getDataOfAllTb();

		for (size_t j = 0, len3 = vDataAll.size(); j < len3 ; ++j)	{

			if ((vDataAll[j]->pszName && vDataAll[j]->pszName[0]) && (!vCont[i]->isTbVis(vDataAll[j])))	{

				PluginDlgDockingInfo pddi(vDataAll[j]->pszModuleName, vDataAll[j]->dlgID, int32_t(i), vDataAll[j]->iPrevCont, false);
				vPluginDockInfo.push_back(pddi);
			}
		}

		// save the position, when container is a floated one
		if (i >= DOCKCONT_MAX)	{

			RECT	rc;
			vCont[i]->getWindowRect(rc);
			FloatingWindowInfo fwi(int32_t(i), rc.left, rc.top, rc.right, rc.bottom);
			vFloatingWindowInfo.push_back(fwi);
		}

		// save the active tab
		ContainerTabInfo act(int32_t(i), vCont[i]->getActiveTb());
		nppGUI._dockingData._containerTabInfo.push_back(act);
	}

	// add the missing information and store it in nppGUI
	UCHAR floatContArray[50];
	memset(floatContArray, 0, 50);

	for (size_t i = 0, len4 = nppGUI._dockingData._pluginDockInfo.size(); i < len4 ; ++i)	{

		BOOL	isStored = FALSE;
		for (size_t j = 0, len5 = vPluginDockInfo.size(); j < len5; ++j)	{

			if (nppGUI._dockingData._pluginDockInfo[i] == vPluginDockInfo[j])	{

				isStored = TRUE;
				break;
			}
		}

		if (isStored == FALSE)	{

			int floatCont	= 0;

			if (nppGUI._dockingData._pluginDockInfo[i]._currContainer >= DOCKCONT_MAX)
				floatCont = nppGUI._dockingData._pluginDockInfo[i]._currContainer;
			else
				floatCont = nppGUI._dockingData._pluginDockInfo[i]._prevContainer;

			if (!floatContArray[floatCont])	{

				RECT rc;
				if (nppGUI._dockingData.getFloatingRCFrom(floatCont, rc))	{

					vFloatingWindowInfo.push_back(FloatingWindowInfo(floatCont, rc.left, rc.top, rc.right, rc.bottom));
				}
				floatContArray[floatCont] = 1;
			}
			if (i < nppGUI._dockingData._pluginDockInfo.size()) // to prevent from crash in debug mode
				vPluginDockInfo.push_back(nppGUI._dockingData._pluginDockInfo[i]);
		}
	}

	nppGUI._dockingData._pluginDockInfo = vPluginDockInfo;
	nppGUI._dockingData._flaotingWindowInfo = vFloatingWindowInfo;
}


void Notepad_plus::saveUserDefineLangs()	{

	param.writeNeed2SaveUDL();
}


void Notepad_plus::saveShortcuts()	{

	param.writeShortcuts();
}


void Notepad_plus::saveFindHistory()	{

	_findReplaceDlg.saveFindHistory();
	param.writeFindHistory();
}

EncodingMapper& em = EncodingMapper::getInstance();

int Notepad_plus::getHtmlXmlEncoding(const TCHAR *fileName) const
{
	// Get Language type
	TCHAR *ext = PathFindExtension(fileName);
	if (*ext == '.')	{ //extension found

		ext += 1;
	}
	else	{

		return -1;
	}
	LangType langT = param.getLangFromExt(ext);

	if ((langT != L_XML) && (langT != L_HTML))
		return -1;

	// Get the beginning of file data
	FILE *f = generic_fopen(fileName, L"rb");
	if (!f)	return -1;

	constexpr int blockSize = 1024; // To ensure that length is long enough to capture the encoding in html
	char data[blockSize];
	size_t lenFile = fread(data, 1, blockSize, f);
	fclose(f);

	// Put data in _invisibleEditView
	_invisibleEditView.f(SCI_CLEARALL);
	_invisibleEditView.f(SCI_APPENDTEXT, lenFile, reinterpret_cast<LPARAM>(data));

	const char *encodingAliasRegExpr = "[a-zA-Z0-9_-]+";
	constexpr size_t encodingStrLen = 128;
	if (langT == L_XML)	{

		// find encoding by RegExpr

		const char *xmlHeaderRegExpr =
		R"(<?xml[ \t]+version[ \t]*=[ \t]*\"[^\"]+\"[ \t]+encoding[ \t]*=[ \t]*\"[^\"]+\"[ \t]*.*?>)";

		size_t startPos = 0;
		size_t endPos = lenFile-1;
		_invisibleEditView.f(SCI_SETSEARCHFLAGS, SCFIND_REGEXP|SCFIND_POSIX);

		_invisibleEditView.f(SCI_SETTARGETRANGE, startPos, endPos);

		auto posFound = _invisibleEditView.f(SCI_SEARCHINTARGET, strlen(xmlHeaderRegExpr), reinterpret_cast<LPARAM>(xmlHeaderRegExpr));
		if (posFound != -1 && posFound != -2)	{

				const char *encodingBlockRegExpr = "encoding[ \\t]*=[ \\t]*\"[^\".]+\"";
			_invisibleEditView.f(SCI_SEARCHINTARGET, strlen(encodingBlockRegExpr), reinterpret_cast<LPARAM>(encodingBlockRegExpr));

				const char *encodingRegExpr = "\".+\"";
			_invisibleEditView.f(SCI_SEARCHINTARGET, strlen(encodingRegExpr), reinterpret_cast<LPARAM>(encodingRegExpr));

			_invisibleEditView.f(SCI_SEARCHINTARGET, strlen(encodingAliasRegExpr), reinterpret_cast<LPARAM>(encodingAliasRegExpr));

				startPos = int(_invisibleEditView.f(SCI_GETTARGETSTART));
			endPos = _invisibleEditView.f(SCI_GETTARGETEND);
			
			size_t len = endPos - startPos;
			if (len >= encodingStrLen)	{

				return -1;
			}

				char encodingStr[encodingStrLen];
				_invisibleEditView.getText(encodingStr, startPos, endPos);

				int enc = em.getEncodingFromString(encodingStr);
				return (enc == CP_ACP ? -1 : enc);
		}
		return -1;
	}
	else	{ // if (langT == L_HTML)

		const char *htmlHeaderRegExpr  = "<meta[ \\t]+http-equiv[ \\t]*=[ \\t\"']*Content-Type[ \\t\"']*content[ \\t]*= *[\"']text/html;[ \\t]+charset[ \\t]*=[ \\t]*.+[\"'] */*>";
		const char *htmlHeaderRegExpr2 = "<meta[ \\t]+content[ \\t]*= *[\"']text/html;[ \\t]+charset[ \\t]*=[ \\t]*.+[ \\t\"']http-equiv[ \\t]*=[ \\t\"']*Content-Type[ \\t\"']*/*>";
		const char *charsetBlock = "charset[ \\t]*=[ \\t]*[^\"']+";
		const char *intermediaire = "=[ \\t]*.+";
		const char *encodingStrRE = "[^ \\t=]+";

		int startPos = 0;
		auto endPos = lenFile - 1;
		_invisibleEditView.f(SCI_SETSEARCHFLAGS, SCFIND_REGEXP|SCFIND_POSIX);

		_invisibleEditView.f(SCI_SETTARGETRANGE, startPos, endPos);

		int posFound = int(_invisibleEditView.f(SCI_SEARCHINTARGET, strlen(htmlHeaderRegExpr), reinterpret_cast<LPARAM>(htmlHeaderRegExpr)));

		if (posFound == -1 || posFound == -2)	{

			posFound = int(_invisibleEditView.f(SCI_SEARCHINTARGET, strlen(htmlHeaderRegExpr2), reinterpret_cast<LPARAM>(htmlHeaderRegExpr2)));
			if (posFound == -1 || posFound == -2)
				return -1;
		}
		_invisibleEditView.f(SCI_SEARCHINTARGET, strlen(charsetBlock), reinterpret_cast<LPARAM>(charsetBlock));
		_invisibleEditView.f(SCI_SEARCHINTARGET, strlen(intermediaire), reinterpret_cast<LPARAM>(intermediaire));
		_invisibleEditView.f(SCI_SEARCHINTARGET, strlen(encodingStrRE), reinterpret_cast<LPARAM>(encodingStrRE));

		startPos = int(_invisibleEditView.f(SCI_GETTARGETSTART));
		endPos = _invisibleEditView.f(SCI_GETTARGETEND);

		size_t len = endPos - startPos;
		if (len >= encodingStrLen)	{

			return -1;
		}

		char encodingStr[encodingStrLen];
		_invisibleEditView.getText(encodingStr, startPos, endPos);

		int enc = em.getEncodingFromString(encodingStr);
		return (enc == CP_ACP ? -1 : enc);
	}
}


bool Notepad_plus::replaceInOpenedFiles()	{


	ScintillaEditView *pOldView = _pEditView;
	_pEditView = &_invisibleEditView;
	Document oldDoc = _invisibleEditView.f(SCI_GETDOCPOINTER);
	Buffer * oldBuf = _invisibleEditView.getCurrentBuffer();	//for manually setting the buffer, so notifications can be handled properly

	Buffer * pBuf = NULL;

	int nbTotal = 0;
	constexpr bool isEntireDoc = true;

	if (_mainWindowStatus & WindowMainActive)	{

		for (size_t i = 0, len = _mainDocTab.nbItem(); i < len ; ++i)	{

			pBuf = MainFileManager.getBufferByID(_mainDocTab.getBufferByIndex(i));
			if (pBuf->isReadOnly())
				continue;
			_invisibleEditView.f(SCI_SETDOCPOINTER, 0, pBuf->getDocument());
			UINT cp = static_cast<UINT>(_invisibleEditView.f(SCI_GETCODEPAGE));
			_invisibleEditView.f(SCI_SETCODEPAGE, pBuf->getUnicodeMode() == uni8Bit ? cp : SC_CP_UTF8);
			_invisibleEditView.setCurrentBuffer(pBuf);
			_invisibleEditView.f(SCI_BEGINUNDOACTION);
			nbTotal += _findReplaceDlg.processAll(ProcessReplaceAll, isEntireDoc);
			_invisibleEditView.f(SCI_ENDUNDOACTION);
		}
	}

	if (_mainWindowStatus & WindowSubActive)	{

		for (size_t i = 0, len = _subDocTab.nbItem(); i < len; ++i)	{

			pBuf = MainFileManager.getBufferByID(_subDocTab.getBufferByIndex(i));
			if (pBuf->isReadOnly())
				continue;
			_invisibleEditView.f(SCI_SETDOCPOINTER, 0, pBuf->getDocument());
			UINT cp = static_cast<UINT>(_invisibleEditView.f(SCI_GETCODEPAGE));
			_invisibleEditView.f(SCI_SETCODEPAGE, pBuf->getUnicodeMode() == uni8Bit ? cp : SC_CP_UTF8);
			_invisibleEditView.setCurrentBuffer(pBuf);
			_invisibleEditView.f(SCI_BEGINUNDOACTION);
			nbTotal += _findReplaceDlg.processAll(ProcessReplaceAll, isEntireDoc);
			_invisibleEditView.f(SCI_ENDUNDOACTION);
		}
	}

	_invisibleEditView.f(SCI_SETDOCPOINTER, 0, oldDoc);
	_invisibleEditView.setCurrentBuffer(oldBuf);
	_pEditView = pOldView;


	if (nbTotal < 0)	{

		generic_string msg = _nativeLangSpeaker.getLocalizedStrFromID("find-status-replaceinfiles-re-malformed", L"Replace in Opened Files: The regular expression is malformed.");
		_findReplaceDlg.setStatusbarMessage(msg, FSNotFound);
	}
	else	{

		if (nbTotal)
			enableCommand(IDM_FILE_SAVEALL, true, MENU | TOOLBAR);

		generic_string result;
		if (nbTotal == 1)	{

			result = _nativeLangSpeaker.getLocalizedStrFromID("find-status-replaceinopenedfiles-1-replaced", L"Replace in Opened Files: 1 occurrence was replaced.");
		}
		else	{

			result = _nativeLangSpeaker.getLocalizedStrFromID("find-status-replaceinopenedfiles-nb-replaced", L"Replace in Opened Files: $INT_REPLACE$ occurrences were replaced.");
			result = stringReplace(result, L"$INT_REPLACE$", std::to_wstring(nbTotal));
		}
		_findReplaceDlg.setStatusbarMessage(result, FSMessage);
	}
	return true;
}


void Notepad_plus::wsTabConvert(spaceTab whichWay)	{

	int tabWidth = int(_pEditView->f(SCI_GETTABWIDTH));
	int currentPos = int(_pEditView->f(SCI_GETCURRENTPOS));
	int lastLine = _pEditView->lastZeroBasedLineNumber();
	int docLength = int(_pEditView->f(SCI_GETLENGTH) + 1);
	if (docLength < 2)
		return;

	int count = 0;
	int column = 0;
	int newCurrentPos = 0;
	int tabStop = int(tabWidth - 1);   // remember, counting from zero !
	bool onlyLeading = false;
	vector<int> bookmarks;
	vector<int> folding;

	for (int i=0; i<lastLine; ++i)	{

		if (bookmarkPresent(i))
				bookmarks.push_back(i);

		if ((_pEditView->f(SCI_GETFOLDLEVEL, i) & SC_FOLDLEVELHEADERFLAG))
				if (!bool(_pEditView->f(SCI_GETFOLDEXPANDED, i)))
					folding.push_back(i);
	}

	char * source = new char[docLength];
	if (!source)
		return;
	_pEditView->f(SCI_GETTEXT, docLength, reinterpret_cast<LPARAM>(source));

	if (whichWay == tab2Space)	{

		// count how many tabs are there
		for (const char * ch=source; *ch; ++ch)	{

				if (*ch == '\t')
					++count;
		}
		if (!count)	{

				delete [] source;
				return;
		}
	}
	// allocate tabwidth-1 chars extra per tab, just to be safe
	size_t newlen = docLength + count * (tabWidth - 1) + 1;
	char * destination = new char[newlen];
	if (!destination)	{

		delete [] source;
		return;
	}
	char * dest = destination;

	switch (whichWay)	{

		case tab2Space:	{

				// rip through each line of the file
				for (int i = 0; source[i] != '\0'; ++i)	{

					if (source[i] == '\t')	{

						int insertTabs = tabWidth - (column % tabWidth);
						for (int j = 0; j < insertTabs; ++j)	{

								*dest++ = ' ';
								if (i <= currentPos)
									++newCurrentPos;
						}
						column += insertTabs;
					}
					else	{

						*dest++ = source[i];
						if (i <= currentPos)
								++newCurrentPos;
						if ((source[i] == '\n') || (source[i] == '\r'))
								column = 0;
						else if ((source[i] & 0xC0) != 0x80)  // UTF_8 support: count only bytes that don't start with 10......
								++column;
					}
				}
				*dest = '\0';
				break;
		}
		case space2TabLeading:	{

				onlyLeading = true;
		}
		case space2TabAll:	{

				bool nextChar = false;
			int counter = 0;
			bool nonSpaceFound = false;
				for (int i=0; source[i] != '\0'; ++i)	{

					if (nonSpaceFound == false)	{

						while (source[i + counter] == ' ')	{

								if ((column + counter) == tabStop)	{

									tabStop += tabWidth;
									if (counter >= 1)	{        // counter is counted from 0, so counter >= max-1

										*dest++ = '\t';
										i += counter;
										column += counter + 1;
										counter = 0;
										nextChar = true;
										if (i <= currentPos)
												++newCurrentPos;
										break;
									}
									else if (source[i+1] == ' ' || source[i+1] == '\t')	{  // if followed by space or TAB, convert even a single space to TAB

										*dest++ = '\t';
										i++;
										column += 1;
										counter = 0;
										if (i <= currentPos)
												++newCurrentPos;
									}
									else	{       // single space, don't convert it to TAB

										*dest++ = source[i];
										column += 1;
										counter = 0;
										nextChar = true;
										if (i <= currentPos)
												++newCurrentPos;
										break;
									}
								}
								else
									++counter;
						}

						if (nextChar == true)	{

								nextChar = false;
								continue;
						}

						if (source[i] == ' ' && source[i + counter] == '\t')	{ // spaces "absorbed" by a TAB on the right

								*dest++ = '\t';
								i += counter;
								column = tabStop + 1;
								tabStop += tabWidth;
								counter = 0;
								if (i <= currentPos)
									++newCurrentPos;
								continue;
						}
					}

					if (onlyLeading == true && nonSpaceFound == false)
						nonSpaceFound = true;

					if (source[i] == '\n' || source[i] == '\r')	{

						*dest++ = source[i];
						column = 0;
						tabStop = tabWidth - 1;
						nonSpaceFound = false;
					}
					else if (source[i] == '\t')	{

						*dest++ = source[i];
						column = tabStop + 1;
						tabStop += tabWidth;
						counter = 0;
					}
					else	{

						*dest++ = source[i];
						counter = 0;
						if ((source[i] & 0xC0) != 0x80)	{   // UTF_8 support: count only bytes that don't start with 10......

								++column;

								if (column > 0 && column % tabWidth == 0)
									tabStop += tabWidth;
						}
					}

					if (i <= currentPos)
						++newCurrentPos;
				}
				*dest = '\0';
				break;
		}
	}

	_pEditView->f(SCI_BEGINUNDOACTION);
	_pEditView->f(SCI_SETTEXT, 0, reinterpret_cast<LPARAM>(destination));
	_pEditView->f(SCI_GOTOPOS, newCurrentPos);

	for (size_t i=0; i<bookmarks.size(); ++i)
		_pEditView->f(SCI_MARKERADD, bookmarks[i], MARK_BOOKMARK);

	for (size_t i=0; i<folding.size(); ++i)
		_pEditView->fold(folding[i], false);

	_pEditView->f(SCI_ENDUNDOACTION);

	// clean up
	delete [] source;
	delete [] destination;
}

void Notepad_plus::doTrim(trimOp whichPart)	{

	// whichPart : line head or line tail
	FindOption env;
	if (whichPart == lineHeader)	{

		env._str2Search = L"^[	 ]+";
	}
	else if (whichPart == lineTail)	{

		env._str2Search = L"[	 ]+$";
	}
	else
		return;
	env._str4Replace = L"";
	env._searchType = FindRegex;
	_findReplaceDlg.processAll(ProcessReplaceAll, &env, true);
}

void Notepad_plus::removeEmptyLine(bool isBlankContained)	{

	// whichPart : line head or line tail
	FindOption env;
	if (isBlankContained)	{

		env._str2Search = L"^[\\t ]*$(\\r\\n|\\r|\\n)";
	}
	else	{

		env._str2Search = L"^$(\\r\\n|\\r|\\n)";
	}
	env._str4Replace = L"";
	env._searchType = FindRegex;

	_findReplaceDlg.processAll(ProcessReplaceAll, &env, true);


	// remove the last line if it's an empty line.
	if (isBlankContained)	{

		env._str2Search = L"(\\r\\n|\\r|\\n)^[\\t ]*$";
	}
	else	{

		env._str2Search = L"(\\r\\n|\\r|\\n)^$";
	}
	_findReplaceDlg.processAll(ProcessReplaceAll, &env, true);
}

void Notepad_plus::removeDuplicateLines()	{

	// whichPart : line head or line tail
	FindOption env;

	env._str2Search = L"^(.*(\\r?\\n|\\r))(\\1)+";
	env._str4Replace = L"\\1";
	env._searchType = FindRegex;
	_findReplaceDlg.processAll(ProcessReplaceAll, &env, true);

	// remove the last line if it's a duplicate line.
	env._str2Search = L"^(.+)(\\r?\\n|\\r)(\\1)$";
	env._str4Replace = L"\\1";
	env._searchType = FindRegex;
	_findReplaceDlg.processAll(ProcessReplaceAll, &env, true);
}

void Notepad_plus::getMatchedFileNames(const TCHAR *dir, const vector<generic_string> & patterns, vector<generic_string> & fileNames, bool isRecursive, bool isInHiddenDir)	{

	generic_string dirFilter(dir);
	dirFilter += L"*.*";
	WIN32_FIND_DATA foundData;

	HANDLE hFile = ::FindFirstFile(dirFilter.c_str(), &foundData);

	if (hFile != INVALID_HANDLE_VALUE)	{


		if (foundData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)	{

			if (!isInHiddenDir && (foundData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN))	{

				// do nothing
			}
			else if (isRecursive)	{

				if ((OrdinalIgnoreCaseCompareStrings(foundData.cFileName, L".") != 0) && (OrdinalIgnoreCaseCompareStrings(foundData.cFileName, L"..") != 0))	{

					generic_string pathDir(dir);
					pathDir += foundData.cFileName;
					pathDir += L"\\";
					getMatchedFileNames(pathDir.c_str(), patterns, fileNames, isRecursive, isInHiddenDir);
				}
			}
		}
		else	{

			if (matchInList(foundData.cFileName, patterns))	{

				generic_string pathFile(dir);
				pathFile += foundData.cFileName;
				fileNames.push_back(pathFile.c_str());
			}
		}
	}
	while (::FindNextFile(hFile, &foundData))	{

		if (foundData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)	{

			if (!isInHiddenDir && (foundData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN))	{

				// do nothing
			}
			else if (isRecursive)	{

				if ((OrdinalIgnoreCaseCompareStrings(foundData.cFileName, L".") != 0) && (OrdinalIgnoreCaseCompareStrings(foundData.cFileName, L"..") != 0))	{

					generic_string pathDir(dir);
					pathDir += foundData.cFileName;
					pathDir += L"\\";
					getMatchedFileNames(pathDir.c_str(), patterns, fileNames, isRecursive, isInHiddenDir);
				}
			}
		}
		else	{

			if (matchInList(foundData.cFileName, patterns))	{

				generic_string pathFile(dir);
				pathFile += foundData.cFileName;
				fileNames.push_back(pathFile.c_str());
			}
		}
	}
	::FindClose(hFile);
}

std::mutex replaceInFiles_mutex;

bool Notepad_plus::replaceInFiles()	{

	std::lock_guard<std::mutex> lock(replaceInFiles_mutex);

	const TCHAR *dir2Search = _findReplaceDlg._options._directory.c_str();
	if (!dir2Search[0] || !::PathFileExists(dir2Search))	{

		return false;
	}

	bool isRecursive = _findReplaceDlg.isRecursive();
	bool isInHiddenDir = _findReplaceDlg.isInHiddenDir();
	int nbTotal = 0;

	ScintillaEditView *pOldView = _pEditView;
	_pEditView = &_invisibleEditView;
	Document oldDoc = _invisibleEditView.f(SCI_GETDOCPOINTER);
	Buffer * oldBuf = _invisibleEditView.getCurrentBuffer();	//for manually setting the buffer, so notifications can be handled properly

	vector<generic_string> patterns2Match;
	_findReplaceDlg.getPatterns(patterns2Match);
	if (!patterns2Match.size())	{

		_findReplaceDlg.setFindInFilesDirFilter(NULL, L"*.*");
		_findReplaceDlg.getPatterns(patterns2Match);
	}
	vector<generic_string> fileNames;

	getMatchedFileNames(dir2Search, patterns2Match, fileNames, isRecursive, isInHiddenDir);

	Progress progress(_pPublicInterface->getHinst());
	size_t filesCount = fileNames.size();
	size_t filesPerPercent = 7;

	if (filesCount > 1)	{

		if (filesCount >= 200)
			filesPerPercent = filesCount / 55;
		progress.open(_findReplaceDlg.getHSelf(), L"Replace In Files progress...");
	}

	for (size_t i = 0, updateOnCount = filesPerPercent; i < filesCount; ++i)	{

		if (progress.isCancelled()) break;

		bool closeBuf = false;

		BufferID id = MainFileManager.getBufferFromName(fileNames.at(i).c_str());
		if (id == BUFFER_INVALID)	{

			id = MainFileManager.loadFile(fileNames.at(i).c_str());
			closeBuf = true;
		}

		if (id != BUFFER_INVALID)	{

			Buffer * pBuf = MainFileManager.getBufferByID(id);
			_invisibleEditView.f(SCI_SETDOCPOINTER, 0, pBuf->getDocument());
			auto cp = _invisibleEditView.f(SCI_GETCODEPAGE);
			_invisibleEditView.f(SCI_SETCODEPAGE, pBuf->getUnicodeMode() == uni8Bit ? cp : SC_CP_UTF8);
			_invisibleEditView.setCurrentBuffer(pBuf);

			FindersInfo findersInfo;
			findersInfo._pFileName = fileNames.at(i).c_str();
			int nbReplaced = _findReplaceDlg.processAll(ProcessReplaceAll, true, &findersInfo);
			nbTotal += nbReplaced;
			if (nbReplaced)	{

				MainFileManager.saveBuffer(id, pBuf->getFullPathName());
			}

			if (closeBuf)
				MainFileManager.closeBuffer(id, _pEditView);
		}
		if (i == updateOnCount)	{

			updateOnCount += filesPerPercent;
			progress.setPercent(int32_t((i * 100) / filesCount), fileNames.at(i).c_str());
		}
		else
			progress.setInfo(fileNames.at(i).c_str());
	}

	progress.close();

	_invisibleEditView.f(SCI_SETDOCPOINTER, 0, oldDoc);
	_invisibleEditView.setCurrentBuffer(oldBuf);
	_pEditView = pOldView;

	generic_string result;
	// if (nbTotal == 1)
		result = _nativeLangSpeaker.getLocalizedStrFromID("find-status-replaceinfiles-nb-replaced", L"Replace in Files: $INT_REPLACE$ occurrences were replaced.");
		result = stringReplace(result, L"$INT_REPLACE$", std::to_wstring(nbTotal));

	_findReplaceDlg.setStatusbarMessage(result, FSMessage);

	return true;
}

bool Notepad_plus::findInFinderFiles(FindersInfo *findInFolderInfo)	{

	int nbTotal = 0;
	ScintillaEditView *pOldView = _pEditView;
	_pEditView = &_invisibleEditView;
	Document oldDoc = _invisibleEditView.f(SCI_GETDOCPOINTER);

	vector<generic_string> patterns2Match;
	_findReplaceDlg.getPatterns(patterns2Match);
	if (!patterns2Match.size())	{

		_findReplaceDlg.setFindInFilesDirFilter(NULL, L"*.*");
		_findReplaceDlg.getPatterns(patterns2Match);
	}

	vector<generic_string> fileNames = findInFolderInfo->_pSourceFinder->getResultFilePaths();

	findInFolderInfo->_pDestFinder->beginNewFilesSearch();
	findInFolderInfo->_pDestFinder->addSearchLine(findInFolderInfo->_findOption._str2Search.c_str());

	Progress progress(_pPublicInterface->getHinst());

	size_t filesCount = fileNames.size();
	size_t filesPerPercent = 7;

	if (filesCount > 1)	{

		if (filesCount >= 210)
			filesPerPercent = filesCount / 57;
		progress.open(_findReplaceDlg.getHSelf(), L"Find In Files progress...");
	}

	for (size_t i = 0, updateOnCount = filesPerPercent; i < filesCount; ++i)	{

		if (progress.isCancelled()) break;

		bool closeBuf = false;
		BufferID id = MainFileManager.getBufferFromName(fileNames.at(i).c_str());
		if (id == BUFFER_INVALID)	{

			id = MainFileManager.loadFile(fileNames.at(i).c_str());
			closeBuf = true;
		}

		if (id != BUFFER_INVALID)	{

			Buffer * pBuf = MainFileManager.getBufferByID(id);
			_invisibleEditView.f(SCI_SETDOCPOINTER, 0, pBuf->getDocument());
			auto cp = _invisibleEditView.f(SCI_GETCODEPAGE);
			_invisibleEditView.f(SCI_SETCODEPAGE, pBuf->getUnicodeMode() == uni8Bit ? cp : SC_CP_UTF8);

			findInFolderInfo->_pFileName = fileNames.at(i).c_str();
			nbTotal += _findReplaceDlg.processAll(ProcessFindInFinder, &(findInFolderInfo->_findOption), true, findInFolderInfo);
			if (closeBuf)
				MainFileManager.closeBuffer(id, _pEditView);
		}
		if (i == updateOnCount)	{

			updateOnCount += filesPerPercent;
			progress.setPercent(int32_t((i * 100) / filesCount), fileNames.at(i).c_str());
		}
		else	{

			progress.setInfo(fileNames.at(i).c_str());
		}
	}
	progress.close();

	findInFolderInfo->_pDestFinder->finishFilesSearch(nbTotal, 1, nullptr, findInFolderInfo->_findOption._isMatchLineNumber);

	_invisibleEditView.f(SCI_SETDOCPOINTER, 0, oldDoc);
	_pEditView = pOldView;

	return true;
}

bool Notepad_plus::findInFiles()	{
	const TCHAR *dir2Search;
	generic_string dir, front = _findReplaceDlg._options._directory.c_str();
	if (not front[0])	return false;
	size_t st, dirOff;
	bool noFold=0, fold=1, hasMore=1, get1=0;
	bool isRecursive = _findReplaceDlg.isRecursive();
	bool isInHiddenDir = _findReplaceDlg.isInHiddenDir();
	ScintillaEditView *pOldView = _pEditView;
	_pEditView = &_invisibleEditView;
	Document oldDoc = _invisibleEditView.f(SCI_GETDOCPOINTER);

	while (hasMore)	{
		dirOff = front.find_last_of(L';');
		if (dirOff==string::npos){
			hasMore=0;
			dir=front;
		}
		else{
			dir=front.substr(dirOff+1);
			front= front.substr(0,dirOff);
		}
		st = dir.find_first_not_of(L' ');
		if (st==string::npos)	{

			if (hasMore)	continue;
			return false;
		}
		if(st)	dir=dir.substr(st);
		if (dir[dir.find_last_not_of(L' ')] != L'\\')
			dir += L'\\';
		if (!::PathFileExists(dir2Search=dir.c_str()))	{

			if (hasMore)	continue;
			return false;
		}
		int nbTotal = 0;
		vector<generic_string> patterns2Match;
		_findReplaceDlg.getPatterns(patterns2Match);
		if (!patterns2Match.size())	{

			_findReplaceDlg.setFindInFilesDirFilter(NULL, L"*.*");
			_findReplaceDlg.getPatterns(patterns2Match);
		}
		vector<generic_string> fileNames;
		getMatchedFileNames(dir2Search, patterns2Match, fileNames, isRecursive, isInHiddenDir);

		_findReplaceDlg.beginNewFilesSearch();
		Progress progress(_pPublicInterface->getHinst());

		size_t filesPerPercent = 9;
		size_t filesCount = fileNames.size();
		_findReplaceDlg._fileTot = int(filesCount);

		if (filesCount > 1)	{
			if (filesCount >= 210)		filesPerPercent = filesCount / 57;
			progress.open(_findReplaceDlg.getHSelf(), L"Find In Files progress...");
		}

		for (size_t i = 0, updateOnCount = filesPerPercent; i < filesCount; ++i)	{
			if (progress.isCancelled()) break;
			bool closeBuf = false;
			BufferID id = MainFileManager.getBufferFromName(fileNames.at(i).c_str());
		
			if (id == BUFFER_INVALID)	{
				id = MainFileManager.loadFile(fileNames.at(i).c_str());
				closeBuf = true;
			}
			if (id != BUFFER_INVALID)	{
				Buffer * pBuf = MainFileManager.getBufferByID(id);
				_invisibleEditView.f(SCI_SETDOCPOINTER, 0, pBuf->getDocument());
				auto cp = _invisibleEditView.f(SCI_GETCODEPAGE);
				_invisibleEditView.f(SCI_SETCODEPAGE, pBuf->getUnicodeMode() == uni8Bit ? cp : SC_CP_UTF8);
				FindersInfo findersInfo;
				findersInfo._pFileName = fileNames.at(i).c_str();
				findersInfo._unDir = dir2Search;
				nbTotal += _findReplaceDlg.processAll(ProcessFindAll, true, &findersInfo);
				if (closeBuf)		MainFileManager.closeBuffer(id, _pEditView);
			}
			if (i == updateOnCount)	{
				updateOnCount += filesPerPercent;
				progress.setPercent(int32_t((i * 100) / filesCount), fileNames.at(i).c_str());
			}
			else
				progress.setInfo(fileNames.at(i).c_str());
		}
		progress.close();

		_findReplaceDlg.finishFilesSearch(nbTotal, (hasMore? noFold: fold), dir2Search);
		get1 |= bool(nbTotal);
	}
	_invisibleEditView.f(SCI_SETDOCPOINTER, 0, oldDoc);
	_pEditView = pOldView;
	if (get1 && !param.getFindHistory()._isDlgAlwaysVisible)
		_findReplaceDlg.display(false);
	return true;
}


bool Notepad_plus::findInOpenedFiles()	{
	int nbTotal = 0;
	ScintillaEditView *pOldView = _pEditView;
	_pEditView = &_invisibleEditView;
	Document oldDoc = _invisibleEditView.f(SCI_GETDOCPOINTER);

	Buffer * pBuf = NULL;

	constexpr bool isEntireDoc = true;

	_findReplaceDlg.beginNewFilesSearch();

	size_t i=0, len = _mainDocTab.nbItem();
	if (_mainWindowStatus & WindowMainActive)	{

		// _findReplaceDlg.nf(len);
		for (; i < len ; ++i)	{

			pBuf = MainFileManager.getBufferByID(_mainDocTab.getBufferByIndex(i));
			_invisibleEditView.f(SCI_SETDOCPOINTER, 0, pBuf->getDocument());
			auto cp = _invisibleEditView.f(SCI_GETCODEPAGE);
			_invisibleEditView.f(SCI_SETCODEPAGE, pBuf->getUnicodeMode() == uni8Bit ? cp : SC_CP_UTF8);
			FindersInfo findersInfo;
			findersInfo._pFileName = pBuf->getFullPathName();
			findersInfo._unDir = nullptr;
			nbTotal += _findReplaceDlg.processAll(ProcessFindAll, isEntireDoc, &findersInfo);
		}
	}

	size_t j=0;
	if (_mainWindowStatus & WindowSubActive)	{

		for (size_t i = 0; i < _subDocTab.nbItem() ; ++i)	{

			pBuf = MainFileManager.getBufferByID(_subDocTab.getBufferByIndex(i));
			if (_mainDocTab.getIndexByBuffer(pBuf) != -1)// clone is skipped searching in sub
				continue; 
			++j;
			_invisibleEditView.f(SCI_SETDOCPOINTER, 0, pBuf->getDocument());
			auto cp = _invisibleEditView.f(SCI_GETCODEPAGE);
			_invisibleEditView.f(SCI_SETCODEPAGE, pBuf->getUnicodeMode() == uni8Bit ? cp : SC_CP_UTF8);
			FindersInfo findersInfo;
			findersInfo._pFileName = pBuf->getFullPathName();
			nbTotal += _findReplaceDlg.processAll(ProcessFindAll, isEntireDoc, &findersInfo);
		}
	}

	_findReplaceDlg.nf(len + j);
	_findReplaceDlg.finishFilesSearch(nbTotal);

	_invisibleEditView.f(SCI_SETDOCPOINTER, 0, oldDoc);
	_pEditView = pOldView;

	if (nbTotal && !param.getFindHistory()._isDlgAlwaysVisible)
		_findReplaceDlg.display(false);
	return true;
}


bool Notepad_plus::findInCurrentFile()	{

	int nbTotal;
	Buffer * pBuf = _pEditView->getCurrentBuffer();
	ScintillaEditView *pOldView = _pEditView;
	_pEditView = &_invisibleEditView;
	Document oldDoc = _invisibleEditView.f(SCI_GETDOCPOINTER);

	constexpr bool isEntireDoc = true;

	_findReplaceDlg.beginNewFilesSearch();

	_invisibleEditView.f(SCI_SETDOCPOINTER, 0, pBuf->getDocument());
	UINT cp = static_cast<UINT>(_invisibleEditView.f(SCI_GETCODEPAGE));
	_invisibleEditView.f(SCI_SETCODEPAGE, pBuf->getUnicodeMode() == uni8Bit ? cp : SC_CP_UTF8);
	FindersInfo findersInfo;
	findersInfo._pFileName = pBuf->getFullPathName();
	nbTotal = _findReplaceDlg.processAll(ProcessFindAll, isEntireDoc, &findersInfo);

	_findReplaceDlg.finishFilesSearch(nbTotal);

	_invisibleEditView.f(SCI_SETDOCPOINTER, 0, oldDoc);
	_pEditView = pOldView;

	if (nbTotal && !param.getFindHistory()._isDlgAlwaysVisible)
		_findReplaceDlg.display(false);
	return true;
}

void Notepad_plus::filePrint(bool showDialog)	{

	Printer printer;

	int startPos = int(_pEditView->f(SCI_GETSELECTIONSTART));
	int endPos = int(_pEditView->f(SCI_GETSELECTIONEND));

	printer.init(_pPublicInterface->getHinst(), _pPublicInterface->getHSelf(), _pEditView, showDialog, startPos, endPos, _nativeLangSpeaker.isRTL());
	printer.doPrint();
}

int Notepad_plus::doSaveOrNot(const TCHAR* fn, bool isMulti)	{

	// In case Notepad++ is iconized into notification zone
	if (!::IsWindowVisible(_pPublicInterface->getHSelf()))	{

		::ShowWindow(_pPublicInterface->getHSelf(), SW_SHOW);

		// Send sizing info to make window fit (specially to show tool bar.)
		::SendMessage(_pPublicInterface->getHSelf(), WM_SIZE, 0, 0);
	}

	if (!isMulti)	{

		generic_string title, msg;

		if (!_nativeLangSpeaker.getDoSaveOrNotStrings(title, msg))	{

			title = L"Save";
			msg = L"Save file \"$STR_REPLACE$\" ?";
		}

		msg = stringReplace(msg, L"$STR_REPLACE$", fn);

		return ::MessageBox(_pPublicInterface->getHSelf(), msg.c_str(), title.c_str(), MB_YESNOCANCEL | MB_ICONQUESTION | MB_APPLMODAL);
	}

	DoSaveOrNotBox doSaveOrNotBox;
	doSaveOrNotBox.init(_pPublicInterface->getHinst(), _pPublicInterface->getHSelf(), fn, isMulti);
	doSaveOrNotBox.doDialog(_nativeLangSpeaker.isRTL());
	int buttonID = doSaveOrNotBox.getClickedButtonId();
	doSaveOrNotBox.destroy();

	return buttonID;
}

int Notepad_plus::doReloadOrNot(const TCHAR *fn, bool dirty)	{

	if (dirty)
		return _nativeLangSpeaker.messageBox("DoReloadOrNotAndLooseChange",
			_pPublicInterface->getHSelf(),
			L"\"$STR_REPLACE$\"\r\rThis file has been modified by another program.\rDo you want to reload it and lose the changes made in Notepad++?",
			L"Reload",
			MB_YESNO | MB_APPLMODAL | MB_ICONEXCLAMATION,
			0, // not used
			fn);
	else
		return _nativeLangSpeaker.messageBox("DoReloadOrNot",
			_pPublicInterface->getHSelf(),
			L"\"$STR_REPLACE$\"\r\rThis file has been modified by another program.\rDo you want to reload it?",
			L"Reload",
			MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION,
			0, // not used
			fn);
}

int Notepad_plus::doCloseOrNot(const TCHAR *fn)	{

	return _nativeLangSpeaker.messageBox("DoCloseOrNot",
		_pPublicInterface->getHSelf(),
		L"The file \"$STR_REPLACE$\" doesn't exist anymore.\rKeep this file in editor?",
		L"Keep non existing file",
		MB_YESNO | MB_ICONQUESTION | MB_APPLMODAL,
		0, // not used
		fn);
}

int Notepad_plus::doDeleteOrNot(const TCHAR *fn)	{

	return _nativeLangSpeaker.messageBox("DoDeleteOrNot",
		_pPublicInterface->getHSelf(),
		L"The file \"$STR_REPLACE$\"\rwill be moved to your Recycle Bin and this document will be closed.\rContinue?",
		L"Delete file",
		MB_YESNO | MB_ICONQUESTION | MB_APPLMODAL,
		0, // not used
		fn);
}

/* void Notepad_plus::enableMenu(int cmdID, bool doEnable) const
{
	int flag = doEnable?MF_ENABLED | MF_BYCOMMAND:MF_DISABLED | MF_GRAYED | MF_BYCOMMAND;
	::EnableMenuItem(_mainMenuHandle, cmdID, flag);
} */

void Notepad_plus::enableCommand(int cmdID, bool doEnable, int which) const
{
	if (which & MENU)	{

	::EnableMenuItem(_mainMenuHandle, cmdID,doEnable?MF_ENABLED | MF_BYCOMMAND:MF_DISABLED | MF_GRAYED | MF_BYCOMMAND);
		// enableMenu(cmdID, doEnable);
	}
	if (which & TOOLBAR)	{

		_toolBar.enable(cmdID, doEnable);
	}
}

void Notepad_plus::checkDocState()	{

	Buffer * curBuf = _pEditView->getCurrentBuffer();

	bool isCurrentDirty = curBuf->isDirty(),
	isSeveralDirty = isCurrentDirty,
	isFileExisting = PathFileExists(curBuf->getFullPathName());

	if (!isCurrentDirty)	{

		for (size_t i = 0; i < MainFileManager.getNbBuffers(); ++i)
			if (MainFileManager.getBufferByIndex(i)->isDirty())	{

				isSeveralDirty = true;
				break;
			}
	}

	bool isCurrentUntitled = curBuf->isUntitled();
	enableCommand(IDM_FILE_SAVE, isCurrentDirty, MENU | TOOLBAR);
	enableCommand(IDM_FILE_SAVEALL, isSeveralDirty, MENU | TOOLBAR);
	enableCommand(IDM_VIEW_GOTO_NEW_INSTANCE, !(isCurrentDirty || isCurrentUntitled), MENU);
	enableCommand(IDM_VIEW_LOAD_IN_NEW_INSTANCE, !(isCurrentDirty || isCurrentUntitled), MENU);

	bool isSysReadOnly = curBuf->getFileReadOnly();
	enableCommand(IDM_EDIT_CLEARREADONLY, isSysReadOnly, MENU);

	bool doEnable = not (curBuf->isMonitoringOn() || isSysReadOnly);
	enableCommand(IDM_EDIT_SETREADONLY, doEnable, MENU);

	bool isUserReadOnly = curBuf->getUserReadOnly();
	::CheckMenuItem(_mainMenuHandle, IDM_EDIT_SETREADONLY, MF_BYCOMMAND | (isUserReadOnly ? MF_CHECKED : MF_UNCHECKED));

	enableCommand(IDM_FILE_DELETE, isFileExisting, MENU);
	//enableCommand(IDM_FILE_RENAME, isFileExisting, MENU);
	enableCommand(IDM_FILE_OPEN_CMD, isFileExisting, MENU);
	enableCommand(IDM_FILE_OPEN_FOLDER, isFileExisting, MENU);
	enableCommand(IDM_FILE_RELOAD, isFileExisting, MENU);

	enableCommand(IDM_FILE_OPEN_DEFAULT_VIEWER, isAssoCommandExisting(curBuf->getFullPathName()), MENU);

	enableCommand(IDM_VIEW_IN_FIREFOX, isFileExisting, MENU);
	enableCommand(IDM_VIEW_IN_CHROME, isFileExisting, MENU);
	enableCommand(IDM_VIEW_IN_IE, isFileExisting, MENU);
	enableCommand(IDM_VIEW_IN_EDGE, isFileExisting, MENU);

	enableConvertMenuItems(curBuf->getEolFormat());
	checkUnicodeMenuItems();
	checkLangsMenu(-1);

	if (_pAnsiCharPanel)
		_pAnsiCharPanel->switchEncoding();

	enableCommand(IDM_VIEW_MONITORING, not curBuf->isUntitled(), MENU | TOOLBAR);
	checkMenuItem(IDM_VIEW_MONITORING, curBuf->isMonitoringOn());
	_toolBar.setCheck(IDM_VIEW_MONITORING, curBuf->isMonitoringOn());
}


void Notepad_plus::checkMacroState()	{

	enableCommand(IDM_MACRO_STARTRECORDINGMACRO, !_recordingMacro, MENU | TOOLBAR);
	enableCommand(IDM_MACRO_STOPRECORDINGMACRO, _recordingMacro, MENU | TOOLBAR);
	enableCommand(IDM_MACRO_PLAYBACKRECORDEDMACRO, !_macro.empty() && !_recordingMacro, MENU | TOOLBAR);
	enableCommand(IDM_MACRO_SAVECURRENTMACRO, !_macro.empty() && !_recordingMacro && !_recordingSaved, MENU | TOOLBAR);

	enableCommand(IDM_MACRO_RUNMULTIMACRODLG, (!_macro.empty() && !_recordingMacro) || !((param).getMacroList()).empty(), MENU | TOOLBAR);
}

void Notepad_plus::checkSyncState()	{

	bool canDoSync = viewVisible(MAIN_VIEW) && viewVisible(SUB_VIEW);
	if (!canDoSync)	{

		_syncInfo._isSynScollV = false;
		_syncInfo._isSynScollH = false;
		checkMenuItem(IDM_VIEW_SYNSCROLLV, false);
		checkMenuItem(IDM_VIEW_SYNSCROLLH, false);
		_toolBar.setCheck(IDM_VIEW_SYNSCROLLV, false);
		_toolBar.setCheck(IDM_VIEW_SYNSCROLLH, false);
	}
	enableCommand(IDM_VIEW_SYNSCROLLV, canDoSync, MENU | TOOLBAR);
	enableCommand(IDM_VIEW_SYNSCROLLH, canDoSync, MENU | TOOLBAR);
}

void doCheck(HMENU mainHandle, int id)	{

	MENUITEMINFO mii;
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_SUBMENU | MIIM_FTYPE | MIIM_ID | MIIM_STATE;

	int count = ::GetMenuItemCount(mainHandle);
	for (int i = 0; i < count; ++i )	{

		::GetMenuItemInfo(mainHandle, i, MF_BYPOSITION, &mii);
		if (mii.fType == MFT_RADIOCHECK || mii.fType == MFT_STRING)	{

			if (!mii.hSubMenu)	{

				if (mii.wID == (unsigned int)id)	{

					::CheckMenuRadioItem(mainHandle, 0, count, i, MF_BYPOSITION);
				}
				else	{

					mii.fState = 0;
					::SetMenuItemInfo(mainHandle, i, MF_BYPOSITION, &mii);
				}
			}
			else	{

				doCheck(mii.hSubMenu, id);
			}
		}
	}
}

void Notepad_plus::checkLangsMenu(int id) const
{
	Buffer * curBuf = _pEditView->getCurrentBuffer();
	if (id == -1)	{

		id = param.langTypeToCommandID(curBuf->getLangType());
		if (id == IDM_LANG_USER)	{

			if (curBuf->isUserDefineLangExt())	{

				const TCHAR *userLangName = curBuf->getUserDefineLangName();
				TCHAR menuLangName[langNameLenMax];

				for (int i = IDM_LANG_USER + 1 ; i <= IDM_LANG_USER_LIMIT ; ++i)	{

					if (::GetMenuString(_mainMenuHandle, i, menuLangName, langNameLenMax, MF_BYCOMMAND))	{

						if (!lstrcmp(userLangName, menuLangName))	{

							HMENU _langMenuHandle = ::GetSubMenu(_mainMenuHandle, MENUINDEX_LANGUAGE);
							doCheck(_langMenuHandle, i);
							return;
						}
					}
				}
			}
		}
	}
	HMENU _langMenuHandle = ::GetSubMenu(_mainMenuHandle, MENUINDEX_LANGUAGE);
	doCheck(_langMenuHandle, id);
}

generic_string Notepad_plus::getLangDesc(LangType langType, bool getName)	{


	if (langType >= L_EXTERNAL && (langType < param.L_END))	{

		ExternalLangContainer & elc = param.getELCFromIndex(langType - L_EXTERNAL);
		// if ()
			return generic_string(getName? elc._name: elc._desc);
		// else
			// return generic_string();
	}

	if (langType > L_EXTERNAL)
		langType = L_TEXT;

	generic_string str2Show;
	if (getName)
		str2Show = ScintillaEditView::langNames[langType].shortName;
	else
		str2Show = ScintillaEditView::langNames[langType].longName;

	if (langType == L_USER)	{

		Buffer * currentBuf = _pEditView->getCurrentBuffer();
		if (currentBuf->isUserDefineLangExt())	{

			str2Show += L" - ";
			str2Show += currentBuf->getUserDefineLangName();
		}
	}
	return str2Show;
}

void Notepad_plus::copyMarkedLines()	{

	int lastLine = _pEditView->lastZeroBasedLineNumber();
	generic_string globalStr = L"";
	for (int i = lastLine ; i >= 0 ; --i )	{

		if (bookmarkPresent(i))	{

			generic_string currentStr = getMarkedLine(i) + globalStr;
			globalStr = currentStr;
		}
	}
	str2Cliboard(globalStr);
}

std::mutex mark_mutex;

void Notepad_plus::cutMarkedLines()	{

	std::lock_guard<std::mutex> lock(mark_mutex);

	int lastLine = _pEditView->lastZeroBasedLineNumber();
	generic_string globalStr = L"";

	_pEditView->f(SCI_BEGINUNDOACTION);
	for (int i = lastLine ; i >= 0 ; --i )	{

		if (bookmarkPresent(i))	{

			generic_string currentStr = getMarkedLine(i) + globalStr;
			globalStr = currentStr;

			deleteMarkedline(i);
		}
	}
	_pEditView->f(SCI_ENDUNDOACTION);
	str2Cliboard(globalStr);
}

void Notepad_plus::deleteMarkedLines(bool isMarked)	{

	std::lock_guard<std::mutex> lock(mark_mutex);

	int lastLine = _pEditView->lastZeroBasedLineNumber();

	_pEditView->f(SCI_BEGINUNDOACTION);
	for (int i = lastLine ; i >= 0 ; --i )	{

		if (bookmarkPresent(i) == isMarked)
			deleteMarkedline(i);
	}
	_pEditView->f(SCI_ENDUNDOACTION);
}

void Notepad_plus::pasteToMarkedLines()	{

	std::lock_guard<std::mutex> lock(mark_mutex);

	int clipFormat;
	clipFormat = CF_UNICODETEXT;

	BOOL canPaste = ::IsClipboardFormatAvailable(clipFormat);
	if (!canPaste)
		return;
	int lastLine = _pEditView->lastZeroBasedLineNumber();

	::OpenClipboard(_pPublicInterface->getHSelf());
	HANDLE clipboardData = ::GetClipboardData(clipFormat);
	::GlobalSize(clipboardData);
	LPVOID clipboardDataPtr = ::GlobalLock(clipboardData);

	generic_string clipboardStr = (const TCHAR *)clipboardDataPtr;

	::GlobalUnlock(clipboardData);
	::CloseClipboard();

	_pEditView->f(SCI_BEGINUNDOACTION);
	for (int i = lastLine ; i >= 0 ; --i )	{

		if (bookmarkPresent(i))	{

			replaceMarkedline(i, clipboardStr.c_str());
		}
	}
	_pEditView->f(SCI_ENDUNDOACTION);
}

void Notepad_plus::deleteMarkedline(int ln)	{

	int lineLen = int(_pEditView->f(SCI_LINELENGTH, ln));
	int lineBegin = int(_pEditView->f(SCI_POSITIONFROMLINE, ln));

	bookmarkDelete(ln);
	TCHAR emptyString[2] = L"";
	_pEditView->replaceTarget(emptyString, lineBegin, lineBegin + lineLen);
}

void Notepad_plus::inverseMarks()	{

	int lastLine = _pEditView->lastZeroBasedLineNumber();
	for (int i = 0 ; i <= lastLine  ; ++i)	{

		if (bookmarkPresent(i))	{

			bookmarkDelete(i);
		}
		else	{

			bookmarkAdd(i);
		}
	}
}

void Notepad_plus::replaceMarkedline(int ln, const TCHAR *str)	{

	int lineBegin = int(_pEditView->f(SCI_POSITIONFROMLINE, ln));
	int lineEnd = int(_pEditView->f(SCI_GETLINEENDPOSITION, ln));

	_pEditView->replaceTarget(str, lineBegin, lineEnd);
}

generic_string Notepad_plus::getMarkedLine(int ln)	{

	auto lineLen = _pEditView->f(SCI_LINELENGTH, ln);
	auto lineBegin = _pEditView->f(SCI_POSITIONFROMLINE, ln);

	TCHAR * buf = new TCHAR[lineLen+1];
	_pEditView->getGenericText(buf, lineLen + 1, lineBegin, lineBegin + lineLen);
	generic_string line = buf;
	delete [] buf;

	return line;
}

void Notepad_plus::findMatchingBracePos(int & bracePosCaret, int & braceOpposite)	{

	if (_pEditView->f(SCI_GETLENGTH) <=0)	return;

	int caretPos = bracePosCaret = int(_pEditView->f(SCI_GETCURRENTPOS));
	TCHAR charBefore, charAtCaret = TCHAR(_pEditView->f(SCI_GETCHARAT, caretPos));

	if (generic_strchr(L"[](){}", charAtCaret))	{
		braceOpposite = int(_pEditView->f(SCI_BRACEMATCH, caretPos, 0)); //<0 ?	0: 1;
		return;
	}
	else if (bracePosCaret > 0)	{
		charBefore = caretPos >0 ? TCHAR(_pEditView->f(SCI_GETCHARAT, --bracePosCaret)) : 0;
		
		if (charBefore && generic_strchr(L"[](){}", charBefore))	{
			braceOpposite = int(_pEditView->f(SCI_BRACEMATCH, bracePosCaret, 0)); //<0 ?	0: 1;
			return;
		}
	}
	bracePosCaret = braceOpposite = -1;//, 0;
	return;
}

/* bool Notepad_plus::selectBracePairContainCaret()	{

	int caretPos = int(_pEditView->f(SCI_GETCURRENTPOS));
	
	_pEditView->f(SCI_SETTARGETRANGE, caretPos, caretPos -128);
	_pEditView->f(SCI_SETSEARCHFLAGS, SCFIND_REGEXP
	|SCFIND_POSIX
	|SCFIND_REGEXP_DOTMATCHESNL
	|SCFIND_REGEXP_EMPTYMATCH_NONE
	| SCFIND_REGEXP_SKIPCRLFASONE); //|SCFIND_REGEXP_EMPTYMATCH_ALL

	// R"(\([^()]*+(?![()]))";
	const char re[] = R"(\((?>[^()]|(?R))*\))";
	int startMatch = int(_pEditView->f(SCI_SEARCHINTARGET, strlen(re), reinterpret_cast<LPARAM>(re))),
	endMatch = int(_pEditView->f(SCI_GETTARGETEND)),
	matchLen, U_len;
	char *result = new char[matchLen=endMatch-startMatch+1] ;

	if (startMatch >= 0)	{
		_pEditView->f(SCI_GETTARGETTEXT, 0, reinterpret_cast<LPARAM>(result));
		UINT CP=static_cast<UINT>(_pEditView->f(SCI_GETCODEPAGE));
		U_len = MultiByteToWideChar(CP, 0, result, -1, NULL, 0);
		TCHAR *U_txt = new TCHAR[U_len+1],
		*reinfo = new TCHAR[U_len+21+1];
		MultiByteToWideChar(CP, 0, result, -1, U_txt, U_len);

		wsprintf(reinfo, L"%s", U_txt);
		_statusBar.setText( STATUSBAR_INFOS,reinfo);
		_pEditView->f(SCI_SETSEL, startMatch, endMatch);
		delete[] result;
		delete[] U_txt;
		delete[] reinfo;
		return 1;
	}
	else	{
		_statusBar.setText( STATUSBAR_INFOS,L"Left bound not found ");
		return 0;
	}
}
 */

// return true if 1 or 2 (matched) brace(s) is found
bool Notepad_plus::braceMatch()	{

	int braceAtCaret, braceOpposite;
	findMatchingBracePos(braceAtCaret, braceOpposite);

	if (braceAtCaret != -1 && braceOpposite == -1)	{

		_pEditView->f(SCI_BRACEBADLIGHT, braceAtCaret);
		_pEditView->f(SCI_SETHIGHLIGHTGUIDE, 0);
	}
	else	{
		_pEditView->f(SCI_BRACEHIGHLIGHT, braceAtCaret, braceOpposite);

		if (_pEditView->isShownIndentGuide())	{

			int columnAtCaret = int(_pEditView->f(SCI_GETCOLUMN, braceAtCaret));
			int columnOpposite = int(_pEditView->f(SCI_GETCOLUMN, braceOpposite));
			_pEditView->f(SCI_SETHIGHLIGHTGUIDE, (columnAtCaret < columnOpposite)?columnAtCaret:columnOpposite);
		}
	}

/* 	const bool enable = braceAtCaret != -1 && braceOpposite != -1;
	enableCommand(IDM_SEARCH_GOTOMATCHINGBRACE, enable, MENU | TOOLBAR);
	enableCommand(IDM_SEARCH_SELECTMATCHINGBRACES, enable, MENU);
	enableCommand(IDM_SEARCH_SELECTMATCHBRACEX, enable, MENU);*/

	return braceAtCaret != -1;
}


void Notepad_plus::setLangStatus(LangType langType)	{

	generic_string s= getLangDesc(langType,true);
	_statusBar.setText( STATUSBAR_DOC_TYPE,s.c_str());

}


void Notepad_plus::setDisplayFormat(EolType format)	{

	const TCHAR* str;

	str = format == EolType::windows ? L"\\r\\n WIN"
	: format == EolType::macos ? L"\\r MAC"
	: format == EolType::unix ? L"\\n NIX"
	: L"Unknown";
	assert(format != EolType::unknown);

	_statusBar.setText( STATUSBAR_EOF_FORMAT,str);
}


void Notepad_plus::setUniModeText()	{

	Buffer *buf = _pEditView->getCurrentBuffer();
	int encoding = buf->getEncoding();
	UniMode um = buf->getUnicodeMode();

	generic_string uniModeTextString;

	if (encoding == -1)	{

		switch (um)	{

			case uniUTF8:
				uniModeTextString = L"UTF-8-BOM"; break;
			case uni16BE:
				uniModeTextString = L"UCS-2 BE BOM"; break;
			case uni16LE:
				uniModeTextString = L"UCS-2 LE BOM"; break;
			case uni16BE_NoBOM:
				uniModeTextString = L"UCS-2 BE"; break;
			case uni16LE_NoBOM:
				uniModeTextString = L"UCS-2 LE"; break;
			case uniCookie:
				uniModeTextString = L"UTF-8"; break;
			default :
				uniModeTextString = L"ANSI";
		}
	}
	else	{

		int cmdID = em.getIndexFromEncoding(encoding);
		if (cmdID == -1)	{

			//printStr(L"Encoding problem. Encoding is not added in encoding_table?");
			return;
		}
		cmdID += IDM_FORMAT_ENCODE;

		constexpr int itemSize = 64;
		TCHAR uniModeText[itemSize];
		::GetMenuString(_mainMenuHandle, cmdID, uniModeText, itemSize, MF_BYCOMMAND);
		uniModeTextString = uniModeText;
	}

	_statusBar.setText(STATUSBAR_ENCODING,uniModeTextString.c_str());
}


void Notepad_plus::addHotSpot()	{

	int startPos = 0;
	int endPos = -1;
	auto endStyle = _pEditView->f(SCI_GETENDSTYLED);

	_pEditView->getVisibleStartAndEndPosition(&startPos, &endPos);

	_pEditView->f(SCI_SETSEARCHFLAGS, SCFIND_REGEXP|SCFIND_POSIX);

	_pEditView->f(SCI_SETTARGETRANGE, startPos, endPos);

	std::vector<unsigned char> hotspotPairs; //= _pEditView->GetHotspotPairs();

	unsigned char style_hotspot = 0;
	unsigned char mask = 0x40; // INDIC1_MASK;
	// INDIC2_MASK == 255 and it represents MSB bit		
	// only LEX_HTML and LEX_POSTSCRIPT use use INDIC2_MASK bit internally		
	// LEX_HTML is using INDIC2_MASK bit even though it has only 127 states, so it is safe to overwrite 8th bit		
	// INDIC2_MASK will be used for LEX_HTML		

	// LEX_POSTSCRIPT is using INDIC2_MASK bit for "tokenization", and is using mask=31 in lexer,		
	// therefore hotspot in LEX_POSTSCRIPT will be saved to 5th bit		
	// there are only 15 states in LEX_POSTSCRIPT, so it is safe to overwrite 5th bit		

	// rule of the thumb is, any lexet that calls: styler.StartAt(startPos, 255);		
	// must have special processing here, all other lexers are fine with INDIC1_MASK (7th bit)		

	LangType type = _pEditView->getCurrentBuffer()->getLangType();

	if (type == L_HTML || type == L_PHP || type == L_ASP || type == L_JSP)
		mask = 0x80; // INDIC2_MASK;
	else if (type == L_PS)
		mask = 16;

	int posFound = int(_pEditView->f(SCI_SEARCHINTARGET, strlen(URL_REG_EXPR), reinterpret_cast<LPARAM>(URL_REG_EXPR)));

	while (posFound != -1 && posFound != -2)	{

		int start = int(_pEditView->f(SCI_GETTARGETSTART));
		int end = int(_pEditView->f(SCI_GETTARGETEND));
		int foundTextLen = end - start;
		unsigned char idStyle = static_cast<unsigned char>(_pEditView->f(SCI_GETSTYLEAT, posFound));

		// Search the style
		int fs = -1;
		for (size_t i = 0, len = hotspotPairs.size(); i < len ; ++i)	{

			// make sure to ignore "hotspot bit" when comparing document style with archived hotspot style
			if ((hotspotPairs[i] & ~mask) == (idStyle & ~mask))	{

				fs = hotspotPairs[i];
				_pEditView->f(SCI_STYLEGETFORE, fs);
					break;
			}
		}

		// if we found it then use it to colourize
		if (fs != -1)	{

			_pEditView->f(SCI_STARTSTYLING, start, 0xFF);
			_pEditView->f(SCI_SETSTYLING, foundTextLen, fs);
		}
		else	{ // generalize a new style and add it into a array

			style_hotspot = idStyle | mask;	// set "hotspot bit"
			hotspotPairs.push_back(style_hotspot);
			unsigned char idStyleMSBunset = idStyle & ~mask;
			char fontNameA[128];

			Style hotspotStyle;

			hotspotStyle._styleID = static_cast<int>(style_hotspot);
			_pEditView->f(SCI_STYLEGETFONT, idStyleMSBunset, reinterpret_cast<LPARAM>(fontNameA));
			constexpr size_t generic_fontnameLen = 128;
			TCHAR *generic_fontname = new TCHAR[generic_fontnameLen];

			const wchar_t * fontNameW = wmc.char2wchar(fontNameA, _nativeLangSpeaker.getLangEncoding());
			wcscpy_s(generic_fontname, generic_fontnameLen, fontNameW);
			hotspotStyle._fontName = generic_fontname;

			hotspotStyle._fgColor = static_cast<COLORREF>(_pEditView->f(SCI_STYLEGETFORE, idStyleMSBunset));
			hotspotStyle._bgColor = static_cast<COLORREF>(_pEditView->f(SCI_STYLEGETBACK, idStyleMSBunset));
			hotspotStyle._fontSize = int(_pEditView->f(SCI_STYLEGETSIZE, idStyleMSBunset));

			auto isBold = _pEditView->f(SCI_STYLEGETBOLD, idStyleMSBunset);
			auto isItalic = _pEditView->f(SCI_STYLEGETITALIC, idStyleMSBunset);
			auto isUnderline = _pEditView->f(SCI_STYLEGETUNDERLINE, idStyleMSBunset);
			hotspotStyle._fontStyle = (isBold?FONTSTYLE_BOLD:0) | (isItalic?FONTSTYLE_ITALIC:0) | (isUnderline?FONTSTYLE_UNDERLINE:0);

			int urlAction = nGUI._styleURL;
			if (urlAction == 2)
				hotspotStyle._fontStyle |= FONTSTYLE_UNDERLINE;

			_pEditView->setHotspotStyle(hotspotStyle);

			_pEditView->f(SCI_STYLESETHOTSPOT, style_hotspot, TRUE);
			int activeFG = 0xFF0000;
			Style *urlHovered = getStyleFromName(L"URL hovered");
			if (urlHovered)
				activeFG = urlHovered->_fgColor;
			_pEditView->f(SCI_SETHOTSPOTACTIVEFORE, TRUE, activeFG);
			_pEditView->f(SCI_SETHOTSPOTSINGLELINE, style_hotspot, 0);

			// colourize it!
			_pEditView->f(SCI_STARTSTYLING, start, 0xFF);
			_pEditView->f(SCI_SETSTYLING, foundTextLen, style_hotspot);
		}

		_pEditView->f(SCI_SETTARGETRANGE, posFound + foundTextLen, endPos);

		posFound = int(_pEditView->f(SCI_SEARCHINTARGET, strlen(URL_REG_EXPR), reinterpret_cast<LPARAM>(URL_REG_EXPR)));
	}

	_pEditView->f(SCI_STARTSTYLING, endStyle, 0xFF);
	_pEditView->f(SCI_SETSTYLING, 0, 0);
}

bool Notepad_plus::isConditionExprLine(int lineNumber)	{

	if (lineNumber < 0 || lineNumber > _pEditView->f(SCI_GETLINECOUNT))
		return false;

	auto startPos = _pEditView->f(SCI_POSITIONFROMLINE, lineNumber);
	auto endPos = _pEditView->f(SCI_GETLINEENDPOSITION, lineNumber);
	_pEditView->f(SCI_SETSEARCHFLAGS, SCFIND_REGEXP | SCFIND_POSIX);
	_pEditView->f(SCI_SETTARGETRANGE, startPos, endPos);

	const char ifElseForWhileExpr[] = "((else[ \t]+)?if|for|while)[ \t]*[(].*[)][ \t]*|else[ \t]*";

	auto posFound = _pEditView->f(SCI_SEARCHINTARGET, strlen(ifElseForWhileExpr), reinterpret_cast<LPARAM>(ifElseForWhileExpr));
	if (posFound != -1 && posFound != -2)	{

		auto end = _pEditView->f(SCI_GETTARGETEND);
		if (end == endPos)
			return true;
	}

	return false;
}

int Notepad_plus::findMachedBracePos(size_t startPos, size_t endPos, char targetSymbol, char matchedSymbol)	{

	if (startPos == endPos)
		return -1;

	if (startPos > endPos)	{ // backward

		int balance = 0;
		for (int i = int(startPos); i >= int(endPos); --i)	{

			char aChar = static_cast<char>(_pEditView->f(SCI_GETCHARAT, i));
			if (aChar == targetSymbol)	{

				if (!balance)
					return i;
				--balance;
			}
			else if (aChar == matchedSymbol)	{

				++balance;
			}
		}
	}
	else	{ // forward

	}
	return -1;
}

void Notepad_plus::maintainIndentation(TCHAR ch)	{

	int eolMode = int((_pEditView->f(SCI_GETEOLMODE)));
	int curLine = int((_pEditView->getCurrentLineNumber()));
	int prevLine = curLine - 1;
	int indentAmountPrevLine = 0;
	int tabWidth = int(_pEditView->f(SCI_GETTABWIDTH));

	LangType type = _pEditView->getCurrentBuffer()->getLangType();

	// Do not alter indentation if we were at the beginning of the line and we pressed Enter
	if ((((eolMode == SC_EOL_CRLF || eolMode == SC_EOL_LF) && ch == '\n') ||
		(eolMode == SC_EOL_CR && ch == '\r')) && prevLine >= 0 &&!_pEditView->getLineLength(prevLine))
		return;

	if (type == L_C || type == L_CPP || type == L_JAVA || type == L_CS || type == L_OBJC ||
		type == L_PHP || type == L_JS || type == L_JAVASCRIPT || type == L_JSP || type == L_CSS)
	{
		if (((eolMode == SC_EOL_CRLF || eolMode == SC_EOL_LF) && ch == '\n') ||
			(eolMode == SC_EOL_CR && ch == '\r'))
		{
			// Search the non-empty previous line
			while (prevLine >= 0 &&!_pEditView->getLineLength(prevLine))
				prevLine--;

			// Get previous line's Indent
			if (prevLine >= 0)	{

				indentAmountPrevLine = _pEditView->getLineIndent(prevLine);
			}

			// get previous char from current line
			int prevPos = int(_pEditView->f(SCI_GETCURRENTPOS)) - (eolMode == SC_EOL_CRLF ? 3 : 2);
			UCHAR prevChar = (UCHAR)_pEditView->f(SCI_GETCHARAT, prevPos);
			auto curPos = _pEditView->f(SCI_GETCURRENTPOS);
			UCHAR nextChar = (UCHAR)_pEditView->f(SCI_GETCHARAT, curPos);

			if (prevChar == '{')	{

				if (nextChar == '}')	{

					const char *eolChars;
					if (eolMode == SC_EOL_CRLF)
						eolChars = "\r\n";
					else if (eolMode == SC_EOL_LF)
						eolChars = "\n";
					else
						eolChars = "\r";

					_pEditView->f(SCI_INSERTTEXT, _pEditView->f(SCI_GETCURRENTPOS), reinterpret_cast<LPARAM>(eolChars));
					_pEditView->setLineIndent(curLine + 1, indentAmountPrevLine);
				}
				_pEditView->setLineIndent(curLine, indentAmountPrevLine + tabWidth);
			}
			else if (nextChar == '{')	{

				_pEditView->setLineIndent(curLine, indentAmountPrevLine);
			}
			else if (isConditionExprLine(prevLine))	{

				_pEditView->setLineIndent(curLine, indentAmountPrevLine + tabWidth);
			}
			else	{

				if (indentAmountPrevLine > 0)	{

					if (prevLine > 0 && isConditionExprLine(prevLine - 1))
						_pEditView->setLineIndent(curLine, indentAmountPrevLine - tabWidth);
					else
						_pEditView->setLineIndent(curLine, indentAmountPrevLine);
				}
			}
		}
		else if (ch == '{')	{

			// if no character in front of {, aligned with prev line's indentation
			auto startPos = _pEditView->f(SCI_POSITIONFROMLINE, curLine);
			LRESULT endPos = _pEditView->f(SCI_GETCURRENTPOS);

			for (LRESULT i = endPos - 2; i > 0 && i > startPos; --i)	{

				UCHAR aChar = (UCHAR)_pEditView->f(SCI_GETCHARAT, i);
				if (aChar != ' ' && aChar != '\t')
					return;
			}

			// Search the non-empty previous line
			while (prevLine >= 0 &&!_pEditView->getLineLength(prevLine))
				prevLine--;

			// Get previous line's Indent
			if (prevLine >= 0)	{

				indentAmountPrevLine = _pEditView->getLineIndent(prevLine);

				auto startPos2 = _pEditView->f(SCI_POSITIONFROMLINE, prevLine);
				auto endPos2 = _pEditView->f(SCI_GETLINEENDPOSITION, prevLine);
				_pEditView->f(SCI_SETSEARCHFLAGS, SCFIND_REGEXP | SCFIND_POSIX);
				_pEditView->f(SCI_SETTARGETRANGE, startPos2, endPos2);

				const char braceExpr[] = "[ \t]*\\{.*";

				int posFound = int(_pEditView->f(SCI_SEARCHINTARGET, strlen(braceExpr), reinterpret_cast<LPARAM>(braceExpr)));
				if (posFound != -1 && posFound != -2)	{

					int end = int(_pEditView->f(SCI_GETTARGETEND));
					if (end == endPos2)
						indentAmountPrevLine += tabWidth;
				}
			}

			_pEditView->setLineIndent(curLine, indentAmountPrevLine);

		}
		else if (ch == '}')	{

			// Look backward for the pair {
			int startPos = int(_pEditView->f(SCI_GETCURRENTPOS));
			if (startPos)
				startPos -= 1;
			int posFound = findMachedBracePos(startPos - 1, 0, '{', '}');

			// if no { found, do nothing
			if (posFound == -1)
				return;

			// if { is in the same line, do nothing
			int matchedPairLine = int(_pEditView->f(SCI_LINEFROMPOSITION, posFound));
			if (matchedPairLine == curLine)
				return;

			// { is in another line, get its indentation
			indentAmountPrevLine = _pEditView->getLineIndent(matchedPairLine);

			// aligned } indent with {
			_pEditView->setLineIndent(curLine, indentAmountPrevLine);

			/*
			// indent lines from { to }
			for (int i = matchedPairLine + 1; i < curLine; ++i)
				_pEditView->setLineIndent(i, indentAmountPrevLine + tabWidth);
			*/
		}
	}
	else	{ // Basic indentation mode

		if (((eolMode == SC_EOL_CRLF || eolMode == SC_EOL_LF) && ch == '\n') ||
			(eolMode == SC_EOL_CR && ch == '\r'))
		{
			// Search the non-empty previous line
			while (prevLine >= 0 &&!_pEditView->getLineLength(prevLine))
				prevLine--;

			if (prevLine >= 0)	{

				indentAmountPrevLine = _pEditView->getLineIndent(prevLine);
			}

			if (indentAmountPrevLine > 0)	{

				_pEditView->setLineIndent(curLine, indentAmountPrevLine);
			}
		}
	}
}

BOOL Notepad_plus::processFindAccel(MSG *msg) const
{
	if (not ::IsChild(_findReplaceDlg.getHSelf(), ::GetFocus()))
		return FALSE;
	return ::TranslateAccelerator(_findReplaceDlg.getHSelf(), _accelerator.getFindAccTable(), msg);
}

BOOL Notepad_plus::processIncrFindAccel(MSG *msg) const
{
	if (not ::IsChild(_incrementFindDlg.getHSelf(), ::GetFocus()))
		return FALSE;
	return ::TranslateAccelerator(_incrementFindDlg.getHSelf(), _accelerator.getIncrFindAccTable(), msg);
}

void Notepad_plus::setLanguage(LangType langType)	{

	//Add logic to prevent changing a language when a document is shared between two views
	//If so, release one document
	bool reset = false;
	Document prev = 0;
	if (bothActive())	{

		if (_mainEditView.getCurrentBufferID() == _subEditView.getCurrentBufferID())	{

			reset = true;
			_subEditView.saveCurrentPos();
			prev = _subEditView.f(SCI_GETDOCPOINTER);
			_subEditView.f(SCI_SETDOCPOINTER, 0, 0);
		}
	}
	
	if (reset)	{

		_mainEditView.getCurrentBuffer()->setLangType(langType);
	}
	else	{

		_pEditView->getCurrentBuffer()->setLangType(langType);
	}

	if (reset)	{

		_subEditView.f(SCI_SETDOCPOINTER, 0, prev);
		_subEditView.restoreCurrentPosPreStep();
	}
};

LangType Notepad_plus::menuID2LangType(int cmdID)
{
	switch (cmdID)	{

		case IDM_LANG_C	:
				return L_C;
		case IDM_LANG_CPP :
				return L_CPP;
		case IDM_LANG_JAVA :
				return L_JAVA;
		case IDM_LANG_CS :
				return L_CS;
		case IDM_LANG_HTML :
				return L_HTML;
		case IDM_LANG_XML :
				return L_XML;
		case IDM_LANG_JS :
			return L_JAVASCRIPT;
		case IDM_LANG_JSON:
			return L_JSON;
		case IDM_LANG_PHP :
				return L_PHP;
		case IDM_LANG_ASP :
				return L_ASP;
		case IDM_LANG_JSP :
				return L_JSP;
		case IDM_LANG_CSS :
				return L_CSS;
		case IDM_LANG_LUA :
				return L_LUA;
		case IDM_LANG_PERL :
				return L_PERL;
		case IDM_LANG_PYTHON :
				return L_PYTHON;
		case IDM_LANG_PASCAL :
				return L_PASCAL;
		case IDM_LANG_BATCH :
				return L_BATCH;
		case IDM_LANG_OBJC :
				return L_OBJC;
		case IDM_LANG_VB :
				return L_VB;
		case IDM_LANG_SQL :
				return L_SQL;
		case IDM_LANG_ASCII :
				return L_ASCII;
		case IDM_LANG_TEXT :
				return L_TEXT;
		case IDM_LANG_RC :
				return L_RC;
		case IDM_LANG_MAKEFILE :
				return L_MAKEFILE;
		case IDM_LANG_INI :
				return L_INI;
		case IDM_LANG_TEX :
				return L_TEX;
		case IDM_LANG_FORTRAN :
				return L_FORTRAN;
		case IDM_LANG_FORTRAN_77 :
			return L_FORTRAN_77;
		case IDM_LANG_BASH :
				return L_BASH;
		case IDM_LANG_FLASH :
				return L_FLASH;
		case IDM_LANG_NSIS :
				return L_NSIS;
		case IDM_LANG_TCL :
				return L_TCL;
		case IDM_LANG_LISP :
			return L_LISP;
		case IDM_LANG_SCHEME :
			return L_SCHEME;
		case IDM_LANG_ASM :
				return L_ASM;
		case IDM_LANG_DIFF :
				return L_DIFF;
		case IDM_LANG_PROPS :
				return L_PROPS;
		case IDM_LANG_PS:
				return L_PS;
		case IDM_LANG_RUBY:
				return L_RUBY;
		case IDM_LANG_SMALLTALK:
				return L_SMALLTALK;
		case IDM_LANG_VHDL :
				return L_VHDL;
		case IDM_LANG_KIX :
				return L_KIX;
		case IDM_LANG_CAML :
				return L_CAML;
		case IDM_LANG_ADA :
				return L_ADA;
		case IDM_LANG_VERILOG :
				return L_VERILOG;
		case IDM_LANG_MATLAB :
				return L_MATLAB;
		case IDM_LANG_HASKELL :
				return L_HASKELL;
		case IDM_LANG_AU3 :
				return L_AU3;
		case IDM_LANG_INNO :
				return L_INNO;
		case IDM_LANG_CMAKE :
				return L_CMAKE;
		case IDM_LANG_YAML :
			return L_YAML;
		case IDM_LANG_COBOL :
				return L_COBOL;
		case IDM_LANG_D :
				return L_D;
		case IDM_LANG_GUI4CLI :
				return L_GUI4CLI;
		case IDM_LANG_POWERSHELL :
				return L_POWERSHELL;
		case IDM_LANG_R :
				return L_R;
		case IDM_LANG_COFFEESCRIPT :
				return L_COFFEESCRIPT;
		case IDM_LANG_BAANC:
			return L_BAANC;
		case IDM_LANG_SREC :
				return L_SREC;
		case IDM_LANG_IHEX :
				return L_IHEX;
		case IDM_LANG_TEHEX :
				return L_TEHEX;
		case IDM_LANG_SWIFT:
			return L_SWIFT;
		case IDM_LANG_ASN1 :
				return L_ASN1;
		case IDM_LANG_AVS :
				return L_AVS;
		case IDM_LANG_BLITZBASIC :
				return L_BLITZBASIC;
		case IDM_LANG_PUREBASIC :
				return L_PUREBASIC;
		case IDM_LANG_FREEBASIC :
				return L_FREEBASIC;
		case IDM_LANG_CSOUND :
				return L_CSOUND;
		case IDM_LANG_ERLANG :
				return L_ERLANG;
		case IDM_LANG_ESCRIPT :
				return L_ESCRIPT;
		case IDM_LANG_FORTH :
				return L_FORTH;
		case IDM_LANG_LATEX :
				return L_LATEX;
		case IDM_LANG_MMIXAL :
				return L_MMIXAL;
		case IDM_LANG_NIMROD :
				return L_NIMROD;
		case IDM_LANG_NNCRONTAB :
				return L_NNCRONTAB;
		case IDM_LANG_OSCRIPT :
				return L_OSCRIPT;
		case IDM_LANG_REBOL :
				return L_REBOL;
		case IDM_LANG_REGISTRY :
				return L_REGISTRY;
		case IDM_LANG_RUST :
				return L_RUST;
		case IDM_LANG_SPICE :
				return L_SPICE;
		case IDM_LANG_TXT2TAGS :
				return L_TXT2TAGS;
		case IDM_LANG_VISUALPROLOG:
				return L_VISUALPROLOG;
		case IDM_LANG_USER :
				return L_USER;
		default:
		{
			if (cmdID >= IDM_LANG_USER && cmdID <= IDM_LANG_USER_LIMIT)	{

				return L_USER;
			}
			break; 
		}
	}
	return L_EXTERNAL;
}


void Notepad_plus::setTitle()	{

	//Get the buffer
	Buffer * buf = _pEditView->getCurrentBuffer();

	generic_string result = L"";
	if (buf->isDirty())	{

		result += L"*";
	}

	if (nGUI._shortTitlebar)	{

		result += buf->getFileName();
	}
	else	{

		result += buf->getFullPathName();
	}
	result += L" - ";
	result += _pPublicInterface->getClassName();

	if (_isAdministrator)
		result += L" (Admin)";

	::SendMessage(_pPublicInterface->getHSelf(), WM_SETTEXT, 0, reinterpret_cast<LPARAM>(result.c_str()));
}

void Notepad_plus::activateNextDoc(bool direction)	{

	int nbDoc = int(_pDocTab->nbItem()),
	curIndex = _pDocTab->getCurrentTabIndex();

//	_recBuf = _pDocTab->getBufferByIndex(curIndex);
//	// _recBuf[rB++] = _pDocTab->getBufferByIndex(curIndex);

	curIndex += direction == dirUp? -1: 1 ;

	if (curIndex >= nbDoc)	{

		if (viewVisible(otherView()))
			switchEditViewTo(otherView());
		curIndex = 0;
	}
	else if (curIndex < 0)	{
		if (viewVisible(otherView()))
			switchEditViewTo(otherView());
		curIndex = nbDoc - 1;
	}

	activateBuffer(_pDocTab->getBufferByIndex(curIndex), currentView());
}

void Notepad_plus::activateDoc(size_t pos)	{

	if (pos == static_cast<size_t>(_pDocTab->getCurrentTabIndex()))
		_pEditView->getCurrentBuffer()->increaseRecentTag();

	else if (pos < _pDocTab->nbItem())
		activateBuffer(_pDocTab->getBufferByIndex(pos), currentView());
}


static const char utflen[] = {1,1,2,3};

size_t Notepad_plus::getSelectedCharNumber(UniMode u)
{
	size_t result = 0;
	int numSel = int(_pEditView->f(SCI_GETSELECTIONS));
	if (u == uniUTF8 || u == uniCookie)	{

		for (int i=0; i < numSel; ++i)	{

			size_t line1 = _pEditView->f(SCI_LINEFROMPOSITION, _pEditView->f(SCI_GETSELECTIONNSTART, i));
			size_t line2 = _pEditView->f(SCI_LINEFROMPOSITION, _pEditView->f(SCI_GETSELECTIONNEND, i));
			for (size_t j = line1; j <= line2; ++j)	{

				size_t stpos = _pEditView->f(SCI_GETLINESELSTARTPOSITION, j);
				if (stpos != INVALID_POSITION)	{

					size_t endpos = _pEditView->f(SCI_GETLINESELENDPOSITION, j);
					for (size_t pos = stpos; pos < endpos; ++pos)	{

						unsigned char c = 0xf0 & static_cast<unsigned char>(_pEditView->f(SCI_GETCHARAT, pos));
						if (c >= 0xc0)
							pos += utflen[(c & 0x30) >>  4];
						++result;
					}
				}
			}
		}
	}
	else	{

		for (int i=0; i < numSel; ++i)	{

			size_t stpos = _pEditView->f(SCI_GETSELECTIONNSTART, i);
			size_t endpos = _pEditView->f(SCI_GETSELECTIONNEND, i);
			result += (endpos - stpos);
			size_t line1 = _pEditView->f(SCI_LINEFROMPOSITION, stpos);
			size_t line2 = _pEditView->f(SCI_LINEFROMPOSITION, endpos);
			line2 -= line1;
			if (_pEditView->f(SCI_GETEOLMODE) == SC_EOL_CRLF) line2 *= 2;
			result -= line2;
		}
		if (u != uni8Bit && u != uni7Bit) result *= 2;
	}
	return result;
}


#ifdef _OPENMP
#include <omp.h>
#endif
static inline size_t countUtf8Characters(unsigned char *buf, size_t pos, size_t endpos)	{

	size_t result = 0;
	while (pos < endpos)	{

		unsigned char c = buf[pos++];
		if ((c&0xc0) == 0x80 // do not count unexpected continuation bytes (this handles the case where an UTF-8 character is split in the middle)
			|| c == '\n' || c == '\r') continue; // do not count end of lines
		if (c >= 0xc0) 
			pos += utflen[(c & 0x30) >>  4];
		++result;
	}
	return result;
}


size_t Notepad_plus::getCurrentDocCharCount(UniMode u)
{
	if (u != uniUTF8 && u != uniCookie)	{

		size_t numLines = _pEditView->f(SCI_GETLINECOUNT);
		auto result = _pEditView->f(SCI_GETLENGTH);
		size_t lines = !numLines?0:numLines-1;
		if (_pEditView->f(SCI_GETEOLMODE) == SC_EOL_CRLF) lines *= 2;
		result -= lines;
		return (result < 0) ? 0 : result;
	}
 	else	{

		// Note that counting is not well defined for invalid UTF-8 characters.
		// This method is O(filelength) regardless of the number of characters we count (due to SCI_GETCHARACTERPOINTER);
		// it would not be appropriate for counting characters in a small selection.
		size_t result = 0;

		size_t endpos = _pEditView->f(SCI_GETLENGTH);
		unsigned char* buf = (unsigned char*)_pEditView->f(SCI_GETCHARACTERPOINTER); // Scintilla doc said the pointer can be invalidated by any other "execute"

#ifdef _OPENMP // parallel counting of characters with OpenMP
		if (endpos > 50000)	{ // starting threads takes time; for small files it is better to simply count in one thread

			#pragma omp parallel reduction(+: result)
			{
				// split in chunks of same size (except last chunk if it's not evenly divisible)
				unsigned int num_threads = omp_get_num_threads();
				unsigned int thread_num = omp_get_thread_num();
				size_t chunk_size = endpos/num_threads;
				size_t pos = chunk_size*thread_num;
				size_t endpos_local = (thread_num == num_threads-1) ? endpos : pos+chunk_size;
				result = countUtf8Characters(buf, pos, endpos_local);
			}
		}
		else
#endif
		{
			result = countUtf8Characters(buf, 0, endpos);
		}
 		return result;
 	}
}


bool Notepad_plus::isFormatUnicode(UniMode u)	{

	return (u != uni8Bit && u != uni7Bit && u != uniUTF8 && u != uniCookie);
}

int Notepad_plus::getBOMSize(UniMode u)	{

	switch(u)	{

		case uni16LE:
		case uni16BE:
			return 2;
		case uniUTF8:
			return 3;
		default:
			return 0;
	}
}

size_t Notepad_plus::getSelectedAreas()
{
	size_t numSel = _pEditView->f(SCI_GETSELECTIONS);
	if (numSel == 1) // either 0 or 1 selection
		return (_pEditView->f(SCI_GETSELECTIONNSTART, 0) == _pEditView->f(SCI_GETSELECTIONNEND, 0)) ? 0 : 1;
	return (_pEditView->f(SCI_SELECTIONISRECTANGLE)) ? 1 : numSel;
}

size_t Notepad_plus::getSelectedBytes()
{
	size_t numSel = _pEditView->f(SCI_GETSELECTIONS);
	size_t result = 0;
	for (size_t i = 0; i < numSel; ++i)
		result += (_pEditView->f(SCI_GETSELECTIONNEND, i) - _pEditView->f(SCI_GETSELECTIONNSTART, i));
	return result;
}
int Notepad_plus::wordCount()	{
	FindOption env;
	env._str2Search = L"[^ 	\\\\.,;:!?()+\\r\\n\\-\\*/=\\]\\[{}&~\"'`|@$%<>\\^]+";
	env._searchType = FindRegex;
	return _findReplaceDlg.processAll(ProcessCountAll, &env, true);
}

void Notepad_plus::updateStatusBar()	{
	TCHAR strSel[32];
	int selByte = 0, selLine = 0;
	
	_pEditView->getSelectedCount(selByte, selLine);
	wsprintf(strSel, L"%s %s", commafyInt(_pEditView->getUnicodeSelectedLength()).c_str(), commafyInt(selLine).c_str());
/* 	long selected_length = _pEditView->getUnicodeSelectedLength();
	if (selected_length != -1)
		wsprintf(strSel, L"%s %s", commafyInt(selected_length).c_str(), commafyInt(selLine).c_str());
	else	wsprintf(strSel, L"N/A"); */

	generic_string strLnCol, p;
	strLnCol = L"L ";
	strLnCol += commafyInt(_pEditView->getCurrentLineNumber() + 1);
	strLnCol += L"  C ";
	strLnCol += commafyInt(_pEditView->getCurrentColumnNumber() + 1);
	strLnCol += L"  ";
	strLnCol += strSel;
	strLnCol += L"  ";
	strLnCol += _statusBar.beText;

	if (offsetSB) {
		p = commafyInt(_pEditView->getOffset());
		p += L"  ";
		_statusBar.setText(STATUSBAR_CUR_POS,strLnCol.c_str(), p.c_str());
	}
	else
		_statusBar.setText(STATUSBAR_CUR_POS,strLnCol.c_str());


	TCHAR strDocLen[32];
	wsprintf(strDocLen, L"%s  %s",
		commafyInt(_pEditView->getCurrentDocLen()).c_str(),
		commafyInt(_pEditView->f(SCI_GETLINECOUNT)).c_str());

	_statusBar.setText( STATUSBAR_DOC_SIZE,strDocLen);

}

void Notepad_plus::dropFiles(HDROP hdrop)	{

	if (hdrop)	{

		// Determinate in which view the file(s) is (are) dropped
		POINT p;
		::DragQueryPoint(hdrop, &p);
		HWND hWin = ::RealChildWindowFromPoint(_pPublicInterface->getHSelf(), p);
		if (!hWin) return;

		if ((_subEditView.getHSelf() == hWin) || (_subDocTab.getHSelf() == hWin))
			switchEditViewTo(SUB_VIEW);
		else
			switchEditViewTo(MAIN_VIEW);

		int filesDropped = ::DragQueryFile(hdrop, 0xffffffff, NULL, 0);

		vector<generic_string> folderPaths;
		vector<generic_string> filePaths;
		for (int i = 0; i < filesDropped; ++i)	{

			TCHAR pathDropped[MAX_PATH];
			::DragQueryFile(hdrop, i, pathDropped, MAX_PATH);
			if (::PathIsDirectory(pathDropped))	{

				size_t len = lstrlen(pathDropped);
				if (len > 0 && pathDropped[len - 1] != TCHAR('\\'))	{

					pathDropped[len] = TCHAR('\\');
					pathDropped[len + 1] = TCHAR('\0');
				}
				folderPaths.push_back(pathDropped);
			}
			else	{

				filePaths.push_back(pathDropped);
			}
		}
		
				bool isOldMode = nppGUI._isFolderDroppedOpenFiles;

		if (isOldMode || !folderPaths.size())	{ // old mode or new mode + only files

			BufferID lastOpened = BUFFER_INVALID;
			for (int i = 0; i < filesDropped; ++i)	{

				TCHAR pathDropped[MAX_PATH];
				::DragQueryFile(hdrop, i, pathDropped, MAX_PATH);
				BufferID test = doOpen(pathDropped);
				if (test != BUFFER_INVALID)
					lastOpened = test;
			}

			if (lastOpened != BUFFER_INVALID)	{

				switchToFile(lastOpened);
			}
		}
		else if (not isOldMode && (folderPaths.size() != 0 && filePaths.size() != 0))	{ // new mode && both folders & files

			// display error & do nothing
			_nativeLangSpeaker.messageBox("DroppingFolderAsProjectModeWarning",
				_pPublicInterface->getHSelf(),
				L"You can only drop files or folders but not both, because you're in dropping Folder as Project mode.\ryou have to enable \"Open all files of folder instead of launching Folder as Workspace on folder dropping\" in \"Default Directory\" section of Preferences dialog to make this operation work.",
				L"Invalid action",
				MB_OK | MB_APPLMODAL);
		}
		else if (not isOldMode && (folderPaths.size() != 0 && !filePaths.size()))	{ // new mode && only folders

			// process new mode
			launchFileBrowser(folderPaths);
		}

		::DragFinish(hdrop);
		// Put Notepad_plus to forefront
		// May not work for Win2k, but OK for lower versions
		// Note: how to drop a file to an iconic window?
		// Actually, it is the Send To command that generates a drop.
		if (::IsIconic(_pPublicInterface->getHSelf()))	{

			::ShowWindow(_pPublicInterface->getHSelf(), SW_RESTORE);
		}
		::SetForegroundWindow(_pPublicInterface->getHSelf());
	}
}

void Notepad_plus::checkModifiedDocument(bool bCheckOnlyCurrentBuffer)	{

	//this will trigger buffer updates. If the status changes, Notepad++ will be informed and can do its magic
	MainFileManager.checkFilesystemChanges(bCheckOnlyCurrentBuffer);
}

void Notepad_plus::getMainClientRect(RECT &rc) const
{
	_pPublicInterface->getClientRect(rc);
	rc.top += _rebarTop.getHeight();
	rc.bottom -= rc.top + _rebarBottom.getHeight() + _statusBar.getHeight();
}

void Notepad_plus::showView(int whichOne)	{

	if (viewVisible(whichOne))	//no use making visible view visible
		return;

	if (_mainWindowStatus & WindowUserActive)	{

		_pMainSplitter->setWin0(&_subSplitter);
		_pMainWindow = _pMainSplitter;
	}
	else	{

		_pMainWindow = &_subSplitter;
	}

	if (whichOne == MAIN_VIEW)	{

		_mainEditView.display(true);
		_mainDocTab.display(true);
	}
	else if (whichOne == SUB_VIEW)	{

		_subEditView.display(true);
		_subDocTab.display(true);
	}
	_pMainWindow->display(true);

	_mainWindowStatus |= (whichOne==MAIN_VIEW)?WindowMainActive:WindowSubActive;

	//Send sizing info to make windows fit
	::SendMessage(_pPublicInterface->getHSelf(), WM_SIZE, 0, 0);
}

bool Notepad_plus::viewVisible(int whichOne)	{

	int viewToCheck = (whichOne == SUB_VIEW?WindowSubActive:WindowMainActive);
	return (_mainWindowStatus & viewToCheck) != 0;
}

void Notepad_plus::hideView(int whichOne)	{

	if (!(bothActive()))	return;	//cannot close if both viewsin invisible

	Window * windowToSet = (whichOne == MAIN_VIEW)?&_subDocTab:&_mainDocTab;
	if (_mainWindowStatus & WindowUserActive)	{

		_pMainSplitter->setWin0(windowToSet);
	}
	else	{

		// otherwise the main window is the spltter container that we just created
		_pMainWindow = windowToSet;
	}

	_subSplitter.display(false);	//hide splitter
	//hide scintilla and doctab
	if (whichOne == MAIN_VIEW)	{

		_mainEditView.display(false);
		_mainDocTab.display(false);
	}
	else if (whichOne == SUB_VIEW)	{

		_subEditView.display(false);
		_subDocTab.display(false);
	}

	// resize the main window
	::SendMessage(_pPublicInterface->getHSelf(), WM_SIZE, 0, 0);

	switchEditViewTo(otherView(whichOne));
	int viewToDisable = (whichOne == SUB_VIEW?WindowSubActive:WindowMainActive);
	_mainWindowStatus &= ~viewToDisable;
}

bool Notepad_plus::loadStyles()	{

		return param.reloadStylers();
}

bool Notepad_plus::canHideView(int whichOne)	{

	if (!viewVisible(whichOne))
		return false;	//cannot hide hidden view
	if (!bothActive())
		return false;	//cannot hide only window
	DocTabView * tabToCheck = (whichOne == MAIN_VIEW)?&_mainDocTab:&_subDocTab;
	Buffer * buf = MainFileManager.getBufferByID(tabToCheck->getBufferByIndex(0));
	bool canHide = ((tabToCheck->nbItem() == 1) && !buf->isDirty() && buf->isUntitled());
	return canHide;
}

bool Notepad_plus::isEmpty()	{

	if (bothActive()) return false;

	DocTabView * tabToCheck = (_mainWindowStatus & WindowMainActive) ? &_mainDocTab : &_subDocTab;
	
	Buffer * buf = MainFileManager.getBufferByID(tabToCheck->getBufferByIndex(0));
	bool isEmpty = ((tabToCheck->nbItem() == 1) && !buf->isDirty() && buf->isUntitled());
	return isEmpty;
}

void Notepad_plus::loadBufferIntoView(BufferID id, int whichOne, bool dontClose)	{

	DocTabView * tabToOpen = (whichOne == MAIN_VIEW)?&_mainDocTab:&_subDocTab;
	ScintillaEditView * viewToOpen = (whichOne == MAIN_VIEW)?&_mainEditView:&_subEditView;

	//check if buffer exists
	int index = tabToOpen->getIndexByBuffer(id);
	if (index != -1)	//already open, done
		return;

	BufferID idToClose = BUFFER_INVALID;
	//Check if the tab has a single clean buffer. Close it if so
	if (!dontClose && tabToOpen->nbItem() == 1)	{

		idToClose = tabToOpen->getBufferByIndex(0);
		Buffer * buf = MainFileManager.getBufferByID(idToClose);
		if (buf->isDirty() || !buf->isUntitled())	{

			idToClose = BUFFER_INVALID;
		}
	}

	MainFileManager.addBufferReference(id, viewToOpen);

	//close clean doc. Use special logic to prevent flicker of tab showing then hiding
	if (idToClose != BUFFER_INVALID)	{

		tabToOpen->setBuffer(0, id);	//index 0 since only one open
		activateBuffer(id, whichOne);	//activate. DocTab already activated but not a problem
		MainFileManager.closeBuffer(idToClose, viewToOpen);	//delete the buffer
		if (_pFileSwitcherPanel)
			_pFileSwitcherPanel->closeItem(idToClose, whichOne);
	}
	else	{

		tabToOpen->addBuffer(id);
	}
}

bool Notepad_plus::removeBufferFromView(BufferID id, int whichOne)	{

	DocTabView * tabToClose = (whichOne == MAIN_VIEW) ? &_mainDocTab : &_subDocTab;
	ScintillaEditView * viewToClose = (whichOne == MAIN_VIEW) ? &_mainEditView : &_subEditView;

	//check if buffer exists
	int index = tabToClose->getIndexByBuffer(id);
	if (index == -1)        //doesn't exist, done
		return false;

	Buffer * buf = MainFileManager.getBufferByID(id);

	//Cannot close doc if last and clean
	if (tabToClose->nbItem() == 1)	{

		if (!buf->isDirty() && buf->isUntitled())	{

			return false;
		}
	}

	int active = tabToClose->getCurrentTabIndex();
	if (active == index)	{ //need an alternative (close real doc, put empty one back)

		if (tabToClose->nbItem() == 1)	{  //need alternative doc, add new one. Use special logic to prevent flicker of adding new tab then closing other

			BufferID newID = MainFileManager.newEmptyDocument();
			MainFileManager.addBufferReference(newID, viewToClose);
			tabToClose->setBuffer(0, newID);        //can safely use id 0, last (only) tab open
			activateBuffer(newID, whichOne);        //activate. DocTab already activated but not a problem
		}
		else	{

			int toActivate = 0;
			//activate next doc, otherwise prev if not possible
			if (size_t(active) == tabToClose->nbItem() - 1)	{ //prev

				toActivate = active - 1;
			}
			else	{

				toActivate = active;    //activate the 'active' index. Since we remove the tab first, the indices shift (on the right side)
			}
			tabToClose->deletItemAt((size_t)index); //delete first
			_isFolding = true; // So we can ignore events while folding is taking place
			activateBuffer(tabToClose->getBufferByIndex(toActivate), whichOne);     //then activate. The prevent jumpy tab behaviour
			_isFolding = false;
		}
	}
	else	{

		tabToClose->deletItemAt((size_t)index);
	}

	MainFileManager.closeBuffer(id, viewToClose);
	return true;
}

int Notepad_plus::switchEditViewTo(int gid)	{

	if (currentView() == gid)	{
		//make sure focus is ok, then leave
		_pEditView->focus();	//set the focus
		return gid;
	}

	if (!viewVisible(gid))
		return currentView();	//cannot activate invisible view
	int oldView = currentView();
	_activeView = otherView();

	//Good old switcheroo
	std::swap(_pDocTab, _pNonDocTab);
	std::swap(_pEditView, _pNonEditView);

	_pEditView->beSwitched();
	_pEditView->focus();	//set the focus

	if (_pDocMap)	{

		_pDocMap->initWrapMap();
	}

	if (nppGUI.isSnapshotMode())	{

		// Before switching off, synchronize backup file
		MainFileManager.backupCurrentBuffer();
	}

	notifyBufferActivated(_pEditView->getCurrentBufferID(), currentView());
	return oldView;
}

void Notepad_plus::dockUserDlg()	{

	if (!_pMainSplitter)	{

		_pMainSplitter = new SplitterContainer;
		_pMainSplitter->init(_pPublicInterface->getHinst(), _pPublicInterface->getHSelf());

		Window *pWindow;
		if (_mainWindowStatus & (WindowMainActive | WindowSubActive))
				pWindow = &_subSplitter;
		else
				pWindow = _pDocTab;

		_pMainSplitter->create(pWindow, ScintillaEditView::getUserDefineDlg(), 8, SplitterMode::RIGHT_FIX, 45);
	}

	if (bothActive())
		_pMainSplitter->setWin0(&_subSplitter);
	else
		_pMainSplitter->setWin0(_pDocTab);

	_pMainSplitter->display();

	_mainWindowStatus |= WindowUserActive;
	_pMainWindow = _pMainSplitter;

	::SendMessage(_pPublicInterface->getHSelf(), WM_SIZE, 0, 0);
}

void Notepad_plus::undockUserDlg()	{

	// a cause de surchargement de "display"
	::ShowWindow(_pMainSplitter->getHSelf(), SW_HIDE);

	if (bothActive())
		_pMainWindow = &_subSplitter;
	else
		_pMainWindow = _pDocTab;

	::SendMessage(_pPublicInterface->getHSelf(), WM_SIZE, 0, 0);

	_mainWindowStatus &= ~WindowUserActive;
	(ScintillaEditView::getUserDefineDlg())->display();
}

void Notepad_plus::docOpenInNewInstance(FileTransferMode mode, int x, int y)	{

	BufferID bufferID = _pEditView->getCurrentBufferID();
	Buffer * buf = MainFileManager.getBufferByID(bufferID);
	if (buf->isUntitled() || buf->isDirty())
		return;

	TCHAR nppName[MAX_PATH];
	::GetModuleFileName(NULL, nppName, MAX_PATH);
	generic_string command = L"\"";
	command += nppName;
	command += L"\"";
	command += L" \"$(FULL_CURRENT_PATH)\" -multiInst -nosession";

	if (x)	{

		TCHAR pX[10];
		generic_itoa(x, pX, 10);
		command += L" -x";
		command += pX;
	}

	if (y)	{

		TCHAR pY[10];
		generic_itoa(y, pY, 10);
		command += L" -y";
		command += pY;
	}

	command += L" -l";
	command += ScintillaEditView::langNames[buf->getLangType()].lexerName;
	command += L" -n";
	command += to_wstring(_pEditView->getCurrentLineNumber() + 1);
	command += L" -c";
	command += to_wstring(_pEditView->getCurrentColumnNumber() + 1);

	Command cmd(command);
	cmd.run(_pPublicInterface->getHSelf());
	if (mode == TransferMove)	{

		doClose(bufferID, currentView());
		if (noOpenedDoc())
			::SendMessage(_pPublicInterface->getHSelf(), WM_CLOSE, 0, 0);
	}
}

void Notepad_plus::docGotoAnotherEditView(FileTransferMode mode)	{

	// Test if it's only doc to transfer on the hidden view
	// If so then do nothing
	if (mode == TransferMove)	{

		if (_pDocTab->nbItem() == 1)	{

			ScintillaEditView *pOtherView = NULL;
			if (_pEditView == &_mainEditView)	{

				pOtherView = &_subEditView;
			}
			else if (_pEditView == &_subEditView)	{

				pOtherView = &_mainEditView;
			}
			else
				return;

			if (!pOtherView->isVisible())
				return;
		}
	}

	//First put the doc in the other view if not present (if it is, activate it).
	//Then if needed close in the original tab
	BufferID current = _pEditView->getCurrentBufferID();
	int viewToGo = otherView();
	int indexFound = _pNonDocTab->getIndexByBuffer(current);
	if (indexFound != -1)	{	//activate it

		activateBuffer(current, otherView());
	}
	else	{	//open the document, also copying the position

		// If both the views are visible then first save the position of non-edit view
		// So that moving document between views does not lose caret position
		// How it works =>
		//		non-edit view becomes edit view as document from edit view is sent to non edit view
		//		restoreCurrentPos is called on non-edit view, which will restore the position of
		//		active document/tab on non-edit view  (whatever position we set in below if condition)
		if (_pEditView->isVisible() && _pNonEditView->isVisible())	{

			_pNonEditView->saveCurrentPos();
		}

		loadBufferIntoView(current, viewToGo);
		Buffer *buf = MainFileManager.getBufferByID(current);
		_pEditView->saveCurrentPos();	//allow copying of position
		buf->setPosition(buf->getPosition(_pEditView), _pNonEditView);
		_pNonEditView->restoreCurrentPosPreStep();	//set position
		activateBuffer(current, viewToGo);
	}

	//Open the view if it was hidden
	int viewToOpen = (viewToGo == SUB_VIEW?WindowSubActive:WindowMainActive);
	if (!(_mainWindowStatus & viewToOpen))	{

		showView(viewToGo);
	}

	bool monitoringWasOn = false;

	//Close the document if we transfered the document instead of cloning it
	if (mode == TransferMove)	{

		Buffer *buf = MainFileManager.getBufferByID(current);
		monitoringWasOn = buf->isMonitoringOn();

		//just close the activate document, since thats the one we moved (no search)
		doClose(_pEditView->getCurrentBufferID(), currentView());
	} // else it was cone, so leave it

	//Activate the other view since thats where the document went
	switchEditViewTo(viewToGo);

	if (monitoringWasOn)	{

		command(IDM_VIEW_MONITORING);
	}
}

bool Notepad_plus::activateBuffer(BufferID id, int whichOne){

	if (nppGUI.isSnapshotMode())
		// Before switching off, synchronize backup file
		MainFileManager.backupCurrentBuffer();

	Buffer& pBuf = *MainFileManager.getBufferByID(id);
	bool reload = pBuf.getNeedReload();
	if (reload)	{

		MainFileManager.reloadBuffer(id);
		pBuf.setNeedReload(false);
	}
	if (whichOne == MAIN_VIEW)	{

		if (_mainDocTab.activateBuffer(id))	{	//only activate if possible

			_isFolding = true;
			_mainEditView.activateBuffer(id);
			_isFolding = false;
		}
		else
			return false;
	}
	else	{

		if (_subDocTab.activateBuffer(id))	{

			_isFolding = true;
			_subEditView.activateBuffer(id);
			_isFolding = false;
		}
		else
			return false;
	}

	if (reload)		performPostReload(whichOne);

	notifyBufferActivated(id, whichOne);

	bool isCurrBuffDetection = (nppGUI._fileAutoDetection & cdEnabledNew) ? true : false;
	if (!reload && isCurrBuffDetection)
		// Buffer has been activated, now check for file modification If enabled for current buffer
		pBuf.checkFileState();

	return true;
}

void Notepad_plus::performPostReload(int whichOne)	{

	bool toEnd = (nGUI._fileAutoDetection & cdGo2end) ? true : false;
	if (!toEnd)
		return;
	if (whichOne == MAIN_VIEW)	{

		_mainEditView.f(SCI_GOTOLINE, _mainEditView.f(SCI_GETLINECOUNT) -1);
	}
	else	{

		_subEditView.f(SCI_GOTOLINE, _subEditView.f(SCI_GETLINECOUNT) -1);
	}
}

void Notepad_plus::bookmarkNext(bool forwardScan)	{

	size_t lineno = _pEditView->getCurrentLineNumber();
	int sci_marker = SCI_MARKERNEXT;
	size_t lineStart = lineno + 1;	//Scan starting from next line
	int lineRetry = 0;				//If not found, try from the beginning
	if (!forwardScan)	{

		lineStart = lineno - 1;		//Scan starting from previous line
		lineRetry = int(_pEditView->f(SCI_GETLINECOUNT));	//If not found, try from the end
		sci_marker = SCI_MARKERPREVIOUS;
	}
	int nextLine = int(_pEditView->f(sci_marker, lineStart, 1 << MARK_BOOKMARK));
	if (nextLine < 0)
		nextLine = int(_pEditView->f(sci_marker, lineRetry, 1 << MARK_BOOKMARK));

	if (nextLine < 0)
		return;

	_pEditView->f(SCI_ENSUREVISIBLEENFORCEPOLICY, nextLine);
	_pEditView->f(SCI_GOTOLINE, nextLine);
}

void Notepad_plus::staticCheckMenuAndTB() const
{
	// Visibility of invisible characters
	bool wsTabShow = _pEditView->isInvisibleCharsShown();
	bool eolShow = _pEditView->isEolVisible();

	bool onlyWS = false;
	bool onlyEOL = false;
	bool bothWSEOL = false;
	if (wsTabShow)	{

		if (eolShow)	{

			bothWSEOL = true;
		}
		else	{

			onlyWS = true;
		}
	}
	else if (eolShow)	{

		onlyEOL = true;
	}

	checkMenuItem(IDM_VIEW_TAB_SPACE, onlyWS);
	checkMenuItem(IDM_VIEW_EOL, onlyEOL);
	checkMenuItem(IDM_VIEW_ALL_CHARACTERS, bothWSEOL);
	_toolBar.setCheck(IDM_VIEW_ALL_CHARACTERS, bothWSEOL);

	// Visibility of the indentation guide line
	bool b = _pEditView->isShownIndentGuide();
	checkMenuItem(IDM_VIEW_INDENT_GUIDE, b);
	_toolBar.setCheck(IDM_VIEW_INDENT_GUIDE, b);


	// Wrap
	b = _pEditView->isWrap();
	checkMenuItem(IDM_VIEW_WRAP, b);
	_toolBar.setCheck(IDM_VIEW_WRAP, b);
	checkMenuItem(IDM_VIEW_WRAP_SYMBOL, _pEditView->isWrapSymbolVisible());
}


void Notepad_plus::dynamicCheckMenuAndTB() const
{
	//Format conversion
	enableConvertMenuItems(_pEditView->getCurrentBuffer()->getEolFormat());
	checkUnicodeMenuItems();
}


void Notepad_plus::enableConvertMenuItems(EolType format) const
{
	enableCommand(IDM_FORMAT_TODOS, (format != EolType::windows), MENU);
	enableCommand(IDM_FORMAT_TOUNIX, (format != EolType::unix), MENU);
	enableCommand(IDM_FORMAT_TOMAC, (format != EolType::macos), MENU);
}


void Notepad_plus::checkUnicodeMenuItems() const
{
	Buffer *buf = _pEditView->getCurrentBuffer();
	UniMode um = buf->getUnicodeMode();
	int encoding = buf->getEncoding();

	int id = -1;
	switch (um)	{

		case uniUTF8   : id = IDM_FORMAT_UTF_8; break;
		case uni16BE   : id = IDM_FORMAT_UCS_2BE; break;
		case uni16LE   : id = IDM_FORMAT_UCS_2LE; break;
		case uniCookie : id = IDM_FORMAT_AS_UTF_8; break;
		case uni8Bit   : id = IDM_FORMAT_ANSI; break;
	}

	if (encoding == -1)	{

		// Uncheck all in the sub encoding menu
		HMENU _formatMenuHandle = ::GetSubMenu(_mainMenuHandle, MENUINDEX_ENCODE);
		doCheck(_formatMenuHandle, IDM_FORMAT_ENCODE);
		::CheckMenuItem(_mainMenuHandle, IDM_FORMAT_ENCODE, MF_UNCHECKED | MF_BYCOMMAND);

		if (id == -1)	{ //um == uni16BE_NoBOM || um == uni16LE_NoBOM

			// Uncheck all in the main encoding menu
			::CheckMenuRadioItem(_mainMenuHandle, IDM_FORMAT_ANSI, IDM_FORMAT_AS_UTF_8, IDM_FORMAT_ANSI, MF_BYCOMMAND);
			::CheckMenuItem(_mainMenuHandle, IDM_FORMAT_ANSI, MF_UNCHECKED | MF_BYCOMMAND);
		}
		else	{

			::CheckMenuRadioItem(_mainMenuHandle, IDM_FORMAT_ANSI, IDM_FORMAT_AS_UTF_8, id, MF_BYCOMMAND);
		}
	}
	else	{

		int cmdID = em.getIndexFromEncoding(encoding);
		if (cmdID == -1)	{

			//printStr(L"Encoding problem. Encoding is not added in encoding_table?");
			return;
		}
		cmdID += IDM_FORMAT_ENCODE;

		// Uncheck all in the main encoding menu
		::CheckMenuRadioItem(_mainMenuHandle, IDM_FORMAT_ANSI, IDM_FORMAT_AS_UTF_8, IDM_FORMAT_ANSI, MF_BYCOMMAND);
		::CheckMenuItem(_mainMenuHandle, IDM_FORMAT_ANSI, MF_UNCHECKED | MF_BYCOMMAND);

		// Check the encoding item
		HMENU _formatMenuHandle = ::GetSubMenu(_mainMenuHandle, MENUINDEX_ENCODE);
		doCheck(_formatMenuHandle, cmdID);
	}
}

void Notepad_plus::showAutoComp()	{

	bool isFromPrimary = _pEditView == &_mainEditView;
	AutoCompletion * autoC = isFromPrimary?&_autoCompleteMain:&_autoCompleteSub;
	autoC->showApiComplete();
}

void Notepad_plus::showPathCompletion()	{

	bool isFromPrimary = _pEditView == &_mainEditView;
	AutoCompletion * autoC = isFromPrimary?&_autoCompleteMain:&_autoCompleteSub;
	autoC->showPathCompletion();
}

void Notepad_plus::autoCompFromCurrentFile(bool autoInsert)	{

	bool isFromPrimary = _pEditView == &_mainEditView;
	AutoCompletion * autoC = isFromPrimary?&_autoCompleteMain:&_autoCompleteSub;
	autoC->showWordComplete(autoInsert);
}

void Notepad_plus::showFunctionComp()	{

	bool isFromPrimary = _pEditView == &_mainEditView;
	AutoCompletion * autoC = isFromPrimary?&_autoCompleteMain:&_autoCompleteSub;
	autoC->showFunctionComplete();
}

static generic_string extractSymbol(TCHAR firstChar, TCHAR secondChar, const TCHAR *str2extract)	{

	bool found = false;
	constexpr size_t extractedLen = 128;
	TCHAR extracted[extractedLen] = {'\0'};

	for (size_t i = 0, j = 0, len = lstrlen(str2extract) ; i < len && j < extractedLen - 1; ++i)	{

		if (found)	{

			if (!str2extract[i] || str2extract[i] == ' ')	{

				extracted[j] = '\0';
				return generic_string(extracted);
			}
			extracted[j++] = str2extract[i];
		}
		else	{

			if (!str2extract[i])
				return L"";

			if (str2extract[i] == firstChar && str2extract[i+1] == secondChar)	{

				found = true;
				++i;
			}
		}
	}
	return  generic_string(extracted);
};

bool Notepad_plus::doBlockComment(comment_mode currCommentMode)	{

	Buffer * buf = _pEditView->getCurrentBuffer();
	if (buf->isReadOnly())		return false;

	//-- BlockToStreamComment:
	const TCHAR *commentStart,
	*commentEnd,
	*commentLineSymbol;
	
	generic_string symbol,
	symbolStart, symbolEnd;

	//Single Line Comment/Uncomment/Toggle can have two modes:
	// * a NORMAL MODE which uses a commentLineSymbol to comment/uncomment code per line, and
	// * an ADVANCED MODE which uses commentStart and commentEnd symbols to comment/uncomment code per line.
	//The NORMAL MODE is used for all Lexers which have a commentLineSymbol defined.
	//The ADVANCED MODE is only available for Lexers which do not have a commentLineSymbol but commentStreamSymbols (start/end) defined.
	//The ADVANCED MODE behaves the same way as the NORMAL MODE (comment/uncomment every single line in the selection range separately)
	//but uses two symbols to accomplish this.
	bool isSingleLineAdvancedMode = false;

	if (buf->getLangType() == L_USER)	{

		UserLangContainer * userLangContainer = param.getULCFromName(buf->getUserDefineLangName());
		if (!userLangContainer)		return false;

		symbol = extractSymbol('0', '0', userLangContainer->_keywordLists[SCE_USER_KWLIST_COMMENTS]);
		commentLineSymbol = symbol.c_str();
		// BlockToStreamComment: Needed to decide, if stream-comment can be called below!
		symbolStart = extractSymbol('0', '3', userLangContainer->_keywordLists[SCE_USER_KWLIST_COMMENTS]);
		commentStart = symbolStart.c_str();
		symbolEnd = extractSymbol('0', '4', userLangContainer->_keywordLists[SCE_USER_KWLIST_COMMENTS]);
		commentEnd = symbolEnd.c_str();
	}
	else	{

		commentLineSymbol = buf->getCommentLineSymbol();
		// BlockToStreamComment: Needed to decide, if stream-comment can be called below!
		commentStart = buf->getCommentStart();
		commentEnd = buf->getCommentEnd();
	}

	if ((!commentLineSymbol) || (!commentLineSymbol[0]) || (!commentLineSymbol))	{

	// BlockToStreamComment: If there is no block-comment symbol, try the stream comment:
		if (!(!commentStart || !commentStart[0] ||!commentStart || !commentEnd || !commentEnd[0] ||!commentEnd))	{

			if (currCommentMode == cm_comment)	{

				//Do an advanced "Single Line Comment" by using stream-comment symbols (start/end) per line in this case.
				isSingleLineAdvancedMode = true;
				//return doStreamComment(); //Use "Block Comment" for this.
			}
			else if (currCommentMode == cm_uncomment)	{

				//"undoStreamComment()" can be more flexible than "isSingleLineAdvancedMode = true", 
				//since it can uncomment more embedded levels at once and the commentEnd symbol can be located everywhere. 
				//But, depending on the selection start/end position, the first/last selected line may not be uncommented properly!
				return undoStreamComment(false);
				//isSingleLineAdvancedMode = true;
			}
			else if (currCommentMode == cm_toggle)	{

				//Do an advanced "Toggle Single Line Comment" by using stream-comment symbols (start/end) per line in this case.
				isSingleLineAdvancedMode = true;
			}
			else
				return false;
		}
		else
			return false;
	}

	//For Single Line NORMAL MODE
	generic_string comment;
	size_t comment_length = 0;

	//For Single Line ADVANCED MODE
	generic_string advCommentStart;
	generic_string advCommentEnd;
	size_t advCommentStart_length = 0;
	size_t advCommentEnd_length = 0;

	constexpr TCHAR aSpace[] { L" "};

	//Only values that have passed through will be assigned, to be sure they are valid!
	if (not isSingleLineAdvancedMode)	{

		comment = commentLineSymbol;

		if (!(buf->getLangType() == L_BAANC)) // BaanC standardization - no space.
			comment += aSpace;

		comment_length = comment.length();
	}
	else	{ // isSingleLineAdvancedMode

		advCommentStart = commentStart;
		advCommentStart += aSpace;
		advCommentEnd = aSpace;
		advCommentEnd += commentEnd;

		advCommentStart_length = advCommentStart.length();
		advCommentEnd_length = advCommentEnd.length();
	}

	size_t selectionStart = _pEditView->f(SCI_GETSELECTIONSTART);
	size_t selectionEnd = _pEditView->f(SCI_GETSELECTIONEND);
	size_t caretPosition = _pEditView->f(SCI_GETCURRENTPOS);
	// checking if caret is located in _beginning_ of selected block
	bool move_caret = caretPosition < selectionEnd;
	int selStartLine = int(_pEditView->f(SCI_LINEFROMPOSITION, selectionStart));
	int selEndLine = int(_pEditView->f(SCI_LINEFROMPOSITION, selectionEnd));
	int lines = selEndLine - selStartLine;
	// "caret return" is part of the last selected line
	if ((lines > 0) && (selectionEnd == static_cast<size_t>(_pEditView->f(SCI_POSITIONFROMLINE, selEndLine))))
		selEndLine--;
	// count lines which were un-commented to decide if undoStreamComment() shall be called.
	int nUncomments = 0;
	//Some Lexers need line-comments at the beginning of a line.
	const bool avoidIndent = (buf->getLangType() == L_FORTRAN_77 || buf->getLangType() == L_BAANC);
	//Some Lexers comment blank lines, per their standards.
	const bool commentEmptyLines = (buf->getLangType() == L_BAANC);

	_pEditView->f(SCI_BEGINUNDOACTION);

	for (int i = selStartLine; i <= selEndLine; ++i)	{

		size_t lineStart = _pEditView->f(SCI_POSITIONFROMLINE, i);
		size_t lineIndent = _pEditView->f(SCI_GETLINEINDENTPOSITION, i);
		size_t lineEnd = _pEditView->f(SCI_GETLINEENDPOSITION, i);

		// empty lines are not commented, unless required by the language.
		if (lineIndent == lineEnd && !commentEmptyLines)
			continue;

		if (avoidIndent)
			lineIndent = lineStart;

		size_t linebufferSize = lineEnd - lineIndent + 1;
		TCHAR* linebuf = new TCHAR[linebufferSize];

		_pEditView->getGenericText(linebuf, linebufferSize, lineIndent, lineEnd);

		generic_string linebufStr = linebuf;
		delete [] linebuf;

			if (currCommentMode != cm_comment)	{ // uncomment/toggle

			if (not isSingleLineAdvancedMode)	{

				// In order to do get case insensitive comparison use strnicmp() instead case-sensitive comparison.
				//      Case insensitive comparison is needed e.g. for "REM" and "rem" in Batchfiles.
				if (generic_strnicmp(linebufStr.c_str(), comment.c_str(), !(buf->getLangType() == L_BAANC) ? comment_length - 1 : comment_length) == 0)	{

					size_t len = linebufStr[comment_length - 1] == aSpace[0] ? comment_length : !(buf->getLangType() == L_BAANC) ? comment_length - 1 : comment_length;

					_pEditView->f(SCI_SETSEL, lineIndent, lineIndent + len);
					_pEditView->replaceSelWith("");

					// SELECTION RANGE ADJUSTMENT .......................
					if (i == selStartLine)	{ // first selected line

						if (selectionStart > lineIndent + len)
							selectionStart -= len;
						else if (selectionStart > lineIndent)
							selectionStart = lineIndent;
					} // ................................................
					if (i == selEndLine)	{ // last selected line

						if (selectionEnd > lineIndent + len)
							selectionEnd -= len;
						else if (selectionEnd > lineIndent)	{

							selectionEnd = lineIndent;
							if (lineIndent == lineStart && i != selStartLine)
								++selectionEnd; // avoid caret return in this case
						}
					} // ................................................
					else // every iteration except the last selected line
						selectionEnd -= len;
					// ..................................................

					++nUncomments;
					continue;
				}
			}
			else	{ // isSingleLineAdvancedMode

				if ((generic_strnicmp(linebufStr.c_str(), advCommentStart.c_str(), advCommentStart_length - 1) == 0) &&
					(generic_strnicmp(linebufStr.substr(linebufStr.length() - advCommentEnd_length + 1, advCommentEnd_length - 1).c_str(), advCommentEnd.substr(1, advCommentEnd_length - 1).c_str(), advCommentEnd_length - 1) == 0))
				{
					size_t startLen = linebufStr[advCommentStart_length - 1] == aSpace[0] ? advCommentStart_length : advCommentStart_length - 1;
					size_t endLen = linebufStr[linebufStr.length() - advCommentEnd_length] == aSpace[0] ? advCommentEnd_length : advCommentEnd_length - 1;

					_pEditView->f(SCI_SETSEL, lineIndent, lineIndent + startLen);
					_pEditView->replaceSelWith("");
					_pEditView->f(SCI_SETSEL, lineEnd - startLen - endLen, lineEnd - startLen);
					_pEditView->replaceSelWith("");

					// SELECTION RANGE ADJUSTMENT .......................
					if (i == selStartLine)	{ // first selected line

						if (selectionStart > lineEnd - endLen)
							selectionStart = lineEnd - startLen - endLen;
						else if (selectionStart > lineIndent + startLen)
							selectionStart -= startLen;
						else if (selectionStart > lineIndent)
							selectionStart = lineIndent;
					} // ................................................
					if (i == selEndLine)	{ // last selected line

						if (selectionEnd > lineEnd)
							selectionEnd -= (startLen + endLen);
						else if (selectionEnd > lineEnd - endLen)
							selectionEnd = lineEnd - startLen - endLen;
						else if (selectionEnd > lineIndent + startLen)
							selectionEnd -= startLen;
						else if (selectionEnd > lineIndent)	{

							selectionEnd = lineIndent;
							if (lineIndent == lineStart && i != selStartLine)
								++selectionEnd; // avoid caret return in this case
						}
					} // ................................................
					else // every iteration except the last selected line
						selectionEnd -= (startLen + endLen);
					// ..................................................

					++nUncomments;
					continue;
				}
			}
		} // uncomment/toggle

		if (currCommentMode != cm_uncomment)	{ // comment/toggle

			if (not isSingleLineAdvancedMode)	{

				_pEditView->insertGenericTextFrom(lineIndent, comment.c_str());

				// SELECTION RANGE ADJUSTMENT .......................
				if (i == selStartLine)	{ // first selected line

					if (selectionStart >= lineIndent)
						selectionStart += comment_length;
				} // ................................................
				if (i == selEndLine)	{ // last selected line

					if (selectionEnd >= lineIndent)
						selectionEnd += comment_length;
				} // ................................................
				else // every iteration except the last selected line
					selectionEnd += comment_length;
				// ..................................................
			}
			else	{ // isSingleLineAdvancedMode

				_pEditView->insertGenericTextFrom(lineIndent, advCommentStart.c_str());
				_pEditView->insertGenericTextFrom(lineEnd + advCommentStart_length, advCommentEnd.c_str());

				// SELECTION RANGE ADJUSTMENT .......................
				if (i == selStartLine)	{ // first selected line

					if (selectionStart >= lineIndent)
						selectionStart += advCommentStart_length;
				} // ................................................
				if (i == selEndLine)	{ // last selected line

					if (selectionEnd > lineEnd)
						selectionEnd += (advCommentStart_length + advCommentEnd_length);
					else if (selectionEnd >= lineIndent)
						selectionEnd += advCommentStart_length;
				} // ................................................
				else // every iteration except the last selected line
					selectionEnd += (advCommentStart_length + advCommentEnd_length);
				// ..................................................
			}
		} // comment/toggle
	} // for (...)

	if (move_caret)	{

		// moving caret to the beginning of selected block
		_pEditView->f(SCI_GOTOPOS, selectionEnd);
		_pEditView->f(SCI_SETCURRENTPOS, selectionStart);
	}
	else	{

		_pEditView->f(SCI_SETSEL, selectionStart, selectionEnd);
	}
	_pEditView->f(SCI_ENDUNDOACTION);

	// undoStreamComment: If there were no block-comments to un-comment try uncommenting of stream-comment.
	if ((currCommentMode == cm_uncomment) && (!nUncomments))	{

		return undoStreamComment(false);
	}
	return true;
}

bool Notepad_plus::doStreamComment()	{

	const TCHAR *commentStart;
	const TCHAR *commentEnd;

	generic_string symbolStart;
	generic_string symbolEnd;

	// BlockToStreamComment:
	const TCHAR *commentLineSymbol;
	generic_string symbol;

	Buffer * buf = _pEditView->getCurrentBuffer();
	// Avoid side-effects (e.g. cursor moves number of comment-characters) when file is read-only.
	if (buf->isReadOnly())
		return false;

	if (buf->getLangType() == L_USER)	{

		UserLangContainer * userLangContainer = param.getULCFromName(buf->getUserDefineLangName());

		if (!userLangContainer)
			return false;

		// BlockToStreamComment: Next two lines needed to decide, if block-comment can be called below!
		symbol = extractSymbol('0', '0', userLangContainer->_keywordLists[SCE_USER_KWLIST_COMMENTS]);
		commentLineSymbol = symbol.c_str();

		symbolStart = extractSymbol('0', '3', userLangContainer->_keywordLists[SCE_USER_KWLIST_COMMENTS]);
		commentStart = symbolStart.c_str();
		symbolEnd = extractSymbol('0', '4', userLangContainer->_keywordLists[SCE_USER_KWLIST_COMMENTS]);
		commentEnd = symbolEnd.c_str();
	}
	else	{

		// BlockToStreamComment: Next line needed to decide, if block-comment can be called below!
		commentLineSymbol = buf->getCommentLineSymbol();
		commentStart = buf->getCommentStart();
		commentEnd = buf->getCommentEnd();
	}

	// BlockToStreamComment: If there is no stream-comment symbol, try the block comment:
	if ((!commentStart) || (!commentStart[0]) || (!commentStart) || (!commentEnd) || (!commentEnd[0]) || (!commentEnd))	{

		if (!(!commentLineSymbol || !commentLineSymbol[0] ||!commentLineSymbol))
			return doBlockComment(cm_comment);
		else
		return false;
	}

	generic_string start_comment(commentStart);
	generic_string end_comment(commentEnd);
	generic_string white_space(L" ");

	start_comment += white_space;
	white_space += end_comment;
	end_comment = white_space;
	size_t start_comment_length = start_comment.length();
	size_t selectionStart = _pEditView->f(SCI_GETSELECTIONSTART);
	size_t selectionEnd = _pEditView->f(SCI_GETSELECTIONEND);
	size_t caretPosition = _pEditView->f(SCI_GETCURRENTPOS);
	// checking if caret is located in _beginning_ of selected block
	bool move_caret = caretPosition < selectionEnd;

	// if there is no selection?
	if (selectionEnd - selectionStart <= 0)	{

		auto selLine = _pEditView->f(SCI_LINEFROMPOSITION, selectionStart);
		selectionStart = _pEditView->f(SCI_GETLINEINDENTPOSITION, selLine);
		selectionEnd = _pEditView->f(SCI_GETLINEENDPOSITION, selLine);
	}
	_pEditView->f(SCI_BEGINUNDOACTION);
	_pEditView->insertGenericTextFrom(selectionStart, start_comment.c_str());
	selectionEnd += start_comment_length;
	selectionStart += start_comment_length;
	_pEditView->insertGenericTextFrom(selectionEnd, end_comment.c_str());
	if (move_caret)	{

		// moving caret to the beginning of selected block
		_pEditView->f(SCI_GOTOPOS, selectionEnd);
		_pEditView->f(SCI_SETCURRENTPOS, selectionStart);
	}
	else	{

		_pEditView->f(SCI_SETSEL, selectionStart, selectionEnd);
	}
	_pEditView->f(SCI_ENDUNDOACTION);
	return true;
}

void Notepad_plus::saveScintillasZoom()	{

		ScintillaViewParams & svp = (ScintillaViewParams &)param.getSVP();
	svp._zoom = static_cast<int>(_mainEditView.f(SCI_GETZOOM));
	svp._zoom2 = static_cast<int>(_subEditView.f(SCI_GETZOOM));
}

bool Notepad_plus::addCurrentMacro()	{

		vector<MacroShortcut> & theMacros = param.getMacroList();

	int nbMacro = int(theMacros.size());

	int cmdID = ID_MACRO + nbMacro;
	MacroShortcut ms(Shortcut(), _macro, cmdID);
	ms.init(_pPublicInterface->getHinst(), _pPublicInterface->getHSelf());

	if (ms.doDialog() != -1)	{

		HMENU hMacroMenu = ::GetSubMenu(_mainMenuHandle, MENUINDEX_MACRO);
		int const posBase = 6;	//separator at index 5
		if (!nbMacro)	{

			::InsertMenu(hMacroMenu, posBase-1, MF_BYPOSITION, static_cast<UINT>(-1), 0);	//no separator yet, add one

				// Insert the separator and modify/delete command
			::InsertMenu(hMacroMenu, posBase + nbMacro + 1, MF_BYPOSITION, static_cast<UINT>(-1), 0);

			NativeLangSpeaker *pNativeLangSpeaker = param.getNativeLangSpeaker();
			generic_string nativeLangShortcutMapperMacro = pNativeLangSpeaker->getNativeLangMenuString(IDM_SETTING_SHORTCUT_MAPPER_MACRO);
			if (nativeLangShortcutMapperMacro == L"")
				nativeLangShortcutMapperMacro = L"Modify Shortcut/Delete Macro...";

			::InsertMenu(hMacroMenu, posBase + nbMacro + 2, MF_BYCOMMAND, IDM_SETTING_SHORTCUT_MAPPER_MACRO, nativeLangShortcutMapperMacro.c_str());
		}
		theMacros.push_back(ms);
		::InsertMenu(hMacroMenu, posBase + nbMacro, MF_BYPOSITION, cmdID, ms.toMenuItemString().c_str());
		_accelerator.updateShortcuts();
		param.setShortcutDirty();
		return true;
	}
	return false;
}

void Notepad_plus::changeToolBarIcons()	{

	_toolBar.changeIcons();
}

bool Notepad_plus::switchToFile(BufferID id)	{

	int i = 0,
	iView = currentView();
	if (id == BUFFER_INVALID)		return false;

	if ((i = _pDocTab->getIndexByBuffer(id)) != -1)
		iView = currentView();
	else if ((i = _pNonDocTab->getIndexByBuffer(id)) != -1)
		iView = otherView();

	if (i != -1)	{

		switchEditViewTo(iView);
		activateBuffer(id, currentView());
		return true;
	}
	return false;
}

void Notepad_plus::getTaskListInfo(TaskListInfo *tli)	{

	int currentNbDoc = int(_pDocTab->nbItem());
	int nonCurrentNbDoc = int(_pNonDocTab->nbItem());

	tli->_currentIndex = 0;

	if (!viewVisible(otherView()))
		nonCurrentNbDoc = 0;

	for (int i = 0 ; i < currentNbDoc ; ++i)	{

		BufferID bufID = _pDocTab->getBufferByIndex(i);
		Buffer * b = MainFileManager.getBufferByID(bufID);
		int status = b->isReadOnly()?tb_ro:(b->isDirty()?tb_unsaved:tb_saved);
		tli->_tlfsLst.push_back(TaskLstFnStatus(currentView(), i, b->getFullPathName(), status, (void *)bufID));
	}
	for (int i = 0 ; i < nonCurrentNbDoc ; ++i)	{

		BufferID bufID = _pNonDocTab->getBufferByIndex(i);
		Buffer * b = MainFileManager.getBufferByID(bufID);
		int status = b->isReadOnly()?tb_ro:(b->isDirty()?tb_unsaved:tb_saved);
		tli->_tlfsLst.push_back(TaskLstFnStatus(otherView(), i, b->getFullPathName(), status, (void *)bufID));
	}
}


bool Notepad_plus::goToPreviousIndicator(int indicID2Search, bool isWrap) const
{
	auto position = _pEditView->f(SCI_GETCURRENTPOS);
	auto docLen = _pEditView->getCurrentDocLen();

	bool isInIndicator = _pEditView->f(SCI_INDICATORVALUEAT, indicID2Search,  position) != 0;
	auto posStart = _pEditView->f(SCI_INDICATORSTART, indicID2Search,  position);
	auto posEnd = _pEditView->f(SCI_INDICATOREND, indicID2Search,  position);

	// pre-condition
	if ((!posStart) && (posEnd == docLen - 1))
		return false;

	if (posStart <= 0)	{

		if (!isWrap)
			return false;

		isInIndicator = _pEditView->f(SCI_INDICATORVALUEAT, indicID2Search,  docLen - 1) != 0;
		posStart = _pEditView->f(SCI_INDICATORSTART, indicID2Search,  docLen - 1);
	}

	if (isInIndicator)	{ // try to get out of indicator

		posStart = _pEditView->f(SCI_INDICATORSTART, indicID2Search, posStart - 1);
		if (posStart <= 0)	{

			if (!isWrap)
				return false;
			posStart = _pEditView->f(SCI_INDICATORSTART, indicID2Search,  docLen - 1);
		}
	}

	auto newPos = posStart - 1;
	posStart = _pEditView->f(SCI_INDICATORSTART, indicID2Search, newPos);
	posEnd = _pEditView->f(SCI_INDICATOREND, indicID2Search, newPos);

	// found
	if (_pEditView->f(SCI_INDICATORVALUEAT, indicID2Search, posStart))	{

		
		nppGUI._disableSmartHiliteTmp = true;

		auto currentline = _pEditView->f(SCI_LINEFROMPOSITION, posEnd);
		_pEditView->f(SCI_ENSUREVISIBLE, currentline);	// make sure target line is unfolded

		_pEditView->f(SCI_SETSEL, posEnd, posStart);
		_pEditView->f(SCI_SCROLLCARET);
		return true;
	}
	return false;
}

bool Notepad_plus::goToNextIndicator(int indicID2Search, bool isWrap) const
{
	auto position = _pEditView->f(SCI_GETCURRENTPOS);
	int docLen = _pEditView->getCurrentDocLen();

	bool isInIndicator = _pEditView->f(SCI_INDICATORVALUEAT, indicID2Search,  position) != 0;
	auto posStart = _pEditView->f(SCI_INDICATORSTART, indicID2Search,  position);
	auto posEnd = _pEditView->f(SCI_INDICATOREND, indicID2Search,  position);

	// pre-condition
	if ((!posStart) && (posEnd == docLen - 1))
		return false;

	if (posEnd >= docLen)	{

		if (!isWrap)
			return false;

		isInIndicator = _pEditView->f(SCI_INDICATORVALUEAT, indicID2Search,  0) != 0;
		posEnd = _pEditView->f(SCI_INDICATOREND, indicID2Search, 0);
	}

	if (isInIndicator)	{ // try to get out of indicator

		posEnd = _pEditView->f(SCI_INDICATOREND, indicID2Search, posEnd);

		if (posEnd >= docLen)	{

			if (!isWrap)
				return false;
			posEnd = _pEditView->f(SCI_INDICATOREND, indicID2Search, 0);
		}
	}
	auto newPos = posEnd;
	posStart = _pEditView->f(SCI_INDICATORSTART, indicID2Search, newPos);
	posEnd = _pEditView->f(SCI_INDICATOREND, indicID2Search, newPos);

	// found
	if (_pEditView->f(SCI_INDICATORVALUEAT, indicID2Search, posStart))	{

		
		nppGUI._disableSmartHiliteTmp = true;

		auto currentline = _pEditView->f(SCI_LINEFROMPOSITION, posEnd);
		_pEditView->f(SCI_ENSUREVISIBLE, currentline);	// make sure target line is unfolded

		_pEditView->f(SCI_SETSEL, posStart, posEnd);
		_pEditView->f(SCI_SCROLLCARET);
		return true;
	}
	return false;
}

void Notepad_plus::fullScreenToggle()	{

	if (!_beforeSpecialView.isFullScreen)	{	//toggle fullscreen on

		_beforeSpecialView._winPlace.length = sizeof(_beforeSpecialView._winPlace);
		::GetWindowPlacement(_pPublicInterface->getHSelf(), &_beforeSpecialView._winPlace);

		RECT fullscreenArea;		//RECT used to calculate window fullscreen size
		//Preset view area, in case something fails, primary monitor values
		fullscreenArea.top = 0;
		fullscreenArea.left = 0;
		fullscreenArea.right = GetSystemMetrics(SM_CXSCREEN);
		fullscreenArea.bottom = GetSystemMetrics(SM_CYSCREEN);

		//if (_winVersion != WV_NT)
		{
			HMONITOR currentMonitor;	//Handle to monitor where fullscreen should go
			MONITORINFO mi;				//Info of that monitor
			//Caution, this will not work on windows 95, so probably add some checking of some sorts like Unicode checks, IF 95 were to be supported
			currentMonitor = ::MonitorFromWindow(_pPublicInterface->getHSelf(), MONITOR_DEFAULTTONEAREST);	//should always be valid monitor handle
			mi.cbSize = sizeof(MONITORINFO);
			if (::GetMonitorInfo(currentMonitor, &mi) != FALSE)	{

				fullscreenArea = mi.rcMonitor;
				fullscreenArea.right -= fullscreenArea.left;
				fullscreenArea.bottom -= fullscreenArea.top;
			}
		}

		//Setup GUI
		int bs = buttonStatus_fullscreen;
		if (_beforeSpecialView.isPostIt)	{

				bs |= buttonStatus_postit;
		}
		else	{

			//only change the GUI if not already done by postit
			_beforeSpecialView.isMenuShown = ::SendMessage(_pPublicInterface->getHSelf(), NPPM_ISMENUHIDDEN, 0, 0) != TRUE;
			if (_beforeSpecialView.isMenuShown)
				::SendMessage(_pPublicInterface->getHSelf(), NPPM_HIDEMENU, 0, TRUE);

			//Hide rebar
			_rebarTop.display(false);
			_rebarBottom.display(false);
		}
		_restoreButton.setButtonStatus(bs);

		//Hide window so windows can properly update it
		::ShowWindow(_pPublicInterface->getHSelf(), SW_HIDE);

		//Set popup style for fullscreen window and store the old style
		if (!_beforeSpecialView.isPostIt)	{

			_beforeSpecialView.preStyle = ::SetWindowLongPtr(_pPublicInterface->getHSelf(), GWL_STYLE, WS_POPUP);
			if (!_beforeSpecialView.preStyle)	{

				//something went wrong, use default settings
				_beforeSpecialView.preStyle = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN;
			}
		}

		//Set fullscreen window, highest non-top z-order, show the window and redraw it (refreshing the windowmanager cache aswell)
		::ShowWindow(_pPublicInterface->getHSelf(), SW_SHOW);
		::SetWindowPos(_pPublicInterface->getHSelf(), HWND_TOP, fullscreenArea.left, fullscreenArea.top, fullscreenArea.right, fullscreenArea.bottom, SWP_NOZORDER|SWP_DRAWFRAME|SWP_FRAMECHANGED);
		::SetForegroundWindow(_pPublicInterface->getHSelf());

		// show restore button
		_restoreButton.doDialog(_nativeLangSpeaker.isRTL());

		RECT rect;
		GetWindowRect(_restoreButton.getHSelf(), &rect);
		int w = rect.right - rect.left;
		int h = rect.bottom - rect.top;

		RECT nppRect;
		GetWindowRect(_pPublicInterface->getHSelf(), &nppRect);
		int x = nppRect.right - w;
		int y = nppRect.top;
		::MoveWindow(_restoreButton.getHSelf(), x, y, w, h, FALSE);

		_pEditView->focus();
	}
	else	{	//toggle fullscreen off

		//Hide window for updating, restore style and menu then restore position and Z-Order
		::ShowWindow(_pPublicInterface->getHSelf(), SW_HIDE);

		_restoreButton.setButtonStatus(buttonStatus_fullscreen ^ _restoreButton.getButtonStatus());
		_restoreButton.display(false);

		//Setup GUI
		if (!_beforeSpecialView.isPostIt)	{

			//only change the GUI if postit isnt active
			if (_beforeSpecialView.isMenuShown)
				::SendMessage(_pPublicInterface->getHSelf(), NPPM_HIDEMENU, 0, FALSE);

			//Show rebar
			_rebarTop.display(true);
			_rebarBottom.display(true);
		}

		//Set old style if not fullscreen
		if (!_beforeSpecialView.isPostIt)	{

			::SetWindowLongPtr( _pPublicInterface->getHSelf(), GWL_STYLE, _beforeSpecialView.preStyle);
			//Redraw the window and refresh windowmanager cache, dont do anything else, sizing is done later on
			::SetWindowPos(_pPublicInterface->getHSelf(), HWND_TOP,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_DRAWFRAME|SWP_FRAMECHANGED);
			::ShowWindow(_pPublicInterface->getHSelf(), SW_SHOW);
		}

		if (_beforeSpecialView._winPlace.length)	{

			if (_beforeSpecialView._winPlace.showCmd == SW_SHOWMAXIMIZED)	{

				::ShowWindow(_pPublicInterface->getHSelf(), SW_SHOWMAXIMIZED);
			}
			else	{

				::SetWindowPlacement(_pPublicInterface->getHSelf(), &_beforeSpecialView._winPlace);
			}
		}
		else	{	//fallback

			::ShowWindow(_pPublicInterface->getHSelf(), SW_SHOW);
		}
	}
	//::SetForegroundWindow(_pPublicInterface->getHSelf());
	_beforeSpecialView.isFullScreen = !_beforeSpecialView.isFullScreen;
	::SendMessage(_pPublicInterface->getHSelf(), WM_SIZE, 0, 0);
	if (_beforeSpecialView.isPostIt)	{

		// show restore button on the right position
		RECT rect;
		GetWindowRect(_restoreButton.getHSelf(), &rect);
		int w = rect.right - rect.left;
		int h = rect.bottom - rect.top;

		RECT nppRect;
		GetWindowRect(_pPublicInterface->getHSelf(), &nppRect);
		int x = nppRect.right - w - w;
		int y = nppRect.top + 1;
		::MoveWindow(_restoreButton.getHSelf(), x, y, w, h, FALSE);
	}
}

void Notepad_plus::postItToggle()	{

		if (!_beforeSpecialView.isPostIt)	{	// PostIt disabled, enable it

		// get current status before switch to postIt
		//check these always
		{
			_beforeSpecialView.isAlwaysOnTop = ::GetMenuState(_mainMenuHandle, IDM_VIEW_ALWAYSONTOP, MF_BYCOMMAND) == MF_CHECKED;
			_beforeSpecialView.isTabbarShown = ::SendMessage(_pPublicInterface->getHSelf(), NPPM_ISTABBARHIDDEN, 0, 0) != TRUE;
			_beforeSpecialView.isStatusbarShown = nppGUI._statusBarShow;
			if (nppGUI._statusBarShow)
				::SendMessage(_pPublicInterface->getHSelf(), NPPM_HIDESTATUSBAR, 0, TRUE);
			if (_beforeSpecialView.isTabbarShown)
				::SendMessage(_pPublicInterface->getHSelf(), NPPM_HIDETABBAR, 0, TRUE);
			if (!_beforeSpecialView.isAlwaysOnTop)
				::SendMessage(_pPublicInterface->getHSelf(), WM_COMMAND, IDM_VIEW_ALWAYSONTOP, 0);
		}
		//Only check these if not fullscreen
		int bs = buttonStatus_postit;
		if (_beforeSpecialView.isFullScreen)	{

				bs |= buttonStatus_fullscreen;
		}
		else	{

			_beforeSpecialView.isMenuShown = ::SendMessage(_pPublicInterface->getHSelf(), NPPM_ISMENUHIDDEN, 0, 0) != TRUE;
			if (_beforeSpecialView.isMenuShown)
				::SendMessage(_pPublicInterface->getHSelf(), NPPM_HIDEMENU, 0, TRUE);

			//Hide rebar
			_rebarTop.display(false);
			_rebarBottom.display(false);
		}
		_restoreButton.setButtonStatus(bs);

		// PostIt!

		//Set popup style for fullscreen window and store the old style
		if (!_beforeSpecialView.isFullScreen)	{

			//Hide window so windows can properly update it
			::ShowWindow(_pPublicInterface->getHSelf(), SW_HIDE);
			_beforeSpecialView.preStyle = ::SetWindowLongPtr( _pPublicInterface->getHSelf(), GWL_STYLE, WS_POPUP );
			if (!_beforeSpecialView.preStyle)	{

				//something went wrong, use default settings
				_beforeSpecialView.preStyle = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN;
			}
			//Redraw the window and refresh windowmanager cache, dont do anything else, sizing is done later on
			::SetWindowPos(_pPublicInterface->getHSelf(), HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_DRAWFRAME|SWP_FRAMECHANGED);
			::ShowWindow(_pPublicInterface->getHSelf(), SW_SHOW);
		}

		// show restore button
		_restoreButton.doDialog(_nativeLangSpeaker.isRTL());

		RECT rect;
		GetWindowRect(_restoreButton.getHSelf(), &rect);
		int w = rect.right - rect.left;
		int h = rect.bottom - rect.top;

		RECT nppRect;
		GetWindowRect(_pPublicInterface->getHSelf(), &nppRect);
		int x = nppRect.right - w - w;
		int y = nppRect.top + 1;
		::MoveWindow(_restoreButton.getHSelf(), x, y, w, h, FALSE);

		_pEditView->focus();
	}
	else	{	//PostIt enabled, disable it

		_restoreButton.setButtonStatus(buttonStatus_postit ^ _restoreButton.getButtonStatus());
		_restoreButton.display(false);

		//Setup GUI
		if (!_beforeSpecialView.isFullScreen)	{

			//only change the these parts of GUI if not already done by fullscreen
			if (_beforeSpecialView.isMenuShown)
				::SendMessage(_pPublicInterface->getHSelf(), NPPM_HIDEMENU, 0, FALSE);

			//Show rebar
			_rebarTop.display(true);
			_rebarBottom.display(true);
		}
		//Do this GUI config always
		if (_beforeSpecialView.isStatusbarShown)
			::SendMessage(_pPublicInterface->getHSelf(), NPPM_HIDESTATUSBAR, 0, FALSE);
		if (_beforeSpecialView.isTabbarShown)
			::SendMessage(_pPublicInterface->getHSelf(), NPPM_HIDETABBAR, 0, FALSE);
		if (!_beforeSpecialView.isAlwaysOnTop)
			::SendMessage(_pPublicInterface->getHSelf(), WM_COMMAND, IDM_VIEW_ALWAYSONTOP, 0);

		//restore window style if not fullscreen
		if (!_beforeSpecialView.isFullScreen)	{

			//dwStyle |= (WS_CAPTION | WS_SIZEBOX);
			::ShowWindow(_pPublicInterface->getHSelf(), SW_HIDE);
			::SetWindowLongPtr(_pPublicInterface->getHSelf(), GWL_STYLE, _beforeSpecialView.preStyle);

			//Redraw the window and refresh windowmanager cache, dont do anything else, sizing is done later on
			::SetWindowPos(_pPublicInterface->getHSelf(), HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_DRAWFRAME|SWP_FRAMECHANGED);
			::ShowWindow(_pPublicInterface->getHSelf(), SW_SHOW);
		}
	}

	_beforeSpecialView.isPostIt = !_beforeSpecialView.isPostIt;
	::SendMessage(_pPublicInterface->getHSelf(), WM_SIZE, 0, 0);
}

void Notepad_plus::doSynScorll(HWND whichView)	{

	int column = 0;
	int line = 0;
	ScintillaEditView *pView;

	// var for Line
	int mainCurrentLine, subCurrentLine;

	// var for Column
	int mxoffset, sxoffset;
	int pixel;
	int mainColumn, subColumn;

	if (whichView == _mainEditView.getHSelf())	{

		if (_syncInfo._isSynScollV)	{

			// Compute for Line
			mainCurrentLine = int(_mainEditView.f(SCI_GETFIRSTVISIBLELINE));
			subCurrentLine = int(_subEditView.f(SCI_GETFIRSTVISIBLELINE));
			line = mainCurrentLine - _syncInfo._line - subCurrentLine;
		}
		if (_syncInfo._isSynScollH)	{

			// Compute for Column
			mxoffset = int(_mainEditView.f(SCI_GETXOFFSET));
			pixel = int(_mainEditView.f(SCI_TEXTWIDTH, STYLE_DEFAULT, reinterpret_cast<LPARAM>("P")));
			mainColumn = mxoffset/pixel;

			sxoffset = int(_subEditView.f(SCI_GETXOFFSET));
			pixel = int(_subEditView.f(SCI_TEXTWIDTH, STYLE_DEFAULT, reinterpret_cast<LPARAM>("P")));
			subColumn = sxoffset/pixel;
			column = mainColumn - _syncInfo._column - subColumn;
		}
		pView = &_subEditView;
	}
	else if (whichView == _subEditView.getHSelf())	{

		if (_syncInfo._isSynScollV)	{

			// Compute for Line
			mainCurrentLine = int(_mainEditView.f(SCI_GETFIRSTVISIBLELINE));
			subCurrentLine = int(_subEditView.f(SCI_GETFIRSTVISIBLELINE));
			line = subCurrentLine + _syncInfo._line - mainCurrentLine;
		}
		if (_syncInfo._isSynScollH)	{

			// Compute for Column
			mxoffset = int(_mainEditView.f(SCI_GETXOFFSET));
			pixel = int(_mainEditView.f(SCI_TEXTWIDTH, STYLE_DEFAULT, reinterpret_cast<LPARAM>("P")));
			mainColumn = mxoffset/pixel;

			sxoffset = int(_subEditView.f(SCI_GETXOFFSET));
			pixel = int(_subEditView.f(SCI_TEXTWIDTH, STYLE_DEFAULT, reinterpret_cast<LPARAM>("P")));
			subColumn = sxoffset/pixel;
			column = subColumn + _syncInfo._column - mainColumn;
		}
		pView = &_mainEditView;
	}
	else
		return;

	pView->scroll(column, line);
}

bool Notepad_plus::getIntegralDockingData(tTbData & dockData, int & iCont, bool & isVisible)	{

	DockingManagerData & dockingData = (DockingManagerData &)(param).getNppGUI()._dockingData;

	for (size_t i = 0, len = dockingData._pluginDockInfo.size(); i < len ; ++i)	{

		const PluginDlgDockingInfo & pddi = dockingData._pluginDockInfo[i];

		if (!generic_stricmp(pddi._name.c_str(), dockData.pszModuleName) && (pddi._internalID == dockData.dlgID))	{

			iCont				= pddi._currContainer;
			isVisible			= pddi._isVisible;
			dockData.iPrevCont	= pddi._prevContainer;

			if (dockData.iPrevCont != -1)	{

				int cont = (pddi._currContainer < DOCKCONT_MAX ? pddi._prevContainer : pddi._currContainer);
				RECT rc;
				if (dockingData.getFloatingRCFrom(cont, rc))
					dockData.rcFloat = rc;
			}
			return true;
		}
	}
	return false;
}


void Notepad_plus::getCurrentOpenedFiles(Session & session, bool includUntitledDoc)	{

	_mainEditView.saveCurrentPos();	//save position so itll be correct in the session
	_subEditView.saveCurrentPos();	//both views
	session._activeView = currentView();
	session._activeMainIndex = _mainDocTab.getCurrentTabIndex();
	session._activeSubIndex = _subDocTab.getCurrentTabIndex();

	//Use _invisibleEditView to temporarily open documents to retrieve markers
	Document oldDoc = _invisibleEditView.f(SCI_GETDOCPOINTER);
	constexpr int nbElem = 2;
	DocTabView *docTab[nbElem];
	docTab[0] = &_mainDocTab;
	docTab[1] = &_subDocTab;
	for (size_t k = 0; k < nbElem; ++k)	{

		for (size_t i = 0, len = docTab[k]->nbItem(); i < len ; ++i)	{

			BufferID bufID = docTab[k]->getBufferByIndex(i);
			ScintillaEditView *editView = !k?&_mainEditView:&_subEditView;
			size_t activeIndex = !k ? session._activeMainIndex : session._activeSubIndex;
			vector<sessionFileInfo> *viewFiles = (vector<sessionFileInfo> *)(!k?&(session._mainViewFiles):&(session._subViewFiles));

			Buffer * buf = MainFileManager.getBufferByID(bufID);

			if (buf->isUntitled() && buf->docLength() == 0)
				continue;

			if (!includUntitledDoc)
				if (!PathFileExists(buf->getFullPathName()))
					continue;


			generic_string	languageName = getLangFromMenu(buf);
			const TCHAR *langName = languageName.c_str();
			sessionFileInfo sfi(buf->getFullPathName(), langName, buf->getEncoding(), buf->getUserReadOnly(), buf->getPosition(editView), buf->getBackupFileName().c_str(), buf->getLastModifiedTimestamp(), buf->getMapPosition());

			sfi._isMonitoring = buf->isMonitoringOn();

			_invisibleEditView.f(SCI_SETDOCPOINTER, 0, buf->getDocument());
			size_t maxLine = static_cast<size_t>(_invisibleEditView.f(SCI_GETLINECOUNT));

			for (size_t j = 0 ; j < maxLine ; ++j)	{

				if ((_invisibleEditView.f(SCI_MARKERGET, j) & (1 << MARK_BOOKMARK)) != 0)	{

					sfi._marks.push_back(j);
				}
			}

			if (i == activeIndex)	{

				editView->getCurrentFoldStates(sfi._foldStates);
			}
			else	{

				sfi._foldStates = buf->getHeaderLineState(editView);
			}
			viewFiles->push_back(sfi);
		}
	}
	_invisibleEditView.f(SCI_SETDOCPOINTER, 0, oldDoc);
}

bool Notepad_plus::str2Cliboard(const generic_string & str2cpy)	{

	return str2Clipboard(str2cpy, _pPublicInterface->getHSelf());
}

//ONLY CALL IN CASE OF EMERGENCY: EXCEPTION
//This function is destructive
bool Notepad_plus::emergency(const generic_string& emergencySavedDir)	{

	::CreateDirectory(emergencySavedDir.c_str(), NULL);
	return dumpFiles(emergencySavedDir.c_str(), L"File");
}

bool Notepad_plus::dumpFiles(const TCHAR * outdir, const TCHAR * fileprefix)	{

	//start dumping unsaved files to recovery directory
	bool somethingsaved = false;
	bool somedirty = false;
	TCHAR savePath[MAX_PATH] = {0};

	//rescue primary
	for (size_t i = 0; i < MainFileManager.getNbBuffers(); ++i)	{

		Buffer * docbuf = MainFileManager.getBufferByIndex(i);
		if (!docbuf->isDirty())	//skip saved documents
			continue;
		else
			somedirty = true;

		const TCHAR * unitext = (docbuf->getUnicodeMode() != uni8Bit)?L"_utf8":L"";
		wsprintf(savePath, L"%s\\%s%03d%s.dump", outdir, fileprefix, i, unitext);

		bool res = MainFileManager.saveBuffer(docbuf->getID(), savePath);

		somethingsaved |= res;
	}

	return somethingsaved || !somedirty;
}

void Notepad_plus::drawTabbarColoursFromStylerArray()	{

	Style *stActText = getStyleFromName(TABBAR_ACTIVETEXT);
	if (stActText && stActText->_fgColor != -1)
		TabBarPlus::setColour(stActText->_fgColor, TabBarPlus::activeText);

	Style *stActfocusTop = getStyleFromName(TABBAR_ACTIVEFOCUSEDINDCATOR);
	if (stActfocusTop && stActfocusTop->_fgColor != -1)
		TabBarPlus::setColour(stActfocusTop->_fgColor, TabBarPlus::activeFocusedTop);

	Style *stActunfocusTop = getStyleFromName(TABBAR_ACTIVEUNFOCUSEDINDCATOR);
	if (stActunfocusTop && stActunfocusTop->_fgColor != -1)
		TabBarPlus::setColour(stActunfocusTop->_fgColor, TabBarPlus::activeUnfocusedTop);

	Style *stInact = getStyleFromName(TABBAR_INACTIVETEXT);
	if (stInact && stInact->_fgColor != -1)
		TabBarPlus::setColour(stInact->_fgColor, TabBarPlus::inactiveText);
	if (stInact && stInact->_bgColor != -1)
		TabBarPlus::setColour(stInact->_bgColor, TabBarPlus::inactiveBg);
}

void Notepad_plus::prepareBufferChangedDialog(Buffer * buffer)	{

	// immediately show window if it was minimized before
	if (::IsIconic(_pPublicInterface->getHSelf()))
		::ShowWindow(_pPublicInterface->getHSelf(), SW_RESTORE);

	// switch to the file that changed
	int index = _pDocTab->getIndexByBuffer(buffer->getID());
	int iView = currentView();
	if (index == -1)
		iView = otherView();
	activateBuffer(buffer->getID(), iView);	//activate the buffer in the first view possible

	// prevent flickering issue by "manually" clicking and activating the _pEditView
	// (mouse events seem to get lost / improperly handled when showing the dialog)
	auto curPos = _pEditView->f(SCI_GETCURRENTPOS);
	::PostMessage(_pEditView->getHSelf(), WM_LBUTTONDOWN, 0, 0);
	::PostMessage(_pEditView->getHSelf(), WM_LBUTTONUP, 0, 0);
	::PostMessage(_pEditView->getHSelf(), SCI_SETSEL, curPos, curPos);
}

void Notepad_plus::notifyBufferChanged(Buffer * buffer, int mask)	{

	// To avoid to crash while MS-DOS style is set as default language,
	// Checking the validity of current instance is necessary.
	if (!this) return;

	_mainEditView.bufferUpdated(buffer, mask);
	_subEditView.bufferUpdated(buffer, mask);
	_mainDocTab.bufferUpdated(buffer, mask);
	_subDocTab.bufferUpdated(buffer, mask);

	bool mainActive = (_mainEditView.getCurrentBuffer() == buffer);
	bool subActive = (_subEditView.getCurrentBuffer() == buffer);

	//Only event that applies to non-active Buffers
	if (mask & BufferChangeStatus)	{
	//reload etc
		switch(buffer->getStatus())	{

			case DOC_UNNAMED: 	//nothing todo
			case DOC_REGULAR:	{ 	//nothing todo

				break;
			}
			case DOC_MODIFIED:	{	//ask for reloading

				// Since it is being monitored DOC_NEEDRELOAD is going to handle the change.
				if (buffer->isMonitoringOn())
					break;

				bool autoUpdate = (nGUI._fileAutoDetection & cdAutoUpdate) ? true : false;
				if (!autoUpdate || buffer->isDirty())	{

					prepareBufferChangedDialog(buffer);

					// Then we ask user to update
					if (doReloadOrNot(buffer->getFullPathName(), buffer->isDirty()) != IDYES)	{

						// Since the file content has changed but the user doesn't want to reload it, set state to dirty
						buffer->setDirty(true);

						break;	//abort
					}
				}
				// Set _isLoadedDirty false so when the document clean state is reached the icon will be set to blue
				buffer->setLoadedDirty(false);
				doReload(buffer->getID(), false);
				if (mainActive || subActive)	{

					performPostReload(mainActive?MAIN_VIEW:SUB_VIEW);
				}
				break;
			}
			case DOC_NEEDRELOAD:	{ // by log monitoring

				doReload(buffer->getID(), false);

				// not only test main view
				if (buffer == _mainEditView.getCurrentBuffer())	{

					_mainEditView.f(SCI_GOTOLINE, _mainEditView.f(SCI_GETLINECOUNT) - 1);
				}
				// but also test sub-view, because the buffer could be clonned
				if (buffer == _subEditView.getCurrentBuffer())	{

					_subEditView.f(SCI_GOTOLINE, _subEditView.f(SCI_GETLINECOUNT) - 1);
				}

				break;
			}
			case DOC_DELETED:	{ 	//ask for keep

				prepareBufferChangedDialog(buffer);

				SCNotification scnN;
				scnN.nmhdr.code = NPPN_FILEDELETED;
				scnN.nmhdr.hwndFrom = _pPublicInterface->getHSelf();
				scnN.nmhdr.idFrom = (uptr_t)buffer->getID();
				_pluginsManager.notify(&scnN);

				int doCloseDoc = doCloseOrNot(buffer->getFullPathName()) == IDNO;
				if (doCloseDoc)	{

					//close in both views, doing current view last since that has to remain opened
					bool isSnapshotMode = nGUI.isSnapshotMode();
					doClose(buffer->getID(), otherView(), isSnapshotMode);
					doClose(buffer->getID(), currentView(), isSnapshotMode);
					return;
				}
				break;
			}
		}
	}

	if (mask & (BufferChangeReadonly))	{

		checkDocState();

		bool isSysReadOnly = buffer->getFileReadOnly();
		bool isUserReadOnly = buffer->getUserReadOnly();
		bool isDirty = buffer->isDirty();

		// To notify plugins ro status is changed
		SCNotification scnN;
		scnN.nmhdr.hwndFrom = (void *)buffer->getID();
		scnN.nmhdr.idFrom = (uptr_t)  ((isSysReadOnly || isUserReadOnly? DOCSTATUS_READONLY : 0) | (isDirty ? DOCSTATUS_BUFFERDIRTY : 0));
		scnN.nmhdr.code = NPPN_READONLYCHANGED;
		_pluginsManager.notify(&scnN);

	}

	if (_pFileSwitcherPanel)
		_pFileSwitcherPanel->setItemIconStatus(buffer);

	if (!mainActive && !subActive)	{

		return;
	}

	if (mask & (BufferChangeLanguage))	{

		if (mainActive)
			_autoCompleteMain.setLanguage(buffer->getLangType());
		if (subActive)
			_autoCompleteSub.setLanguage(buffer->getLangType());
	}

	if ((currentView() == MAIN_VIEW) && !mainActive)
		return;

	if ((currentView() == SUB_VIEW) && !subActive)
		return;

	if (mask & (BufferChangeDirty|BufferChangeFilename))	{

		if (mask & BufferChangeFilename)
			command(IDM_VIEW_REFRESHTABAR);
		checkDocState();
		setTitle();
		generic_string dir(buffer->getFullPathName());
		PathRemoveFileSpec(dir);
		if (nppGUI._openSaveDir != dir_last)
			setDefOpenSaveDir(dir.c_str());
	}

	if (mask & (BufferChangeLanguage))	{

		checkLangsMenu(-1);	//let N++ do search for the item
		setLangStatus(buffer->getLangType());
		if (_mainEditView.getCurrentBuffer() == buffer)
			_autoCompleteMain.setLanguage(buffer->getLangType());
		else if (_subEditView.getCurrentBuffer() == buffer)
			_autoCompleteSub.setLanguage(buffer->getLangType());

		SCNotification scnN;
		scnN.nmhdr.code = NPPN_LANGCHANGED;
		scnN.nmhdr.hwndFrom = _pPublicInterface->getHSelf();
		scnN.nmhdr.idFrom = (uptr_t)_pEditView->getCurrentBufferID();
		_pluginsManager.notify(&scnN);
	}

	if (mask & (BufferChangeFormat|BufferChangeLanguage|BufferChangeUnicode))	{

		updateStatusBar(1);
		checkUnicodeMenuItems(/*buffer->getUnicodeMode()*/);
		setUniModeText();
		setDisplayFormat(buffer->getEolFormat());
		enableConvertMenuItems(buffer->getEolFormat());
	}
}

void Notepad_plus::notifyBufferActivated(BufferID bufid, int view)	{

	Buffer * buf = MainFileManager.getBufferByID(bufid);
	buf->increaseRecentTag();

	if (view == MAIN_VIEW)	{

		_autoCompleteMain.setLanguage(buf->getLangType());
	}
	else if (view == SUB_VIEW)	{

		_autoCompleteSub.setLanguage(buf->getLangType());
	}

	if (view != currentView())
		return;	//dont care if another view did something


	checkDocState();
	dynamicCheckMenuAndTB();
	setLangStatus(buf->getLangType());
	updateStatusBar(1);
	checkUnicodeMenuItems(/*buf->getUnicodeMode()*/);
	setUniModeText();
	setDisplayFormat(buf->getEolFormat());
	enableConvertMenuItems(buf->getEolFormat());
	generic_string dir(buf->getFullPathName());
	PathRemoveFileSpec(dir);
	if (nppGUI._openSaveDir != dir_last)
		setDefOpenSaveDir(dir.c_str());
	setTitle();
	//Make sure the colors of the tab controls match
	::InvalidateRect(_mainDocTab.getHSelf(), NULL, FALSE);
	::InvalidateRect(_subDocTab.getHSelf(), NULL, FALSE);

	SCNotification scnN;
	scnN.nmhdr.code = NPPN_BUFFERACTIVATED;
	scnN.nmhdr.hwndFrom = _pPublicInterface->getHSelf();
	scnN.nmhdr.idFrom = (uptr_t)bufid;
	_pluginsManager.notify(&scnN);

	if (_pFileSwitcherPanel)	{

		_pFileSwitcherPanel->activateItem(bufid, currentView());
	}

	if (_pDocMap && (!_pDocMap->isClosed()) && _pDocMap->isVisible())	{

		_pDocMap->reloadMap();
		_pDocMap->setSyntaxHiliting();
	}

	if (_pFuncList && (!_pFuncList->isClosed()) && _pFuncList->isVisible())	{

		_pFuncList->reload();
	}

	_linkTriggered = true;
}

std::vector<generic_string> Notepad_plus::loadCommandlineParams(const TCHAR * commandLine, const CmdLineParamsDTO * pCmdParams)	{

	if (!commandLine || ! pCmdParams)
		return std::vector<generic_string>();

		FileNameStringSplitter fnss(commandLine);

	// loading file as session file is allowed only when there is only one file
	if (pCmdParams->_isSessionFile && fnss.size() == 1)	{

		Session session2Load;
		if ((param).loadSession(session2Load, fnss.getFileName(0)))	{

			loadSession(session2Load);
		}
		return std::vector<generic_string>();
	}

 	LangType lt = pCmdParams->_langType;
	int ln =  pCmdParams->_line2go;
	int cn = pCmdParams->_column2go;
	int cpos = pCmdParams->_pos2go;
	bool recursive = pCmdParams->_isRecursive;
	bool readOnly = pCmdParams->_isReadOnly;
	bool openFoldersAsWorkspace = pCmdParams->_openFoldersAsWorkspace;

	if (openFoldersAsWorkspace)	{

		// All the filepath in argument will be used as folder in workspace
		// call launchFileBrowser later with fnss
		return fnss.getFileNames();
	}

	BufferID lastOpened = BUFFER_INVALID;
	for (int i = 0, len = fnss.size(); i < len ; ++i)	{

		const TCHAR *pFn = fnss.getFileName(i);
		if (!pFn) return std::vector<generic_string>();

		BufferID bufID = doOpen(pFn, recursive, readOnly);
		if (bufID == BUFFER_INVALID)	//cannot open file
			continue;

		lastOpened = bufID;

		if (lt != L_EXTERNAL && lt < param.L_END)	{

			Buffer * pBuf = MainFileManager.getBufferByID(bufID);
			pBuf->setLangType(lt);
		}

		if (ln != -1 || cpos != -1)	{
	//we have to move the cursor manually
			int iView = currentView();	//store view since fileswitch can cause it to change
			switchToFile(bufID);	//switch to the file. No deferred loading, but this way we can easily move the cursor to the right position

			if (cpos != -1)	{

				_pEditView->f(SCI_GOTOPOS, cpos);
			}
				else
			if (cn == -1)	{

				_pEditView->f(SCI_GOTOLINE, ln-1);
			}
				else	{

					auto pos = _pEditView->f(SCI_FINDCOLUMN, ln-1, cn-1);
					_pEditView->f(SCI_GOTOPOS, pos);
				}

			_pEditView->scrollPosToCenter(_pEditView->f(SCI_GETCURRENTPOS));

			switchEditViewTo(iView);	//restore view
		}
	}
	if (lastOpened != BUFFER_INVALID)	{

		switchToFile(lastOpened);
	}

	return fnss.getFileNames();
}


void Notepad_plus::setFindReplaceFolderFilter(const TCHAR *dir, const TCHAR *filter)	{

	generic_string fltr;
		FindHistory & findHistory = param.getFindHistory();

	// get current directory in case it's not provided.
	if (!dir && findHistory._isFolderFollowDoc)	{

		dir = param.getWorkingDir();
	}

	// get current language file extensions in case it's not provided.
	if (!filter && findHistory._isFilterFollowDoc)	{

		// Get current language file extensions
		const TCHAR *ext = NULL;
		LangType lt = _pEditView->getCurrentBuffer()->getLangType();

		if (lt == L_USER)	{

			Buffer * buf = _pEditView->getCurrentBuffer();
			UserLangContainer * userLangContainer = param.getULCFromName(buf->getUserDefineLangName());
			if (userLangContainer)
				ext = userLangContainer->getExtention();
		}
		else	{

			ext = param.getLangExtFromLangType(lt);
		}

		if (ext && ext[0])	{

			fltr = L"";
			vector<generic_string> vStr;
			cutString(ext, vStr);
			for (size_t i = 0 ,len = vStr.size(); i < len; ++i)	{

				fltr += L"*.";
				fltr += vStr[i] + L" ";
			}
		}
		else	{

			fltr = L"*.*";
		}
		filter = fltr.c_str();
	}
	_findReplaceDlg.setFindInFilesDirFilter(dir, filter);
}

vector<generic_string> Notepad_plus::addNppComponents(const TCHAR *destDir, const TCHAR *extFilterName, const TCHAR *extFilter)
{
	FileDialog fDlg(_pPublicInterface->getHSelf(), _pPublicInterface->getHinst());
	fDlg.setExtFilter(extFilterName, extFilter, NULL);

	vector<generic_string> copiedFiles;

	if (stringVector *pfns = fDlg.doOpenMultiFilesDlg())	{

		// Get plugins dir
		generic_string destDirName = param.getNppPath();
		PathAppend(destDirName, destDir);

		if (!::PathFileExists(destDirName.c_str()))	{

				::CreateDirectory(destDirName.c_str(), NULL);
		}

		destDirName += L"\\";

		size_t sz = pfns->size();
		for (size_t i = 0 ; i < sz ; ++i)	{

				if (::PathFileExists(pfns->at(i).c_str()))	{

					// copy to plugins directory
					generic_string destName = destDirName;
					destName += ::PathFindFileName(pfns->at(i).c_str());
					if (::CopyFile(pfns->at(i).c_str(), destName.c_str(), FALSE))
						copiedFiles.push_back(destName.c_str());
				}
		}
	}
	return copiedFiles;
}

vector<generic_string> Notepad_plus::addNppPlugins(const TCHAR *extFilterName, const TCHAR *extFilter)
{
	FileDialog fDlg(_pPublicInterface->getHSelf(), _pPublicInterface->getHinst());
	fDlg.setExtFilter(extFilterName, extFilter, NULL);

	vector<generic_string> copiedFiles;

	if (stringVector *pfns = fDlg.doOpenMultiFilesDlg())	{

		// Get plugins dir
		generic_string destDirName = param.getPluginRootDir();

		if (!::PathFileExists(destDirName.c_str()))	{

				::CreateDirectory(destDirName.c_str(), NULL);
		}

		size_t sz = pfns->size();
		for (size_t i = 0 ; i < sz ; ++i)	{

				if (::PathFileExists(pfns->at(i).c_str()))	{

					// copy to plugins directory
					generic_string destName = destDirName;
				
				generic_string nameExt = ::PathFindFileName(pfns->at(i).c_str());
				auto pos = nameExt.find_last_of(L".");
				if (pos == generic_string::npos)
					continue;

				generic_string name = nameExt.substr(0, pos);
				PathAppend(destName, name);
				if (!::PathFileExists(destName.c_str()))	{

					::CreateDirectory(destName.c_str(), NULL);
				}
				PathAppend(destName, nameExt);

					if (::CopyFile(pfns->at(i).c_str(), destName.c_str(), FALSE))
						copiedFiles.push_back(destName.c_str());
				}
		}
	}
	return copiedFiles;
}

void Notepad_plus::setDefOpenSaveDir(const TCHAR *dir)	{

	// NppParameters& param = param;
	// if (nGUI._openSaveDir == dir_last)
		// return;
	//else
	if (dir && PathIsDirectory(dir))
		param.setDefOpenSaveDir(dir);
}

int Notepad_plus::getLangFromMenuName(const TCHAR * langName)	{

	int	id	= 0;
	constexpr int menuSize = 64;
	TCHAR menuLangName[menuSize];

	for ( int i = IDM_LANG_C; i <= IDM_LANG_USER; ++i )
		if ( ::GetMenuString( _mainMenuHandle, i, menuLangName, menuSize, MF_BYCOMMAND ) )
			if ( !lstrcmp( langName, menuLangName ) )	{

				id	= i;
				break;
			}

	if (!id )	{

		for ( int i = IDM_LANG_USER + 1; i <= IDM_LANG_USER_LIMIT; ++i )
			if ( ::GetMenuString( _mainMenuHandle, i, menuLangName, menuSize, MF_BYCOMMAND ) )
				if ( !lstrcmp( langName, menuLangName ) )	{

					id	= i;
					break;
				}
	}

	return id;
}

generic_string Notepad_plus::getLangFromMenu(const Buffer * buf)	{


	int	id;
	generic_string userLangName;
	constexpr int nbChar = 32;
	TCHAR menuLangName[nbChar];

	id = param.langTypeToCommandID( buf->getLangType() );
	if ( ( id != IDM_LANG_USER ) || !( buf->isUserDefineLangExt() ) )	{

		::GetMenuString(_mainMenuHandle, id, menuLangName, nbChar-1, MF_BYCOMMAND);
		userLangName = menuLangName;
	}
	else	{

		userLangName = buf->getUserDefineLangName();
	}
	return	userLangName;
}

Style * Notepad_plus::getStyleFromName(const TCHAR *styleName)	{
	bool is2Be;
	Style& style = param.getMiscStylerArray().styleOf(styleName, is2Be);
	return is2Be ? &style : nullptr;
}

bool Notepad_plus::noOpenedDoc() const
{
	if (_mainDocTab.isVisible() && _subDocTab.isVisible())
		return false;
	if (_pDocTab->nbItem() == 1)	{

		BufferID buffer = _pDocTab->getBufferByIndex(0);
		Buffer * buf = MainFileManager.getBufferByID(buffer);
		if (!buf->isDirty() && buf->isUntitled())
			return true;
	}
	return false;
}

bool Notepad_plus::reloadLang()	{

	
	if (!param.reloadLang())	{

		return false;
	}

	TiXmlDocumentA *nativeLangDocRootA = param.getNativeLangA();
	if (!nativeLangDocRootA)	{

		return false;
	}

	_nativeLangSpeaker.init(nativeLangDocRootA, true);

	param.reloadContextMenuFromXmlTree(_mainMenuHandle, _pluginsManager.getMenuHandle());

	generic_string pluginsTrans, windowTrans;
	_nativeLangSpeaker.changeMenuLang(_mainMenuHandle, pluginsTrans, windowTrans);
	::DrawMenuBar(_pPublicInterface->getHSelf());

	int indexWindow = ::GetMenuItemCount(_mainMenuHandle) - 3;

	if (pluginsTrans != L"")	{

		::ModifyMenu(_mainMenuHandle, indexWindow - 1, MF_BYPOSITION, 0, pluginsTrans.c_str());
	}

	if (windowTrans != L"")	{

		::ModifyMenu(_mainMenuHandle, indexWindow, MF_BYPOSITION, 0, windowTrans.c_str());
		windowTrans += L"...";
		::ModifyMenu(_mainMenuHandle, IDM_WINDOW_WINDOWS, MF_BYCOMMAND, IDM_WINDOW_WINDOWS, windowTrans.c_str());
	}
	// Update scintilla context menu strings
	vector<MenuItemUnit> & tmp = param.getContextMenuItems();
	size_t len = tmp.size();
	TCHAR menuName[64];
	for (size_t i = 0 ; i < len ; ++i)	{

		if (tmp[i]._itemName == L"")	{

			::GetMenuString(_mainMenuHandle, tmp[i]._cmdID, menuName, 64, MF_BYCOMMAND);
			tmp[i]._itemName = purgeMenuItemString(menuName);
		}
	}

	vector<CommandShortcut> & shortcuts = param.getUserShortcuts();
	len = shortcuts.size();

	for (size_t i = 0; i < len; ++i)	{

		CommandShortcut & csc = shortcuts[i];
		// If menu item is not present (e.g. "Restore recent close file" might not present initially)
		// then fill the localized string directly
		if (::GetMenuString(_mainMenuHandle, csc.getID(), menuName, 64, MF_BYCOMMAND))	{

			csc.setName(purgeMenuItemString(menuName, true).c_str());
		}
		else	{

			generic_string localizedMenuName = _nativeLangSpeaker.getNativeLangMenuString(csc.getID());
			if (!localizedMenuName.empty())
				csc.setName(purgeMenuItemString(localizedMenuName.c_str(), true).c_str());
		}
	}
	_accelerator.updateFullMenu();

	_scintaccelerator.updateKeys();


	if (_tabPopupMenu.isCreated())	{

		_nativeLangSpeaker.changeLangTabContextMenu(_tabPopupMenu.getMenuHandle());
	}
	if (_tabPopupDropMenu.isCreated())	{

		_nativeLangSpeaker.changeLangTabDrapContextMenu(_tabPopupDropMenu.getMenuHandle());
	}
	if (_fileSwitcherMultiFilePopupMenu.isCreated())	{

		//_nativeLangSpeaker.changeLangTabDrapContextMenu(_fileSwitcherMultiFilePopupMenu.getMenuHandle());
	}
	if (_preference.isCreated())	{

		_nativeLangSpeaker.changePrefereceDlgLang(_preference);
	}

	if (_configStyleDlg.isCreated())	{

		_nativeLangSpeaker.changeConfigLang(_configStyleDlg.getHSelf());
	}

	if (_findReplaceDlg.isCreated())	{

		_nativeLangSpeaker.changeFindReplaceDlgLang(_findReplaceDlg);
	}

	if (_goToLineDlg.isCreated())	{

		_nativeLangSpeaker.changeDlgLang(_goToLineDlg.getHSelf(), "GoToLine");
	}

	if (_runDlg.isCreated())	{

		_nativeLangSpeaker.changeDlgLang(_runDlg.getHSelf(), "Run");
	}

	if (_md5FromFilesDlg.isCreated())	{

		_nativeLangSpeaker.changeDlgLang(_md5FromFilesDlg.getHSelf(), "MD5FromFilesDlg");
	}

	if (_md5FromTextDlg.isCreated())	{

		_nativeLangSpeaker.changeDlgLang(_md5FromTextDlg.getHSelf(), "MD5FromTextDlg");
	}

	if (_sha2FromFilesDlg.isCreated())	{

		_nativeLangSpeaker.changeDlgLang(_sha2FromFilesDlg.getHSelf(), "SHA256FromFilesDlg");
	}

	if (_sha2FromTextDlg.isCreated())	{

		_nativeLangSpeaker.changeDlgLang(_sha2FromTextDlg.getHSelf(), "SHA256FromTextDlg");
	}

	if (_runMacroDlg.isCreated())	{

		_nativeLangSpeaker.changeDlgLang(_runMacroDlg.getHSelf(), "MultiMacro");
	}

	if (_findCharsInRangeDlg.isCreated())	{

		_nativeLangSpeaker.changeDlgLang(_findCharsInRangeDlg.getHSelf(), "FindCharsInRange");
	}

	if (_colEditorDlg.isCreated())	{

		_nativeLangSpeaker.changeDlgLang(_colEditorDlg.getHSelf(), "ColumnEditor");
	}

	if (_pluginsAdminDlg.isCreated())	{

		_nativeLangSpeaker.changePluginsAdminDlgLang(_pluginsAdminDlg);
	}

	UserDefineDialog *udd = _pEditView->getUserDefineDlg();
	if (udd->isCreated())	{

		_nativeLangSpeaker.changeUserDefineLang(udd);
	}

	_lastRecentFileList.setLangEncoding(_nativeLangSpeaker.getLangEncoding());
	return true;
}


void Notepad_plus::launchClipboardHistoryPanel()	{

	if (!_pClipboardHistoryPanel)	{

		_pClipboardHistoryPanel = new ClipboardHistoryPanel();

		_pClipboardHistoryPanel->init(_pPublicInterface->getHinst(), _pPublicInterface->getHSelf(), &_pEditView);

		tTbData	data = {0};
		_pClipboardHistoryPanel->create(&data);

		::SendMessage(_pPublicInterface->getHSelf(), NPPM_MODELESSDIALOG, MODELESSDIALOGREMOVE, reinterpret_cast<LPARAM>(_pClipboardHistoryPanel->getHSelf()));
		// define the default docking behaviour
		data.uMask = DWS_DF_CONT_RIGHT | DWS_ICONTAB;
		data.hIconTab = (HICON)::LoadImage(_pPublicInterface->getHinst(), MAKEINTRESOURCE(IDR_CLIPBOARDPANEL_ICO), IMAGE_ICON, 14, 14, LR_LOADMAP3DCOLORS | LR_LOADTRANSPARENT);
		data.pszModuleName = NPP_INTERNAL_FUCTION_STR;

		// the dlgDlg should be the index of funcItem where the current function pointer is
		// in this case is DOCKABLE_DEMO_INDEX
		// In the case of Notepad++ internal function, it'll be the command ID which triggers this dialog
		data.dlgID = IDM_EDIT_CLIPBOARDHISTORY_PANEL;
		NativeLangSpeaker *pNativeSpeaker = param.getNativeLangSpeaker();
		generic_string title_temp = pNativeSpeaker->getAttrNameStr(CH_PROJECTPANELTITLE, "ClipboardHistory", "PanelTitle");
		static TCHAR title[32];
		if (title_temp.length() < 32)	{

			wcscpy_s(title, title_temp.c_str());
			data.pszName = title;
		}
		::SendMessage(_pPublicInterface->getHSelf(), NPPM_DMMREGASDCKDLG, 0, reinterpret_cast<LPARAM>(&data));

		COLORREF fgColor = param.getCurrentDefaultFgColor();
		COLORREF bgColor = param.getCurrentDefaultBgColor();

		_pClipboardHistoryPanel->setBackgroundColor(bgColor);
		_pClipboardHistoryPanel->setForegroundColor(fgColor);
	}

	_pClipboardHistoryPanel->display();
}


void Notepad_plus::launchFileSwitcherPanel()	{

	if (!_pFileSwitcherPanel)	{

		_pFileSwitcherPanel = new VerticalFileSwitcher;
		HIMAGELIST hImgLst = _docTabIconList.getHandle();
		_pFileSwitcherPanel->init(_pPublicInterface->getHinst(), _pPublicInterface->getHSelf(), hImgLst);

		tTbData	data = {0};
		_pFileSwitcherPanel->create(&data);

		::SendMessage(_pPublicInterface->getHSelf(), NPPM_MODELESSDIALOG, MODELESSDIALOGREMOVE, reinterpret_cast<LPARAM>(_pFileSwitcherPanel->getHSelf()));
		// define the default docking behaviour
		data.uMask = DWS_DF_CONT_LEFT | DWS_ICONTAB;
		data.hIconTab = (HICON)::LoadImage(_pPublicInterface->getHinst(), MAKEINTRESOURCE(IDR_DOCSWITCHER_ICO), IMAGE_ICON, 14, 14, LR_LOADMAP3DCOLORS | LR_LOADTRANSPARENT);
		data.pszModuleName = NPP_INTERNAL_FUCTION_STR;

		// the dlgDlg should be the index of funcItem where the current function pointer is
		// in this case is DOCKABLE_DEMO_INDEX
		// In the case of Notepad++ internal function, it'll be the command ID which triggers this dialog
		data.dlgID = IDM_VIEW_FILESWITCHER_PANEL;

		NativeLangSpeaker *pNativeSpeaker = param.getNativeLangSpeaker();
		generic_string title_temp = pNativeSpeaker->getAttrNameStr(FS_PROJECTPANELTITLE, "DocSwitcher", "PanelTitle");
		static TCHAR title[32];
		if (title_temp.length() < 32)	{

			wcscpy_s(title, title_temp.c_str());
			data.pszName = title;
		}
		::SendMessage(_pPublicInterface->getHSelf(), NPPM_DMMREGASDCKDLG, 0, reinterpret_cast<LPARAM>(&data));

		COLORREF fgColor = param.getCurrentDefaultFgColor();
		COLORREF bgColor = param.getCurrentDefaultBgColor();

		_pFileSwitcherPanel->setBackgroundColor(bgColor);
		_pFileSwitcherPanel->setForegroundColor(fgColor);
	}
	_pFileSwitcherPanel->display();
}


void Notepad_plus::initAnsiCharPanel()	{

		_pAnsiCharPanel = new AnsiCharPanel();
		_pAnsiCharPanel->init(_pPublicInterface->getHinst(), _pPublicInterface->getHSelf(), &_pEditView);

		tTbData	data = {0};
		_pAnsiCharPanel->create(&data);

		::SendMessage(_pPublicInterface->getHSelf(), NPPM_MODELESSDIALOG, MODELESSDIALOGREMOVE, reinterpret_cast<LPARAM>(_pAnsiCharPanel->getHSelf()));
		// define the default docking behaviour
		data.uMask = DWS_DF_CONT_RIGHT | DWS_ICONTAB;
		data.hIconTab = (HICON)::LoadImage(_pPublicInterface->getHinst(), MAKEINTRESOURCE(IDR_ASCIIPANEL_ICO), IMAGE_ICON, 14, 14, LR_LOADMAP3DCOLORS | LR_LOADTRANSPARENT);
		data.pszModuleName = NPP_INTERNAL_FUCTION_STR;

		// the dlgDlg should be the index of funcItem where the current function pointer is
		// in this case is DOCKABLE_DEMO_INDEX
		// In the case of Notepad++ internal function, it'll be the command ID which triggers this dialog
		data.dlgID = IDM_VIEW_CHAR_PANEL;

		NativeLangSpeaker *pNativeSpeaker = param.getNativeLangSpeaker();
		generic_string title_temp = pNativeSpeaker->getAttrNameStr(AI_PROJECTPANELTITLE, "AsciiInsertion", "PanelTitle");
		static TCHAR title[85];
		if (title_temp.length() < 85)	{

			wcscpy_s(title, title_temp.c_str());
			data.pszName = title;
		}
		::SendMessage(_pPublicInterface->getHSelf(), NPPM_DMMREGASDCKDLG, 0, reinterpret_cast<LPARAM>(&data));

		_pAnsiCharPanel->setBackgroundColor(param.getCurrentDefaultBgColor());
		_pAnsiCharPanel->setForegroundColor(param.getCurrentDefaultFgColor());
}

void Notepad_plus::launchFileBrowser(const vector<generic_string> & folders, bool fromScratch)	{

	if (!_pFileBrowser)	{

		_pFileBrowser = new FileBrowser;
		_pFileBrowser->init(_pPublicInterface->getHinst(), _pPublicInterface->getHSelf());

		tTbData	data;
		memset(&data, 0, sizeof(data));
		_pFileBrowser->create(&data);
		data.pszName = L"ST";

		::SendMessage(_pPublicInterface->getHSelf(), NPPM_MODELESSDIALOG, MODELESSDIALOGREMOVE, reinterpret_cast<LPARAM>(_pFileBrowser->getHSelf()));
		// define the default docking behaviour
		data.uMask = DWS_DF_CONT_LEFT | DWS_ICONTAB;
		data.hIconTab = (HICON)::LoadImage(_pPublicInterface->getHinst(), MAKEINTRESOURCE(IDR_FILEBROWSER_ICO), IMAGE_ICON, 14, 14, LR_LOADMAP3DCOLORS | LR_LOADTRANSPARENT);
		data.pszModuleName = NPP_INTERNAL_FUCTION_STR;

		// the dlgDlg should be the index of funcItem where the current function pointer is
		// in this case is DOCKABLE_DEMO_INDEX
		// In the case of Notepad++ internal function, it'll be the command ID which triggers this dialog
		data.dlgID = IDM_VIEW_FILEBROWSER;

		NativeLangSpeaker *pNativeSpeaker = param.getNativeLangSpeaker();
		generic_string title_temp = pNativeSpeaker->getAttrNameStr(FB_PANELTITLE, "FolderAsWorkspace", "PanelTitle");

		static TCHAR title[32];
		if (title_temp.length() < 32)	{

			wcscpy_s(title, title_temp.c_str());
			data.pszName = title;
		}
		::SendMessage(_pPublicInterface->getHSelf(), NPPM_DMMREGASDCKDLG, 0, reinterpret_cast<LPARAM>(&data));

		COLORREF fgColor = param.getCurrentDefaultFgColor();
		COLORREF bgColor = param.getCurrentDefaultBgColor();

		_pFileBrowser->setBackgroundColor(bgColor);
		_pFileBrowser->setForegroundColor(fgColor);
	}

	if (fromScratch)	{

		_pFileBrowser->deleteAllFromTree();
	}

	for (size_t i = 0; i <folders.size(); ++i)	{

		_pFileBrowser->addRootFolder(folders[i]);
	}

	_pFileBrowser->display();

	checkMenuItem(IDM_VIEW_FILEBROWSER, true);
	_toolBar.setCheck(IDM_VIEW_FILEBROWSER, true);
	_pFileBrowser->setClosed(false);
}


void Notepad_plus::launchProjectPanel(int cmdID, ProjectPanel ** pProjPanel, int panelID)	{

	if (!(*pProjPanel))	{

		
		(*pProjPanel) = new ProjectPanel;
		(*pProjPanel)->init(_pPublicInterface->getHinst(), _pPublicInterface->getHSelf());
		(*pProjPanel)->setWorkSpaceFilePath(param.getWorkSpaceFilePath(panelID));

		tTbData	data;
		memset(&data, 0, sizeof(data));
		(*pProjPanel)->create(&data);
		data.pszName = L"ST";

		::SendMessage(_pPublicInterface->getHSelf(), NPPM_MODELESSDIALOG, MODELESSDIALOGREMOVE, reinterpret_cast<LPARAM>((*pProjPanel)->getHSelf()));
		// define the default docking behaviour
		data.uMask = DWS_DF_CONT_LEFT | DWS_ICONTAB;
		data.hIconTab = (HICON)::LoadImage(_pPublicInterface->getHinst(), MAKEINTRESOURCE(IDR_PROJECTPANEL_ICO), IMAGE_ICON, 14, 14, LR_LOADMAP3DCOLORS | LR_LOADTRANSPARENT);
		data.pszModuleName = NPP_INTERNAL_FUCTION_STR;

		// the dlgDlg should be the index of funcItem where the current function pointer is
		// in this case is DOCKABLE_DEMO_INDEX
		// In the case of Notepad++ internal function, it'll be the command ID which triggers this dialog
		data.dlgID = cmdID;

		NativeLangSpeaker *pNativeSpeaker = param.getNativeLangSpeaker();
		generic_string title_temp = pNativeSpeaker->getAttrNameStr(PM_PROJECTPANELTITLE, "ProjectManager", "PanelTitle");

		static TCHAR title[32];
		if (title_temp.length() < 32)	{

			wcscpy_s(title, title_temp.c_str());
			data.pszName = title;
		}
		::SendMessage(_pPublicInterface->getHSelf(), NPPM_DMMREGASDCKDLG, 0, reinterpret_cast<LPARAM>(&data));

		COLORREF fgColor = param.getCurrentDefaultFgColor();
		COLORREF bgColor = param.getCurrentDefaultBgColor();

		(*pProjPanel)->setBackgroundColor(bgColor);
		(*pProjPanel)->setForegroundColor(fgColor);
	}
	(*pProjPanel)->display();
}


void Notepad_plus::launchDocMap()	{

	if (!(param).isTransparentAvailable())	{

		_nativeLangSpeaker.messageBox("PrehistoricSystemDetected",
			_pPublicInterface->getHSelf(),
			L"It seems you still use a prehistoric system. This feature works only on a modern system, sorry.",
			L"Prehistoric system detected",
			MB_OK);

		return;
	}

	if (!_pDocMap)	{

		_pDocMap = new DocumentMap();
		_pDocMap->init(_pPublicInterface->getHinst(), _pPublicInterface->getHSelf(), &_pEditView);

		tTbData	data = {0};
		_pDocMap->create(&data);

		::SendMessage(_pPublicInterface->getHSelf(), NPPM_MODELESSDIALOG, MODELESSDIALOGREMOVE, reinterpret_cast<LPARAM>(_pDocMap->getHSelf()));
		// define the default docking behaviour
		data.uMask = DWS_DF_CONT_RIGHT | DWS_ICONTAB;
		data.hIconTab = (HICON)::LoadImage(_pPublicInterface->getHinst(), MAKEINTRESOURCE(IDR_DOCMAP_ICO), IMAGE_ICON, 14, 14, LR_LOADMAP3DCOLORS | LR_LOADTRANSPARENT);
		data.pszModuleName = NPP_INTERNAL_FUCTION_STR;

		// the dlgDlg should be the index of funcItem where the current function pointer is
		// in this case is DOCKABLE_DEMO_INDEX
		// In the case of Notepad++ internal function, it'll be the command ID which triggers this dialog
		data.dlgID = IDM_VIEW_DOC_MAP;

		NativeLangSpeaker *pNativeSpeaker = param.getNativeLangSpeaker();
		generic_string title_temp = pNativeSpeaker->getAttrNameStr(DM_PANELTITLE, "DocumentMap", "PanelTitle");
		static TCHAR title[32];
		if (title_temp.length() < 32)	{

			wcscpy_s(title, title_temp.c_str());
			data.pszName = title;
		}
		::SendMessage(_pPublicInterface->getHSelf(), NPPM_DMMREGASDCKDLG, 0, reinterpret_cast<LPARAM>(&data));
	}

	_pDocMap->initWrapMap();
	_pDocMap->wrapMap();
	_pDocMap->display();

	_pEditView->focus();
}


void Notepad_plus::launchFunctionList()	{

	if (!_pFuncList)	{

		_pFuncList = new FunctionListPanel();
		_pFuncList->init(_pPublicInterface->getHinst(), _pPublicInterface->getHSelf(), &_pEditView);

		tTbData	data = {0};
		_pFuncList->create(&data);

		::SendMessage(_pPublicInterface->getHSelf(), NPPM_MODELESSDIALOG, MODELESSDIALOGREMOVE, reinterpret_cast<LPARAM>(_pFuncList->getHSelf()));
		// define the default docking behaviour
		data.uMask = DWS_DF_CONT_RIGHT | DWS_ICONTAB;
		data.hIconTab = (HICON)::LoadImage(_pPublicInterface->getHinst(), MAKEINTRESOURCE(IDR_FUNC_LIST_ICO), IMAGE_ICON, 14, 14, LR_LOADMAP3DCOLORS | LR_LOADTRANSPARENT);
		data.pszModuleName = NPP_INTERNAL_FUCTION_STR;

		// the dlgDlg should be the index of funcItem where the current function pointer is
		// in this case is DOCKABLE_DEMO_INDEX
		// In the case of Notepad++ internal function, it'll be the command ID which triggers this dialog
		data.dlgID = IDM_VIEW_FUNC_LIST;

		NativeLangSpeaker *pNativeSpeaker = param.getNativeLangSpeaker();
		generic_string title_temp = pNativeSpeaker->getAttrNameStr(FL_PANELTITLE, "FunctionList", "PanelTitle");

		static TCHAR title[32];
		if (title_temp.length() < 32)	{

			wcscpy_s(title, title_temp.c_str());
			data.pszName = title;
		}

		::SendMessage(_pPublicInterface->getHSelf(), NPPM_DMMREGASDCKDLG, 0, reinterpret_cast<LPARAM>(&data));
		COLORREF fgColor = param.getCurrentDefaultFgColor();
		COLORREF bgColor = param.getCurrentDefaultBgColor();

		_pFuncList->setBackgroundColor(bgColor);
		_pFuncList->setForegroundColor(fgColor);
	}

	_pFuncList->display();
	_pFuncList->reload();

	_pEditView->focus();
}



void Notepad_plus::launchDocumentBackupTask()	{

	HANDLE hThread = ::CreateThread(NULL, 0, backupDocument, NULL, 0, NULL);
	if (hThread)
		::CloseHandle(hThread);
}


DWORD WINAPI Notepad_plus::backupDocument(void * /*param*/)	{

	bool isSnapshotMode = true;
	while (isSnapshotMode)	{

		
		size_t timer = nppGUI._snapshotBackupTiming;
		if (timer < 1000)
			timer = 1000;

		::Sleep(DWORD(timer));

		isSnapshotMode = nppGUI.isSnapshotMode();
		if (!isSnapshotMode)
			break;

		::PostMessage(Notepad_plus_Window::gNppHWND, NPPM_INTERNAL_SAVEBACKUP, 0, 0);
	}
	return TRUE;
}



#pragma warning( disable : 4127 )
//-- undoStreamComment: New function to undo stream comment around or within selection end-points.
bool Notepad_plus::undoStreamComment(bool tryBlockComment)	{

	const TCHAR *commentStart;
	const TCHAR *commentEnd;
	const TCHAR *commentLineSymbol;

	generic_string symbolStart;
	generic_string symbolEnd;
	generic_string symbol;

	constexpr int charbufLen = 10;
	TCHAR charbuf[charbufLen];

	bool retVal = false;

	Buffer * buf = _pEditView->getCurrentBuffer();
	//-- Avoid side-effects (e.g. cursor moves number of comment-characters) when file is read-only.
	if (buf->isReadOnly())
		return false;
	if (buf->getLangType() == L_USER)	{

		UserLangContainer * userLangContainer = param.getULCFromName(buf->getUserDefineLangName());
		if (!userLangContainer)
			return false;

		symbol = extractSymbol('0', '0', userLangContainer->_keywordLists[SCE_USER_KWLIST_COMMENTS]);
		commentLineSymbol = symbol.c_str();
		symbolStart = extractSymbol('0', '3', userLangContainer->_keywordLists[SCE_USER_KWLIST_COMMENTS]);
		commentStart = symbolStart.c_str();
		symbolEnd = extractSymbol('0', '4', userLangContainer->_keywordLists[SCE_USER_KWLIST_COMMENTS]);
		commentEnd = symbolEnd.c_str();
	}
	else	{

		commentLineSymbol = buf->getCommentLineSymbol();
		commentStart = buf->getCommentStart();
		commentEnd = buf->getCommentEnd();
	}


	// BlockToStreamComment: If there is no stream-comment symbol and we came not from doBlockComment, try the block comment:
	if ((!commentStart) || (!commentStart[0]) || (!commentStart) || (!commentEnd) || (!commentEnd[0]) || (!commentEnd))	{

		if (!(!commentLineSymbol || !commentLineSymbol[0] ||!commentLineSymbol) && tryBlockComment)
			return doBlockComment(cm_uncomment);
		else
			return false;
	}

	generic_string start_comment(commentStart);
	generic_string end_comment(commentEnd);
	generic_string white_space(L" ");
	size_t start_comment_length = start_comment.length();
	size_t end_comment_length = end_comment.length();

	// do as long as stream-comments are within selection
	do
	{
		auto selectionStart = _pEditView->f(SCI_GETSELECTIONSTART);
		auto selectionEnd = _pEditView->f(SCI_GETSELECTIONEND);
		auto caretPosition = _pEditView->f(SCI_GETCURRENTPOS);
		auto docLength = _pEditView->f(SCI_GETLENGTH);

		// checking if caret is located in _beginning_ of selected block
		bool move_caret = caretPosition < selectionEnd;

		//-- Note: The caretPosition is either at selectionEnd or at selectionStart!! selectionStart is always before (smaller) than selectionEnd!!

		//-- First, search all start_comment and end_comment before and after the selectionStart and selectionEnd position.
		constexpr int iSelStart=0, iSelEnd=1;
		constexpr size_t N_CMNT = 2;
		int posStartCommentBefore[N_CMNT], posEndCommentBefore[N_CMNT], posStartCommentAfter[N_CMNT], posEndCommentAfter[N_CMNT];
		bool blnStartCommentBefore[N_CMNT], blnEndCommentBefore[N_CMNT], blnStartCommentAfter[N_CMNT], blnEndCommentAfter[N_CMNT];
		int posStartComment, posEndComment;
		int selectionStartMove, selectionEndMove;
		int flags;

		//-- Directly use Scintilla-Functions
		//   rather than _findReplaceDlg.processFindNext()which does not return the find-position and is not quiet!
		flags = SCFIND_WORDSTART;
		_pEditView->f(SCI_SETSEARCHFLAGS, flags);
		//-- Find all start- and end-comments before and after the selectionStart position.
		//-- When searching upwards the start-position for searching must be moved one after the current position
		//   to find a search-string just starting before the current position!
		//-- Direction DIR_UP ---
		posStartCommentBefore[iSelStart] = _pEditView->searchInTarget(start_comment.c_str(), start_comment_length, selectionStart, 0);
		(posStartCommentBefore[iSelStart] == -1 ? blnStartCommentBefore[iSelStart] = false : blnStartCommentBefore[iSelStart] = true);
		posEndCommentBefore[iSelStart] = _pEditView->searchInTarget(end_comment.c_str(), end_comment_length, selectionStart, 0);
		(posEndCommentBefore[iSelStart] == -1 ? blnEndCommentBefore[iSelStart] = false : blnEndCommentBefore[iSelStart] = true);
		//-- Direction DIR_DOWN ---
		posStartCommentAfter[iSelStart] = _pEditView->searchInTarget(start_comment.c_str(), start_comment_length, selectionStart, docLength);
		(posStartCommentAfter[iSelStart] == -1 ? blnStartCommentAfter[iSelStart] = false : blnStartCommentAfter[iSelStart] = true);
		posEndCommentAfter[iSelStart] = _pEditView->searchInTarget(end_comment.c_str(), end_comment_length, selectionStart, docLength);
		(posEndCommentAfter[iSelStart] == -1 ? blnEndCommentAfter[iSelStart] = false : blnEndCommentAfter[iSelStart] = true);

		//-- Check, if selectionStart or selectionEnd is within a stream comment -----
		//   or if the selection includes a complete stream-comment!! ----------------

		//-- First, check if there is a stream-comment around the selectionStart position:
		if ((blnStartCommentBefore[iSelStart] && blnEndCommentAfter[iSelStart])
			&& (!blnEndCommentBefore[iSelStart] || (posStartCommentBefore[iSelStart] >= posEndCommentBefore[iSelStart]))
			&& (!blnStartCommentAfter[iSelStart] || (posEndCommentAfter[iSelStart] <= posStartCommentAfter[iSelStart])))
		{
				posStartComment = posStartCommentBefore[iSelStart];
				posEndComment   = posEndCommentAfter[iSelStart];
		}
		else	{ //-- Second, check if there is a stream-comment around the selectionEnd position:

			//-- Find all start- and end-comments before and after the selectionEnd position.
			//-- Direction DIR_UP ---
			posStartCommentBefore[iSelEnd] = _pEditView->searchInTarget(start_comment.c_str(), start_comment_length, selectionEnd, 0);
			(posStartCommentBefore[iSelEnd] == -1 ? blnStartCommentBefore[iSelEnd] = false : blnStartCommentBefore[iSelEnd] = true);
			posEndCommentBefore[iSelEnd] = _pEditView->searchInTarget(end_comment.c_str(), end_comment_length, selectionEnd, 0);
			(posEndCommentBefore[iSelEnd] == -1 ? blnEndCommentBefore[iSelEnd] = false : blnEndCommentBefore[iSelEnd] = true);
			//-- Direction DIR_DOWN ---
			posStartCommentAfter[iSelEnd] = _pEditView->searchInTarget(start_comment.c_str(), start_comment_length, selectionEnd, docLength);
			(posStartCommentAfter[iSelEnd] == -1 ? blnStartCommentAfter[iSelEnd] = false : blnStartCommentAfter[iSelEnd] = true);
			posEndCommentAfter[iSelEnd] = _pEditView->searchInTarget(end_comment.c_str(), end_comment_length, selectionEnd, docLength);
			(posEndCommentAfter[iSelEnd] == -1 ? blnEndCommentAfter[iSelEnd] = false : blnEndCommentAfter[iSelEnd] = true);

			if ((blnStartCommentBefore[iSelEnd] && blnEndCommentAfter[iSelEnd])
				&& (!blnEndCommentBefore[iSelEnd] || (posStartCommentBefore[iSelEnd] >= posEndCommentBefore[iSelEnd]))
				&& (!blnStartCommentAfter[iSelEnd] || (posEndCommentAfter[iSelEnd] <= posStartCommentAfter[iSelEnd])))
			{
					posStartComment = posStartCommentBefore[iSelEnd];
					posEndComment   = posEndCommentAfter[iSelEnd];
			}
			//-- Third, check if there is a stream-comment within the selected area:
			else if ( (blnStartCommentAfter[iSelStart] && (posStartCommentAfter[iSelStart] < selectionEnd))
				&& (blnEndCommentBefore[iSelEnd] && (posEndCommentBefore[iSelEnd] >  selectionStart)))
			{
					//-- If there are more than one stream-comment within the selection, take the first one after selectionStart!!
					posStartComment = posStartCommentAfter[iSelStart];
					posEndComment   = posEndCommentAfter[iSelStart];
			}
			//-- Finally, if there is no stream-comment, return
			else
				return retVal;
		}

		//-- Ok, there are valid start-comment and valid end-comment around the caret-position.
		//   Now, un-comment stream-comment:
		retVal = true;
		int startCommentLength = int(start_comment_length);
		int endCommentLength = int(end_comment_length);

		//-- First delete end-comment, so that posStartCommentBefore does not change!
		//-- Get character before end-comment to decide, if there is a white character before the end-comment, which will be removed too!
		_pEditView->getGenericText(charbuf, charbufLen, posEndComment-1, posEndComment);
		if (!generic_strncmp(charbuf, white_space.c_str(), white_space.length()))	{

			endCommentLength +=1;
			posEndComment-=1;
		}
		//-- Delete end stream-comment string ---------
		_pEditView->f(SCI_BEGINUNDOACTION);
		_pEditView->f(SCI_SETSEL, posEndComment, posEndComment + endCommentLength);
		_pEditView->f(SCI_REPLACESEL, 0, reinterpret_cast<LPARAM>(""));

		//-- Get character after start-comment to decide, if there is a white character after the start-comment, which will be removed too!
		_pEditView->getGenericText(charbuf, charbufLen, posStartComment+startCommentLength, posStartComment+startCommentLength+1);
		if (!generic_strncmp(charbuf, white_space.c_str(), white_space.length()))
			startCommentLength +=1;

		//-- Delete starting stream-comment string ---------
		_pEditView->f(SCI_SETSEL, posStartComment, posStartComment + startCommentLength);
		_pEditView->f(SCI_REPLACESEL, 0, reinterpret_cast<LPARAM>(""));
		_pEditView->f(SCI_ENDUNDOACTION);

		//-- Reset selection before calling the routine
		//-- Determine selection movement
		//   selectionStart
		if (selectionStart > posStartComment)	{

			if (selectionStart >= posStartComment+startCommentLength)
				selectionStartMove = -static_cast<int>(startCommentLength);
			else
				selectionStartMove = -static_cast<int>(selectionStart - posStartComment);
		}
		else
			selectionStartMove = 0;

		//   selectionEnd
		if (selectionEnd >= posEndComment+endCommentLength)
			selectionEndMove = -static_cast<int>(startCommentLength+endCommentLength);
		else if (selectionEnd <= posEndComment)
			selectionEndMove = -static_cast<int>(startCommentLength);
		else
			selectionEndMove = -static_cast<int>(startCommentLength + (selectionEnd - posEndComment));

		//-- Reset selection of text without deleted stream-comment-string
		if (move_caret)	{

			// moving caret to the beginning of selected block
			_pEditView->f(SCI_GOTOPOS, selectionEnd+selectionEndMove);
			_pEditView->f(SCI_SETCURRENTPOS, selectionStart+selectionStartMove);
		}
		else	{

			_pEditView->f(SCI_SETSEL, selectionStart+selectionStartMove, selectionEnd+selectionEndMove);
		}
	}
	while (1); //do as long as stream-comments are within selection
}

void Notepad_plus::monitoringStartOrStopAndUpdateUI(Buffer* pBuf, bool isStarting)	{

	if (pBuf)	{

		if (isStarting)
			pBuf->startMonitoring();
		else
			pBuf->stopMonitoring();

		checkMenuItem(IDM_VIEW_MONITORING, isStarting);
		_toolBar.setCheck(IDM_VIEW_MONITORING, isStarting);
		pBuf->setUserReadOnly(isStarting);
	}
}
