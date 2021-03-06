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

#pragma once

#include <vector>
#include <iso646.h>
#include "ScintillaEditView.h"
#include "DocTabView.h"
#include "SplitterContainer.h"
#include "FindReplaceDlg.h"
#include "AboutDlg.h"
#include "RunDlg.h"
#include "StatusBar.h"
#include "lastRecentFileList.h"
#include "GoToLineDlg.h"
#include "FindCharsInRange.h"
#include "columnEditor.h"
#include "WordStyleDlg.h"
#include "trayIconControler.h"
#include "PluginsManager.h"
#include "preferenceDlg.h"
#include "WindowsDlg.h"
#include "RunMacroDlg.h"
#include "DockingManager.h"
#include "Processus.h"
#include "AutoCompletion.h"
#include "SmartHighlighter.h"
#include "ScintillaCtrls.h"
#include "lesDlgs.h"
#include "pluginsAdmin.h"
#include "localization.h"
#include "documentSnapshot.h"
#include "md5Dlgs.h"
#include "ansiCharPanel.h"


#define MENU 0x01
#define TOOLBAR 0x02

#define URL_REG_EXPR "[A-Za-z]+://[A-Za-z0-9_\\-\\+~.:?&@=/%#,;\\{\\}\\(\\)\\[\\]\\|\\*\\!\\\\]+"

enum FileTransferMode {
	TransferClone		= 0x01,
	TransferMove		= 0x02
};

enum WindowStatus {	//bitwise mask
	WindowMainActive	= 0x01,
	WindowSubActive		= 0x02,
	WindowBothActive	= 0x03,	//little helper shortcut
	WindowUserActive	= 0x04,
	WindowMask			= 0x07
};

enum trimOp {
	lineHeader = 0,
	lineTail = 1,
	lineEol = 2
};

enum spaceTab {
	tab2Space = 0,
	space2TabLeading = 1,
	space2TabAll = 2
};

struct TaskListInfo;


struct VisibleGUIConf final	{

	bool isPostIt = false;
	bool isFullScreen = false;

	//Used by both views
	bool isMenuShown = true;
	//bool isToolbarShown;	//toolbar forcefully hidden by hiding rebar
	DWORD_PTR preStyle = (WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN);

	//used by postit only
	bool isTabbarShown = true;
	bool isAlwaysOnTop = false;
	bool isStatusbarShown = true;

	//used by fullscreen only
	WINDOWPLACEMENT _winPlace;

	VisibleGUIConf()	{

		memset(&_winPlace, 0x0, sizeof(_winPlace));
	}
};

class FileDialog;
class Notepad_plus_Window;
class AnsiCharPanel;
class ClipboardHistoryPanel;
class VerticalFileSwitcher;
class ProjectPanel;
class DocumentMap;
class FunctionListPanel;
class FileBrowser;

class Notepad_plus final	{

friend class Notepad_plus_Window;
friend class FileManager;

public:
	Notepad_plus();
	~Notepad_plus();

	enum comment_mode {cm_comment, cm_uncomment, cm_toggle};
	LRESULT init(HWND hwnd);
	LRESULT process(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);

	void killAllChildren();
	void setTitle();
	void getTaskListInfo(TaskListInfo *tli);

	inline void checkUndoState();
	inline void checkClipboard();	
	inline void cClipb(){checkClipboard();}
	inline void cUndoSt(){checkUndoState();}

	// For filtering the modeless Dialog message
	//! \name File Operations
	//@{
	//The doXXX functions apply to a single buffer and dont need to worry about views, with the excpetion of doClose, since closing one view doesnt have to mean the document is gone
	BufferID doOpen(const generic_string& fileName, bool isRecursive = false, bool isReadOnly = false, int encoding = -1, const TCHAR *backupFileName = NULL, FILETIME fileNameTimestamp = {});
	bool doReload(BufferID id, bool alert = true);
	bool doSave(BufferID, const TCHAR * filename, bool isSaveCopy = false);
	void doClose(BufferID, int whichOne, bool doDeleteBackup = false);
	//bool doDelete(const TCHAR *fileName) const {return ::DeleteFile(fileName) != 0;};

	void fileOpen();
	void fileNew();
	bool fileReload();
	bool fileClose(BufferID id = BUFFER_INVALID, int curView = -1);	//use curView to override view to close from
	bool fileCloseAll(bool doDeleteBackup, bool isSnapshotMode = false);
	bool fileCloseAllButCurrent();
	bool fileCloseAllGiven(const std::vector<int>& krvecBufferIndexes);
	bool fileCloseAllToLeft();
	bool fileCloseAllToRight();
	bool fileCloseAllUnchanged();
	bool fileSave(BufferID id = BUFFER_INVALID);
	bool fileSaveAll();
	bool fileSaveSpecific(const generic_string& fileNameToSave);
	bool fileSaveAs(BufferID id = BUFFER_INVALID, bool isSaveCopy = false);
	bool fileDelete(BufferID id = BUFFER_INVALID);
	bool fileRename(BufferID id = BUFFER_INVALID);

	bool switchToFile(BufferID buffer);		//find buffer in active view then in other view.
	//@}

	bool isFileSession(const TCHAR * filename);
	bool isFileWorkspace(const TCHAR * filename);
	void filePrint(bool showDialog);
	void saveScintillasZoom();
	bool saveGUIParams();
	bool saveProjectPanelsParams();
	bool saveFileBrowserParam();
	void saveDockingParams();
	void saveUserDefineLangs();
	void saveShortcuts();
	void saveSession(const Session & session);
	void saveCurrentSession();
	void saveFindHistory();

	void getCurrentOpenedFiles(Session& session, bool includUntitledDoc = false);

	bool fileLoadSession(const TCHAR* fn = nullptr);
	const TCHAR * fileSaveSession(size_t nbFile, TCHAR ** fileNames, const TCHAR *sessionFile2save);
	const TCHAR * fileSaveSession(size_t nbFile = 0, TCHAR** fileNames = nullptr);
	void changeToolBarIcons();

	bool doBlockComment(comment_mode currCommentMode);
	bool doStreamComment();
	bool undoStreamComment(bool tryBlockComment = true);

	bool addCurrentMacro();
	void macroPlayback(Macro);

	void loadLastSession();
	bool loadSession(Session & session, bool isSnapshotMode = false);

	void prepareBufferChangedDialog(Buffer * buffer);
	void notifyBufferChanged(Buffer * buffer, int mask);
	bool findInFinderFiles(FindersInfo *findInFolderInfo);
	bool findInFiles();
	bool replaceInFiles();
	void setFindReplaceFolderFilter(const TCHAR *dir, const TCHAR *filters);
	std::vector<generic_string> addNppComponents(const TCHAR *destDir, const TCHAR *extFilterName, const TCHAR *extFilter);
	std::vector<generic_string> addNppPlugins(const TCHAR *extFilterName, const TCHAR *extFilter);
	int getHtmlXmlEncoding(const TCHAR *fileName) const;
	size_t openedFiles;

	HACCEL getAccTable() const{
		return _accelerator.getAccTable();
	};

	bool emergency(const generic_string& emergencySavedDir);
	void launchDocumentBackupTask();

	Buffer* getCurrentBuffer()	{
		return _pEditView->getCurrentBuffer();
	};
	
	inline void checkMenuItem(int itemID, bool willBeChecked) const {
		::CheckMenuItem(_mainMenuHandle, itemID, MF_BYCOMMAND | (willBeChecked?MF_CHECKED:MF_UNCHECKED));
	}
	
	inline void updateBeginEndSelectPosition(bool is_insert, size_t position, size_t length)	{
		if (static_cast<long long>(position) < beginSelectPos - 1)
			beginSelectPos += is_insert ? static_cast<long long>(length)
			: -static_cast<long long>(length);
		assert(beginSelectPos >= 0);
}

	inline generic_string getPluginListVerStr() const {
		return _pluginsAdminDlg.getPluginListVerStr();	}

// static int rB;
/* BufferID _recBuf[16] ={nullptr};*/
//	BufferID _recBuf =nullptr;

private:
	Notepad_plus_Window *_pPublicInterface = nullptr;
	Window *_pMainWindow = nullptr;
	DockingManager _dockingManager;
	std::vector<int> _internalFuncIDs;

	AutoCompletion _autoCompleteMain;
	AutoCompletion _autoCompleteSub; // each Scintilla has its own autoComplete

	SmartHighlighter _smartHighlighter;
	NativeLangSpeaker _nativeLangSpeaker;
	DocTabView _mainDocTab;
	DocTabView _subDocTab;
	DocTabView* _pDocTab = nullptr;
	DocTabView* _pNonDocTab = nullptr;

	ScintillaEditView _subEditView;
	ScintillaEditView _mainEditView;
	ScintillaEditView _invisibleEditView; // for searches
	ScintillaEditView _fileEditView;      // for FileManager
	ScintillaEditView* _pEditView = nullptr;
	ScintillaEditView* _pNonEditView = nullptr;

	SplitterContainer* _pMainSplitter = nullptr;
	SplitterContainer _subSplitter;

	ContextMenu _tabPopupMenu;
	ContextMenu _tabPopupDropMenu;
	ContextMenu _fileSwitcherMultiFilePopupMenu;

	ToolBar	_toolBar;
	IconList _docTabIconList;

	StatusBar _statusBar;
	bool _toReduceTabBar = false;
	ReBar _rebarTop;
	ReBar _rebarBottom;

	// Dialog
	FindReplaceDlg _findReplaceDlg;
	FindInFinderDlg _findInFinderDlg;

	FindIncrementDlg _incrementFindDlg;
	AboutDlg _aboutDlg;
	DebugInfoDlg _debugInfoDlg;
	RunDlg _runDlg;
	HashFromFilesDlg _md5FromFilesDlg;
	HashFromTextDlg _md5FromTextDlg;
	HashFromFilesDlg _sha2FromFilesDlg;
	HashFromTextDlg _sha2FromTextDlg;
	GoToLineDlg _goToLineDlg;
	ColumnEditorDlg _colEditorDlg;
	WordStyleDlg _configStyleDlg;
	PreferenceDlg _preference;
	FindCharsInRangeDlg _findCharsInRangeDlg;
	PluginsAdminDlg _pluginsAdminDlg;
	DocumentPeeker _documentPeeker;

	// a handle list of all the Notepad++ dialogs
	std::vector<HWND> _hModelessDlgs;

	LastRecentFileList _lastRecentFileList;

	//vector<iconLocator> _customIconVect;

	WindowsMenu _windowsMenu;
	HMENU _mainMenuHandle = NULL;

	bool _sysMenuEntering = false;
	// make sure we don't recursively call doClose when closing the last file with -quitOnEmpty
	bool _isAttemptingCloseOnQuit = false;

	// For FullScreen/PostIt features
	VisibleGUIConf	_beforeSpecialView;
	void fullScreenToggle();
	void postItToggle();

	// Keystroke macro recording and playback
	bool _recordingMacro = false,
	_playingBackMacro = false,
	_recordingSaved = false;
	Macro _macro;
	RunMacroDlg _runMacroDlg;

	// For conflict detection when saving Macros or RunCommands
	ShortcutMapper * _pShortcutMapper = nullptr;


	//For Dynamic selection highlight
	Sci_CharacterRange _prevSelectedRange;

	//Synchronized Scolling
	struct SyncInfo final	{

		int _line = 0;
		int _column = 0;
		bool _isSynScollV = false;
		bool _isSynScollH = false;

		bool doSync() const {return (_isSynScollV || _isSynScollH); }
	}
	_syncInfo;

	bool _isUDDocked = false;

	trayIconControler* _pTrayIco = nullptr;
	int _zoomOriginalValue = 0;

	Accelerator _accelerator;
	ScintillaAccelerator _scintaccelerator;

	PluginsManager _pluginsManager;
	ButtonDlg _restoreButton;

	bool _isFileOpening = false, _isAdministrator = false,
	_linkTriggered = true, _isFolding = false;	// Scintilla hotspot

	ScintillaCtrls _scintillaCtrls4Plugins;

	std::vector<std::pair<int, int> > _hideLinesMarks;
	StyleArray _hotspotStyles;

	AnsiCharPanel* _pAnsiCharPanel = nullptr;
	ClipboardHistoryPanel* _pClipboardHistoryPanel = nullptr;
	VerticalFileSwitcher* _pFileSwitcherPanel = nullptr;
	ProjectPanel* _pProjectPanel_1 = nullptr;
	ProjectPanel* _pProjectPanel_2 = nullptr;
	ProjectPanel* _pProjectPanel_3 = nullptr;
	FileBrowser* _pFileBrowser = nullptr;
	DocumentMap* _pDocMap = nullptr;
	FunctionListPanel* _pFuncList = nullptr;

	BOOL notify(SCNotification *notification);
	void command(int id);

//Document management
	UCHAR _mainWindowStatus = 0; //For 2 views and user dialog if docked

	int _activeView = MAIN_VIEW;

	//User dialog docking
	void dockUserDlg();
	void undockUserDlg();

	//View visibility
	void showView(int whichOne);
	bool viewVisible(int whichOne);
	void hideView(int whichOne);
	bool bothActive() { return (_mainWindowStatus & WindowBothActive) == WindowBothActive; };
	bool reloadLang();
	bool loadStyles();

	int currentView() {	return _activeView;	}

	inline int otherView(int view)	{
		return view == MAIN_VIEW? SUB_VIEW: MAIN_VIEW;	}

	inline int otherView(){
		return _activeView == MAIN_VIEW?SUB_VIEW:MAIN_VIEW;	}
	

	bool canHideView(int whichOne);	//true if view can safely be hidden (no open docs etc)

	bool isEmpty(); // true if we have 1 view with 1 clean, untitled doc

	int switchEditViewTo(int gid);	//activate other view (set focus etc)

	void docGotoAnotherEditView(FileTransferMode mode);	//TransferMode
	void docOpenInNewInstance(FileTransferMode mode, int x = 0, int y = 0);

	void loadBufferIntoView(BufferID id, int whichOne, bool dontClose = false);		//Doesnt _activate_ the buffer
	bool removeBufferFromView(BufferID id, int whichOne);	//Activates alternative of possible, or creates clean document if not clean already

	bool activateBuffer(BufferID id, int whichOne=MAIN_VIEW);			//activate buffer in that view if found
	void notifyBufferActivated(BufferID bufid, int view);
	void performPostReload(int whichOne);
//END: Document management

	int doSaveOrNot(const TCHAR *fn, bool isMulti = false);
	int doReloadOrNot(const TCHAR *fn, bool dirty);
	int doCloseOrNot(const TCHAR *fn);
	int doDeleteOrNot(const TCHAR *fn);

	// void enableMenu(int cmdID, bool doEnable) const;
	void enableCommand(int cmdID, bool doEnable, int which) const;
	void checkDocState();
	void checkMacroState();
	void checkSyncState();
	void dropFiles(HDROP hdrop);
	void checkModifiedDocument(bool bCheckOnlyCurrentBuffer);

	void getMainClientRect(RECT & rc) const;
	void staticCheckMenuAndTB() const;
	void dynamicCheckMenuAndTB() const;
	void enableConvertMenuItems(EolType f) const;
	void checkUnicodeMenuItems() const;

	generic_string getLangDesc(LangType langType, bool getName = false);

	void setLangStatus(LangType langType);
	void setDisplayFormat(EolType f);
	void setUniModeText();
	void checkLangsMenu(int id) const ;
	void setLanguage(LangType langType);
	
	LangType menuID2LangType(int cmdID);

	BOOL processIncrFindAccel(MSG *msg) const;
	BOOL processFindAccel(MSG *msg) const;

	bool isConditionExprLine(int lineNumber);
	int findMachedBracePos(size_t startPos, size_t endPos, char targetSymbol, char matchedSymbol);
	void maintainIndentation(TCHAR ch);

	void addHotSpot();

	void bookmarkAdd(int ln) const	{

		if (ln == -1)
			ln = int(_pEditView->getCurrentLineNumber());
		if (!bookmarkPresent(ln))
			_pEditView->f(SCI_MARKERADD, ln, MARK_BOOKMARK);
	}

	inline bool bookmarkPresent(int ln) const	{

		LRESULT state = _pEditView->f(SCI_MARKERGET, ln);
		return ((state & (1 << MARK_BOOKMARK)) != 0);
	}

	inline void bookmarkDelete(int ln) const	{

		while (bookmarkPresent(ln))
			_pEditView->f(SCI_MARKERDELETE, ln, MARK_BOOKMARK);
	}

	inline void bookmarkToggle() const	{
		bookmarkToggle(int(_pEditView->getCurrentLineNumber()));	}
	
	inline void bookmarkToggle(int ln) const	{

		if (bookmarkPresent(ln))
			bookmarkDelete(ln);
		else
			bookmarkAdd(ln);
	}

	void bookmarkNext(bool forwardScan);
	void bookmarkClearAll() const	{

		_pEditView->f(SCI_MARKERDELETEALL, MARK_BOOKMARK);
	}

	void copyMarkedLines();
	void cutMarkedLines();
	void deleteMarkedLines(bool isMarked);
	void pasteToMarkedLines();
	void deleteMarkedline(int ln);
	void inverseMarks();
	void replaceMarkedline(int ln, const TCHAR *str);
	generic_string getMarkedLine(int ln);
	void findMatchingBracePos(int & braceAtCaret, int & braceOpposite);
	bool selectBracePairContainCaret();
	bool braceMatch();

	void activateNextDoc(bool direction);
	void activateDoc(size_t pos);

	void updateStatusBar();
	inline void updateStatusBar(bool);
	size_t getSelectedCharNumber(UniMode);
	size_t getCurrentDocCharCount(UniMode u);
	size_t getSelectedAreas();
	size_t getSelectedBytes();
	bool isFormatUnicode(UniMode);
	int getBOMSize(UniMode);

	void showAutoComp();
	void autoCompFromCurrentFile(bool autoInsert = true);
	void showFunctionComp();
	void showPathCompletion();

	//void changeStyleCtrlsLang(HWND hDlg, int *idArray, const char **translatedText);
	bool replaceInOpenedFiles();
	bool findInOpenedFiles();
	bool findInCurrentFile();

	void getMatchedFileNames(const TCHAR *dir, const std::vector<generic_string> & patterns, std::vector<generic_string> & fileNames, bool isRecursive, bool isInHiddenDir);
	void doSynScorll(HWND hW);
	void setDefOpenSaveDir(const TCHAR *dir);
	bool str2Cliboard(const generic_string & str2cpy);

	bool getIntegralDockingData(tTbData & dockData, int & iCont, bool & isVisible);
	int getLangFromMenuName(const TCHAR * langName);
	generic_string getLangFromMenu(const Buffer * buf);

	generic_string exts2Filters(const generic_string& exts) const;
	int setFileOpenSaveDlgFilters(FileDialog & fDlg, int langType = -1);
	Style * getStyleFromName(const TCHAR *styleName);
	bool dumpFiles(const TCHAR * outdir, const TCHAR * fileprefix = L"");	//helper func
	void drawTabbarColoursFromStylerArray();

	std::vector<generic_string> loadCommandlineParams(const TCHAR * commandLine, const CmdLineParams * pCmdParams)	{

		const CmdLineParamsDTO dto = CmdLineParamsDTO::FromCmdLineParams(*pCmdParams);
		return loadCommandlineParams(commandLine, &dto);
	}
	std::vector<generic_string> loadCommandlineParams(const TCHAR * commandLine, const CmdLineParamsDTO * pCmdParams);
	bool noOpenedDoc() const;
	bool goToPreviousIndicator(int indicID2Search, bool isWrap = true) const;
	bool goToNextIndicator(int indicID2Search, bool isWrap = true) const;
	int wordCount();

	void wsTabConvert(spaceTab whichWay);
	void doTrim(trimOp whichPart);
	void removeEmptyLine(bool isBlankContained);
	void removeDuplicateLines();
	void initAnsiCharPanel();
	void launchClipboardHistoryPanel();
	void launchFileSwitcherPanel();
	void launchProjectPanel(int cmdID, ProjectPanel ** pProjPanel, int panelID);
	void launchDocMap();
	void launchFunctionList();
	void launchFileBrowser(const std::vector<generic_string> & folders, bool fromScratch = false);

	static DWORD WINAPI backupDocument(void *params);

	static DWORD WINAPI monitorFileOnChange(void * params);
	struct MonitorInfo final {
		MonitorInfo(Buffer *buf, HWND nppHandle) :
			_buffer(buf), _nppHandle(nppHandle) {};
		Buffer *_buffer = nullptr;
		HWND _nppHandle = nullptr;
	};
	void monitoringStartOrStopAndUpdateUI(Buffer* pBuf, bool isStarting);
	long long beginSelectPos = -1;
	bool offsetSB = 0;
};

void Notepad_plus::updateStatusBar(bool full)	{
	size_t j = 0;
	if (_mainWindowStatus & WindowSubActive)
		for (size_t i = 0; i < _subDocTab.nbItem() ; ++i)
			if (_mainDocTab.getIndexByBuffer(MainFileManager.getBufferByID(_subDocTab.getBufferByIndex(i))) == -1)	//skip clone, count only one same doc in either view
				++j;
	generic_string s = std::to_wstring(_mainDocTab.nbItem() + j);
	s += L" ";
	s += _pEditView->getCurrentBuffer()->getFileName();
	_statusBar.setText(STATUSBAR_DOC_NAME,s.c_str());
	if (full) updateStatusBar();

}

void Notepad_plus::checkUndoState()	{
enableCommand(IDM_EDIT_UNDO, _pEditView->f(SCI_CANUNDO) != 0, MENU | TOOLBAR);
enableCommand(IDM_EDIT_REDO, _pEditView->f(SCI_CANREDO) != 0, MENU | TOOLBAR);
}
void Notepad_plus::checkClipboard()	{
	bool hasSelection = (_pEditView->f(SCI_GETSELECTIONSTART) != _pEditView->f(SCI_GETSELECTIONEND));
	enableCommand(IDM_EDIT_CUT, hasSelection, MENU | TOOLBAR);
	enableCommand(IDM_EDIT_COPY, hasSelection, MENU | TOOLBAR);

	enableCommand(IDM_EDIT_PASTE, _pEditView->f(SCI_CANPASTE), MENU | TOOLBAR);
	// enableCommand(IDM_EDIT_DELETE, hasSelection, MENU | TOOLBAR);
	// enableCommand(IDM_EDIT_UPPERCASE, hasSelection, MENU);
	// enableCommand(IDM_EDIT_LOWERCASE, hasSelection, MENU);
	// enableCommand(IDM_EDIT_PROPERCASE_FORCE, hasSelection, MENU);
	// enableCommand(IDM_EDIT_PROPERCASE_BLEND, hasSelection, MENU);
	// enableCommand(IDM_EDIT_SENTENCECASE_FORCE, hasSelection, MENU);
	// enableCommand(IDM_EDIT_SENTENCECASE_BLEND, hasSelection, MENU);
	// enableCommand(IDM_EDIT_INVERTCASE, hasSelection, MENU);
	// enableCommand(IDM_EDIT_RANDOMCASE, hasSelection, MENU);
}

