﻿// This file is part of Notepad++ project
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
#include <shlobj.h>
#include <uxtheme.h>
#include "FindReplaceDlg.h"
#include "ScintillaEditView.h"
#include "Notepad_plus_msgs.h"
#include "UniConversion.h"
#include "localization.h"
#include "Notepad_plus.h"

#define SHIFTED 0x8000
using namespace std;

// (f(SCI_GETFOLDLEVEL, line) & SC_FOLDLEVELHEADERFLAG && ((?:const|constexpr|virtual|void|volatile)auto|bool|case|char|char16_t|char32_t|class|double|enum|float|inline|int|long|mutable|short|struct|union|unsigned|wchar_t))	;

FindOption *FindReplaceDlg::_env;
FindOption FindReplaceDlg::_options;

void addText2Combo(const TCHAR * txt2add, HWND hCombo)	{

	if (!hCombo || !lstrcmp(txt2add, L"")) return;

	auto i = ::SendMessage(hCombo, CB_FINDSTRINGEXACT, static_cast<WPARAM>(-1), reinterpret_cast<LPARAM>(txt2add));
	if (i != CB_ERR)	{ // found

		::SendMessage(hCombo, CB_DELETESTRING, i, 0);
	}

	i = ::SendMessage(hCombo, CB_INSERTSTRING, 0, reinterpret_cast<LPARAM>(txt2add));
	::SendMessage(hCombo, CB_SETCURSEL, i, 0);
};

inline generic_string getTextFromCombo(HWND hCombo)	{

	TCHAR str[FINDREPLACE_MAXLENGTH];
	::SendMessage(hCombo, WM_GETTEXT, FINDREPLACE_MAXLENGTH - 1, reinterpret_cast<LPARAM>(str));
	return generic_string(str);
};

int Searching::convertExtendedToString(const TCHAR * query, TCHAR * result, int length)	{ 
	//query may equal to result, since it always gets smaller
	int i = 0, j = 0, charLeft = length;
	TCHAR current;
	while (i < length)	{
	//because the backslash escape quences always reduce the size of the generic_string, no overflow checks have to be made for target, assuming parameters are correct
		current = query[i];
		--charLeft;
		if (charLeft && current == '\\')	{
	//possible escape sequence
			++i;
			--charLeft;
			current = query[i];
			switch(current)	{

				case 'r':
					result[j] = '\r';
					break;
				case 'n':
					result[j] = '\n';
					break;
				case '0':
					result[j] = '\0';
					break;
				case 't':
					result[j] = '\t';
					break;
				case '\\':
					result[j] = '\\';
					break;
				case 'b':
				case 'd':
				case 'o':
				case 'x':
				case 'u': 
				{
					int size = 0, base = 0;
					if (current == 'b')	{
	//11111111
						size = 8, base = 2;
					}
					else if (current == 'o')	{
	//377
						size = 3, base = 8;
					}
					else if (current == 'd')	{
	//255
						size = 3, base = 10;
					}
					else if (current == 'x')	{
	//0xFF
						size = 2, base = 16;
					}
					else if (current == 'u')	{
	//0xCDCD
						size = 4, base = 16;
					}
					
					if (charLeft >= size)	{ 

						int res = 0;
						if (Searching::readBase(query+(i+1), &res, base, size))	{

							result[j] = static_cast<TCHAR>(res);
							i += size;
							break;
						}
					}
					//not enough chars to make parameter, use default method as fallback
				}
				
				default: 
				{	//unknown sequence, treat as regular text
					result[j] = '\\';
					++j;
					result[j] = current;
					break;
				}
			}
		}
		else	{

			result[j] = query[i];
		}
		++i;
		++j;
	}
	result[j] = 0;
	return j;
}

bool Searching::readBase(const TCHAR * str, int * value, int base, int size)	{

	int i = 0, temp = 0;
	*value = 0;
	TCHAR max = '0' + static_cast<TCHAR>(base) - 1;
	TCHAR current;
	while (i < size)	{

		current = str[i];
		if (current >= 'A')	{ 

			current &= 0xdf;
			current -= ('A' - '0' - 10);
		}
		else if (current > '9')
			return false;

		if (current >= '0' && current <= max)	{

			temp *= base;
			temp += (current - '0');
		}
		else	{

			return false;
		}
		++i;
	}
	*value = temp;
	return true;
}

LONG_PTR FindReplaceDlg::originalFinderProc = NULL;

// important : to activate all styles
const int STYLING_MASK = 255;

FindReplaceDlg::~FindReplaceDlg()
{
	_tab.destroy();
	delete _pFinder;
	for (int n = static_cast<int32_t>(_findersOfFinder.size()) - 1; n >= 0; --n)	{

		delete _findersOfFinder[n];
		_findersOfFinder.erase(_findersOfFinder.begin() + n);
	}

	if (_shiftTrickUpTip)
		::DestroyWindow(_shiftTrickUpTip);

	if (_2ButtonsTip)
		::DestroyWindow(_2ButtonsTip);

	if (_filterTip)
		::DestroyWindow(_filterTip);

	if (_hMonospaceFont)
		::DeleteObject(_hMonospaceFont);

	delete[] _uniFileName;
}

void FindReplaceDlg::create(int dialogID, bool isRTL, bool msgDestParent)	{

	StaticDialog::create(dialogID, isRTL, msgDestParent);
	fillFindHistory();
	_currentStatus = REPLACE_DLG;
	initOptionsFromDlg();
	
	_statusBar.init(GetModuleHandle(NULL), _hSelf, 0);
	_statusBar.display();

	RECT rect;
	//::GetWindowRect(_hSelf, &rect);
	getClientRect(rect);
	_tab.init(_hInst, _hSelf, false, true);
	int tabDpiDynamicalHeight = param._dpiManager.scaleY(13);
	_tab.setFont(L"Tahoma", tabDpiDynamicalHeight);
	
	// const TCHAR *find = L"Find";
	const TCHAR *replace = L"FIND/REPLACE on RAM";
	const TCHAR *findInFiles = L"FIND/REPLACE in FILES";
	const TCHAR *mark = L"MARK";

	//_tab.insertAtEnd(find);
	_tab.insertAtEnd(replace);
	_tab.insertAtEnd(findInFiles);
	_tab.insertAtEnd(mark);

	_tab.reSizeTo(rect);
	_tab.display();

	_initialClientWidth = rect.right - rect.left;
	
	//fill min dialog size info
	this->getWindowRect(_initialWindowRect);
	_initialWindowRect.right = _initialWindowRect.right - _initialWindowRect.left;
	_initialWindowRect.left = 0;
	_initialWindowRect.bottom = _initialWindowRect.bottom - _initialWindowRect.top;
	_initialWindowRect.top = 0;	

	ETDTProc enableDlgTheme = (ETDTProc)::SendMessage(_hParent, NPPM_GETENABLETHEMETEXTUREFUNC, 0, 0);
	if (enableDlgTheme)
		enableDlgTheme(_hSelf, ETDT_ENABLETAB);

	goToCenter();
}

void FindReplaceDlg::fillFindHistory()	{

	FindHistory & findHistory = param.getFindHistory();

	fillComboHistory(IDFINDWHAT, findHistory._findHistoryFinds);
	fillComboHistory(IDREPLACEWITH, findHistory._findHistoryReplaces);
	fillComboHistory(IDD_FINDINFILES_FILTERS_COMBO, findHistory._findHistoryFilters);
	fillComboHistory(IDD_FINDINFILES_DIR_COMBO, findHistory._findHistoryPaths);

	::SendDlgItemMessage(_hSelf, IDWRAP, BM_SETCHECK, findHistory._isWrap, 0);
	::SendDlgItemMessage(_hSelf, IDWHOLEWORD, BM_SETCHECK, findHistory._isMatchWord, 0);
	::SendDlgItemMessage(_hSelf, IDMATCHCASE, BM_SETCHECK, findHistory._isMatchCase, 0);
	::SendDlgItemMessage(_hSelf, IDC_BACKWARDDIRECTION, BM_SETCHECK, !findHistory._isDirectionDown, 0);

	::SendDlgItemMessage(_hSelf, IDD_FINDINFILES_INHIDDENDIR_CHECK, BM_SETCHECK, findHistory._isFifInHiddenFolder, 0);
	::SendDlgItemMessage(_hSelf, IDD_FINDINFILES_RECURSIVE_CHECK, BM_SETCHECK, findHistory._isFifRecuisive, 0);
	::SendDlgItemMessage(_hSelf, IDD_FINDINFILES_FOLDERFOLLOWSDOC_CHECK, BM_SETCHECK, findHistory._isFolderFollowDoc, 0);

	::SendDlgItemMessage(_hSelf, IDNORMAL, BM_SETCHECK, findHistory._searchMode == FindHistory::normal, 0);
	::SendDlgItemMessage(_hSelf, IDEXTENDED, BM_SETCHECK, findHistory._searchMode == FindHistory::extended, 0);
	::SendDlgItemMessage(_hSelf, IDREGEXP, BM_SETCHECK, findHistory._searchMode == FindHistory::regExpr, 0);
	::SendDlgItemMessage(_hSelf, IDREDOTMATCHNL, BM_SETCHECK, findHistory._dotMatchesNewline, 0);

	::SendDlgItemMessage(_hSelf, IDC_2_BUTTONS_MODE, BM_SETCHECK, findHistory._isSearch2ButtonsMode, 0);

	if (findHistory._searchMode == FindHistory::regExpr)	{

		//regex doesn't allow wholeword
		::SendDlgItemMessage(_hSelf, IDWHOLEWORD, BM_SETCHECK, BST_UNCHECKED, 0);
		::EnableWindow(::GetDlgItem(_hSelf, IDWHOLEWORD), (BOOL)false);

		//regex upward search is disable in v6.3 due to a regression
		//::EnableWindow(::GetDlgItem(_hSelf, IDC_FINDPREV), (BOOL)false);
		
		// If the search mode from history is regExp then enable the checkbox (. matches newline)
		::EnableWindow(GetDlgItem(_hSelf, IDREDOTMATCHNL), true);
	}
	
	if (param.isTransparentAvailable())	{

		::ShowWindow(::GetDlgItem(_hSelf, IDC_TRANSPARENT_CHECK), SW_SHOW);
		::ShowWindow(::GetDlgItem(_hSelf, IDC_TRANSPARENT_GRPBOX), SW_SHOW);
		::ShowWindow(::GetDlgItem(_hSelf, IDC_TRANSPARENT_LOSSFOCUS_RADIO), SW_SHOW);
		::ShowWindow(::GetDlgItem(_hSelf, IDC_TRANSPARENT_ALWAYS_RADIO), SW_SHOW);
		::ShowWindow(::GetDlgItem(_hSelf, IDC_PERCENTAGE_SLIDER), SW_SHOW);
		
		::SendDlgItemMessage(_hSelf, IDC_PERCENTAGE_SLIDER, TBM_SETRANGE, FALSE, MAKELONG(20, 200));
		::SendDlgItemMessage(_hSelf, IDC_PERCENTAGE_SLIDER, TBM_SETPOS, TRUE, findHistory._transparency);
		
		if (findHistory._transparencyMode == FindHistory::none)	{

			::EnableWindow(::GetDlgItem(_hSelf, IDC_TRANSPARENT_GRPBOX), FALSE);
			::EnableWindow(::GetDlgItem(_hSelf, IDC_TRANSPARENT_LOSSFOCUS_RADIO), FALSE);
			::EnableWindow(::GetDlgItem(_hSelf, IDC_TRANSPARENT_ALWAYS_RADIO), FALSE);
			::EnableWindow(::GetDlgItem(_hSelf, IDC_PERCENTAGE_SLIDER), FALSE);
		}
		else	{

			::SendDlgItemMessage(_hSelf, IDC_TRANSPARENT_CHECK, BM_SETCHECK, TRUE, 0);
			
			int id;
			if (findHistory._transparencyMode == FindHistory::onLossingFocus)	{

				id = IDC_TRANSPARENT_LOSSFOCUS_RADIO;
			}
			else	{

				id = IDC_TRANSPARENT_ALWAYS_RADIO;
				param.SetTransparent(_hSelf, findHistory._transparency);

			}
			::SendDlgItemMessage(_hSelf, id, BM_SETCHECK, TRUE, 0);
		}
	}
}

void FindReplaceDlg::fillComboHistory(int id, const vector<generic_string> & strings)	{

	HWND hCombo = ::GetDlgItem(_hSelf, id);

	for (vector<generic_string>::const_reverse_iterator i = strings.rbegin() ; i != strings.rend(); ++i)	{

		addText2Combo(i->c_str(), hCombo);
	}

	//empty string is not added to CB items, so we need to set it manually
	if (!strings.empty() && strings.begin()->empty())	{

		SetWindowText(hCombo, _T(""));
		return;
	}

	::SendMessage(hCombo, CB_SETCURSEL, 0, 0); // select first item
}


void FindReplaceDlg::saveFindHistory()	{

	if (! isCreated()) return;
	FindHistory& findHistory = param.getFindHistory();

	saveComboHistory(IDD_FINDINFILES_DIR_COMBO, findHistory._nbMaxFindHistoryPath, findHistory._findHistoryPaths, false);
	saveComboHistory(IDD_FINDINFILES_FILTERS_COMBO, findHistory._nbMaxFindHistoryFilter, findHistory._findHistoryFilters, true);
	saveComboHistory(IDFINDWHAT,                    findHistory._nbMaxFindHistoryFind, findHistory._findHistoryFinds, false);
	saveComboHistory(IDREPLACEWITH,                 findHistory._nbMaxFindHistoryReplace, findHistory._findHistoryReplaces, true);
}

int FindReplaceDlg::saveComboHistory(int id, int maxcount, vector<generic_string> & strings, bool saveEmpty)	{

	TCHAR text[FINDREPLACE_MAXLENGTH];
	HWND hCombo = ::GetDlgItem(_hSelf, id);
	int count = static_cast<int32_t>(::SendMessage(hCombo, CB_GETCOUNT, 0, 0));
	count = min(count, maxcount);

	if (count == CB_ERR) return 0;

	if (count)
		strings.clear();

	if (saveEmpty)	{

		if (!::GetWindowTextLength(hCombo))	{

			strings.push_back(generic_string());
		}
	}

	for (int i = 0 ; i < count ; ++i)	{

		auto cbTextLen = ::SendMessage(hCombo, CB_GETLBTEXTLEN, i, 0);
		if (cbTextLen <= FINDREPLACE_MAXLENGTH - 1)	{

			::SendMessage(hCombo, CB_GETLBTEXT, i, reinterpret_cast<LPARAM>(text));
			strings.push_back(generic_string(text));
		}
	}
	return count;
}

void FindReplaceDlg::updateCombos(){
	updateCombo(IDREPLACEWITH);
	updateCombo(IDFINDWHAT);
}

void FindReplaceDlg::updateCombo(int comboID){

	HWND hCombo = ::GetDlgItem(_hSelf, comboID);
	addText2Combo(getTextFromCombo(hCombo).c_str(), hCombo);
}

void FindReplaceDlg::clearAllFinder()	{
		if (_pFinder)
			_pFinder->removeAll();
		::SendMessage(_hParent, NPPM_DMMHIDE, 0, reinterpret_cast<LPARAM>(_pFinder->getHSelf()));
		(*_ppEditView)->focus();
}

FoundInfo Finder::EmptyFoundInfo(0, 0, 0, L"");
SearchResultMarking Finder::EmptySearchResultMarking;

bool Finder::notify(SCNotification *notification)	{

	static bool isDoubleClicked = false;

	switch (notification->nmhdr.code)	{

		case SCN_MARGINCLICK:
			if (notification->margin == ScintillaEditView::_SC_MARGE_FOLDER)
				_scintView.marginClick(notification->position, notification->modifiers);
			break;

		case SCN_PAINTED :
			if (isDoubleClicked)	{
				(*_ppEditView)->focus();
				isDoubleClicked = false;
			}
			break;

		case SCN_DOUBLECLICK:	{
			// remove selection from the finder
			isDoubleClicked = true;
			size_t pos = notification->position;
			if (pos == INVALID_POSITION)
				pos = static_cast<int32_t>(_scintView.f(SCI_GETLINEENDPOSITION, notification->line));
			_scintView.f(SCI_SETSEL, pos, pos);

			gotoFoundLine();
		}
		break;
	}
	return false;
}

void Finder::gotoFoundLine(){
	auto lno = _scintView.f(SCI_LINEFROMPOSITION, _scintView.f(SCI_GETCURRENTPOS));
	auto start = _scintView.f(SCI_POSITIONFROMLINE, lno);
	auto end = _scintView.f(SCI_GETLINEENDPOSITION, lno);

	if (start+2 >= end || _scintView.f(SCI_GETFOLDLEVEL, lno) & SC_FOLDLEVELHEADERFLAG)	{
		_scintView.f(SCI_TOGGLEFOLD, lno);
		return;
	}
	
	const FoundInfo fInfo = *(_pMainFoundInfos->begin() + lno);
	// Switch to another document
	::SendMessage(::GetParent(_hParent), WM_DOOPEN, 0, reinterpret_cast<LPARAM>(fInfo._fullPath.c_str()));
	(*_ppEditView)->_positionRestoreNeeded = false;

	(*_ppEditView)->putMvmntInView(fInfo._start, fInfo._end);

/*// Then we colourise the double clicked line
 	setFinderStyle();
	_scintView.f(SCI_STYLESETEOLFILLED, SCE_SEARCHRESULT_HIGHLIGHT_LINE, true);
	_scintView.f(SCI_STARTSTYLING, start, STYLING_MASK);
	_scintView.f(SCI_SETSTYLING, end - start + 2, SCE_SEARCHRESULT_HIGHLIGHT_LINE);
	_scintView.f(SCI_COLOURISE, start, end + 1); */
}

void Finder::deleteResult()	{

	auto currentPos = _scintView.f(SCI_GETCURRENTPOS); // yniq - add handling deletion of multiple lines?

	auto lno = _scintView.f(SCI_LINEFROMPOSITION, currentPos);
	auto start = _scintView.f(SCI_POSITIONFROMLINE, lno);
	auto end = _scintView.f(SCI_GETLINEENDPOSITION, lno);
	if (start + 2 >= end) return; // avoid empty lines

	_scintView.setLexer(SCLEX_SEARCHRESULT, L_SEARCHRESULT, 0); // Restore searchResult lexer in case the lexer was changed to SCLEX_NULL in GotoFoundLine()

	if (_scintView.f(SCI_GETFOLDLEVEL, lno) & SC_FOLDLEVELHEADERFLAG)	{  // delete a folder

		auto endline = _scintView.f(SCI_GETLASTCHILD, lno, -1) + 1;
		assert((size_t) endline <= _pMainFoundInfos->size());

		_pMainFoundInfos->erase(_pMainFoundInfos->begin() + lno, _pMainFoundInfos->begin() + endline); // remove found info
		_pMainMarkings->erase(_pMainMarkings->begin() + lno, _pMainMarkings->begin() + endline);

		auto end2 = _scintView.f(SCI_POSITIONFROMLINE, endline);
		_scintView.f(SCI_SETSEL, start, end2);
		setFinderReadOnly(false);
		_scintView.f(SCI_CLEAR);
		setFinderReadOnly(true);
	}
	else	{ // delete one line

		assert((size_t) lno < _pMainFoundInfos->size());

		_pMainFoundInfos->erase(_pMainFoundInfos->begin() + lno); // remove found info
		_pMainMarkings->erase(_pMainMarkings->begin() + lno);

		setFinderReadOnly(false);
		_scintView.f(SCI_LINEDELETE);
		setFinderReadOnly(true);
	}
	_markingsStruct._length = static_cast<long>(_pMainMarkings->size());

	assert(_pMainFoundInfos->size() == _pMainMarkings->size());
	assert(size_t(_scintView.f(SCI_GETLINECOUNT)) == _pMainFoundInfos->size() + 1);
}

vector<generic_string> Finder::getResultFilePaths() const
{
	vector<generic_string> paths;
	for (size_t i = 0; i < _pMainFoundInfos->size(); ++i)	{

		// make sure that path is not already in
		generic_string & path2add = (*_pMainFoundInfos)[i]._fullPath;
		bool found = 0;//path2add.empty();
		for (size_t j = 0; j < paths.size() && not found; ++j)
			if (paths[j] == path2add)	{
	found = true;
				break;
			};
		if (not found)
			paths.push_back(path2add);
	}
	return paths;
}

bool Finder::canFind(const TCHAR *fileName, size_t lineNumber) const
{
	size_t len = _pMainFoundInfos->size();
	for (size_t i = 0; i < len; ++i)	{

		if ((*_pMainFoundInfos)[i]._fullPath == fileName &&
			lineNumber == (*_pMainFoundInfos)[i]._lineNumber)
				return true;

	}
	return false; 
}

void Finder::gotoNextFoundResult(int increment)	{

	auto currentPos = _scintView.f(SCI_GETCURRENTPOS);
	auto lno = _scintView.f(SCI_LINEFROMPOSITION, currentPos);
	auto total_lines = _scintView.f(SCI_GETLINECOUNT);
	if (total_lines <= 1) return;
	
	if (lno==total_lines -1) --lno; // last line doesn't belong to any search, use last search

	auto init_lno = lno;
	auto max_lno = _scintView.f(SCI_GETLASTCHILD, lno, searchHeaderLevel);

	assert(max_lno <= total_lines - 2);

	// get the line number of the current search (searchHeaderLevel)
	int level = _scintView.f(SCI_GETFOLDLEVEL, lno) & SC_FOLDLEVELNUMBERMASK;
	auto min_lno = lno;
	while (fileHeaderLevel <= level--)	{

		min_lno = _scintView.f(SCI_GETFOLDPARENT, min_lno);
		assert(min_lno >= 0);
	}

	if (min_lno < 0) min_lno = lno; // when lno is a search header line

	assert(min_lno <= max_lno);

	do	{
		lno += increment;
		lno = lno > max_lno? min_lno : lno < min_lno? max_lno : lno;
	}
	while (_scintView.f(SCI_GETFOLDLEVEL, lno) & SC_FOLDLEVELHEADERFLAG && lno != init_lno);
/* 		{

		lno += increment;
		if      (lno > max_lno) lno = min_lno;
		else if (lno < min_lno) lno = max_lno;
		if (lno == init_lno) break;
	} */

	if (!(_scintView.f(SCI_GETFOLDLEVEL, lno) & SC_FOLDLEVELHEADERFLAG))	{

		_scintView.f(SCI_SETCURRENTPOS, _scintView.f(SCI_POSITIONFROMLINE, lno));
		_scintView.f(SCI_ENSUREVISIBLE, lno);
		_scintView.f(SCI_SCROLLCARET);

		gotoFoundLine();
	}
}

void Finder::addSearchLine(const TCHAR *searchName)	{

	generic_string str = L"Search \"";
	str += searchName;
	str += L"\"  \r\n";

	setFinderReadOnly(false);
	_scintView.addGenericText(str.c_str());
	setFinderReadOnly(true);
	_lastSearchHeaderPos = static_cast<int32_t>(_scintView.f(SCI_GETCURRENTPOS) - 2);

	_pMainFoundInfos->push_back(EmptyFoundInfo);
	_pMainMarkings->push_back(EmptySearchResultMarking);
}

void Finder::addFileNameTitle(const TCHAR *fullName, const TCHAR *dir)	{

	generic_string fn, str = L" ";
	str += fullName;
	if (dir){
		fn = str.substr(lstrlen(dir));											  
		str = str.substr(0,str.find_first_of('\\')+5);
		str += L"..";
		str += fn;
	}
	str += L"\r\n";
	
	_FileHeader1stPos = static_cast<int32_t>(_scintView.f(SCI_GETCURRENTPOS));
	setFinderReadOnly(false);
	_scintView.addGenericText(str.c_str());
	setFinderReadOnly(true);

	_pMainFoundInfos->push_back(EmptyFoundInfo);
	_pMainMarkings->push_back(EmptySearchResultMarking);
}

void Finder::addFileHitCount(int count)	{

	TCHAR text[8];
	if (count>1) {
		wsprintf(text, L" %i  ", count);													
		setFinderReadOnly(false);
		_scintView.insertGenericTextFrom(_FileHeader1stPos, text);
		setFinderReadOnly(true);
	}							  
	++_nbFoundFiles;
}

void Finder::addSearchHitCount(int count, const TCHAR *dir, bool isMatchLines){
	const TCHAR *moreInfo = isMatchLines ? L" - Only the matched pattern" :L"";

	TCHAR text[512];
	if(count)
		if (dir)
			if(_nbFoundFiles >1)
				wsprintf(text, L": %i in %i %s%s", count, _nbFoundFiles, (L"files under "+generic_string(dir)).c_str(), moreInfo);
			else
				wsprintf(text, L": %i in file below at %s%s", count, dir, moreInfo);
		else
			if(_nbFoundFiles >1)
				wsprintf(text, L": %i in %i files of %i opened files%s", count, _nbFoundFiles, _nbOpenedFiles, moreInfo);
			else
				if (_findAllInCurrent)
					wsprintf(text, L": %i in the current file%s", count, moreInfo);
				else
					wsprintf(text, L": %i in below opened file out of %i opened files%s", count, _nbOpenedFiles, moreInfo);
	else
		if (dir)
			wsprintf(text, L"was not found under %s", dir);
		else if(_nbOpenedFiles)		wsprintf(text, L" wasn't found in any of %i opened files", _nbOpenedFiles);
		else		wsprintf(text, L"was not found in current file");
	
	setFinderReadOnly(false);
	_scintView.insertGenericTextFrom(_lastSearchHeaderPos, text);
	setFinderReadOnly(true);
}


void Finder::add(FoundInfo fi, SearchResultMarking mi, const TCHAR* foundline)	{

	_pMainFoundInfos->push_back(fi);

	TCHAR lnb[16];
	wsprintf(lnb, L"%d: ", fi._lineNumber);
	generic_string str = L"L    ";
	str += lnb;
	mi._start += static_cast<int32_t>(str.length());
	mi._end += static_cast<int32_t>(str.length());
	str += foundline;

	if (str.length() >= SC_SEARCHRESULT_LINEBUFFERMAXLENGTH)	{

		const TCHAR * endOfLongLine = L"...\r\n";
		str = str.substr(0, SC_SEARCHRESULT_LINEBUFFERMAXLENGTH - lstrlen(endOfLongLine) - 1);
		str += endOfLongLine;
	}
	setFinderReadOnly(false);
	_scintView.addGenericText(str.c_str(), &mi._start, &mi._end);
	setFinderReadOnly(true);
	_pMainMarkings->push_back(mi);
}

void Finder::removeAll()	{ 

	_pMainFoundInfos->clear();
	_pMainMarkings->clear();
	setFinderReadOnly(false);
	_scintView.f(SCI_CLEARALL);
	setFinderReadOnly(true);
}

void Finder::openAll()	{

	size_t sz = _pMainFoundInfos->size();

	for (size_t i = 0; i < sz; ++i)	{

		::SendMessage(::GetParent(_hParent), WM_DOOPEN, 0, reinterpret_cast<LPARAM>(_pMainFoundInfos->at(i)._fullPath.c_str()));
	}
}

bool Finder::isLineActualSearchResult(const generic_string & s) const{
	return (!s.find(L"Line "));
}

generic_string& Finder::prepareStringForClipboard(generic_string & s) const
{
	// Input: a string like "\tLine 3: search result".
	// Output: "search result"
	s = stringReplace(s, L"\r", L"");
	s = stringReplace(s, L"\n", L"");
	const auto firstColon = s.find(L':');
	if (firstColon == std::string::npos)	{

		// Should never happen.
		assert(false);
		return s;
	}
	else	{

		// Plus 2 in order to deal with ": ".
		s = s.substr(2 + firstColon);
		return s;
	}
}

void Finder::copy()	{

	size_t fromLine, toLine;
	{
		const auto selStart = _scintView.f(SCI_GETSELECTIONSTART);
		const auto selEnd = _scintView.f(SCI_GETSELECTIONEND);
		const bool hasSelection = selStart != selEnd;
		const pair<int, int> lineRange = _scintView.getSelectionLinesRange();
		if (hasSelection && lineRange.first != lineRange.second)	{

			fromLine = lineRange.first;
			toLine = lineRange.second;
		}
		else	{

			// Abuse fold levels to find out which lines to copy to clipboard.
			// We get the current line and then the next line which has a smaller fold level (SCI_GETLASTCHILD).
			// Then we loop all lines between them and determine which actually contain search results.
			fromLine = _scintView.getCurrentLineNumber();
			const int selectedLineFoldLevel = _scintView.f(SCI_GETFOLDLEVEL, fromLine) & SC_FOLDLEVELNUMBERMASK;
			toLine = _scintView.f(SCI_GETLASTCHILD, fromLine, selectedLineFoldLevel);
		}
	}

	std::vector<generic_string> lines;
	for (size_t line = fromLine; line <= toLine; ++line)	{

		generic_string lineStr = _scintView.getLine(line);
		if (isLineActualSearchResult(lineStr))	{

			lines.push_back(prepareStringForClipboard(lineStr));
		}
	}
	const generic_string toClipboard = stringJoin(lines, L"\r\n");
	if (!toClipboard.empty())	{

		if (!str2Clipboard(toClipboard, _hSelf))	{

			assert(false);
			::MessageBox(NULL, L"Error placing text in clipboard.", L"Notepad++", MB_ICONINFORMATION);
		}
	}
}

void Finder::beginNewFilesSearch()	{
	//_scintView.f(SCI_SETLEXER, SCLEX_NULL);
	_scintView.f(SCI_SETCURRENTPOS, 0);
	_pMainFoundInfos = _pMainFoundInfos == &_foundInfos1 ? &_foundInfos2 : &_foundInfos1;
	_pMainMarkings = _pMainMarkings == &_markings1 ? &_markings2 : &_markings1;
	_nbFoundFiles = 0;
	// fold all old searches (1st level only)
	_scintView.collapse(searchHeaderLevel - SC_FOLDLEVELBASE, fold_collapse);
}

void Finder::finishFilesSearch(int count, bool isfold,const TCHAR *dir, bool isMatchLines)	{

	std::vector<FoundInfo>* _pOldFoundInfos;
	std::vector<SearchResultMarking>* _pOldMarkings;
	_pOldFoundInfos = _pMainFoundInfos == &_foundInfos1 ? &_foundInfos2 : &_foundInfos1;
	_pOldMarkings = _pMainMarkings == &_markings1 ? &_markings2 : &_markings1;
	
	_pOldFoundInfos->insert(_pOldFoundInfos->begin(), _pMainFoundInfos->begin(), _pMainFoundInfos->end());
	_pOldMarkings->insert(_pOldMarkings->begin(), _pMainMarkings->begin(), _pMainMarkings->end());
	_pMainFoundInfos->clear();
	_pMainMarkings->clear();
	_pMainFoundInfos = _pOldFoundInfos;
	_pMainMarkings = _pOldMarkings;

	if ((_markingsStruct._length = static_cast<long>(_pMainMarkings->size())) > 0)
		_markingsStruct._markings = &(*_pMainMarkings)[0];

	addSearchHitCount(count, dir, isMatchLines);

	_scintView.f(SCI_SETLEXER, SCLEX_SEARCHRESULT);
	_scintView.f(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold"), reinterpret_cast<LPARAM>( isfold? "1" : "0"));

	auto o=_scintView.f(SCI_POSITIONFROMLINE, 2) +(*_pMainMarkings)[2]._start;
	_scintView.f(SCI_GOTOPOS, o);
	_scintView.f(SCI_SETANCHOR, _scintView.f(SCI_POSITIONFROMLINE, 2) +(*_pMainMarkings)[2]._end);

	_scintView.f(SCI_SETWRAPMODE, 2);
	_scintView.f(SCI_SETWRAPVISUALFLAGS,SC_WRAPVISUALFLAG_START);
	_scintView.f(SCI_SETWRAPSTARTINDENT,5);
	_scintView.f(SCI_SETWRAPINDENTMODE,SC_WRAPINDENT_FIXED);
	_scintView.f(SCI_SCROLLRANGE, 0, o);
}

void Finder::setFinderStyle()	{

	// Set global styles for the finder
	_scintView.performGlobalStyles();
	
	// Set current line background color for the finder
	const TCHAR * lexerName = ScintillaEditView::langNames[L_SEARCHRESULT].lexerName;
	LexerStyler *pStyler = param.getLStylerArray().getLexerStylerByName(lexerName);
	if (pStyler)	{

		int i = pStyler->getStylerIndexByID(SCE_SEARCHRESULT_CURRENT_LINE);
		if (i != -1)	{

			Style & style = pStyler->getStyler(i);
			_scintView.f(SCI_SETCARETLINEBACK, style._bgColor);
		}
	}
	_scintView.setSearchResultLexer();
	
	// Override foreground & background colour by default foreground & background coulour
	StyleArray & stylers = param.getMiscStylerArray();
	int iStyleDefault = stylers.getStylerIndexByID(STYLE_DEFAULT);
	if (iStyleDefault != -1)	{

		Style & styleDefault = stylers.getStyler(iStyleDefault);
		_scintView.setStyle(styleDefault);

		GlobalOverride & go = param.getGlobalOverrideStyle();
		if (go.isEnable())	{

			int iGlobalOverride = stylers.getStylerIndexByName(L"Global override");
			if (iGlobalOverride != -1)	{

				Style & styleGlobalOverride = stylers.getStyler(iGlobalOverride);
				if (go.enableFg)	{

					styleDefault._fgColor = styleGlobalOverride._fgColor;
				}
				if (go.enableBg)	{

					styleDefault._bgColor = styleGlobalOverride._bgColor;
				}
			}
		}

		_scintView.f(SCI_STYLESETFORE, SCE_SEARCHRESULT_DEFAULT, styleDefault._fgColor);
		_scintView.f(SCI_STYLESETBACK, SCE_SEARCHRESULT_DEFAULT, styleDefault._bgColor);
	}

	_scintView.f(SCI_COLOURISE, 0, -1);

	// finder fold style follows user preference but use box when user selects none
	ScintillaViewParams& svp = (ScintillaViewParams&)param.getSVP();
	_scintView.setMakerStyle(svp._folderStyle == FOLDER_STYLE_NONE ? FOLDER_STYLE_BOX : svp._folderStyle);
}

INT_PTR CALLBACK Finder::run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam)	{

	switch (message)	{ 

		case WM_COMMAND :	{ 

			switch (wParam)	{

				case NPPM_INTERNAL_FINDINFINDERDLG:	{

					::SendMessage(::GetParent(_hParent), NPPM_INTERNAL_FINDINFINDERDLG, reinterpret_cast<WPARAM>(this), 0);
					return TRUE;
				}

				case NPPM_INTERNAL_REMOVEFINDER:	{

					if (_canBeVolatiled)	{

						::SendMessage(::GetParent(_hParent), NPPM_DMMHIDE, 0, reinterpret_cast<LPARAM>(_hSelf));
						setClosed(true);
					}
					return TRUE;
				}

				case NPPM_INTERNAL_FINFERCOLLAPSE :	{

					_scintView.foldAll(fold_collapse);
					return TRUE;
				}

				case NPPM_INTERNAL_FINFERUNCOLLAPSE :	{

					_scintView.foldAll(fold_uncollapse);
					return TRUE;
				}

				case NPPM_INTERNAL_FINFERCOPY :	{

					copy();
					return TRUE;
				}

				case NPPM_INTERNAL_FINFERSELECTALL :	{

					_scintView.f(SCI_SELECTALL);
					return TRUE;
				}

				case NPPM_INTERNAL_FINFERCLEARALL:	{

					removeAll();
					return TRUE;
				}

				case NPPM_INTERNAL_FINFEROPENALL:	{

					openAll();
					return TRUE;
				}

				default :
				{
					return FALSE;
				}
			}
		}
		
		case WM_CONTEXTMENU :	{

			if (HWND(wParam) == _scintView.getHSelf())	{

				POINT p;
				::GetCursorPos(&p);
				ContextMenu scintillaContextmenu;
				vector<MenuItemUnit> tmp;

				NativeLangSpeaker *pNativeSpeaker = param.getNativeLangSpeaker();

				generic_string findInFinder = pNativeSpeaker->getLocalizedStrFromID("finder-find-in-finder", L"Find in these found results...");
				generic_string closeThis = pNativeSpeaker->getLocalizedStrFromID("finder-close-this", L"Close this finder");
				generic_string collapseAll = pNativeSpeaker->getLocalizedStrFromID("finder-collapse-all", L"Collapse all");
				generic_string uncollapseAll = pNativeSpeaker->getLocalizedStrFromID("finder-uncollapse-all", L"Uncollapse all");
				generic_string copy = pNativeSpeaker->getLocalizedStrFromID("finder-copy", L"Copy");
				generic_string selectAll = pNativeSpeaker->getLocalizedStrFromID("finder-select-all", L"Select all");
				generic_string clearAll = pNativeSpeaker->getLocalizedStrFromID("finder-clear-all", L"Clear all");
				generic_string openAll = pNativeSpeaker->getLocalizedStrFromID("finder-open-all", L"Open all");

				tmp.push_back(MenuItemUnit(NPPM_INTERNAL_FINDINFINDERDLG, findInFinder));
				if (_canBeVolatiled)
					tmp.push_back(MenuItemUnit(NPPM_INTERNAL_REMOVEFINDER, closeThis));
				tmp.push_back(MenuItemUnit(0, L"Separator"));
				tmp.push_back(MenuItemUnit(NPPM_INTERNAL_FINFERCOLLAPSE, collapseAll));
				tmp.push_back(MenuItemUnit(NPPM_INTERNAL_FINFERUNCOLLAPSE, uncollapseAll));
				tmp.push_back(MenuItemUnit(0, L"Separator"));
				tmp.push_back(MenuItemUnit(NPPM_INTERNAL_FINFERCOPY, copy));
				tmp.push_back(MenuItemUnit(NPPM_INTERNAL_FINFERSELECTALL, selectAll));
				tmp.push_back(MenuItemUnit(NPPM_INTERNAL_FINFERCLEARALL, clearAll));
				tmp.push_back(MenuItemUnit(0, L"Separator"));
				tmp.push_back(MenuItemUnit(NPPM_INTERNAL_FINFEROPENALL, openAll));

				scintillaContextmenu.create(_hSelf, tmp);

				scintillaContextmenu.display(p);
				return TRUE;
			}
			return ::DefWindowProc(_hSelf, message, wParam, lParam);
		}

		case WM_SIZE :	{

			RECT rc;
			getClientRect(rc);
			_scintView.reSizeTo(rc);
			break;
		}

		case WM_NOTIFY:	{

			notify(reinterpret_cast<SCNotification *>(lParam));
			return FALSE;
		}
		default :
			return DockingDlgInterface::run_dlgProc(message, wParam, lParam);
	}
	return FALSE;
}

void FindIncrementDlg::init(HINSTANCE hInst, HWND hPere, FindReplaceDlg *pFRDlg, bool isRTL)	{

	Window::init(hInst, hPere);
	if (!pFRDlg)
		throw std::runtime_error("FindIncrementDlg::init : Parameter pFRDlg is null");

	_pFRDlg = pFRDlg;
	create(IDD_INCREMENT_FIND, isRTL);
	_isRTL = isRTL;
}

void FindInFinderDlg::initFromOptions()	{

	HWND hFindCombo = ::GetDlgItem(_hSelf, IDFINDWHAT_FIFOLDER);
	addText2Combo(_options._str2Search.c_str(), hFindCombo);

	::SendDlgItemMessage(_hSelf, IDC_MATCHLINENUM_CHECK_FIFOLDER, BM_SETCHECK, _options._isMatchLineNumber ? BST_CHECKED : BST_UNCHECKED, 0);
	::SendDlgItemMessage(_hSelf, IDWHOLEWORD_FIFOLDER, BM_SETCHECK, _options._isWholeWord ? BST_CHECKED : BST_UNCHECKED, 0);
	::SendDlgItemMessage(_hSelf, IDMATCHCASE_FIFOLDER, BM_SETCHECK, _options._isMatchCase ? BST_CHECKED : BST_UNCHECKED, 0);
	
	::SendDlgItemMessage(_hSelf, IDNORMAL_FIFOLDER, BM_SETCHECK, _options._searchType == FindNormal ? BST_CHECKED : BST_UNCHECKED, 0);
	::SendDlgItemMessage(_hSelf, IDEXTENDED_FIFOLDER, BM_SETCHECK, _options._searchType == FindExtended ? BST_CHECKED : BST_UNCHECKED, 0);
	::SendDlgItemMessage(_hSelf, IDREGEXP_FIFOLDER, BM_SETCHECK, _options._searchType == FindRegex ? BST_CHECKED : BST_UNCHECKED, 0);

	::SendDlgItemMessage(_hSelf, IDREDOTMATCHNL_FIFOLDER, BM_SETCHECK, _options._dotMatchesNewline ? BST_CHECKED : BST_UNCHECKED, 0);
}

void FindInFinderDlg::writeOptions()	{

	HWND hFindCombo = ::GetDlgItem(_hSelf, IDFINDWHAT_FIFOLDER);
	_options._str2Search = getTextFromCombo(hFindCombo);
	_options._isMatchLineNumber = isCheckedOrNot(IDC_MATCHLINENUM_CHECK_FIFOLDER);
	_options._isWholeWord = isCheckedOrNot(IDWHOLEWORD_FIFOLDER);
	_options._isMatchCase = isCheckedOrNot(IDMATCHCASE_FIFOLDER);
	_options._searchType = isCheckedOrNot(IDREGEXP_FIFOLDER) ? FindRegex : isCheckedOrNot(IDEXTENDED_FIFOLDER) ? FindExtended : FindNormal;

	_options._dotMatchesNewline = isCheckedOrNot(IDREDOTMATCHNL_FIFOLDER);
}

INT_PTR CALLBACK FindInFinderDlg::run_dlgProc(UINT message, WPARAM wParam, LPARAM /*lParam*/)	{

	switch (message)	{

		case WM_INITDIALOG:	{

			NativeLangSpeaker *pNativeSpeaker = param.getNativeLangSpeaker();
			pNativeSpeaker->changeDlgLang(_hSelf, "FindInFinder");
			initFromOptions();
		}
		return TRUE;

		case WM_COMMAND:	{

			switch (LOWORD(wParam))	{

				case IDCANCEL:
					::EndDialog(_hSelf, -1);
				return TRUE;

				case IDOK:
					writeOptions();
					::EndDialog(_hSelf, -1);
					FindersInfo findersInfo;
					findersInfo._pSourceFinder = _pFinder2Search;
					findersInfo._findOption = _options;
					::SendMessage(_hParent, WM_FINDALL_INCURRENTFINDER, reinterpret_cast<WPARAM>(&findersInfo), 0);
					return TRUE;
			}
			return FALSE;
		}
		default:
			return FALSE;
	}
}


void FindReplaceDlg::resizeDialogElements(LONG newWidth)	{

	//elements that need to be resized horizontally (all edit/combo boxes etc.)
	const auto resizeWindowIDs = { IDFINDWHAT, IDREPLACEWITH, IDD_FINDINFILES_FILTERS_COMBO, IDD_FINDINFILES_DIR_COMBO };

	//elements that need to be moved
	const auto moveWindowIDs = {
		IDD_FINDINFILES_FOLDERFOLLOWSDOC_CHECK,IDD_FINDINFILES_RECURSIVE_CHECK, IDD_FINDINFILES_INHIDDENDIR_CHECK,
		IDC_FIND_REPLACE_SWAP, IDC_FIND_REPLACE_COPY,
		IDSWAP_S, IDCOPY_S,
		IDC_TRANSPARENT_GRPBOX, IDC_TRANSPARENT_CHECK, IDC_TRANSPARENT_LOSSFOCUS_RADIO, IDC_TRANSPARENT_ALWAYS_RADIO,
		IDC_PERCENTAGE_SLIDER , IDC_REPLACEINSELECTION , IDC_IN_SELECTION_CHECK,

		IDD_FINDINFILES_BROWSE_BUTTON, IDCMARKALL, IDC_CLEAR_ALL, IDCCOUNTALL, IDC_CLEAR_FINDALL_OPENEDFILES, IDC_FINDALL_OPENEDFILES, IDC_CLEAR_FINDALL_CURRENTFILE, IDC_FINDALL_CURRENTFILE,
		IDREPLACE1, IDREPLACE_FINDNEXT,
		IDREPLACEALL,IDREPLACEALL_SAVE,IDC_REPLACE_OPENEDFILES, IDD_FINDINFILES_FIND_BUTTON, IDD_FINDINFILES_CLEAR_FIND,IDD_FINDINFILES_REPLACEINFILES, IDOK, IDCANCEL,
		IDC_FINDPREV, IDC_FINDNEXT, IDC_2_BUTTONS_MODE
	};

	const UINT flags = SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE | SWP_NOCOPYBITS;

	auto newDeltaWidth = newWidth - _initialClientWidth;
	auto addWidth = newDeltaWidth - _deltaWidth;
	_deltaWidth = newDeltaWidth;

	RECT rc;
	for (int id : resizeWindowIDs)	{

		HWND resizeHwnd = ::GetDlgItem(_hSelf, id);
		::GetClientRect(resizeHwnd, &rc);

		// Combo box for some reasons selects text on resize. So let's check befor resize if selection is present and clear it manually after resize.
		DWORD endSelection = 0;
		SendMessage(resizeHwnd, CB_GETEDITSEL, 0, (LPARAM)&endSelection);

		::SetWindowPos(resizeHwnd, NULL, 0, 0, rc.right + addWidth, rc.bottom, SWP_NOMOVE | flags);

		if (!endSelection)	{

			SendMessage(resizeHwnd, CB_SETEDITSEL, 0, 0);
		}
	}

	for (int moveWndID : moveWindowIDs)	{

		HWND moveHwnd = GetDlgItem(_hSelf, moveWndID);
		::GetWindowRect(moveHwnd, &rc);
		::MapWindowPoints(NULL, _hSelf, (LPPOINT)&rc, 2);

		::SetWindowPos(moveHwnd, NULL, rc.left + addWidth, rc.top, 0, 0, SWP_NOSIZE | flags);
	}

	auto additionalWindowHwndsToResize = { _tab.getHSelf() , _statusBar.getHSelf() };
	for (HWND resizeHwnd : additionalWindowHwndsToResize)	{

		::GetClientRect(resizeHwnd, &rc);
		::SetWindowPos(resizeHwnd, NULL, 0, 0, rc.right + addWidth, rc.bottom, SWP_NOMOVE | flags);
	}
}

std::mutex findOps_mutex;

INT_PTR CALLBACK FindReplaceDlg::run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam)	{

	switch (message)	{ 

		case WM_GETMINMAXINFO:	{

			MINMAXINFO* mmi = reinterpret_cast<MINMAXINFO*>(lParam);
			mmi->ptMinTrackSize.y = _initialWindowRect.bottom;
			mmi->ptMinTrackSize.x = _initialWindowRect.right;
			mmi->ptMaxTrackSize.y = _initialWindowRect.bottom;
			return 0;
		}

		case WM_SIZE:	{

			resizeDialogElements(LOWORD(lParam));
			return TRUE;
		}

		case WM_INITDIALOG :	{

			if (nGUI._monospacedFontFindDlg)	{

				HWND hFindCombo = ::GetDlgItem(_hSelf, IDFINDWHAT);
				HWND hReplaceCombo = ::GetDlgItem(_hSelf, IDREPLACEWITH);
				HWND hFiltersCombo = ::GetDlgItem(_hSelf, IDD_FINDINFILES_FILTERS_COMBO);
				HWND hDirCombo = ::GetDlgItem(_hSelf, IDD_FINDINFILES_DIR_COMBO);

				const TCHAR* fontName = _T("Courier New");
				const long nFontSize = 8;

				HDC hdc = GetDC(_hSelf);

				LOGFONT logFont = { 0 };
				logFont.lfHeight = -MulDiv(nFontSize, GetDeviceCaps(hdc, LOGPIXELSY), 72);
				_tcscpy_s(logFont.lfFaceName, fontName);

				_hMonospaceFont = CreateFontIndirect(&logFont);

				ReleaseDC(_hSelf, hdc);

				SendMessage(hFindCombo, WM_SETFONT, (WPARAM)_hMonospaceFont, MAKELPARAM(true, 0));
				SendMessage(hReplaceCombo, WM_SETFONT, (WPARAM)_hMonospaceFont, MAKELPARAM(true, 0));
				SendMessage(hFiltersCombo, WM_SETFONT, (WPARAM)_hMonospaceFont, MAKELPARAM(true, 0));
				SendMessage(hDirCombo, WM_SETFONT, (WPARAM)_hMonospaceFont, MAKELPARAM(true, 0));
			}

			RECT arc;
			::GetWindowRect(::GetDlgItem(_hSelf, IDCANCEL), &arc);
			// _findInFilesClosePos.bottom = 
			_replaceClosePos.bottom = _findClosePos.bottom = arc.bottom - arc.top;
			// _findInFilesClosePos.right =
			_replaceClosePos.right = _findClosePos.right = arc.right - arc.left;

			POINT p;
			p.x = arc.left;
			p.y = arc.top;
			::ScreenToClient(_hSelf, &p);

			p = getTopPoint(::GetDlgItem(_hSelf, IDCANCEL), !_isRTL);
			_replaceClosePos.left = p.x;
			_replaceClosePos.top = p.y;

			// p = getTopPoint(::GetDlgItem(_hSelf, IDREPLACEALL), !_isRTL);
			// _findInFilesClosePos.left = p.x;	_findInFilesClosePos.top = p.y;

			p = getTopPoint(::GetDlgItem(_hSelf, IDCANCEL), !_isRTL);
			_findClosePos.left = p.x;
			_findClosePos.top = p.y + 10;

			// in selection check
			RECT checkRect;
			::GetWindowRect(::GetDlgItem(_hSelf, IDC_IN_SELECTION_CHECK), &checkRect);
			_countInSelCheckPos.bottom = _replaceInSelCheckPos.bottom = checkRect.bottom - checkRect.top;
			_countInSelCheckPos.right = _replaceInSelCheckPos.right = checkRect.right - checkRect.left;

			p = getTopPoint(::GetDlgItem(_hSelf, IDC_IN_SELECTION_CHECK), !_isRTL);
			_countInSelCheckPos.left = _replaceInSelCheckPos.left = p.x;
			_countInSelCheckPos.top = _replaceInSelCheckPos.top = p.y;

			POINT countP = getTopPoint(::GetDlgItem(_hSelf, IDCCOUNTALL), !_isRTL);
			_countInSelCheckPos.top = countP.y + 4;

			// in selection Frame
			RECT frameRect;
			::GetWindowRect(::GetDlgItem(_hSelf, IDC_REPLACEINSELECTION), &frameRect);
			_countInSelFramePos.bottom = _replaceInSelFramePos.bottom = frameRect.bottom - frameRect.top;
			_countInSelFramePos.right = _replaceInSelFramePos.right = frameRect.right - frameRect.left;

			p = getTopPoint(::GetDlgItem(_hSelf, IDC_REPLACEINSELECTION), !_isRTL);
			_countInSelFramePos.left = _replaceInSelFramePos.left = p.x;
			_countInSelFramePos.top = _replaceInSelFramePos.top = p.y;

			_countInSelFramePos.top = countP.y - 9;

			NativeLangSpeaker *pNativeSpeaker = param.getNativeLangSpeaker();

			generic_string searchButtonTip = pNativeSpeaker->getLocalizedStrFromID("shift-change-direction-tip", L"Use Shift+Enter to search in the opposite direction.");
			_shiftTrickUpTip = CreateToolTip(IDOK, _hSelf, _hInst, const_cast<PTSTR>(searchButtonTip.c_str()));

			generic_string checkboxTip = pNativeSpeaker->getLocalizedStrFromID("two-find-buttons-tip", L"2 find buttons mode");
			_2ButtonsTip = CreateToolTip(IDC_2_BUTTONS_MODE, _hSelf, _hInst, const_cast<PTSTR>(checkboxTip.c_str()));

			generic_string findInFilesFilterTip = pNativeSpeaker->getLocalizedStrFromID("find-in-files-filter-tip", L"Find in cpp, cxx, h, hxx && hpp:\r*.cpp *.cxx *.h *.hxx *.hpp\r\rFind in all files except exe, obj && log:\r*.* !*.exe !*.obj !*.log");
			_filterTip = CreateToolTip(IDD_FINDINFILES_FILTERS_STATIC, _hSelf, _hInst, const_cast<PTSTR>(findInFilesFilterTip.c_str()));

			::SetWindowTextW(::GetDlgItem(_hSelf, IDC_FINDPREV), L"▲");
			::SetWindowTextW(::GetDlgItem(_hSelf, IDC_FINDNEXT), L"Next▼");
			::SetWindowTextW(::GetDlgItem(_hSelf, IDSWAP_S), L"▼▲");
			::SetWindowTextW(::GetDlgItem(_hSelf, IDCOPY_S), L"+▼");
			return TRUE;
		}

		case WM_DRAWITEM :	{

			drawItem((DRAWITEMSTRUCT *)lParam);
			return TRUE;
		}

		case WM_HSCROLL :	{

			if (reinterpret_cast<HWND>(lParam) == ::GetDlgItem(_hSelf, IDC_PERCENTAGE_SLIDER))	{

				int percent = static_cast<int32_t>(::SendDlgItemMessage(_hSelf, IDC_PERCENTAGE_SLIDER, TBM_GETPOS, 0, 0));
				FindHistory & findHistory = param.getFindHistory();
				findHistory._transparency = percent;
				if (isCheckedOrNot(IDC_TRANSPARENT_ALWAYS_RADIO))	{

					param.SetTransparent(_hSelf, percent);
				}
			}
			return TRUE;
		}

		case WM_NOTIFY:	{

			NMHDR *nmhdr = (NMHDR *)lParam;
			if (nmhdr->code == TCN_SELCHANGE)	{

				HWND tabHandle = _tab.getHSelf();
				if (nmhdr->hwndFrom == tabHandle)	{

					int indexClicked = static_cast<int32_t>(::SendMessage(tabHandle, TCM_GETCURSEL, 0, 0));
					doDialog((DIALOG_TYPE)indexClicked);
				}
				return TRUE;
			}
			break;
		}

		case WM_ACTIVATE :	{

			if (LOWORD(wParam) == WA_ACTIVE || LOWORD(wParam) == WA_CLICKACTIVE)	{

				Sci_CharacterRange cr = (*_ppEditView)->getSelection();
				int nbSelected = cr.cpMax - cr.cpMin;

				_options._isInSelection = isCheckedOrNot(IDC_IN_SELECTION_CHECK)?1:0;
				int checkVal = _options._isInSelection?BST_CHECKED:BST_UNCHECKED;
				
				if (!_options._isInSelection)	{

					if (nbSelected <= 1024)	{

						checkVal = BST_UNCHECKED;
						_options._isInSelection = false;
					}
					else	{

						checkVal = BST_CHECKED;
						_options._isInSelection = true;
					}
				}
				// Searching/replacing in multiple selections or column selection is not allowed 
				if (((*_ppEditView)->f(SCI_GETSELECTIONMODE) == SC_SEL_RECTANGLE) || ((*_ppEditView)->f(SCI_GETSELECTIONS) > 1))	{

					checkVal = BST_UNCHECKED;
					_options._isInSelection = false;
					nbSelected = 0;
				}
				::EnableWindow(::GetDlgItem(_hSelf, IDC_IN_SELECTION_CHECK), nbSelected);
				// uncheck if the control is disable
				if (!nbSelected)	{

					checkVal = BST_UNCHECKED;
					_options._isInSelection = false;
				}
				::SendDlgItemMessage(_hSelf, IDC_IN_SELECTION_CHECK, BM_SETCHECK, checkVal, 0);
			}
			
			if (isCheckedOrNot(IDC_TRANSPARENT_LOSSFOCUS_RADIO))	{

				if (LOWORD(wParam) == WA_INACTIVE && isVisible())	{

					int percent = static_cast<int32_t>(::SendDlgItemMessage(_hSelf, IDC_PERCENTAGE_SLIDER, TBM_GETPOS, 0, 0));
					param.SetTransparent(_hSelf, percent);
				}
				else	{

					param.removeTransparent(_hSelf);
				}
			}

			// At very first time (when find dlg is launched), search mode is Normal.
			// In that case, ". Matches newline" should be disabled as it applicable on for Regex
			if (isCheckedOrNot(IDREGEXP))	{

				::EnableWindow(GetDlgItem(_hSelf, IDREDOTMATCHNL), true);
			}
			else	{

				::EnableWindow(GetDlgItem(_hSelf, IDREDOTMATCHNL), false);
			}
			return TRUE;
		}

		case NPPM_MODELESSDIALOG :
			return ::SendMessage(_hParent, NPPM_MODELESSDIALOG, wParam, lParam);
 
		case WM_COMMAND :	{ 

			bool isMacroRecording = (::SendMessage(_hParent, WM_GETCURRENTMACROSTATUS,0,0) == MACRO_RECORDING_IN_PROGRESS);
			FindHistory & findHistory = param.getFindHistory();

			switch (LOWORD(wParam))	{
				case IDC_2_BUTTONS_MODE:	{

					bool is2ButtonsMode = isCheckedOrNot(IDC_2_BUTTONS_MODE);
					findHistory._isSearch2ButtonsMode = is2ButtonsMode;

					::ShowWindow(::GetDlgItem(_hSelf, IDC_FINDPREV), is2ButtonsMode ? SW_SHOW : SW_HIDE);
					::ShowWindow(::GetDlgItem(_hSelf, IDC_FINDNEXT), is2ButtonsMode ? SW_SHOW : SW_HIDE);
					::ShowWindow(::GetDlgItem(_hSelf, IDOK), is2ButtonsMode ? SW_HIDE : SW_SHOW);
				}
				break;

				case IDCANCEL:
					(*_ppEditView)->f(SCI_CALLTIPCANCEL);
					setStatusbarMessage(generic_string(), FSNoMessage);
					display(false);
					break;

				case IDC_FINDPREV:
				case IDC_FINDNEXT:
				case IDOK :	{

					setStatusbarMessage(generic_string(), FSNoMessage);

					HWND hFindCombo = ::GetDlgItem(_hSelf, IDFINDWHAT);
					_options._str2Search = getTextFromCombo(hFindCombo);
					updateCombo(IDFINDWHAT);

					param._isFindReplacing = true;
					if (isMacroRecording)
						saveInMacro(wParam, FR_OP_FIND);

					bool direction_bak = _options._whichDirection;

					if (LOWORD(wParam) == IDC_FINDPREV)
						_options._whichDirection = DIR_UP;

					else if (LOWORD(wParam) == IDC_FINDNEXT)
						_options._whichDirection = DIR_DOWN;

					else	{
						// if shift-key is pressed, revert search direction
						// if shift-key is not pressed, use the normal setting
						if (GetKeyState(VK_SHIFT) & SHIFTED)
							_options._whichDirection = !_options._whichDirection;
					}

					FindStatus findStatus = FSFound;
					processFindNext(_options._str2Search.c_str(), _env, &findStatus);
					// restore search direction which may have been overwritten because shift-key was pressed
					_options._whichDirection = direction_bak;

					NativeLangSpeaker& pNativeSpeaker = *param.getNativeLangSpeaker();
					if (findStatus == FSEndReached)	{

						generic_string msg = pNativeSpeaker.getLocalizedStrFromID("find-status-end-reached", L"Find: Found the 1st occurrence from the top. The end of the document has been reached.");
						setStatusbarMessage(msg, FSEndReached);
					}
					else if (findStatus == FSTopReached)	{

						generic_string msg = pNativeSpeaker.getLocalizedStrFromID("find-status-top-reached", L"Find: Found the 1st occurrence from the bottom. The beginning of the document has been reached.");
						setStatusbarMessage(msg, FSTopReached);
					}

					param._isFindReplacing = false;
				}
				return TRUE;

			/* 	case IDM_SEARCH_FIND:
					goToCenter();
					return TRUE;
 */
				case IDREPLACE_FINDNEXT :	{
					std::lock_guard<std::mutex> lock(findOps_mutex);

					if (_currentStatus == REPLACE_DLG)	{
						setStatusbarMessage(L"", FSNoMessage);
						HWND hFindCombo = ::GetDlgItem(_hSelf, IDFINDWHAT);
						HWND hReplaceCombo = ::GetDlgItem(_hSelf, IDREPLACEWITH);
						_options._str2Search = getTextFromCombo(hFindCombo);
						_options._str4Replace = getTextFromCombo(hReplaceCombo);
						updateCombos();

						param._isFindReplacing = true;
						if (isMacroRecording) saveInMacro(wParam, FR_OP_REPLACE);
						processReplace(_options._str2Search.c_str(), _options._str4Replace.c_str());
						param._isFindReplacing = false;
					}
				}
				return TRUE;

				case IDREPLACE1 :	{
					std::lock_guard<std::mutex> lock(findOps_mutex);
					if (_currentStatus == REPLACE_DLG)	{
						setStatusbarMessage(L"", FSNoMessage);
						HWND hFindCombo = ::GetDlgItem(_hSelf, IDFINDWHAT);
						HWND hReplaceCombo = ::GetDlgItem(_hSelf, IDREPLACEWITH);
						_options._str2Search = getTextFromCombo(hFindCombo);
						_options._str4Replace = getTextFromCombo(hReplaceCombo);
						updateCombos();

						param._isFindReplacing = true;
						if (isMacroRecording) saveInMacro(wParam, FR_OP_REPLACE);
						if (_options._str2Search[0])
							processReplc1(_options._str2Search.c_str(), _options._str4Replace.c_str());
						param._isFindReplacing = false;
					}
				}
				return TRUE;

//Process actions
				case IDC_CLEAR_FINDALL_OPENEDFILES :
					_pFinder->_pMainFoundInfos->clear();
					_pFinder->_pMainMarkings->clear();
					_pFinder->setFinderReadOnly(false);
					_pFinder->_scintView.f(SCI_CLEARALL);
	
				case IDC_FINDALL_OPENEDFILES :	{

					if (_currentStatus == REPLACE_DLG)	{
						setStatusbarMessage(L"", FSNoMessage);
						HWND hFindCombo = ::GetDlgItem(_hSelf, IDFINDWHAT);
								combo2ExtendedMode(IDFINDWHAT);
						_options._str2Search = getTextFromCombo(hFindCombo);
						updateCombo(IDFINDWHAT);

						param._isFindReplacing = true;
						if (isMacroRecording) saveInMacro(wParam, FR_OP_FIND + FR_OP_GLOBAL);
						findAllIn(WM_FINDALL_INOPENEDDOC);
						param._isFindReplacing = false;
					}
				}
				return TRUE;

				case IDC_CLEAR_FINDALL_CURRENTFILE :
					_pFinder->_pMainFoundInfos->clear();
					_pFinder->_pMainMarkings->clear();
					_pFinder->setFinderReadOnly(false);
					_pFinder->_scintView.f(SCI_CLEARALL);
	
				case IDC_FINDALL_CURRENTFILE :	{

					setStatusbarMessage(L"", FSNoMessage);
					HWND hFindCombo = ::GetDlgItem(_hSelf, IDFINDWHAT);
						combo2ExtendedMode(IDFINDWHAT);
					_options._str2Search = getTextFromCombo(hFindCombo);
					updateCombo(IDFINDWHAT);

					param._isFindReplacing = true;
					if (isMacroRecording) saveInMacro(wParam, FR_OP_FIND + FR_OP_GLOBAL);
					findAllIn(WM_FINDALL_INCURRENTDOC);
					param._isFindReplacing = false;
				}
				return TRUE;

				case IDD_FINDINFILES_CLEAR_FIND:
					_pFinder->_pMainFoundInfos->clear();
					_pFinder->_pMainMarkings->clear();
					_pFinder->setFinderReadOnly(false);
					_pFinder->_scintView.f(SCI_CLEARALL);

				case IDD_FINDINFILES_FIND_BUTTON :	{
					setStatusbarMessage(L"", FSNoMessage);
					const int filterSize = 256;
					TCHAR filters[filterSize+1];
					filters[filterSize] = '\0';
					TCHAR directory[MAX_PATH];
					
					::GetDlgItemText(_hSelf, IDD_FINDINFILES_FILTERS_COMBO, filters, filterSize);
					_options._filters = filters;
					addText2Combo(filters, ::GetDlgItem(_hSelf, IDD_FINDINFILES_FILTERS_COMBO));

					::GetDlgItemText(_hSelf, IDD_FINDINFILES_DIR_COMBO, directory, MAX_PATH);
					_options._directory = directory;
					addText2Combo(directory, ::GetDlgItem(_hSelf, IDD_FINDINFILES_DIR_COMBO));
					
					if (directory[0] && (directory[lstrlen(directory)-1] != '\\'))		_options._directory += L"\\";

					HWND hFindCombo = ::GetDlgItem(_hSelf, IDFINDWHAT);
						combo2ExtendedMode(IDFINDWHAT);
					_options._str2Search = getTextFromCombo(hFindCombo);
					updateCombo(IDFINDWHAT);

					param._isFindReplacing = true;
					if (isMacroRecording) saveInMacro(wParam, FR_OP_FIND + FR_OP_FIF);
					findAllIn(WM_FINDINFILES);
					param._isFindReplacing = false;
				}
				return TRUE;

				case IDD_FINDINFILES_REPLACEINFILES :
				if (_currentStatus == FINDINFILES_DLG)	{
					std::lock_guard<std::mutex> lock(findOps_mutex);

					setStatusbarMessage(L"", FSNoMessage);
					const int filterSize = 256;
					TCHAR filters[filterSize];
					TCHAR directory[MAX_PATH];
					::GetDlgItemText(_hSelf, IDD_FINDINFILES_FILTERS_COMBO, filters, filterSize);
					_options._filters = filters;
					addText2Combo(filters, ::GetDlgItem(_hSelf, IDD_FINDINFILES_FILTERS_COMBO));

					::GetDlgItemText(_hSelf, IDD_FINDINFILES_DIR_COMBO, directory, MAX_PATH);
					_options._directory = directory;
					addText2Combo(directory, ::GetDlgItem(_hSelf, IDD_FINDINFILES_DIR_COMBO));
					
					if (directory[0] && (directory[lstrlen(directory)-1] != '\\'))
						_options._directory += L"\\";

					generic_string msg = L"Are you sure you want to replace all occurrences in :\r";
					msg += _options._directory + L"\rfor file type : ";
					msg += _options._filters[0]?_options._filters:L"*.*";
					int res = ::MessageBox(_hParent, msg.c_str(), L"Are you sure?", MB_OKCANCEL | MB_DEFBUTTON2);
					if (res == IDOK)	{

						HWND hFindCombo = ::GetDlgItem(_hSelf, IDFINDWHAT);
						_options._str2Search = getTextFromCombo(hFindCombo);
						HWND hReplaceCombo = ::GetDlgItem(_hSelf, IDREPLACEWITH);
						_options._str4Replace = getTextFromCombo(hReplaceCombo);
						updateCombo(IDFINDWHAT);
						updateCombo(IDREPLACEWITH);

						param._isFindReplacing = true;
						if (isMacroRecording) saveInMacro(wParam, FR_OP_REPLACE + FR_OP_FIF);
						::SendMessage(_hParent, WM_REPLACEINFILES, 0, 0);
						param._isFindReplacing = false;
					}
				::SetFocus(::GetDlgItem(_hSelf, IDFINDWHAT));
				}
				return TRUE;

				case IDC_REPLACE_OPENEDFILES :	{

					std::lock_guard<std::mutex> lock(findOps_mutex);

					if (_currentStatus == REPLACE_DLG)	{

						setStatusbarMessage(L"", FSNoMessage);
						HWND hFindCombo = ::GetDlgItem(_hSelf, IDFINDWHAT);
						_options._str2Search = getTextFromCombo(hFindCombo);
						HWND hReplaceCombo = ::GetDlgItem(_hSelf, IDREPLACEWITH);
						_options._str4Replace = getTextFromCombo(hReplaceCombo);
						updateCombos();

						param._isFindReplacing = true;
						if (isMacroRecording) saveInMacro(wParam, FR_OP_REPLACE + FR_OP_GLOBAL);
						replaceAllInOpenedDocs();
						param._isFindReplacing = false;
					}
				}			
				return TRUE;

				case IDREPLACEALL_SAVE :
				case IDREPLACEALL :	{

					std::lock_guard<std::mutex> lock(findOps_mutex);

					if (_currentStatus == REPLACE_DLG)	{

						setStatusbarMessage(L"", FSNoMessage);
						if ((*_ppEditView)->getCurrentBuffer()->isReadOnly())	{

							NativeLangSpeaker *pNativeSpeaker = param.getNativeLangSpeaker();
							generic_string msg = pNativeSpeaker->getLocalizedStrFromID("find-status-replace-readonly", L"Replace: Cannot replace text. The current document is read only.");
							setStatusbarMessage(msg, FSNotFound);
							return TRUE;
						}

						HWND hFindCombo = ::GetDlgItem(_hSelf, IDFINDWHAT);
						_options._str2Search = getTextFromCombo(hFindCombo);
						HWND hReplaceCombo = ::GetDlgItem(_hSelf, IDREPLACEWITH);
						_options._str4Replace = getTextFromCombo(hReplaceCombo);
						updateCombos();

						param._isFindReplacing = true;
						if (isMacroRecording) saveInMacro(wParam, FR_OP_REPLACE);
						(*_ppEditView)->f(SCI_BEGINUNDOACTION);
						int nbReplaced = processAll(ProcessReplaceAll, &_options);
						(*_ppEditView)->f(SCI_ENDUNDOACTION);
						param._isFindReplacing = false;

						generic_string result;
						NativeLangSpeaker *pNativeSpeaker = param.getNativeLangSpeaker();
						if (nbReplaced < 0)	{

							result = pNativeSpeaker->getLocalizedStrFromID("find-status-replaceall-re-malformed", L"Replace All: The regular expression is malformed.");
						}
						else	{

							if (nbReplaced == 1)	{
								if(wParam == IDREPLACEALL_SAVE)
								result = pNativeSpeaker->getLocalizedStrFromID("find-status-replaceall-1-replaced", L"Replace All: 1 occurrence was replaced, then saved.");
								else
								result = pNativeSpeaker->getLocalizedStrFromID("find-status-replaceall-1-replaced", L"Replace All: 1 occurrence was replaced.");
							}
							else	{
								if(wParam == IDREPLACEALL_SAVE)	{
									nGUI.pNpp->fileSave();
									result = pNativeSpeaker->getLocalizedStrFromID("find-status-replaceall-nb-replaced", L"Replace All: $INT_REPLACE$ occurrences were replaced, then saved.");
								}
								result = pNativeSpeaker->getLocalizedStrFromID("find-status-replaceall-nb-replaced", L"Replace All: $INT_REPLACE$ occurrences were replaced.");
								result = stringReplace(result, L"$INT_REPLACE$", std::to_wstring(nbReplaced));
							}
						}
						setStatusbarMessage(result, FSMessage);
						focus();
					}
				}
				return TRUE;

				case IDC_FIND_REPLACE_SWAP :	{

					HWND hFCombo = ::GetDlgItem(_hSelf, IDFINDWHAT),
					hRCombo = ::GetDlgItem(_hSelf, IDREPLACEWITH);
					generic_string _str4Replace = getTextFromCombo(hRCombo);
					addText2Combo(getTextFromCombo(hFCombo).c_str(), hRCombo);
					addText2Combo(_str4Replace.c_str(), hFCombo);
					focus();
				}
				return TRUE;

				case IDC_FIND_REPLACE_COPY:	{

					HWND hFCombo = ::GetDlgItem(_hSelf, IDFINDWHAT);
					addText2Combo(getTextFromCombo(hFCombo).c_str(), ::GetDlgItem(_hSelf, IDREPLACEWITH));
					updateCombo(IDREPLACEWITH);
					::SetFocus(::GetDlgItem(_hSelf, IDREPLACEWITH));
				}
				return TRUE;
				
				case IDC_UNDO:	{
					std::lock_guard<std::mutex> lock(findOps_mutex);
					(*_ppEditView)->f(WM_UNDO);
					nGUI.pNpp->cClipb();
					nGUI.pNpp->cUndoSt();
					break;
				}
				case IDC_REDO:	{
					std::lock_guard<std::mutex> lock(findOps_mutex);
					(*_ppEditView)->f(SCI_REDO);
					nGUI.pNpp->cClipb();
					nGUI.pNpp->cUndoSt();
					break;
				}				

				case IDCCOUNTALL :	{

						setStatusbarMessage(L"", FSNoMessage);
						HWND hFindCombo = ::GetDlgItem(_hSelf, IDFINDWHAT);
						updateCombo(IDFINDWHAT);
						_options._str2Search = getTextFromCombo(hFindCombo);

						int nbCounted = processAll(ProcessCountAll, &_options);

						generic_string result;
						NativeLangSpeaker *pNativeSpeaker = param.getNativeLangSpeaker();
						if (nbCounted < 0)	{
							result = pNativeSpeaker->getLocalizedStrFromID("find-status-count-re-malformed", L"Count: The regular expression to search is malformed.");
						}
						else	{
							result = pNativeSpeaker->getLocalizedStrFromID("find-status-count-nb-matches", L"Count: $INT_REPLACE$ match(es).");
							result = stringReplace(result, L"$INT_REPLACE$", std::to_wstring(nbCounted));
						}
						if (isMacroRecording) saveInMacro(wParam, FR_OP_FIND);
						setStatusbarMessage(result, FSMessage);
						focus();
				}
				return TRUE;

				case IDCMARKALL :	{

					if (_currentStatus == MARK_DLG)	{

						setStatusbarMessage(L"", FSNoMessage);
						HWND hFindCombo = ::GetDlgItem(_hSelf, IDFINDWHAT);
						_options._str2Search = getTextFromCombo(hFindCombo);
						updateCombo(IDFINDWHAT);

						if (isMacroRecording) saveInMacro(wParam, FR_OP_FIND);
						param._isFindReplacing = true;
						int nbMarked = processAll(ProcessMarkAll, &_options);
						param._isFindReplacing = false;
						
						generic_string result;
						NativeLangSpeaker *pNativeSpeaker = param.getNativeLangSpeaker();
						if (nbMarked < 0)	{

							result = pNativeSpeaker->getLocalizedStrFromID("find-status-mark-re-malformed", L"Mark: The regular expression to search is malformed.");
						}
						else	{

							if (nbMarked == 1)	{

								result = pNativeSpeaker->getLocalizedStrFromID("find-status-mark-1-match", L"Mark: 1 match.");
							}
							else	{

								result = pNativeSpeaker->getLocalizedStrFromID("find-status-mark-nb-matches", L"Mark: $INT_REPLACE$ matches.");
								result = stringReplace(result, L"$INT_REPLACE$", std::to_wstring(nbMarked));
							}
						}
						setStatusbarMessage(result, FSMessage);
						focus();
					}
				}
				return TRUE;

				case IDC_CLEAR_ALL :	{

					if (_currentStatus == MARK_DLG)	{

						if (isMacroRecording) saveInMacro(wParam, FR_OP_FIND);
						clearMarks(_options);
					}
				}
				return TRUE;
//Option actions
				case IDREDOTMATCHNL:
					findHistory._dotMatchesNewline = _options._dotMatchesNewline = isCheckedOrNot(IDREDOTMATCHNL);
					return TRUE;

				case IDWHOLEWORD :
					findHistory._isMatchWord = _options._isWholeWord = isCheckedOrNot(IDWHOLEWORD);
					return TRUE;

				case IDMATCHCASE :
					findHistory._isMatchCase = _options._isMatchCase = isCheckedOrNot(IDMATCHCASE);
					return TRUE;

				case IDNORMAL:
				case IDEXTENDED:
				case IDREGEXP : {
					if (isCheckedOrNot(IDREGEXP))	{

						_options._searchType = FindRegex;
						findHistory._searchMode = FindHistory::regExpr;
						::EnableWindow(GetDlgItem(_hSelf, IDREDOTMATCHNL), true);
					}
					else if (isCheckedOrNot(IDEXTENDED))	{

						_options._searchType = FindExtended;
						findHistory._searchMode = FindHistory::extended;
						::EnableWindow(GetDlgItem(_hSelf, IDREDOTMATCHNL), false);
					}
					else	{

						_options._searchType = FindNormal;
						findHistory._searchMode = FindHistory::normal;
						::EnableWindow(GetDlgItem(_hSelf, IDREDOTMATCHNL), false);
					}

					bool isRegex = (_options._searchType == FindRegex);
					if (isRegex)	{ 
	
						//regex doesn't allow whole word
						_options._isWholeWord = false;
						::SendDlgItemMessage(_hSelf, IDWHOLEWORD, BM_SETCHECK, _options._isWholeWord?BST_CHECKED:BST_UNCHECKED, 0);

						//regex upward search is disable in v6.3 due to a regression
						::SendDlgItemMessage(_hSelf, IDC_BACKWARDDIRECTION, BM_SETCHECK, BST_UNCHECKED, 0);
						_options._whichDirection = DIR_DOWN;
					}

					::EnableWindow(::GetDlgItem(_hSelf, IDWHOLEWORD), (BOOL)!isRegex);

					//regex upward search is disable in v6.3 due to a regression
					::EnableWindow(::GetDlgItem(_hSelf, IDC_BACKWARDDIRECTION), (BOOL)!isRegex);
					return TRUE; }

				case IDWRAP :
					findHistory._isWrap = _options._isWrapAround = isCheckedOrNot(IDWRAP);
					return TRUE;

				case IDC_BACKWARDDIRECTION:
					_options._whichDirection = isCheckedOrNot(IDC_BACKWARDDIRECTION) ? DIR_UP : DIR_DOWN;
					findHistory._isDirectionDown = _options._whichDirection == DIR_DOWN;
					return TRUE;

				case IDC_PURGE_CHECK :	{

					if (_currentStatus == MARK_DLG)
						_options._doPurge = isCheckedOrNot(IDC_PURGE_CHECK);
				}
				return TRUE;

				case IDC_MARKLINE_CHECK :	{

					if (_currentStatus == MARK_DLG)
						_options._doMarkLine = isCheckedOrNot(IDC_MARKLINE_CHECK);
				}
				return TRUE;

				case IDC_IN_SELECTION_CHECK :	{

					//(_currentStatus == FIND_DLG) ||					
					if ((_currentStatus == REPLACE_DLG) || (_currentStatus == MARK_DLG))
						_options._isInSelection = isCheckedOrNot(IDC_IN_SELECTION_CHECK);
				}
				return TRUE;

				case IDC_TRANSPARENT_CHECK :	{

					bool isChecked = isCheckedOrNot(IDC_TRANSPARENT_CHECK);

					::EnableWindow(::GetDlgItem(_hSelf, IDC_TRANSPARENT_GRPBOX), isChecked);
					::EnableWindow(::GetDlgItem(_hSelf, IDC_TRANSPARENT_LOSSFOCUS_RADIO), isChecked);
					::EnableWindow(::GetDlgItem(_hSelf, IDC_TRANSPARENT_ALWAYS_RADIO), isChecked);
					::EnableWindow(::GetDlgItem(_hSelf, IDC_PERCENTAGE_SLIDER), isChecked);

					if (isChecked)	{

						::SendDlgItemMessage(_hSelf, IDC_TRANSPARENT_LOSSFOCUS_RADIO, BM_SETCHECK, BST_CHECKED, 0);
						findHistory._transparencyMode = FindHistory::onLossingFocus;
					}
					else	{

						::SendDlgItemMessage(_hSelf, IDC_TRANSPARENT_LOSSFOCUS_RADIO, BM_SETCHECK, BST_UNCHECKED, 0);
						::SendDlgItemMessage(_hSelf, IDC_TRANSPARENT_ALWAYS_RADIO, BM_SETCHECK, BST_UNCHECKED, 0);
						param.removeTransparent(_hSelf);
						findHistory._transparencyMode = FindHistory::none;
					}

					return TRUE;
				}

				case IDC_TRANSPARENT_ALWAYS_RADIO :	{

					int percent = static_cast<int32_t>(::SendDlgItemMessage(_hSelf, IDC_PERCENTAGE_SLIDER, TBM_GETPOS, 0, 0));
					param.SetTransparent(_hSelf, percent);
					findHistory._transparencyMode = FindHistory::persistant;
				}
				return TRUE;

				case IDC_TRANSPARENT_LOSSFOCUS_RADIO :	{

					param.removeTransparent(_hSelf);
					findHistory._transparencyMode = FindHistory::onLossingFocus;
				}
				return TRUE;

				//
				// Find in Files
				//
				case IDD_FINDINFILES_RECURSIVE_CHECK :	{

					if (_currentStatus == FINDINFILES_DLG)
						findHistory._isFifRecuisive = _options._isRecursive = isCheckedOrNot(IDD_FINDINFILES_RECURSIVE_CHECK);
					
				}
				return TRUE;

				case IDD_FINDINFILES_INHIDDENDIR_CHECK :	{

					if (_currentStatus == FINDINFILES_DLG)
						findHistory._isFifInHiddenFolder = _options._isInHiddenDir = isCheckedOrNot(IDD_FINDINFILES_INHIDDENDIR_CHECK);
					
				}
				return TRUE;

					case IDD_FINDINFILES_FOLDERFOLLOWSDOC_CHECK :	{

					if (_currentStatus == FINDINFILES_DLG)
								findHistory._isFolderFollowDoc = isCheckedOrNot(IDD_FINDINFILES_FOLDERFOLLOWSDOC_CHECK);

						if (findHistory._isFolderFollowDoc)	{

								const TCHAR * dir = param.getWorkingDir();
								::SetDlgItemText(_hSelf, IDD_FINDINFILES_DIR_COMBO, dir);
						}
					
				}
				return TRUE;

				case IDD_FINDINFILES_BROWSE_BUTTON :	{

					if (_currentStatus == FINDINFILES_DLG)
						folderBrowser(_hSelf, L"Select a folder to search from", IDD_FINDINFILES_DIR_COMBO, _options._directory.c_str());
				}
				return TRUE;

				default :
					break;
			}
			break;
		}
	}
	return FALSE;
}

// return value :
// true  : the text2find is found
// false : the text2find is not found

bool FindReplaceDlg::processFindNext(const TCHAR *txt2find, const FindOption *options, FindStatus *oFindStatus, FindNextType findNextType /* = FINDNEXTTYPE_FINDNEXT */)	{

	ScintillaEditView *pScv =	*_ppEditView;
	if (oFindStatus)
		*oFindStatus = FSFound;

	if (!txt2find || !txt2find[0])	return false;

	const FindOption *pOptions = options?options:_env;

	pScv->f(SCI_CALLTIPCANCEL);

	int stringSizeFind = lstrlen(txt2find);
	TCHAR *pText = new TCHAR[stringSizeFind + 1];
	wcscpy_s(pText, stringSizeFind + 1, txt2find);
	
	if (pOptions->_searchType == FindExtended)	{

		stringSizeFind = Searching::convertExtendedToString(txt2find, pText, stringSizeFind);
	}

	int docLength = static_cast<int32_t>(pScv->f(SCI_GETLENGTH));
	Sci_CharacterRange cr = pScv->getSelection();


	//The search "zone" is relative to the selection, so search happens 'outside'
	int startPosition = cr.cpMax;
	int endPosition = docLength;

	
	if (pOptions->_whichDirection == DIR_UP)	{

		//When searching upwards, start is the lower part, end the upper, for backwards search
		startPosition = cr.cpMin;
		endPosition = 0;
	}

	if (pOptions->_incrementalType == FirstIncremental)	{

		// the text to find is modified so use the current position
		startPosition = cr.cpMin;
		endPosition = docLength;

		if (pOptions->_whichDirection == DIR_UP)	{

			//When searching upwards, start is the lower part, end the upper, for backwards search
			startPosition = cr.cpMax;
			endPosition = 0;
		}
	}
	else if (pOptions->_incrementalType == NextIncremental)	{

		// text to find is not modified, so use current position +1
		startPosition = cr.cpMin + 1;
		endPosition = docLength;	

		if (pOptions->_whichDirection == DIR_UP)	{

			//When searching upwards, start is the lower part, end the upper, for backwards search
			startPosition = cr.cpMax - 1;
			endPosition = 0;
		}
	}

	int flags = Searching::buildSearchFlags(pOptions);
	switch (findNextType)	{

		case FINDNEXTTYPE_FINDNEXT:
			flags |= SCFIND_REGEXP_EMPTYMATCH_ALL | SCFIND_REGEXP_SKIPCRLFASONE;
		break;

		case FINDNEXTTYPE_REPLACENEXT:
			flags |= SCFIND_REGEXP_EMPTYMATCH_NOTAFTERMATCH | SCFIND_REGEXP_SKIPCRLFASONE;
			break;

		case FINDNEXTTYPE_FINDNEXTFORREPLACE:
			flags |= SCFIND_REGEXP_EMPTYMATCH_ALL | SCFIND_REGEXP_EMPTYMATCH_ALLOWATSTART | SCFIND_REGEXP_SKIPCRLFASONE;
			break;
	}

	int end, fndStart;

	/* Never allow a zero length match in the middle of a line end marker
	if (pScv->f(SCI_GETCHARAT, startPosition - 1) == '\r'
		&& pScv->f(SCI_GETCHARAT, startPosition) == '\n') 
	{
		flags = flags & ~SCFIND_REGEXP_EMPTYMATCH_MASK | SCFIND_REGEXP_EMPTYMATCH_NONE;
	} */

	pScv->f(SCI_SETSEARCHFLAGS, flags);
	fndStart = pScv->searchInTarget(pText, stringSizeFind, startPosition, endPosition);
	
	 if (fndStart == -2)	{ // Invalid Regular expression

		NativeLangSpeaker *pNativeSpeaker = param.getNativeLangSpeaker();
		generic_string msg = pNativeSpeaker->getLocalizedStrFromID("find-status-invalid-re", L"Find: Invalid regular expression");
		setStatusbarMessage(msg, FSNotFound);
		return false;
	}
	
	else if (fndStart == -1)	{ //no match found in target, check if a new target

		if (pOptions->_isWrapAround)	{ 

			//when wrapping, use the rest of the document (entire document is usable)
			if (pOptions->_whichDirection == DIR_DOWN)	{

				if (oFindStatus)
					*oFindStatus = FSEndReached;
				fndStart = pScv->searchInTarget(pText, stringSizeFind, 0, docLength);
			}
			else	{
				if (oFindStatus)
					*oFindStatus = FSTopReached;
				fndStart = pScv->searchInTarget(pText, stringSizeFind, docLength, 0);
			}

		}

		if (fndStart == -1)	{

			if (oFindStatus)
				*oFindStatus = FSNotFound;
			//failed, or failed twice with wrap
			if (NotIncremental == pOptions->_incrementalType)	{ //incremental search doesnt trigger messages
					generic_string newTxt2find = stringReplace(txt2find, L"&", L"&&");
				generic_string msg = param.getNativeLangSpeaker()->getLocalizedStrFromID("find-status-cannot-find", L"Find: Can't find the text \"$STR_REPLACE$\"");
				setStatusbarMessage(stringReplace(msg, L"$STR_REPLACE$", newTxt2find), FSNotFound);
				
				// if the dialog is not shown, pass the focus to his parent(ie. Notepad++)
				if (::IsWindowVisible(_hSelf))
					::SetFocus(::GetDlgItem(_hSelf, IDFINDWHAT));
				else
					pScv->focus();
			}
			delete [] pText;
			return false;
		}
	}
	end = static_cast<int32_t>(pScv->f(SCI_GETTARGETEND));

	pScv->f(SCI_STOPRECORD);// prevent recording of absolute positioning commands issued in the process

	pScv->putMvmntInView(fndStart, end, startPosition);

	if (fndStart == end)	{// Show a calltip for a zero length match
		pScv->f(SCI_CALLTIPSETBACK, 0x33270D);pScv->f(SCI_CALLTIPSETFORE, 0xE1E9F3);
		pScv->f(SCI_CALLTIPSHOW, fndStart, reinterpret_cast<LPARAM>("empty match"));
	}
	if (::SendMessage(_hParent, WM_GETCURRENTMACROSTATUS,0,0) == MACRO_RECORDING_IN_PROGRESS)
		pScv->f(SCI_STARTRECORD);

	delete [] pText;

	return true;
}

// return value :
// true  : the text is replaced, and find the next occurrence
// false : the text2find is not found, so the text is NOT replace  || the text is replaced, and do NOT find the next occurrence
bool FindReplaceDlg::processReplace(const TCHAR *txt2find, const TCHAR *txt2replace, const FindOption *options)	{

	if (!txt2find || !txt2find[0] || !txt2replace)	return false;
	if ((*_ppEditView)->getCurrentBuffer()->isReadOnly())	{
		generic_string msg = param.getNativeLangSpeaker()->getLocalizedStrFromID("find-status-replace-readonly", L"Replace: Cannot replace text. The current document is read only.");
		setStatusbarMessage(msg, FSNotFound);
		return false;
	}
	FindOption replaceOptions = options ? *options : *_env;
	replaceOptions._incrementalType = FirstIncremental;

	Sci_CharacterRange currentSelection = (*_ppEditView)->getSelection();
	FindStatus status;
	bool moreMatches = processFindNext(txt2find, &replaceOptions, &status, FINDNEXTTYPE_FINDNEXTFORREPLACE);

	if (moreMatches)	{
		Sci_CharacterRange nextFind = (*_ppEditView)->getSelection();
		// If the next find is the same as the last, then perform the replacement
		if (nextFind.cpMin == currentSelection.cpMin && nextFind.cpMax == currentSelection.cpMax)	{

			int start = currentSelection.cpMin;
			int replacedLen = 0;
			if (replaceOptions._searchType == FindRegex)
				replacedLen = (*_ppEditView)->replaceTargetRegExMode(txt2replace);
			else if (replaceOptions._searchType == FindExtended)	{
				int stringSizeReplace = lstrlen(txt2replace);
				TCHAR *pText2ReplaceExtended = new TCHAR[stringSizeReplace + 1];

				Searching::convertExtendedToString(txt2replace, pText2ReplaceExtended, stringSizeReplace);
				replacedLen = (*_ppEditView)->replaceTarget(pText2ReplaceExtended);
				delete[] pText2ReplaceExtended;
			}
			else
				replacedLen = (*_ppEditView)->replaceTarget(txt2replace);

			(*_ppEditView)->f(SCI_SETSEL, start + replacedLen, start + replacedLen);

			NativeLangSpeaker *pNativeSpeaker = param.getNativeLangSpeaker();
			// Do the next find
			moreMatches = processFindNext(txt2find, &replaceOptions, &status, FINDNEXTTYPE_REPLACENEXT);

			if (status == FSEndReached)	{

				generic_string msg = pNativeSpeaker->getLocalizedStrFromID("find-status-replace-end-reached", L"Replace: Replaced the 1st occurrence from the top. The end of document has been reached.");
				setStatusbarMessage(msg, FSEndReached);
			}
			else if (status == FSTopReached)	{

				generic_string msg = pNativeSpeaker->getLocalizedStrFromID("find-status-replace-top-reached", L"Replace: Replaced the 1st occurrence from the bottom. The begin of document has been reached.");
				setStatusbarMessage(msg, FSTopReached);
			}
			else	{

				generic_string msg;
				if (moreMatches)
					msg = pNativeSpeaker->getLocalizedStrFromID("find-status-replaced-next-found", L"Replace: 1 occurrence was replaced. The next occurence found");
				else
					msg = pNativeSpeaker->getLocalizedStrFromID("find-status-replaced-next-not-found", L"Replace: 1 occurrence was replaced. The next occurence not found");

				setStatusbarMessage(msg, FSMessage);
			}
		}
	}
	else	{
		NativeLangSpeaker *pNativeSpeaker = param.getNativeLangSpeaker();
		generic_string msg = pNativeSpeaker->getLocalizedStrFromID("find-status-replace-not-found", L"Replace: no occurrence was found.");
		setStatusbarMessage(msg, FSNotFound);
	}
	return moreMatches;	
}

void FindReplaceDlg::processReplc1()	{
	FindReplaceDlg::processReplc1(
		(_options._str2Search = getTextFromCombo( ::GetDlgItem(_hSelf, IDFINDWHAT))).c_str(),
		(_options._str4Replace = getTextFromCombo(::GetDlgItem(_hSelf, IDREPLACEWITH))).c_str()
	);
}
void FindReplaceDlg::processReplc1(const TCHAR *txt2find, const TCHAR *txt2replace)	{

	FindOption replaceOptions = *_env;
	replaceOptions._incrementalType = FirstIncremental;

	Sci_CharacterRange currentSelection = (*_ppEditView)->getSelection();

	if (processFindNext(txt2find, &replaceOptions, nullptr, FINDNEXTTYPE_FINDNEXTFORREPLACE))	{
		Sci_CharacterRange nextFind = (*_ppEditView)->getSelection();
		if (nextFind.cpMin == currentSelection.cpMin && nextFind.cpMax == currentSelection.cpMax)	{

			int start = currentSelection.cpMin;
			int replacedLen = 0;
			if (replaceOptions._searchType == FindRegex)
				replacedLen = (*_ppEditView)->replaceTargetRegExMode(txt2replace);
			else if (replaceOptions._searchType == FindExtended)	{
				int stringSizeReplace = lstrlen(txt2replace);
				TCHAR *pText2ReplaceExtended = new TCHAR[stringSizeReplace + 1];

				Searching::convertExtendedToString(txt2replace, pText2ReplaceExtended, stringSizeReplace);
				replacedLen = (*_ppEditView)->replaceTarget(pText2ReplaceExtended);
				delete[] pText2ReplaceExtended;
			}
			else
				replacedLen = (*_ppEditView)->replaceTarget(txt2replace);

			(*_ppEditView)->f(SCI_SETSEL, start + replacedLen, start + replacedLen);
		}
	}
}


int FindReplaceDlg::processRange(ProcessOperation op, FindReplaceInfo & findReplaceInfo, const FindersInfo * pFindersInfo, const FindOption *opt, int colourStyleID, ScintillaEditView *view2Process)	{
	int nbProcessed = 0;
	
	ScintillaEditView *pEditView = view2Process ? view2Process: *_ppEditView;

	if (!isCreated() && not findReplaceInfo._txt2find
	|| op == ProcessReplaceAll && pEditView->getCurrentBuffer()->isReadOnly()
	|| findReplaceInfo._startRange == findReplaceInfo._endRange)
		return 0;

	const FindOption *pOptions = opt? opt : _env;

	LRESULT stringSizeFind = 0, stringSizeReplace = 0;

	TCHAR *pTextFind;
	if (not findReplaceInfo._txt2find)	{

		HWND hFindCombo = ::GetDlgItem(_hSelf, IDFINDWHAT);
		generic_string str2Search = getTextFromCombo(hFindCombo);
		stringSizeFind = str2Search.length();
		pTextFind = new TCHAR[stringSizeFind + 1];
		wcscpy_s(pTextFind, stringSizeFind + 1, str2Search.c_str());
	}
	else	{

		stringSizeFind = lstrlen(findReplaceInfo._txt2find);
		pTextFind = new TCHAR[stringSizeFind + 1];
		wcscpy_s(pTextFind, stringSizeFind + 1, findReplaceInfo._txt2find);
	}

	if (!pTextFind[0])	{ 	delete [] pTextFind;	return 0;	}

	int flags = Searching::buildSearchFlags(pOptions) | SCFIND_REGEXP_SKIPCRLFASONE; 

	TCHAR *pTextReplace = nullptr;
	if (op == ProcessReplaceAll)	{

		if (not findReplaceInfo._txt2replace)	{

			HWND hReplaceCombo = ::GetDlgItem(_hSelf, IDREPLACEWITH);
			generic_string str2Replace = getTextFromCombo(hReplaceCombo);
			stringSizeReplace = str2Replace.length();
			pTextReplace = new TCHAR[stringSizeReplace + 1];
			wcscpy_s(pTextReplace, stringSizeReplace + 1, str2Replace.c_str());
		}
		else	{

			stringSizeReplace = lstrlen(findReplaceInfo._txt2replace);
			pTextReplace = new TCHAR[stringSizeReplace + 1];
			wcscpy_s(pTextReplace, stringSizeReplace + 1, findReplaceInfo._txt2replace);
		}
	//  in batch search allow empty matches but not immediately after previous match, in otherwise ignore empty matches completely.
		flags |= SCFIND_REGEXP_EMPTYMATCH_NOTAFTERMATCH;
	}	

	else if (op == ProcessFindAll)
		flags |= SCFIND_REGEXP_EMPTYMATCH_NOTAFTERMATCH;

	else if (op == ProcessMarkAll && colourStyleID == -1 && _env->_doPurge)
		clearMarks(*_env);	//if marking, check if purging is needed

	if (pOptions->_searchType == FindExtended)	{

		stringSizeFind = Searching::convertExtendedToString(pTextFind, pTextFind, static_cast<int32_t>(stringSizeFind));
		if (op == ProcessReplaceAll)
			stringSizeReplace = Searching::convertExtendedToString(pTextReplace, pTextReplace, static_cast<int32_t>(stringSizeReplace));
	}

	pEditView->f(SCI_SETSEARCHFLAGS, flags);
	
	int targetStart = pEditView->searchInTarget(pTextFind, stringSizeFind, findReplaceInfo._startRange, findReplaceInfo._endRange);
	int targetEnd = static_cast<int32_t>(pEditView->f(SCI_GETTARGETEND));

	switch (op)	{

		case ProcessFindAll:	{
			
			if (targetStart >= 0)	{
			_pFinder->addFileNameTitle(pFindersInfo->_pFileName, pFindersInfo->_unDir);
			while (targetStart >= 0 && targetEnd <= findReplaceInfo._endRange)	{

				int replaceDelta = 0, foundTextLen = targetEnd - targetStart;

				auto lineNumber = pEditView->f(SCI_LINEFROMPOSITION, targetStart);
				int lend = static_cast<int32_t>(pEditView->f(SCI_GETLINEENDPOSITION, lineNumber));
				int lstart = static_cast<int32_t>(pEditView->f(SCI_POSITIONFROMLINE, lineNumber));
				int nbChar = lend - lstart;

				int start_mark = targetStart - lstart;
				int end_mark = targetEnd - lstart;

				// use the static buffer
				TCHAR lineBuf[1024];
				if (nbChar >= 1024 - 2)		lend = lstart + 1020;

				pEditView->getGenericText(lineBuf, 1024, lstart, lend, &start_mark, &end_mark);

				generic_string line = lineBuf;
				line += L"\r\n";
				SearchResultMarking srm;
				srm._start = start_mark;
				srm._end = end_mark;
				_pFinder->add(FoundInfo(targetStart, targetEnd, lineNumber + 1, pFindersInfo->_pFileName), srm, line.c_str());

				++nbProcessed;

				// After the processing of the last string occurence the search loop should be stopped
				// This helps to avoid the endless replacement during the EOL ("$") searching
				if (targetStart + foundTextLen == findReplaceInfo._endRange)
						break;

				findReplaceInfo._startRange = targetStart + foundTextLen + replaceDelta;		//search from result onwards
				findReplaceInfo._endRange += replaceDelta;									//adjust end of range in case of replace

				targetStart = pEditView->searchInTarget(pTextFind, stringSizeFind, findReplaceInfo._startRange, findReplaceInfo._endRange);
				targetEnd = static_cast<int32_t>(pEditView->f(SCI_GETTARGETEND));
			}
			}
			break; 
			}

		case ProcessFindInFinder:	{
			const TCHAR *pFileName = pFindersInfo->_pFileName;
			if (targetStart >= 0)	{
			pFindersInfo->_pDestFinder->addFileNameTitle(pFileName, pFindersInfo->_unDir);

			while (targetStart >= 0 && targetEnd <= findReplaceInfo._endRange)	{

				int replaceDelta = 0, foundTextLen = targetEnd - targetStart;

				if (not pFindersInfo || not pFindersInfo->_pSourceFinder || not pFindersInfo->_pDestFinder)
					break;

				auto lineNumber = pEditView->f(SCI_LINEFROMPOSITION, targetStart);
				int lend = static_cast<int32_t>(pEditView->f(SCI_GETLINEENDPOSITION, lineNumber));
				int lstart = static_cast<int32_t>(pEditView->f(SCI_POSITIONFROMLINE, lineNumber));
				int nbChar = lend - lstart;

				// use the static buffer
				TCHAR lineBuf[1024];

				if (nbChar > 1024 - 3)
					lend = lstart + 1020;

				int start_mark = targetStart - lstart;
				int end_mark = targetEnd - lstart;

				pEditView->getGenericText(lineBuf, 1024, lstart, lend, &start_mark, &end_mark);

				generic_string line = lineBuf;
				line += L"\r\n";
				SearchResultMarking srm;
				srm._start = start_mark;
				srm._end = end_mark;
				
				if (pOptions->_isMatchLineNumber
				&& pFindersInfo->_pSourceFinder->canFind(pFileName, lineNumber + 1))

					pFindersInfo->_pDestFinder->add(FoundInfo(targetStart, targetEnd, lineNumber + 1, pFileName), srm, line.c_str());

				else

					pFindersInfo->_pDestFinder->add(FoundInfo(targetStart, targetEnd, lineNumber + 1, pFileName), srm, line.c_str());
				
				++nbProcessed;

				// After the processing of the last string occurence the search loop should be stopped
				// This helps to avoid the endless replacement during the EOL ("$") searching
				if (targetStart + foundTextLen == findReplaceInfo._endRange)
						break;

				findReplaceInfo._startRange = targetStart + foundTextLen + replaceDelta;		//search from result onwards
				findReplaceInfo._endRange += replaceDelta;									//adjust end of range in case of replace

				targetStart = pEditView->searchInTarget(pTextFind, stringSizeFind, findReplaceInfo._startRange, findReplaceInfo._endRange);
				targetEnd = static_cast<int32_t>(pEditView->f(SCI_GETTARGETEND));

			}
			}
			break;
			}

		case ProcessReplaceAll:	{ 
			while (targetStart >= 0 && targetEnd <= findReplaceInfo._endRange)	{

				int replaceDelta = 0, foundTextLen = targetEnd - targetStart;

				int replacedLength;
				if (pOptions->_searchType == FindRegex)
					replacedLength = pEditView->replaceTargetRegExMode(pTextReplace);
				else
					replacedLength = pEditView->replaceTarget(pTextReplace);

				replaceDelta = replacedLength - foundTextLen;

			++nbProcessed;

			// After the processing of the last string occurence the search loop should be stopped
			// This helps to avoid the endless replacement during the EOL ("$") searching
			if (targetStart + foundTextLen == findReplaceInfo._endRange)
					break;

			findReplaceInfo._startRange = targetStart + foundTextLen + replaceDelta;		//search from result onwards
			findReplaceInfo._endRange += replaceDelta;									//adjust end of range in case of replace

			targetStart = pEditView->searchInTarget(pTextFind, stringSizeFind, findReplaceInfo._startRange, findReplaceInfo._endRange);
			targetEnd = static_cast<int32_t>(pEditView->f(SCI_GETTARGETEND));
			}

		break; 
		}

		case ProcessMarkAll:	{ 
		// In theory, we can't have empty matches for a ProcessMarkAll, but because scintilla 
		// gets upset if we call INDICATORFILLRANGE with a length of 0, we protect against it here.
		// At least in version 2.27, after calling INDICATORFILLRANGE with length 0, further indicators 
		// on the same line would simply not be shown.  This may have been fixed in later version of Scintilla.

			while (targetStart >= 0 && targetEnd <= findReplaceInfo._endRange)	{

				int replaceDelta = 0, foundTextLen = targetEnd - targetStart;

				if (foundTextLen > 0)	{  

					pEditView->f(SCI_SETINDICATORCURRENT, SCE_UNIVERSAL_FOUND_STYLE);
					pEditView->f(SCI_INDICATORFILLRANGE,  targetStart, foundTextLen);
				}

				if (_env->_doMarkLine)	{

					auto lineNumber = pEditView->f(SCI_LINEFROMPOSITION, targetStart);
					auto lineNumberEnd = pEditView->f(SCI_LINEFROMPOSITION, targetEnd - 1);

					for (auto i = lineNumber; i <= lineNumberEnd; ++i)	{

						auto state = pEditView->f(SCI_MARKERGET, i);

						if (!(state & (1 << MARK_BOOKMARK)))
							pEditView->f(SCI_MARKERADD, i, MARK_BOOKMARK);
					}
				}
				++nbProcessed;

				// After the processing of the last string occurence the search loop should be stopped
				// This helps to avoid the endless replacement during the EOL ("$") searching
				if (targetStart + foundTextLen == findReplaceInfo._endRange)
						break;

				findReplaceInfo._startRange = targetStart + foundTextLen + replaceDelta;		//search from result onwards
				findReplaceInfo._endRange += replaceDelta;									//adjust end of range in case of replace

				targetStart = pEditView->searchInTarget(pTextFind, stringSizeFind, findReplaceInfo._startRange, findReplaceInfo._endRange);
				targetEnd = static_cast<int32_t>(pEditView->f(SCI_GETTARGETEND));
			}
				
			break; 
		}
			
		case ProcessMarkAllExt:	{
			// See comment by ProcessMarkAll
			while (targetStart >= 0 && targetEnd <= findReplaceInfo._endRange)	{

				int replaceDelta = 0, foundTextLen = targetEnd - targetStart;
				if (foundTextLen > 0)	{

					pEditView->f(SCI_SETINDICATORCURRENT,  colourStyleID);
					pEditView->f(SCI_INDICATORFILLRANGE,  targetStart, foundTextLen);
				}
				++nbProcessed;

				if (targetStart + foundTextLen == findReplaceInfo._endRange)
						break;

				findReplaceInfo._startRange = targetStart + foundTextLen + replaceDelta;		//search from result onwards
				findReplaceInfo._endRange += replaceDelta;									//adjust end of range in case of replace

				targetStart = pEditView->searchInTarget(pTextFind, stringSizeFind, findReplaceInfo._startRange, findReplaceInfo._endRange);
				targetEnd = static_cast<int32_t>(pEditView->f(SCI_GETTARGETEND));
			}
				
		break;
		}

		case ProcessMarkAll_2:	{
			while (targetStart >= 0 && targetEnd <= findReplaceInfo._endRange)	{

				int replaceDelta = 0, foundTextLen = targetEnd - targetStart;
				if (foundTextLen > 0)	{

					pEditView->f(SCI_SETINDICATORCURRENT,  SCE_UNIVERSAL_FOUND_STYLE_SMART);
					pEditView->f(SCI_INDICATORFILLRANGE,  targetStart, foundTextLen);
				}
				++nbProcessed;

				// After the processing of the last string occurence the search loop should be stopped
				// This helps to avoid the endless replacement during the EOL ("$") searching
				if (targetStart + foundTextLen == findReplaceInfo._endRange)
						break;

				findReplaceInfo._startRange = targetStart + foundTextLen + replaceDelta;		//search from result onwards
				findReplaceInfo._endRange += replaceDelta;									//adjust end of range in case of replace

				targetStart = pEditView->searchInTarget(pTextFind, stringSizeFind, findReplaceInfo._startRange, findReplaceInfo._endRange);
				targetEnd = static_cast<int32_t>(pEditView->f(SCI_GETTARGETEND));

			}
		break;
		}

		case ProcessMarkAll_IncSearch:	{
			while (targetStart >= 0 && targetEnd <= findReplaceInfo._endRange)	{

				int replaceDelta = 0, foundTextLen = targetEnd - targetStart;

				// See comment by ProcessMarkAll
				if (foundTextLen > 0)	{

					pEditView->f(SCI_SETINDICATORCURRENT,  SCE_UNIVERSAL_FOUND_STYLE_INC);
					pEditView->f(SCI_INDICATORFILLRANGE,  targetStart, foundTextLen);
				}
				++nbProcessed;

				if (targetStart + foundTextLen == findReplaceInfo._endRange)
						break;

				findReplaceInfo._startRange = targetStart + foundTextLen + replaceDelta;		//search from result onwards
				findReplaceInfo._endRange += replaceDelta;									//adjust end of range in case of replace

				targetStart = pEditView->searchInTarget(pTextFind, stringSizeFind, findReplaceInfo._startRange, findReplaceInfo._endRange);
				targetEnd = static_cast<int32_t>(pEditView->f(SCI_GETTARGETEND));

		}
		break;
		}

		case ProcessCountAll:
			while (targetStart >= 0 && targetEnd <= findReplaceInfo._endRange)	{

				++nbProcessed;
				if (targetEnd == findReplaceInfo._endRange)		break;

				targetStart = pEditView->searchInTarget(pTextFind, stringSizeFind, targetEnd/* search from result onwards */, findReplaceInfo._endRange);
				targetEnd = static_cast<int32_t>(pEditView->f(SCI_GETTARGETEND));

			}				
		break;

		default:
			delete [] pTextFind;	delete [] pTextReplace;
			return 0;

	}

	delete [] pTextFind;
	delete [] pTextReplace;

	if (nbProcessed > 0)
		if (op == ProcessFindAll)
			_pFinder->addFileHitCount(nbProcessed);

		else if (op == ProcessFindInFinder)

			if (pFindersInfo && pFindersInfo->_pDestFinder)
				pFindersInfo->_pDestFinder->addFileHitCount(nbProcessed);
			else
				_pFinder->addFileHitCount(nbProcessed);;

	return nbProcessed;
}

void FindReplaceDlg::replaceAllInOpenedDocs()	{

	::SendMessage(_hParent, WM_REPLACEALL_INOPENEDDOC, 0, 0);
}

void FindReplaceDlg::findAllIn(int WM_cmd)	{

	if (!_pFinder)	{
		_pFinder = new Finder();
		_pFinder->init(_hInst, _hSelf, _ppEditView);
		_pFinder->setVolatiled(false);
		
		tTbData	data = {0};
		_pFinder->create(&data, false);
		::SendMessage(_hParent, NPPM_MODELESSDIALOG, MODELESSDIALOGREMOVE, reinterpret_cast<LPARAM>(_pFinder->getHSelf()));
		// define the default docking behaviour
		data.uMask = DWS_DF_CONT_BOTTOM | DWS_ICONTAB | DWS_ADDINFO;
		data.hIconTab = (HICON)::LoadImage(_hInst, MAKEINTRESOURCE(IDI_FIND_RESULT_ICON), IMAGE_ICON, 0, 0, LR_LOADMAP3DCOLORS | LR_LOADTRANSPARENT);
		data.pszAddInfo = _findAllResultStr;

		data.pszModuleName = NPP_INTERNAL_FUCTION_STR;

		// the dlgDlg should be the index of funcItem where the current function pointer is
		// in this case is DOCKABLE_DEMO_INDEX
		data.dlgID = 0;
		::SendMessage(_hParent, NPPM_DMMREGASDCKDLG_N, 0, reinterpret_cast<LPARAM>(&data));		
		(*_ppEditView)->focus();	::SetFocus(::GetDlgItem(_hSelf, IDFINDWHAT));
		_pFinder->_scintView.init(_hInst, _pFinder->getHSelf());


		// Subclass the ScintillaEditView for the Finder (Scintilla doesn't notify all key presses)
		originalFinderProc = SetWindowLongPtr(_pFinder->_scintView.getHSelf(), GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(finderProc));

		_pFinder->setFinderReadOnly(true);
		_pFinder->_scintView.f(SCI_SETCODEPAGE, SC_CP_UTF8);
		_pFinder->_scintView.f(SCI_USEPOPUP, FALSE);
		_pFinder->_scintView.f(SCI_SETUNDOCOLLECTION, false);
		_pFinder->_scintView.f(SCI_SETCARETLINEVISIBLE, true);
		_pFinder->_scintView.f(SCI_SETCARETLINEVISIBLEALWAYS, true);
		_pFinder->_scintView.f(SCI_SETCARETWIDTH, 2);
		_pFinder->_scintView.f(SCI_SETYCARETPOLICY, nGUI.caretUZ? 13: 8, nGUI.caretUZ);

		// get the width of FindDlg
		RECT findRect;
		::GetWindowRect(_pFinder->getHSelf(), &findRect);

		// overwrite some default settings
		_pFinder->_scintView.showMargin(ScintillaEditView::_SC_MARGE_SYBOLE, false);
		_pFinder->_scintView.setMakerStyle(FOLDER_STYLE_SIMPLE);
		_pFinder->_scintView.display();
		// _pFinder->display();
		::UpdateWindow(_hParent);
		_pFinder->setFinderStyle();

		// Send the address of _MarkingsStruct to the lexer
		char ptrword[sizeof(void*)*2+1];
		sprintf(ptrword, "%p", &_pFinder->_markingsStruct);
		_pFinder->_scintView.f(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("@MarkingsStruct"), reinterpret_cast<LPARAM>(ptrword));
	}

	::SendMessage(_pFinder->getHSelf(), WM_SIZE, 0, 0);

	_pFinder->_findAllInCurrent = WM_cmd == WM_FINDALL_INCURRENTDOC;

	if (_options._searchType == FindRegex)	{
		const wchar_t *textU=_options._str2Search.c_str();		 
		UINT CP=static_cast<UINT>((*_ppEditView)->f(SCI_GETCODEPAGE));
		int txtALen = WideCharToMultiByte(CP, 0, textU, -1, NULL, 0, NULL, NULL);
		char *textA = new char[txtALen];
		WideCharToMultiByte(CP, 0, textU, -1, textA, txtALen, NULL, NULL);

		(*_ppEditView)->f(SCI_SETSEARCHFLAGS, SCFIND_REGEXP|SCFIND_POSIX);
		if ((*_ppEditView)->f(SCI_SEARCHINTARGET, txtALen, reinterpret_cast<LPARAM>(textA)) == -2)	{
			generic_string msg = param.getNativeLangSpeaker()->getLocalizedStrFromID("find-status-invalid-re", L"Find: Invalid regular expression");
			setStatusbarMessage(msg, FSNotFound);
			return;
		}
	}

	if (::SendMessage(_hParent, WM_cmd, 0, 0))	{

		if (_findAllResult)
			openFinder();
 		else	{
			TCHAR s[64];
			generic_string msg = param.getNativeLangSpeaker()->getLocalizedStrFromID("find-status-cannot-find", L"Find: Can't find the text \"$STR_REPLACE$\" in "),
			ms = stringReplace(msg, L"$STR_REPLACE$", stringReplace(_options._str2Search, L"&", L"&&"));
			if (WM_cmd==WM_FINDALL_INOPENEDDOC)
				wsprintf(s, L"%i opened file(s)", _pFinder->_nbOpenedFiles);
			else if (WM_cmd==WM_FINDALL_INCURRENTDOC)
				wsprintf(s, L"current file");
			else
				wsprintf(s, L"%i file(s) as specified", _fileTot);
			setStatusbarMessage(ms + generic_string(s), FSNotFound);
				
			(*_ppEditView)->focus();	::SetFocus(::GetDlgItem(_hSelf, IDFINDWHAT));
		}
	}
	else // error - search folder doesn't exist
		::SendMessage(_hSelf, WM_NEXTDLGCTL, reinterpret_cast<WPARAM>(::GetDlgItem(_hSelf, IDD_FINDINFILES_DIR_COMBO)), TRUE);
}

Finder * FindReplaceDlg::createFinder()	{
	Finder *pFinder = new Finder();

	pFinder->init(_hInst, _hSelf, _ppEditView);

	tTbData	data = { 0 };
	pFinder->create(&data, false);
	::SendMessage(_hParent, NPPM_MODELESSDIALOG, MODELESSDIALOGREMOVE, reinterpret_cast<WPARAM>(pFinder->getHSelf()));
	// define the default docking behaviour
	data.uMask = DWS_DF_CONT_BOTTOM | DWS_ICONTAB | DWS_ADDINFO;
	data.hIconTab = (HICON)::LoadImage(_hInst, MAKEINTRESOURCE(IDI_FIND_RESULT_ICON), IMAGE_ICON, 0, 0, LR_LOADMAP3DCOLORS | LR_LOADTRANSPARENT);
	data.pszAddInfo = _findAllResultStr;

	data.pszModuleName = NPP_INTERNAL_FUCTION_STR;

	// the dlgDlg should be the index of funcItem where the current function pointer is
	// in this case is DOCKABLE_DEMO_INDEX
	data.dlgID = 0;
	::SendMessage(_hParent, NPPM_DMMREGASDCKDLG_N, 0, reinterpret_cast<LPARAM>(&data));
	pFinder->_scintView.init(_hInst, pFinder->getHSelf());

	// Subclass the ScintillaEditView for the Finder (Scintilla doesn't notify all key presses)
	originalFinderProc = SetWindowLongPtr(pFinder->_scintView.getHSelf(), GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(finderProc));

	pFinder->setFinderReadOnly(true);
	pFinder->_scintView.f(SCI_SETCODEPAGE, SC_CP_UTF8);
	pFinder->_scintView.f(SCI_USEPOPUP, FALSE);
	pFinder->_scintView.f(SCI_SETUNDOCOLLECTION, false);	//dont store any undo information
	pFinder->_scintView.f(SCI_SETCARETLINEVISIBLE, 1);
	pFinder->_scintView.f(SCI_SETCARETLINEVISIBLEALWAYS, true);
	pFinder->_scintView.f(SCI_SETCARETWIDTH, 2);
	pFinder->_scintView.showMargin(ScintillaEditView::_SC_MARGE_FOLDER, true);

	// get the width of Finder
	RECT findRect;
	::GetWindowRect(pFinder->getHSelf(), &findRect);

	// overwrite some default settings
	pFinder->_scintView.showMargin(ScintillaEditView::_SC_MARGE_SYBOLE, false);
	pFinder->_scintView.setMakerStyle(FOLDER_STYLE_SIMPLE);

	pFinder->_scintView.display();
	pFinder->display();
	::UpdateWindow(_hParent);
	
	pFinder->setFinderStyle();

	// Send the address of _MarkingsStruct to the lexer
	char ptrword[sizeof(void*) * 2 + 1];
	sprintf(ptrword, "%p", &pFinder->_markingsStruct);
	pFinder->_scintView.f(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("@MarkingsStruct"), reinterpret_cast<LPARAM>(ptrword));

	_findersOfFinder.push_back(pFinder);

	::SendMessage(pFinder->getHSelf(), WM_SIZE, 0, 0);

	// Show finder
	::SendMessage(_hParent, NPPM_DMMSHOW, 0, reinterpret_cast<LPARAM>(pFinder->getHSelf()));
	pFinder->_scintView.focus();

	return pFinder;
}

bool FindReplaceDlg::removeFinder(Finder *finder2remove)	{

	for (vector<Finder *>::iterator i = _findersOfFinder.begin(); i != _findersOfFinder.end(); ++i)	{

		if (*i == finder2remove)	{

			delete finder2remove;
			_findersOfFinder.erase(i);
			return true;
		}
	}
	return false;
}

void FindReplaceDlg::setSearchText(TCHAR * txt2find)	{

	HWND hCombo = ::GetDlgItem(_hSelf, IDFINDWHAT);
	if (txt2find && txt2find[0])	{

		// We got a valid search string
		::SendMessage(hCombo, CB_SETCURSEL, static_cast<WPARAM>(-1), 0); // remove selection - to allow using down arrow to get to last searched word
		::SetDlgItemText(_hSelf, IDFINDWHAT, txt2find);
	}
	::SendMessage(hCombo, CB_SETEDITSEL, 0, MAKELPARAM(0, -1)); // select all text - fast edit
}

void FindReplaceDlg::enableReplaceFunc(bool isEnable)	{ 

	_currentStatus = REPLACE_DLG;
	int hideOrShow = isEnable?SW_SHOW:SW_HIDE;
	RECT *pClosePos = &_replaceClosePos;//isEnable ? : &_findClosePos;
	RECT *pInSelectionFramePos = isEnable ? &_replaceInSelFramePos : &_countInSelFramePos;
	RECT *pInSectionCheckPos = isEnable ? &_replaceInSelCheckPos : &_countInSelCheckPos;

	enableFindInFilesControls(false);
	enableMarkAllControls(false);
	// replace controls
	::ShowWindow(::GetDlgItem(_hSelf, ID_STATICTEXT_REPLACE),hideOrShow);
	::ShowWindow(::GetDlgItem(_hSelf, IDREPLACE1),hideOrShow);
	::ShowWindow(::GetDlgItem(_hSelf, IDREPLACE_FINDNEXT),hideOrShow);
	::ShowWindow(::GetDlgItem(_hSelf, IDREPLACEWITH),hideOrShow);
	::ShowWindow(::GetDlgItem(_hSelf, IDREPLACEALL),hideOrShow);
	::ShowWindow(::GetDlgItem(_hSelf, IDREPLACEALL_SAVE),hideOrShow);
	::ShowWindow(::GetDlgItem(_hSelf, IDREPLACEINSEL),hideOrShow);
	::ShowWindow(::GetDlgItem(_hSelf, IDC_REPLACE_OPENEDFILES),hideOrShow);
	::ShowWindow(::GetDlgItem(_hSelf, IDC_REPLACEINSELECTION), SW_SHOW);
	::ShowWindow(::GetDlgItem(_hSelf, IDC_IN_SELECTION_CHECK), SW_SHOW);
	::ShowWindow(::GetDlgItem(_hSelf, IDC_2_BUTTONS_MODE), SW_SHOW);
	bool is2ButtonMode = isCheckedOrNot(IDC_2_BUTTONS_MODE);
	::ShowWindow(::GetDlgItem(_hSelf, IDOK), is2ButtonMode ? SW_HIDE : SW_SHOW);
	::ShowWindow(::GetDlgItem(_hSelf, IDC_FINDPREV), !is2ButtonMode ? SW_HIDE : SW_SHOW);
	::ShowWindow(::GetDlgItem(_hSelf, IDC_FINDNEXT), !is2ButtonMode ? SW_HIDE : SW_SHOW);


	// find controls
	::ShowWindow(::GetDlgItem(_hSelf, IDCCOUNTALL),SW_SHOW);
	::ShowWindow(::GetDlgItem(_hSelf, IDC_CLEAR_FINDALL_OPENEDFILES), SW_SHOW);
	::ShowWindow(::GetDlgItem(_hSelf, IDC_FINDALL_OPENEDFILES), SW_SHOW);
	::ShowWindow(::GetDlgItem(_hSelf, IDC_CLEAR_FINDALL_CURRENTFILE),SW_SHOW);
	::ShowWindow(::GetDlgItem(_hSelf, IDC_FINDALL_CURRENTFILE),SW_SHOW);

	gotoCorrectTab();

	::MoveWindow(::GetDlgItem(_hSelf, IDCANCEL), pClosePos->left + _deltaWidth, pClosePos->top, pClosePos->right, pClosePos->bottom, TRUE);
	::MoveWindow(::GetDlgItem(_hSelf, IDC_IN_SELECTION_CHECK), pInSectionCheckPos->left + _deltaWidth, pInSectionCheckPos->top, pInSectionCheckPos->right, pInSectionCheckPos->bottom, TRUE);
	::MoveWindow(::GetDlgItem(_hSelf, IDC_REPLACEINSELECTION), pInSelectionFramePos->left + _deltaWidth, pInSelectionFramePos->top, pInSelectionFramePos->right, pInSelectionFramePos->bottom, TRUE);

	TCHAR label[MAX_PATH];
	_tab.getCurrentTitle(label, MAX_PATH);
	::SetWindowText(_hSelf, label);

	setDefaultButton(IDOK);
}

void FindReplaceDlg::enableFindInFilesControls(bool isEnable)	{

	// Hide Items
	bool HS = isEnable?SW_HIDE:SW_SHOW, SH=!HS;
	::ShowWindow(::GetDlgItem(_hSelf, IDC_UNDO), HS);
	::ShowWindow(::GetDlgItem(_hSelf, IDC_REDO), HS);
	::ShowWindow(::GetDlgItem(_hSelf, IDC_BACKWARDDIRECTION), HS);
	::ShowWindow(::GetDlgItem(_hSelf, IDWRAP), HS);
	::ShowWindow(::GetDlgItem(_hSelf, IDCCOUNTALL), HS);
	::ShowWindow(::GetDlgItem(_hSelf, IDC_CLEAR_FINDALL_OPENEDFILES), HS);
	::ShowWindow(::GetDlgItem(_hSelf, IDC_FINDALL_OPENEDFILES), HS);
	::ShowWindow(::GetDlgItem(_hSelf, IDC_CLEAR_FINDALL_CURRENTFILE), HS);
	::ShowWindow(::GetDlgItem(_hSelf, IDC_FINDALL_CURRENTFILE), HS);

	if (isEnable)	{

		::ShowWindow(::GetDlgItem(_hSelf, IDC_2_BUTTONS_MODE), SW_HIDE);
		::ShowWindow(::GetDlgItem(_hSelf, IDOK), SW_HIDE);
		::ShowWindow(::GetDlgItem(_hSelf, IDC_FINDPREV), SW_HIDE);
		::ShowWindow(::GetDlgItem(_hSelf, IDC_FINDNEXT), SW_HIDE);
	::ShowWindow(::GetDlgItem(_hSelf, IDC_FIND_REPLACE_SWAP), SW_SHOW);
	::ShowWindow(::GetDlgItem(_hSelf, IDC_FIND_REPLACE_COPY), SW_SHOW);
	::ShowWindow(::GetDlgItem(_hSelf, IDSWAP_S), SW_SHOW);
	::ShowWindow(::GetDlgItem(_hSelf, IDCOPY_S), SW_SHOW);
	}
	else	{

		::ShowWindow(::GetDlgItem(_hSelf, IDC_2_BUTTONS_MODE), SW_SHOW);
		bool is2ButtonMode = isCheckedOrNot(IDC_2_BUTTONS_MODE);

		::ShowWindow(::GetDlgItem(_hSelf, IDOK), is2ButtonMode ? SW_HIDE : SW_SHOW);

		::ShowWindow(::GetDlgItem(_hSelf, IDC_FINDPREV), !is2ButtonMode ? SW_HIDE : SW_SHOW);
		::ShowWindow(::GetDlgItem(_hSelf, IDC_FINDNEXT), !is2ButtonMode ? SW_HIDE : SW_SHOW);
	}

	::ShowWindow(::GetDlgItem(_hSelf, IDC_MARKLINE_CHECK), HS);
	::ShowWindow(::GetDlgItem(_hSelf, IDC_PURGE_CHECK), HS);
	::ShowWindow(::GetDlgItem(_hSelf, IDC_IN_SELECTION_CHECK), HS);
	::ShowWindow(::GetDlgItem(_hSelf, IDC_CLEAR_ALL), HS);
	::ShowWindow(::GetDlgItem(_hSelf, IDCMARKALL), HS);
	
	::ShowWindow(::GetDlgItem(_hSelf, IDREPLACE1), HS);
	::ShowWindow(::GetDlgItem(_hSelf, IDREPLACE_FINDNEXT), HS);
	::ShowWindow(::GetDlgItem(_hSelf, IDC_REPLACEINSELECTION), HS);
	::ShowWindow(::GetDlgItem(_hSelf, IDREPLACEALL), HS);
	::ShowWindow(::GetDlgItem(_hSelf, IDREPLACEALL_SAVE), HS);
	::ShowWindow(::GetDlgItem(_hSelf, IDC_REPLACE_OPENEDFILES), HS);

	// Show Items
	if (isEnable)	{

		::ShowWindow(::GetDlgItem(_hSelf, ID_STATICTEXT_REPLACE), SW_SHOW);
		::ShowWindow(::GetDlgItem(_hSelf, IDREPLACEWITH), SW_SHOW);
	}
	::ShowWindow(::GetDlgItem(_hSelf, IDD_FINDINFILES_REPLACEINFILES), SH);
	::ShowWindow(::GetDlgItem(_hSelf, IDD_FINDINFILES_FILTERS_STATIC), SH);
	::ShowWindow(::GetDlgItem(_hSelf, IDD_FINDINFILES_FILTERS_COMBO), SH);
	::ShowWindow(::GetDlgItem(_hSelf, IDD_FINDINFILES_DIR_STATIC), SH);
	::ShowWindow(::GetDlgItem(_hSelf, IDD_FINDINFILES_DIR_COMBO), SH);
	::ShowWindow(::GetDlgItem(_hSelf, IDD_FINDINFILES_BROWSE_BUTTON), SH);
	::ShowWindow(::GetDlgItem(_hSelf, IDD_FINDINFILES_FIND_BUTTON), SH);
	::ShowWindow(::GetDlgItem(_hSelf, IDD_FINDINFILES_CLEAR_FIND), SH);
	::ShowWindow(::GetDlgItem(_hSelf, IDD_FINDINFILES_GOBACK_BUTTON), SH);
	::ShowWindow(::GetDlgItem(_hSelf, IDD_FINDINFILES_RECURSIVE_CHECK), SH);
	::ShowWindow(::GetDlgItem(_hSelf, IDD_FINDINFILES_INHIDDENDIR_CHECK), SH);
	::ShowWindow(::GetDlgItem(_hSelf, IDD_FINDINFILES_FOLDERFOLLOWSDOC_CHECK), SH);
}

void FindReplaceDlg::getPatterns(vector<generic_string> & patternVect)	{

	cutString(_env->_filters.c_str(), patternVect);
}

void FindReplaceDlg::saveInMacro(size_t cmd, int cmdType)	{

	int booleans = 0;
	::SendMessage(_hParent, WM_FRSAVE_INT, IDC_FRCOMMAND_INIT, 0);
	::SendMessage(_hParent, WM_FRSAVE_STR, IDFINDWHAT,  reinterpret_cast<LPARAM>(cmd == IDC_CLEAR_ALL ? L"": _options._str2Search.c_str()));
	booleans |= _options._isWholeWord?IDF_WHOLEWORD:0;
	booleans |= _options._isMatchCase?IDF_MATCHCASE:0;
	booleans |= _options._dotMatchesNewline?IDF_REDOTMATCHNL:0;

	::SendMessage(_hParent, WM_FRSAVE_INT, IDNORMAL, _options._searchType);
	if (cmd == IDCMARKALL)	{

		booleans |= _options._doPurge?IDF_PURGE_CHECK:0;
		booleans |= _options._doMarkLine?IDF_MARKLINE_CHECK:0;
	}
	if (cmdType & FR_OP_REPLACE)
		::SendMessage(_hParent, WM_FRSAVE_STR, IDREPLACEWITH, reinterpret_cast<LPARAM>(_options._str4Replace.c_str()));
	if (cmdType & FR_OP_FIF)	{

		::SendMessage(_hParent, WM_FRSAVE_STR, IDD_FINDINFILES_DIR_COMBO, reinterpret_cast<LPARAM>(_options._directory.c_str()));
		
		::SendMessage(_hParent, WM_FRSAVE_STR, IDD_FINDINFILES_FILTERS_COMBO, reinterpret_cast<LPARAM>(_options._filters.c_str()));
		booleans |= _options._isRecursive?IDF_FINDINFILES_RECURSIVE_CHECK:0;
		booleans |= _options._isInHiddenDir?IDF_FINDINFILES_INHIDDENDIR_CHECK:0;
	}
	else if (!(cmdType & FR_OP_GLOBAL))	{

		booleans |= _options._isInSelection?IDF_IN_SELECTION_CHECK:0;
		booleans |= _options._isWrapAround?IDF_WRAP:0;
		booleans |= _options._whichDirection?IDF_WHICH_DIRECTION:0;
	}
	if (cmd == IDC_CLEAR_ALL)	{

		booleans = _options._doMarkLine ? IDF_MARKLINE_CHECK : 0;
		booleans |= _options._isInSelection ? IDF_IN_SELECTION_CHECK : 0;
	}
	::SendMessage(_hParent, WM_FRSAVE_INT, IDC_FRCOMMAND_BOOLEANS, booleans);
	::SendMessage(_hParent, WM_FRSAVE_INT, IDC_FRCOMMAND_EXEC, cmd);
}

void FindReplaceDlg::setStatusbarMessage(const generic_string & msg, FindStatus status) {
	if (status == FSNotFound)	{

		::MessageBeep(0xFFFFFFFF);
		FLASHWINFO flashInfo;
		flashInfo.cbSize = sizeof(FLASHWINFO);
		flashInfo.hwnd = isVisible()?_hSelf:GetParent(_hSelf);
		flashInfo.uCount = 2;
		flashInfo.dwTimeout = 41;
		flashInfo.dwFlags = FLASHW_ALL;
		FlashWindowEx(&flashInfo);
	}
	else if ((status == FSTopReached || status == FSEndReached) &&!isVisible())	{

		FLASHWINFO flashInfo;
		flashInfo.cbSize = sizeof(FLASHWINFO);
		flashInfo.hwnd = GetParent(_hSelf);
		flashInfo.uCount = 2;
		flashInfo.dwTimeout = 33;
		flashInfo.dwFlags = FLASHW_ALL;
		FlashWindowEx(&flashInfo);
	}

	if (isVisible())	{

		_statusbarFindStatus = status;
		_statusBar.setOwnerDrawText(msg.c_str());
	}
}

void FindReplaceDlg::execSavedCommand(int cmd, uptr_t intValue, const generic_string& stringValue)	{

	try
	{
		switch (cmd)	{

			case IDC_FRCOMMAND_INIT:
				_env = new FindOption;
				break;
			case IDFINDWHAT:
				_env->_str2Search = stringValue;
				break;
			case IDC_FRCOMMAND_BOOLEANS:
				_env->_isWholeWord = ((intValue & IDF_WHOLEWORD) > 0);
				_env->_isMatchCase = ((intValue & IDF_MATCHCASE) > 0);
				_env->_isRecursive = ((intValue & IDF_FINDINFILES_RECURSIVE_CHECK) > 0);
				_env->_isInHiddenDir = ((intValue & IDF_FINDINFILES_INHIDDENDIR_CHECK) > 0);
				_env->_doPurge = ((intValue & IDF_PURGE_CHECK) > 0);
				_env->_doMarkLine = ((intValue & IDF_MARKLINE_CHECK) > 0);
				_env->_isInSelection = ((intValue & IDF_IN_SELECTION_CHECK) > 0);
				_env->_isWrapAround = ((intValue & IDF_WRAP) > 0);
				_env->_whichDirection = ((intValue & IDF_WHICH_DIRECTION) > 0);
				_env->_dotMatchesNewline = ((intValue & IDF_REDOTMATCHNL) > 0);
				break;
			case IDNORMAL:
				_env->_searchType = static_cast<SearchType>(intValue);
				break;
			case IDREPLACEWITH:
				_env->_str4Replace = stringValue;
				break;
			case IDD_FINDINFILES_DIR_COMBO:
				_env->_directory = stringValue;
				break;
			case IDD_FINDINFILES_FILTERS_COMBO:
				_env->_filters = stringValue;
				break;
			case IDC_FRCOMMAND_EXEC:	{

				switch (intValue)	{

					case IDOK:
						param._isFindReplacing = true;
						processFindNext(_env->_str2Search.c_str());
						param._isFindReplacing = false;
						break;

					case IDC_FINDNEXT:	{

						param._isFindReplacing = true;
						_options._whichDirection = DIR_DOWN;
						processFindNext(_env->_str2Search.c_str());
						param._isFindReplacing = false;
					}
					break;
					
					case IDC_FINDPREV:	{

						param._isFindReplacing = true;
						_env->_whichDirection = DIR_UP;
						processFindNext(_env->_str2Search.c_str());
						param._isFindReplacing = false;
					}
					break;

					case IDREPLACE1:
					case IDREPLACE_FINDNEXT:
						param._isFindReplacing = true;
						processReplace(_env->_str2Search.c_str(), _env->_str4Replace.c_str(), _env);
						param._isFindReplacing = false;
						break;
					case IDC_FINDALL_OPENEDFILES:
						param._isFindReplacing = true;
						findAllIn(WM_FINDALL_INOPENEDDOC);
						param._isFindReplacing = false;
						break;
					case IDC_FINDALL_CURRENTFILE:
						param._isFindReplacing = true;
						findAllIn(WM_FINDALL_INCURRENTDOC);
						param._isFindReplacing = false;
						break;
					case IDC_REPLACE_OPENEDFILES:
						param._isFindReplacing = true;
						replaceAllInOpenedDocs();
						param._isFindReplacing = false;
						break;
					case IDD_FINDINFILES_FIND_BUTTON:
						param._isFindReplacing = true;
						findAllIn(WM_FINDINFILES);
						param._isFindReplacing = false;
						break;

					case IDD_FINDINFILES_REPLACEINFILES:	{

						generic_string msg = L"Are you sure you want to replace all occurrences in :\r";
						msg += _env->_directory;
						msg += L"\rfor file type : ";
						msg += (_env->_filters[0]) ? _env->_filters : L"*.*";

						if (::MessageBox(_hParent, msg.c_str(), L"Are you sure?", MB_OKCANCEL | MB_DEFBUTTON2) == IDOK)	{

							param._isFindReplacing = true;
							::SendMessage(_hParent, WM_REPLACEINFILES, 0, 0);
							param._isFindReplacing = false;
						}
						break;
					}
					case IDREPLACEALL:	{

						param._isFindReplacing = true;
						(*_ppEditView)->f(SCI_BEGINUNDOACTION);
						int nbReplaced = processAll(ProcessReplaceAll);
						(*_ppEditView)->f(SCI_ENDUNDOACTION);
						param._isFindReplacing = false;

						generic_string result;
						NativeLangSpeaker *pNativeSpeaker = param.getNativeLangSpeaker();
						if (nbReplaced < 0)	{

							result = pNativeSpeaker->getLocalizedStrFromID("find-status-replaceall-re-malformed", L"Replace All: The regular expression is malformed.");
						}
						else	{

							/* if (nbReplaced == 1)
							{
								result = pNativeSpeaker->getLocalizedStrFromID("find-status-replaceall-1-replaced", L"Replace All: 1 occurrence was replaced.");
							}
							else	{
 */
								result = pNativeSpeaker->getLocalizedStrFromID("find-status-replaceall-nb-replaced", L"Replace All: replaced $INT_REPLACE$ occurrence(s).");
								result = stringReplace(result, L"$INT_REPLACE$", std::to_wstring(nbReplaced));
							//}
						}

						setStatusbarMessage(result, FSMessage);
						break;
					}

					case IDCCOUNTALL:	{

						int nbCounted = processAll(ProcessCountAll);
						generic_string result;
						NativeLangSpeaker *pNativeSpeaker = param.getNativeLangSpeaker();
						if (nbCounted < 0)	{

							result = pNativeSpeaker->getLocalizedStrFromID("find-status-count-re-malformed", L"Count: The regular expression to search is malformed.");
						}
						else	{

							if (nbCounted == 1)	{

								result = pNativeSpeaker->getLocalizedStrFromID("find-status-count-1-match", L"Count: 1 match.");
							}
							else	{

								result = pNativeSpeaker->getLocalizedStrFromID("find-status-count-nb-matches", L"Count: $INT_REPLACE$ matches.");
								result = stringReplace(result, L"$INT_REPLACE$", std::to_wstring(nbCounted));
							}
						}
						setStatusbarMessage(result, FSMessage);
						break;
					}

					case IDCMARKALL:	{

						param._isFindReplacing = true;
						int nbMarked = processAll(ProcessMarkAll);
						param._isFindReplacing = false;
						generic_string result;

						NativeLangSpeaker *pNativeSpeaker = param.getNativeLangSpeaker();
						if (nbMarked < 0)	{

							result = pNativeSpeaker->getLocalizedStrFromID("find-status-mark-re-malformed", L"Mark: The regular expression to search is malformed.");
						}
						else	{

							if (nbMarked == 1)	{

								result = pNativeSpeaker->getLocalizedStrFromID("find-status-mark-1-match", L"Mark: 1 match.");
							}
							else	{

								result = pNativeSpeaker->getLocalizedStrFromID("find-status-mark-nb-matches", L"Mark: $INT_REPLACE$ matches.");
								result = stringReplace(result, L"$INT_REPLACE$", std::to_wstring(nbMarked));
							}
						}

						setStatusbarMessage(result, FSMessage);
						break;
					}

					case IDC_CLEAR_ALL:	{

						clearMarks(*_env);
						break;
					}

					default:
						throw std::runtime_error("Internal error: unknown saved command!");
				}

				delete _env;
				_env = &_options;
				break;
			}
			default:
				throw std::runtime_error("Internal error: unknown SnR command!");
		}
	}
	catch (const std::runtime_error& err)	{

		MessageBoxA(NULL, err.what(), "Play Macro Exception", MB_OK);
	}
}

int FindReplaceDlg::markAll(const TCHAR *txt2find, int styleID, bool isWholeWordSelected)	{

	FindOption opt;
	opt._isMatchCase = _options._isMatchCase;
	// if whole word is selected for being colorized, isWholeWord option in Find/Replace dialog will be checked
	// otherwise this option is false, because user may want to find the words contain the parts to search 
	opt._isWholeWord = isWholeWordSelected?_options._isWholeWord:false;
	opt._str2Search = txt2find;

	int nbFound = processAll(ProcessMarkAllExt, &opt, true, NULL, styleID);
	return nbFound;
}

int FindReplaceDlg::markAllInc(const FindOption *opt)	{

	int nbFound = processAll(ProcessMarkAll_IncSearch, opt,  true);
	return nbFound;
}

void FindReplaceDlg::enableMarkAllControls(bool isEnable)	{

	bool SH = isEnable?SW_SHOW:SW_HIDE, HS=!SH;
	::ShowWindow(::GetDlgItem(_hSelf, IDCMARKALL),SH);
	::ShowWindow(::GetDlgItem(_hSelf, IDC_MARKLINE_CHECK),SH);
	::ShowWindow(::GetDlgItem(_hSelf, IDC_PURGE_CHECK),SH);
	::ShowWindow(::GetDlgItem(_hSelf, IDC_CLEAR_ALL),SH);
	::ShowWindow(::GetDlgItem(_hSelf, IDC_IN_SELECTION_CHECK), SH);
	::ShowWindow(::GetDlgItem(_hSelf, IDC_FIND_REPLACE_SWAP), HS);
	::ShowWindow(::GetDlgItem(_hSelf, IDC_FIND_REPLACE_COPY), HS);
	::ShowWindow(::GetDlgItem(_hSelf, IDSWAP_S), HS);
	::ShowWindow(::GetDlgItem(_hSelf, IDCOPY_S), HS);
}

void FindReplaceDlg::clearMarks(const FindOption& opt)	{

	if (opt._isInSelection)	{
		Sci_CharacterRange cr = (*_ppEditView)->getSelection();
		int startPosition = cr.cpMin;
		int endPosition = cr.cpMax;

		(*_ppEditView)->f(SCI_SETINDICATORCURRENT, SCE_UNIVERSAL_FOUND_STYLE);
		(*_ppEditView)->f(SCI_INDICATORCLEARRANGE, startPosition, endPosition);

		if (opt._doMarkLine)	{

			auto lineNumber = (*_ppEditView)->f(SCI_LINEFROMPOSITION, startPosition);
			auto lineNumberEnd = (*_ppEditView)->f(SCI_LINEFROMPOSITION, endPosition - 1);

			for (auto i = lineNumber; i <= lineNumberEnd; ++i)	{

				auto state = (*_ppEditView)->f(SCI_MARKERGET, i);

				if (state & (1 << MARK_BOOKMARK))
					(*_ppEditView)->f(SCI_MARKERDELETE, i, MARK_BOOKMARK);
			}
		}
	}
	else	{

		(*_ppEditView)->clearIndicator(SCE_UNIVERSAL_FOUND_STYLE);
		if (opt._doMarkLine)	{

			(*_ppEditView)->f(SCI_MARKERDELETEALL, MARK_BOOKMARK);
		}
	}

	setStatusbarMessage(L"", FSNoMessage);
}

void FindReplaceDlg::setFindInFilesDirFilter(const TCHAR *dir, const TCHAR *filters)	{

	if (dir)	{

		_options._directory = dir;
		::SetDlgItemText(_hSelf, IDD_FINDINFILES_DIR_COMBO, dir);
	}
	if (filters)	{

		_options._filters = filters;
		::SetDlgItemText(_hSelf, IDD_FINDINFILES_FILTERS_COMBO, filters);
	}
}

void FindReplaceDlg::initOptionsFromDlg()	{

	_options._isWholeWord = isCheckedOrNot(IDWHOLEWORD);
	_options._isMatchCase = isCheckedOrNot(IDMATCHCASE);
	_options._searchType = isCheckedOrNot(IDREGEXP)?FindRegex:isCheckedOrNot(IDEXTENDED)?FindExtended:FindNormal;
	_options._isWrapAround = isCheckedOrNot(IDWRAP);
	_options._isInSelection = isCheckedOrNot(IDC_IN_SELECTION_CHECK);

	_options._dotMatchesNewline = isCheckedOrNot(IDREDOTMATCHNL);
	_options._doPurge = isCheckedOrNot(IDC_PURGE_CHECK);
	_options._doMarkLine = isCheckedOrNot(IDC_MARKLINE_CHECK);

	_options._whichDirection = isCheckedOrNot(IDC_BACKWARDDIRECTION) ? DIR_UP : DIR_DOWN;
	
	_options._isRecursive = isCheckedOrNot(IDD_FINDINFILES_RECURSIVE_CHECK);
	_options._isInHiddenDir = isCheckedOrNot(IDD_FINDINFILES_INHIDDENDIR_CHECK);
}

void FindInFinderDlg::doDialog(Finder *launcher, bool isRTL)	{

	_pFinder2Search = launcher;
	if (isRTL)	{

		DLGTEMPLATE *pMyDlgTemplate = NULL;
		HGLOBAL hMyDlgTemplate = makeRTLResource(IDD_FINDINFINDER_DLG, &pMyDlgTemplate);
		::DialogBoxIndirectParam(_hInst, pMyDlgTemplate, _hParent, dlgProc, reinterpret_cast<LPARAM>(this));
		::GlobalFree(hMyDlgTemplate);
	}
	else
		::DialogBoxParam(_hInst, MAKEINTRESOURCE(IDD_FINDINFINDER_DLG), _hParent, dlgProc, reinterpret_cast<LPARAM>(this));
	
}

void FindReplaceDlg::doDialog(DIALOG_TYPE whichType, bool isRTL, bool toShow)	{

	if (!isCreated())
		create(IDD_FIND_REPLACE_DLG, _isRTL = isRTL);
	setStatusbarMessage(L"", FSNoMessage);

	if (whichType == FINDINFILES_DLG)
		enableFindInFilesFunc();
	else if (whichType == MARK_DLG)
		enableMarkFunc();
	else
		enableReplaceFunc(whichType == REPLACE_DLG);

	::SetFocus(::GetDlgItem(_hSelf, IDFINDWHAT));
	display(toShow);
}

LRESULT FAR PASCAL FindReplaceDlg::finderProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)	{

	if (message == WM_KEYDOWN && (wParam == VK_DELETE || wParam == VK_RETURN))	{

		ScintillaEditView *pScint = (ScintillaEditView *)(::GetWindowLongPtr(hwnd, GWLP_USERDATA));
		Finder *pFinder = (Finder *)(::GetWindowLongPtr(pScint->getHParent(), GWLP_USERDATA));
		if (wParam == VK_RETURN)
			pFinder->gotoFoundLine();
		else // VK_DELETE
			pFinder->deleteResult();
		return 0;
	}
	else
		// Call default (original) window procedure
		return CallWindowProc((WNDPROC) originalFinderProc, hwnd, message, wParam, lParam);
}

void FindReplaceDlg::enableFindInFilesFunc()	{

	enableFindInFilesControls();
	_currentStatus = FINDINFILES_DLG;
	gotoCorrectTab();
	// ::MoveWindow(::GetDlgItem(_hSelf, IDCANCEL), _findInFilesClosePos.left + _deltaWidth, _findInFilesClosePos.top, _findInFilesClosePos.right, _findInFilesClosePos.bottom, TRUE);
	TCHAR label[MAX_PATH];
	_tab.getCurrentTitle(label, MAX_PATH);
	::SetWindowText(_hSelf, label);
	setDefaultButton(IDD_FINDINFILES_FIND_BUTTON);
}

void FindReplaceDlg::enableMarkFunc()	{

	enableFindInFilesControls(false);
	enableMarkAllControls(true);

	// find controls to hide
	::ShowWindow(::GetDlgItem(_hSelf, IDC_UNDO), SW_HIDE);
	::ShowWindow(::GetDlgItem(_hSelf, IDC_REDO), SW_HIDE);
	::ShowWindow(::GetDlgItem(_hSelf, IDC_CLEAR_FINDALL_OPENEDFILES), SW_HIDE);
	::ShowWindow(::GetDlgItem(_hSelf, IDC_FINDALL_OPENEDFILES), SW_HIDE);
	::ShowWindow(::GetDlgItem(_hSelf, IDC_CLEAR_FINDALL_CURRENTFILE),SW_HIDE);
	::ShowWindow(::GetDlgItem(_hSelf, IDC_FINDALL_CURRENTFILE),SW_HIDE);
	::ShowWindow(::GetDlgItem(_hSelf, IDOK),SW_HIDE);
	::ShowWindow(::GetDlgItem(_hSelf, IDC_2_BUTTONS_MODE), SW_HIDE);
	::ShowWindow(::GetDlgItem(_hSelf, IDC_FINDPREV), SW_HIDE);
	::ShowWindow(::GetDlgItem(_hSelf, IDC_FINDNEXT), SW_HIDE);

	// Replace controls to hide
	::ShowWindow(::GetDlgItem(_hSelf, ID_STATICTEXT_REPLACE),SW_HIDE);
	::ShowWindow(::GetDlgItem(_hSelf, IDREPLACE1),SW_HIDE);
	::ShowWindow(::GetDlgItem(_hSelf, IDREPLACE_FINDNEXT),SW_HIDE);
	::ShowWindow(::GetDlgItem(_hSelf, IDREPLACEWITH),SW_HIDE);
	::ShowWindow(::GetDlgItem(_hSelf, IDREPLACEALL),SW_HIDE);
	::ShowWindow(::GetDlgItem(_hSelf, IDREPLACEALL_SAVE),SW_HIDE);
	::ShowWindow(::GetDlgItem(_hSelf, IDREPLACEINSEL),SW_HIDE);
	::ShowWindow(::GetDlgItem(_hSelf, IDC_REPLACE_OPENEDFILES),SW_HIDE);
	::ShowWindow(::GetDlgItem(_hSelf, IDC_REPLACEINSELECTION),SW_HIDE);

	_currentStatus = MARK_DLG;
	gotoCorrectTab();
	// ::MoveWindow(::GetDlgItem(_hSelf, IDCANCEL), _findInFilesClosePos.left + _deltaWidth, _findInFilesClosePos.top, _findInFilesClosePos.right, _findInFilesClosePos.bottom, TRUE);
	::MoveWindow(::GetDlgItem(_hSelf, IDC_IN_SELECTION_CHECK), _replaceInSelCheckPos.left + _deltaWidth, _replaceInSelCheckPos.top, _replaceInSelCheckPos.right, _replaceInSelCheckPos.bottom, TRUE);
	::MoveWindow(::GetDlgItem(_hSelf, IDC_REPLACEINSELECTION), _replaceInSelFramePos.left + _deltaWidth, _replaceInSelFramePos.top, _replaceInSelFramePos.right, _replaceInSelFramePos.bottom, TRUE);

	TCHAR label[MAX_PATH];
	_tab.getCurrentTitle(label, MAX_PATH);
	::SetWindowText(_hSelf, label);
	setDefaultButton(IDCMARKALL);
}
void FindReplaceDlg::combo2ExtendedMode(int comboID)	{

	HWND hFindCombo = ::GetDlgItem(_hSelf, comboID);
	if (!hFindCombo) return;
	
	generic_string str2transform = getTextFromCombo(hFindCombo);
		
	// Count the number of character '\n' and '\r'
	size_t nbEOL = 0;
	size_t str2transformLen = lstrlen(str2transform.c_str());
	for (size_t i = 0 ; i < str2transformLen ; ++i)	{

		if (str2transform[i] == '\r' || str2transform[i] == '\n')
				++nbEOL;
	}

	if (nbEOL)	{

		TCHAR * newBuffer = new TCHAR[str2transformLen + nbEOL*2 + 1];
		int j = 0;
		for (size_t i = 0 ; i < str2transformLen ; ++i)	{

				if (str2transform[i] == '\r')	{

					newBuffer[j++] = '\\';
					newBuffer[j++] = 'r';
				}
				else if (str2transform[i] == '\n')	{

					newBuffer[j++] = '\\';
					newBuffer[j++] = 'n';
				}
				else	{

					newBuffer[j++] = str2transform[i];
				}
		}
		newBuffer[j++] = '\0';
		setSearchText(newBuffer);

		_options._searchType = FindExtended;
		::SendDlgItemMessage(_hSelf, IDNORMAL, BM_SETCHECK, FALSE, 0);
		::SendDlgItemMessage(_hSelf, IDEXTENDED, BM_SETCHECK, TRUE, 0);
		::SendDlgItemMessage(_hSelf, IDREGEXP, BM_SETCHECK, FALSE, 0);

		delete [] newBuffer;
	}
}

void FindReplaceDlg::drawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)	{

	//printStr(L"OK");
	COLORREF fgColor;
	PCTSTR ptStr =(PCTSTR)lpDrawItemStruct->itemData;

	if (_statusbarFindStatus == FSNotFound)
		fgColor = RGB(0xFF, 79, 70); // red

	else if (_statusbarFindStatus == FSTopReached || _statusbarFindStatus == FSEndReached)
		fgColor = RGB(99, 215, 0); // green

	else if (_statusbarFindStatus == FSMessage)
		fgColor = RGB(0xFF,0xFF,0xFF);

	else	{

		fgColor = RGB(0xCF,0xC1,0xDF);
		ptStr = L" Hit ALT<underlined character> to set/do a job, or TAB to get at one (SHIFT-TAB backwardly) then SPACE";
	}
	::SetTextColor(lpDrawItemStruct->hDC, fgColor);
	::SetBkColor(lpDrawItemStruct->hDC, RGB(0,0,0));//getCtrlBgColor(_statusBar.getHSelf())
	RECT rect;
	_statusBar.getClientRect(rect);
	::DrawText(lpDrawItemStruct->hDC, ptStr, lstrlen(ptStr), &rect, DT_SINGLELINE | DT_VCENTER | DT_LEFT);
}

void FindIncrementDlg::destroy()	{

	if (_pRebar)	{ 

		_pRebar->removeBand(_rbBand.wID);
		_pRebar = NULL;
	}
}

void FindIncrementDlg::display(bool toShow) const
{
	if (!_pRebar)	{

		Window::display(toShow);
		return;
	}
	if (toShow)	{

		::SetFocus(::GetDlgItem(_hSelf, IDC_INCFINDTEXT));
		// select the whole find editor text
		::SendDlgItemMessage(_hSelf, IDC_INCFINDTEXT, EM_SETSEL, 0, -1);
	}
	_pRebar->setIDVisible(_rbBand.wID, toShow);
}

INT_PTR CALLBACK FindIncrementDlg::run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam)	{

	switch (message)	{

		// Make edit field red if not found
		case WM_CTLCOLOREDIT :	{

			// if the text not found modify the background color of the editor
			static HBRUSH hBrushBackground = CreateSolidBrush(BCKGRD_COLOR);
			if (FSNotFound != getFindStatus())
				return FALSE; // text found, use the default color

			// text not found
			SetTextColor((HDC)wParam, TXT_COLOR);
			SetBkColor((HDC)wParam, BCKGRD_COLOR);
			return (LRESULT)hBrushBackground;
		}

		case WM_COMMAND :	{ 

			bool updateSearch = false;
			bool forward = true;
			bool advance = false;
			bool updateHiLight = false;
			bool updateCase = false;

			switch (LOWORD(wParam))	{

				case IDCANCEL :
					(*(_pFRDlg->_ppEditView))->clearIndicator(SCE_UNIVERSAL_FOUND_STYLE_INC);
					(*(_pFRDlg->_ppEditView))->focus();
					display(false);
					return TRUE;

				case IDM_SEARCH_FINDINCREMENT:	// Accel table: Start incremental search
					// if focus is on a some other control, return it to the edit field
					if (::GetFocus() != ::GetDlgItem(_hSelf, IDC_INCFINDTEXT))	{

						HWND hFindTxt = ::GetDlgItem(_hSelf, IDC_INCFINDTEXT);
						::PostMessage(_hSelf, WM_NEXTDLGCTL, reinterpret_cast<WPARAM>(hFindTxt), TRUE);
						return TRUE;
					}
					// otherwise, repeat the search
				case IDM_SEARCH_FINDNEXT:		// Accel table: find next
				case IDM_SEARCH_FINDPREV:		// Accel table: find prev
				case IDC_INCFINDPREVOK:
				case IDC_INCFINDNXTOK:
				case IDOK:
					updateSearch = true;
					advance = true;
					forward = (LOWORD(wParam) == IDC_INCFINDNXTOK) ||
						(LOWORD(wParam) == IDM_SEARCH_FINDNEXT) ||
						(LOWORD(wParam) == IDM_SEARCH_FINDINCREMENT) ||
						((LOWORD(wParam) == IDOK) && !(GetKeyState(VK_SHIFT) & SHIFTED));
					break;

				case IDC_INCFINDMATCHCASE:
					updateSearch = true;
					updateCase = true;
					updateHiLight = true;
					break;

				case IDC_INCFINDHILITEALL:
					updateHiLight = true;
					break;

				case IDC_INCFINDTEXT:
					if (HIWORD(wParam) == EN_CHANGE)	{

						updateSearch = true;
						updateHiLight = isCheckedOrNot(IDC_INCFINDHILITEALL);
						updateCase = isCheckedOrNot(IDC_INCFINDMATCHCASE);
						break;
					}
					// treat other edit notifications as unhandled
				default:
					return DefWindowProc(getHSelf(), message, wParam, lParam);
			}
			FindOption fo;
			fo._isWholeWord = false;
			fo._incrementalType = advance ? NextIncremental : FirstIncremental;
			fo._whichDirection = forward ? DIR_DOWN : DIR_UP;
			fo._isMatchCase = (BST_CHECKED == ::SendDlgItemMessage(_hSelf, IDC_INCFINDMATCHCASE, BM_GETCHECK, 0, 0));

			generic_string str2Search = getTextFromCombo(::GetDlgItem(_hSelf, IDC_INCFINDTEXT));
			if (updateSearch)	{

				FindStatus findStatus = FSFound;
				bool isFound = _pFRDlg->processFindNext(str2Search.c_str(), &fo, &findStatus);
				
				fo._str2Search = str2Search;
				int nbCounted = _pFRDlg->processAll(ProcessCountAll, &fo);
				setFindStatus(findStatus, nbCounted);

				// If case-sensitivity changed (to Match=yes), there may have been a matched selection that
				// now does not match; so if Not Found, clear selection and put caret at beginning of what was
				// selected (no change, if there was no selection)
				if (updateCase && !isFound)	{

					Sci_CharacterRange range = (*(_pFRDlg->_ppEditView))->getSelection();
					(*(_pFRDlg->_ppEditView))->f(SCI_SETSEL, static_cast<WPARAM>(-1), range.cpMin);
				}
			}

			if (updateHiLight)	{

				bool highlight = !str2Search.empty() &&
					(BST_CHECKED == ::SendDlgItemMessage(_hSelf, IDC_INCFINDHILITEALL, BM_GETCHECK, 0, 0));
				markSelectedTextInc(highlight, &fo);
			}
			return TRUE;
		}

		case WM_ERASEBKGND:	{

			HWND hParent = ::GetParent(_hSelf);
			HDC winDC = (HDC)wParam;
			//RTL handling
			POINT pt = {0, 0}, ptOrig = {0, 0};
			::MapWindowPoints(_hSelf, hParent, &pt, 1);
			::OffsetWindowOrgEx((HDC)wParam, pt.x, pt.y, &ptOrig);
			LRESULT lResult = SendMessage(hParent, WM_ERASEBKGND, reinterpret_cast<WPARAM>(winDC), 0);
			::SetWindowOrgEx(winDC, ptOrig.x, ptOrig.y, NULL);
			return (BOOL)lResult;
		}
	}
	return DefWindowProc(getHSelf(), message, wParam, lParam);
}

void FindIncrementDlg::markSelectedTextInc(bool enable, FindOption *opt)	{

	(*(_pFRDlg->_ppEditView))->clearIndicator(SCE_UNIVERSAL_FOUND_STYLE_INC);

	if (!enable)
		return;

	//Get selection
	Sci_CharacterRange range = (*(_pFRDlg->_ppEditView))->getSelection();

	//If nothing selected, dont mark anything
	if (range.cpMin == range.cpMax)
		return;

	TCHAR text2Find[FINDREPLACE_MAXLENGTH];
	(*(_pFRDlg->_ppEditView))->getGenericSelectedText(text2Find, FINDREPLACE_MAXLENGTH, false);	//do not expand selection (false)
	opt->_str2Search = text2Find;
	_pFRDlg->markAllInc(opt);
}

void FindIncrementDlg::setFindStatus(FindStatus iStatus, int nbCounted)	{

	static TCHAR findCount[128] = L"";
	static const TCHAR * const findStatus[] = { findCount, // FSFound
	                               L"Phrase not found", //FSNotFound
	                               L"Reached top of page, continued from bottom", // FSTopReached
	                               L"Reached end of page, continued from top"}; // FSEndReached
	if (nbCounted <= 0)
		findCount[0] = '\0';
	else if (nbCounted == 1)
		wsprintf(findCount, L"%d match.", nbCounted);
	else
		wsprintf(findCount, L"%s matches.", commafyInt(nbCounted).c_str());

	if (iStatus<0 || iStatus >= sizeof(findStatus)/sizeof(findStatus[0]))
		return; // out of range

	_findStatus = iStatus;

	// get the HWND of the editor
	HWND hEditor = ::GetDlgItem(_hSelf, IDC_INCFINDTEXT);

	// invalidate the editor rect
	::InvalidateRect(hEditor, NULL, TRUE);
	::SendDlgItemMessage(_hSelf, IDC_INCFINDSTATUS, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(findStatus[iStatus]));
}

void FindIncrementDlg::addToRebar(ReBar * rebar)	{ 

	if (_pRebar)
		return;

	_pRebar = rebar;
	RECT client;
	getClientRect(client);

	ZeroMemory(&_rbBand, REBARBAND_SIZE);
	_rbBand.cbSize  = REBARBAND_SIZE;

	_rbBand.fMask   = RBBIM_STYLE | RBBIM_CHILD | RBBIM_CHILDSIZE |
					RBBIM_SIZE | RBBIM_ID;

	_rbBand.fStyle = RBBS_HIDDEN | RBBS_NOGRIPPER;
	_rbBand.hwndChild	= getHSelf();
	_rbBand.wID			= REBAR_BAR_SEARCH;	//ID REBAR_BAR_SEARCH for search dialog
	_rbBand.cxMinChild	= 0;
	_rbBand.cyIntegral	= 1;
	_rbBand.cyMinChild	= _rbBand.cyMaxChild	= client.bottom-client.top;
	_rbBand.cxIdeal		= _rbBand.cx			= client.right-client.left;

	_pRebar->addBand(&_rbBand, true);
	_pRebar->setGrayBackground(_rbBand.wID);
}

const TCHAR Progress::cClassName[] = L"NppProgressClass";
const TCHAR Progress::cDefaultHeader[] = L"Operation progress...";
const int Progress::cBackgroundColor = COLOR_3DFACE;
const int Progress::cPBwidth = 600;
const int Progress::cPBheight = 10;
const int Progress::cBTNwidth = 80;
const int Progress::cBTNheight = 25;


volatile LONG Progress::refCount = 0;


Progress::Progress(HINSTANCE hInst) : _hwnd(NULL), _hCallerWnd(NULL)	{

	if (::InterlockedIncrement(&refCount) == 1)	{

		_hInst = hInst;

		WNDCLASSEX wcex = {0};
		wcex.cbSize = sizeof(wcex);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = wndProc;
		wcex.hInstance = _hInst;
		wcex.hCursor = ::LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground = ::GetSysColorBrush(cBackgroundColor);
		wcex.lpszClassName = cClassName;

		::RegisterClassEx(&wcex);

		INITCOMMONCONTROLSEX icex = {0};
		icex.dwSize = sizeof(icex);
		icex.dwICC = ICC_STANDARD_CLASSES | ICC_PROGRESS_CLASS;

		::InitCommonControlsEx(&icex);
	}
}


Progress::~Progress()
{
	close();

	if (!::InterlockedDecrement(&refCount))
		::UnregisterClass(cClassName, _hInst);
}


HWND Progress::open(HWND hCallerWnd, const TCHAR* header)	{

	if (_hwnd)
		return _hwnd;

	// Create manually reset non-signalled event
	_hActiveState = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	if (!_hActiveState)
		return NULL;

	_hCallerWnd = hCallerWnd;

	for (HWND hwnd = _hCallerWnd; hwnd; hwnd = ::GetParent(hwnd))
		::UpdateWindow(hwnd);

	if (header)
		_tcscpy_s(_header, _countof(_header), header);
	else
		_tcscpy_s(_header, _countof(_header), cDefaultHeader);

	_hThread = ::CreateThread(NULL, 0, threadFunc, this, 0, NULL);
	if (!_hThread)	{

		::CloseHandle(_hActiveState);
		return NULL;
	}

	// Wait for the progress window to be created
	::WaitForSingleObject(_hActiveState, INFINITE);

	// On progress window create fail
	if (!_hwnd)	{

		::WaitForSingleObject(_hThread, INFINITE);
		::CloseHandle(_hThread);
		::CloseHandle(_hActiveState);
	}

	return _hwnd;
}


void Progress::close()	{

	if (_hwnd)	{

		::PostMessage(_hwnd, WM_CLOSE, 0, 0);
		_hwnd = NULL;
		::WaitForSingleObject(_hThread, INFINITE);

		::CloseHandle(_hThread);
		::CloseHandle(_hActiveState);
	}
}


void Progress::setPercent(unsigned percent, const TCHAR *fileName) const
{
	if (_hwnd)	{

		::PostMessage(_hPBar, PBM_SETPOS, percent, 0);
		::SendMessage(_hPText, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(fileName));
	}
}


DWORD WINAPI Progress::threadFunc(LPVOID data)	{

	Progress* pw = static_cast<Progress*>(data);
	return (DWORD)pw->thread();
}


int Progress::thread()	{

	BOOL r = createProgressWindow();
	::SetEvent(_hActiveState);
	if (r)
		return r;

	// Window message loop
	MSG msg;
	while ((r = ::GetMessage(&msg, NULL, 0, 0)) != 0 && r != -1)	{

		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}

	return r;
}


int Progress::createProgressWindow()	{

	_hwnd = ::CreateWindowEx(
		WS_EX_APPWINDOW | WS_EX_TOOLWINDOW | WS_EX_OVERLAPPEDWINDOW,
		cClassName, _header, WS_POPUP | WS_CAPTION,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, _hInst, (LPVOID)this);
	if (!_hwnd)
		return -1;

	int width = cPBwidth + 10;
	int height = cPBheight + cBTNheight + 35;
	RECT win = adjustSizeAndPos(width, height);
	::MoveWindow(_hwnd, win.left, win.top,
		win.right - win.left, win.bottom - win.top, TRUE);

	::GetClientRect(_hwnd, &win);
	width = win.right - win.left;
	height = win.bottom - win.top;

	_hPText = ::CreateWindowEx(0, L"STATIC", L"",
		WS_CHILD | WS_VISIBLE | BS_TEXT | SS_PATHELLIPSIS,
		5, 5,
		width - 10, 20, _hwnd, NULL, _hInst, NULL);
	HFONT hf = (HFONT)::GetStockObject(DEFAULT_GUI_FONT);
	if (hf)
		::SendMessage(_hPText, WM_SETFONT, reinterpret_cast<WPARAM>(hf), MAKELPARAM(TRUE, 0));

	_hPBar = ::CreateWindowEx(0, PROGRESS_CLASS, L"Progress Bar",
		WS_CHILD | WS_VISIBLE | PBS_SMOOTH,
		5, 25, width - 10, cPBheight,
		_hwnd, NULL, _hInst, NULL);
	SendMessage(_hPBar, PBM_SETRANGE, 0, MAKELPARAM(0, 100));

	_hBtn = ::CreateWindowEx(0, L"BUTTON", L"Cancel",
		WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON | BS_TEXT,
		(width - cBTNwidth) / 2, height - cBTNheight - 5,
		cBTNwidth, cBTNheight, _hwnd, NULL, _hInst, NULL);

	if (hf)
		::SendMessage(_hBtn, WM_SETFONT, reinterpret_cast<WPARAM>(hf), MAKELPARAM(TRUE, 0));

	::ShowWindow(_hwnd, SW_SHOWNORMAL);
	::UpdateWindow(_hwnd);

	return 0;
}


RECT Progress::adjustSizeAndPos(int width, int height)	{

	RECT maxWin;
	maxWin.left		= ::GetSystemMetrics(SM_XVIRTUALSCREEN);
	maxWin.top		= ::GetSystemMetrics(SM_YVIRTUALSCREEN);
	maxWin.right	= ::GetSystemMetrics(SM_CXVIRTUALSCREEN) + maxWin.left;
	maxWin.bottom	= ::GetSystemMetrics(SM_CYVIRTUALSCREEN) + maxWin.top;

	POINT center;

	if (_hCallerWnd)	{

		RECT biasWin;
		::GetWindowRect(_hCallerWnd, &biasWin);
		center.x = (biasWin.left + biasWin.right) / 2;
		center.y = (biasWin.top + biasWin.bottom) / 2;
	}
	else	{

		center.x = (maxWin.left + maxWin.right) / 2;
		center.y = (maxWin.top + maxWin.bottom) / 2;
	}

	RECT win = maxWin;
	win.right = win.left + width;
	win.bottom = win.top + height;

	DWORD style = static_cast<DWORD>(::GetWindowLongPtr(_hwnd, GWL_EXSTYLE));
	::AdjustWindowRectEx(&win, static_cast<DWORD>(::GetWindowLongPtr(_hwnd, GWL_STYLE)), FALSE, style);

	width = win.right - win.left;
	height = win.bottom - win.top;

	if (width < maxWin.right - maxWin.left)	{

		win.left = center.x - width / 2;
		if (win.left < maxWin.left)
			win.left = maxWin.left;
		win.right = win.left + width;
		if (win.right > maxWin.right)	{

			win.right = maxWin.right;
			win.left = win.right - width;
		}
	}
	else	{

		win.left = maxWin.left;
		win.right = maxWin.right;
	}

	if (height < maxWin.bottom - maxWin.top)	{

		win.top = center.y - height / 2;
		if (win.top < maxWin.top)
			win.top = maxWin.top;
		win.bottom = win.top + height;
		if (win.bottom > maxWin.bottom)	{

			win.bottom = maxWin.bottom;
			win.top = win.bottom - height;
		}
	}
	else	{

		win.top = maxWin.top;
		win.bottom = maxWin.bottom;
	}

	return win;
}


LRESULT APIENTRY Progress::wndProc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)	{

	switch (umsg)	{

		case WM_CREATE:	{

			Progress* pw = reinterpret_cast<Progress*>(reinterpret_cast<LPCREATESTRUCT>(lparam)->lpCreateParams);
			::SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pw));
			return 0;
		}

		case WM_SETFOCUS:	{

			Progress* pw =	reinterpret_cast<Progress*>(static_cast<LONG_PTR>
			(::GetWindowLongPtr(hwnd, GWLP_USERDATA)));
			::SetFocus(pw->_hBtn);
			return 0;
		}

		case WM_COMMAND:
			if (HIWORD(wparam) == BN_CLICKED)	{

				Progress* pw = reinterpret_cast<Progress*>(static_cast<LONG_PTR>(::GetWindowLongPtr(hwnd, GWLP_USERDATA)));
				::ResetEvent(pw->_hActiveState);
				::EnableWindow(pw->_hBtn, FALSE);
				pw->setInfo(L"Cancelling operation, please wait...");
				return 0;
			}
			break;

		case WM_DESTROY:
			::PostQuitMessage(0);
			return 0;
	}

	return ::DefWindowProc(hwnd, umsg, wparam, lparam);
}
