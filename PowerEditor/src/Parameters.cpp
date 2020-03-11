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
//	installer, such as those produced by InstallShield.
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
#include <shlobj.h>
#include "Parameters.h"
#include "FileDialog.h"
#include "ScintillaEditView.h"
#include "keys.h"
#include "localization.h"
#include "localizationString.h"
#include "UserDefineDialog.h"

#pragma warning(disable : 4996) // for GetVersionEx()

using namespace std;


namespace // anonymous namespace
{


struct WinMenuKeyDefinition //more or less matches accelerator table definition, easy copy/paste
{
	//const TCHAR * name;	//name retrieved from menu?
	int vKey;
	int functionId;
	bool isCtrl;
	bool isAlt;
	bool isShift;
	const TCHAR * specialName;		//Used when no real menu name exists (in case of toggle for example)
};


struct ScintillaKeyDefinition
{
	const TCHAR * name;
	int functionId;
	bool isCtrl;
	bool isAlt;
	bool isShift;
	int vKey;
	int redirFunctionId;	//this gets set  when a function is being redirected through Notepad++ if Scintilla doesnt do it properly :)
};


/*!
** \brief array of accelerator keys for all std menu items
**
** values can be 0 for vKey, which means its unused
*/
static const WinMenuKeyDefinition winKeyDefs[] =
{
	// V_KEY,    COMMAND_ID,                                    Ctrl,  Alt,   Shift, cmdName
	// -------------------------------------------------------------------------------------
	//
	{ VK_N,       IDM_FILE_NEW,                                 true,  false, false, nullptr },
	{ VK_O,       IDM_FILE_OPEN,                                true,  false, false, nullptr },
	{ VK_NULL,    IDM_FILE_OPEN_FOLDER,                         false, false, false, nullptr },
	{ VK_NULL,    IDM_FILE_OPEN_CMD,                            false, false, false, nullptr },
	{ VK_NULL,    IDM_FILE_OPEN_DEFAULT_VIEWER,                 false, false, false, nullptr },
	{ VK_NULL,    IDM_FILE_OPENFOLDERASWORSPACE,                false, false, false, nullptr },
	{ VK_R,       IDM_FILE_RELOAD,                              true,  false, false, nullptr },
	{ VK_S,       IDM_FILE_SAVE,                                true,  false, false, nullptr },
	{ VK_S,       IDM_FILE_SAVEAS,                              true,  true,  false, nullptr },
	{ VK_NULL,    IDM_FILE_SAVECOPYAS,                          false, false, false, nullptr },
	{ VK_S,       IDM_FILE_SAVEALL,                             true,  false, true,  nullptr },
	{ VK_NULL,    IDM_FILE_RENAME,                              false, false, false, nullptr },
	{ VK_W,       IDM_FILE_CLOSE,                               true,  false, false, nullptr },
	{ VK_W,       IDM_FILE_CLOSEALL,                            true,  false, true,  nullptr },
	{ VK_NULL,    IDM_FILE_CLOSEALL_BUT_CURRENT,                false, false, false, nullptr },
	{ VK_NULL,    IDM_FILE_CLOSEALL_TOLEFT,                     false, false, false, nullptr },
	{ VK_NULL,    IDM_FILE_CLOSEALL_TORIGHT,                    false, false, false, nullptr },
	{ VK_NULL,    IDM_FILE_CLOSEALL_UNCHANGED,                  false, false, false, nullptr },
	{ VK_NULL,    IDM_FILE_DELETE,                              false, false, false, nullptr },
	{ VK_NULL,    IDM_FILE_LOADSESSION,                         false, false, false, nullptr },
	{ VK_NULL,    IDM_FILE_SAVESESSION,                         false, false, false, nullptr },
	{ VK_P,       IDM_FILE_PRINT,                               true,  false, false, nullptr },
	{ VK_NULL,    IDM_FILE_PRINTNOW,                            false, false, false, nullptr },
	{ VK_F4,      IDM_FILE_EXIT,                                false, true,  false, nullptr },
	{ VK_T,       IDM_FILE_RESTORELASTCLOSEDFILE,               true,  false, true,  L"Restore Recent Closed File"},

//	{ VK_NULL,    IDM_EDIT_UNDO,                                false, false, false, nullptr },
//	{ VK_NULL,    IDM_EDIT_REDO,                                false, false, false, nullptr },
//	{ VK_NULL,    IDM_EDIT_CUT,                                 false, false, false, nullptr },
//	{ VK_NULL,    IDM_EDIT_COPY,                                false, false, false, nullptr },
//	{ VK_NULL,    IDM_EDIT_PASTE,                               false, false, false, nullptr },
//	{ VK_NULL,    IDM_EDIT_DELETE,                              false, false, false, nullptr },
//	{ VK_NULL,    IDM_EDIT_SELECTALL,                           false, false, false, nullptr },
	{ VK_NULL,    IDM_EDIT_BEGINENDSELECT,                      false, false, false, nullptr },

	{ VK_NULL,    IDM_EDIT_FULLPATHTOCLIP,                      false, false, false, nullptr },
	{ VK_NULL,    IDM_EDIT_FILENAMETOCLIP,                      false, false, false, nullptr },
	{ VK_NULL,    IDM_EDIT_CURRENTDIRTOCLIP,                    false, false, false, nullptr },
//	{ VK_NULL,    IDM_EDIT_INS_TAB,                             false, false, false, nullptr },
//	{ VK_NULL,    IDM_EDIT_RMV_TAB,                             false, false, false, nullptr },
	{ VK_U,       IDM_EDIT_UPPERCASE,                           true,  false, true,  nullptr },
	{ VK_U,       IDM_EDIT_LOWERCASE,                           true,  false, false, nullptr },
	{ VK_U,       IDM_EDIT_PROPERCASE_FORCE,                    false, true,  false, nullptr },
	{ VK_U,       IDM_EDIT_PROPERCASE_BLEND,                    false, true,  true,  nullptr },
	{ VK_U,       IDM_EDIT_SENTENCECASE_FORCE,                  true,  true,  false, nullptr },
	{ VK_U,       IDM_EDIT_SENTENCECASE_BLEND,                  true,  true,  true,  nullptr },
	{ VK_NULL,    IDM_EDIT_INVERTCASE,                          false, false, false, nullptr },
	{ VK_NULL,    IDM_EDIT_RANDOMCASE,                          false, false, false, nullptr },
//	{ VK_NULL,    IDM_EDIT_DUP_LINE,                            false, false, false, nullptr },
	{ VK_I,       IDM_EDIT_SPLIT_LINES,                         true,  false, false, nullptr },
	{ VK_J,       IDM_EDIT_JOIN_LINES,                          true,  false, false, nullptr },
	{ VK_UP,      IDM_EDIT_LINE_UP,                             true,  false, true,  nullptr },
	{ VK_DOWN,    IDM_EDIT_LINE_DOWN,                           true,  false, true,  nullptr },
	{ VK_NULL,    IDM_EDIT_REMOVEEMPTYLINES,                    false, false, false, nullptr },
	{ VK_NULL,    IDM_EDIT_REMOVEEMPTYLINESWITHBLANK,           false, false, false, nullptr },
	{ VK_RETURN,  IDM_EDIT_BLANKLINEABOVECURRENT,               true,  true,  false, nullptr },
	{ VK_RETURN,  IDM_EDIT_BLANKLINEBELOWCURRENT,               true,  true,  true,  nullptr },
	{ VK_NULL,    IDM_EDIT_SORTLINES_LEXICOGRAPHIC_ASCENDING,   false, false, false, nullptr },
	{ VK_NULL,    IDM_EDIT_SORTLINES_LEXICOGRAPHIC_DESCENDING,  false, false, false, nullptr },
	{ VK_NULL,    IDM_EDIT_SORTLINES_INTEGER_ASCENDING,         false, false, false, nullptr },
	{ VK_NULL,    IDM_EDIT_SORTLINES_INTEGER_DESCENDING,        false, false, false, nullptr },
	{ VK_NULL,    IDM_EDIT_SORTLINES_DECIMALCOMMA_ASCENDING,    false, false, false, nullptr },
	{ VK_NULL,    IDM_EDIT_SORTLINES_DECIMALCOMMA_DESCENDING,   false, false, false, nullptr },
	{ VK_NULL,    IDM_EDIT_SORTLINES_DECIMALDOT_ASCENDING,      false, false, false, nullptr },
	{ VK_NULL,    IDM_EDIT_SORTLINES_DECIMALDOT_DESCENDING,     false, false, false, nullptr },
	{ VK_Q,       IDM_EDIT_BLOCK_COMMENT,                       true,  false, false, nullptr },
	{ VK_K,       IDM_EDIT_BLOCK_COMMENT_SET,                   true,  false, false, nullptr },
	{ VK_K,       IDM_EDIT_BLOCK_UNCOMMENT,                     true,  false, true,  nullptr },
	{ VK_Q,       IDM_EDIT_STREAM_COMMENT,                      true,  false, true,  nullptr },
	{ VK_NULL,    IDM_EDIT_STREAM_UNCOMMENT,                    false, false, false, nullptr },
	{ VK_SPACE,   IDM_EDIT_AUTOCOMPLETE,                        true,  false, false, nullptr },
	{ VK_SPACE,   IDM_EDIT_AUTOCOMPLETE_PATH,                   true,  true,  false, nullptr },
	{ VK_RETURN,  IDM_EDIT_AUTOCOMPLETE_CURRENTFILE,            true,  false, false, nullptr },
	{ VK_SPACE,   IDM_EDIT_FUNCCALLTIP,                         true,  false, true,  nullptr },
	{ VK_NULL,    IDM_FORMAT_TODOS,                             false, false, false, nullptr },
	{ VK_NULL,    IDM_FORMAT_TOUNIX,                            false, false, false, nullptr },
	{ VK_NULL,    IDM_FORMAT_TOMAC,                             false, false, false, nullptr },
	{ VK_NULL,    IDM_EDIT_TRIMTRAILING,                        false, false, false, nullptr },
	{ VK_NULL,    IDM_EDIT_TRIMLINEHEAD,                        false, false, false, nullptr },
	{ VK_NULL,    IDM_EDIT_TRIM_BOTH,                           false, false, false, nullptr },
	{ VK_NULL,    IDM_EDIT_EOL2WS,                              false, false, false, nullptr },
	{ VK_NULL,    IDM_EDIT_TRIMALL,                             false, false, false, nullptr },
	{ VK_NULL,    IDM_EDIT_TAB2SW,                              false, false, false, nullptr },
	{ VK_NULL,    IDM_EDIT_SW2TAB_ALL,                          false, false, false, nullptr },
	{ VK_NULL,    IDM_EDIT_SW2TAB_LEADING,                      false, false, false, nullptr },
	{ VK_NULL,    IDM_EDIT_PASTE_AS_HTML,                       false, false, false, nullptr },
	{ VK_NULL,    IDM_EDIT_PASTE_AS_RTF,                        false, false, false, nullptr },
	{ VK_NULL,    IDM_EDIT_COPY_BINARY,                         false, false, false, nullptr },
	{ VK_NULL,    IDM_EDIT_CUT_BINARY,                          false, false, false, nullptr },
	{ VK_NULL,    IDM_EDIT_PASTE_BINARY,                        false, false, false, nullptr },
	{ VK_NULL,    IDM_EDIT_OPENASFILE,                          false, false, false, nullptr },
	{ VK_NULL,    IDM_EDIT_OPENINFOLDER,                        false, false, false, nullptr },
	{ VK_NULL,    IDM_EDIT_SEARCHONINTERNET,                    false, false, false, nullptr },
	{ VK_NULL,    IDM_EDIT_CHANGESEARCHENGINE,                  false, false, false, nullptr },
//  { VK_NULL,    IDM_EDIT_COLUMNMODETIP,                       false, false, false, nullptr },
	{ VK_C,       IDM_EDIT_COLUMNMODE,                          false, true,  false, nullptr },
	{ VK_NULL,    IDM_EDIT_CHAR_PANEL,                          false, false, false, nullptr },
	{ VK_NULL,    IDM_EDIT_CLIPBOARDHISTORY_PANEL,              false, false, false, nullptr },
	{ VK_NULL,    IDM_EDIT_SETREADONLY,                         false, false, false, nullptr },
	{ VK_NULL,    IDM_EDIT_CLEARREADONLY,                       false, false, false, nullptr },
	// { VK_F,       IDM_SEARCH_FIND,                              true,  false, false, nullptr },
	{ VK_F,       IDM_SEARCH_FINDINFILES,                       true,  false, true,  nullptr },
	{ VK_F3,      IDM_SEARCH_FINDNEXT,                          false, false, false, nullptr },
	{ VK_F3,      IDM_SEARCH_FINDPREV,                          false, false, true,  nullptr },
	{ VK_F3,      IDM_SEARCH_SETANDFINDNEXT,                    true,  false, false, nullptr },
	{ VK_F3,      IDM_SEARCH_SETANDFINDPREV,                    true,  false, true,  nullptr },
	{ VK_F3,      IDM_SEARCH_VOLATILE_FINDNEXT,                 true,  true,  false, nullptr },
	{ VK_F3,      IDM_SEARCH_VOLATILE_FINDPREV,                 true,  true,  true,  nullptr },
	{ VK_H,       IDM_SEARCH_REPLACE,                           true,  false, false, nullptr },
	{ VK_I,       IDM_SEARCH_FINDINCREMENT,                     true,  true,  false, nullptr },
	{ VK_F7,      IDM_FOCUS_ON_FOUND_RESULTS,                   false, false, false, nullptr },
	{ VK_RETURN,      IDM_CLOSE_FOUND_RESULTS,                  false, true,  true, nullptr },
	{ VK_F4,      IDM_SEARCH_GOTOPREVFOUND,                     false, false, true,  nullptr },
	{ VK_F4,      IDM_SEARCH_GOTONEXTFOUND,                     false, false, false, nullptr },
	{ VK_G,       IDM_SEARCH_GOTOLINE,                          true,  false, false, nullptr },
	{ VK_B,       IDM_SEARCH_GOTOMATCHINGBRACE,                 true,  false, false, nullptr },
	{ VK_B,       IDM_SEARCH_SELECTMATCHINGBRACES,              true,  true,  false, nullptr },
	{ VK_NULL,    IDM_SEARCH_MARK,                              false, false, false, nullptr },
	{ VK_NULL,    IDM_SEARCH_MARKALLEXT1,                       false, false, false, nullptr },
	{ VK_NULL,    IDM_SEARCH_MARKALLEXT2,                       false, false, false, nullptr },
	{ VK_NULL,    IDM_SEARCH_MARKALLEXT3,                       false, false, false, nullptr },
	{ VK_NULL,    IDM_SEARCH_MARKALLEXT4,                       false, false, false, nullptr },
	{ VK_NULL,    IDM_SEARCH_MARKALLEXT5,                       false, false, false, nullptr },
	{ VK_NULL,    IDM_SEARCH_UNMARKALLEXT1,                     false, false, false, nullptr },
	{ VK_NULL,    IDM_SEARCH_UNMARKALLEXT2,                     false, false, false, nullptr },
	{ VK_NULL,    IDM_SEARCH_UNMARKALLEXT3,                     false, false, false, nullptr },
	{ VK_NULL,    IDM_SEARCH_UNMARKALLEXT4,                     false, false, false, nullptr },
	{ VK_NULL,    IDM_SEARCH_UNMARKALLEXT5,                     false, false, false, nullptr },
	{ VK_NULL,    IDM_SEARCH_CLEARALLMARKS,                     false, false, false, nullptr },
	{ VK_1,       IDM_SEARCH_GOPREVMARKER1,                     true,  false, true,  nullptr },
	{ VK_2,       IDM_SEARCH_GOPREVMARKER2,                     true,  false, true,  nullptr },
	{ VK_3,       IDM_SEARCH_GOPREVMARKER3,                     true,  false, true,  nullptr },
	{ VK_4,       IDM_SEARCH_GOPREVMARKER4,                     true,  false, true,  nullptr },
	{ VK_5,       IDM_SEARCH_GOPREVMARKER5,                     true,  false, true,  nullptr },
	{ VK_0,       IDM_SEARCH_GOPREVMARKER_DEF,                  true,  false, true,  nullptr },
	{ VK_1,       IDM_SEARCH_GONEXTMARKER1,                     true,  false, false, nullptr },
	{ VK_2,       IDM_SEARCH_GONEXTMARKER2,                     true,  false, false, nullptr },
	{ VK_3,       IDM_SEARCH_GONEXTMARKER3,                     true,  false, false, nullptr },
	{ VK_4,       IDM_SEARCH_GONEXTMARKER4,                     true,  false, false, nullptr },
	{ VK_5,       IDM_SEARCH_GONEXTMARKER5,                     true,  false, false, nullptr },
	{ VK_0,       IDM_SEARCH_GONEXTMARKER_DEF,                  true,  false, false, nullptr },
				 
	{ VK_F2,      IDM_SEARCH_TOGGLE_BOOKMARK,                   true,  false, false, nullptr },
	{ VK_F2,      IDM_SEARCH_NEXT_BOOKMARK,                     false, false, false, nullptr },
	{ VK_F2,      IDM_SEARCH_PREV_BOOKMARK,                     false, false, true, nullptr  },
	{ VK_NULL,    IDM_SEARCH_CLEAR_BOOKMARKS,                   false, false, false, nullptr },
	{ VK_NULL,    IDM_SEARCH_CUTMARKEDLINES,                    false, false, false, nullptr },
	{ VK_NULL,    IDM_SEARCH_COPYMARKEDLINES,                   false, false, false, nullptr },
	{ VK_NULL,    IDM_SEARCH_PASTEMARKEDLINES,                  false, false, false, nullptr },
	{ VK_NULL,    IDM_SEARCH_DELETEMARKEDLINES,                 false, false, false, nullptr },
	{ VK_NULL,    IDM_SEARCH_DELETEUNMARKEDLINES,               false, false, false, nullptr },
	{ VK_NULL,    IDM_SEARCH_INVERSEMARKS,                      false, false, false, nullptr },
	{ VK_NULL,    IDM_SEARCH_FINDCHARINRANGE,                   false, false, false, nullptr },
				 
	{ VK_NULL,    IDM_VIEW_ALWAYSONTOP,                         false, false, false, nullptr },
	{ VK_F11,     IDM_VIEW_FULLSCREENTOGGLE,                    false, false, false, nullptr },
	{ VK_F12,     IDM_VIEW_POSTIT,                              false, false, false, nullptr },

	{ VK_NULL,    IDM_VIEW_IN_FIREFOX,                          false, false, false, nullptr },
	{ VK_NULL,    IDM_VIEW_IN_CHROME,                           false, false, false, nullptr },
	{ VK_NULL,    IDM_VIEW_IN_IE,                               false, false, false, nullptr },
	{ VK_NULL,    IDM_VIEW_IN_EDGE,                             false, false, false, nullptr },

	{ VK_NULL,    IDM_VIEW_TAB_SPACE,                           false, false, false, nullptr },
	{ VK_NULL,    IDM_VIEW_EOL,                                 false, false, false, nullptr },
	{ VK_NULL,    IDM_VIEW_ALL_CHARACTERS,                      false, false, false, nullptr },
	{ VK_NULL,    IDM_VIEW_INDENT_GUIDE,                        false, false, false, nullptr },
	{ VK_NULL,    IDM_VIEW_WRAP_SYMBOL,                         false, false, false, nullptr },
//  { VK_NULL,    IDM_VIEW_ZOOMIN,                              false, false, false, nullptr },
//  { VK_NULL,    IDM_VIEW_ZOOMOUT,                             false, false, false, nullptr },
//  { VK_NULL,    IDM_VIEW_ZOOMRESTORE,                         false, false, false, nullptr },
	{ VK_NULL,    IDM_VIEW_GOTO_ANOTHER_VIEW,                   false, false, false, nullptr },
	{ VK_NULL,    IDM_VIEW_CLONE_TO_ANOTHER_VIEW,               false, false, false, nullptr },
	{ VK_NULL,    IDM_VIEW_GOTO_NEW_INSTANCE,                   false, false, false, nullptr },
	{ VK_NULL,    IDM_VIEW_LOAD_IN_NEW_INSTANCE,                false, false, false, nullptr },

	{ VK_NUMPAD1, IDM_VIEW_TAB1,                                true,  false, false, nullptr },
	{ VK_NUMPAD2, IDM_VIEW_TAB2,                                true,  false, false, nullptr },
	{ VK_NUMPAD3, IDM_VIEW_TAB3,                                true,  false, false, nullptr },
	{ VK_NUMPAD4, IDM_VIEW_TAB4,                                true,  false, false, nullptr },
	{ VK_NUMPAD5, IDM_VIEW_TAB5,                                true,  false, false, nullptr },
	{ VK_NUMPAD6, IDM_VIEW_TAB6,                                true,  false, false, nullptr },
	{ VK_NUMPAD7, IDM_VIEW_TAB7,                                true,  false, false, nullptr },
	{ VK_NUMPAD8, IDM_VIEW_TAB8,                                true,  false, false, nullptr },
	{ VK_NUMPAD9, IDM_VIEW_TAB9,                                true,  false, false, nullptr },
	{ VK_NEXT,    IDM_VIEW_TAB_NEXT,                            true,  false, false, nullptr },
	{ VK_PRIOR,   IDM_VIEW_TAB_PREV,                            true,  false, false, nullptr },
	{ VK_NEXT,    IDM_VIEW_TAB_MOVEFORWARD,                     true,  false, true,  nullptr },
	{ VK_PRIOR,   IDM_VIEW_TAB_MOVEBACKWARD,                    true,  false, true,  nullptr },
	{ VK_TAB,     IDC_PREV_DOC,                                 true,  false, true,  L"Switch to previous document"},
	{ VK_TAB,     IDC_NEXT_DOC,                                 true,  false, false, L"Switch to next document"},
	{ VK_NULL,    IDM_VIEW_WRAP,                                false, false, false, nullptr },
	{ VK_H,       IDM_VIEW_HIDELINES,                           false, true,  false, nullptr },
	{ VK_F8,      IDM_VIEW_SWITCHTO_OTHER_VIEW,                 false, false, false, nullptr },

	{ VK_0,       IDM_VIEW_TOGGLE_FOLDALL,                      false, true,  false, nullptr },
	{ VK_0,       IDM_VIEW_TOGGLE_UNFOLDALL,                    false, true,  true,  nullptr },
	{ VK_F,       IDM_VIEW_FOLD_CURRENT,                        true,  true,  false, nullptr },
	{ VK_F,       IDM_VIEW_UNFOLD_CURRENT,                      true,  true,  true,  nullptr },
	{ VK_1,       IDM_VIEW_FOLD_1,                              false, true,  false, nullptr },
	{ VK_2,       IDM_VIEW_FOLD_2,                              false, true,  false, nullptr },
	{ VK_3,       IDM_VIEW_FOLD_3,                              false, true,  false, nullptr },
	{ VK_4,       IDM_VIEW_FOLD_4,                              false, true,  false, nullptr },
	{ VK_5,       IDM_VIEW_FOLD_5,                              false, true,  false, nullptr },
	{ VK_6,       IDM_VIEW_FOLD_6,                              false, true,  false, nullptr },
	{ VK_7,       IDM_VIEW_FOLD_7,                              false, true,  false, nullptr },
	{ VK_8,       IDM_VIEW_FOLD_8,                              false, true,  false, nullptr },

	{ VK_1,       IDM_VIEW_UNFOLD_1,                            false, true,  true,  nullptr },
	{ VK_2,       IDM_VIEW_UNFOLD_2,                            false, true,  true,  nullptr },
	{ VK_3,       IDM_VIEW_UNFOLD_3,                            false, true,  true,  nullptr },
	{ VK_4,       IDM_VIEW_UNFOLD_4,                            false, true,  true,  nullptr },
	{ VK_5,       IDM_VIEW_UNFOLD_5,                            false, true,  true,  nullptr },
	{ VK_6,       IDM_VIEW_UNFOLD_6,                            false, true,  true,  nullptr },
	{ VK_7,       IDM_VIEW_UNFOLD_7,                            false, true,  true,  nullptr },
	{ VK_8,       IDM_VIEW_UNFOLD_8,                            false, true,  true,  nullptr },
	{ VK_NULL,    IDM_VIEW_SUMMARY,                             false, false, false, nullptr },
	{ VK_NULL,    IDM_VIEW_PROJECT_PANEL_1,                     false, false, false, nullptr },
	{ VK_NULL,    IDM_VIEW_PROJECT_PANEL_2,                     false, false, false, nullptr },
	{ VK_NULL,    IDM_VIEW_PROJECT_PANEL_3,                     false, false, false, nullptr },
	{ VK_NULL,    IDM_VIEW_FILEBROWSER,                         false, false, false, nullptr },
	{ VK_NULL,    IDM_VIEW_DOC_MAP,                             false, false, false, nullptr },
	{ VK_NULL,    IDM_VIEW_FUNC_LIST,                           false, false, false, nullptr },
	{ VK_NULL,    IDM_VIEW_SYNSCROLLV,                          false, false, false, nullptr },
	{ VK_NULL,    IDM_VIEW_SYNSCROLLH,                          false, false, false, nullptr },
	{ VK_R,       IDM_EDIT_RTL,                                 true,  true,  false, nullptr },
	{ VK_L,       IDM_EDIT_LTR,                                 true,  true,  false, nullptr },
	{ VK_NULL,    IDM_VIEW_MONITORING,                          false, false, false, nullptr },

	{ VK_NULL,    IDM_FORMAT_ANSI,                              false, false, false, nullptr },
	{ VK_NULL,    IDM_FORMAT_AS_UTF_8,                          false, false, false, nullptr },
	{ VK_NULL,    IDM_FORMAT_UTF_8,                             false, false, false, nullptr },
	{ VK_NULL,    IDM_FORMAT_UCS_2BE,                           false, false, false, nullptr },
	{ VK_NULL,    IDM_FORMAT_UCS_2LE,                           false, false, false, nullptr },

	{ VK_NULL,    IDM_FORMAT_ISO_8859_6,                        false, false, false, nullptr },
	{ VK_NULL,    IDM_FORMAT_WIN_1256,                          false, false, false, nullptr },
	{ VK_NULL,    IDM_FORMAT_ISO_8859_13,                       false, false, false, nullptr },
	{ VK_NULL,    IDM_FORMAT_WIN_1257,                          false, false, false, nullptr },
	{ VK_NULL,    IDM_FORMAT_ISO_8859_14,                       false, false, false, nullptr },
	{ VK_NULL,    IDM_FORMAT_ISO_8859_5,                        false, false, false, nullptr },
	{ VK_NULL,    IDM_FORMAT_MAC_CYRILLIC,                      false, false, false, nullptr },
	{ VK_NULL,    IDM_FORMAT_KOI8R_CYRILLIC,                    false, false, false, nullptr },
	{ VK_NULL,    IDM_FORMAT_KOI8U_CYRILLIC,                    false, false, false, nullptr },
	{ VK_NULL,    IDM_FORMAT_WIN_1251,                          false, false, false, nullptr },
	{ VK_NULL,    IDM_FORMAT_WIN_1250,                          false, false, false, nullptr },
	{ VK_NULL,    IDM_FORMAT_DOS_437,                           false, false, false, nullptr },
	{ VK_NULL,    IDM_FORMAT_DOS_720,                           false, false, false, nullptr },
	{ VK_NULL,    IDM_FORMAT_DOS_737,                           false, false, false, nullptr },
	{ VK_NULL,    IDM_FORMAT_DOS_775,                           false, false, false, nullptr },
	{ VK_NULL,    IDM_FORMAT_DOS_850,                           false, false, false, nullptr },
	{ VK_NULL,    IDM_FORMAT_DOS_852,                           false, false, false, nullptr },
	{ VK_NULL,    IDM_FORMAT_DOS_855,                           false, false, false, nullptr },
	{ VK_NULL,    IDM_FORMAT_DOS_857,                           false, false, false, nullptr },
	{ VK_NULL,    IDM_FORMAT_DOS_858,                           false, false, false, nullptr },
	{ VK_NULL,    IDM_FORMAT_DOS_860,                           false, false, false, nullptr },
	{ VK_NULL,    IDM_FORMAT_DOS_861,                           false, false, false, nullptr },
	{ VK_NULL,    IDM_FORMAT_DOS_862,                           false, false, false, nullptr },
	{ VK_NULL,    IDM_FORMAT_DOS_863,                           false, false, false, nullptr },
	{ VK_NULL,    IDM_FORMAT_DOS_865,                           false, false, false, nullptr },
	{ VK_NULL,    IDM_FORMAT_DOS_866,                           false, false, false, nullptr },
	{ VK_NULL,    IDM_FORMAT_DOS_869,                           false, false, false, nullptr },
	{ VK_NULL,    IDM_FORMAT_BIG5,                              false, false, false, nullptr },
	{ VK_NULL,    IDM_FORMAT_GB2312,                            false, false, false, nullptr },
	{ VK_NULL,    IDM_FORMAT_ISO_8859_2,                        false, false, false, nullptr },
	{ VK_NULL,    IDM_FORMAT_ISO_8859_7,                        false, false, false, nullptr },
	{ VK_NULL,    IDM_FORMAT_WIN_1253,                          false, false, false, nullptr },
	{ VK_NULL,    IDM_FORMAT_ISO_8859_8,                        false, false, false, nullptr },
	{ VK_NULL,    IDM_FORMAT_WIN_1255,                          false, false, false, nullptr },
	{ VK_NULL,    IDM_FORMAT_SHIFT_JIS,                         false, false, false, nullptr },
	{ VK_NULL,    IDM_FORMAT_EUC_KR,                            false, false, false, nullptr },
	//{ VK_NULL,    IDM_FORMAT_ISO_8859_10,                       false, false, false, nullptr },
	{ VK_NULL,    IDM_FORMAT_ISO_8859_15,                       false, false, false, nullptr },
	{ VK_NULL,    IDM_FORMAT_ISO_8859_4,                        false, false, false, nullptr },
	//{ VK_NULL,    IDM_FORMAT_ISO_8859_16,                       false, false, false, nullptr },
	{ VK_NULL,    IDM_FORMAT_ISO_8859_3,                        false, false, false, nullptr },
	//{ VK_NULL,    IDM_FORMAT_ISO_8859_11,                       false, false, false, nullptr },
	{ VK_NULL,    IDM_FORMAT_TIS_620,                           false, false, false, nullptr },
	{ VK_NULL,    IDM_FORMAT_ISO_8859_9,                        false, false, false, nullptr },
	{ VK_NULL,    IDM_FORMAT_WIN_1254,                          false, false, false, nullptr },
	{ VK_NULL,    IDM_FORMAT_WIN_1252,                          false, false, false, nullptr },
	{ VK_NULL,    IDM_FORMAT_ISO_8859_1,                        false, false, false, nullptr },
	{ VK_NULL,    IDM_FORMAT_WIN_1258,                          false, false, false, nullptr },
	{ VK_NULL,    IDM_FORMAT_CONV2_ANSI,                        false, false, false, nullptr },
	{ VK_NULL,    IDM_FORMAT_CONV2_AS_UTF_8,                    false, false, false, nullptr },
	{ VK_NULL,    IDM_FORMAT_CONV2_UTF_8,                       false, false, false, nullptr },
	{ VK_NULL,    IDM_FORMAT_CONV2_UCS_2BE,                     false, false, false, nullptr },
	{ VK_NULL,    IDM_FORMAT_CONV2_UCS_2LE,                     false, false, false, nullptr },

	{ VK_NULL,    IDM_LANG_USER_DLG,                            false, false, false, nullptr },
	{ VK_NULL,    IDM_LANG_USER,                                false, false, false, nullptr },
	{ VK_NULL,    IDM_LANG_OPENUDLDIR,                          false, false, false, nullptr },

	{ VK_NULL,    IDM_SETTING_PREFERENCE,                       false, false, false, nullptr },
	{ VK_NULL,    IDM_LANGSTYLE_CONFIG_DLG,                     false, false, false, nullptr },
	{ VK_NULL,    IDM_SETTING_SHORTCUT_MAPPER,                  false, false, false, nullptr },
	{ VK_NULL,    IDM_SETTING_IMPORTPLUGIN,                     false, false, false, nullptr },
	{ VK_NULL,    IDM_SETTING_IMPORTSTYLETHEMS,                 false, false, false, nullptr },
	{ VK_NULL,    IDM_SETTING_EDITCONTEXTMENU,                  false, false, false, nullptr },

	{ VK_R,       IDC_EDIT_TOGGLEMACRORECORDING,                true,  false, true,  L"Toggle macro record"},
	{ VK_NULL,    IDM_MACRO_STARTRECORDINGMACRO,                false, false, false, nullptr },
	{ VK_NULL,    IDM_MACRO_STOPRECORDINGMACRO,                 false, false, false, nullptr },
	{ VK_P,       IDM_MACRO_PLAYBACKRECORDEDMACRO,              true,  false, true,  nullptr },
	{ VK_NULL,    IDM_MACRO_SAVECURRENTMACRO,                   false, false, false, nullptr },
	{ VK_NULL,    IDM_MACRO_RUNMULTIMACRODLG,                   false, false, false, nullptr },

	{ VK_F5,      IDM_EXECUTE,                                  false, false, false, nullptr },

	{ VK_NULL,    IDM_CMDLINEARGUMENTS,                         false, false, false, nullptr },
	{ VK_NULL,    IDM_HOMESWEETHOME,                            false, false, false, nullptr },
	{ VK_NULL,    IDM_PROJECTPAGE,                              false, false, false, nullptr },
//  { VK_NULL,    IDM_ONLINEDOCUMENT,                               false, false, false, nullptr },
	{ VK_NULL,    IDM_FORUM,                                    false, false, false, nullptr },
	{ VK_NULL,    IDM_ONLINESUPPORT,                            false, false, false, nullptr },
//	{ VK_NULL,    IDM_PLUGINSHOME,                              false, false, false, nullptr },
	{ VK_NULL,    IDM_UPDATE_NPP,                               false, false, false, nullptr },
	{ VK_NULL,    IDM_CONFUPDATERPROXY,                         false, false, false, nullptr },
	{ VK_NULL,    IDM_DEBUGINFO,                                false, false, false, nullptr },
	{ VK_F1,      IDM_ABOUT,                                    false, false, false, nullptr }
//  { VK_F1,      IDM_HELP,                                     false, false, true,  nullptr }
};




/*!
** \brief array of accelerator keys for all possible scintilla functions
**
** values can be 0 for vKey, which means its unused
*/
static const ScintillaKeyDefinition scintKeyDefs[] =
{
	{L"SCI_CUT",                     SCI_CUT,                     true,  false, false, VK_X,        IDM_EDIT_CUT},
	{L"",                            SCI_CUT,                     false, false, true,  VK_DELETE,   0},
	{L"SCI_COPY",                    SCI_COPY,                    true,  false, false, VK_C,        IDM_EDIT_COPY},
	{L"",                            SCI_COPY,                    true,  false, false, VK_INSERT,   0},
	{L"SCI_PASTE",                   SCI_PASTE,                   true,  false, false, VK_V,        IDM_EDIT_PASTE},
	{L"",                            SCI_PASTE,                   false, false, true,  VK_INSERT,   0},
	{L"SCI_SELECTALL",               SCI_SELECTALL,               true,  false, false, VK_A,        IDM_EDIT_SELECTALL},
	{L"SCI_CLEAR",                   SCI_CLEAR,                   false, false, false, VK_DELETE,   IDM_EDIT_DELETE},
	{L"SCI_CLEARALL",                SCI_CLEARALL,                false, false, false, 0,           0},
	{L"SCI_UNDO",                    SCI_UNDO,                    true,  false, false, VK_Z,        IDM_EDIT_UNDO},
	{L"",                            SCI_UNDO,                    false, true,  false, VK_BACK,     0},
	{L"SCI_REDO",                    SCI_REDO,                    true,  false, false, VK_Y,        IDM_EDIT_REDO},
	{L"",                            SCI_REDO,                    true,  false, true,  VK_Z,        0},
	{L"SCI_NEWLINE",                 SCI_NEWLINE,                 false, false, false, VK_RETURN,   0},
	{L"",                            SCI_NEWLINE,                 false, false, true,  VK_RETURN,   0},
	{L"SCI_TAB",                     SCI_TAB,                     false, false, false, VK_TAB,      IDM_EDIT_INS_TAB},
	{L"SCI_BACKTAB",                 SCI_BACKTAB,                 false, false, true,  VK_TAB,      IDM_EDIT_RMV_TAB},
	{L"SCI_FORMFEED",                SCI_FORMFEED,                false, false, false, 0,           0},
	{L"SCI_ZOOMIN",                  SCI_ZOOMIN,                  true,  false, false, VK_ADD,      IDM_VIEW_ZOOMIN},
	{L"SCI_ZOOMOUT",                 SCI_ZOOMOUT,                 true,  false, false, VK_SUBTRACT, IDM_VIEW_ZOOMOUT},
	{L"SCI_SETZOOM",                 SCI_SETZOOM,                 true,  false, false, VK_DIVIDE,   IDM_VIEW_ZOOMRESTORE},
	{L"SCI_SELECTIONDUPLICATE",      SCI_SELECTIONDUPLICATE,      true,  false, false, VK_D,        IDM_EDIT_DUP_LINE},
	{L"SCI_LINESJOIN",               SCI_LINESJOIN,               false, false, false, 0,           0},
	{L"SCI_SCROLLCARET",             SCI_SCROLLCARET,             false, false, false, 0,           0},
	{L"SCI_EDITTOGGLEOVERTYPE",      SCI_EDITTOGGLEOVERTYPE,      false, false, false, VK_INSERT,   0},
	{L"SCI_MOVECARETINSIDEVIEW",     SCI_MOVECARETINSIDEVIEW,     false, false, false, 0,           0},
	{L"SCI_LINEDOWN",                SCI_LINEDOWN,                false, false, false, VK_DOWN,     0},
	{L"SCI_LINEDOWNEXTEND",          SCI_LINEDOWNEXTEND,          false, false, true,  VK_DOWN,     0},
	{L"SCI_LINEDOWNRECTEXTEND",      SCI_LINEDOWNRECTEXTEND,      false, true,  true,  VK_DOWN,     0},
	{L"SCI_LINESCROLLDOWN",          SCI_LINESCROLLDOWN,          true,  false, false, VK_DOWN,     0},
	{L"SCI_LINEUP",                  SCI_LINEUP,                  false, false, false, VK_UP,       0},
	{L"SCI_LINEUPEXTEND",            SCI_LINEUPEXTEND,            false, false, true,  VK_UP,       0},
	{L"SCI_LINEUPRECTEXTEND",        SCI_LINEUPRECTEXTEND,        false, true,  true,  VK_UP,       0},
	{L"SCI_LINESCROLLUP",            SCI_LINESCROLLUP,            true,  false, false, VK_UP,       0},
	{L"SCI_PARADOWN",                SCI_PARADOWN,                true,  false, false, VK_OEM_6,    0},
	{L"SCI_PARADOWNEXTEND",          SCI_PARADOWNEXTEND,          true,  false, true,  VK_OEM_6,    0},
	{L"SCI_PARAUP",                  SCI_PARAUP,                  true,  false, false, VK_OEM_4,    0},
	{L"SCI_PARAUPEXTEND",            SCI_PARAUPEXTEND,            true,  false, true,  VK_OEM_4,    0},
	{L"SCI_CHARLEFT",                SCI_CHARLEFT,                false, false, false, VK_LEFT,     0},
	{L"SCI_CHARLEFTEXTEND",          SCI_CHARLEFTEXTEND,          false, false, true,  VK_LEFT,     0},
	{L"SCI_CHARLEFTRECTEXTEND",      SCI_CHARLEFTRECTEXTEND,      false, true,  true,  VK_LEFT,     0},
	{L"SCI_CHARRIGHT",               SCI_CHARRIGHT,               false, false, false, VK_RIGHT,    0},
	{L"SCI_CHARRIGHTEXTEND",         SCI_CHARRIGHTEXTEND,         false, false, true,  VK_RIGHT,    0},
	{L"SCI_CHARRIGHTRECTEXTEND",     SCI_CHARRIGHTRECTEXTEND,     false, true,  true,  VK_RIGHT,    0},
	{L"SCI_WORDLEFT",                SCI_WORDLEFT,                true,  false, false, VK_LEFT,     0},
	{L"SCI_WORDLEFTEXTEND",          SCI_WORDLEFTEXTEND,          true,  false, true,  VK_LEFT,     0},
	{L"SCI_WORDRIGHT",               SCI_WORDRIGHT,               true,  false, false, VK_RIGHT,    0},
	{L"SCI_WORDRIGHTEXTEND",         SCI_WORDRIGHTEXTEND,         false, false, false, 0,           0},
	{L"SCI_WORDLEFTEND",             SCI_WORDLEFTEND,             false, false, false, 0,           0},
	{L"SCI_WORDLEFTENDEXTEND",       SCI_WORDLEFTENDEXTEND,       false, false, false, 0,           0},
	{L"SCI_WORDRIGHTEND",            SCI_WORDRIGHTEND,            false, false, false, 0,           0},
	{L"SCI_WORDRIGHTENDEXTEND",      SCI_WORDRIGHTENDEXTEND,      true,  false, true,  VK_RIGHT,    0},
	{L"SCI_WORDPARTLEFT",            SCI_WORDPARTLEFT,            true,  false, false, VK_OEM_2,    0},
	{L"SCI_WORDPARTLEFTEXTEND",      SCI_WORDPARTLEFTEXTEND,      true,  false, true,  VK_OEM_2,    0},
	{L"SCI_WORDPARTRIGHT",           SCI_WORDPARTRIGHT,           true,  false, false, VK_OEM_5,    0},
	{L"SCI_WORDPARTRIGHTEXTEND",     SCI_WORDPARTRIGHTEXTEND,     true,  false, true,  VK_OEM_5,    0},
	{L"SCI_HOME",                    SCI_HOME,                    false, false, false, 0,           0},
	{L"SCI_HOMEEXTEND",              SCI_HOMEEXTEND,              false, false, false, 0,           0},
	{L"SCI_HOMERECTEXTEND",          SCI_HOMERECTEXTEND,          false, false, false, 0,           0},
	{L"SCI_HOMEDISPLAY",             SCI_HOMEDISPLAY,             false, true,  false, VK_HOME,     0},
	{L"SCI_HOMEDISPLAYEXTEND",       SCI_HOMEDISPLAYEXTEND,       false, false, false, 0,           0},
	{L"SCI_HOMEWRAP",                SCI_HOMEWRAP,                false, false, false, 0,           0},
	{L"SCI_HOMEWRAPEXTEND",          SCI_HOMEWRAPEXTEND,          false, false, false, 0,           0},
	{L"SCI_VCHOME",                  SCI_VCHOME,                  false, false, false, 0,           0},
	{L"SCI_VCHOMEWRAPEXTEND",        SCI_VCHOMEWRAPEXTEND,        false, false, true,  VK_HOME,     0},
	{L"SCI_VCHOMERECTEXTEND",        SCI_VCHOMERECTEXTEND,        false, true,  true,  VK_HOME,     0},
	{L"SCI_VCHOMEWRAP",              SCI_VCHOMEWRAP,              false, false, false, VK_HOME,     0},
	{L"SCI_LINEEND",                 SCI_LINEEND,                 false, false, false, 0,           0},
	{L"SCI_LINEENDWRAPEXTEND",       SCI_LINEENDWRAPEXTEND,       false, false, true,  VK_END,      0},
	{L"SCI_LINEENDRECTEXTEND",       SCI_LINEENDRECTEXTEND,       false, true,  true,  VK_END,      0},
	{L"SCI_LINEENDDISPLAY",          SCI_LINEENDDISPLAY,          false, true,  false, VK_END,      0},
	{L"SCI_LINEENDDISPLAYEXTEND",    SCI_LINEENDDISPLAYEXTEND,    false, false, false, 0,           0},
	{L"SCI_LINEENDWRAP",             SCI_LINEENDWRAP,             false, false, false, VK_END,      0},
	{L"SCI_LINEENDEXTEND",           SCI_LINEENDEXTEND,           false, false, false, 0,           0},
	{L"SCI_DOCUMENTSTART",           SCI_DOCUMENTSTART,           true,  false, false, VK_HOME,     0},
	{L"SCI_DOCUMENTSTARTEXTEND",     SCI_DOCUMENTSTARTEXTEND,     true,  false, true,  VK_HOME,     0},
	{L"SCI_DOCUMENTEND",             SCI_DOCUMENTEND,             true,  false, false, VK_END,      0},
	{L"SCI_DOCUMENTENDEXTEND",       SCI_DOCUMENTENDEXTEND,       true,  false, true,  VK_END,      0},
	{L"SCI_PAGEUP",                  SCI_PAGEUP,                  false, false, false, VK_PRIOR,    0},
	{L"SCI_PAGEUPEXTEND",            SCI_PAGEUPEXTEND,            false, false, true,  VK_PRIOR,    0},
	{L"SCI_PAGEUPRECTEXTEND",        SCI_PAGEUPRECTEXTEND,        false, true,  true,  VK_PRIOR,    0},
	{L"SCI_PAGEDOWN",                SCI_PAGEDOWN,                false, false, false, VK_NEXT,     0},
	{L"SCI_PAGEDOWNEXTEND",          SCI_PAGEDOWNEXTEND,          false, false, true,  VK_NEXT,     0},
	{L"SCI_PAGEDOWNRECTEXTEND",      SCI_PAGEDOWNRECTEXTEND,      false, true,  true,  VK_NEXT,     0},
	{L"SCI_STUTTEREDPAGEUP",         SCI_STUTTEREDPAGEUP,         false, false, false, 0,           0},
	{L"SCI_STUTTEREDPAGEUPEXTEND",   SCI_STUTTEREDPAGEUPEXTEND,   false, false, false, 0,           0},
	{L"SCI_STUTTEREDPAGEDOWN",       SCI_STUTTEREDPAGEDOWN,       false, false, false, 0,           0},
	{L"SCI_STUTTEREDPAGEDOWNEXTEND", SCI_STUTTEREDPAGEDOWNEXTEND, false, false, false, 0,           0},
	{L"SCI_DELETEBACK",              SCI_DELETEBACK,              false, false, false, VK_BACK,     0},
	{L"",                            SCI_DELETEBACK,              false, false, true,  VK_BACK,     0},
	{L"SCI_DELETEBACKNOTLINE",       SCI_DELETEBACKNOTLINE,       false, false, false, 0,           0},
	{L"SCI_DELWORDLEFT",             SCI_DELWORDLEFT,             true,  false, false, VK_BACK,     0},
	{L"SCI_DELWORDRIGHT",            SCI_DELWORDRIGHT,            true,  false, false, VK_DELETE,   0},
	{L"SCI_DELLINELEFT",             SCI_DELLINELEFT,             true,  false, true,  VK_BACK,     0},
	{L"SCI_DELLINERIGHT",            SCI_DELLINERIGHT,            true,  false, true,  VK_DELETE,   0},
	{L"SCI_LINEDELETE",              SCI_LINEDELETE,              true,  false, true,  VK_L,        0},
	{L"SCI_LINECUT",                 SCI_LINECUT,                 true,  false, false, VK_L,        0},
	{L"SCI_LINECOPY",                SCI_LINECOPY,                true,  false, true,  VK_X,        0},
	{L"SCI_LINETRANSPOSE",           SCI_LINETRANSPOSE,           true,  false, false, VK_T,        0},
	{L"SCI_LINEDUPLICATE",           SCI_LINEDUPLICATE,           false, false, false, 0,           0},
	{L"SCI_CANCEL",                  SCI_CANCEL,                  false, false, false, VK_ESCAPE,   0},
	{L"SCI_SWAPMAINANCHORCARET",     SCI_SWAPMAINANCHORCARET,     false, false, false, 0,           0},
	{L"SCI_ROTATESELECTION",         SCI_ROTATESELECTION,         false, false, false, 0,           0}
};


typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);

int strVal(const TCHAR *str, int base)
{
	if (!str) return -1;
	if (!str[0]) return 0;

	TCHAR *finStr;
	int result = generic_strtol(str, &finStr, base);
	if (*finStr != '\0')
		return -1;
	return result;
}


int decStrVal(const TCHAR *str)
{
	return strVal(str, 10);
}

int hexStrVal(const TCHAR *str)
{
	return strVal(str, 16);
}

int getKwClassFromName(const TCHAR *str)
{
	if (!lstrcmp(L"instre1", str)) return LANG_INDEX_INSTR;
	if (!lstrcmp(L"instre2", str)) return LANG_INDEX_INSTR2;
	if (!lstrcmp(L"type1", str)) return LANG_INDEX_TYPE;
	if (!lstrcmp(L"type2", str)) return LANG_INDEX_TYPE2;
	if (!lstrcmp(L"type3", str)) return LANG_INDEX_TYPE3;
	if (!lstrcmp(L"type4", str)) return LANG_INDEX_TYPE4;
	if (!lstrcmp(L"type5", str)) return LANG_INDEX_TYPE5;
	if (!lstrcmp(L"type6", str)) return LANG_INDEX_TYPE6;
	if (!lstrcmp(L"type7", str)) return LANG_INDEX_TYPE7;

	if ((str[1] == '\0') && (str[0] >= '0') && (str[0] <= '8')) // up to KEYWORDSET_MAX
		return str[0] - '0';

	return -1;
}

	
size_t getAsciiLenFromBase64Len(size_t base64StrLen)
{
	return (base64StrLen % 4) ? 0 : (base64StrLen - base64StrLen / 4);
}


int base64ToAscii(char *dest, const char *base64Str)
{
	static const int base64IndexArray[123] =
	{
		-1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, 62, -1, -1, -1, 63,
		52, 53, 54, 55 ,56, 57, 58, 59,
		60, 61, -1, -1, -1, -1, -1, -1,
		-1,  0,  1,  2,  3,  4,  5,  6,
			7,  8,  9, 10, 11, 12, 13, 14,
		15, 16, 17, 18, 19, 20, 21, 22,
		23, 24, 25, -1, -1, -1, -1 ,-1,
		-1, 26, 27, 28, 29, 30, 31, 32,
		33, 34, 35, 36, 37, 38, 39, 40,
		41, 42, 43, 44, 45, 46, 47, 48,
		49, 50, 51
	};

	size_t b64StrLen = strlen(base64Str);
	size_t nbLoop = b64StrLen / 4;

	size_t i = 0;
	int k = 0;

	enum {b64_just, b64_1padded, b64_2padded} padd = b64_just;
	for ( ; i < nbLoop ; i++)
	{
		size_t j = i * 4;
		UCHAR uc0, uc1, uc2, uc3, p0, p1;

		uc0 = (UCHAR)base64IndexArray[base64Str[j]];
		uc1 = (UCHAR)base64IndexArray[base64Str[j+1]];
		uc2 = (UCHAR)base64IndexArray[base64Str[j+2]];
		uc3 = (UCHAR)base64IndexArray[base64Str[j+3]];

		if ((uc0 == -1) || (uc1 == -1) || (uc2 == -1) || (uc3 == -1))
			return -1;

		if (base64Str[j+2] == '=') // && (uc3 == '=')
		{
			uc2 = uc3 = 0;
			padd = b64_2padded;
		}
		else if (base64Str[j+3] == '=')
		{
			uc3 = 0;
			padd = b64_1padded;
		}

		p0 = uc0 << 2;
		p1 = uc1 << 2;
		p1 >>= 6;
		dest[k++] = p0 | p1;

		p0 = uc1 << 4;
		p1 = uc2 << 2;
		p1 >>= 4;
		dest[k++] = p0 | p1;

		p0 = uc2 << 6;
		p1 = uc3;
		dest[k++] = p0 | p1;
	}

	//dest[k] = '\0';
	if (padd == b64_1padded)
	//	dest[k-1] = '\0';
		return k-1;
	else if (padd == b64_2padded)
	//	dest[k-2] = '\0';
		return k-2;

	return k;
}

} // anonymous namespace


void cutString(const TCHAR* str2cut, vector<generic_string>& patternVect)
{
	if (str2cut == nullptr) return;

	const TCHAR *pBegin = str2cut;
	const TCHAR *pEnd = pBegin;

	while (*pEnd != '\0')
	{
		if (_istspace(*pEnd))
		{
			if (pBegin != pEnd)
				patternVect.emplace_back(pBegin, pEnd);
			pBegin = pEnd + 1;
		
		}
		++pEnd;
	}

	if (pBegin != pEnd)
		patternVect.emplace_back(pBegin, pEnd);
}


std::wstring LocalizationSwitcher::getLangFromXmlFileName(const wchar_t *fn) const
{
	size_t nbItem = sizeof(localizationDefs)/sizeof(LocalizationSwitcher::LocalizationDefinition);
	for (size_t i = 0 ; i < nbItem ; ++i)
	{
		if (0 == wcsicmp(fn, localizationDefs[i]._xmlFileName))
			return localizationDefs[i]._langName;
	}
	return std::wstring();
}


std::wstring LocalizationSwitcher::getXmlFilePathFromLangName(const wchar_t *langName) const
{
	for (size_t i = 0, len = _localizationList.size(); i < len ; ++i)
	{
		if (0 == wcsicmp(langName, _localizationList[i].first.c_str()))
			return _localizationList[i].second;
	}
	return std::wstring();
}


bool LocalizationSwitcher::addLanguageFromXml(const std::wstring& xmlFullPath)
{
	wchar_t * fn = ::PathFindFileNameW(xmlFullPath.c_str());
	wstring foundLang = getLangFromXmlFileName(fn);
	if (not foundLang.empty())
	{
		_localizationList.push_back(pair<wstring, wstring>(foundLang, xmlFullPath));
		return true;
	}
	return false;
}


bool LocalizationSwitcher::switchToLang(const wchar_t *lang2switch) const
{
	wstring langPath = getXmlFilePathFromLangName(lang2switch);
	if (langPath.empty())
		return false;

	return ::CopyFileW(langPath.c_str(), _nativeLangPath.c_str(), FALSE) != FALSE;
}


generic_string ThemeSwitcher::getThemeFromXmlFileName(const TCHAR *xmlFullPath) const
{
	if (!xmlFullPath || !xmlFullPath[0])
		return generic_string();
	generic_string fn(::PathFindFileName(xmlFullPath));
	PathRemoveExtension(const_cast<TCHAR *>(fn.c_str()));
	return fn;
}


winVer NppParameters::getWindowsVersion()
{
	OSVERSIONINFOEX osvi;
	SYSTEM_INFO si;
	PGNSI pGNSI;

	ZeroMemory(&si, sizeof(SYSTEM_INFO));
	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));

	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	BOOL bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *)&osvi);
	if (!bOsVersionInfoEx)
	{
		osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
		if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) )
			return WV_UNKNOWN;
	}

	pGNSI = (PGNSI) GetProcAddress(GetModuleHandle(L"kernel32.dll"), "GetNativeSystemInfo");
	if (pGNSI != NULL)
		pGNSI(&si);
	else
		GetSystemInfo(&si);

	switch (si.wProcessorArchitecture)
	{
	case PROCESSOR_ARCHITECTURE_IA64:
		_platForm = PF_IA64;
		break;

	case PROCESSOR_ARCHITECTURE_AMD64:
		_platForm = PF_X64;
		break;

	case PROCESSOR_ARCHITECTURE_INTEL:
		_platForm = PF_X86;
		break;

	default:
		_platForm = PF_UNKNOWN;
	}

   switch (osvi.dwPlatformId)
   {
		case VER_PLATFORM_WIN32_NT:
		{
			if (osvi.dwMajorVersion == 10 && osvi.dwMinorVersion == 0)
				return WV_WIN10;

			if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 3)
				return WV_WIN81;

			if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 2)
				return WV_WIN8;

			if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 1)
				return WV_WIN7;

			if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 0)
				return WV_VISTA;

			if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2)
			{
				if (osvi.wProductType == VER_NT_WORKSTATION && si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64)
					return WV_XPX64;
				return WV_S2003;
			}

			if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1)
				return WV_XP;

			if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0)
				return WV_W2K;

			if (osvi.dwMajorVersion <= 4)
				return WV_NT;
			break;
		}

		// Test for the Windows Me/98/95.
		case VER_PLATFORM_WIN32_WINDOWS:
		{
			if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 0)
				return WV_95;

			if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 10)
				return WV_98;

			if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 90)
				return WV_ME;
			break;
		}

		case VER_PLATFORM_WIN32s:
			return WV_WIN32S;

		default:
			return WV_UNKNOWN;
   }

   return WV_UNKNOWN;
}

int FileDialog::_dialogFileBoxId = (NppParameters::getInstance()).getWinVersion() < WV_W2K?edt1:cmb13;


NppParameters::NppParameters()
{
	//Get windows version
	_winVersion = getWindowsVersion();

	// Prepare for default path
	TCHAR nppPath[MAX_PATH];
	::GetModuleFileName(NULL, nppPath, MAX_PATH);

	PathRemoveFileSpec(nppPath);
	_nppPath = nppPath;

	//Initialize current directory to startup directory
	TCHAR curDir[MAX_PATH];
	::GetCurrentDirectory(MAX_PATH, curDir);
	_currentDirectory = curDir;

	_appdataNppDir.clear();
	generic_string notepadStylePath(_nppPath);
	PathAppend(notepadStylePath, notepadStyleFile);

	_asNotepadStyle = (PathFileExists(notepadStylePath.c_str()) == TRUE);

	//Load initial accelerator key definitions
	initMenuKeys();
	initScintillaKeys();
}


NppParameters::~NppParameters()
{
	for (int i = 0 ; i < _nbLang ; ++i)
		delete _langList[i];
	for (int i = 0 ; i < _nbRecentFile ; ++i)
		delete _LRFileList[i];
	for (int i = 0 ; i < _nbUserLang ; ++i)
		delete _userLangArray[i];
	if (_hUXTheme)
		FreeLibrary(_hUXTheme);

	for (std::vector<TiXmlDocument *>::iterator it = _pXmlExternalLexerDoc.begin(), end = _pXmlExternalLexerDoc.end(); it != end; ++it )
		delete (*it);

	_pXmlExternalLexerDoc.clear();
}


bool NppParameters::reloadStylers(TCHAR* stylePath)
{
	delete _pXmlUserStylerDoc;

	const TCHAR* stylePathToLoad = stylePath != nullptr ? stylePath : _stylerPath.c_str();
	_pXmlUserStylerDoc = new TiXmlDocument(stylePathToLoad);

	bool loadOkay = _pXmlUserStylerDoc->LoadFile();
	if (!loadOkay)
	{
		if (!_pNativeLangSpeaker)
		{
			::MessageBox(NULL, stylePathToLoad, L"Load stylers.xml failed", MB_OK);
		}
		else
		{
			_pNativeLangSpeaker->messageBox("LoadStylersFailed",
				NULL,
				L"Load \"$STR_REPLACE$\" failed!",
				L"Load stylers.xml failed",
				MB_OK,
				0,
				stylePathToLoad);
		}
		delete _pXmlUserStylerDoc;
		_pXmlUserStylerDoc = NULL;
		return false;
	}
	_lexerStylerArray.eraseAll();
	_widgetStyleArray.setNbStyler( 0 );

	getUserStylersFromXmlTree();

	//  Reload plugin styles.
	for ( size_t i = 0; i < getExternalLexerDoc()->size(); ++i)
	{
		getExternalLexerFromXmlTree( getExternalLexerDoc()->at(i) );
	}
	return true;
}

bool NppParameters::reloadLang()
{
	// use user path
	generic_string nativeLangPath(_localizationSwitcher._nativeLangPath);

	// if "nativeLang.xml" does not exist, use npp path
	if (!PathFileExists(nativeLangPath.c_str()))
	{
		nativeLangPath = _nppPath;
		PathAppend(nativeLangPath, generic_string(L"nativeLang.xml"));
		if (!PathFileExists(nativeLangPath.c_str()))
			return false;
	}

	delete _pXmlNativeLangDocA;

	_pXmlNativeLangDocA = new TiXmlDocumentA();

	bool loadOkay = _pXmlNativeLangDocA->LoadUnicodeFilePath(nativeLangPath.c_str());
	if (!loadOkay)
	{
		delete _pXmlNativeLangDocA;
		_pXmlNativeLangDocA = nullptr;
		return false;
	}
	return loadOkay;
}

generic_string NppParameters::getSpecialFolderLocation(int folderKind)
{
	TCHAR path[MAX_PATH];
	const HRESULT specialLocationResult = SHGetFolderPath(nullptr, folderKind, nullptr, SHGFP_TYPE_CURRENT, path);

	generic_string result;
	if (SUCCEEDED(specialLocationResult))
	{
		result = path;
	}
	return result;
}


generic_string NppParameters::getSettingsFolder()
{
	if (_isLocal)
		return _nppPath;

	generic_string settingsFolderPath = getSpecialFolderLocation(CSIDL_APPDATA);

	if (settingsFolderPath.empty())
		return _nppPath;

	PathAppend(settingsFolderPath, L"Notepad++");
	return settingsFolderPath;
}


bool NppParameters::load()
{
	L_END = L_EXTERNAL;
	bool isAllLaoded = true;
	for (int i = 0 ; i < NB_LANG ; _langList[i] = NULL, ++i)
	{}

	_isx64 = sizeof(void *) == 8;

	// Make localConf.xml path
	generic_string localConfPath(_nppPath);
	PathAppend(localConfPath, localConfFile);

	// Test if localConf.xml exist
	_isLocal = (PathFileExists(localConfPath.c_str()) == TRUE);

	// Under vista and windows 7, the usage of doLocalConf.xml is not allowed
	// if Notepad++ is installed in "program files" directory, because of UAC
	if (_isLocal)
	{
		// We check if OS is Vista or greater version
		if (_winVersion >= WV_VISTA)
		{
			generic_string progPath = getSpecialFolderLocation(CSIDL_PROGRAM_FILES);
			TCHAR nppDirLocation[MAX_PATH];
			wcscpy_s(nppDirLocation, _nppPath.c_str());
			::PathRemoveFileSpec(nppDirLocation);

			if  (progPath == nppDirLocation)
				_isLocal = false;
		}
	}

	_pluginRootDir = _nppPath;
	PathAppend(_pluginRootDir, L"plugins");

	generic_string nppPluginRootParent;
	if (_isLocal)
	{
		_userPath = nppPluginRootParent = _nppPath;
		_userPluginConfDir = _pluginRootDir;
		PathAppend(_userPluginConfDir, L"Config");
	}
	else
	{
		_userPath = getSpecialFolderLocation(CSIDL_APPDATA);

		PathAppend(_userPath, L"Notepad++");
		if (!PathFileExists(_userPath.c_str()))
			::CreateDirectory(_userPath.c_str(), NULL);

		_appdataNppDir = _userPluginConfDir = _userPath;
		PathAppend(_userPluginConfDir, L"plugins");
		if (!PathFileExists(_userPluginConfDir.c_str()))
			::CreateDirectory(_userPluginConfDir.c_str(), NULL);
		PathAppend(_userPluginConfDir, L"Config");
		if (!PathFileExists(_userPluginConfDir.c_str()))
			::CreateDirectory(_userPluginConfDir.c_str(), NULL);

		// For PluginAdmin to launch the wingup with UAC
		setElevationRequired(true);
	}

	_pluginConfDir = _pluginRootDir; // for plugin list home
	PathAppend(_pluginConfDir, L"Config");

	if (!PathFileExists(nppPluginRootParent.c_str()))
		::CreateDirectory(nppPluginRootParent.c_str(), NULL);
	if (!PathFileExists(_pluginRootDir.c_str()))
		::CreateDirectory(_pluginRootDir.c_str(), NULL);

	_sessionPath = _userPath; // Session stock the absolute file path, it should never be on cloud

	// Detection cloud settings
	generic_string cloudChoicePath{_userPath};
	cloudChoicePath += L"\\cloud\\choice";

	// cloudChoicePath doesn't exist, just quit
	if (::PathFileExists(cloudChoicePath.c_str()))
	{
		// Read cloud choice
		std::string cloudChoiceStr = getFileContent(cloudChoicePath.c_str());
		WcharMbcsConvertor& wmc = WcharMbcsConvertor::getInstance();
		std::wstring cloudChoiceStrW = wmc.char2wchar(cloudChoiceStr.c_str(), SC_CP_UTF8);

		if (not cloudChoiceStrW.empty() and ::PathFileExists(cloudChoiceStrW.c_str()))
		{
			_userPath = cloudChoiceStrW;
			_nppGUI._cloudPath = cloudChoiceStrW;
			_initialCloudChoice = _nppGUI._cloudPath;
		}
	}


	//-------------------------------------//
	// Transparent function for w2k and xp //
	//-------------------------------------//
	HMODULE hUser32 = ::GetModuleHandle(L"User32");
	if (hUser32)
		_transparentFuncAddr = (WNDPROC)::GetProcAddress(hUser32, "SetLayeredWindowAttributes");

	//---------------------------------------------//
	// Dlg theme texture function for xp and vista //
	//---------------------------------------------//
	_hUXTheme = ::LoadLibrary(L"uxtheme.dll");
	if (_hUXTheme)
		_enableThemeDialogTextureFuncAddr = (WNDPROC)::GetProcAddress(_hUXTheme, "EnableThemeDialogTexture");

	//--------------------------//
	// langs.xml : for per user //
	//--------------------------//
	generic_string langs_xml_path(_userPath);
	PathAppend(langs_xml_path, L"langs.xml");

	BOOL doRecover = FALSE;
	if (::PathFileExists(langs_xml_path.c_str()))
	{
		WIN32_FILE_ATTRIBUTE_DATA attributes;

		if (GetFileAttributesEx(langs_xml_path.c_str(), GetFileExInfoStandard, &attributes) != 0)
		{
			if (attributes.nFileSizeLow == 0 && attributes.nFileSizeHigh == 0)
			{
				if (_pNativeLangSpeaker)
				{
					doRecover = _pNativeLangSpeaker->messageBox("LoadLangsFailed",
						NULL,
						L"Load langs.xml failed!\rDo you want to recover your langs.xml?",
						L"Configurator",
						MB_YESNO);
				}
				else
				{
					doRecover = ::MessageBox(NULL, L"Load langs.xml failed!\rDo you want to recover your langs.xml?", L"Configurator", MB_YESNO);
				}
			}
		}
	}
	else
		doRecover = true;

	if (doRecover)
	{
		generic_string srcLangsPath(_nppPath);
		PathAppend(srcLangsPath, L"langs.model.xml");
		::CopyFile(srcLangsPath.c_str(), langs_xml_path.c_str(), FALSE);
	}

	_pXmlDoc = new TiXmlDocument(langs_xml_path);


	bool loadOkay = _pXmlDoc->LoadFile();
	if (!loadOkay)
	{
		if (_pNativeLangSpeaker)
		{
			_pNativeLangSpeaker->messageBox("LoadLangsFailedFinal",
				NULL,
				L"Load langs.xml failed!",
				L"Configurator",
				MB_OK);
		}
		else
		{
			::MessageBox(NULL, L"Load langs.xml failed!", L"Configurator", MB_OK);
		}

		delete _pXmlDoc;
		_pXmlDoc = nullptr;
		isAllLaoded = false;
	}
	else
		getLangKeywordsFromXmlTree();

	//---------------------------//
	// config.xml : for per user //
	//---------------------------//
	generic_string configPath(_userPath);
	PathAppend(configPath, L"config.xml");

	generic_string srcConfigPath(_nppPath);
	PathAppend(srcConfigPath, L"config.model.xml");

	if (!::PathFileExists(configPath.c_str()))
		::CopyFile(srcConfigPath.c_str(), configPath.c_str(), FALSE);

	_pXmlUserDoc = new TiXmlDocument(configPath);
	loadOkay = _pXmlUserDoc->LoadFile();
	
	if (!loadOkay)
	{
		TiXmlDeclaration* decl = new TiXmlDeclaration(L"1.0", L"Windows-1252", L"");
		_pXmlUserDoc->LinkEndChild(decl);
	}
	else
	{
		getUserParametersFromXmlTree();
	}

	//----------------------------//
	// stylers.xml : for per user //
	//----------------------------//

	_stylerPath = _userPath;
	PathAppend(_stylerPath, L"stylers.xml");

	if (!PathFileExists(_stylerPath.c_str()))
	{
		generic_string srcStylersPath(_nppPath);
		PathAppend(srcStylersPath, L"stylers.model.xml");

		::CopyFile(srcStylersPath.c_str(), _stylerPath.c_str(), TRUE);
	}

	if (_nppGUI._themeName.empty() || (!PathFileExists(_nppGUI._themeName.c_str())))
		_nppGUI._themeName.assign(_stylerPath);

	_pXmlUserStylerDoc = new TiXmlDocument(_nppGUI._themeName.c_str());

	loadOkay = _pXmlUserStylerDoc->LoadFile();
	if (!loadOkay)
	{
		if (_pNativeLangSpeaker)
		{
			_pNativeLangSpeaker->messageBox("LoadStylersFailed",
				NULL,
				L"Load \"$STR_REPLACE$\" failed!",
				L"Load stylers.xml failed",
				MB_OK,
				0,
				_stylerPath.c_str());
		}
		else
		{
			::MessageBox(NULL, _stylerPath.c_str(), L"Load stylers.xml failed", MB_OK);
		}
		delete _pXmlUserStylerDoc;
		_pXmlUserStylerDoc = NULL;
		isAllLaoded = false;
	}
	else
		getUserStylersFromXmlTree();

	_themeSwitcher._stylesXmlPath = _stylerPath;
	// Firstly, add the default theme
	_themeSwitcher.addDefaultThemeFromXml(_stylerPath);

	//-----------------------------------//
	// userDefineLang.xml : for per user //
	//-----------------------------------//
	_userDefineLangsFolderPath = _userDefineLangPath = _userPath;
	PathAppend(_userDefineLangPath, L"userDefineLang.xml");
	PathAppend(_userDefineLangsFolderPath, L"userDefineLangs");

	std::vector<generic_string> udlFiles;
	getFilesInFolder(udlFiles, L"*.xml", _userDefineLangsFolderPath);

	_pXmlUserLangDoc = new TiXmlDocument(_userDefineLangPath);
	loadOkay = _pXmlUserLangDoc->LoadFile();
	if (!loadOkay)
	{
		delete _pXmlUserLangDoc;
		_pXmlUserLangDoc = nullptr;
		isAllLaoded = false;
	}
	else
	{
		auto r = addUserDefineLangsFromXmlTree(_pXmlUserLangDoc);
		if (r.second - r.first > 0)
			_pXmlUserLangsDoc.push_back(UdlXmlFileState(_pXmlUserLangDoc, false, r));
	}

	for (const auto& i : udlFiles)
	{
		auto udlDoc = new TiXmlDocument(i);
		loadOkay = udlDoc->LoadFile();
		if (!loadOkay)
		{
			delete udlDoc;
		}
		else
		{
			auto r = addUserDefineLangsFromXmlTree(udlDoc);
			if (r.second - r.first > 0)
				_pXmlUserLangsDoc.push_back(UdlXmlFileState(udlDoc, false, r));
		}
	}

	//----------------------------------------------//
	// nativeLang.xml : for per user				//
	// In case of absence of user's nativeLang.xml, //
	// We'll look in the Notepad++ Dir.			 //
	//----------------------------------------------//

	generic_string nativeLangPath;
	nativeLangPath = _userPath;
	PathAppend(nativeLangPath, L"nativeLang.xml");

	// LocalizationSwitcher should use always user path
	_localizationSwitcher._nativeLangPath = nativeLangPath;

	if (not _startWithLocFileName.empty()) // localization argument detected, use user wished localization
	{
		// overwrite nativeLangPath variable
		nativeLangPath = _nppPath;
		PathAppend(nativeLangPath, L"localization\\");
		PathAppend(nativeLangPath, _startWithLocFileName);
	}
	else // use %appdata% location, or (if absence then) npp installed location
	{
		if (!PathFileExists(nativeLangPath.c_str()))
		{
			nativeLangPath = _nppPath;
			PathAppend(nativeLangPath, L"nativeLang.xml");
		}
	}


	_pXmlNativeLangDocA = new TiXmlDocumentA();

	loadOkay = _pXmlNativeLangDocA->LoadUnicodeFilePath(nativeLangPath.c_str());
	if (!loadOkay)
	{
		delete _pXmlNativeLangDocA;
		_pXmlNativeLangDocA = nullptr;
		isAllLaoded = false;
	}

	//---------------------------------//
	// toolbarIcons.xml : for per user //
	//---------------------------------//
	generic_string toolbarIconsPath(_userPath);
	PathAppend(toolbarIconsPath, L"toolbarIcons.xml");

	_pXmlToolIconsDoc = new TiXmlDocument(toolbarIconsPath);
	loadOkay = _pXmlToolIconsDoc->LoadFile();
	if (!loadOkay)
	{
		delete _pXmlToolIconsDoc;
		_pXmlToolIconsDoc = nullptr;
		isAllLaoded = false;
	}

	//------------------------------//
	// shortcuts.xml : for per user //
	//------------------------------//
	_shortcutsPath = _userPath;
	PathAppend(_shortcutsPath, L"shortcuts.xml");

	if (!PathFileExists(_shortcutsPath.c_str()))
	{
		generic_string srcShortcutsPath(_nppPath);
		PathAppend(srcShortcutsPath, L"shortcuts.xml");

		::CopyFile(srcShortcutsPath.c_str(), _shortcutsPath.c_str(), TRUE);
	}

	_pXmlShortcutDoc = new TiXmlDocument(_shortcutsPath);
	loadOkay = _pXmlShortcutDoc->LoadFile();
	if (!loadOkay)
	{
		delete _pXmlShortcutDoc;
		_pXmlShortcutDoc = nullptr;
		isAllLaoded = false;
	}
	else
	{
		getShortcutsFromXmlTree();
		getMacrosFromXmlTree();
		getUserCmdsFromXmlTree();

		// fill out _scintillaModifiedKeys :
		// those user defined Scintilla key will be used remap Scintilla Key Array
		getScintKeysFromXmlTree();
	}

	//---------------------------------//
	// contextMenu.xml : for per user //
	//---------------------------------//
	_contextMenuPath = _userPath;
	PathAppend(_contextMenuPath, L"contextMenu.xml");

	if (!PathFileExists(_contextMenuPath.c_str()))
	{
		generic_string srcContextMenuPath(_nppPath);
		PathAppend(srcContextMenuPath, L"contextMenu.xml");

		::CopyFile(srcContextMenuPath.c_str(), _contextMenuPath.c_str(), TRUE);
	}

	_pXmlContextMenuDocA = new TiXmlDocumentA();
	loadOkay = _pXmlContextMenuDocA->LoadUnicodeFilePath(_contextMenuPath.c_str());
	if (!loadOkay)
	{
		delete _pXmlContextMenuDocA;
		_pXmlContextMenuDocA = nullptr;
		isAllLaoded = false;
	}

	//----------------------------//
	// session.xml : for per user //
	//----------------------------//

	PathAppend(_sessionPath, L"session.xml");

	// Don't load session.xml if not required in order to speed up!!
	const NppGUI & nppGUI = (NppParameters::getInstance()).getNppGUI();
	if (nppGUI._rememberLastSession)
	{
		_pXmlSessionDoc = new TiXmlDocument(_sessionPath);

		loadOkay = _pXmlSessionDoc->LoadFile();
		if (!loadOkay)
			isAllLaoded = false;
		else
			getSessionFromXmlTree();

		delete _pXmlSessionDoc;
		for (size_t i = 0, len = _pXmlExternalLexerDoc.size() ; i < len ; ++i)
			if (_pXmlExternalLexerDoc[i])
				delete _pXmlExternalLexerDoc[i];

		_pXmlSessionDoc = nullptr;
	}

	//------------------------------//
	// blacklist.xml : for per user //
	//------------------------------//
	_blacklistPath = _userPath;
	PathAppend(_blacklistPath, L"blacklist.xml");

	if (PathFileExists(_blacklistPath.c_str()))
	{
		_pXmlBlacklistDoc = new TiXmlDocument(_blacklistPath);
		loadOkay = _pXmlBlacklistDoc->LoadFile();
		if (loadOkay)
			getBlackListFromXmlTree();
	}
	return isAllLaoded;
}


void NppParameters::destroyInstance()
{
	delete _pXmlDoc;
	delete _pXmlUserDoc;
	delete _pXmlUserStylerDoc;
	
	//delete _pXmlUserLangDoc; will be deleted in the vector
	for (auto l : _pXmlUserLangsDoc)
	{
		delete l._udlXmlDoc;
	}

	delete _pXmlNativeLangDocA;
	delete _pXmlToolIconsDoc;
	delete _pXmlShortcutDoc;
	delete _pXmlContextMenuDocA;
	delete _pXmlSessionDoc;
	delete _pXmlBlacklistDoc;
	delete 	getInstancePointer();
}


void NppParameters::saveConfig_xml()
{
	if (_pXmlUserDoc)
		_pXmlUserDoc->SaveFile();
}


void NppParameters::setWorkSpaceFilePath(int i, const TCHAR* wsFile)
{
	if (i < 0 || i > 2 || !wsFile)
		return;
	_workSpaceFilePathes[i] = wsFile;
}


void NppParameters::removeTransparent(HWND hwnd)
{
	if (hwnd != NULL)
		::SetWindowLongPtr(hwnd, GWL_EXSTYLE,  ::GetWindowLongPtr(hwnd, GWL_EXSTYLE) & ~0x00080000);
}


void NppParameters::SetTransparent(HWND hwnd, int percent)
{
	if (nullptr != _transparentFuncAddr)
	{
		::SetWindowLongPtr(hwnd, GWL_EXSTYLE, ::GetWindowLongPtr(hwnd, GWL_EXSTYLE) | 0x00080000);
		if (percent > 255)
			percent = 255;
		if (percent < 0)
			percent = 0;
		_transparentFuncAddr(hwnd, 0, percent, 0x00000002);
	}
}


bool NppParameters::isExistingExternalLangName(const TCHAR *newName) const
{
	if ((!newName) || (!newName[0]))
		return true;

	for (int i = 0 ; i < _nbExternalLang ; ++i)
	{
		if (!lstrcmp(_externalLangArray[i]->_name, newName))
			return true;
	}
	return false;
}


const TCHAR* NppParameters::getUserDefinedLangNameFromExt(TCHAR *ext, TCHAR *fullName) const
{
	if ((!ext) || (!ext[0]))
		return nullptr;

	std::vector<generic_string> extVect;
	for (int i = 0 ; i < _nbUserLang ; ++i)
	{
		extVect.clear();
		cutString(_userLangArray[i]->_ext.c_str(), extVect);

		for (size_t j = 0, len = extVect.size(); j < len; ++j)
		{
			if (!generic_stricmp(extVect[j].c_str(), ext) || (_tcschr(fullName, '.') && !generic_stricmp(extVect[j].c_str(), fullName)))
				return _userLangArray[i]->_name.c_str();
		}
	}
	return nullptr;
}


int NppParameters::getExternalLangIndexFromName(const TCHAR* externalLangName) const
{
	for (int i = 0 ; i < _nbExternalLang ; ++i)
	{
		if (!lstrcmp(externalLangName, _externalLangArray[i]->_name))
			return i;
	}
	return -1;
}


UserLangContainer* NppParameters::getULCFromName(const TCHAR *userLangName)
{
	for (int i = 0 ; i < _nbUserLang ; ++i)
	{
		if (0 == lstrcmp(userLangName, _userLangArray[i]->_name.c_str()))
			return _userLangArray[i];
	}

	//qui doit etre jamais passer
	return nullptr;
}


COLORREF NppParameters::getCurLineHilitingColour()
{
	int i = _widgetStyleArray.getStylerIndexByName(L"Current line background colour");
	if (i == -1)
		return i;
	Style & style = _widgetStyleArray.getStyler(i);
	return style._bgColor;
}


void NppParameters::setCurLineHilitingColour(COLORREF colour2Set)
{
	int i = _widgetStyleArray.getStylerIndexByName(L"Current line background colour");
	if (i == -1)
		return;

	Style& style = _widgetStyleArray.getStyler(i);
	style._bgColor = colour2Set;
}



static int CALLBACK EnumFontFamExProc(const LOGFONT* lpelfe, const TEXTMETRIC*, DWORD, LPARAM lParam)
{
	std::vector<generic_string>& strVect = *(std::vector<generic_string> *)lParam;
	const int32_t vectSize = static_cast<int32_t>(strVect.size());
	const TCHAR* lfFaceName = ((ENUMLOGFONTEX*)lpelfe)->elfLogFont.lfFaceName;

	//Search through all the fonts, EnumFontFamiliesEx never states anything about order
	//Start at the end though, that's the most likely place to find a duplicate
	for (int i = vectSize - 1 ; i >= 0 ; i--)
	{
		if (0 == lstrcmp(strVect[i].c_str(), lfFaceName))
			return 1;	//we already have seen this typeface, ignore it
	}

	//We can add the font
	//Add the face name and not the full name, we do not care about any styles
	strVect.push_back(lfFaceName);
	return 1; // I want to get all fonts
}


void NppParameters::setFontList(HWND hWnd)
{
	//---------------//
	// Sys font list //
	//---------------//
	LOGFONT lf;
	_fontlist.clear();
	_fontlist.reserve(64); // arbitrary
	_fontlist.push_back(generic_string());

	lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfFaceName[0]='\0';
	lf.lfPitchAndFamily = 0;
	HDC hDC = ::GetDC(hWnd);
	::EnumFontFamiliesEx(hDC, &lf, EnumFontFamExProc, reinterpret_cast<LPARAM>(&_fontlist), 0);
}

bool NppParameters::isInFontList(const generic_string& fontName2Search) const
{
	if (fontName2Search.empty())
		return false;

	for (size_t i = 0, len = _fontlist.size(); i < len; i++)
	{
		if (_fontlist[i] == fontName2Search)
			return true;
	}
	return false;
}

void NppParameters::getLangKeywordsFromXmlTree()
{
	TiXmlNode *root =
		_pXmlDoc->FirstChild(L"NotepadPlus");
		if (!root) return;
	feedKeyWordsParameters(root);
}


void NppParameters::getExternalLexerFromXmlTree(TiXmlDocument *doc)
{
	TiXmlNode *root = doc->FirstChild(L"NotepadPlus");
		if (!root) return;
	feedKeyWordsParameters(root);
	feedStylerArray(root);
}


int NppParameters::addExternalLangToEnd(ExternalLangContainer * externalLang)
{
	_externalLangArray[_nbExternalLang] = externalLang;
	++_nbExternalLang;
	++L_END;
	return _nbExternalLang-1;
}


bool NppParameters::getUserStylersFromXmlTree()
{
	TiXmlNode *root = _pXmlUserStylerDoc->FirstChild(L"NotepadPlus");
		if (!root) return false;
	return feedStylerArray(root);
}


bool NppParameters::getUserParametersFromXmlTree()
{
	if (!_pXmlUserDoc)
		return false;

	TiXmlNode *root = _pXmlUserDoc->FirstChild(L"NotepadPlus");
	if (not root)
		return false;

	// Get GUI parameters
	feedGUIParameters(root);

	// Get History parameters
	feedFileListParameters(root);

	// Erase the History root
	TiXmlNode *node = root->FirstChildElement(L"History");
	root->RemoveChild(node);

	// Add a new empty History root
	TiXmlElement HistoryNode(L"History");
	root->InsertEndChild(HistoryNode);

	//Get Find history parameters
	feedFindHistoryParameters(root);

	//Get Project Panel parameters
	feedProjectPanelsParameters(root);

	//Get File browser parameters
	feedFileBrowserParameters(root);

	return true;
}


std::pair<unsigned char, unsigned char> NppParameters::addUserDefineLangsFromXmlTree(TiXmlDocument *tixmldoc)
{
	if (!tixmldoc)
		return std::make_pair(static_cast<unsigned char>(0), static_cast<unsigned char>(0));

	TiXmlNode *root = tixmldoc->FirstChild(L"NotepadPlus");
	if (!root)
		return std::make_pair(static_cast<unsigned char>(0), static_cast<unsigned char>(0));

	return feedUserLang(root);
}



bool NppParameters::getShortcutsFromXmlTree()
{
	if (!_pXmlShortcutDoc)
		return false;

	TiXmlNode *root = _pXmlShortcutDoc->FirstChild(L"NotepadPlus");
	if (!root)
		return false;

	feedShortcut(root);
	return true;
}


bool NppParameters::getMacrosFromXmlTree()
{
	if (!_pXmlShortcutDoc)
		return false;

	TiXmlNode *root = _pXmlShortcutDoc->FirstChild(L"NotepadPlus");
	if (!root)
		return false;

	feedMacros(root);
	return true;
}


bool NppParameters::getUserCmdsFromXmlTree()
{
	if (!_pXmlShortcutDoc)
		return false;

	TiXmlNode *root = _pXmlShortcutDoc->FirstChild(L"NotepadPlus");
	if (!root)
		return false;

	feedUserCmds(root);
	return true;
}


bool NppParameters::getPluginCmdsFromXmlTree()
{
	if (!_pXmlShortcutDoc)
		return false;

	TiXmlNode *root = _pXmlShortcutDoc->FirstChild(L"NotepadPlus");
	if (!root)
		return false;

	feedPluginCustomizedCmds(root);
	return true;
}


bool NppParameters::getScintKeysFromXmlTree()
{
	if (!_pXmlShortcutDoc)
		return false;

	TiXmlNode *root = _pXmlShortcutDoc->FirstChild(L"NotepadPlus");
	if (!root)
		return false;

	feedScintKeys(root);
	return true;
}

bool NppParameters::getBlackListFromXmlTree()
{
	if (!_pXmlBlacklistDoc)
		return false;

	TiXmlNode *root = _pXmlBlacklistDoc->FirstChild(L"NotepadPlus");
	if (!root)
		return false;

	return feedBlacklist(root);
}

void NppParameters::initMenuKeys()
{
	int nbCommands = sizeof(winKeyDefs)/sizeof(WinMenuKeyDefinition);
	WinMenuKeyDefinition wkd;
	for (int i = 0; i < nbCommands; ++i)
	{
		wkd = winKeyDefs[i];
		Shortcut sc((wkd.specialName ? wkd.specialName : L""), wkd.isCtrl, wkd.isAlt, wkd.isShift, static_cast<unsigned char>(wkd.vKey));
		_shortcuts.push_back( CommandShortcut(sc, wkd.functionId) );
	}
}

void NppParameters::initScintillaKeys()
{
	int nbCommands = sizeof(scintKeyDefs)/sizeof(ScintillaKeyDefinition);

	//Warning! Matching function have to be consecutive
	ScintillaKeyDefinition skd;
	int prevIndex = -1;
	int prevID = -1;
	for (int i = 0; i < nbCommands; ++i)
	{
		skd = scintKeyDefs[i];
		if (skd.functionId == prevID)
		{
			KeyCombo kc;
			kc._isCtrl = skd.isCtrl;
			kc._isAlt = skd.isAlt;
			kc._isShift = skd.isShift;
			kc._key = static_cast<unsigned char>(skd.vKey);
			_scintillaKeyCommands[prevIndex].addKeyCombo(kc);
		}
		else
		{
			Shortcut s = Shortcut(skd.name, skd.isCtrl, skd.isAlt, skd.isShift, static_cast<unsigned char>(skd.vKey));
			ScintillaKeyMap sm = ScintillaKeyMap(s, skd.functionId, skd.redirFunctionId);
			_scintillaKeyCommands.push_back(sm);
			++prevIndex;
		}
		prevID = skd.functionId;
	}
}
bool NppParameters::reloadContextMenuFromXmlTree(HMENU mainMenuHadle, HMENU pluginsMenu)
{
	_contextMenuItems.clear();
	return getContextMenuFromXmlTree(mainMenuHadle, pluginsMenu);
}

int NppParameters::getCmdIdFromMenuEntryItemName(HMENU mainMenuHadle, const generic_string& menuEntryName, const generic_string& menuItemName)
{
	int nbMenuEntry = ::GetMenuItemCount(mainMenuHadle);
	for (int i = 0; i < nbMenuEntry; ++i)
	{
		TCHAR menuEntryString[64];
		::GetMenuString(mainMenuHadle, i, menuEntryString, 64, MF_BYPOSITION);
		if (generic_stricmp(menuEntryName.c_str(), purgeMenuItemString(menuEntryString).c_str()) == 0)
		{
			vector< pair<HMENU, int> > parentMenuPos;
			HMENU topMenu = ::GetSubMenu(mainMenuHadle, i);
			int maxTopMenuPos = ::GetMenuItemCount(topMenu);
			HMENU currMenu = topMenu;
			int currMaxMenuPos = maxTopMenuPos;

			int currMenuPos = 0;
			bool notFound = false;

			do {
				if (::GetSubMenu(currMenu, currMenuPos))
				{
					//  Go into sub menu
					parentMenuPos.push_back(::make_pair(currMenu, currMenuPos));
					currMenu = ::GetSubMenu(currMenu, currMenuPos);
					currMenuPos = 0;
					currMaxMenuPos = ::GetMenuItemCount(currMenu);
				}
				else
				{
					//  Check current menu position.
					TCHAR cmdStr[256];
					::GetMenuString(currMenu, currMenuPos, cmdStr, 256, MF_BYPOSITION);
					if (generic_stricmp(menuItemName.c_str(), purgeMenuItemString(cmdStr).c_str()) == 0)
					{
						return ::GetMenuItemID(currMenu, currMenuPos);
					}

					if ((currMenuPos >= currMaxMenuPos) && (parentMenuPos.size() > 0))
					{
						currMenu = parentMenuPos.back().first;
						currMenuPos = parentMenuPos.back().second;
						parentMenuPos.pop_back();
						currMaxMenuPos = ::GetMenuItemCount(currMenu);
					}

					if ((currMenu == topMenu) && (currMenuPos >= maxTopMenuPos))
					{
						notFound = true;
					}
					else
					{
						++currMenuPos;
					}
				}
			} while (!notFound);
		}
	}
	return -1;
}

int NppParameters::getPluginCmdIdFromMenuEntryItemName(HMENU pluginsMenu, const generic_string& pluginName, const generic_string& pluginCmdName)
{
	int nbPlugins = ::GetMenuItemCount(pluginsMenu);
	for (int i = 0; i < nbPlugins; ++i)
	{
		TCHAR menuItemString[256];
		::GetMenuString(pluginsMenu, i, menuItemString, 256, MF_BYPOSITION);
		if (generic_stricmp(pluginName.c_str(), purgeMenuItemString(menuItemString).c_str()) == 0)
		{
			HMENU pluginMenu = ::GetSubMenu(pluginsMenu, i);
			int nbPluginCmd = ::GetMenuItemCount(pluginMenu);
			for (int j = 0; j < nbPluginCmd; ++j)
			{
				TCHAR pluginCmdStr[256];
				::GetMenuString(pluginMenu, j, pluginCmdStr, 256, MF_BYPOSITION);
				if (generic_stricmp(pluginCmdName.c_str(), purgeMenuItemString(pluginCmdStr).c_str()) == 0)
				{
					return ::GetMenuItemID(pluginMenu, j);
				}
			}
		}
	}
	return -1;
}

bool NppParameters::getContextMenuFromXmlTree(HMENU mainMenuHadle, HMENU pluginsMenu)
{
	if (!_pXmlContextMenuDocA)
		return false;
	TiXmlNodeA *root = _pXmlContextMenuDocA->FirstChild("NotepadPlus");
	if (!root)
		return false;

	WcharMbcsConvertor& wmc = WcharMbcsConvertor::getInstance();

	TiXmlNodeA *contextMenuRoot = root->FirstChildElement("ScintillaContextMenu");
	if (contextMenuRoot)
	{
		for (TiXmlNodeA *childNode = contextMenuRoot->FirstChildElement("Item");
			childNode ;
			childNode = childNode->NextSibling("Item") )
		{
			const char *folderNameA = (childNode->ToElement())->Attribute("FolderName");
			const char *displayAsA = (childNode->ToElement())->Attribute("ItemNameAs");

			generic_string folderName;
			generic_string displayAs;
			folderName = folderNameA?wmc.char2wchar(folderNameA, SC_CP_UTF8):L"";
			displayAs = displayAsA?wmc.char2wchar(displayAsA, SC_CP_UTF8):L"";

			int id;
			const char *idStr = (childNode->ToElement())->Attribute("id", &id);
			if (idStr)
			{
				_contextMenuItems.push_back(MenuItemUnit(id, displayAs.c_str(), folderName.c_str()));
			}
			else
			{
				const char *menuEntryNameA = (childNode->ToElement())->Attribute("MenuEntryName");
				const char *menuItemNameA = (childNode->ToElement())->Attribute("MenuItemName");

				generic_string menuEntryName;
				generic_string menuItemName;
				menuEntryName = menuEntryNameA?wmc.char2wchar(menuEntryNameA, SC_CP_UTF8):L"";
				menuItemName = menuItemNameA?wmc.char2wchar(menuItemNameA, SC_CP_UTF8):L"";

				if (not menuEntryName.empty() and not menuItemName.empty())
				{
					int cmd = getCmdIdFromMenuEntryItemName(mainMenuHadle, menuEntryName, menuItemName);
					if (cmd != -1)
						_contextMenuItems.push_back(MenuItemUnit(cmd, displayAs.c_str(), folderName.c_str()));
				}
				else
				{
					const char *pluginNameA = (childNode->ToElement())->Attribute("PluginEntryName");
					const char *pluginCmdNameA = (childNode->ToElement())->Attribute("PluginCommandItemName");

					generic_string pluginName;
					generic_string pluginCmdName;
					pluginName = pluginNameA?wmc.char2wchar(pluginNameA, SC_CP_UTF8):L"";
					pluginCmdName = pluginCmdNameA?wmc.char2wchar(pluginCmdNameA, SC_CP_UTF8):L"";

					// if plugin menu existing plls the value of PluginEntryName and PluginCommandItemName are valid
					if (pluginsMenu && not pluginName.empty() && not pluginCmdName.empty())
					{
						int pluginCmdId = getPluginCmdIdFromMenuEntryItemName(pluginsMenu, pluginName, pluginCmdName);
						if (pluginCmdId != -1)
							_contextMenuItems.push_back(MenuItemUnit(pluginCmdId, displayAs.c_str(), folderName.c_str()));
					}
				}
			}
		}
	}
	return true;
}


void NppParameters::setWorkingDir(const TCHAR * newPath)
{
	if (newPath && newPath[0])
	{
		_currentDirectory = newPath;
	}
	else
	{
		if (PathFileExists(_nppGUI._defaultDirExp))
			_currentDirectory = _nppGUI._defaultDirExp;
		else
			_currentDirectory = _nppPath.c_str();
	}
}


bool NppParameters::loadSession(Session & session, const TCHAR *sessionFileName)
{
	TiXmlDocument *pXmlSessionDocument = new TiXmlDocument(sessionFileName);
	bool loadOkay = pXmlSessionDocument->LoadFile();
	if (loadOkay)
		loadOkay = getSessionFromXmlTree(pXmlSessionDocument, &session);

	delete pXmlSessionDocument;
	return loadOkay;
}


bool NppParameters::getSessionFromXmlTree(TiXmlDocument *pSessionDoc, Session *pSession)
{
	if ((pSessionDoc) && (!pSession))
		return false;

	TiXmlDocument **ppSessionDoc = &_pXmlSessionDoc;
	Session *ptrSession = &_session;

	if (pSessionDoc)
	{
		ppSessionDoc = &pSessionDoc;
		ptrSession = pSession;
	}

	if (!*ppSessionDoc)
		return false;

	TiXmlNode *root = (*ppSessionDoc)->FirstChild(L"NotepadPlus");
	if (!root)
		return false;

	TiXmlNode *sessionRoot = root->FirstChildElement(L"Session");
	if (!sessionRoot)
		return false;

	TiXmlElement *actView = sessionRoot->ToElement();
	int index = 0;
	const TCHAR *str = actView->Attribute(L"activeView", &index);
	if (str)
	{
		(*ptrSession)._activeView = index;
	}

	const size_t nbView = 2;
	TiXmlNode *viewRoots[nbView];
	viewRoots[0] = sessionRoot->FirstChildElement(L"mainView");
	viewRoots[1] = sessionRoot->FirstChildElement(L"subView");
	for (size_t k = 0; k < nbView; ++k)
	{
		if (viewRoots[k])
		{
			int index2 = 0;
			TiXmlElement *actIndex = viewRoots[k]->ToElement();
			str = actIndex->Attribute(L"activeIndex", &index2);
			if (str)
			{
				if (k == 0)
					(*ptrSession)._activeMainIndex = index2;
				else // k == 1
					(*ptrSession)._activeSubIndex = index2;
			}
			for (TiXmlNode *childNode = viewRoots[k]->FirstChildElement(L"File");
				childNode ;
				childNode = childNode->NextSibling(L"File") )
			{
				const TCHAR *fileName = (childNode->ToElement())->Attribute(L"filename");
				if (fileName)
				{
					Position position;
					(childNode->ToElement())->Attribute(L"firstVisibleLine", &position._firstVisibleLine);
					(childNode->ToElement())->Attribute(L"xOffset", &position._xOffset);
					(childNode->ToElement())->Attribute(L"startPos", &position._startPos);
					(childNode->ToElement())->Attribute(L"endPos", &position._endPos);
					(childNode->ToElement())->Attribute(L"selMode", &position._selMode);
					(childNode->ToElement())->Attribute(L"scrollWidth", &position._scrollWidth);
					(childNode->ToElement())->Attribute(L"offset", &position._offset);
					(childNode->ToElement())->Attribute(L"wrapCount", &position._wrapCount);
					MapPosition mapPosition;
					int32_t mapPosVal;
					const TCHAR *mapPosStr = (childNode->ToElement())->Attribute(L"mapFirstVisibleDisplayLine", &mapPosVal);
					if (mapPosStr)
						mapPosition._firstVisibleDisplayLine = mapPosVal;
					mapPosStr = (childNode->ToElement())->Attribute(L"mapFirstVisibleDocLine", &mapPosVal);
					if (mapPosStr)
						mapPosition._firstVisibleDocLine = mapPosVal;
					mapPosStr = (childNode->ToElement())->Attribute(L"mapLastVisibleDocLine", &mapPosVal);
					if (mapPosStr)
						mapPosition._lastVisibleDocLine = mapPosVal;
					mapPosStr = (childNode->ToElement())->Attribute(L"mapNbLine", &mapPosVal);
					if (mapPosStr)
						mapPosition._nbLine = mapPosVal;
					mapPosStr = (childNode->ToElement())->Attribute(L"mapHigherPos", &mapPosVal);
					if (mapPosStr)
						mapPosition._higherPos = mapPosVal;
					mapPosStr = (childNode->ToElement())->Attribute(L"mapWidth", &mapPosVal);
					if (mapPosStr)
						mapPosition._width = mapPosVal;
					mapPosStr = (childNode->ToElement())->Attribute(L"mapHeight", &mapPosVal);
					if (mapPosStr)
						mapPosition._height = mapPosVal;
					mapPosStr = (childNode->ToElement())->Attribute(L"mapKByteInDoc", &mapPosVal);
					if (mapPosStr)
						mapPosition._KByteInDoc = mapPosVal;
					mapPosStr = (childNode->ToElement())->Attribute(L"mapWrapIndentMode", &mapPosVal);
					if (mapPosStr)
						mapPosition._wrapIndentMode = mapPosVal;
					const TCHAR *boolStr = (childNode->ToElement())->Attribute(L"mapIsWrap");
					if (boolStr)
						mapPosition._isWrap = (lstrcmp(L"yes", boolStr) == 0);

					const TCHAR *langName;
					langName = (childNode->ToElement())->Attribute(L"lang");
					int encoding = -1;
					const TCHAR *encStr = (childNode->ToElement())->Attribute(L"encoding", &encoding);
					const TCHAR *backupFilePath = (childNode->ToElement())->Attribute(L"backupFilePath");

					FILETIME fileModifiedTimestamp;
					(childNode->ToElement())->Attribute(L"originalFileLastModifTimestamp", reinterpret_cast<int32_t*>(&fileModifiedTimestamp.dwLowDateTime));
					(childNode->ToElement())->Attribute(L"originalFileLastModifTimestampHigh", reinterpret_cast<int32_t*>(&fileModifiedTimestamp.dwHighDateTime));

					bool isUserReadOnly = false;
					const TCHAR *boolStrReadOnly = (childNode->ToElement())->Attribute(L"userReadOnly");
					if (boolStrReadOnly)
						isUserReadOnly = _wcsicmp(L"yes", boolStrReadOnly) == 0;

					sessionFileInfo sfi(fileName, langName, encStr ? encoding : -1, isUserReadOnly, position, backupFilePath, fileModifiedTimestamp, mapPosition);

					for (TiXmlNode *markNode = childNode->FirstChildElement(L"Mark");
						markNode ;
						markNode = markNode->NextSibling(L"Mark"))
					{
						int lineNumber;
						const TCHAR *lineNumberStr = (markNode->ToElement())->Attribute(L"line", &lineNumber);
						if (lineNumberStr)
						{
							sfi._marks.push_back(lineNumber);
						}
					}

					for (TiXmlNode *foldNode = childNode->FirstChildElement(L"Fold");
						foldNode ;
						foldNode = foldNode->NextSibling(L"Fold"))
					{
						int lineNumber;
						const TCHAR *lineNumberStr = (foldNode->ToElement())->Attribute(L"line", &lineNumber);
						if (lineNumberStr)
						{
							sfi._foldStates.push_back(lineNumber);
						}
					}
					if (k == 0)
						(*ptrSession)._mainViewFiles.push_back(sfi);
					else // k == 1
						(*ptrSession)._subViewFiles.push_back(sfi);
				}
			}
		}
	}

	return true;
}

void NppParameters::feedFileListParameters(TiXmlNode *node)
{
	TiXmlNode *historyRoot = node->FirstChildElement(L"History");
	if (!historyRoot) return;

	// nbMaxFile value
	int nbMaxFile;
	const TCHAR *strVal = (historyRoot->ToElement())->Attribute(L"nbMaxFile", &nbMaxFile);
	if (strVal && (nbMaxFile >= 0) && (nbMaxFile <= 50))
		_nbMaxRecentFile = nbMaxFile;

	// customLen value
	int customLen;
	strVal = (historyRoot->ToElement())->Attribute(L"customLength", &customLen);
	if (strVal)
		_recentFileCustomLength = customLen;

	// inSubMenu value
	strVal = (historyRoot->ToElement())->Attribute(L"inSubMenu");
	if (strVal)
		_putRecentFileInSubMenu = (lstrcmp(strVal, L"yes") == 0);

	for (TiXmlNode *childNode = historyRoot->FirstChildElement(L"File");
		childNode && (_nbRecentFile < NB_MAX_LRF_FILE);
		childNode = childNode->NextSibling(L"File") )
	{
		const TCHAR *filePath = (childNode->ToElement())->Attribute(L"filename");
		if (filePath)
		{
			_LRFileList[_nbRecentFile] = new generic_string(filePath);
			++_nbRecentFile;
		}
	}
}

void NppParameters::feedProjectPanelsParameters(TiXmlNode *node)
{
	TiXmlNode *fileBrowserRoot = node->FirstChildElement(L"FileBrowser");
	if (!fileBrowserRoot) return;

	for (TiXmlNode *childNode = fileBrowserRoot->FirstChildElement(L"root");
		childNode;
		childNode = childNode->NextSibling(L"root") )
	{
		const TCHAR *filePath = (childNode->ToElement())->Attribute(L"foldername");
		if (filePath)
		{
			_fileBrowserRoot.push_back(filePath);
		}
	}
}

void NppParameters::feedFileBrowserParameters(TiXmlNode *node)
{
	TiXmlNode *projPanelRoot = node->FirstChildElement(L"ProjectPanels");
	if (!projPanelRoot) return;

	for (TiXmlNode *childNode = projPanelRoot->FirstChildElement(L"ProjectPanel");
		childNode;
		childNode = childNode->NextSibling(L"ProjectPanel") )
	{
		int index = 0;
		const TCHAR *idStr = (childNode->ToElement())->Attribute(L"id", &index);
		if (idStr && (index >= 0 && index <= 2))
		{
			const TCHAR *filePath = (childNode->ToElement())->Attribute(L"workSpaceFile");
			if (filePath)
			{
				_workSpaceFilePathes[index] = filePath;
			}
		}
	}
}

void NppParameters::feedFindHistoryParameters(TiXmlNode *node)
{
	TiXmlNode *findHistoryRoot = node->FirstChildElement(L"FindHistory");
	if (!findHistoryRoot) return;

	(findHistoryRoot->ToElement())->Attribute(L"nbMaxFindHistoryPath", &_findHistory._nbMaxFindHistoryPath);
	if ((_findHistory._nbMaxFindHistoryPath > 0) && (_findHistory._nbMaxFindHistoryPath <= NB_MAX_FINDHISTORY_PATH))
	{
		for (TiXmlNode *childNode = findHistoryRoot->FirstChildElement(L"Path");
			childNode && (_findHistory._findHistoryPaths.size() < NB_MAX_FINDHISTORY_PATH);
			childNode = childNode->NextSibling(L"Path") )
		{
			const TCHAR *filePath = (childNode->ToElement())->Attribute(L"name");
			if (filePath)
			{
				_findHistory._findHistoryPaths.push_back(generic_string(filePath));
			}
		}
	}

	(findHistoryRoot->ToElement())->Attribute(L"nbMaxFindHistoryFilter", &_findHistory._nbMaxFindHistoryFilter);
	if ((_findHistory._nbMaxFindHistoryFilter > 0) && (_findHistory._nbMaxFindHistoryFilter <= NB_MAX_FINDHISTORY_FILTER))
	{
		for (TiXmlNode *childNode = findHistoryRoot->FirstChildElement(L"Filter");
			childNode && (_findHistory._findHistoryFilters.size() < NB_MAX_FINDHISTORY_FILTER);
			childNode = childNode->NextSibling(L"Filter"))
		{
			const TCHAR *fileFilter = (childNode->ToElement())->Attribute(L"name");
			if (fileFilter)
			{
				_findHistory._findHistoryFilters.push_back(generic_string(fileFilter));
			}
		}
	}

	(findHistoryRoot->ToElement())->Attribute(L"nbMaxFindHistoryFind", &_findHistory._nbMaxFindHistoryFind);
	if ((_findHistory._nbMaxFindHistoryFind > 0) && (_findHistory._nbMaxFindHistoryFind <= NB_MAX_FINDHISTORY_FIND))
	{
		for (TiXmlNode *childNode = findHistoryRoot->FirstChildElement(L"Find");
			childNode && (_findHistory._findHistoryFinds.size() < NB_MAX_FINDHISTORY_FIND);
			childNode = childNode->NextSibling(L"Find"))
		{
			const TCHAR *fileFind = (childNode->ToElement())->Attribute(L"name");
			if (fileFind)
			{
				_findHistory._findHistoryFinds.push_back(generic_string(fileFind));
			}
		}
	}

	(findHistoryRoot->ToElement())->Attribute(L"nbMaxFindHistoryReplace", &_findHistory._nbMaxFindHistoryReplace);
	if ((_findHistory._nbMaxFindHistoryReplace > 0) && (_findHistory._nbMaxFindHistoryReplace <= NB_MAX_FINDHISTORY_REPLACE))
	{
		for (TiXmlNode *childNode = findHistoryRoot->FirstChildElement(L"Replace");
			childNode && (_findHistory._findHistoryReplaces.size() < NB_MAX_FINDHISTORY_REPLACE);
			childNode = childNode->NextSibling(L"Replace"))
		{
			const TCHAR *fileReplace = (childNode->ToElement())->Attribute(L"name");
			if (fileReplace)
			{
				_findHistory._findHistoryReplaces.push_back(generic_string(fileReplace));
			}
		}
	}

	const TCHAR *boolStr = (findHistoryRoot->ToElement())->Attribute(L"matchWord");
	if (boolStr)
		_findHistory._isMatchWord = (lstrcmp(L"yes", boolStr) == 0);

	boolStr = (findHistoryRoot->ToElement())->Attribute(L"matchCase");
	if (boolStr)
		_findHistory._isMatchCase = (lstrcmp(L"yes", boolStr) == 0);

	boolStr = (findHistoryRoot->ToElement())->Attribute(L"wrap");
	if (boolStr)
		_findHistory._isWrap = (lstrcmp(L"yes", boolStr) == 0);

	boolStr = (findHistoryRoot->ToElement())->Attribute(L"directionDown");
	if (boolStr)
		_findHistory._isDirectionDown = (lstrcmp(L"yes", boolStr) == 0);

	boolStr = (findHistoryRoot->ToElement())->Attribute(L"fifRecuisive");
	if (boolStr)
		_findHistory._isFifRecuisive = (lstrcmp(L"yes", boolStr) == 0);

	boolStr = (findHistoryRoot->ToElement())->Attribute(L"fifInHiddenFolder");
	if (boolStr)
		_findHistory._isFifInHiddenFolder = (lstrcmp(L"yes", boolStr) == 0);

	boolStr = (findHistoryRoot->ToElement())->Attribute(L"dlgAlwaysVisible");
	if (boolStr)
		_findHistory._isDlgAlwaysVisible = (lstrcmp(L"yes", boolStr) == 0);

	boolStr = (findHistoryRoot->ToElement())->Attribute(L"fifFilterFollowsDoc");
	if (boolStr)
		_findHistory._isFilterFollowDoc = (lstrcmp(L"yes", boolStr) == 0);

	boolStr = (findHistoryRoot->ToElement())->Attribute(L"fifFolderFollowsDoc");
	if (boolStr)
		_findHistory._isFolderFollowDoc = (lstrcmp(L"yes", boolStr) == 0);

	int mode = 0;
	boolStr = (findHistoryRoot->ToElement())->Attribute(L"searchMode", &mode);
	if (boolStr)
		_findHistory._searchMode = (FindHistory::searchMode)mode;

	boolStr = (findHistoryRoot->ToElement())->Attribute(L"transparencyMode", &mode);
	if (boolStr)
		_findHistory._transparencyMode = (FindHistory::transparencyMode)mode;

	(findHistoryRoot->ToElement())->Attribute(L"transparency", &_findHistory._transparency);
	if (_findHistory._transparency <= 0 || _findHistory._transparency > 200)
		_findHistory._transparency = 150;

	boolStr = (findHistoryRoot->ToElement())->Attribute(L"dotMatchesNewline");
	if (boolStr)
		_findHistory._dotMatchesNewline = (lstrcmp(L"yes", boolStr) == 0);

	boolStr = (findHistoryRoot->ToElement())->Attribute(L"isSearch2ButtonsMode");
	if (boolStr)
		_findHistory._isSearch2ButtonsMode = (lstrcmp(L"yes", boolStr) == 0);
}

void NppParameters::feedShortcut(TiXmlNode *node)
{
	TiXmlNode *shortcutsRoot = node->FirstChildElement(L"InternalCommands");
	if (!shortcutsRoot) return;

	for (TiXmlNode *childNode = shortcutsRoot->FirstChildElement(L"Shortcut");
		childNode ;
		childNode = childNode->NextSibling(L"Shortcut") )
	{
		int id;
		const TCHAR *idStr = (childNode->ToElement())->Attribute(L"id", &id);
		if (idStr)
		{
			//find the commandid that matches this Shortcut sc and alter it, push back its index in the modified list, if not present
			size_t len = _shortcuts.size();
			for (size_t i = 0; i < len; ++i)
			{
				if (_shortcuts[i].getID() == (unsigned long)id)
				{	//found our match
					getShortcuts(childNode, _shortcuts[i]);
					addUserModifiedIndex(i);
				}
			}
		}
	}
}

void NppParameters::feedMacros(TiXmlNode *node)
{
	TiXmlNode *macrosRoot = node->FirstChildElement(L"Macros");
	if (!macrosRoot) return;

	for (TiXmlNode *childNode = macrosRoot->FirstChildElement(L"Macro");
		childNode ;
		childNode = childNode->NextSibling(L"Macro") )
	{
		Shortcut sc;
		if (getShortcuts(childNode, sc))// && sc.isValid())
		{
			Macro macro;
			getActions(childNode, macro);
			int cmdID = ID_MACRO + static_cast<int32_t>(_macros.size());
			MacroShortcut ms(sc, macro, cmdID);
			_macros.push_back(ms);
		}
	}
}


void NppParameters::getActions(TiXmlNode *node, Macro & macro)
{
	for (TiXmlNode *childNode = node->FirstChildElement(L"Action");
		childNode ;
		childNode = childNode->NextSibling(L"Action") )
	{
		int type;
		const TCHAR *typeStr = (childNode->ToElement())->Attribute(L"type", &type);
		if ((!typeStr) || (type > 3))
			continue;

		int msg = 0;
		(childNode->ToElement())->Attribute(L"message", &msg);

		int wParam = 0;
		(childNode->ToElement())->Attribute(L"wParam", &wParam);

		int lParam = 0;
		(childNode->ToElement())->Attribute(L"lParam", &lParam);

		const TCHAR *sParam = (childNode->ToElement())->Attribute(L"sParam");
		if (!sParam)
			sParam = L"";
		recordedMacroStep step(msg, wParam, lParam, sParam, type);
		if (step.isValid())
			macro.push_back(step);

	}
}

void NppParameters::feedUserCmds(TiXmlNode *node)
{
	TiXmlNode *userCmdsRoot = node->FirstChildElement(L"UserDefinedCommands");
	if (!userCmdsRoot) return;

	for (TiXmlNode *childNode = userCmdsRoot->FirstChildElement(L"Command");
		childNode ;
		childNode = childNode->NextSibling(L"Command") )
	{
		Shortcut sc;
		if (getShortcuts(childNode, sc))
		{
			TiXmlNode *aNode = childNode->FirstChild();
			if (aNode)
			{
				const TCHAR *cmdStr = aNode->Value();
				if (cmdStr)
				{
					int cmdID = ID_USER_CMD + static_cast<int32_t>(_userCommands.size());
					UserCommand uc(sc, cmdStr, cmdID);
					_userCommands.push_back(uc);
				}
			}
		}
	}
}

void NppParameters::feedPluginCustomizedCmds(TiXmlNode *node)
{
	TiXmlNode *pluginCustomizedCmdsRoot = node->FirstChildElement(L"PluginCommands");
	if (!pluginCustomizedCmdsRoot) return;

	for (TiXmlNode *childNode = pluginCustomizedCmdsRoot->FirstChildElement(L"PluginCommand");
		childNode ;
		childNode = childNode->NextSibling(L"PluginCommand") )
	{
		const TCHAR *moduleName = (childNode->ToElement())->Attribute(L"moduleName");
		if (!moduleName)
			continue;

		int internalID = -1;
		const TCHAR *internalIDStr = (childNode->ToElement())->Attribute(L"internalID", &internalID);

		if (!internalIDStr)
			continue;

		//Find the corresponding plugincommand and alter it, put the index in the list
		size_t len = _pluginCommands.size();
		for (size_t i = 0; i < len; ++i)
		{
			PluginCmdShortcut & pscOrig = _pluginCommands[i];
			if (!generic_strnicmp(pscOrig.getModuleName(), moduleName, lstrlen(moduleName)) && pscOrig.getInternalID() == internalID)
			{
				//Found matching command
				getShortcuts(childNode, _pluginCommands[i]);
				addPluginModifiedIndex(i);
				break;
			}
		}
	}
}

void NppParameters::feedScintKeys(TiXmlNode *node)
{
	TiXmlNode *scintKeysRoot = node->FirstChildElement(L"ScintillaKeys");
	if (!scintKeysRoot) return;

	for (TiXmlNode *childNode = scintKeysRoot->FirstChildElement(L"ScintKey");
		childNode ;
		childNode = childNode->NextSibling(L"ScintKey") )
	{
		int scintKey;
		const TCHAR *keyStr = (childNode->ToElement())->Attribute(L"ScintID", &scintKey);
		if (!keyStr)
			continue;

		int menuID;
		keyStr = (childNode->ToElement())->Attribute(L"menuCmdID", &menuID);
		if (!keyStr)
			continue;

		//Find the corresponding scintillacommand and alter it, put the index in the list
		size_t len = _scintillaKeyCommands.size();
		for (int32_t i = 0; i < static_cast<int32_t>(len); ++i)
		{
			ScintillaKeyMap & skmOrig = _scintillaKeyCommands[i];
			if (skmOrig.getScintillaKeyID() == (unsigned long)scintKey && skmOrig.getMenuCmdID() == menuID)
			{
				//Found matching command
				_scintillaKeyCommands[i].clearDups();
				getShortcuts(childNode, _scintillaKeyCommands[i]);
				_scintillaKeyCommands[i].setKeyComboByIndex(0, _scintillaKeyCommands[i].getKeyCombo());
				addScintillaModifiedIndex(i);
				KeyCombo kc;
				for (TiXmlNode *nextNode = childNode->FirstChildElement(L"NextKey");
					nextNode ;
					nextNode = nextNode->NextSibling(L"NextKey"))
				{
					const TCHAR *str = (nextNode->ToElement())->Attribute(L"Ctrl");
					if (!str)
						continue;
					kc._isCtrl = (lstrcmp(L"yes", str) == 0);

					str = (nextNode->ToElement())->Attribute(L"Alt");
					if (!str)
						continue;
					kc._isAlt = (lstrcmp(L"yes", str) == 0);

					str = (nextNode->ToElement())->Attribute(L"Shift");
					if (!str)
						continue;
					kc._isShift = (lstrcmp(L"yes", str) == 0);

					int key;
					str = (nextNode->ToElement())->Attribute(L"Key", &key);
					if (!str)
						continue;
					kc._key = static_cast<unsigned char>(key);
					_scintillaKeyCommands[i].addKeyCombo(kc);
				}
				break;
			}
		}
	}
}

bool NppParameters::feedBlacklist(TiXmlNode *node)
{
	TiXmlNode *blackListRoot = node->FirstChildElement(L"PluginBlackList");
	if (!blackListRoot) return false;

	for (TiXmlNode *childNode = blackListRoot->FirstChildElement(L"Plugin");
		childNode ;
		childNode = childNode->NextSibling(L"Plugin") )
	{
		const TCHAR *name = (childNode->ToElement())->Attribute(L"name");
		if (name)
		{
			_blacklist.push_back(name);
		}
	}
	return true;
}

bool NppParameters::getShortcuts(TiXmlNode *node, Shortcut & sc)
{
	if (!node) return false;

	const TCHAR *name = (node->ToElement())->Attribute(L"name");
	if (!name)
		name = L"";

	bool isCtrl = false;
	const TCHAR *isCtrlStr = (node->ToElement())->Attribute(L"Ctrl");
	if (isCtrlStr)
		isCtrl = (lstrcmp(L"yes", isCtrlStr) == 0);

	bool isAlt = false;
	const TCHAR *isAltStr = (node->ToElement())->Attribute(L"Alt");
	if (isAltStr)
		isAlt = (lstrcmp(L"yes", isAltStr) == 0);

	bool isShift = false;
	const TCHAR *isShiftStr = (node->ToElement())->Attribute(L"Shift");
	if (isShiftStr)
		isShift = (lstrcmp(L"yes", isShiftStr) == 0);

	int key;
	const TCHAR *keyStr = (node->ToElement())->Attribute(L"Key", &key);
	if (!keyStr)
		return false;

	sc = Shortcut(name, isCtrl, isAlt, isShift, static_cast<unsigned char>(key));
	return true;
}


std::pair<unsigned char, unsigned char> NppParameters::feedUserLang(TiXmlNode *node)
{
	int iBegin = _nbUserLang;

	for (TiXmlNode *childNode = node->FirstChildElement(L"UserLang");
		childNode && (_nbUserLang < NB_MAX_USER_LANG);
		childNode = childNode->NextSibling(L"UserLang") )
	{
		const TCHAR *name = (childNode->ToElement())->Attribute(L"name");
		const TCHAR *ext = (childNode->ToElement())->Attribute(L"ext");
		const TCHAR *udlVersion = (childNode->ToElement())->Attribute(L"udlVersion");

		if (!name || !name[0] || !ext)
		{
			// UserLang name is missing, just ignore this entry
			continue;
		}

		try {
			if (!udlVersion)
				_userLangArray[_nbUserLang] = new UserLangContainer(name, ext, L"");
			else
				_userLangArray[_nbUserLang] = new UserLangContainer(name, ext, udlVersion);
			++_nbUserLang;

			TiXmlNode *settingsRoot = childNode->FirstChildElement(L"Settings");
			if (!settingsRoot)
				throw std::runtime_error("NppParameters::feedUserLang : Settings node is missing");

			feedUserSettings(settingsRoot);

			TiXmlNode *keywordListsRoot = childNode->FirstChildElement(L"KeywordLists");
			if (!keywordListsRoot)
				throw std::runtime_error("NppParameters::feedUserLang : KeywordLists node is missing");

			feedUserKeywordList(keywordListsRoot);

			TiXmlNode *stylesRoot = childNode->FirstChildElement(L"Styles");
			if (!stylesRoot)
				throw std::runtime_error("NppParameters::feedUserLang : Styles node is missing");

			feedUserStyles(stylesRoot);

			// styles that were not read from xml file should get default values
			for (int i=0; i<SCE_USER_STYLE_TOTAL_STYLES; ++i)
			{
				Style & style = _userLangArray[_nbUserLang - 1]->_styleArray.getStyler(i);
				if (style._styleID == -1)
					_userLangArray[_nbUserLang - 1]->_styleArray.addStyler(i, globalMappper().styleNameMapper[i].c_str());
			}

		}
		catch (const std::exception& /*e*/)
		{
			delete _userLangArray[--_nbUserLang];
		}
	}
	int iEnd = _nbUserLang;
	return pair<unsigned char, unsigned char>(static_cast<unsigned char>(iBegin), static_cast<unsigned char>(iEnd));
}

bool NppParameters::importUDLFromFile(const generic_string& sourceFile)
{
	TiXmlDocument *pXmlUserLangDoc = new TiXmlDocument(sourceFile);
	bool loadOkay = pXmlUserLangDoc->LoadFile();
	if (loadOkay)
	{
		auto r = addUserDefineLangsFromXmlTree(pXmlUserLangDoc);
		loadOkay = (r.second - r.first) != 0;
		if (loadOkay)
		{
			_pXmlUserLangsDoc.push_back(UdlXmlFileState(nullptr, true, r));

			// imported UDL from xml file will be added into default udl, so we should make default udl dirty
			setUdlXmlDirtyFromXmlDoc(_pXmlUserLangDoc);
		}
	}
	delete pXmlUserLangDoc;
	return loadOkay;
}

bool NppParameters::exportUDLToFile(size_t langIndex2export, const generic_string& fileName2save)
{
	if (langIndex2export >= NB_MAX_USER_LANG)
		return false;

	if (static_cast<int32_t>(langIndex2export) >= _nbUserLang)
		return false;

	TiXmlDocument *pNewXmlUserLangDoc = new TiXmlDocument(fileName2save);
	TiXmlNode *newRoot2export = pNewXmlUserLangDoc->InsertEndChild(TiXmlElement(L"NotepadPlus"));

	insertUserLang2Tree(newRoot2export, _userLangArray[langIndex2export]);
	bool result = pNewXmlUserLangDoc->SaveFile();

	delete pNewXmlUserLangDoc;
	return result;
}

LangType NppParameters::getLangFromExt(const TCHAR *ext)
{
	int i = getNbLang();
	i--;
	while (i >= 0)
	{
		Lang *l = getLangFromIndex(i--);

		const TCHAR *defList = l->getDefaultExtList();
		const TCHAR *userList = NULL;

		LexerStylerArray &lsa = getLStylerArray();
		const TCHAR *lName = l->getLangName();
		LexerStyler *pLS = lsa.getLexerStylerByName(lName);

		if (pLS)
			userList = pLS->getLexerUserExt();

		generic_string list;
		if (defList)
			list += defList;

		if (userList)
		{
			list += L" ";
			list += userList;
		}
		if (isInList(ext, list.c_str()))
			return l->getLangID();
	}
	return L_TEXT;
}

void NppParameters::setCloudChoice(const TCHAR *pathChoice)
{
	generic_string cloudChoicePath = getSettingsFolder();
	cloudChoicePath += L"\\cloud\\";

	if (!PathFileExists(cloudChoicePath.c_str()))
	{
		::CreateDirectory(cloudChoicePath.c_str(), NULL);
	}
	cloudChoicePath += L"choice";

	WcharMbcsConvertor& wmc = WcharMbcsConvertor::getInstance();
	std::string cloudPathA = wmc.wchar2char(pathChoice, SC_CP_UTF8);

	writeFileContent(cloudChoicePath.c_str(), cloudPathA.c_str());
}

void NppParameters::removeCloudChoice()
{
	generic_string cloudChoicePath = getSettingsFolder();

	cloudChoicePath += L"\\cloud\\choice";
	if (PathFileExists(cloudChoicePath.c_str()))
	{
		::DeleteFile(cloudChoicePath.c_str());
	}
}

bool NppParameters::isCloudPathChanged() const
{
	if (_initialCloudChoice == _nppGUI._cloudPath)
		return false;
	else if (_initialCloudChoice.size() - _nppGUI._cloudPath.size() == 1)
	{
		TCHAR c = _initialCloudChoice.at(_initialCloudChoice.size()-1);
		if (c == '\\' || c == '/')
		{
			if (_initialCloudChoice.find(_nppGUI._cloudPath) == 0)
				return false;
		}
	}
	else if (_nppGUI._cloudPath.size() - _initialCloudChoice.size() == 1)
	{
		TCHAR c = _nppGUI._cloudPath.at(_nppGUI._cloudPath.size() - 1);
		if (c == '\\' || c == '/')
		{
			if (_nppGUI._cloudPath.find(_initialCloudChoice) == 0)
				return false;
		}
	}
	return true;
}

bool NppParameters::writeSettingsFilesOnCloudForThe1stTime(const generic_string & cloudSettingsPath)
{
	bool isOK = false;

	if (cloudSettingsPath.empty())
		return false;

	// config.xml
	generic_string cloudConfigPath = cloudSettingsPath;
	PathAppend(cloudConfigPath, L"config.xml");
	if (!::PathFileExists(cloudConfigPath.c_str()) && _pXmlUserDoc)
	{
		isOK = _pXmlUserDoc->SaveFile(cloudConfigPath.c_str());
		if (!isOK)
			return false;
	}

	// stylers.xml
	generic_string cloudStylersPath = cloudSettingsPath;
	PathAppend(cloudStylersPath, L"stylers.xml");
	if (!::PathFileExists(cloudStylersPath.c_str()) && _pXmlUserStylerDoc)
	{
		isOK = _pXmlUserStylerDoc->SaveFile(cloudStylersPath.c_str());
		if (!isOK)
			return false;
	}

	// langs.xml
	generic_string cloudLangsPath = cloudSettingsPath;
	PathAppend(cloudLangsPath, L"langs.xml");
	if (!::PathFileExists(cloudLangsPath.c_str()) && _pXmlUserDoc)
	{
		isOK = _pXmlDoc->SaveFile(cloudLangsPath.c_str());
		if (!isOK)
			return false;
	}

	// userDefineLang.xml
	generic_string cloudUserLangsPath = cloudSettingsPath;
	PathAppend(cloudUserLangsPath, L"userDefineLang.xml");
	if (!::PathFileExists(cloudUserLangsPath.c_str()) && _pXmlUserLangDoc)
	{
		isOK = _pXmlUserLangDoc->SaveFile(cloudUserLangsPath.c_str());
		if (!isOK)
			return false;
	}

	// shortcuts.xml
	generic_string cloudShortcutsPath = cloudSettingsPath;
	PathAppend(cloudShortcutsPath, L"shortcuts.xml");
	if (!::PathFileExists(cloudShortcutsPath.c_str()) && _pXmlShortcutDoc)
	{
		isOK = _pXmlShortcutDoc->SaveFile(cloudShortcutsPath.c_str());
		if (!isOK)
			return false;
	}

	// contextMenu.xml
	generic_string cloudContextMenuPath = cloudSettingsPath;
	PathAppend(cloudContextMenuPath, L"contextMenu.xml");
	if (!::PathFileExists(cloudContextMenuPath.c_str()) && _pXmlContextMenuDocA)
	{
		isOK = _pXmlContextMenuDocA->SaveUnicodeFilePath(cloudContextMenuPath.c_str());
		if (!isOK)
			return false;
	}

	// nativeLang.xml
	generic_string cloudNativeLangPath = cloudSettingsPath;
	PathAppend(cloudNativeLangPath, L"nativeLang.xml");
	if (!::PathFileExists(cloudNativeLangPath.c_str()) && _pXmlNativeLangDocA)
	{
		isOK = _pXmlNativeLangDocA->SaveUnicodeFilePath(cloudNativeLangPath.c_str());
		if (!isOK)
			return false;
	}

	return true;
}

/*
Default UDL + Created + Imported

*/
void NppParameters::writeDefaultUDL()
{
	bool firstCleanDone = false;
	std::vector<bool> deleteState;
	for (auto udl : _pXmlUserLangsDoc)
	{
		if (!_pXmlUserLangDoc)
		{
			_pXmlUserLangDoc = new TiXmlDocument(_userDefineLangPath);
			_pXmlUserLangDoc->InsertEndChild(TiXmlElement(L"NotepadPlus"));
		}

		bool toDelete = (udl._indexRange.second - udl._indexRange.first) == 0;
		deleteState.push_back(toDelete);
		if ((!udl._udlXmlDoc || udl._udlXmlDoc == _pXmlUserLangDoc) && udl._isDirty && !toDelete) // new created or/and imported UDL plus _pXmlUserLangDoc (if exist)
		{
			TiXmlNode *root = _pXmlUserLangDoc->FirstChild(L"NotepadPlus");
			if (root && !firstCleanDone)
			{
				_pXmlUserLangDoc->RemoveChild(root);
				_pXmlUserLangDoc->InsertEndChild(TiXmlElement(L"NotepadPlus"));
				firstCleanDone = true;
			}

			root = _pXmlUserLangDoc->FirstChild(L"NotepadPlus");

			for (int i = udl._indexRange.first; i < udl._indexRange.second; ++i)
			{
				insertUserLang2Tree(root, _userLangArray[i]);
			}
		}
	}

	bool deleteAll = true;
	for (bool del : deleteState)
	{
		if (!del)
		{
			deleteAll = false;
			break;
		}
	}

	if (firstCleanDone) // at least one udl is for saving, the udl to be deleted are ignored
	{
		_pXmlUserLangDoc->SaveFile();
	}
	else if (deleteAll)
	{
		if (::PathFileExists(_userDefineLangPath.c_str()))
		{
			::DeleteFile(_userDefineLangPath.c_str());
		}
	}
	// else nothing to change, do nothing
}

void NppParameters::writeNonDefaultUDL()
{
	for (auto udl : _pXmlUserLangsDoc)
	{
		if (udl._isDirty && udl._udlXmlDoc != nullptr && udl._udlXmlDoc != _pXmlUserLangDoc)
		{
			if (udl._indexRange.second == udl._indexRange.first) // no more udl for this xmldoc container
			{
				// no need to save, delete file
				const TCHAR* docFilePath = udl._udlXmlDoc->Value();
				if (docFilePath && ::PathFileExists(docFilePath))
				{
					::DeleteFile(docFilePath);
				}
			}
			else
			{
				TiXmlNode *root = udl._udlXmlDoc->FirstChild(L"NotepadPlus");
				if (root)
				{
					udl._udlXmlDoc->RemoveChild(root);
				}

				udl._udlXmlDoc->InsertEndChild(TiXmlElement(L"NotepadPlus"));

				root = udl._udlXmlDoc->FirstChild(L"NotepadPlus");

				for (int i = udl._indexRange.first; i < udl._indexRange.second; ++i)
				{
					insertUserLang2Tree(root, _userLangArray[i]);
				}
				udl._udlXmlDoc->SaveFile();
			}
		}
	}
}

void NppParameters::writeNeed2SaveUDL()
{
	stylerStrOp(DUP);

	writeDefaultUDL();
	writeNonDefaultUDL();
	
	stylerStrOp(FREE);
}


void NppParameters::insertCmd(TiXmlNode *shortcutsRoot, const CommandShortcut & cmd)
{
	const KeyCombo & key = cmd.getKeyCombo();
	TiXmlNode *sc = shortcutsRoot->InsertEndChild(TiXmlElement(L"Shortcut"));
	sc->ToElement()->SetAttribute(L"id", cmd.getID());
	sc->ToElement()->SetAttribute(L"Ctrl", key._isCtrl?L"yes":L"no");
	sc->ToElement()->SetAttribute(L"Alt", key._isAlt?L"yes":L"no");
	sc->ToElement()->SetAttribute(L"Shift", key._isShift?L"yes":L"no");
	sc->ToElement()->SetAttribute(L"Key", key._key);
}


void NppParameters::insertMacro(TiXmlNode *macrosRoot, const MacroShortcut & macro)
{
	const KeyCombo & key = macro.getKeyCombo();
	TiXmlNode *macroRoot = macrosRoot->InsertEndChild(TiXmlElement(L"Macro"));
	macroRoot->ToElement()->SetAttribute(L"name", macro.getMenuName());
	macroRoot->ToElement()->SetAttribute(L"Ctrl", key._isCtrl?L"yes":L"no");
	macroRoot->ToElement()->SetAttribute(L"Alt", key._isAlt?L"yes":L"no");
	macroRoot->ToElement()->SetAttribute(L"Shift", key._isShift?L"yes":L"no");
	macroRoot->ToElement()->SetAttribute(L"Key", key._key);

	for (size_t i = 0, len = macro._macro.size(); i < len ; ++i)
	{
		TiXmlNode *actionNode = macroRoot->InsertEndChild(TiXmlElement(L"Action"));
		const recordedMacroStep & action = macro._macro[i];
		actionNode->ToElement()->SetAttribute(L"type", action._macroType);
		actionNode->ToElement()->SetAttribute(L"message", action._message);
		actionNode->ToElement()->SetAttribute(L"wParam", static_cast<int>(action._wParameter));
		actionNode->ToElement()->SetAttribute(L"lParam", static_cast<int>(action._lParameter));
		actionNode->ToElement()->SetAttribute(L"sParam", action._sParameter.c_str());
	}
}


void NppParameters::insertUserCmd(TiXmlNode *userCmdRoot, const UserCommand & userCmd)
{
	const KeyCombo & key = userCmd.getKeyCombo();
	TiXmlNode *cmdRoot = userCmdRoot->InsertEndChild(TiXmlElement(L"Command"));
	cmdRoot->ToElement()->SetAttribute(L"name", userCmd.getMenuName());
	cmdRoot->ToElement()->SetAttribute(L"Ctrl", key._isCtrl?L"yes":L"no");
	cmdRoot->ToElement()->SetAttribute(L"Alt", key._isAlt?L"yes":L"no");
	cmdRoot->ToElement()->SetAttribute(L"Shift", key._isShift?L"yes":L"no");
	cmdRoot->ToElement()->SetAttribute(L"Key", key._key);
	cmdRoot->InsertEndChild(TiXmlText(userCmd._cmd.c_str()));
}


void NppParameters::insertPluginCmd(TiXmlNode *pluginCmdRoot, const PluginCmdShortcut & pluginCmd)
{
	const KeyCombo & key = pluginCmd.getKeyCombo();
	TiXmlNode *pluginCmdNode = pluginCmdRoot->InsertEndChild(TiXmlElement(L"PluginCommand"));
	pluginCmdNode->ToElement()->SetAttribute(L"moduleName", pluginCmd.getModuleName());
	pluginCmdNode->ToElement()->SetAttribute(L"internalID", pluginCmd.getInternalID());
	pluginCmdNode->ToElement()->SetAttribute(L"Ctrl", key._isCtrl?L"yes":L"no");
	pluginCmdNode->ToElement()->SetAttribute(L"Alt", key._isAlt?L"yes":L"no");
	pluginCmdNode->ToElement()->SetAttribute(L"Shift", key._isShift?L"yes":L"no");
	pluginCmdNode->ToElement()->SetAttribute(L"Key", key._key);
}


void NppParameters::insertScintKey(TiXmlNode *scintKeyRoot, const ScintillaKeyMap & scintKeyMap)
{
	TiXmlNode *keyRoot = scintKeyRoot->InsertEndChild(TiXmlElement(L"ScintKey"));
	keyRoot->ToElement()->SetAttribute(L"ScintID", scintKeyMap.getScintillaKeyID());
	keyRoot->ToElement()->SetAttribute(L"menuCmdID", scintKeyMap.getMenuCmdID());

	//Add main shortcut
	KeyCombo key = scintKeyMap.getKeyComboByIndex(0);
	keyRoot->ToElement()->SetAttribute(L"Ctrl", key._isCtrl?L"yes":L"no");
	keyRoot->ToElement()->SetAttribute(L"Alt", key._isAlt?L"yes":L"no");
	keyRoot->ToElement()->SetAttribute(L"Shift", key._isShift?L"yes":L"no");
	keyRoot->ToElement()->SetAttribute(L"Key", key._key);

	//Add additional shortcuts
	size_t size = scintKeyMap.getSize();
	if (size > 1)
	{
		for (size_t i = 1; i < size; ++i)
		{
			TiXmlNode *keyNext = keyRoot->InsertEndChild(TiXmlElement(L"NextKey"));
			key = scintKeyMap.getKeyComboByIndex(i);
			keyNext->ToElement()->SetAttribute(L"Ctrl", key._isCtrl?L"yes":L"no");
			keyNext->ToElement()->SetAttribute(L"Alt", key._isAlt?L"yes":L"no");
			keyNext->ToElement()->SetAttribute(L"Shift", key._isShift?L"yes":L"no");
			keyNext->ToElement()->SetAttribute(L"Key", key._key);
		}
	}
}


void NppParameters::writeSession(const Session & session, const TCHAR *fileName)
{
	const TCHAR *pathName = fileName?fileName:_sessionPath.c_str();

	_pXmlSessionDoc = new TiXmlDocument(pathName);
	TiXmlNode *root = _pXmlSessionDoc->InsertEndChild(TiXmlElement(L"NotepadPlus"));

	if (root)
	{
		TiXmlNode *sessionNode = root->InsertEndChild(TiXmlElement(L"Session"));
		(sessionNode->ToElement())->SetAttribute(L"activeView", static_cast<int32_t>(session._activeView));

		struct ViewElem {
			TiXmlNode *viewNode;
			vector<sessionFileInfo> *viewFiles;
			size_t activeIndex;
		};
		const int nbElem = 2;
		ViewElem viewElems[nbElem];
		viewElems[0].viewNode = sessionNode->InsertEndChild(TiXmlElement(L"mainView"));
		viewElems[1].viewNode = sessionNode->InsertEndChild(TiXmlElement(L"subView"));
		viewElems[0].viewFiles = (vector<sessionFileInfo> *)(&(session._mainViewFiles));
		viewElems[1].viewFiles = (vector<sessionFileInfo> *)(&(session._subViewFiles));
		viewElems[0].activeIndex = session._activeMainIndex;
		viewElems[1].activeIndex = session._activeSubIndex;

		for (size_t k = 0; k < nbElem ; ++k)
		{
			(viewElems[k].viewNode->ToElement())->SetAttribute(L"activeIndex", static_cast<int32_t>(viewElems[k].activeIndex));
			vector<sessionFileInfo> & viewSessionFiles = *(viewElems[k].viewFiles);

			for (size_t i = 0, len = viewElems[k].viewFiles->size(); i < len ; ++i)
			{
				TiXmlNode *fileNameNode = viewElems[k].viewNode->InsertEndChild(TiXmlElement(L"File"));

				(fileNameNode->ToElement())->SetAttribute(L"firstVisibleLine", viewSessionFiles[i]._firstVisibleLine);
				(fileNameNode->ToElement())->SetAttribute(L"xOffset", viewSessionFiles[i]._xOffset);
				(fileNameNode->ToElement())->SetAttribute(L"scrollWidth", viewSessionFiles[i]._scrollWidth);
				(fileNameNode->ToElement())->SetAttribute(L"startPos", viewSessionFiles[i]._startPos);
				(fileNameNode->ToElement())->SetAttribute(L"endPos", viewSessionFiles[i]._endPos);
				(fileNameNode->ToElement())->SetAttribute(L"selMode", viewSessionFiles[i]._selMode);
				(fileNameNode->ToElement())->SetAttribute(L"offset", viewSessionFiles[i]._offset);
				(fileNameNode->ToElement())->SetAttribute(L"wrapCount", viewSessionFiles[i]._wrapCount);
				(fileNameNode->ToElement())->SetAttribute(L"lang", viewSessionFiles[i]._langName.c_str());
				(fileNameNode->ToElement())->SetAttribute(L"encoding", viewSessionFiles[i]._encoding);
				(fileNameNode->ToElement())->SetAttribute(L"userReadOnly", (viewSessionFiles[i]._isUserReadOnly && !viewSessionFiles[i]._isMonitoring) ? L"yes" : L"no");
				(fileNameNode->ToElement())->SetAttribute(L"filename", viewSessionFiles[i]._fileName.c_str());
				(fileNameNode->ToElement())->SetAttribute(L"backupFilePath", viewSessionFiles[i]._backupFilePath.c_str());
				(fileNameNode->ToElement())->SetAttribute(L"originalFileLastModifTimestamp", static_cast<int32_t>(viewSessionFiles[i]._originalFileLastModifTimestamp.dwLowDateTime));
				(fileNameNode->ToElement())->SetAttribute(L"originalFileLastModifTimestampHigh", static_cast<int32_t>(viewSessionFiles[i]._originalFileLastModifTimestamp.dwHighDateTime));
				
				// docMap 
				(fileNameNode->ToElement())->SetAttribute(L"mapFirstVisibleDisplayLine", viewSessionFiles[i]._mapPos._firstVisibleDisplayLine);
				(fileNameNode->ToElement())->SetAttribute(L"mapFirstVisibleDocLine", viewSessionFiles[i]._mapPos._firstVisibleDocLine);
				(fileNameNode->ToElement())->SetAttribute(L"mapLastVisibleDocLine", viewSessionFiles[i]._mapPos._lastVisibleDocLine);
				(fileNameNode->ToElement())->SetAttribute(L"mapNbLine", viewSessionFiles[i]._mapPos._nbLine);
				(fileNameNode->ToElement())->SetAttribute(L"mapHigherPos", viewSessionFiles[i]._mapPos._higherPos);
				(fileNameNode->ToElement())->SetAttribute(L"mapWidth", viewSessionFiles[i]._mapPos._width);
				(fileNameNode->ToElement())->SetAttribute(L"mapHeight", viewSessionFiles[i]._mapPos._height);
				(fileNameNode->ToElement())->SetAttribute(L"mapKByteInDoc", static_cast<int>(viewSessionFiles[i]._mapPos._KByteInDoc));
				(fileNameNode->ToElement())->SetAttribute(L"mapWrapIndentMode", viewSessionFiles[i]._mapPos._wrapIndentMode);
				fileNameNode->ToElement()->SetAttribute(L"mapIsWrap", viewSessionFiles[i]._mapPos._isWrap ? L"yes" : L"no");

				for (size_t j = 0, len = viewSessionFiles[i]._marks.size() ; j < len ; ++j)
				{
					size_t markLine = viewSessionFiles[i]._marks[j];
					TiXmlNode *markNode = fileNameNode->InsertEndChild(TiXmlElement(L"Mark"));
					markNode->ToElement()->SetAttribute(L"line", static_cast<int32_t>(markLine));
				}

				for (size_t j = 0, len = viewSessionFiles[i]._foldStates.size() ; j < len ; ++j)
				{
					size_t foldLine = viewSessionFiles[i]._foldStates[j];
					TiXmlNode *foldNode = fileNameNode->InsertEndChild(TiXmlElement(L"Fold"));
					foldNode->ToElement()->SetAttribute(L"line", static_cast<int32_t>(foldLine));
				}
			}
		}
	}
	_pXmlSessionDoc->SaveFile();
}


void NppParameters::writeShortcuts()
{
	if (not _isAnyShortcutModified) return;

	if (not _pXmlShortcutDoc)
	{
		//do the treatment
		_pXmlShortcutDoc = new TiXmlDocument(_shortcutsPath);
	}

	TiXmlNode *root = _pXmlShortcutDoc->FirstChild(L"NotepadPlus");
	if (!root)
	{
		root = _pXmlShortcutDoc->InsertEndChild(TiXmlElement(L"NotepadPlus"));
	}

	TiXmlNode *cmdRoot = root->FirstChild(L"InternalCommands");
	if (cmdRoot)
		root->RemoveChild(cmdRoot);

	cmdRoot = root->InsertEndChild(TiXmlElement(L"InternalCommands"));
	for (size_t i = 0, len = _customizedShortcuts.size(); i < len ; ++i)
	{
		size_t index = _customizedShortcuts[i];
		CommandShortcut csc = _shortcuts[index];
		insertCmd(cmdRoot, csc);
	}

	TiXmlNode *macrosRoot = root->FirstChild(L"Macros");
	if (macrosRoot)
		root->RemoveChild(macrosRoot);

	macrosRoot = root->InsertEndChild(TiXmlElement(L"Macros"));

	for (size_t i = 0, len = _macros.size(); i < len ; ++i)
	{
		insertMacro(macrosRoot, _macros[i]);
	}

	TiXmlNode *userCmdRoot = root->FirstChild(L"UserDefinedCommands");
	if (userCmdRoot)
		root->RemoveChild(userCmdRoot);

	userCmdRoot = root->InsertEndChild(TiXmlElement(L"UserDefinedCommands"));

	for (size_t i = 0, len = _userCommands.size(); i < len ; ++i)
	{
		insertUserCmd(userCmdRoot, _userCommands[i]);
	}

	TiXmlNode *pluginCmdRoot = root->FirstChild(L"PluginCommands");
	if (pluginCmdRoot)
		root->RemoveChild(pluginCmdRoot);

	pluginCmdRoot = root->InsertEndChild(TiXmlElement(L"PluginCommands"));
	for (size_t i = 0, len = _pluginCustomizedCmds.size(); i < len ; ++i)
	{
		insertPluginCmd(pluginCmdRoot, _pluginCommands[_pluginCustomizedCmds[i]]);
	}

	TiXmlNode *scitillaKeyRoot = root->FirstChild(L"ScintillaKeys");
	if (scitillaKeyRoot)
		root->RemoveChild(scitillaKeyRoot);

	scitillaKeyRoot = root->InsertEndChild(TiXmlElement(L"ScintillaKeys"));
	for (size_t i = 0, len = _scintillaModifiedKeyIndices.size(); i < len ; ++i)
	{
		insertScintKey(scitillaKeyRoot, _scintillaKeyCommands[_scintillaModifiedKeyIndices[i]]);
	}
	_pXmlShortcutDoc->SaveFile();
}


int NppParameters::addUserLangToEnd(const UserLangContainer & userLang, const TCHAR *newName)
{
	if (isExistingUserLangName(newName))
		return -1;
	unsigned char iBegin = _nbUserLang;
	_userLangArray[_nbUserLang] = new UserLangContainer();
	*(_userLangArray[_nbUserLang]) = userLang;
	_userLangArray[_nbUserLang]->_name = newName;
	++_nbUserLang;
	unsigned char iEnd = _nbUserLang;

	_pXmlUserLangsDoc.push_back(UdlXmlFileState(nullptr, true, make_pair(iBegin, iEnd)));

	// imported UDL from xml file will be added into default udl, so we should make default udl dirty
	setUdlXmlDirtyFromXmlDoc(_pXmlUserLangDoc);

	return _nbUserLang-1;
}


void NppParameters::removeUserLang(size_t index)
{
	if (static_cast<int32_t>(index) >= _nbUserLang)
		return;
	delete _userLangArray[index];

	for (int32_t i = static_cast<int32_t>(index); i < (_nbUserLang - 1); ++i)
		_userLangArray[i] = _userLangArray[i+1];
	_nbUserLang--;

	removeIndexFromXmlUdls(index);
}


void NppParameters::feedUserSettings(TiXmlNode *settingsRoot)
{
	const TCHAR *boolStr;
	TiXmlNode *globalSettingNode = settingsRoot->FirstChildElement(L"Global");
	if (globalSettingNode)
	{
		boolStr = (globalSettingNode->ToElement())->Attribute(L"caseIgnored");
		if (boolStr)
			_userLangArray[_nbUserLang - 1]->_isCaseIgnored = (lstrcmp(L"yes", boolStr) == 0);

		boolStr = (globalSettingNode->ToElement())->Attribute(L"allowFoldOfComments");
		if (boolStr)
			_userLangArray[_nbUserLang - 1]->_allowFoldOfComments = (lstrcmp(L"yes", boolStr) == 0);

		(globalSettingNode->ToElement())->Attribute(L"forcePureLC", &_userLangArray[_nbUserLang - 1]->_forcePureLC);
		(globalSettingNode->ToElement())->Attribute(L"decimalSeparator", &_userLangArray[_nbUserLang - 1]->_decimalSeparator);

		boolStr = (globalSettingNode->ToElement())->Attribute(L"foldCompact");
		if (boolStr)
			_userLangArray[_nbUserLang - 1]->_foldCompact = (lstrcmp(L"yes", boolStr) == 0);
	}

	TiXmlNode *prefixNode = settingsRoot->FirstChildElement(L"Prefix");
	if (prefixNode)
	{
		const TCHAR *udlVersion = _userLangArray[_nbUserLang - 1]->_udlVersion.c_str();
		if (!lstrcmp(udlVersion, L"2.1") || !lstrcmp(udlVersion, L"2.0"))
		{
			for (int i = 0 ; i < SCE_USER_TOTAL_KEYWORD_GROUPS ; ++i)
			{
				boolStr = (prefixNode->ToElement())->Attribute(globalMappper().keywordNameMapper[i+SCE_USER_KWLIST_KEYWORDS1]);
				if (boolStr)
					_userLangArray[_nbUserLang - 1]->_isPrefix[i] = (lstrcmp(L"yes", boolStr) == 0);
			}
		}
		else	// support for old style (pre 2.0)
		{
			TCHAR names[SCE_USER_TOTAL_KEYWORD_GROUPS][7] = {L"words1", L"words2", L"words3", L"words4"};
			for (int i = 0 ; i < 4 ; ++i)
			{
				boolStr = (prefixNode->ToElement())->Attribute(names[i]);
				if (boolStr)
					_userLangArray[_nbUserLang - 1]->_isPrefix[i] = (lstrcmp(L"yes", boolStr) == 0);
			}
		}
	}
}


void NppParameters::feedUserKeywordList(TiXmlNode *node)
{
	const TCHAR * udlVersion = _userLangArray[_nbUserLang - 1]->_udlVersion.c_str();
	int id = -1;

	for (TiXmlNode *childNode = node->FirstChildElement(L"Keywords");
		childNode ;
		childNode = childNode->NextSibling(L"Keywords"))
	{
		const TCHAR * keywordsName = (childNode->ToElement())->Attribute(L"name");
		TiXmlNode *valueNode = childNode->FirstChild();
		if (valueNode)
		{
			const TCHAR *kwl = nullptr;
			if (!lstrcmp(udlVersion, L"") && !lstrcmp(keywordsName, L"Delimiters"))	// support for old style (pre 2.0)
			{
				basic_string<TCHAR> temp;
				kwl = (valueNode)?valueNode->Value():L"000000";

				temp += L"00";	 if (kwl[0] != '0') temp += kwl[0];	 temp += L" 01";
				temp += L" 02";	if (kwl[3] != '0') temp += kwl[3];
				temp += L" 03";	if (kwl[1] != '0') temp += kwl[1];	 temp += L" 04";
				temp += L" 05";	if (kwl[4] != '0') temp += kwl[4];
				temp += L" 06";	if (kwl[2] != '0') temp += kwl[2];	 temp += L" 07";
				temp += L" 08";	if (kwl[5] != '0') temp += kwl[5];

				temp += L" 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23";
				wcscpy_s(_userLangArray[_nbUserLang - 1]->_keywordLists[SCE_USER_KWLIST_DELIMITERS], temp.c_str());
			}
			else if (!lstrcmp(keywordsName, L"Comment"))
			{
				kwl = (valueNode)?valueNode->Value():L"";
				//int len = _tcslen(kwl);
				basic_string<TCHAR> temp{L" "};

				temp += kwl;
				size_t pos = 0;

				pos = temp.find(L" 0");
				while (pos != string::npos)
				{
					temp.replace(pos, 2, L" 00");
					pos = temp.find(L" 0", pos+1);
				}
				pos = temp.find(L" 1");
				while (pos != string::npos)
				{
					temp.replace(pos, 2, L" 03");
					pos = temp.find(L" 1");
				}
				pos = temp.find(L" 2");
				while (pos != string::npos)
				{
					temp.replace(pos, 2, L" 04");
					pos = temp.find(L" 2");
				}

				temp += L" 01 02";
				if (temp[0] == ' ')
					temp.erase(0, 1);

				wcscpy_s(_userLangArray[_nbUserLang - 1]->_keywordLists[SCE_USER_KWLIST_COMMENTS], temp.c_str());
			}
			else
			{
				kwl = (valueNode)?valueNode->Value():L"";
				if (globalMappper().keywordIdMapper.find(keywordsName) != globalMappper().keywordIdMapper.end())
				{
					id = globalMappper().keywordIdMapper[keywordsName];
					if (_tcslen(kwl) < max_char)
					{
						wcscpy_s(_userLangArray[_nbUserLang - 1]->_keywordLists[id], kwl);
					}
					else
					{
						wcscpy_s(_userLangArray[_nbUserLang - 1]->_keywordLists[id], L"imported string too long, needs to be < max_char(30720)");
					}
				}
			}
		}
	}
}

void NppParameters::feedUserStyles(TiXmlNode *node)
{
	int id = -1;

	for (TiXmlNode *childNode = node->FirstChildElement(L"WordsStyle");
		childNode ;
		childNode = childNode->NextSibling(L"WordsStyle"))
	{
		const TCHAR *styleName = (childNode->ToElement())->Attribute(L"name");
		if (styleName)
		{
			if (globalMappper().styleIdMapper.find(styleName) != globalMappper().styleIdMapper.end())
			{
				id = globalMappper().styleIdMapper[styleName];
				_userLangArray[_nbUserLang - 1]->_styleArray.addStyler((id | L_USER << 16), childNode);
			}
		}
	}
}

bool NppParameters::feedStylerArray(TiXmlNode *node)
{
	TiXmlNode *styleRoot = node->FirstChildElement(L"LexerStyles");
	if (!styleRoot) return false;

	// For each lexer
	for (TiXmlNode *childNode = styleRoot->FirstChildElement(L"LexerType");
		 childNode ;
		 childNode = childNode->NextSibling(L"LexerType") )
	{
	 	if (!_lexerStylerArray.hasEnoughSpace()) return false;

		TiXmlElement *element = childNode->ToElement();
		const TCHAR *lexerName = element->Attribute(L"name");
		const TCHAR *lexerDesc = element->Attribute(L"desc");
		const TCHAR *lexerUserExt = element->Attribute(L"ext");
		const TCHAR *lexerExcluded = element->Attribute(L"excluded");
		if (lexerName)
		{
			_lexerStylerArray.addLexerStyler(lexerName, lexerDesc, lexerUserExt, childNode);
			if (lexerExcluded != NULL && (lstrcmp(lexerExcluded, L"yes") == 0))
			{
				int index = getExternalLangIndexFromName(lexerName);
				if (index != -1)
					_nppGUI._excludedLangList.push_back(LangMenuItem((LangType)(index + L_EXTERNAL)));
			}
		}
	}

	// The global styles for all lexers
	TiXmlNode *globalStyleRoot = node->FirstChildElement(L"GlobalStyles");
	if (!globalStyleRoot) return false;

	for (TiXmlNode *childNode = globalStyleRoot->FirstChildElement(L"WidgetStyle");
		 childNode ;
		 childNode = childNode->NextSibling(L"WidgetStyle") )
	{
	 	if (!_widgetStyleArray.hasEnoughSpace()) return false;

		TiXmlElement *element = childNode->ToElement();
		const TCHAR *styleIDStr = element->Attribute(L"styleID");

		int styleID = -1;
		if ((styleID = decStrVal(styleIDStr)) != -1)
		{
			_widgetStyleArray.addStyler(styleID, childNode);
		}
	}
	return true;
}

void LexerStylerArray::addLexerStyler(const TCHAR *lexerName, const TCHAR *lexerDesc, const TCHAR *lexerUserExt , TiXmlNode *lexerNode)
{
	LexerStyler & ls = _lexerStylerArray[_nbLexerStyler++];
	ls.setLexerName(lexerName);
	if (lexerDesc)
		ls.setLexerDesc(lexerDesc);

	if (lexerUserExt)
		ls.setLexerUserExt(lexerUserExt);

	for (TiXmlNode *childNode = lexerNode->FirstChildElement(L"WordsStyle");
		 childNode ;
		 childNode = childNode->NextSibling(L"WordsStyle") )
	{
		if (!ls.hasEnoughSpace())
			return;

		TiXmlElement *element = childNode->ToElement();
		const TCHAR *styleIDStr = element->Attribute(L"styleID");

		if (styleIDStr)
		{
			int styleID = -1;
			if ((styleID = decStrVal(styleIDStr)) != -1)
			{
				ls.addStyler(styleID, childNode);
			}
		}
	}
}

void LexerStylerArray::eraseAll()
{

	for (int i = 0 ; i < _nbLexerStyler ; ++i)
	{
		_lexerStylerArray[i].setNbStyler( 0 );
	}

	_nbLexerStyler = 0;
}

void StyleArray::addStyler(int styleID, TiXmlNode *styleNode)
{
	int index = _nbStyler;
	bool isUser = styleID >> 16 == L_USER;
	if (isUser)
	{
		styleID = (styleID & 0xFFFF);
		index = styleID;
		if (index >= SCE_USER_STYLE_TOTAL_STYLES || _styleArray[index]._styleID != -1)
			return;
	}

	_styleArray[index]._styleID = styleID;

	if (styleNode)
	{
		TiXmlElement *element = styleNode->ToElement();

		// TODO: translate to English
		// Pour _fgColor, _bgColor :
		// RGB() | (result & 0xFF000000) c'est pour le cas de -1 (0xFFFFFFFF)
		// retourné par hexStrVal(str)
		const TCHAR *str = element->Attribute(L"name");
		if (str)
		{
			if (isUser)
				_styleArray[index]._styleDesc = globalMappper().styleNameMapper[index].c_str();
			else
				_styleArray[index]._styleDesc = str;
		}

		str = element->Attribute(L"fgColor");
		if (str)
		{
			unsigned long result = hexStrVal(str);
			_styleArray[index]._fgColor = (RGB((result >> 16) & 0xFF, (result >> 8) & 0xFF, result & 0xFF)) | (result & 0xFF000000);

		}

		str = element->Attribute(L"bgColor");
		if (str)
		{
			unsigned long result = hexStrVal(str);
			_styleArray[index]._bgColor = (RGB((result >> 16) & 0xFF, (result >> 8) & 0xFF, result & 0xFF)) | (result & 0xFF000000);
		}

		str = element->Attribute(L"colorStyle");
		if (str)
		{
			_styleArray[index]._colorStyle = decStrVal(str);
		}

		str = element->Attribute(L"fontName");
		_styleArray[index]._fontName = str;

		str = element->Attribute(L"fontStyle");
		if (str)
		{
			_styleArray[index]._fontStyle = decStrVal(str);
		}

		str = element->Attribute(L"fontSize");
		if (str)
		{
			_styleArray[index]._fontSize = decStrVal(str);
		}
		str = element->Attribute(L"nesting");

		if (str)
		{
			_styleArray[index]._nesting = decStrVal(str);
		}

		str = element->Attribute(L"keywordClass");
		if (str)
		{
			_styleArray[index]._keywordClass = getKwClassFromName(str);
		}

		TiXmlNode *v = styleNode->FirstChild();
		if (v)
		{
			_styleArray[index]._keywords = new generic_string(v->Value());
		}
	}
	++_nbStyler;
}

bool NppParameters::writeRecentFileHistorySettings(int nbMaxFile) const
{
	if (not _pXmlUserDoc) return false;

	TiXmlNode *nppRoot = _pXmlUserDoc->FirstChild(L"NotepadPlus");
	if (not nppRoot)
	{
		nppRoot = _pXmlUserDoc->InsertEndChild(TiXmlElement(L"NotepadPlus"));
	}

	TiXmlNode *historyNode = nppRoot->FirstChildElement(L"History");
	if (not historyNode)
	{
		historyNode = nppRoot->InsertEndChild(TiXmlElement(L"History"));
	}

	(historyNode->ToElement())->SetAttribute(L"nbMaxFile", nbMaxFile!=-1?nbMaxFile:_nbMaxRecentFile);
	(historyNode->ToElement())->SetAttribute(L"inSubMenu", _putRecentFileInSubMenu?L"yes":L"no");
	(historyNode->ToElement())->SetAttribute(L"customLength", _recentFileCustomLength);
	return true;
}

bool NppParameters::writeProjectPanelsSettings() const
{
	if (!_pXmlUserDoc) return false;

	TiXmlNode *nppRoot = _pXmlUserDoc->FirstChild(L"NotepadPlus");
	if (not nppRoot)
	{
		nppRoot = _pXmlUserDoc->InsertEndChild(TiXmlElement(L"NotepadPlus"));
	}

	TiXmlNode *oldProjPanelRootNode = nppRoot->FirstChildElement(L"ProjectPanels");
	if (oldProjPanelRootNode)
	{
		// Erase the Project Panel root
		nppRoot->RemoveChild(oldProjPanelRootNode);
	}

	// Create the Project Panel root
	TiXmlElement projPanelRootNode{L"ProjectPanels"};

	// Add 3 Project Panel parameters
	for (int32_t i = 0 ; i < 3 ; ++i)
	{
		TiXmlElement projPanelNode{L"ProjectPanel"};
		(projPanelNode.ToElement())->SetAttribute(L"id", i);
		(projPanelNode.ToElement())->SetAttribute(L"workSpaceFile", _workSpaceFilePathes[i]);

		(projPanelRootNode.ToElement())->InsertEndChild(projPanelNode);
	}

	// (Re)Insert the Project Panel root
	(nppRoot->ToElement())->InsertEndChild(projPanelRootNode);
	return true;
}

bool NppParameters::writeFileBrowserSettings(const vector<generic_string> & rootPaths, const generic_string & latestSelectedItemPath) const
{
	if (!_pXmlUserDoc) return false;

	TiXmlNode *nppRoot = _pXmlUserDoc->FirstChild(L"NotepadPlus");
	if (not nppRoot)
	{
		nppRoot = _pXmlUserDoc->InsertEndChild(TiXmlElement(L"NotepadPlus"));
	}

	TiXmlNode *oldFileBrowserRootNode = nppRoot->FirstChildElement(L"FileBrowser");
	if (oldFileBrowserRootNode)
	{
		// Erase the file broser root
		nppRoot->RemoveChild(oldFileBrowserRootNode);
	}

	// Create the file browser root
	TiXmlElement fileBrowserRootNode{ L"FileBrowser"};

	if (rootPaths.size() != 0)
	{
		fileBrowserRootNode.SetAttribute(L"latestSelectedItem", latestSelectedItemPath.c_str());

		// add roots
		size_t len = rootPaths.size();
		for (size_t i = 0; i < len; ++i)
		{
			TiXmlElement fbRootNode{ L"root"};
			(fbRootNode.ToElement())->SetAttribute(L"foldername", rootPaths[i].c_str());

			(fileBrowserRootNode.ToElement())->InsertEndChild(fbRootNode);
		}
	}

	// (Re)Insert the file browser root
	(nppRoot->ToElement())->InsertEndChild(fileBrowserRootNode);
	return true;
}

bool NppParameters::writeHistory(const TCHAR *fullpath)
{
	TiXmlNode *nppRoot = _pXmlUserDoc->FirstChild(L"NotepadPlus");
	if (not nppRoot)
	{
		nppRoot = _pXmlUserDoc->InsertEndChild(TiXmlElement(L"NotepadPlus"));
	}

	TiXmlNode *historyNode = nppRoot->FirstChildElement(L"History");
	if (not historyNode)
	{
		historyNode = nppRoot->InsertEndChild(TiXmlElement(L"History"));
	}

	TiXmlElement recentFileNode(L"File");
	(recentFileNode.ToElement())->SetAttribute(L"filename", fullpath);

	(historyNode->ToElement())->InsertEndChild(recentFileNode);
	return true;
}

TiXmlNode * NppParameters::getChildElementByAttribut(TiXmlNode *pere, const TCHAR *childName,\
			const TCHAR *attributName, const TCHAR *attributVal) const
{
	for (TiXmlNode *childNode = pere->FirstChildElement(childName);
		childNode ;
		childNode = childNode->NextSibling(childName))
	{
		TiXmlElement *element = childNode->ToElement();
		const TCHAR *val = element->Attribute(attributName);
		if (val)
		{
			if (!lstrcmp(val, attributVal))
				return childNode;
		}
	}
	return NULL;
}

// 2 restes : L_H, L_USER
LangType NppParameters::getLangIDFromStr(const TCHAR *langName)
{
	int lang = static_cast<int32_t>(L_TEXT);
	for (; lang < L_EXTERNAL; ++lang)
	{
		const TCHAR * name = ScintillaEditView::langNames[lang].lexerName;
		if (!lstrcmp(name, langName)) //found lang?
		{
			return (LangType)lang;
		}
	}

	//Cannot find language, check if its an external one

	LangType l = (LangType)lang;
	if (l == L_EXTERNAL) //try find external lexer
	{
		int id = NppParameters::getInstance().getExternalLangIndexFromName(langName);
		if (id != -1) return (LangType)(id + L_EXTERNAL);
	}

	return L_TEXT;
}

generic_string NppParameters::getLocPathFromStr(const generic_string & localizationCode)
{
	if (localizationCode == L"af")
		return L"afrikaans.xml";
	if (localizationCode == L"sq")
		return L"albanian.xml";
	if (localizationCode == L"ar" || localizationCode == L"ar-dz" || localizationCode == L"ar-bh" || localizationCode == L"ar-eg" ||localizationCode == L"ar-iq" || localizationCode == L"ar-jo" || localizationCode == L"ar-kw" || localizationCode == L"ar-lb" || localizationCode == L"ar-ly" || localizationCode == L"ar-ma" || localizationCode == L"ar-om" || localizationCode == L"ar-qa" || localizationCode == L"ar-sa" || localizationCode == L"ar-sy" || localizationCode == L"ar-tn" || localizationCode == L"ar-ae" || localizationCode == L"ar-ye")
		return L"arabic.xml";
	if (localizationCode == L"an")
		return L"aragonese.xml";
	if (localizationCode == L"az")
		return L"azerbaijani.xml";
	if (localizationCode == L"eu")
		return L"basque.xml";
	if (localizationCode == L"be")
		return L"belarusian.xml";
	if (localizationCode == L"bn")
		return L"bengali.xml";
	if (localizationCode == L"bs")
		return L"bosnian.xml";
	if (localizationCode == L"pt-br")
		return L"brazilian_portuguese.xml";
	if (localizationCode == L"br-fr")
		return L"breton.xml";
	if (localizationCode == L"bg")
		return L"bulgarian.xml";
	if (localizationCode == L"ca")
		return L"catalan.xml";
	if (localizationCode == L"zh-tw" || localizationCode == L"zh-hk" || localizationCode == L"zh-sg")
		return L"chinese.xml";
	if (localizationCode == L"zh" || localizationCode == L"zh-cn")
		return L"chineseSimplified.xml";
	if (localizationCode == L"co" || localizationCode == L"co-fr")
		return L"corsican.xml";
	if (localizationCode == L"hr")
		return L"croatian.xml";
	if (localizationCode == L"cs")
		return L"czech.xml";
	if (localizationCode == L"da")
		return L"danish.xml";
	if (localizationCode == L"nl" || localizationCode == L"nl-be")
		return L"dutch.xml";
	if (localizationCode == L"eo")
		return L"esperanto.xml";
	if (localizationCode == L"et")
		return L"estonian.xml";
	if (localizationCode == L"fa")
		return L"farsi.xml";
	if (localizationCode == L"fi")
		return L"finnish.xml";
	if (localizationCode == L"fr" || localizationCode == L"fr-be" || localizationCode == L"fr-ca" || localizationCode == L"fr-fr" || localizationCode == L"fr-lu" || localizationCode == L"fr-mc" || localizationCode == L"fr-ch")
		return L"french.xml";
	if (localizationCode == L"fur")
		return L"friulian.xml";
	if (localizationCode == L"gl")
		return L"galician.xml";
	if (localizationCode == L"ka")
		return L"georgian.xml";
	if (localizationCode == L"de" || localizationCode == L"de-at" || localizationCode == L"de-de" || localizationCode == L"de-li" || localizationCode == L"de-lu" || localizationCode == L"de-ch")
		return L"german.xml";
	if (localizationCode == L"el")
		return L"greek.xml";
	if (localizationCode == L"gu")
		return L"gujarati.xml";
	if (localizationCode == L"he")
		return L"hebrew.xml";
	if (localizationCode == L"hi")
		return L"hindi.xml";
	if (localizationCode == L"hu")
		return L"hungarian.xml";
	if (localizationCode == L"id")
		return L"indonesian.xml";
	if (localizationCode == L"it" || localizationCode == L"it-ch")
		return L"italian.xml";
	if (localizationCode == L"ja")
		return L"japanese.xml";
	if (localizationCode == L"kn")
		return L"kannada.xml";
	if (localizationCode == L"kk")
		return L"kazakh.xml";
	if (localizationCode == L"ko" || localizationCode == L"ko-kp" || localizationCode == L"ko-kr")
		return L"korean.xml";
	if (localizationCode == L"ku")
		return L"kurdish.xml";
	if (localizationCode == L"ky")
		return L"kyrgyz.xml";
	if (localizationCode == L"lv")
		return L"latvian.xml";
	if (localizationCode == L"lt")
		return L"lithuanian.xml";
	if (localizationCode == L"lb")
		return L"luxembourgish.xml";
	if (localizationCode == L"mk")
		return L"macedonian.xml";
	if (localizationCode == L"ms")
		return L"malay.xml";
	if (localizationCode == L"mr")
		return L"marathi.xml";
	if (localizationCode == L"mn")
		return L"mongolian.xml";
	if (localizationCode == L"no" || localizationCode == L"nb")
		return L"norwegian.xml";
	if (localizationCode == L"nn")
		return L"nynorsk.xml";
	if (localizationCode == L"oc")
		return L"occitan.xml";
	if (localizationCode == L"pl")
		return L"polish.xml";
	if (localizationCode == L"pt" || localizationCode == L"pt-pt")
		return L"portuguese.xml";
	if (localizationCode == L"pa" || localizationCode == L"pa-in")
		return L"punjabi.xml";
	if (localizationCode == L"ro" || localizationCode == L"ro-mo")
		return L"romanian.xml";
	if (localizationCode == L"ru" || localizationCode == L"ru-mo")
		return L"russian.xml";
	if (localizationCode == L"sc")
		return L"sardinian.xml";
	if (localizationCode == L"sr")
		return L"serbian.xml";
	if (localizationCode == L"sr-cyrl-ba" || localizationCode == L"sr-cyrl-sp")
		return L"serbianCyrillic.xml";
	if (localizationCode == L"si")
		return L"sinhala.xml";
	if (localizationCode == L"sk")
		return L"slovak.xml";
	if (localizationCode == L"sl")
		return L"slovenian.xml";
	if (localizationCode == L"es" || localizationCode == L"es-bo" || localizationCode == L"es-cl" || localizationCode == L"es-co" || localizationCode == L"es-cr" || localizationCode == L"es-do" || localizationCode == L"es-ec" || localizationCode == L"es-sv" || localizationCode == L"es-gt" || localizationCode == L"es-hn" || localizationCode == L"es-mx" || localizationCode == L"es-ni" || localizationCode == L"es-pa" || localizationCode == L"es-py" || localizationCode == L"es-pe" || localizationCode == L"es-pr" || localizationCode == L"es-es" || localizationCode == L"es-uy" || localizationCode == L"es-ve")
		return L"spanish.xml";
	if (localizationCode == L"es-ar")
		return L"spanish_ar.xml";
	if (localizationCode == L"sv")
		return L"swedish.xml";
	if (localizationCode == L"tl")
		return L"tagalog.xml";
	if (localizationCode == L"tg-cyrl-tj")
		return L"tajikCyrillic.xml";
	if (localizationCode == L"ta")
		return L"tamil.xml";
	if (localizationCode == L"tt")
		return L"tatar.xml";
	if (localizationCode == L"te")
		return L"telugu.xml";
	if (localizationCode == L"th")
		return L"thai.xml";
	if (localizationCode == L"tr")
		return L"turkish.xml";
	if (localizationCode == L"uk")
		return L"ukrainian.xml";
	if (localizationCode == L"ur" || localizationCode == L"ur-pk")
		return L"urdu.xml";
	if (localizationCode == L"ug-cn")
		return L"uyghur.xml";
	if (localizationCode == L"uz")
		return L"uzbek.xml";
	if (localizationCode == L"uz-cyrl-uz")
		return L"uzbekCyrillic.xml";
	if (localizationCode == L"vec")
		return L"venetian.xml";
	if (localizationCode == L"vi" || localizationCode == L"vi-vn")
		return L"vietnamese.xml";
	if (localizationCode == L"cy-gb")
		return L"welsh.xml";
	if (localizationCode == L"zu" || localizationCode == L"zu-za")
		return L"zulu.xml";

	return generic_string();
}


void NppParameters::feedKeyWordsParameters(TiXmlNode *node)
{
	TiXmlNode *langRoot = node->FirstChildElement(L"Languages");
	if (!langRoot)
		return;

	for (TiXmlNode *langNode = langRoot->FirstChildElement(L"Language");
		langNode ;
		langNode = langNode->NextSibling(L"Language") )
	{
		if (_nbLang < NB_LANG)
		{
			TiXmlElement* element = langNode->ToElement();
			const TCHAR* name = element->Attribute(L"name");
			if (name)
			{
				_langList[_nbLang] = new Lang(getLangIDFromStr(name), name);
				_langList[_nbLang]->setDefaultExtList(element->Attribute(L"ext"));
				_langList[_nbLang]->setCommentLineSymbol(element->Attribute(L"commentLine"));
				_langList[_nbLang]->setCommentStart(element->Attribute(L"commentStart"));
				_langList[_nbLang]->setCommentEnd(element->Attribute(L"commentEnd"));

				int tabSettings;
				if (element->Attribute(L"tabSettings", &tabSettings))
					_langList[_nbLang]->setTabInfo(tabSettings);

				for (TiXmlNode *kwNode = langNode->FirstChildElement(L"Keywords");
					kwNode ;
					kwNode = kwNode->NextSibling(L"Keywords") )
				{
					const TCHAR *indexName = (kwNode->ToElement())->Attribute(L"name");
					TiXmlNode *kwVal = kwNode->FirstChild();
					const TCHAR *keyWords = L"";
					if ((indexName) && (kwVal))
						keyWords = kwVal->Value();

					int i = getKwClassFromName(indexName);

					if (i >= 0 && i <= KEYWORDSET_MAX)
						_langList[_nbLang]->setWords(keyWords, i);
				}
				++_nbLang;
			}
		}
	}
}

extern "C" {
typedef DWORD (WINAPI * EESFUNC) (LPCTSTR, LPTSTR, DWORD);
}

void NppParameters::feedGUIParameters(TiXmlNode *node)
{
	TiXmlNode *GUIRoot = node->FirstChildElement(L"GUIConfigs");
	if (nullptr == GUIRoot)
		return;

	for (TiXmlNode *childNode = GUIRoot->FirstChildElement(L"GUIConfig");
		childNode ;
		childNode = childNode->NextSibling(L"GUIConfig") )
	{
		TiXmlElement* element = childNode->ToElement();
		const TCHAR* nm = element->Attribute(L"name");
		if (nullptr == nm)
			continue;

		if (!lstrcmp(nm, L"ToolBar"))
		{
			const TCHAR* val = element->Attribute(L"visible");
			if (val)
			{
				if (!lstrcmp(val, L"no"))
					_nppGUI._toolbarShow = false;
				else// if (!lstrcmp(val, L"yes"))
					_nppGUI._toolbarShow = true;
			}
			TiXmlNode *n = childNode->FirstChild();
			if (n)
			{
				val = n->Value();
				if (val)
				{
					if (!lstrcmp(val, L"small"))
						_nppGUI._toolBarStatus = TB_SMALL;
					else if (!lstrcmp(val, L"large"))
						_nppGUI._toolBarStatus = TB_LARGE;
					else// if (!lstrcmp(val, L"standard"))	//assume standard in all other cases
						_nppGUI._toolBarStatus = TB_STANDARD;
				}
			}
		}
		else if (!lstrcmp(nm, L"StatusBar"))
		{
			TiXmlNode *n = childNode->FirstChild();
			if (n)
			{
				const TCHAR* val = n->Value();
				if (val)
				{
					if (!lstrcmp(val, L"hide"))
						_nppGUI._statusBarShow = false;
					else if (!lstrcmp(val, L"show"))
						_nppGUI._statusBarShow = true;
				}
			}
		}
		else if (!lstrcmp(nm, L"MenuBar"))
		{
			TiXmlNode *n = childNode->FirstChild();
			if (n)
			{
				const TCHAR* val = n->Value();
				if (val)
				{
					if (!lstrcmp(val, L"hide"))
						_nppGUI._menuBarShow = false;
					else if (!lstrcmp(val, L"show"))
						_nppGUI._menuBarShow = true;
				}
			}
		}
		else if (!lstrcmp(nm, L"TabBar"))
		{
			bool isFailed = false;
			int oldValue = _nppGUI._tabStatus;
			const TCHAR* val = element->Attribute(L"dragAndDrop");
			if (val)
			{
				if (!lstrcmp(val, L"yes"))
					_nppGUI._tabStatus = TAB_DRAGNDROP;
				else if (!lstrcmp(val, L"no"))
					_nppGUI._tabStatus = 0;
				else
					isFailed = true;
			}

			val = element->Attribute(L"drawTopBar");
			if (val)
			{
				if (!lstrcmp(val, L"yes"))
					_nppGUI._tabStatus |= TAB_DRAWTOPBAR;
				else if (!lstrcmp(val, L"no"))
					_nppGUI._tabStatus |= 0;
				else
					isFailed = true;
			}

			val = element->Attribute(L"drawInactiveTab");
			if (val)
			{
				if (!lstrcmp(val, L"yes"))
					_nppGUI._tabStatus |= TAB_DRAWINACTIVETAB;
				else if (!lstrcmp(val, L"no"))
					_nppGUI._tabStatus |= 0;
				else
					isFailed = true;
			}

			val = element->Attribute(L"reduce");
			if (val)
			{
				if (!lstrcmp(val, L"yes"))
					_nppGUI._tabStatus |= TAB_REDUCE;
				else if (!lstrcmp(val, L"no"))
					_nppGUI._tabStatus |= 0;
				else
					isFailed = true;
			}

			val = element->Attribute(L"closeButton");
			if (val)
			{
				if (!lstrcmp(val, L"yes"))
					_nppGUI._tabStatus |= TAB_CLOSEBUTTON;
				else if (!lstrcmp(val, L"no"))
					_nppGUI._tabStatus |= 0;
				else
					isFailed = true;
			}

			val = element->Attribute(L"doubleClick2Close");
			if (val)
			{
				if (!lstrcmp(val, L"yes"))
					_nppGUI._tabStatus |= TAB_DBCLK2CLOSE;
				else if (!lstrcmp(val, L"no"))
					_nppGUI._tabStatus |= 0;
				else
					isFailed = true;
			}
			val = element->Attribute(L"vertical");
			if (val)
			{
				if (!lstrcmp(val, L"yes"))
					_nppGUI._tabStatus |= TAB_VERTICAL;
				else if (!lstrcmp(val, L"no"))
					_nppGUI._tabStatus |= 0;
				else
					isFailed = true;
			}

			val = element->Attribute(L"multiLine");
			if (val)
			{
				if (!lstrcmp(val, L"yes"))
					_nppGUI._tabStatus |= TAB_MULTILINE;
				else if (!lstrcmp(val, L"no"))
					_nppGUI._tabStatus |= 0;
				else
					isFailed = true;
			}

			val = element->Attribute(L"hide");
			if (val)
			{
				if (!lstrcmp(val, L"yes"))
					_nppGUI._tabStatus |= TAB_HIDE;
				else if (!lstrcmp(val, L"no"))
					_nppGUI._tabStatus |= 0;
				else
					isFailed = true;
			}

			val = element->Attribute(L"quitOnEmpty");
			if (val)
			{
				if (!lstrcmp(val, L"yes"))
					_nppGUI._tabStatus |= TAB_QUITONEMPTY;
				else if (!lstrcmp(val, L"no"))
					_nppGUI._tabStatus |= 0;
				else
					isFailed = true;
			}
			if (isFailed)
				_nppGUI._tabStatus = oldValue;


		}
		else if (!lstrcmp(nm, L"Auto-detection"))
		{
			TiXmlNode *n = childNode->FirstChild();
			if (n)
			{
				const TCHAR* val = n->Value();
				if (val)
				{
					if (!lstrcmp(val, L"yesOld"))
						_nppGUI._fileAutoDetection = cdEnabledOld;
					else if (!lstrcmp(val, L"autoOld"))
						_nppGUI._fileAutoDetection = (cdEnabledOld | cdAutoUpdate);
					else if (!lstrcmp(val, L"Update2EndOld"))
						_nppGUI._fileAutoDetection = (cdEnabledOld | cdGo2end);
					else if (!lstrcmp(val, L"autoUpdate2EndOld"))
						_nppGUI._fileAutoDetection = (cdEnabledOld | cdAutoUpdate | cdGo2end);
					else if (!lstrcmp(val, L"yes"))
						_nppGUI._fileAutoDetection = cdEnabledNew;
					else if (!lstrcmp(val, L"auto"))
						_nppGUI._fileAutoDetection = (cdEnabledNew | cdAutoUpdate);
					else if (!lstrcmp(val, L"Update2End"))
						_nppGUI._fileAutoDetection = (cdEnabledNew | cdGo2end);
					else if (!lstrcmp(val, L"autoUpdate2End"))
						_nppGUI._fileAutoDetection = (cdEnabledNew | cdAutoUpdate | cdGo2end);
					else //(!lstrcmp(val, L"no"))
						_nppGUI._fileAutoDetection = cdDisabled;
				}
			}
		}

		else if (!lstrcmp(nm, L"TrayIcon"))
		{
			TiXmlNode *n = childNode->FirstChild();
			if (n)
			{
				const TCHAR* val = n->Value();
				if (val)
				{
					_nppGUI._isMinimizedToTray = (lstrcmp(val, L"yes") == 0);
				}
			}
		}
		else if (!lstrcmp(nm, L"RememberLastSession"))
		{
			TiXmlNode *n = childNode->FirstChild();
			if (n)
			{
				const TCHAR* val = n->Value();
				if (val)
				{
					if (lstrcmp(val, L"yes") == 0)
						_nppGUI._rememberLastSession = true;
					else
						_nppGUI._rememberLastSession = false;
				}
			}
		}
		else if (!lstrcmp(nm, L"DetectEncoding"))
		{
			TiXmlNode *n = childNode->FirstChild();
			if (n)
			{
				const TCHAR* val = n->Value();
				if (val)
				{
					if (lstrcmp(val, L"yes") == 0)
						_nppGUI._detectEncoding = true;
					else
						_nppGUI._detectEncoding = false;
				}
			}
		}
		else if (lstrcmp(nm, L"MaitainIndent") == 0)
		{
			TiXmlNode *n = childNode->FirstChild();
			if (n)
			{
				const TCHAR* val = n->Value();
				if (val)
				{
					if (lstrcmp(val, L"yes") == 0)
						_nppGUI._maitainIndent = true;
					else
						_nppGUI._maitainIndent = false;
				}
			}
		}
		// <GUIConfig name="SmartHighLight" matchCase="yes" wholeWordOnly="yes" useFindSettings="no">yes</GUIConfig>
		else if (!lstrcmp(nm, L"SmartHighLight"))
		{
			TiXmlNode *n = childNode->FirstChild();
			if (n)
			{
				const TCHAR* val = n->Value();
				if (val)
				{
					if (lstrcmp(val, L"yes") == 0)
						_nppGUI._enableSmartHilite = true;
					else
						_nppGUI._enableSmartHilite = false;
				}

				val = element->Attribute(L"matchCase");
				if (val)
				{
					if (lstrcmp(val, L"yes") == 0)
						_nppGUI._smartHiliteCaseSensitive = true;
					else if (!lstrcmp(val, L"no"))
						_nppGUI._smartHiliteCaseSensitive = false;
				}

				val = element->Attribute(L"wholeWordOnly");
				if (val)
				{
					if (lstrcmp(val, L"yes") == 0)
						_nppGUI._smartHiliteWordOnly = true;
					else if (!lstrcmp(val, L"no"))
						_nppGUI._smartHiliteWordOnly = false;
				}

				val = element->Attribute(L"useFindSettings");
				if (val)
				{
					if (lstrcmp(val, L"yes") == 0)
						_nppGUI._smartHiliteUseFindSettings = true;
					else if (!lstrcmp(val, L"no"))
						_nppGUI._smartHiliteUseFindSettings = false;
				}

				val = element->Attribute(L"onAnotherView");
				if (val)
				{
					if (lstrcmp(val, L"yes") == 0)
						_nppGUI._smartHiliteOnAnotherView = true;
					else if (!lstrcmp(val, L"no"))
						_nppGUI._smartHiliteOnAnotherView = false;
				}
			}
		}

		else if (!lstrcmp(nm, L"TagsMatchHighLight"))
		{
			TiXmlNode *n = childNode->FirstChild();
			if (n)
			{
				const TCHAR* val = n->Value();
				if (val)
				{
					_nppGUI._enableTagsMatchHilite = !lstrcmp(val, L"yes");
					const TCHAR *tahl = element->Attribute(L"TagAttrHighLight");
					if (tahl)
						_nppGUI._enableTagAttrsHilite = !lstrcmp(tahl, L"yes");

					tahl = element->Attribute(L"HighLightNonHtmlZone");
					if (tahl)
						_nppGUI._enableHiliteNonHTMLZone = !lstrcmp(tahl, L"yes");
				}
			}
		}

		else if (!lstrcmp(nm, L"TaskList"))
		{
			TiXmlNode *n = childNode->FirstChild();
			if (n)
			{
				const TCHAR* val = n->Value();
				if (val)
				{
					_nppGUI._doTaskList = (!lstrcmp(val, L"yes"))?true:false;
				}
			}
		}

		else if (!lstrcmp(nm, L"MRU"))
		{
			TiXmlNode *n = childNode->FirstChild();
			if (n)
			{
				const TCHAR* val = n->Value();
				if (val)
					_nppGUI._styleMRU = (!lstrcmp(val, L"yes"));
			}
		}

		else if (!lstrcmp(nm, L"URL"))
		{
			TiXmlNode *n = childNode->FirstChild();
			if (n)
			{
				const TCHAR* val = n->Value();
				if (val)
				{
					if (!lstrcmp(val, L"1"))
						_nppGUI._styleURL = 1;
					else if (!lstrcmp(val, L"2"))
						_nppGUI._styleURL = 2;
					else
						_nppGUI._styleURL = 0;
				}
			}
		}

		else if (!lstrcmp(nm, L"CheckHistoryFiles"))
		{
			TiXmlNode *n = childNode->FirstChild();
			if (n)
			{
				const TCHAR* val = n->Value();
				if (val)
				{
					if (!lstrcmp(val, L"no"))
						_nppGUI._checkHistoryFiles = false;
					else if (!lstrcmp(val, L"yes"))
						_nppGUI._checkHistoryFiles = true;
				}
			}
		}
		else if (!lstrcmp(nm, L"ScintillaViewsSplitter"))
		{
			TiXmlNode *n = childNode->FirstChild();
			if (n)
			{
				const TCHAR* val = n->Value();
				if (val)
				{
					if (!lstrcmp(val, L"vertical"))
						_nppGUI._splitterPos = POS_VERTICAL;
					else if (!lstrcmp(val, L"horizontal"))
						_nppGUI._splitterPos = POS_HORIZOTAL;
				}
			}
		}
		else if (!lstrcmp(nm, L"UserDefineDlg"))
		{
			bool isFailed = false;
			int oldValue = _nppGUI._userDefineDlgStatus;

			TiXmlNode *n = childNode->FirstChild();
			if (n)
			{
				const TCHAR* val = n->Value();
				if (val)
				{
					if (!lstrcmp(val, L"hide"))
						_nppGUI._userDefineDlgStatus = 0;
					else if (!lstrcmp(val, L"show"))
						_nppGUI._userDefineDlgStatus = UDD_SHOW;
					else
						isFailed = true;
				}
			}

			const TCHAR* val = element->Attribute(L"position");
			if (val)
			{
				if (!lstrcmp(val, L"docked"))
					_nppGUI._userDefineDlgStatus |= UDD_DOCKED;
				else if (!lstrcmp(val, L"undocked"))
					_nppGUI._userDefineDlgStatus |= 0;
				else
					isFailed = true;
			}
			if (isFailed)
				_nppGUI._userDefineDlgStatus = oldValue;
		}
		else if (!lstrcmp(nm, L"TabSetting"))
		{
			int i;
			const TCHAR* val = element->Attribute(L"size", &i);
			if (val)
				_nppGUI._tabSize = i;

			if ((_nppGUI._tabSize == -1) || (_nppGUI._tabSize == 0))
				_nppGUI._tabSize = 4;

			val = element->Attribute(L"replaceBySpace");
			if (val)
				_nppGUI._tabReplacedBySpace = (!lstrcmp(val, L"yes"));
		}

		else if (!lstrcmp(nm, L"Caret"))
		{
			int i;
			const TCHAR* val = element->Attribute(L"width", &i);
			if (val)
				_nppGUI._caretWidth = i;

			val = element->Attribute(L"blinkRate", &i);
			if (val)
				_nppGUI._caretBlinkRate = i;
		}

		else if (!lstrcmp(nm, L"ScintillaGlobalSettings"))
		{
			const TCHAR* val = element->Attribute(L"enableMultiSelection");
			if (val)
			{
				if (lstrcmp(val, L"yes") == 0)
					_nppGUI._enableMultiSelection = true;
				else if (lstrcmp(val, L"no") == 0)
					_nppGUI._enableMultiSelection = false;
			}
		}

		else if (!lstrcmp(nm, L"AppPosition"))
		{
			RECT oldRect = _nppGUI._appPos;
			bool fuckUp = true;
			int i;

			if (element->Attribute(L"x", &i))
			{
				_nppGUI._appPos.left = i;

				if (element->Attribute(L"y", &i))
				{
					_nppGUI._appPos.top = i;

					if (element->Attribute(L"width", &i))
					{
						_nppGUI._appPos.right = i;

						if (element->Attribute(L"height", &i))
						{
							_nppGUI._appPos.bottom = i;
							fuckUp = false;
						}
					}
				}
			}
			if (fuckUp)
				_nppGUI._appPos = oldRect;

			const TCHAR* val = element->Attribute(L"isMaximized");
			if (val)
				_nppGUI._isMaximized = (lstrcmp(val, L"yes") == 0);
		}
		else if (!lstrcmp(nm, L"NewDocDefaultSettings"))
		{
			int i;
			if (element->Attribute(L"format", &i))
			{
				EolType newFormat = EolType::osdefault;
				switch (i)
				{
					case static_cast<LPARAM>(EolType::windows) :
						newFormat = EolType::windows;
						break;
					case static_cast<LPARAM>(EolType::macos) :
						newFormat = EolType::macos;
						break;
					case static_cast<LPARAM>(EolType::unix) :
						newFormat = EolType::unix;
						break;
					default:
						assert(false and "invalid buffer format - fallback to default");
				}
				_nppGUI._newDocDefaultSettings._format = newFormat;
			}

			if (element->Attribute(L"encoding", &i))
				_nppGUI._newDocDefaultSettings._unicodeMode = (UniMode)i;

			if (element->Attribute(L"lang", &i))
				_nppGUI._newDocDefaultSettings._lang = (LangType)i;

			if (element->Attribute(L"codepage", &i))
				_nppGUI._newDocDefaultSettings._codepage = (LangType)i;

			const TCHAR* val = element->Attribute(L"openAnsiAsUTF8");
			if (val)
				_nppGUI._newDocDefaultSettings._openAnsiAsUtf8 = (lstrcmp(val, L"yes") == 0);

		}
		else if (!lstrcmp(nm, L"langsExcluded"))
		{
			// TODO
			int g0 = 0; // up to 8
			int g1 = 0; // up to 16
			int g2 = 0; // up to 24
			int g3 = 0; // up to 32
			int g4 = 0; // up to 40
			int g5 = 0; // up to 48
			int g6 = 0; // up to 56
			int g7 = 0; // up to 64
			int g8 = 0; // up to 72
			int g9 = 0; // up to 80
			int g10= 0; // up to 88
			int g11= 0; // up to 96
			int g12= 0; // up to 104

			// TODO some refactoring needed here....
			{
				int i;
				if (element->Attribute(L"gr0", &i))
				{
					if (i <= 255)
						g0 = i;
				}
				if (element->Attribute(L"gr1", &i))
				{
					if (i <= 255)
						g1 = i;
				}
				if (element->Attribute(L"gr2", &i))
				{
					if (i <= 255)
						g2 = i;
				}
				if (element->Attribute(L"gr3", &i))
				{
					if (i <= 255)
						g3 = i;
				}
				if (element->Attribute(L"gr4", &i))
				{
					if (i <= 255)
						g4 = i;
				}
				if (element->Attribute(L"gr5", &i))
				{
					if (i <= 255)
						g5 = i;
				}
				if (element->Attribute(L"gr6", &i))
				{
					if (i <= 255)
						g6 = i;
				}
				if (element->Attribute(L"gr7", &i))
				{
					if (i <= 255)
						g7 = i;
				}
				if (element->Attribute(L"gr8", &i))
				{
					if (i <= 255)
						g8 = i;
				}
				if (element->Attribute(L"gr9", &i))
				{
					if (i <= 255)
						g9 = i;
				}
				if (element->Attribute(L"gr10", &i))
				{
					if (i <= 255)
						g10 = i;
				}
				if (element->Attribute(L"gr11", &i))
				{
					if (i <= 255)
						g11 = i;
				}
				if (element->Attribute(L"gr12", &i))
				{
					if (i <= 255)
						g12 = i;
				}
			}

			UCHAR mask = 1;
			for (int i = 0 ; i < 8 ; ++i)
			{
				if (mask & g0)
					_nppGUI._excludedLangList.push_back(LangMenuItem((LangType)i));
				mask <<= 1;
			}

			mask = 1;
			for (int i = 8 ; i < 16 ; ++i)
			{
				if (mask & g1)
					_nppGUI._excludedLangList.push_back(LangMenuItem((LangType)i));
				mask <<= 1;
			}

			mask = 1;
			for (int i = 16 ; i < 24 ; ++i)
			{
				if (mask & g2)
					_nppGUI._excludedLangList.push_back(LangMenuItem((LangType)i));
				mask <<= 1;
			}

			mask = 1;
			for (int i = 24 ; i < 32 ; ++i)
			{
				if (mask & g3)
					_nppGUI._excludedLangList.push_back(LangMenuItem((LangType)i));
				mask <<= 1;
			}

			mask = 1;
			for (int i = 32 ; i < 40 ; ++i)
			{
				if (mask & g4)
					_nppGUI._excludedLangList.push_back(LangMenuItem((LangType)i));
				mask <<= 1;
			}

			mask = 1;
			for (int i = 40 ; i < 48 ; ++i)
			{
				if (mask & g5)
					_nppGUI._excludedLangList.push_back(LangMenuItem((LangType)i));
				mask <<= 1;
			}

			mask = 1;
			for (int i = 48 ; i < 56 ; ++i)
			{
				if (mask & g6)
					_nppGUI._excludedLangList.push_back(LangMenuItem((LangType)i));
				mask <<= 1;
			}

			mask = 1;
			for (int i = 56 ; i < 64 ; ++i)
			{
				if (mask & g7)
					_nppGUI._excludedLangList.push_back(LangMenuItem((LangType)i));
				mask <<= 1;
			}

			mask = 1;
			for (int i = 64; i < 72; ++i)
			{
				if (mask & g8)
					_nppGUI._excludedLangList.push_back(LangMenuItem((LangType)i));
				mask <<= 1;
			}

			mask = 1;
			for (int i = 72; i < 80; ++i)
			{
				if (mask & g9)
					_nppGUI._excludedLangList.push_back(LangMenuItem((LangType)i));
				mask <<= 1;
			}

			mask = 1;
			for (int i = 80; i < 88; ++i)
			{
				if (mask & g10)
					_nppGUI._excludedLangList.push_back(LangMenuItem((LangType)i));
				mask <<= 1;
			}

			mask = 1;
			for (int i = 88; i < 96; ++i)
			{
				if (mask & g11)
					_nppGUI._excludedLangList.push_back(LangMenuItem((LangType)i));
				mask <<= 1;
			}

			mask = 1;
			for (int i = 96; i < 104; ++i)
			{
				if (mask & g12)
					_nppGUI._excludedLangList.push_back(LangMenuItem((LangType)i));
				mask <<= 1;
			}

			const TCHAR* val = element->Attribute(L"langMenuCompact");
			if (val)
				_nppGUI._isLangMenuCompact = (!lstrcmp(val, L"yes"));
		}

		else if (!lstrcmp(nm, L"Print"))
		{
			const TCHAR* val = element->Attribute(L"lineNumber");
			if (val)
				_nppGUI._printSettings._printLineNumber = (!lstrcmp(val, L"yes"));

			int i;
			if (element->Attribute(L"printOption", &i))
				_nppGUI._printSettings._printOption = i;

			val = element->Attribute(L"headerLeft");
			if (val)
				_nppGUI._printSettings._headerLeft = val;

			val = element->Attribute(L"headerMiddle");
			if (val)
				_nppGUI._printSettings._headerMiddle = val;

			val = element->Attribute(L"headerRight");
			if (val)
				_nppGUI._printSettings._headerRight = val;


			val = element->Attribute(L"footerLeft");
			if (val)
				_nppGUI._printSettings._footerLeft = val;

			val = element->Attribute(L"footerMiddle");
			if (val)
				_nppGUI._printSettings._footerMiddle = val;

			val = element->Attribute(L"footerRight");
			if (val)
				_nppGUI._printSettings._footerRight = val;


			val = element->Attribute(L"headerFontName");
			if (val)
				_nppGUI._printSettings._headerFontName = val;

			val = element->Attribute(L"footerFontName");
			if (val)
				_nppGUI._printSettings._footerFontName = val;

			if (element->Attribute(L"headerFontStyle", &i))
				_nppGUI._printSettings._headerFontStyle = i;

			if (element->Attribute(L"footerFontStyle", &i))
				_nppGUI._printSettings._footerFontStyle = i;

			if (element->Attribute(L"headerFontSize", &i))
				_nppGUI._printSettings._headerFontSize = i;

			if (element->Attribute(L"footerFontSize", &i))
				_nppGUI._printSettings._footerFontSize = i;


			if (element->Attribute(L"margeLeft", &i))
				_nppGUI._printSettings._marge.left = i;

			if (element->Attribute(L"margeTop", &i))
				_nppGUI._printSettings._marge.top = i;

			if (element->Attribute(L"margeRight", &i))
				_nppGUI._printSettings._marge.right = i;

			if (element->Attribute(L"margeBottom", &i))
				_nppGUI._printSettings._marge.bottom = i;
		}

		else if (!lstrcmp(nm, L"ScintillaPrimaryView"))
		{
			feedScintillaParam(element);
		}

		else if (!lstrcmp(nm, L"Backup"))
		{
			int i;
			if (element->Attribute(L"action", &i))
				_nppGUI._backup = (BackupFeature)i;

			const TCHAR *bDir = element->Attribute(L"useCustumDir");
			if (bDir)
			{
				_nppGUI._useDir = (lstrcmp(bDir, L"yes") == 0);;
			}
			const TCHAR *pDir = element->Attribute(L"dir");
			if (pDir)
				_nppGUI._backupDir = pDir;

			const TCHAR *isSnapshotModeStr = element->Attribute(L"isSnapshotMode");
			if (isSnapshotModeStr && !lstrcmp(isSnapshotModeStr, L"no"))
				_nppGUI._isSnapshotMode = false;

			int timing;
			if (element->Attribute(L"snapshotBackupTiming", &timing))
				_nppGUI._snapshotBackupTiming = timing;

		}
		else if (!lstrcmp(nm, L"DockingManager"))
		{
			feedDockingManager(element);
		}

		else if (!lstrcmp(nm, L"globalOverride"))
		{
			const TCHAR *bDir = element->Attribute(L"fg");
			if (bDir)
				_nppGUI._globalOverride.enableFg = (lstrcmp(bDir, L"yes") == 0);

			bDir = element->Attribute(L"bg");
			if (bDir)
				_nppGUI._globalOverride.enableBg = (lstrcmp(bDir, L"yes") == 0);

			bDir = element->Attribute(L"font");
			if (bDir)
				_nppGUI._globalOverride.enableFont = (lstrcmp(bDir, L"yes") == 0);

			bDir = element->Attribute(L"fontSize");
			if (bDir)
				_nppGUI._globalOverride.enableFontSize = (lstrcmp(bDir, L"yes") == 0);

			bDir = element->Attribute(L"bold");
			if (bDir)
				_nppGUI._globalOverride.enableBold = (lstrcmp(bDir, L"yes") == 0);

			bDir = element->Attribute(L"italic");
			if (bDir)
				_nppGUI._globalOverride.enableItalic = (lstrcmp(bDir, L"yes") == 0);

			bDir = element->Attribute(L"underline");
			if (bDir)
				_nppGUI._globalOverride.enableUnderLine = (lstrcmp(bDir, L"yes") == 0);
		}
		else if (!lstrcmp(nm, L"auto-completion"))
		{
			int i;
			if (element->Attribute(L"autoCAction", &i))
				_nppGUI._autocStatus = static_cast<NppGUI::AutocStatus>(i);

			if (element->Attribute(L"triggerFromNbChar", &i))
				_nppGUI._autocFromLen = i;

			const TCHAR * optName = element->Attribute(L"autoCIgnoreNumbers");
			if (optName)
				_nppGUI._autocIgnoreNumbers = (lstrcmp(optName, L"yes") == 0);

			optName = element->Attribute(L"funcParams");
			if (optName)
				_nppGUI._funcParams = (lstrcmp(optName, L"yes") == 0);
		}
		else if (!lstrcmp(nm, L"auto-insert"))
		{
			const TCHAR * optName = element->Attribute(L"htmlXmlTag");
			if (optName)
				_nppGUI._matchedPairConf._doHtmlXmlTag = (lstrcmp(optName, L"yes") == 0);

			optName = element->Attribute(L"parentheses");
			if (optName)
				_nppGUI._matchedPairConf._doParentheses = (lstrcmp(optName, L"yes") == 0);

			optName = element->Attribute(L"brackets");
			if (optName)
				_nppGUI._matchedPairConf._doBrackets = (lstrcmp(optName, L"yes") == 0);

			optName = element->Attribute(L"curlyBrackets");
			if (optName)
				_nppGUI._matchedPairConf._doCurlyBrackets = (lstrcmp(optName, L"yes") == 0);

			optName = element->Attribute(L"quotes");
			if (optName)
				_nppGUI._matchedPairConf._doQuotes = (lstrcmp(optName, L"yes") == 0);

			optName = element->Attribute(L"doubleQuotes");
			if (optName)
				_nppGUI._matchedPairConf._doDoubleQuotes = (lstrcmp(optName, L"yes") == 0);

			for (TiXmlNode *subChildNode = childNode->FirstChildElement(L"UserDefinePair");
				 subChildNode;
				 subChildNode = subChildNode->NextSibling(L"UserDefinePair") )
			{
				int open = -1;
				int openVal = 0;
				const TCHAR *openValStr = (subChildNode->ToElement())->Attribute(L"open", &openVal);
				if (openValStr && (openVal >= 0 && openVal < 128))
					open = openVal;

				int close = -1;
				int closeVal = 0;
				const TCHAR *closeValStr = (subChildNode->ToElement())->Attribute(L"close", &closeVal);
				if (closeValStr && (closeVal >= 0 && closeVal <= 128))
					close = closeVal;

				if (open != -1 && close != -1)
					_nppGUI._matchedPairConf._matchedPairsInit.push_back(pair<char, char>(char(open), char(close)));
			}
		}
		else if (!lstrcmp(nm, L"sessionExt"))
		{
			TiXmlNode *n = childNode->FirstChild();
			if (n)
			{
				const TCHAR* val = n->Value();
				if (val)
					_nppGUI._definedSessionExt = val;
			}
		}
		else if (!lstrcmp(nm, L"workspaceExt"))
		{
			TiXmlNode *n = childNode->FirstChild();
			if (n)
			{
				const TCHAR* val = n->Value();
				if (val)
					_nppGUI._definedWorkspaceExt = val;
			}
		}
		else if (!lstrcmp(nm, L"noUpdate"))
		{
			TiXmlNode *n = childNode->FirstChild();
			if (n)
			{
				const TCHAR* val = n->Value();
				if (val)
					_nppGUI._autoUpdateOpt._doAutoUpdate = (!lstrcmp(val, L"yes"))?false:true;

				int i;
				val = element->Attribute(L"intervalDays", &i);
				if (val)
					_nppGUI._autoUpdateOpt._intervalDays = i;

				val = element->Attribute(L"nextUpdateDate");
				if (val)
					_nppGUI._autoUpdateOpt._nextUpdateDate = Date(val);
			}
		}
		else if (!lstrcmp(nm, L"openSaveDir"))
		{
			const TCHAR * value = element->Attribute(L"value");
			if (value && value[0])
			{
				if (lstrcmp(value, L"1") == 0)
					_nppGUI._openSaveDir = dir_last;
				else if (lstrcmp(value, L"2") == 0)
					_nppGUI._openSaveDir = dir_userDef;
				else
					_nppGUI._openSaveDir = dir_followCurrent;
			}

			const TCHAR * path = element->Attribute(L"defaultDirPath");
			if (path && path[0])
			{
				lstrcpyn(_nppGUI._defaultDir, path, MAX_PATH);
				::ExpandEnvironmentStrings(_nppGUI._defaultDir, _nppGUI._defaultDirExp, MAX_PATH);
			}
 		}
		else if (!lstrcmp(nm, L"titleBar"))
		{
			const TCHAR * value = element->Attribute(L"short");
			_nppGUI._shortTitlebar = false;	//default state
			if (value && value[0])
			{
				if (lstrcmp(value, L"yes") == 0)
					_nppGUI._shortTitlebar = true;
				else if (lstrcmp(value, L"no") == 0)
					_nppGUI._shortTitlebar = false;
			}
		}
		else if (!lstrcmp(nm, L"stylerTheme"))
		{
			const TCHAR *themePath = element->Attribute(L"path");
			if (themePath != NULL && themePath[0])
				_nppGUI._themeName.assign(themePath);
		}
		else if (!lstrcmp(nm, L"wordCharList"))
		{
			const TCHAR * value = element->Attribute(L"useDefault");
			if (value && value[0])
			{
				if (lstrcmp(value, L"yes") == 0)
					_nppGUI._isWordCharDefault = true;
				else if (lstrcmp(value, L"no") == 0)
					_nppGUI._isWordCharDefault = false;
			}

			const TCHAR *charsAddedW = element->Attribute(L"charsAdded");
			if (charsAddedW)
			{
				WcharMbcsConvertor& wmc = WcharMbcsConvertor::getInstance();
				_nppGUI._customWordChars = wmc.wchar2char(charsAddedW, SC_CP_UTF8);
			}
		}
		else if (!lstrcmp(nm, L"delimiterSelection"))
		{
			int leftmost = 0;
			element->Attribute(L"leftmostDelimiter", &leftmost);
			if (leftmost > 0 && leftmost < 256)
				_nppGUI._leftmostDelimiter = static_cast<char>(leftmost);

			int rightmost = 0;
			element->Attribute(L"rightmostDelimiter", &rightmost);
			if (rightmost > 0 && rightmost < 256)
				_nppGUI._rightmostDelimiter = static_cast<char>(rightmost);

			const TCHAR *delimiterSelectionOnEntireDocument = element->Attribute(L"delimiterSelectionOnEntireDocument");
			if (delimiterSelectionOnEntireDocument != NULL && !lstrcmp(delimiterSelectionOnEntireDocument, L"yes"))
				_nppGUI._delimiterSelectionOnEntireDocument = true;
			else
				_nppGUI._delimiterSelectionOnEntireDocument = false;
		}
		else if (!lstrcmp(nm, L"multiInst"))
		{
			int val = 0;
			element->Attribute(L"setting", &val);
			if (val < 0 || val > 2)
				val = 0;
			_nppGUI._multiInstSetting = (MultiInstSetting)val;
		}
		else if (!lstrcmp(nm, L"searchEngine"))
		{
			int i;
			if (element->Attribute(L"searchEngineChoice", &i))
				_nppGUI._searchEngineChoice = static_cast<NppGUI::SearchEngineChoice>(i);

			const TCHAR * searchEngineCustom = element->Attribute(L"searchEngineCustom");
			if (searchEngineCustom && searchEngineCustom[0])
				_nppGUI._searchEngineCustom = searchEngineCustom;
		}
		else if (!lstrcmp(nm, L"MISC"))
		{
			const TCHAR * optName = element->Attribute(L"fileSwitcherWithoutExtColumn");
			if (optName)
				_nppGUI._fileSwitcherWithoutExtColumn = (lstrcmp(optName, L"yes") == 0);

			const TCHAR * optNameBackSlashEscape = element->Attribute(L"backSlashIsEscapeCharacterForSql");
			if (optNameBackSlashEscape && !lstrcmp(optNameBackSlashEscape, L"no"))
				_nppGUI._backSlashIsEscapeCharacterForSql = false;

			const TCHAR * optNameMonoFont = element->Attribute(L"monospacedFontFindDlg");
			if (optNameMonoFont)
				_nppGUI._monospacedFontFindDlg = (lstrcmp(optNameMonoFont, L"yes") == 0);

			const TCHAR * optStopFillingFindField = element->Attribute(L"stopFillingFindField");
			if (optStopFillingFindField)
				_nppGUI._stopFillingFindField = (lstrcmp(optStopFillingFindField, L"yes") == 0);

			const TCHAR * optNameNewStyleSaveDlg = element->Attribute(L"newStyleSaveDlg");
			if (optNameNewStyleSaveDlg)
				_nppGUI._useNewStyleSaveDlg = (lstrcmp(optNameNewStyleSaveDlg, L"yes") == 0);

			const TCHAR * optNameFolderDroppedOpenFiles = element->Attribute(L"isFolderDroppedOpenFiles");
			if (optNameFolderDroppedOpenFiles)
				_nppGUI._isFolderDroppedOpenFiles = (lstrcmp(optNameFolderDroppedOpenFiles, L"yes") == 0);

			const TCHAR * optDocPeekOnTab = element->Attribute(L"docPeekOnTab");
			if (optDocPeekOnTab)
				_nppGUI._isDocPeekOnTab = (lstrcmp(optDocPeekOnTab, L"yes") == 0);

			const TCHAR * optDocPeekOnMap = element->Attribute(L"docPeekOnMap");
			if (optDocPeekOnMap)
				_nppGUI._isDocPeekOnMap = (lstrcmp(optDocPeekOnMap, L"yes") == 0);
		}
		else if (!lstrcmp(nm, L"commandLineInterpreter"))
		{
			TiXmlNode *node = childNode->FirstChild();
			if (node)
			{
				const TCHAR *cli = node->Value();
				if (cli && cli[0])
					_nppGUI._commandLineInterpreter.assign(cli);
			}
		}
	}
}

void NppParameters::feedScintillaParam(TiXmlNode *node)
{
	TiXmlElement* element = node->ToElement();

	// Line Number Margin
	const TCHAR *nm = element->Attribute(L"lineNumberMargin");
	// if (nm)
	// {
		if (!lstrcmp(nm, L"show"))
			_svp._lineNumberMarginShow = true;
		else if (!lstrcmp(nm, L"hide"))
			_svp._lineNumberMarginShow = false;
	// }

	// Bookmark Margin
	nm = element->Attribute(L"bookMarkMargin");
	if (nm)
	{

		if (!lstrcmp(nm, L"show"))
			_svp._bookMarkMarginShow = true;
		else if (!lstrcmp(nm, L"hide"))
			_svp._bookMarkMarginShow = false;
	}

	// Indent GuideLine
	nm = element->Attribute(L"indentGuideLine");
	if (nm)
	{
		if (!lstrcmp(nm, L"show"))
			_svp._indentGuideLineShow = true;
		else if (!lstrcmp(nm, L"hide"))
			_svp._indentGuideLineShow= false;
	}

	// Folder Mark Style
	nm = element->Attribute(L"folderMarkStyle");
	if (nm)
	{
		if (!lstrcmp(nm, L"box"))
			_svp._folderStyle = FOLDER_STYLE_BOX;
		else if (!lstrcmp(nm, L"circle"))
			_svp._folderStyle = FOLDER_STYLE_CIRCLE;
		else if (!lstrcmp(nm, L"arrow"))
			_svp._folderStyle = FOLDER_STYLE_ARROW;
		else if (!lstrcmp(nm, L"simple"))
			_svp._folderStyle = FOLDER_STYLE_SIMPLE;
		else if (!lstrcmp(nm, L"none"))
			_svp._folderStyle = FOLDER_STYLE_NONE;
	}

	// Line Wrap method
	nm = element->Attribute(L"lineWrapMethod");
	if (nm)
	{
		if (!lstrcmp(nm, L"default"))
			_svp._lineWrapMethod = LINEWRAP_DEFAULT;
		else if (!lstrcmp(nm, L"aligned"))
			_svp._lineWrapMethod = LINEWRAP_ALIGNED;
		else if (!lstrcmp(nm, L"indent"))
			_svp._lineWrapMethod = LINEWRAP_INDENT;
	}

	// Current Line Highlighting State
	nm = element->Attribute(L"currentLineHilitingShow");
	if (nm)
	{
		if (!lstrcmp(nm, L"show"))
			_svp._currentLineHilitingShow = true;
		else if (!lstrcmp(nm, L"hide"))
			_svp._currentLineHilitingShow = false;
	}

	// Scrolling Beyond Last Line State
	nm = element->Attribute(L"scrollBeyondLastLine");
	if (nm)
	{
		if (!lstrcmp(nm, L"yes"))
			_svp._scrollBeyondLastLine = true;
		else if (!lstrcmp(nm, L"no"))
			_svp._scrollBeyondLastLine = false;
	}

	// Disable Advanced Scrolling
	nm = element->Attribute(L"disableAdvancedScrolling");
	if (nm)
	{
		if (!lstrcmp(nm, L"yes"))
			_svp._disableAdvancedScrolling = true;
		else if (!lstrcmp(nm, L"no"))
			_svp._disableAdvancedScrolling = false;
	}

	// Current wrap symbol visibility State
	nm = element->Attribute(L"wrapSymbolShow");
	if (nm)
	{
		if (!lstrcmp(nm, L"show"))
			_svp._wrapSymbolShow = true;
		else if (!lstrcmp(nm, L"hide"))
			_svp._wrapSymbolShow = false;
	}

	// Do Wrap
	nm = element->Attribute(L"Wrap");
	if (nm)
	{
		if (!lstrcmp(nm, L"yes"))
			_svp._doWrap = true;
		else if (!lstrcmp(nm, L"no"))
			_svp._doWrap = false;
	}

	// Do Edge
	nm = element->Attribute(L"edge");
	if (nm)
	{
		if (!lstrcmp(nm, L"background"))
			_svp._edgeMode = EDGE_BACKGROUND;
		else if (!lstrcmp(nm, L"line"))
			_svp._edgeMode = EDGE_LINE;
		else
			_svp._edgeMode = EDGE_NONE;
	}

	// Do Scintilla border edge
	nm = element->Attribute(L"borderEdge");
	if (nm)
	{
		if (!lstrcmp(nm, L"yes"))
			_svp._showBorderEdge = true;
		else if (!lstrcmp(nm, L"no"))
			_svp._showBorderEdge = false;
	}

	int val;
	nm = element->Attribute(L"edgeNbColumn", &val);
	if (nm)
	{
		_svp._edgeNbColumn = val;
	}

	nm = element->Attribute(L"zoom", &val);
	if (nm)
	{
		_svp._zoom = val;
	}

	nm = element->Attribute(L"zoom2", &val);
	if (nm)
	{
		_svp._zoom2 = val;
	}

	// White Space visibility State
	nm = element->Attribute(L"whiteSpaceShow");
	if (nm)
	{
		if (!lstrcmp(nm, L"show"))
			_svp._whiteSpaceShow = true;
		else if (!lstrcmp(nm, L"hide"))
			_svp._whiteSpaceShow = false;
	}

	// EOL visibility State
	nm = element->Attribute(L"eolShow");
	if (nm)
	{
		if (!lstrcmp(nm, L"show"))
			_svp._eolShow = true;
		else if (!lstrcmp(nm, L"hide"))
			_svp._eolShow = false;
	}

	nm = element->Attribute(L"borderWidth", &val);
	if (nm)
	{
		if (val >= 0 && val <= 30)
			_svp._borderWidth = val;
	}

	// Do antialiased font
	nm = element->Attribute(L"smoothFont");
	if (nm)
	{
		if (!lstrcmp(nm, L"yes"))
			_svp._doSmoothFont = true;
		else if (!lstrcmp(nm, L"no"))
			_svp._doSmoothFont = false;
	}
}


void NppParameters::feedDockingManager(TiXmlNode *node)
{
	TiXmlElement *element = node->ToElement();

	int i;
	if (element->Attribute(L"leftWidth", &i))
		_nppGUI._dockingData._leftWidth = i;

	if (element->Attribute(L"rightWidth", &i))
		_nppGUI._dockingData._rightWidth = i;

	if (element->Attribute(L"topHeight", &i))
		_nppGUI._dockingData._topHeight = i;

	if (element->Attribute(L"bottomHeight", &i))
		_nppGUI._dockingData._bottomHight = i;



	for (TiXmlNode *childNode = node->FirstChildElement(L"FloatingWindow");
		childNode ;
		childNode = childNode->NextSibling(L"FloatingWindow") )
	{
		TiXmlElement *floatElement = childNode->ToElement();
		int cont;
		if (floatElement->Attribute(L"cont", &cont))
		{
			int x = 0;
			int y = 0;
			int w = 100;
			int h = 100;

			floatElement->Attribute(L"x", &x);
			floatElement->Attribute(L"y", &y);
			floatElement->Attribute(L"width", &w);
			floatElement->Attribute(L"height", &h);
			_nppGUI._dockingData._flaotingWindowInfo.push_back(FloatingWindowInfo(cont, x, y, w, h));
		}
	}

	for (TiXmlNode *childNode = node->FirstChildElement(L"PluginDlg");
		childNode ;
		childNode = childNode->NextSibling(L"PluginDlg") )
	{
		TiXmlElement *dlgElement = childNode->ToElement();
		const TCHAR *name = dlgElement->Attribute(L"pluginName");

		int id;
		const TCHAR *idStr = dlgElement->Attribute(L"id", &id);
		if (name && idStr)
		{
			int curr = 0; // on left
			int prev = 0; // on left

			dlgElement->Attribute(L"curr", &curr);
			dlgElement->Attribute(L"prev", &prev);

			bool isVisible = false;
			const TCHAR *val = dlgElement->Attribute(L"isVisible");
			if (val)
			{
				isVisible = (lstrcmp(val, L"yes") == 0);
			}

			_nppGUI._dockingData._pluginDockInfo.push_back(PluginDlgDockingInfo(name, id, curr, prev, isVisible));
		}
	}

	for (TiXmlNode *childNode = node->FirstChildElement(L"ActiveTabs");
		childNode ;
		childNode = childNode->NextSibling(L"ActiveTabs") )
	{
		TiXmlElement *dlgElement = childNode->ToElement();

		int cont;
		if (dlgElement->Attribute(L"cont", &cont))
		{
			int activeTab = 0;
			dlgElement->Attribute(L"activeTab", &activeTab);
			_nppGUI._dockingData._containerTabInfo.push_back(ContainerTabInfo(cont, activeTab));
		}
	}
}

bool NppParameters::writeScintillaParams()
{
	if (!_pXmlUserDoc) return false;

	const TCHAR *pViewName = L"ScintillaPrimaryView";
	TiXmlNode *nppRoot = _pXmlUserDoc->FirstChild(L"NotepadPlus");
	if (not nppRoot)
	{
		nppRoot = _pXmlUserDoc->InsertEndChild(TiXmlElement(L"NotepadPlus"));
	}

	TiXmlNode *configsRoot = nppRoot->FirstChildElement(L"GUIConfigs");
	if (not configsRoot)
	{
		configsRoot = nppRoot->InsertEndChild(TiXmlElement(L"GUIConfigs"));
	}

	TiXmlNode *scintNode = getChildElementByAttribut(configsRoot, L"GUIConfig", L"name", pViewName);
	if (not scintNode)
	{
		scintNode = configsRoot->InsertEndChild(TiXmlElement(L"GUIConfig"));
		(scintNode->ToElement())->SetAttribute(L"name", pViewName);
	}

	(scintNode->ToElement())->SetAttribute(L"lineNumberMargin", _svp._lineNumberMarginShow?L"show":L"hide");
	(scintNode->ToElement())->SetAttribute(L"bookMarkMargin", _svp._bookMarkMarginShow?L"show":L"hide");
	(scintNode->ToElement())->SetAttribute(L"indentGuideLine", _svp._indentGuideLineShow?L"show":L"hide");
	const TCHAR *pFolderStyleStr = (_svp._folderStyle == FOLDER_STYLE_SIMPLE)?L"simple":
									(_svp._folderStyle == FOLDER_STYLE_ARROW)?L"arrow":
										(_svp._folderStyle == FOLDER_STYLE_CIRCLE)?L"circle":
										(_svp._folderStyle == FOLDER_STYLE_NONE)?L"none":L"box";
	(scintNode->ToElement())->SetAttribute(L"folderMarkStyle", pFolderStyleStr);

	const TCHAR *pWrapMethodStr = (_svp._lineWrapMethod == LINEWRAP_ALIGNED)?L"aligned":
								(_svp._lineWrapMethod == LINEWRAP_INDENT)?L"indent":L"default";
	(scintNode->ToElement())->SetAttribute(L"lineWrapMethod", pWrapMethodStr);

	(scintNode->ToElement())->SetAttribute(L"currentLineHilitingShow", _svp._currentLineHilitingShow?L"show":L"hide");
	(scintNode->ToElement())->SetAttribute(L"scrollBeyondLastLine", _svp._scrollBeyondLastLine?L"yes":L"no");
	(scintNode->ToElement())->SetAttribute(L"disableAdvancedScrolling", _svp._disableAdvancedScrolling?L"yes":L"no");
	(scintNode->ToElement())->SetAttribute(L"wrapSymbolShow", _svp._wrapSymbolShow?L"show":L"hide");
	(scintNode->ToElement())->SetAttribute(L"Wrap", _svp._doWrap?L"yes":L"no");
	(scintNode->ToElement())->SetAttribute(L"borderEdge", _svp._showBorderEdge ? L"yes" : L"no");

	const TCHAR *edgeStr;
	if (_svp._edgeMode == EDGE_NONE)
		edgeStr = L"no";
	else if (_svp._edgeMode == EDGE_LINE)
		edgeStr = L"line";
	else
		edgeStr = L"background";
	(scintNode->ToElement())->SetAttribute(L"edge", edgeStr);
	(scintNode->ToElement())->SetAttribute(L"edgeNbColumn", _svp._edgeNbColumn);
	(scintNode->ToElement())->SetAttribute(L"zoom", _svp._zoom);
	(scintNode->ToElement())->SetAttribute(L"zoom2", _svp._zoom2);
	(scintNode->ToElement())->SetAttribute(L"whiteSpaceShow", _svp._whiteSpaceShow?L"show":L"hide");
	(scintNode->ToElement())->SetAttribute(L"eolShow", _svp._eolShow?L"show":L"hide");
	(scintNode->ToElement())->SetAttribute(L"borderWidth", _svp._borderWidth);
	(scintNode->ToElement())->SetAttribute(L"smoothFont", _svp._doSmoothFont ? L"yes" : L"no");
	return true;
}

void NppParameters::createXmlTreeFromGUIParams()
{
	TiXmlNode *nppRoot = _pXmlUserDoc->FirstChild(L"NotepadPlus");
	if (not nppRoot)
	{
		nppRoot = _pXmlUserDoc->InsertEndChild(TiXmlElement(L"NotepadPlus"));
	}

	TiXmlNode *oldGUIRoot = nppRoot->FirstChildElement(L"GUIConfigs");
	// Remove the old root nod if it exist
	if (oldGUIRoot)
	{
		nppRoot->RemoveChild(oldGUIRoot);
	}

	TiXmlNode *newGUIRoot = nppRoot->InsertEndChild(TiXmlElement(L"GUIConfigs"));

	// <GUIConfig name="ToolBar" visible="yes">standard</GUIConfig>
	{
		TiXmlElement *GUIConfigElement = (newGUIRoot->InsertEndChild(TiXmlElement(L"GUIConfig")))->ToElement();
		GUIConfigElement->SetAttribute(L"name", L"ToolBar");
		const TCHAR *pStr = (_nppGUI._toolbarShow) ? L"yes" : L"no";
		GUIConfigElement->SetAttribute(L"visible", pStr);
		pStr = _nppGUI._toolBarStatus == TB_SMALL ? L"small" : (_nppGUI._toolBarStatus == TB_STANDARD ? L"standard" : L"large");
		GUIConfigElement->InsertEndChild(TiXmlText(pStr));
	}

	// <GUIConfig name="StatusBar">show</GUIConfig>
	{
		TiXmlElement *GUIConfigElement = (newGUIRoot->InsertEndChild(TiXmlElement(L"GUIConfig")))->ToElement();
		GUIConfigElement->SetAttribute(L"name", L"StatusBar");
		const TCHAR *pStr = _nppGUI._statusBarShow ? L"show" : L"hide";
		GUIConfigElement->InsertEndChild(TiXmlText(pStr));
	}

	// <GUIConfig name="TabBar" dragAndDrop="yes" drawTopBar="yes" drawInactiveTab="yes" reduce="yes" closeButton="yes" doubleClick2Close="no" vertical="no" multiLine="no" hide="no" quitOnEmpty="no" />
	{
		TiXmlElement *GUIConfigElement = (newGUIRoot->InsertEndChild(TiXmlElement(L"GUIConfig")))->ToElement();
		GUIConfigElement->SetAttribute(L"name", L"TabBar");

		const TCHAR *pStr = (_nppGUI._tabStatus & TAB_DRAWTOPBAR) ? L"yes" : L"no";
		GUIConfigElement->SetAttribute(L"dragAndDrop", pStr);

		pStr = (_nppGUI._tabStatus & TAB_DRAGNDROP) ? L"yes" : L"no";
		GUIConfigElement->SetAttribute(L"drawTopBar", pStr);

		pStr = (_nppGUI._tabStatus & TAB_DRAWINACTIVETAB) ? L"yes" : L"no";
		GUIConfigElement->SetAttribute(L"drawInactiveTab", pStr);

		pStr = (_nppGUI._tabStatus & TAB_REDUCE) ? L"yes" : L"no";
		GUIConfigElement->SetAttribute(L"reduce", pStr);

		pStr = (_nppGUI._tabStatus & TAB_CLOSEBUTTON) ? L"yes" : L"no";
		GUIConfigElement->SetAttribute(L"closeButton", pStr);

		pStr = (_nppGUI._tabStatus & TAB_DBCLK2CLOSE) ? L"yes" : L"no";
		GUIConfigElement->SetAttribute(L"doubleClick2Close", pStr);

		pStr = (_nppGUI._tabStatus & TAB_VERTICAL) ? L"yes" : L"no";
		GUIConfigElement->SetAttribute(L"vertical", pStr);

		pStr = (_nppGUI._tabStatus & TAB_MULTILINE) ? L"yes" : L"no";
		GUIConfigElement->SetAttribute(L"multiLine", pStr);

		pStr = (_nppGUI._tabStatus & TAB_HIDE) ? L"yes" : L"no";
		GUIConfigElement->SetAttribute(L"hide", pStr);

		pStr = (_nppGUI._tabStatus & TAB_QUITONEMPTY) ? L"yes" : L"no";
		GUIConfigElement->SetAttribute(L"quitOnEmpty", pStr);
	}

	// <GUIConfig name="ScintillaViewsSplitter">vertical</GUIConfig>
	{
		TiXmlElement *GUIConfigElement = (newGUIRoot->InsertEndChild(TiXmlElement(L"GUIConfig")))->ToElement();
		GUIConfigElement->SetAttribute(L"name", L"ScintillaViewsSplitter");
		const TCHAR *pStr = _nppGUI._splitterPos == POS_VERTICAL ? L"vertical" : L"horizontal";
		GUIConfigElement->InsertEndChild(TiXmlText(pStr));
	}

	// <GUIConfig name="UserDefineDlg" position="undocked">hide</GUIConfig>
	{
		TiXmlElement *GUIConfigElement = (newGUIRoot->InsertEndChild(TiXmlElement(L"GUIConfig")))->ToElement();
		GUIConfigElement->SetAttribute(L"name", L"UserDefineDlg");
		const TCHAR *pStr = (_nppGUI._userDefineDlgStatus & UDD_DOCKED) ? L"docked" : L"undocked";
		GUIConfigElement->SetAttribute(L"position", pStr);
		pStr = (_nppGUI._userDefineDlgStatus & UDD_SHOW) ? L"show" : L"hide";
		GUIConfigElement->InsertEndChild(TiXmlText(pStr));
	}

	// <GUIConfig name = "TabSetting" size = "4" replaceBySpace = "no" / >
	{
		TiXmlElement *GUIConfigElement = (newGUIRoot->InsertEndChild(TiXmlElement(L"GUIConfig")))->ToElement();
		GUIConfigElement->SetAttribute(L"name", L"TabSetting");
		const TCHAR *pStr = _nppGUI._tabReplacedBySpace ? L"yes" : L"no";
		GUIConfigElement->SetAttribute(L"replaceBySpace", pStr);
		GUIConfigElement->SetAttribute(L"size", _nppGUI._tabSize);
	}

	// <GUIConfig name = "AppPosition" x = "3900" y = "446" width = "2160" height = "1380" isMaximized = "no" / >
	{
		TiXmlElement *GUIConfigElement = (newGUIRoot->InsertEndChild(TiXmlElement(L"GUIConfig")))->ToElement();
		GUIConfigElement->SetAttribute(L"name", L"AppPosition");
		GUIConfigElement->SetAttribute(L"x", _nppGUI._appPos.left);
		GUIConfigElement->SetAttribute(L"y", _nppGUI._appPos.top);
		GUIConfigElement->SetAttribute(L"width", _nppGUI._appPos.right);
		GUIConfigElement->SetAttribute(L"height", _nppGUI._appPos.bottom);
		GUIConfigElement->SetAttribute(L"isMaximized", _nppGUI._isMaximized ? L"yes" : L"no");
	}

	// <GUIConfig name="noUpdate" intervalDays="15" nextUpdateDate="20161022">no</GUIConfig>
	{
		TiXmlElement *element = insertGUIConfigBoolNode(newGUIRoot, L"noUpdate", !_nppGUI._autoUpdateOpt._doAutoUpdate);
		element->SetAttribute(L"intervalDays", _nppGUI._autoUpdateOpt._intervalDays);
		element->SetAttribute(L"nextUpdateDate", _nppGUI._autoUpdateOpt._nextUpdateDate.toString().c_str());
	}

	// <GUIConfig name="Auto-detection">yes</GUIConfig>	
	{
		const TCHAR *pStr = L"no";

		if (_nppGUI._fileAutoDetection & cdEnabledOld)
		{
			pStr = L"yesOld";

			if ((_nppGUI._fileAutoDetection & cdAutoUpdate) && (_nppGUI._fileAutoDetection & cdGo2end))
			{
				pStr = L"autoUpdate2EndOld";
			}
			else if (_nppGUI._fileAutoDetection & cdAutoUpdate)
			{
				pStr = L"autoOld";
			}
			else if (_nppGUI._fileAutoDetection & cdGo2end)
			{
				pStr = L"Update2EndOld";
			}
		}
		else if (_nppGUI._fileAutoDetection & cdEnabledNew)
		{
			pStr = L"yes";

			if ((_nppGUI._fileAutoDetection & cdAutoUpdate) && (_nppGUI._fileAutoDetection & cdGo2end))
			{
				pStr = L"autoUpdate2End";
			}
			else if (_nppGUI._fileAutoDetection & cdAutoUpdate)
			{
				pStr = L"auto";
			}
			else if (_nppGUI._fileAutoDetection & cdGo2end)
			{
				pStr = L"Update2End";
			}
		}

		TiXmlElement *GUIConfigElement = (newGUIRoot->InsertEndChild(TiXmlElement(L"GUIConfig")))->ToElement();
		GUIConfigElement->SetAttribute(L"name", L"Auto-detection");
		GUIConfigElement->InsertEndChild(TiXmlText(pStr));
	}

	// <GUIConfig name="CheckHistoryFiles">no</GUIConfig>
	{
		insertGUIConfigBoolNode(newGUIRoot, L"CheckHistoryFiles", _nppGUI._checkHistoryFiles);
	}

	// <GUIConfig name="TrayIcon">no</GUIConfig>
	{
		insertGUIConfigBoolNode(newGUIRoot, L"TrayIcon", _nppGUI._isMinimizedToTray);
	}

	// <GUIConfig name="MaitainIndent">yes</GUIConfig>
	{
		insertGUIConfigBoolNode(newGUIRoot, L"MaitainIndent", _nppGUI._maitainIndent);
	}

	// <GUIConfig name = "TagsMatchHighLight" TagAttrHighLight = "yes" HighLightNonHtmlZone = "no">yes< / GUIConfig>
	{
		TiXmlElement * ele = insertGUIConfigBoolNode(newGUIRoot, L"TagsMatchHighLight", _nppGUI._enableTagsMatchHilite);
		ele->SetAttribute(L"TagAttrHighLight", _nppGUI._enableTagAttrsHilite ? L"yes" : L"no");
		ele->SetAttribute(L"HighLightNonHtmlZone", _nppGUI._enableHiliteNonHTMLZone ? L"yes" : L"no");
	}

	// <GUIConfig name = "RememberLastSession">yes< / GUIConfig>
	{
		insertGUIConfigBoolNode(newGUIRoot, L"RememberLastSession", _nppGUI._rememberLastSession);
	}

	// <GUIConfig name = "DetectEncoding">yes< / GUIConfig>
	{
		insertGUIConfigBoolNode(newGUIRoot, L"DetectEncoding", _nppGUI._detectEncoding);
	}

	// <GUIConfig name = "NewDocDefaultSettings" format = "0" encoding = "0" lang = "3" codepage = "-1" openAnsiAsUTF8 = "no" / >
	{
		TiXmlElement *GUIConfigElement = (newGUIRoot->InsertEndChild(TiXmlElement(L"GUIConfig")))->ToElement();
		GUIConfigElement->SetAttribute(L"name", L"NewDocDefaultSettings");
		GUIConfigElement->SetAttribute(L"format", static_cast<int32_t>(_nppGUI._newDocDefaultSettings._format));
		GUIConfigElement->SetAttribute(L"encoding", _nppGUI._newDocDefaultSettings._unicodeMode);
		GUIConfigElement->SetAttribute(L"lang", _nppGUI._newDocDefaultSettings._lang);
		GUIConfigElement->SetAttribute(L"codepage", _nppGUI._newDocDefaultSettings._codepage);
		GUIConfigElement->SetAttribute(L"openAnsiAsUTF8", _nppGUI._newDocDefaultSettings._openAnsiAsUtf8 ? L"yes" : L"no");
	}

	// <GUIConfig name = "langsExcluded" gr0 = "0" gr1 = "0" gr2 = "0" gr3 = "0" gr4 = "0" gr5 = "0" gr6 = "0" gr7 = "0" langMenuCompact = "yes" / >
	{
		TiXmlElement *GUIConfigElement = (newGUIRoot->InsertEndChild(TiXmlElement(L"GUIConfig")))->ToElement();
		GUIConfigElement->SetAttribute(L"name", L"langsExcluded");
		writeExcludedLangList(GUIConfigElement);
		GUIConfigElement->SetAttribute(L"langMenuCompact", _nppGUI._isLangMenuCompact ? L"yes" : L"no");
	}

	// <GUIConfig name="Print" lineNumber="no" printOption="0" headerLeft="$(FULL_CURRENT_PATH)" headerMiddle="" headerRight="$(LONG_DATE) $(TIME)" headerFontName="IBMPC" headerFontStyle="1" headerFontSize="8" footerLeft="" footerMiddle="-$(CURRENT_PRINTING_PAGE)-" footerRight="" footerFontName="" footerFontStyle="0" footerFontSize="9" margeLeft="0" margeTop="0" margeRight="0" margeBottom="0" />
	{
		TiXmlElement *GUIConfigElement = (newGUIRoot->InsertEndChild(TiXmlElement(L"GUIConfig")))->ToElement();
		GUIConfigElement->SetAttribute(L"name", L"Print");
		writePrintSetting(GUIConfigElement);
	}

	// <GUIConfig name="Backup" action="0" useCustumDir="no" dir="" isSnapshotMode="yes" snapshotBackupTiming="7000" />
	{
		TiXmlElement *GUIConfigElement = (newGUIRoot->InsertEndChild(TiXmlElement(L"GUIConfig")))->ToElement();
		GUIConfigElement->SetAttribute(L"name", L"Backup");
		GUIConfigElement->SetAttribute(L"action", _nppGUI._backup);
		GUIConfigElement->SetAttribute(L"useCustumDir", _nppGUI._useDir ? L"yes" : L"no");
		GUIConfigElement->SetAttribute(L"dir", _nppGUI._backupDir.c_str());

		GUIConfigElement->SetAttribute(L"isSnapshotMode", _nppGUI._isSnapshotMode ? L"yes" : L"no");
		GUIConfigElement->SetAttribute(L"snapshotBackupTiming", static_cast<int32_t>(_nppGUI._snapshotBackupTiming));
	}

	// <GUIConfig name = "TaskList">yes< / GUIConfig>
	{
		insertGUIConfigBoolNode(newGUIRoot, L"TaskList", _nppGUI._doTaskList);
	}

	// <GUIConfig name = "MRU">yes< / GUIConfig>
	{
		insertGUIConfigBoolNode(newGUIRoot, L"MRU", _nppGUI._styleMRU);
	}

	// <GUIConfig name="URL">2</GUIConfig>
	{
		const TCHAR *pStr = L"0";
		if (_nppGUI._styleURL == 1)
			pStr = L"1";
		else if (_nppGUI._styleURL == 2)
			pStr = L"2";

		TiXmlElement *GUIConfigElement = (newGUIRoot->InsertEndChild(TiXmlElement(L"GUIConfig")))->ToElement();
		GUIConfigElement->SetAttribute(L"name", L"URL");
		GUIConfigElement->InsertEndChild(TiXmlText(pStr));
	}

	// <GUIConfig name = "globalOverride" fg = "no" bg = "no" font = "no" fontSize = "no" bold = "no" italic = "no" underline = "no" / >
	{
		TiXmlElement *GUIConfigElement = (newGUIRoot->InsertEndChild(TiXmlElement(L"GUIConfig")))->ToElement();
		GUIConfigElement->SetAttribute(L"name", L"globalOverride");
		GUIConfigElement->SetAttribute(L"fg", _nppGUI._globalOverride.enableFg ? L"yes" : L"no");
		GUIConfigElement->SetAttribute(L"bg", _nppGUI._globalOverride.enableBg ? L"yes" : L"no");
		GUIConfigElement->SetAttribute(L"font", _nppGUI._globalOverride.enableFont ? L"yes" : L"no");
		GUIConfigElement->SetAttribute(L"fontSize", _nppGUI._globalOverride.enableFontSize ? L"yes" : L"no");
		GUIConfigElement->SetAttribute(L"bold", _nppGUI._globalOverride.enableBold ? L"yes" : L"no");
		GUIConfigElement->SetAttribute(L"italic", _nppGUI._globalOverride.enableItalic ? L"yes" : L"no");
		GUIConfigElement->SetAttribute(L"underline", _nppGUI._globalOverride.enableUnderLine ? L"yes" : L"no");
	}

	// <GUIConfig name = "auto-completion" autoCAction = "3" triggerFromNbChar = "1" funcParams = "yes" autoCIgnoreNumbers = "yes" / >
	{
		TiXmlElement *GUIConfigElement = (newGUIRoot->InsertEndChild(TiXmlElement(L"GUIConfig")))->ToElement();
		GUIConfigElement->SetAttribute(L"name", L"auto-completion");
		GUIConfigElement->SetAttribute(L"autoCAction", _nppGUI._autocStatus);
		GUIConfigElement->SetAttribute(L"triggerFromNbChar", static_cast<int32_t>(_nppGUI._autocFromLen));

		const TCHAR * pStr = _nppGUI._autocIgnoreNumbers ? L"yes" : L"no";
		GUIConfigElement->SetAttribute(L"autoCIgnoreNumbers", pStr);

		pStr = _nppGUI._funcParams ? L"yes" : L"no";
		GUIConfigElement->SetAttribute(L"funcParams", pStr);
	}

	// <GUIConfig name = "auto-insert" parentheses = "yes" brackets = "yes" curlyBrackets = "yes" quotes = "no" doubleQuotes = "yes" htmlXmlTag = "yes" / >
	{
		TiXmlElement *GUIConfigElement = (newGUIRoot->InsertEndChild(TiXmlElement(L"GUIConfig")))->ToElement();
		GUIConfigElement->SetAttribute(L"name", L"auto-insert");

		GUIConfigElement->SetAttribute(L"parentheses", _nppGUI._matchedPairConf._doParentheses ? L"yes" : L"no");
		GUIConfigElement->SetAttribute(L"brackets", _nppGUI._matchedPairConf._doBrackets ? L"yes" : L"no");
		GUIConfigElement->SetAttribute(L"curlyBrackets", _nppGUI._matchedPairConf._doCurlyBrackets ? L"yes" : L"no");
		GUIConfigElement->SetAttribute(L"quotes", _nppGUI._matchedPairConf._doQuotes ? L"yes" : L"no");
		GUIConfigElement->SetAttribute(L"doubleQuotes", _nppGUI._matchedPairConf._doDoubleQuotes ? L"yes" : L"no");
		GUIConfigElement->SetAttribute(L"htmlXmlTag", _nppGUI._matchedPairConf._doHtmlXmlTag ? L"yes" : L"no");

		TiXmlElement hist_element{ L""};
		hist_element.SetValue(L"UserDefinePair");
		for (size_t i = 0, nb = _nppGUI._matchedPairConf._matchedPairs.size(); i < nb; ++i)
		{
			int open = _nppGUI._matchedPairConf._matchedPairs[i].first;
			int close = _nppGUI._matchedPairConf._matchedPairs[i].second;

			(hist_element.ToElement())->SetAttribute(L"open", open);
			(hist_element.ToElement())->SetAttribute(L"close", close);
			GUIConfigElement->InsertEndChild(hist_element);
		}
	}

	// <GUIConfig name = "sessionExt">< / GUIConfig>
	{
		TiXmlElement *GUIConfigElement = (newGUIRoot->InsertEndChild(TiXmlElement(L"GUIConfig")))->ToElement();
		GUIConfigElement->SetAttribute(L"name", L"sessionExt");
		GUIConfigElement->InsertEndChild(TiXmlText(_nppGUI._definedSessionExt.c_str()));
	}

	// <GUIConfig name="workspaceExt"></GUIConfig>
	{
		TiXmlElement *GUIConfigElement = (newGUIRoot->InsertEndChild(TiXmlElement(L"GUIConfig")))->ToElement();
		GUIConfigElement->SetAttribute(L"name", L"workspaceExt");
		GUIConfigElement->InsertEndChild(TiXmlText(_nppGUI._definedWorkspaceExt.c_str()));
	}

	// <GUIConfig name="MenuBar">show</GUIConfig>
	{
		TiXmlElement *GUIConfigElement = (newGUIRoot->InsertEndChild(TiXmlElement(L"GUIConfig")))->ToElement();
		GUIConfigElement->SetAttribute(L"name", L"MenuBar");
		GUIConfigElement->InsertEndChild(TiXmlText(_nppGUI._menuBarShow ? L"show" : L"hide"));
	}

	// <GUIConfig name="Caret" width="1" blinkRate="250" />
	{
		TiXmlElement *GUIConfigElement = (newGUIRoot->InsertEndChild(TiXmlElement(L"GUIConfig")))->ToElement();
		GUIConfigElement->SetAttribute(L"name", L"Caret");
		GUIConfigElement->SetAttribute(L"width", _nppGUI._caretWidth);
		GUIConfigElement->SetAttribute(L"blinkRate", _nppGUI._caretBlinkRate);
	}

	// <GUIConfig name="ScintillaGlobalSettings" enableMultiSelection="no" />
	{
		TiXmlElement *GUIConfigElement = (newGUIRoot->InsertEndChild(TiXmlElement(L"GUIConfig")))->ToElement();
		GUIConfigElement->SetAttribute(L"name", L"ScintillaGlobalSettings");
		GUIConfigElement->SetAttribute(L"enableMultiSelection", _nppGUI._enableMultiSelection ? L"yes" : L"no");
	}

	// <GUIConfig name="openSaveDir" value="0" defaultDirPath="" />
	{
		TiXmlElement *GUIConfigElement = (newGUIRoot->InsertEndChild(TiXmlElement(L"GUIConfig")))->ToElement();
		GUIConfigElement->SetAttribute(L"name", L"openSaveDir");
		GUIConfigElement->SetAttribute(L"value", _nppGUI._openSaveDir);
		GUIConfigElement->SetAttribute(L"defaultDirPath", _nppGUI._defaultDir);
	}

	// <GUIConfig name="titleBar" short="no" />
	{
		TiXmlElement *GUIConfigElement = (newGUIRoot->InsertEndChild(TiXmlElement(L"GUIConfig")))->ToElement();
		GUIConfigElement->SetAttribute(L"name", L"titleBar");
		const TCHAR *pStr = (_nppGUI._shortTitlebar) ? L"yes" : L"no";
		GUIConfigElement->SetAttribute(L"short", pStr);
	}

	// <GUIConfig name="stylerTheme" path="C:\sources\notepad-plus-plus\PowerEditor\visual.net\..\bin\stylers.xml" />
	{
		TiXmlElement *GUIConfigElement = (newGUIRoot->InsertEndChild(TiXmlElement(L"GUIConfig")))->ToElement();
		GUIConfigElement->SetAttribute(L"name", L"stylerTheme");
		GUIConfigElement->SetAttribute(L"path", _nppGUI._themeName.c_str());
	}

	// <GUIConfig name="wordCharList" useDefault="yes" charsAdded=".$%"  />
	{
		TiXmlElement *GUIConfigElement = (newGUIRoot->InsertEndChild(TiXmlElement(L"GUIConfig")))->ToElement();
		GUIConfigElement->SetAttribute(L"name", L"wordCharList");
		GUIConfigElement->SetAttribute(L"useDefault", _nppGUI._isWordCharDefault ? L"yes" : L"no");
		WcharMbcsConvertor& wmc = WcharMbcsConvertor::getInstance();
		const wchar_t* charsAddStr = wmc.char2wchar(_nppGUI._customWordChars.c_str(), SC_CP_UTF8);
		GUIConfigElement->SetAttribute(L"charsAdded", charsAddStr);
	}

	// <GUIConfig name="delimiterSelection" leftmostDelimiter="40" rightmostDelimiter="41" delimiterSelectionOnEntireDocument="no" />
	{
		TiXmlElement *GUIConfigElement = (newGUIRoot->InsertEndChild(TiXmlElement(L"GUIConfig")))->ToElement();
		GUIConfigElement->SetAttribute(L"name", L"delimiterSelection");
		GUIConfigElement->SetAttribute(L"leftmostDelimiter", _nppGUI._leftmostDelimiter);
		GUIConfigElement->SetAttribute(L"rightmostDelimiter", _nppGUI._rightmostDelimiter);
		GUIConfigElement->SetAttribute(L"delimiterSelectionOnEntireDocument", _nppGUI._delimiterSelectionOnEntireDocument ? L"yes" : L"no");
	}

	// <GUIConfig name="multiInst" setting="0" />
	{
		TiXmlElement *GUIConfigElement = (newGUIRoot->InsertEndChild(TiXmlElement(L"GUIConfig")))->ToElement();
		GUIConfigElement->SetAttribute(L"name", L"multiInst");
		GUIConfigElement->SetAttribute(L"setting", _nppGUI._multiInstSetting);
	}

	// <GUIConfig name="MISC" fileSwitcherWithoutExtColumn="no" backSlashIsEscapeCharacterForSql="yes" newStyleSaveDlg="no" isFolderDroppedOpenFiles="no" />
	{
		TiXmlElement *GUIConfigElement = (newGUIRoot->InsertEndChild(TiXmlElement(L"GUIConfig")))->ToElement();
		GUIConfigElement->SetAttribute(L"name", L"MISC");

		GUIConfigElement->SetAttribute(L"fileSwitcherWithoutExtColumn", _nppGUI._fileSwitcherWithoutExtColumn ? L"yes" : L"no");
		GUIConfigElement->SetAttribute(L"backSlashIsEscapeCharacterForSql", _nppGUI._backSlashIsEscapeCharacterForSql ? L"yes" : L"no");
		GUIConfigElement->SetAttribute(L"monospacedFontFindDlg", _nppGUI._monospacedFontFindDlg ? L"yes" : L"no");
		GUIConfigElement->SetAttribute(L"stopFillingFindField", _nppGUI._stopFillingFindField ? L"yes" : L"no");
		GUIConfigElement->SetAttribute(L"newStyleSaveDlg", _nppGUI._useNewStyleSaveDlg ? L"yes" : L"no");
		GUIConfigElement->SetAttribute(L"isFolderDroppedOpenFiles", _nppGUI._isFolderDroppedOpenFiles ? L"yes" : L"no");
		GUIConfigElement->SetAttribute(L"docPeekOnTab", _nppGUI._isDocPeekOnTab ? L"yes" : L"no");
		GUIConfigElement->SetAttribute(L"docPeekOnMap", _nppGUI._isDocPeekOnMap ? L"yes" : L"no");
	}

	// <GUIConfig name="searchEngine" searchEngineChoice="2" searchEngineCustom="" />
	{
		TiXmlElement *GUIConfigElement = (newGUIRoot->InsertEndChild(TiXmlElement(L"GUIConfig")))->ToElement();
		GUIConfigElement->SetAttribute(L"name", L"searchEngine");
		GUIConfigElement->SetAttribute(L"searchEngineChoice", _nppGUI._searchEngineChoice);
		GUIConfigElement->SetAttribute(L"searchEngineCustom", _nppGUI._searchEngineCustom);
	}

	// <GUIConfig name="SmartHighLight" matchCase="no" wholeWordOnly="yes" useFindSettings="no" onAnotherView="no">yes</GUIConfig>
	{
		TiXmlElement *GUIConfigElement = insertGUIConfigBoolNode(newGUIRoot, L"SmartHighLight", _nppGUI._enableSmartHilite);
		GUIConfigElement->SetAttribute(L"matchCase", _nppGUI._smartHiliteCaseSensitive ? L"yes" : L"no");
		GUIConfigElement->SetAttribute(L"wholeWordOnly", _nppGUI._smartHiliteWordOnly ? L"yes" : L"no");
		GUIConfigElement->SetAttribute(L"useFindSettings", _nppGUI._smartHiliteUseFindSettings ? L"yes" : L"no");
		GUIConfigElement->SetAttribute(L"onAnotherView", _nppGUI._smartHiliteOnAnotherView ? L"yes" : L"no");
	}

	// <GUIConfig name="commandLineInterpreter">powershell</GUIConfig>
	if (_nppGUI._commandLineInterpreter.compare(L"cmd"))
	{
		TiXmlElement *GUIConfigElement = (newGUIRoot->InsertEndChild(TiXmlElement(L"GUIConfig")))->ToElement();
		GUIConfigElement->SetAttribute(L"name", L"commandLineInterpreter");
		GUIConfigElement->InsertEndChild(TiXmlText(_nppGUI._commandLineInterpreter.c_str()));
	}

	// <GUIConfig name="ScintillaPrimaryView" lineNumberMargin="show" bookMarkMargin="show" indentGuideLine="show" folderMarkStyle="box" lineWrapMethod="aligned" currentLineHilitingShow="show" scrollBeyondLastLine="no" disableAdvancedScrolling="no" wrapSymbolShow="hide" Wrap="no" borderEdge="yes" edge="no" edgeNbColumn="80" zoom="0" zoom2="0" whiteSpaceShow="hide" eolShow="hide" borderWidth="2" smoothFont="no" />
	writeScintillaParams();

	// <GUIConfig name="DockingManager" leftWidth="328" rightWidth="359" topHeight="200" bottomHeight="436">
	// ...
	insertDockingParamNode(newGUIRoot);
}

bool NppParameters::writeFindHistory()
{
	if (not _pXmlUserDoc) return false;

	TiXmlNode *nppRoot = _pXmlUserDoc->FirstChild(L"NotepadPlus");
	if (not nppRoot)
	{
		nppRoot = _pXmlUserDoc->InsertEndChild(TiXmlElement(L"NotepadPlus"));
	}

	TiXmlNode *findHistoryRoot = nppRoot->FirstChildElement(L"FindHistory");
	if (!findHistoryRoot)
	{
		TiXmlElement element(L"FindHistory");
		findHistoryRoot = nppRoot->InsertEndChild(element);
	}
	findHistoryRoot->Clear();

	(findHistoryRoot->ToElement())->SetAttribute(L"nbMaxFindHistoryPath",	_findHistory._nbMaxFindHistoryPath);
	(findHistoryRoot->ToElement())->SetAttribute(L"nbMaxFindHistoryFilter",  _findHistory._nbMaxFindHistoryFilter);
	(findHistoryRoot->ToElement())->SetAttribute(L"nbMaxFindHistoryFind",	_findHistory._nbMaxFindHistoryFind);
	(findHistoryRoot->ToElement())->SetAttribute(L"nbMaxFindHistoryReplace", _findHistory._nbMaxFindHistoryReplace);

	(findHistoryRoot->ToElement())->SetAttribute(L"matchWord",				_findHistory._isMatchWord?L"yes":L"no");
	(findHistoryRoot->ToElement())->SetAttribute(L"matchCase",				_findHistory._isMatchCase?L"yes":L"no");
	(findHistoryRoot->ToElement())->SetAttribute(L"wrap",					_findHistory._isWrap?L"yes":L"no");
	(findHistoryRoot->ToElement())->SetAttribute(L"directionDown",			_findHistory._isDirectionDown?L"yes":L"no");

	(findHistoryRoot->ToElement())->SetAttribute(L"fifRecuisive",			_findHistory._isFifRecuisive?L"yes":L"no");
	(findHistoryRoot->ToElement())->SetAttribute(L"fifInHiddenFolder",		_findHistory._isFifInHiddenFolder?L"yes":L"no");
	(findHistoryRoot->ToElement())->SetAttribute(L"dlgAlwaysVisible",		_findHistory._isDlgAlwaysVisible?L"yes":L"no");
	(findHistoryRoot->ToElement())->SetAttribute(L"fifFilterFollowsDoc",	_findHistory._isFilterFollowDoc?L"yes":L"no");
	(findHistoryRoot->ToElement())->SetAttribute(L"fifFolderFollowsDoc",	_findHistory._isFolderFollowDoc?L"yes":L"no");

	(findHistoryRoot->ToElement())->SetAttribute(L"searchMode", _findHistory._searchMode);
	(findHistoryRoot->ToElement())->SetAttribute(L"transparencyMode", _findHistory._transparencyMode);
	(findHistoryRoot->ToElement())->SetAttribute(L"transparency", _findHistory._transparency);
	(findHistoryRoot->ToElement())->SetAttribute(L"dotMatchesNewline",		_findHistory._dotMatchesNewline?L"yes":L"no");
	(findHistoryRoot->ToElement())->SetAttribute(L"isSearch2ButtonsMode",		_findHistory._isSearch2ButtonsMode?L"yes":L"no");

	TiXmlElement hist_element{L""};

	hist_element.SetValue(L"Path");
	for (size_t i = 0, len = _findHistory._findHistoryPaths.size(); i < len; ++i)
	{
		(hist_element.ToElement())->SetAttribute(L"name", _findHistory._findHistoryPaths[i].c_str());
		findHistoryRoot->InsertEndChild(hist_element);
	}

	hist_element.SetValue(L"Filter");
	for (size_t i = 0, len = _findHistory._findHistoryFilters.size(); i < len; ++i)
	{
		(hist_element.ToElement())->SetAttribute(L"name", _findHistory._findHistoryFilters[i].c_str());
		findHistoryRoot->InsertEndChild(hist_element);
	}

	hist_element.SetValue(L"Find");
	for (size_t i = 0, len = _findHistory._findHistoryFinds.size(); i < len; ++i)
	{
		(hist_element.ToElement())->SetAttribute(L"name", _findHistory._findHistoryFinds[i].c_str());
		findHistoryRoot->InsertEndChild(hist_element);
	}

	hist_element.SetValue(L"Replace");
	for (size_t i = 0, len = _findHistory._findHistoryReplaces.size(); i < len; ++i)
	{
		(hist_element.ToElement())->SetAttribute(L"name", _findHistory._findHistoryReplaces[i].c_str());
		findHistoryRoot->InsertEndChild(hist_element);
	}

	return true;
}

void NppParameters::insertDockingParamNode(TiXmlNode *GUIRoot)
{
	TiXmlElement DMNode(L"GUIConfig");
	DMNode.SetAttribute(L"name", L"DockingManager");
	DMNode.SetAttribute(L"leftWidth", _nppGUI._dockingData._leftWidth);
	DMNode.SetAttribute(L"rightWidth", _nppGUI._dockingData._rightWidth);
	DMNode.SetAttribute(L"topHeight", _nppGUI._dockingData._topHeight);
	DMNode.SetAttribute(L"bottomHeight", _nppGUI._dockingData._bottomHight);

	for (size_t i = 0, len = _nppGUI._dockingData._flaotingWindowInfo.size(); i < len ; ++i)
	{
		FloatingWindowInfo & fwi = _nppGUI._dockingData._flaotingWindowInfo[i];
		TiXmlElement FWNode(L"FloatingWindow");
		FWNode.SetAttribute(L"cont", fwi._cont);
		FWNode.SetAttribute(L"x", fwi._pos.left);
		FWNode.SetAttribute(L"y", fwi._pos.top);
		FWNode.SetAttribute(L"width", fwi._pos.right);
		FWNode.SetAttribute(L"height", fwi._pos.bottom);

		DMNode.InsertEndChild(FWNode);
	}

	for (size_t i = 0, len = _nppGUI._dockingData._pluginDockInfo.size() ; i < len ; ++i)
	{
		PluginDlgDockingInfo & pdi = _nppGUI._dockingData._pluginDockInfo[i];
		TiXmlElement PDNode(L"PluginDlg");
		PDNode.SetAttribute(L"pluginName", pdi._name);
		PDNode.SetAttribute(L"id", pdi._internalID);
		PDNode.SetAttribute(L"curr", pdi._currContainer);
		PDNode.SetAttribute(L"prev", pdi._prevContainer);
		PDNode.SetAttribute(L"isVisible", pdi._isVisible?L"yes":L"no");

		DMNode.InsertEndChild(PDNode);
	}

	for (size_t i = 0, len = _nppGUI._dockingData._containerTabInfo.size(); i < len ; ++i)
	{
		ContainerTabInfo & cti = _nppGUI._dockingData._containerTabInfo[i];
		TiXmlElement CTNode(L"ActiveTabs");
		CTNode.SetAttribute(L"cont", cti._cont);
		CTNode.SetAttribute(L"activeTab", cti._activeTab);
		DMNode.InsertEndChild(CTNode);
	}

	GUIRoot->InsertEndChild(DMNode);
}

void NppParameters::writePrintSetting(TiXmlElement *element)
{
	const TCHAR *pStr = _nppGUI._printSettings._printLineNumber?L"yes":L"no";
	element->SetAttribute(L"lineNumber", pStr);

	element->SetAttribute(L"printOption", _nppGUI._printSettings._printOption);

	element->SetAttribute(L"headerLeft", _nppGUI._printSettings._headerLeft.c_str());
	element->SetAttribute(L"headerMiddle", _nppGUI._printSettings._headerMiddle.c_str());
	element->SetAttribute(L"headerRight", _nppGUI._printSettings._headerRight.c_str());
	element->SetAttribute(L"footerLeft", _nppGUI._printSettings._footerLeft.c_str());
	element->SetAttribute(L"footerMiddle", _nppGUI._printSettings._footerMiddle.c_str());
	element->SetAttribute(L"footerRight", _nppGUI._printSettings._footerRight.c_str());

	element->SetAttribute(L"headerFontName", _nppGUI._printSettings._headerFontName.c_str());
	element->SetAttribute(L"headerFontStyle", _nppGUI._printSettings._headerFontStyle);
	element->SetAttribute(L"headerFontSize", _nppGUI._printSettings._headerFontSize);
	element->SetAttribute(L"footerFontName", _nppGUI._printSettings._footerFontName.c_str());
	element->SetAttribute(L"footerFontStyle", _nppGUI._printSettings._footerFontStyle);
	element->SetAttribute(L"footerFontSize", _nppGUI._printSettings._footerFontSize);

	element->SetAttribute(L"margeLeft", _nppGUI._printSettings._marge.left);
	element->SetAttribute(L"margeRight", _nppGUI._printSettings._marge.right);
	element->SetAttribute(L"margeTop", _nppGUI._printSettings._marge.top);
	element->SetAttribute(L"margeBottom", _nppGUI._printSettings._marge.bottom);
}

void NppParameters::writeExcludedLangList(TiXmlElement *element)
{
	int g0 = 0; // up to 8
	int g1 = 0; // up to 16
	int g2 = 0; // up to 24
	int g3 = 0; // up to 32
	int g4 = 0; // up to 40
	int g5 = 0; // up to 48
	int g6 = 0; // up to 56
	int g7 = 0; // up to 64
	int g8 = 0; // up to 72
	int g9 = 0; // up to 80
	int g10= 0; // up to 88
	int g11= 0; // up to 96
	int g12= 0; // up to 104

	const int groupNbMember = 8;

	for (size_t i = 0, len = _nppGUI._excludedLangList.size(); i < len ; ++i)
	{
		LangType langType = _nppGUI._excludedLangList[i]._langType;
		if (langType >= L_EXTERNAL && langType < L_END)
			continue;

		int nGrp = langType / groupNbMember;
		int nMask = 1 << langType % groupNbMember;


		switch (nGrp)
		{
			case 0 :
				g0 |= nMask;
				break;
			case 1 :
				g1 |= nMask;
				break;
			case 2 :
				g2 |= nMask;
				break;
			case 3 :
				g3 |= nMask;
				break;
			case 4 :
				g4 |= nMask;
				break;
			case 5 :
				g5 |= nMask;
				break;
			case 6 :
				g6 |= nMask;
				break;
			case 7 :
				g7 |= nMask;
				break;
			case 8:
				g8 |= nMask;
				break;
			case 9:
				g9 |= nMask;
				break;
			case 10:
				g10 |= nMask;
				break;
			case 11:
				g11 |= nMask;
				break;
			case 12:
				g12 |= nMask;
				break;
		}
	}

	element->SetAttribute(L"gr0", g0);
	element->SetAttribute(L"gr1", g1);
	element->SetAttribute(L"gr2", g2);
	element->SetAttribute(L"gr3", g3);
	element->SetAttribute(L"gr4", g4);
	element->SetAttribute(L"gr5", g5);
	element->SetAttribute(L"gr6", g6);
	element->SetAttribute(L"gr7", g7);
	element->SetAttribute(L"gr8", g8);
	element->SetAttribute(L"gr9", g9);
	element->SetAttribute(L"gr10", g10);
	element->SetAttribute(L"gr11", g11);
	element->SetAttribute(L"gr12", g12);
}

TiXmlElement * NppParameters::insertGUIConfigBoolNode(TiXmlNode *r2w, const TCHAR *name, bool bVal)
{
	const TCHAR *pStr = bVal?L"yes":L"no";
	TiXmlElement *GUIConfigElement = (r2w->InsertEndChild(TiXmlElement(L"GUIConfig")))->ToElement();
	GUIConfigElement->SetAttribute(L"name", name);
	GUIConfigElement->InsertEndChild(TiXmlText(pStr));
	return GUIConfigElement;
}

int RGB2int(COLORREF color)
{
	return (((((DWORD)color) & 0x0000FF) << 16) | ((((DWORD)color) & 0x00FF00)) | ((((DWORD)color) & 0xFF0000) >> 16));
}

int NppParameters::langTypeToCommandID(LangType lt) const
{
	int id;
	switch (lt)
	{
		case L_C :
			id = IDM_LANG_C; break;
		case L_CPP :
			id = IDM_LANG_CPP; break;
		case L_JAVA :
			id = IDM_LANG_JAVA;	break;
		case L_CS :
			id = IDM_LANG_CS; break;
		case L_OBJC :
			id = IDM_LANG_OBJC;	break;
		case L_HTML :
			id = IDM_LANG_HTML;	break;
		case L_XML :
			id = IDM_LANG_XML; break;
		case L_JS :
		case L_JAVASCRIPT:
			id = IDM_LANG_JS; break;
		case L_JSON:
			id = IDM_LANG_JSON; break;
		case L_PHP :
			id = IDM_LANG_PHP; break;
		case L_ASP :
			id = IDM_LANG_ASP; break;
		case L_JSP :
			id = IDM_LANG_JSP; break;
		case L_CSS :
			id = IDM_LANG_CSS; break;
		case L_LUA :
			id = IDM_LANG_LUA; break;
		case L_PERL :
			id = IDM_LANG_PERL; break;
		case L_PYTHON :
			id = IDM_LANG_PYTHON; break;
		case L_BATCH :
			id = IDM_LANG_BATCH; break;
		case L_PASCAL :
			id = IDM_LANG_PASCAL; break;
		case L_MAKEFILE :
			id = IDM_LANG_MAKEFILE;	break;
		case L_INI :
			id = IDM_LANG_INI; break;
		case L_ASCII :
			id = IDM_LANG_ASCII; break;
		case L_RC :
			id = IDM_LANG_RC; break;
		case L_TEX :
			id = IDM_LANG_TEX; break;
		case L_FORTRAN :
			id = IDM_LANG_FORTRAN; break;
		case L_FORTRAN_77 :
			id = IDM_LANG_FORTRAN_77; break;
		case L_BASH :
			id = IDM_LANG_BASH; break;
		case L_FLASH :
			id = IDM_LANG_FLASH; break;
		case L_NSIS :
			id = IDM_LANG_NSIS; break;
		case L_USER :
			id = IDM_LANG_USER; break;
		case L_SQL :
			id = IDM_LANG_SQL; break;
		case L_VB :
			id = IDM_LANG_VB; break;
		case L_TCL :
			id = IDM_LANG_TCL; break;

		case L_LISP :
			id = IDM_LANG_LISP; break;
		case L_SCHEME :
			id = IDM_LANG_SCHEME; break;
		case L_ASM :
			id = IDM_LANG_ASM; break;
		case L_DIFF :
			id = IDM_LANG_DIFF; break;
		case L_PROPS :
			id = IDM_LANG_PROPS; break;
		case L_PS :
			id = IDM_LANG_PS; break;
		case L_RUBY :
			id = IDM_LANG_RUBY; break;
		case L_SMALLTALK :
			id = IDM_LANG_SMALLTALK; break;
		case L_VHDL :
			id = IDM_LANG_VHDL; break;

		case L_ADA :
			id = IDM_LANG_ADA; break;
		case L_MATLAB :
			id = IDM_LANG_MATLAB; break;

		case L_HASKELL :
			id = IDM_LANG_HASKELL; break;

		case L_KIX :
			id = IDM_LANG_KIX; break;
		case L_AU3 :
			id = IDM_LANG_AU3; break;
		case L_VERILOG :
			id = IDM_LANG_VERILOG; break;
		case L_CAML :
			id = IDM_LANG_CAML; break;

		case L_INNO :
			id = IDM_LANG_INNO; break;

		case L_CMAKE :
			id = IDM_LANG_CMAKE; break;

		case L_YAML :
			id = IDM_LANG_YAML; break;

		case L_COBOL :
			id = IDM_LANG_COBOL; break;

		case L_D :
			id = IDM_LANG_D; break;

		case L_GUI4CLI :
			id = IDM_LANG_GUI4CLI; break;

		case L_POWERSHELL :
			id = IDM_LANG_POWERSHELL; break;

		case L_R :
			id = IDM_LANG_R; break;

		case L_COFFEESCRIPT :
			id = IDM_LANG_COFFEESCRIPT; break;

		case L_BAANC:
			id = IDM_LANG_BAANC; break;

		case L_SREC :
			id = IDM_LANG_SREC; break;

		case L_IHEX :
			id = IDM_LANG_IHEX; break;

		case L_TEHEX :
			id = IDM_LANG_TEHEX; break;

		case L_SWIFT:
			id = IDM_LANG_SWIFT; break;

		case L_ASN1 :
			id = IDM_LANG_ASN1; break;

        case L_AVS :
			id = IDM_LANG_AVS; break;

		case L_BLITZBASIC :
			id = IDM_LANG_BLITZBASIC; break;

		case L_PUREBASIC :
			id = IDM_LANG_PUREBASIC; break;

		case L_FREEBASIC :
			id = IDM_LANG_FREEBASIC; break;

		case L_CSOUND :
			id = IDM_LANG_CSOUND; break;

		case L_ERLANG :
			id = IDM_LANG_ERLANG; break;

		case L_ESCRIPT :
			id = IDM_LANG_ESCRIPT; break;

		case L_FORTH :
			id = IDM_LANG_FORTH; break;

		case L_LATEX :
			id = IDM_LANG_LATEX; break;

		case L_MMIXAL :
			id = IDM_LANG_MMIXAL; break;

		case L_NIMROD :
			id = IDM_LANG_NIMROD; break;

		case L_NNCRONTAB :
			id = IDM_LANG_NNCRONTAB; break;

		case L_OSCRIPT :
			id = IDM_LANG_OSCRIPT; break;

		case L_REBOL :
			id = IDM_LANG_REBOL; break;

		case L_REGISTRY :
			id = IDM_LANG_REGISTRY; break;

		case L_RUST :
			id = IDM_LANG_RUST; break;

		case L_SPICE :
			id = IDM_LANG_SPICE; break;

		case L_TXT2TAGS :
			id = IDM_LANG_TXT2TAGS; break;

		case L_VISUALPROLOG:
			id = IDM_LANG_VISUALPROLOG; break;

		case L_SEARCHRESULT :
			id = -1;	break;

		case L_TEXT :
			id = IDM_LANG_TEXT;	break;


		default :
			if (lt >= L_EXTERNAL && lt < L_END)
				id = lt - L_EXTERNAL + IDM_LANG_EXTERNAL;
			else
				id = IDM_LANG_TEXT;
	}
	return id;
}

generic_string NppParameters:: getWinVersionStr() const
{
	switch (_winVersion)
	{
		case WV_WIN32S: return L"Windows 3.1";
		case WV_95: return L"Windows 95";
		case WV_98: return L"Windows 98";
		case WV_ME: return L"Windows Millennium Edition";
		case WV_NT: return L"Windows NT";
		case WV_W2K: return L"Windows 2000";
		case WV_XP: return L"Windows XP";
		case WV_S2003: return L"Windows Server 2003";
		case WV_XPX64: return L"Windows XP 64 bits";
		case WV_VISTA: return L"Windows Vista";
		case WV_WIN7: return L"Windows 7";
		case WV_WIN8: return L"Windows 8";
		case WV_WIN81: return L"Windows 8.1";
		case WV_WIN10: return L"Windows 10";
		default: /*case WV_UNKNOWN:*/ return L"Windows unknown version";
	}
}

generic_string NppParameters::getWinVerBitStr() const
{
	switch (_platForm)
	{
	case PF_X86:
		return L"32-bit";

	case PF_X64:
	case PF_IA64:
		return L"64-bit";

	default:
		return L"Unknown-bit";
	}
}

void NppParameters::writeStyles(LexerStylerArray & lexersStylers, StyleArray & globalStylers)
{
	TiXmlNode *lexersRoot = (_pXmlUserStylerDoc->FirstChild(L"NotepadPlus"))->FirstChildElement(L"LexerStyles");
	for (TiXmlNode *childNode = lexersRoot->FirstChildElement(L"LexerType");
		childNode ;
		childNode = childNode->NextSibling(L"LexerType"))
	{
		TiXmlElement *element = childNode->ToElement();
		const TCHAR *nm = element->Attribute(L"name");

		LexerStyler *pLs = _lexerStylerArray.getLexerStylerByName(nm);
		LexerStyler *pLs2 = lexersStylers.getLexerStylerByName(nm);

		if (pLs)
		{
			const TCHAR *extStr = pLs->getLexerUserExt();
			element->SetAttribute(L"ext", extStr);
			for (TiXmlNode *grChildNode = childNode->FirstChildElement(L"WordsStyle");
					grChildNode ;
					grChildNode = grChildNode->NextSibling(L"WordsStyle"))
			{
				TiXmlElement *grElement = grChildNode->ToElement();
				const TCHAR *styleName = grElement->Attribute(L"name");

				int i = pLs->getStylerIndexByName(styleName);
				if (i != -1)
				{
					Style & style = pLs->getStyler(i);
					Style & style2Sync = pLs2->getStyler(i);

					writeStyle2Element(style, style2Sync, grElement);
				}
			}
		}
	}

	for (size_t x = 0; x < _pXmlExternalLexerDoc.size(); ++x)
	{
		TiXmlNode* lexersRoot2 = ( _pXmlExternalLexerDoc[x]->FirstChild(L"NotepadPlus"))->FirstChildElement(L"LexerStyles");
		for (TiXmlNode* childNode = lexersRoot2->FirstChildElement(L"LexerType");
			childNode ;
			childNode = childNode->NextSibling(L"LexerType"))
		{
			TiXmlElement *element = childNode->ToElement();
			const TCHAR *nm = element->Attribute(L"name");

			LexerStyler *pLs = _lexerStylerArray.getLexerStylerByName(nm);
			LexerStyler *pLs2 = lexersStylers.getLexerStylerByName(nm);

			if (pLs)
			{
				const TCHAR *extStr = pLs->getLexerUserExt();
				element->SetAttribute(L"ext", extStr);

				for (TiXmlNode *grChildNode = childNode->FirstChildElement(L"WordsStyle");
						grChildNode ;
						grChildNode = grChildNode->NextSibling(L"WordsStyle"))
				{
					TiXmlElement *grElement = grChildNode->ToElement();
					const TCHAR *styleName = grElement->Attribute(L"name");

					int i = pLs->getStylerIndexByName(styleName);
					if (i != -1)
					{
						Style & style = pLs->getStyler(i);
						Style & style2Sync = pLs2->getStyler(i);

						writeStyle2Element(style, style2Sync, grElement);
					}
				}
			}
		}
		_pXmlExternalLexerDoc[x]->SaveFile();
	}

	TiXmlNode *globalStylesRoot = (_pXmlUserStylerDoc->FirstChild(L"NotepadPlus"))->FirstChildElement(L"GlobalStyles");

	for (TiXmlNode *childNode = globalStylesRoot->FirstChildElement(L"WidgetStyle");
		childNode ;
		childNode = childNode->NextSibling(L"WidgetStyle"))
	{
		TiXmlElement *pElement = childNode->ToElement();
		const TCHAR *styleName = pElement->Attribute(L"name");
		int i = _widgetStyleArray.getStylerIndexByName(styleName);

		if (i != -1)
		{
			Style & style = _widgetStyleArray.getStyler(i);
			Style & style2Sync = globalStylers.getStyler(i);

			writeStyle2Element(style, style2Sync, pElement);
		}
	}

	_pXmlUserStylerDoc->SaveFile();
}


bool NppParameters::insertTabInfo(const TCHAR *langName, int tabInfo)
{
	if (!_pXmlDoc) return false;
	TiXmlNode *langRoot = (_pXmlDoc->FirstChild(L"NotepadPlus"))->FirstChildElement(L"Languages");
	for (TiXmlNode *childNode = langRoot->FirstChildElement(L"Language");
		childNode ;
		childNode = childNode->NextSibling(L"Language"))
	{
		TiXmlElement *element = childNode->ToElement();
		const TCHAR *nm = element->Attribute(L"name");
		if (nm && lstrcmp(langName, nm) == 0)
		{
			childNode->ToElement()->SetAttribute(L"tabSettings", tabInfo);
			_pXmlDoc->SaveFile();
			return true;
		}
	}
	return false;
}

void NppParameters::writeStyle2Element(Style & style2Write, Style & style2Sync, TiXmlElement *element)
{
	if (HIBYTE(HIWORD(style2Write._fgColor)) != 0xFF)
	{
		int rgbVal = RGB2int(style2Write._fgColor);
		TCHAR fgStr[7];
		wsprintf(fgStr, L"%.6X", rgbVal);
		element->SetAttribute(L"fgColor", fgStr);
	}

	if (HIBYTE(HIWORD(style2Write._bgColor)) != 0xFF)
	{
		int rgbVal = RGB2int(style2Write._bgColor);
		TCHAR bgStr[7];
		wsprintf(bgStr, L"%.6X", rgbVal);
		element->SetAttribute(L"bgColor", bgStr);
	}

	if (style2Write._colorStyle != COLORSTYLE_ALL)
	{
		element->SetAttribute(L"colorStyle", style2Write._colorStyle);
	}

	if (style2Write._fontName)
	{
		const TCHAR *oldFontName = element->Attribute(L"fontName");
		if (lstrcmp(oldFontName, style2Write._fontName))
		{
			element->SetAttribute(L"fontName", style2Write._fontName);
			style2Sync._fontName = style2Write._fontName = element->Attribute(L"fontName");
		}
	}

	if (style2Write._fontSize != STYLE_NOT_USED)
	{
		if (!style2Write._fontSize)
			element->SetAttribute(L"fontSize", L"");
		else
			element->SetAttribute(L"fontSize", style2Write._fontSize);
	}

	if (style2Write._fontStyle != STYLE_NOT_USED)
	{
		element->SetAttribute(L"fontStyle", style2Write._fontStyle);
	}


	if (style2Write._keywords)
	{
		TiXmlNode *teteDeNoeud = element->LastChild();

		if (teteDeNoeud)
			teteDeNoeud->SetValue(style2Write._keywords->c_str());
		else
			element->InsertEndChild(TiXmlText(style2Write._keywords->c_str()));
	}
}

void NppParameters::insertUserLang2Tree(TiXmlNode *node, UserLangContainer *userLang)
{
	TiXmlElement *rootElement = (node->InsertEndChild(TiXmlElement(L"UserLang")))->ToElement();

	TCHAR temp[32];
	generic_string udlVersion;
	udlVersion += generic_itoa(SCE_UDL_VERSION_MAJOR, temp, 10);
	udlVersion += L".";
	udlVersion += generic_itoa(SCE_UDL_VERSION_MINOR, temp, 10);

	rootElement->SetAttribute(L"name", userLang->_name);
	rootElement->SetAttribute(L"ext", userLang->_ext);
	rootElement->SetAttribute(L"udlVersion", udlVersion.c_str());

	TiXmlElement *settingsElement = (rootElement->InsertEndChild(TiXmlElement(L"Settings")))->ToElement();
	{
		TiXmlElement *globalElement = (settingsElement->InsertEndChild(TiXmlElement(L"Global")))->ToElement();
		globalElement->SetAttribute(L"caseIgnored",			userLang->_isCaseIgnored ? L"yes":L"no");
		globalElement->SetAttribute(L"allowFoldOfComments",	userLang->_allowFoldOfComments ? L"yes":L"no");
		globalElement->SetAttribute(L"foldCompact",			userLang->_foldCompact ? L"yes":L"no");
		globalElement->SetAttribute(L"forcePureLC",			userLang->_forcePureLC);
		globalElement->SetAttribute(L"decimalSeparator",	   userLang->_decimalSeparator);

		TiXmlElement *prefixElement = (settingsElement->InsertEndChild(TiXmlElement(L"Prefix")))->ToElement();
		for (int i = 0 ; i < SCE_USER_TOTAL_KEYWORD_GROUPS ; ++i)
			prefixElement->SetAttribute(globalMappper().keywordNameMapper[i+SCE_USER_KWLIST_KEYWORDS1], userLang->_isPrefix[i]?L"yes":L"no");
	}

	TiXmlElement *kwlElement = (rootElement->InsertEndChild(TiXmlElement(L"KeywordLists")))->ToElement();

	for (int i = 0 ; i < SCE_USER_KWLIST_TOTAL ; ++i)
	{
		TiXmlElement *kwElement = (kwlElement->InsertEndChild(TiXmlElement(L"Keywords")))->ToElement();
		kwElement->SetAttribute(L"name", globalMappper().keywordNameMapper[i]);
		kwElement->InsertEndChild(TiXmlText(userLang->_keywordLists[i]));
	}

	TiXmlElement *styleRootElement = (rootElement->InsertEndChild(TiXmlElement(L"Styles")))->ToElement();

	for (int i = 0 ; i < SCE_USER_STYLE_TOTAL_STYLES ; ++i)
	{
		TiXmlElement *styleElement = (styleRootElement->InsertEndChild(TiXmlElement(L"WordsStyle")))->ToElement();
		Style style2Write = userLang->_styleArray.getStyler(i);

		if (style2Write._styleID == -1)
			continue;

		styleElement->SetAttribute(L"name", style2Write._styleDesc);

		//if (HIBYTE(HIWORD(style2Write._fgColor)) != 0xFF)
		{
			int rgbVal = RGB2int(style2Write._fgColor);
			TCHAR fgStr[7];
			wsprintf(fgStr, L"%.6X", rgbVal);
			styleElement->SetAttribute(L"fgColor", fgStr);
		}

		//if (HIBYTE(HIWORD(style2Write._bgColor)) != 0xFF)
		{
			int rgbVal = RGB2int(style2Write._bgColor);
			TCHAR bgStr[7];
			wsprintf(bgStr, L"%.6X", rgbVal);
			styleElement->SetAttribute(L"bgColor", bgStr);
		}

		if (style2Write._colorStyle != COLORSTYLE_ALL)
		{
			styleElement->SetAttribute(L"colorStyle", style2Write._colorStyle);
		}

		if (style2Write._fontName)
		{
			styleElement->SetAttribute(L"fontName", style2Write._fontName);
		}

		if (style2Write._fontStyle == STYLE_NOT_USED)
		{
			styleElement->SetAttribute(L"fontStyle", L"0");
		}
		else
		{
			styleElement->SetAttribute(L"fontStyle", style2Write._fontStyle);
		}

		if (style2Write._fontSize != STYLE_NOT_USED)
		{
			if (!style2Write._fontSize)
				styleElement->SetAttribute(L"fontSize", L"");
			else
				styleElement->SetAttribute(L"fontSize", style2Write._fontSize);
		}

		styleElement->SetAttribute(L"nesting", style2Write._nesting);
	}
}

void NppParameters::stylerStrOp(bool op)
{
	for (int i = 0 ; i < _nbUserLang ; ++i)
	{
		for (int j = 0 ; j < SCE_USER_STYLE_TOTAL_STYLES ; ++j)
		{
			Style & style = _userLangArray[i]->_styleArray.getStyler(j);

			if (op == DUP)
			{
				const size_t strLen = lstrlen(style._styleDesc) + 1;
				TCHAR *str = new TCHAR[strLen];
				wcscpy_s(str, strLen, style._styleDesc);
				style._styleDesc = str;
				if (style._fontName)
				{
					const size_t strLen2 = lstrlen(style._fontName) + 1;
					str = new TCHAR[strLen2];
					wcscpy_s(str, strLen2, style._fontName);
					style._fontName = str;
				}
				else
				{
					str = new TCHAR[2];
					str[0] = str[1] = '\0';
					style._fontName = str;
				}
			}
			else
			{
				delete [] style._styleDesc;
				delete [] style._fontName;
			}
		}
	}
}

void NppParameters::addUserModifiedIndex(size_t index)
{
	size_t len = _customizedShortcuts.size();
	bool found = false;
	for (size_t i = 0; i < len; ++i)
	{
		if (_customizedShortcuts[i] == index)
		{
			found = true;
			break;
		}
	}
	if (!found)
	{
		_customizedShortcuts.push_back(index);
	}
}

void NppParameters::addPluginModifiedIndex(size_t index)
{
	size_t len = _pluginCustomizedCmds.size();
	bool found = false;
	for (size_t i = 0; i < len; ++i)
	{
		if (_pluginCustomizedCmds[i] == index)
		{
			found = true;
			break;
		}
	}
	if (!found)
	{
		_pluginCustomizedCmds.push_back(index);
	}
}

void NppParameters::addScintillaModifiedIndex(int index)
{
	size_t len = _scintillaModifiedKeyIndices.size();
	bool found = false;
	for (size_t i = 0; i < len; ++i)
	{
		if (_scintillaModifiedKeyIndices[i] == index)
		{
			found = true;
			break;
		}
	}
	if (!found)
	{
		_scintillaModifiedKeyIndices.push_back(index);
	}
}

void NppParameters::safeWow64EnableWow64FsRedirection(BOOL Wow64FsEnableRedirection)
{
	HMODULE kernel = GetModuleHandle(L"kernel32");
	if (kernel)
	{
		BOOL isWow64 = FALSE;
		typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
		LPFN_ISWOW64PROCESS IsWow64ProcessFunc = (LPFN_ISWOW64PROCESS) GetProcAddress(kernel,"IsWow64Process");

		if (IsWow64ProcessFunc)
		{
			IsWow64ProcessFunc(GetCurrentProcess(),&isWow64);

			if (isWow64)
			{
				typedef BOOL (WINAPI *LPFN_WOW64ENABLEWOW64FSREDIRECTION)(BOOL);
				LPFN_WOW64ENABLEWOW64FSREDIRECTION Wow64EnableWow64FsRedirectionFunc = (LPFN_WOW64ENABLEWOW64FSREDIRECTION)GetProcAddress(kernel, "Wow64EnableWow64FsRedirection");
				if (Wow64EnableWow64FsRedirectionFunc)
				{
					Wow64EnableWow64FsRedirectionFunc(Wow64FsEnableRedirection);
				}
			}
		}
	}
}

void NppParameters::setUdlXmlDirtyFromIndex(size_t i)
{
	for (auto& uxfs : _pXmlUserLangsDoc)
	{
		if (i >= uxfs._indexRange.first && i < uxfs._indexRange.second)
		{
			uxfs._isDirty = true;
			return;
		}
	}
}

/*
Considering we have done:
load default UDL:  3 languges
load a UDL file:   1 languge
load a UDL file:   2 languges
create a UDL:      1 languge
imported a UDL:    1 languge

the evolution to remove UDL one by one:

0[D1]                        0[D1]                        0[D1]                         [D1]                         [D1]
1[D2]                        1[D2]                        1[D2]                        0[D2]                         [D2]
2[D3]  [DUDL, <0,3>]         2[D3]  [DUDL, <0,3>]         2[D3]  [DUDL, <0,3>]         1[D3]  [DUDL, <0,2>]          [D3]  [DUDL, <0,0>]
3[U1]  [NUDL, <3,4>]         3[U1]  [NUDL, <3,4>]         3[U1]  [NUDL, <3,4>]         2[U1]  [NUDL, <2,3>]          [U1]  [NUDL, <0,0>]
4[U2]                        4[U2]                         [U2]                         [U2]                         [U2]
5[U2]  [NUDL, <4,6>]         5[U2]  [NUDL, <4,6>]         4[U2]  [NUDL, <4,5>]         3[U2]  [NUDL, <3,4>]         0[U2]  [NUDL, <0,1>]
6[C1]  [NULL, <6,7>]          [C1]  [NULL, <6,6>]          [C1]  [NULL, <5,5>]          [C1]  [NULL, <4,4>]          [C1]  [NULL, <1,1>]
7[I1]  [NULL, <7,8>]         6[I1]  [NULL, <6,7>]         5[I1]  [NULL, <5,6>]         4[I1]  [NULL, <4,5>]         1[I1]  [NULL, <1,2>]
*/
void NppParameters::removeIndexFromXmlUdls(size_t i)
{
	bool isUpdateBegin = false;
	for (auto& uxfs : _pXmlUserLangsDoc)
	{
		// Find index
		if (!isUpdateBegin && (i >= uxfs._indexRange.first && i < uxfs._indexRange.second)) // found it
		{
			if (uxfs._indexRange.second > 0)
				uxfs._indexRange.second -= 1;
			uxfs._isDirty = true;

			isUpdateBegin = true;
		}

		// Update
		else if (isUpdateBegin)
		{
			if (uxfs._indexRange.first > 0)
				uxfs._indexRange.first -= 1;
			if (uxfs._indexRange.second > 0)
				uxfs._indexRange.second -= 1;
		}
	}
}

void NppParameters::setUdlXmlDirtyFromXmlDoc(const TiXmlDocument* xmlDoc)
{
	for (auto& uxfs : _pXmlUserLangsDoc)
	{
		if (xmlDoc == uxfs._udlXmlDoc)
		{
			uxfs._isDirty = true;
			return;
		}
	}
}

Date::Date(const TCHAR *dateStr)
{
	// timeStr should be Notepad++ date format : YYYYMMDD
	assert(dateStr);
	int D = lstrlen(dateStr);

	if ( 8==D )
	{
		generic_string ds(dateStr);
		generic_string yyyy(ds, 0, 4);
		generic_string mm(ds, 4, 2);
		generic_string dd(ds, 6, 2);

		int y = generic_atoi(yyyy.c_str());
		int m = generic_atoi(mm.c_str());
		int d = generic_atoi(dd.c_str());

		if ((y > 0 && y <= 9999) && (m > 0 && m <= 12) && (d > 0 && d <= 31))
		{
			_year = y;
			_month = m;
			_day = d;
			return;
		}
	}
	now();
}

// The constructor which makes the date of number of days from now
// nbDaysFromNow could be negative if user want to make a date in the past
// if the value of nbDaysFromNow is 0 then the date will be now
Date::Date(int nbDaysFromNow)
{
	const time_t oneDay = (60 * 60 * 24);

	time_t rawtime;
	tm* timeinfo;

	time(&rawtime);
	rawtime += (nbDaysFromNow * oneDay);

	timeinfo = localtime(&rawtime);

	_year = timeinfo->tm_year + 1900;
	_month = timeinfo->tm_mon + 1;
	_day = timeinfo->tm_mday;
}

void Date::now()
{
	time_t rawtime;
	tm* timeinfo;

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	_year = timeinfo->tm_year + 1900;
	_month = timeinfo->tm_mon + 1;
	_day = timeinfo->tm_mday;
}


EolType convertIntToFormatType(int value, EolType defvalue)
{
	switch (value)
	{
		case static_cast<LPARAM>(EolType::windows) :
			return EolType::windows;
		case static_cast<LPARAM>(EolType::macos) :
				return EolType::macos;
		case static_cast<LPARAM>(EolType::unix) :
			return EolType::unix;
		default:
			return defvalue;
	}
}
