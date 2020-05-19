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
#pragma warning(push)
#pragma warning(disable:4706)

#include "Scintilla.h"
#include "ScintillaRef.h"
#include "SciLexer.h"
#include "Buffer.h"
#include "colors.h"
#include "UserDefineDialog.h"
#include "rgba_icons.h"

#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL 0x020A
#endif //WM_MOUSEWHEEL

#ifndef WM_MOUSEHWHEEL
#define WM_MOUSEHWHEEL 0x020E
#endif //WM_MOUSEHWHEEL

#ifndef WM_APPCOMMAND
#define WM_APPCOMMAND                   0x0319
#define APPCOMMAND_BROWSER_BACKWARD       1
#define APPCOMMAND_BROWSER_FORWARD        2
#define FAPPCOMMAND_MASK  0xF000
#define GET_APPCOMMAND_LPARAM(lParam) ((short)(HIWORD(lParam) & ~FAPPCOMMAND_MASK))
#endif //WM_APPCOMMAND

#define NB_WORD_LIST 4
#define WORD_LIST_LEN 256

typedef sptr_t(*SCINTILLA_FUNC) (void *, unsigned int, uptr_t, sptr_t);
typedef void * SCINTILLA_PTR;

#define WM_DOCK_USERDEFINE_DLG      (SCINTILLA_USER + 1)
#define WM_UNDOCK_USERDEFINE_DLG    (SCINTILLA_USER + 2)
#define WM_CLOSE_USERDEFINE_DLG     (SCINTILLA_USER + 3)
#define WM_REMOVE_USERLANG          (SCINTILLA_USER + 4)
#define WM_RENAME_USERLANG          (SCINTILLA_USER + 5)
#define WM_REPLACEALL_INOPENEDDOC   (SCINTILLA_USER + 6)
#define WM_FINDALL_INOPENEDDOC      (SCINTILLA_USER + 7)
#define WM_DOOPEN                   (SCINTILLA_USER + 8)
#define WM_FINDINFILES              (SCINTILLA_USER + 9)
#define WM_REPLACEINFILES           (SCINTILLA_USER + 10)
#define WM_FINDALL_INCURRENTDOC     (SCINTILLA_USER + 11)
#define WM_FRSAVE_INT               (SCINTILLA_USER + 12)
#define WM_FRSAVE_STR               (SCINTILLA_USER + 13)
#define WM_FINDALL_INCURRENTFINDER  (SCINTILLA_USER + 14)

const int NB_FOLDER_STATE = 7;

// Codepage
const int CP_CHINESE_TRADITIONAL = 950;
const int CP_CHINESE_SIMPLIFIED = 936;
const int CP_JAPANESE = 932;
const int CP_KOREAN = 949;
const int CP_GREEK = 1253;

//wordList
#define LIST_NONE 0
#define LIST_0 1
#define LIST_1 2
#define LIST_2 4
#define LIST_3 8
#define LIST_4 16
#define LIST_5 32
#define LIST_6 64
#define LIST_7 128
#define LIST_8 256

enum { fold_collapse, fold_uncollapse, MAX_FOLD_COLLAPSE_LEVEL	=8 };

enum TextCase : UCHAR
{
	UPPERCASE,
	LOWERCASE,
	TITLECASE_FORCE,
	TITLECASE_BLEND,
	SENTENCECASE_FORCE,
	SENTENCECASE_BLEND,
	INVERTCASE,
	RANDOMCASE
};

enum : UCHAR {
	BASE_10 = 0x00, // Dec
	BASE_16 = 0x01, // Hex
	BASE_08 = 0x02, // Oct
	BASE_02 = 0x03, // Bin
	MASK_FORMAT = 0x03,
	MASK_ZERO_LEADING = 0x04
};

//const int MARK_LINEMODIFIEDUNSAVED = 20;
//const int MARK_LINEMODIFIEDSAVED = 19;
// 24 - 16 reserved for Notepad++ internal used
// 15 - 0  are free to use for plugins
enum {
	MARK_HIDELINESUNDERLINE = 21, MARK_HIDELINESEND, MARK_HIDELINESBEGIN, MARK_BOOKMARK	};


int getNbDigits(int aNum, int base);
HMODULE loadSciLexerDll();

TCHAR * int2str(TCHAR *str, int strLen, int number, int base, int nbChiffre, bool isZeroLeading);

typedef LRESULT (WINAPI *CallWindowProcFunc) (WNDPROC,HWND,UINT,WPARAM,LPARAM);

const bool L2R = true;
const bool R2L = false;

struct ColumnModeInfo {
	int _selLpos;
	int _selRpos;
	int _order; // 0 based index
	bool _direction; // L2R or R2L
	int _nbVirtualCaretSpc;
	int _nbVirtualAnchorSpc;

	ColumnModeInfo() : _selLpos(0), _selRpos(0), _order(-1), _direction(L2R), _nbVirtualAnchorSpc(0), _nbVirtualCaretSpc(0){};
	ColumnModeInfo(int lPos, int rPos, int order, bool dir = L2R, int vAnchorNbSpc = 0, int vCaretNbSpc = 0)
		: _selLpos(lPos), _selRpos(rPos), _order(order), _direction(dir), _nbVirtualAnchorSpc(vAnchorNbSpc), _nbVirtualCaretSpc(vCaretNbSpc){};

	bool isValid() const {
		return (_order >= 0 && _selLpos >= 0 && _selRpos >= 0 && _selLpos <= _selRpos);
	};
};

//
// SortClass for vector<ColumnModeInfo>
// sort in _order : increased order
struct SortInSelectOrder {
	bool operator() (ColumnModeInfo & l, ColumnModeInfo & r) {
		return (l._order < r._order);
	}
};

//
// SortClass for vector<ColumnModeInfo>
// sort in _selLpos : increased order
struct SortInPositionOrder {
	bool operator() (ColumnModeInfo & l, ColumnModeInfo & r) {
		return (l._selLpos < r._selLpos);
	}
};

typedef std::vector<ColumnModeInfo> ColumnModeInfos;

struct LanguageName {
	const TCHAR * lexerName;
	const TCHAR * shortName;
	const TCHAR * longName;
	LangType LangID;
	int lexerID;
};

class ISorter;

class ScintillaEditView : public Window	{

friend class Finder;
public:
	ScintillaEditView(): Window() {
		++_refCount;
	};

	virtual ~ScintillaEditView()
	{
		--_refCount;

		if ((!_refCount)&&(_hLib))	{

			::FreeLibrary(_hLib);

			for (BufferStyleMap::iterator it(_hotspotStyles.begin()); it != _hotspotStyles.end(); ++it )	{

				for (StyleMap::iterator it2(it->second->begin()) ; it2 != it->second->end() ; ++it2)	{

					delete [] it2->second._fontName;
				}
				delete it->second;
			}
		}
	};

	virtual void destroy()	{

		::DestroyWindow(_hSelf);
		_hSelf = NULL;
	};

	virtual void init(HINSTANCE hInst, HWND hPere);

	inline LRESULT f(UINT Msg, WPARAM wParam=0, LPARAM lParam=0) const {
		try {
			return _pScintillaFunc(_pScintillaPtr, Msg, wParam, lParam);
		}
		catch (...)
		{
			return -1;
		}
	};

	void activateBuffer(BufferID buffer);

	void getCurrentFoldStates(std::vector<size_t> & lineStateVector);
	void syncFoldStateWith(const std::vector<size_t> & lineStateVectorNew);

	void getText(char *dest, size_t start, size_t end) const;
	void getGenericText(TCHAR *dest, size_t destlen, size_t start, size_t end) const;
	void getGenericText(TCHAR *dest, size_t deslen, int start, int end, int *mstart, int *mend) const;
	generic_string getGenericTextAsString(size_t start, size_t end) const;
	void insertGenericTextFrom(size_t position, const TCHAR *text2insert) const;
	void replaceSelWith(const char * replaceText);

	int getSelectedTextCount() {
		Sci_CharacterRange range = getSelection();
		return (range.cpMax - range.cpMin);
	};

	void getVisibleStartAndEndPosition(int * startPos, int * endPos);
	char * getWordFromRange(char * txt, int size, int pos1, int pos2);
	char * getSelectedText(char * txt, int size, bool expand = true);
	char * getWordOnCaretPos(char * txt, int size);
	TCHAR * getGenericWordOnCaretPos(TCHAR * txt, int size);
	TCHAR * getGenericSelectedText(TCHAR * txt, int size, bool expand = true);
	int searchInTarget(const TCHAR * Text2Find, size_t lenOfText2Find, size_t fromPos, size_t toPos) const;
	void appendGenericText(const TCHAR * text2Append) const;
	void addGenericText(const TCHAR * text2Append) const;
	void addGenericText(const TCHAR * text2Append, long *mstart, long *mend) const;
	int replaceTarget(const TCHAR * str2replace, int fromTargetPos = -1, int toTargetPos = -1) const;
	int replaceTargetRegExMode(const TCHAR * re, int fromTargetPos = -1, int toTargetPos = -1) const;
	void showAutoC(size_t lenEntered, const TCHAR * list);
	void showCallTip(int startPos, const TCHAR * def);
	generic_string getLine(size_t lineNumber);
	void getLine(size_t lineNumber, TCHAR * line, int lineBufferLen);
	void addText(size_t length, const char *buf);

	void insertNewLineAboveCurrentLine();
	void insertNewLineBelowCurrentLine();

	void saveCurrentPos();
	void restoreCurrentPosPreStep();
	void restoreCurrentPosPostStep();

	void beginOrEndSelect();
	bool beginEndSelectedIsStarted() const {
		return _beginSelectPosition != -1;
	};

	int getCurrentDocLen() const {
		return static_cast<int32_t>(f(SCI_GETLENGTH));
	};

	Sci_CharacterRange getSelection() const {
		Sci_CharacterRange crange;
		crange.cpMin = long(f(SCI_GETSELECTIONSTART));
		crange.cpMax = long(f(SCI_GETSELECTIONEND));
		return crange;
	};

	void getWordToCurrentPos(TCHAR * str, int strLen) const {
		auto caretPos = f(SCI_GETCURRENTPOS);
		auto startPos = f(SCI_WORDSTARTPOSITION, caretPos, true);

		str[0] = '\0';
		if ((caretPos - startPos) < strLen)
			getGenericText(str, strLen, startPos, caretPos);
	};

	void doUserDefineDlg(bool willBeShown = true, bool isRTL = false) {
		_userDefineDlg.doDialog(willBeShown, isRTL);
	};

	static UserDefineDialog * getUserDefineDlg() {return &_userDefineDlg;};

	void setCaretColorWidth(int color, int width = 1) const {
		f(SCI_SETCARETFORE, color);
		f(SCI_SETCARETWIDTH, width);
	};

	void beSwitched() {
		_userDefineDlg.setScintilla(this);
	};

	//Marge member and method
	static const int _SC_MARGE_LINENUMBER;
	static const int _SC_MARGE_SYBOLE;
	static const int _SC_MARGE_FOLDER;
	//static const int _SC_MARGE_MODIFMARKER;

	void showMargin(int whichMarge, bool willBeShowed = true) {
		if (whichMarge == _SC_MARGE_LINENUMBER)
			showLineNumbersMargin(willBeShowed);
		else	{

			int width = 3;
			if (whichMarge == _SC_MARGE_SYBOLE)
				width = param._dpiManager.scaleX(100) >= 150 ? 20 : 16;
			else if (whichMarge == _SC_MARGE_FOLDER)
				width = param._dpiManager.scaleX(100) >= 150 ? 18 : 14;
			f(SCI_SETMARGINWIDTHN, whichMarge, willBeShowed ? width : 0);
		}
	};

	bool hasMarginShowed(int witchMarge) {
		return (f(SCI_GETMARGINWIDTHN, witchMarge, 0));
	};

	void updateBeginEndSelectPosition(bool is_insert, size_t position, size_t length);
	void marginClick(Sci_Position position, int modifiers);

	void setMakerStyle(folderStyle style) {
		bool display;
		if (style == FOLDER_STYLE_NONE)	{

			style = FOLDER_STYLE_BOX;
			display = false;
		}
		else	{

			display = true;
		}

		COLORREF foldfgColor = white, foldbgColor = grey, activeFoldFgColor = red;
		getFoldColor(foldfgColor, foldbgColor, activeFoldFgColor);

		for (int i = 0 ; i < NB_FOLDER_STATE ; ++i)
			defineMarker(_markersArray[FOLDER_TYPE][i], _markersArray[style][i], foldfgColor, foldbgColor, activeFoldFgColor);
		showMargin(ScintillaEditView::_SC_MARGE_FOLDER, display);
	};


	void setWrapMode(lineWrapMethod meth) {
		int mode = (meth == LINEWRAP_ALIGNED)?SC_WRAPINDENT_SAME:\
				(meth == LINEWRAP_INDENT)?SC_WRAPINDENT_INDENT:SC_WRAPINDENT_FIXED;
		f(SCI_SETWRAPINDENTMODE, mode);
	};


	void showWSAndTab(bool willBeShowed = true) {
		f(SCI_SETVIEWWS, willBeShowed?SCWS_VISIBLEALWAYS:SCWS_INVISIBLE);
		f(SCI_SETWHITESPACESIZE, 2, 0);
	};

	void showEOL(bool willBeShowed = true) {
		f(SCI_SETVIEWEOL, willBeShowed);
	};

	bool isEolVisible() {
		return (f(SCI_GETVIEWEOL));
	};
	void showInvisibleChars(bool willBeShowed = true) {
		showWSAndTab(willBeShowed);
		showEOL(willBeShowed);
	};

	bool isInvisibleCharsShown() {
		return (f(SCI_GETVIEWWS));
	};

	void showIndentGuideLine(bool willBeShowed = true);

	bool isShownIndentGuide() const {
		return (f(SCI_GETINDENTATIONGUIDES));
	};

inline	void wrap(bool willBeWrapped = true) {
		f(SCI_SETWRAPMODE, willBeWrapped);
	};

inline	bool isWrap() const {
		return (f(SCI_GETWRAPMODE) == SC_WRAP_WORD);
	};

inline	bool isWrapSymbolVisible() const {
		return (f(SCI_GETWRAPVISUALFLAGS) != SC_WRAPVISUALFLAG_NONE);
	};

inline	void showWrapSymbol(bool willBeShown = true) {
		f(SCI_SETWRAPVISUALFLAGSLOCATION, SC_WRAPVISUALFLAGLOC_DEFAULT);
		f(SCI_SETWRAPVISUALFLAGS, willBeShown?SC_WRAPVISUALFLAG_END:SC_WRAPVISUALFLAG_NONE);
	};

inline	size_t getCurrentLineNumber()const {
	return static_cast<size_t>(f(SCI_LINEFROMPOSITION, f(SCI_GETCURRENTPOS)));
	};

inline	long getCurrentColumnNumber() const {
		return long(f(SCI_GETCOLUMN, f(SCI_GETCURRENTPOS)));
	};

inline	size_t getOffset()const {
		return static_cast<size_t>(f(SCI_GETCURRENTPOS));
	};
	
inline	int32_t lastZeroBasedLineNumber() const {
		auto endPos = f(SCI_GETLENGTH);
		return static_cast<int32_t>(f(SCI_LINEFROMPOSITION, endPos));
	};

inline	long getCurrentXOffset()const{
		return long(f(SCI_GETXOFFSET));
	};

inline	void setCurrentXOffset(long xOffset){
		f(SCI_SETXOFFSET,xOffset);
	};

inline	void scroll(int column, int line){
		f(SCI_LINESCROLL, column, line);
	};

inline	long getCurrentPointX()const{
		return long (f(SCI_POINTXFROMPOSITION, 0, f(SCI_GETCURRENTPOS)));
	};

inline	long getCurrentPointY()const{
		return long (f(SCI_POINTYFROMPOSITION, 0, f(SCI_GETCURRENTPOS)));
	};

	long getTextHeight()const{
		return long(f(SCI_TEXTHEIGHT));
	};

	void gotoLine(int line){
		if (line < f(SCI_GETLINECOUNT))
			f(SCI_GOTOLINE,line);
	};

	bool getSelectedCount(int & selByte, int & selLine) const {
		// return false if it's multi-selection or rectangle selection
		if ((f(SCI_GETSELECTIONS) > 1) || f(SCI_SELECTIONISRECTANGLE))
			return false;
		long pStart = long(f(SCI_GETSELECTIONSTART));
		long pEnd = long(f(SCI_GETSELECTIONEND));
		selByte = pEnd - pStart;

		long lStart = long(f(SCI_LINEFROMPOSITION, pStart));
		long lEnd = long(f(SCI_LINEFROMPOSITION, pEnd));
		selLine = lEnd - lStart;
		if (selLine || selByte)
			++selLine;

		return true;
	};

	long getUnicodeSelectedLength() const	{

		// return -1 if it's multi-selection or rectangle selection
		if ((f(SCI_GETSELECTIONS) > 1) || f(SCI_SELECTIONISRECTANGLE))
			return -1;

		long start = long(f(SCI_GETSELECTIONSTART));
		long end = long(f(SCI_GETSELECTIONEND));
		long length = long(f(SCI_COUNTCHARACTERS, start, end));

		return length;
	};


	long getLineLength(int line) const {
		return long(f(SCI_GETLINEENDPOSITION, line) - f(SCI_POSITIONFROMLINE, line));
	};

	long getLineIndent(int line) const {
		return long(f(SCI_GETLINEINDENTATION, line));
	};

	void setLineIndent(int line, int indent) const;

	void showLineNumbersMargin(bool show)	{

		if (show == _lineNumbersShown) return;
		_lineNumbersShown = show;
		if (show)
			updateLineNumberWidth();
		else
			f(SCI_SETMARGINWIDTHN, _SC_MARGE_LINENUMBER, 0);
	}

	void updateLineNumberWidth();

	void setCurrentLineHiLiting(bool isHiliting, COLORREF bgColor) const {
		f(SCI_SETCARETLINEVISIBLE, isHiliting);
		if (!isHiliting)
			return;
		f(SCI_SETCARETLINEBACK, bgColor);
	};

	bool isCurrentLineHiLiting() const {
		return (f(SCI_GETCARETLINEVISIBLE));
	};

	void performGlobalStyles();

	void expand(size_t& line, bool doExpand, bool force = false, int visLevels = 0, int level = -1);

	std::pair<int, int> getSelectionLinesRange() const;
	void currentLinesUp() const;
	void currentLinesDown() const;

	void changeCase(__inout wchar_t * const strWToConvert, const int & nbChars, const TextCase & caseToConvert) const;
	void convertSelectedTextTo(const TextCase & caseToConvert);
	void setMultiSelections(const ColumnModeInfos & cmi);

	void convertSelectedTextToLowerCase() {
		// if system is w2k or xp
		if (param.isTransparentAvailable())
			convertSelectedTextTo(LOWERCASE);
		else
			f(SCI_LOWERCASE);
	};

	void convertSelectedTextToUpperCase() {
		// if system is w2k or xp
		if (param.isTransparentAvailable())
			convertSelectedTextTo(UPPERCASE);
		else
			f(SCI_UPPERCASE);
	};

	void convertSelectedTextToNewerCase(const TextCase & caseToConvert) {
		// if system is w2k or xp
		if (param.isTransparentAvailable())
			convertSelectedTextTo(caseToConvert);
		else
			::MessageBox(_hSelf, L"This function needs a newer OS version.", L"Change Case Error", MB_OK | MB_ICONHAND);
	};

	bool isFoldIndentationBased() const;
	void collapseFoldIndentationBased(int level2Collapse, bool mode);
	void collapse(int level2Collapse, bool mode);
	void foldAll(bool);
	inline void foldAllToggle();
	void putMvmntInView(int p, int e, int =0);
	inline void fold(size_t line, bool mode);
	inline void toggleFold(size_t line, bool rec =0);
	inline int getCodepage() const {return _codepage;};

	ColumnModeInfos getColumnModeSelectInfo();

	void columnReplace(ColumnModeInfos & cmi, const TCHAR *str);
	void columnReplace(ColumnModeInfos & cmi, int initial, int incr, int repeat, UCHAR format);

	void foldChanged(size_t line, int levelNow, int levelPrev);
	inline void clearIndicator(int indicatorNumber) {
		int docStart = 0;
		int docEnd = getCurrentDocLen();
		f(SCI_SETINDICATORCURRENT, indicatorNumber);
		f(SCI_INDICATORCLEARRANGE, docStart, docEnd-docStart);
	};

	static LanguageName langNames[L_EXTERNAL+1];

	void bufferUpdated(Buffer * buffer, int mask);
	BufferID getCurrentBufferID() { return _currentBufferID; };
	Buffer * getCurrentBuffer() { return _currentBuffer; };
	void setCurrentBuffer(Buffer *buf2set) { _currentBuffer = buf2set; };
	void styleChange();

	void hideLines();

	bool markerMarginClick(int lineNumber);	//true if it did something
	void notifyMarkers(Buffer * buf, bool isHide, int location, bool del);
	void runMarkers(bool doHide, size_t searchStart, bool endOfDoc, bool doDelete);

	bool isSelecting() const {
		static Sci_CharacterRange previousSelRange = getSelection();
		Sci_CharacterRange currentSelRange = getSelection();

		if (currentSelRange.cpMin == currentSelRange.cpMax)	{

			previousSelRange = currentSelRange;
			return false;
		}

		if ((previousSelRange.cpMin == currentSelRange.cpMin) || (previousSelRange.cpMax == currentSelRange.cpMax))	{

			previousSelRange = currentSelRange;
			return true;
		}

		previousSelRange = currentSelRange;
		return false;
	};

	void defineDocType(LangType typeDoc);	//setup stylers for active document

	void addCustomWordChars();
	void restoreDefaultWordChars();
	void setWordChars();

	void mouseWheel(WPARAM wParam, LPARAM lParam) {
		scintillaNew_Proc(_hSelf, WM_MOUSEWHEEL, wParam, lParam);
	};

	void setHotspotStyle(Style& styleToSet);
	void setTabSettings(Lang *lang);
	bool isWrapRestoreNeeded() const {return _wrapRestoreNeeded;};
	void setWrapRestoreNeeded(bool isWrapRestoredNeeded) {_wrapRestoreNeeded = isWrapRestoredNeeded;};

	bool isCJK() const {
		return ((_codepage == CP_CHINESE_TRADITIONAL) || (_codepage == CP_CHINESE_SIMPLIFIED) ||
				(_codepage == CP_JAPANESE) || (_codepage == CP_KOREAN));
	};
	void scrollPosToCenter(size_t pos);
	generic_string getEOLString();
	void setBorderEdge(bool doWithBorderEdge);
	void sortLines(size_t fromLine, size_t toLine, ISorter *pSort);
	void changeTextDirection(bool isRTL);
	bool isTextDirectionRTL() const;

	inline bool ScintillaEditView::isBeingSelect()	{
	return f(SCI_GETCURRENTPOS) != f(SCI_GETANCHOR);
	}

	inline void ScintillaEditView::toLeftEndSelect()	{
	if (f(SCI_GETCURRENTPOS) > f(SCI_GETANCHOR))
		f(SCI_SWAPMAINANCHORCARET);
	}

	inline void ScintillaEditView::toRightEndSelect()	{
	if (f(SCI_GETCURRENTPOS) < f(SCI_GETANCHOR))
		f(SCI_SWAPMAINANCHORCARET);
	}

protected:
	static HINSTANCE _hLib;
	static int _refCount;

	static UserDefineDialog _userDefineDlg;

	static const int _markersArray[][NB_FOLDER_STATE];

	static LRESULT CALLBACK scintillaStatic_Proc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
	LRESULT scintillaNew_Proc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);

	SCINTILLA_FUNC _pScintillaFunc = nullptr;
	SCINTILLA_PTR  _pScintillaPtr = nullptr;
	static WNDPROC _scintillaDefaultProc;
	CallWindowProcFunc _callWindowProc = nullptr;
	BufferID attachDefaultDoc();

	//Store the current buffer so it can be retrieved later
	BufferID _currentBufferID = nullptr;
	Buffer * _currentBuffer = nullptr;

	int _codepage = CP_ACP;
	bool _lineNumbersShown = false;
	bool _wrapRestoreNeeded = false;
	bool _positionRestoreNeeded = false;
	uint32_t _restorePositionRetryCount = 0;

	typedef std::unordered_map<int, Style> StyleMap;
	typedef std::unordered_map<BufferID, StyleMap*> BufferStyleMap;
	BufferStyleMap _hotspotStyles;

	long long _beginSelectPosition = -1;

	static std::string _defaultCharList;

//Lexers and Styling
	void restyleBuffer();
	const char * getCompleteKeywordList(std::basic_string<char> & kwl, LangType langType, int keywordIndex);
	void setKeywords(LangType langType, const char *keywords, int index);
	void setLexer(int lexerID, LangType langType, int whichList);
	void makeStyle(LangType langType, const TCHAR **keywordArray = NULL);
	void setStyle(Style styleToSet);			//NOT by reference	(style edited)
	void setSpecialStyle(const Style & styleToSet);	//by reference
	void setSpecialIndicator(const Style & styleToSet) {
		f(SCI_INDICSETFORE, styleToSet._styleID, styleToSet._bgColor);
	};

	//Complex lexers (same lexer, different language)
	void setXmlLexer(LangType type);
 	void setCppLexer(LangType type);
	void setJsLexer();
	void setTclLexer();
	void setObjCLexer(LangType type);
	void setUserLexer(const TCHAR *userLangName = NULL);
	void setExternalLexer(LangType typeDoc);
	void setEmbeddedJSLexer();
	void setEmbeddedPhpLexer();
	void setEmbeddedAspLexer();
	void setJsonLexer();
	//Simple lexers
	void setCssLexer() {
		setLexer(SCLEX_CSS, L_CSS, LIST_0 | LIST_1);
	};

	void setLuaLexer() {
		setLexer(SCLEX_LUA, L_LUA, LIST_0 | LIST_1 | LIST_2 | LIST_3);
	};

	void setMakefileLexer() {
		f(SCI_SETLEXER, SCLEX_MAKEFILE);
		makeStyle(L_MAKEFILE);
	};

	void setIniLexer() {
		f(SCI_SETLEXER, SCLEX_PROPERTIES);
		f(SCI_STYLESETEOLFILLED, SCE_PROPS_SECTION, true);
		makeStyle(L_INI);
		f(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold"), reinterpret_cast<LPARAM>("1"));
		f(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.compact"), reinterpret_cast<LPARAM>("0"));
	};


	void setSqlLexer() {
		const bool kbBackSlash = nGUI._backSlashIsEscapeCharacterForSql;
		setLexer(SCLEX_SQL, L_SQL, LIST_0 | LIST_1 | LIST_4);
		f(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("sql.backslash.escapes"), reinterpret_cast<LPARAM>(kbBackSlash ? "1" : "0"));
	};

	void setBashLexer() {
		setLexer(SCLEX_BASH, L_BASH, LIST_0);
	};

	void setVBLexer() {
		setLexer(SCLEX_VB, L_VB, LIST_0);
	};

	void setPascalLexer() {
		setLexer(SCLEX_PASCAL, L_PASCAL, LIST_0);
		f(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.preprocessor"), reinterpret_cast<LPARAM>("1"));
	};

	void setPerlLexer() {
		setLexer(SCLEX_PERL, L_PERL, LIST_0);
	};

	void setPythonLexer() {
		setLexer(SCLEX_PYTHON, L_PYTHON, LIST_0 | LIST_1);
		f(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.quotes.python"), reinterpret_cast<LPARAM>("1"));
	};

	void setBatchLexer() {
		setLexer(SCLEX_BATCH, L_BATCH, LIST_0);
	};

	void setTeXLexer() {
		for (int i = 0 ; i < 4 ; ++i)
			f(SCI_SETKEYWORDS, i, reinterpret_cast<LPARAM>(L""));
		setLexer(SCLEX_TEX, L_TEX, 0);
	};

	void setNsisLexer() {
		setLexer(SCLEX_NSIS, L_NSIS, LIST_0 | LIST_1 | LIST_2 | LIST_3);
	};

	void setFortranLexer() {
		setLexer(SCLEX_FORTRAN, L_FORTRAN, LIST_0 | LIST_1 | LIST_2);
	};

	void setFortran77Lexer() {
		setLexer(SCLEX_F77, L_FORTRAN_77, LIST_0 | LIST_1 | LIST_2);
	};

	void setLispLexer(){
		setLexer(SCLEX_LISP, L_LISP, LIST_0 | LIST_1);
	};

	void setSchemeLexer(){
		setLexer(SCLEX_LISP, L_SCHEME, LIST_0 | LIST_1);
	};

	void setAsmLexer(){
		setLexer(SCLEX_ASM, L_ASM, LIST_0 | LIST_1 | LIST_2 | LIST_3 | LIST_4 | LIST_5);
	};

	void setDiffLexer(){
		setLexer(SCLEX_DIFF, L_DIFF, LIST_NONE);
	};

	void setPropsLexer(){
		setLexer(SCLEX_PROPERTIES, L_PROPS, LIST_NONE);
	};

	void setPostscriptLexer(){
		setLexer(SCLEX_PS, L_PS, LIST_0 | LIST_1 | LIST_2 | LIST_3);
	};

	void setRubyLexer(){
		setLexer(SCLEX_RUBY, L_RUBY, LIST_0);
		f(SCI_STYLESETEOLFILLED, SCE_RB_POD, true);
	};

	void setSmalltalkLexer(){
		setLexer(SCLEX_SMALLTALK, L_SMALLTALK, LIST_0);
	};

	void setVhdlLexer(){
		setLexer(SCLEX_VHDL, L_VHDL, LIST_0 | LIST_1 | LIST_2 | LIST_3 | LIST_4 | LIST_5 | LIST_6);
	};

	void setKixLexer(){
		setLexer(SCLEX_KIX, L_KIX, LIST_0 | LIST_1 | LIST_2);
	};

	void setAutoItLexer(){
		setLexer(SCLEX_AU3, L_AU3, LIST_0 | LIST_1 | LIST_2 | LIST_3 | LIST_4 | LIST_5 | LIST_6);
		f(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.preprocessor"), reinterpret_cast<LPARAM>("1"));
	};

	void setCamlLexer(){
		setLexer(SCLEX_CAML, L_CAML, LIST_0 | LIST_1 | LIST_2);
	};

	void setAdaLexer(){
		setLexer(SCLEX_ADA, L_ADA, LIST_0);
	};

	void setVerilogLexer(){
		setLexer(SCLEX_VERILOG, L_VERILOG, LIST_0 | LIST_1);
		f(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.preprocessor"), reinterpret_cast<LPARAM>("1"));
	};

	void setMatlabLexer(){
		setLexer(SCLEX_MATLAB, L_MATLAB, LIST_0);
	};

	void setHaskellLexer(){
		setLexer(SCLEX_HASKELL, L_HASKELL, LIST_0);
	};

	void setInnoLexer() {
		setLexer(SCLEX_INNOSETUP, L_INNO, LIST_0 | LIST_1 | LIST_2 | LIST_3 | LIST_4 | LIST_5);
	};

	void setCmakeLexer() {
		setLexer(SCLEX_CMAKE, L_CMAKE, LIST_0 | LIST_1 | LIST_2);
	};

	void setYamlLexer() {
		setLexer(SCLEX_YAML, L_YAML, LIST_0);
	};

	//--------------------

	void setCobolLexer() {
		setLexer(SCLEX_COBOL, L_COBOL, LIST_0 | LIST_1 | LIST_2);
	};
	void setGui4CliLexer() {
		setLexer(SCLEX_GUI4CLI, L_GUI4CLI, LIST_0 | LIST_1 | LIST_2 | LIST_3 | LIST_4);
	};
	void setDLexer() {
		setLexer(SCLEX_D, L_D, LIST_0 | LIST_1 | LIST_2 | LIST_3 | LIST_4 | LIST_5 | LIST_6);
	};
	void setPowerShellLexer() {
		setLexer(SCLEX_POWERSHELL, L_POWERSHELL, LIST_0 | LIST_1 | LIST_2 | LIST_5);
	};
	void setRLexer() {
		setLexer(SCLEX_R, L_R, LIST_0 | LIST_1 | LIST_2);
	};

	void setCoffeeScriptLexer() {
		setLexer(SCLEX_COFFEESCRIPT, L_COFFEESCRIPT, LIST_0 | LIST_1 | LIST_2  | LIST_3);
	};

	void setBaanCLexer() {
		setLexer(SCLEX_BAAN, L_BAANC, LIST_0 | LIST_1 | LIST_2 | LIST_3 | LIST_4 | LIST_5 | LIST_6 | LIST_7 | LIST_8);
		f(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("lexer.baan.styling.within.preprocessor"), reinterpret_cast<LPARAM>("1"));
		f(SCI_SETWORDCHARS, 0, reinterpret_cast<LPARAM>("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_$:"));
		f(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.preprocessor"), reinterpret_cast<LPARAM>("1"));
		f(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.baan.syntax.based"), reinterpret_cast<LPARAM>("1"));
		f(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.baan.keywords.based"), reinterpret_cast<LPARAM>("1"));
		f(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.baan.sections"), reinterpret_cast<LPARAM>("1"));
		f(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.baan.inner.level"), reinterpret_cast<LPARAM>("1"));
		f(SCI_STYLESETEOLFILLED, SCE_BAAN_STRINGEOL, true);
	};

	void setSrecLexer() {
		setLexer(SCLEX_SREC, L_SREC, LIST_NONE);
	};

	void setIHexLexer() {
		setLexer(SCLEX_IHEX, L_IHEX, LIST_NONE);
	};

	void setTEHexLexer() {
		setLexer(SCLEX_TEHEX, L_TEHEX, LIST_NONE);
	};

	void setAsn1Lexer() {
		setLexer(SCLEX_ASN1, L_ASN1, LIST_0 | LIST_1 | LIST_2 | LIST_3); 
	};

	void setAVSLexer() {
		setLexer(SCLEX_AVS, L_AVS, LIST_0 | LIST_1 | LIST_2 | LIST_3 | LIST_4 | LIST_5);
		f(SCI_SETWORDCHARS, 0, reinterpret_cast<LPARAM>("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_#"));
	};

	void setBlitzBasicLexer() {
		setLexer(SCLEX_BLITZBASIC, L_BLITZBASIC, LIST_0 | LIST_1 | LIST_2 | LIST_3); 
	};

	void setPureBasicLexer() {
		setLexer(SCLEX_PUREBASIC, L_PUREBASIC, LIST_0 | LIST_1 | LIST_2 | LIST_3); 
	};

	void setFreeBasicLexer() {
		setLexer(SCLEX_FREEBASIC, L_FREEBASIC, LIST_0 | LIST_1 | LIST_2 | LIST_3); 
	};

	void setCsoundLexer() {
		setLexer(SCLEX_CSOUND, L_CSOUND, LIST_0 | LIST_1 | LIST_2);
		f(SCI_STYLESETEOLFILLED, SCE_CSOUND_STRINGEOL, true);
	};

	void setErlangLexer() {
		setLexer(SCLEX_ERLANG, L_ERLANG, LIST_0 | LIST_1 | LIST_2 | LIST_3 | LIST_4 | LIST_5); 
	};

	void setESCRIPTLexer() {
		setLexer(SCLEX_ESCRIPT, L_ESCRIPT, LIST_0 | LIST_1 | LIST_2); 
	};

	void setForthLexer() {
		setLexer(SCLEX_FORTH, L_FORTH, LIST_0 | LIST_1 | LIST_2 | LIST_3 | LIST_4 | LIST_5);
		f(SCI_SETWORDCHARS, 0, reinterpret_cast<LPARAM>("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789%-"));
	};

	void setLatexLexer() {
		setLexer(SCLEX_LATEX, L_LATEX, LIST_NONE); 
	};

	void setMMIXALLexer() {
		setLexer(SCLEX_MMIXAL, L_MMIXAL, LIST_0 | LIST_1 | LIST_2); 
	};

	void setNimrodLexer() {
		setLexer(SCLEX_NIMROD, L_NIMROD, LIST_0);
	};

	void setNncrontabLexer() {
		setLexer(SCLEX_NNCRONTAB, L_NNCRONTAB, LIST_0 | LIST_1 | LIST_2); 
		f(SCI_SETWORDCHARS, 0, reinterpret_cast<LPARAM>("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789%-"));
	};

	void setOScriptLexer() {
		setLexer(SCLEX_OSCRIPT, L_OSCRIPT, LIST_0 | LIST_1 | LIST_2 | LIST_3 | LIST_4 | LIST_5);
		f(SCI_SETWORDCHARS, 0, reinterpret_cast<LPARAM>("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_$"));
	};

	void setREBOLLexer() {
		setLexer(SCLEX_REBOL, L_REBOL, LIST_0 | LIST_1 | LIST_2 | LIST_3 | LIST_4 | LIST_5 | LIST_6);
		f(SCI_SETWORDCHARS, 0, reinterpret_cast<LPARAM>("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789?!.’+-*&|=_~"));
	};

	void setRegistryLexer() {
		setLexer(SCLEX_REGISTRY, L_REGISTRY, LIST_NONE); 
	};

	void setRustLexer() {
		setLexer(SCLEX_RUST, L_RUST, LIST_0 | LIST_1 | LIST_2 | LIST_3 | LIST_4 | LIST_5 | LIST_6); 
		f(SCI_SETWORDCHARS, 0, reinterpret_cast<LPARAM>("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_#"));
	};

	void setSpiceLexer() {
		setLexer(SCLEX_SPICE, L_SPICE, LIST_0 | LIST_1 | LIST_2); 
	};

	void setTxt2tagsLexer() {
		setLexer(SCLEX_TXT2TAGS, L_TXT2TAGS, LIST_NONE); 
	};

	void setVisualPrologLexer() {
		setLexer(SCLEX_VISUALPROLOG, L_VISUALPROLOG, LIST_0 | LIST_1 | LIST_2 | LIST_3);
	}

	//--------------------

	void setSearchResultLexer() {
		f(SCI_STYLESETEOLFILLED, SCE_SEARCHRESULT_FILE_HEADER, true);
		f(SCI_STYLESETEOLFILLED, SCE_SEARCHRESULT_SEARCH_HEADER, true);
		setLexer(SCLEX_SEARCHRESULT, L_SEARCHRESULT, 0);
	};

	bool isNeededFolderMarge(LangType typeDoc) const {
		switch (typeDoc)	{

			case L_ASCII:
			case L_BATCH:
			case L_TEXT:
			case L_MAKEFILE:
			case L_ASM:
			case L_HASKELL:
			case L_PROPS:
			case L_SMALLTALK:
			case L_KIX:
			case L_ADA:
				return false;
			default:
				return true;
		}
	};
//END: Lexers and Styling

	void defineMarker(int marker, int markerType, COLORREF fore, COLORREF back, COLORREF foreActive) {
		f(SCI_MARKERDEFINE, marker, markerType);
		f(SCI_MARKERSETFORE, marker, fore);
		f(SCI_MARKERSETBACK, marker, back);
		f(SCI_MARKERSETBACKSELECTED, marker, foreActive);
	};

	int codepage2CharSet() const {
		switch (_codepage)	{
			case CP_CHINESE_TRADITIONAL : return SC_CHARSET_CHINESEBIG5;
			case CP_CHINESE_SIMPLIFIED : return SC_CHARSET_GB2312;
			case CP_KOREAN : return SC_CHARSET_HANGUL;
			case CP_JAPANESE : return SC_CHARSET_SHIFTJIS;
			case CP_GREEK : return SC_CHARSET_GREEK;
			default : return 0;
		}
	};
	std::pair<int, int> getWordRange();
	bool expandWordSelection();
	void getFoldColor(COLORREF& fgColor, COLORREF& bgColor, COLORREF& activeFgColor);
};

void ScintillaEditView::fold(size_t line, bool mode)	{
	if (f(SCI_GETENDSTYLED) < f(SCI_GETLENGTH))
		f(SCI_COLOURISE,0,-1);
	int headerLine;
	if (f(SCI_GETFOLDLEVEL, line) & SC_FOLDLEVELHEADERFLAG)
		headerLine = static_cast<int32_t>(line);
	else if ((headerLine = static_cast<int32_t>(f(SCI_GETFOLDPARENT, line))) == -1)
		return;

	if (bool(f(SCI_GETFOLDEXPANDED, headerLine)) != mode)	{
		f(SCI_TOGGLEFOLD, headerLine);

		SCNotification scnN;
		scnN.nmhdr.code = SCN_FOLDINGSTATECHANGED;
		scnN.nmhdr.hwndFrom = _hSelf;
		scnN.nmhdr.idFrom = 0;
		scnN.line = headerLine;
		scnN.foldLevelNow = bool(f(SCI_GETFOLDEXPANDED, headerLine));

		::SendMessage(_hParent, WM_NOTIFY, 0, reinterpret_cast<LPARAM>(&scnN));
	}
}

void ScintillaEditView::toggleFold(size_t ln, bool rec)	{

	if (f(SCI_GETENDSTYLED) < f(SCI_GETLENGTH))
		f(SCI_COLOURISE,0,-1);
	int hLine;
	if (f(SCI_GETFOLDLEVEL, ln) & SC_FOLDLEVELHEADERFLAG)
		hLine = static_cast<int>(ln);
	else if ((hLine = static_cast<int>(f(SCI_GETFOLDPARENT, ln))) == -1)
		return;

	SCNotification scnN;
	scnN.nmhdr.code = SCN_FOLDINGSTATECHANGED;
	scnN.nmhdr.hwndFrom = _hSelf;
	scnN.nmhdr.idFrom = 0;

	f(SCI_TOGGLEFOLD, hLine);
	scnN.line = hLine;
	bool hdState = scnN.foldLevelNow = bool(f(SCI_GETFOLDEXPANDED, hLine));
	::SendMessage(_hParent, WM_NOTIFY, 0, reinterpret_cast<LPARAM>(&scnN));

	if (rec)	{
		auto hdLevel = f(SCI_GETFOLDLEVEL, hLine) & SC_FOLDLEVELNUMBERMASK;
		while ((f(SCI_GETFOLDLEVEL, ++hLine) & SC_FOLDLEVELNUMBERMASK) > hdLevel)
			if (f(SCI_GETFOLDLEVEL, hLine) & SC_FOLDLEVELHEADERFLAG && bool(f(SCI_GETFOLDEXPANDED, hLine)) != hdState)	{
				f(SCI_TOGGLEFOLD, hLine);
				scnN.line = hLine;
				scnN.foldLevelNow = bool(f(SCI_GETFOLDEXPANDED, hLine));
				::SendMessage(_hParent, WM_NOTIFY, 0, reinterpret_cast<LPARAM>(&scnN));
			}
	}
	f(SCI_SETYCARETPOLICY, 14, 0);f(SCI_SCROLLCARET);
	f(SCI_SETYCARETPOLICY, 13, 1);
}

void ScintillaEditView::foldAllToggle()	{
	bool isNode;
	int ln, n; ln = n = static_cast<int>(f(SCI_LINEFROMPOSITION,f(SCI_GETCURRENTPOS)));
	while (!(isNode=f(SCI_GETFOLDLEVEL, ln)&SC_FOLDLEVELHEADERFLAG) && 0<ln)
		--ln;
	if (isNode)
		foldAll(!bool(f(SCI_GETFOLDEXPANDED, ln)));
	else	{
		while (!(isNode=f(SCI_GETFOLDLEVEL, ++n)&SC_FOLDLEVELHEADERFLAG) && n<f(SCI_GETLINECOUNT)-1);
		if (isNode)	{
			foldAll(!bool(f(SCI_GETFOLDEXPANDED, n)));
			f(SCI_GOTOLINE,n);
		}
	}
	f(SCI_SETYCARETPOLICY, 14, 0);f(SCI_SCROLLCARET);f(SCI_SETYCARETPOLICY, 13, 1);
}