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
#include <cassert>

#ifndef _WIN32_IE
#define _WIN32_IE	0x0600
#endif //_WIN32_IE

#include "Window.h"
#include "Common.h"
#include <vector>

class StatusBar final : public Window	{

	virtual void init(HINSTANCE hInst, HWND hPere) override;
	std::vector<int> _partWidthArray;
	int *_lpParts = nullptr;

	generic_string lastText[NB_SB];
	generic_string savedOVLText;

public:
	virtual ~StatusBar();

	void init(HINSTANCE hInst, HWND hPere, int nbParts);
 
	bool setPartWidth(int whichPart, int width);

	virtual void destroy() override;
	virtual void reSizeTo(const RECT& rc);

	int getHeight() const;
	void adjustParts(int clientWidth);

	inline bool appText(int whichPart, const TCHAR* str=nullptr);
	inline bool prepText(int whichPart, const TCHAR* str=nullptr);
	inline bool ovLText(int whichPart, const TCHAR* str=nullptr, int p=0);

	inline bool setText(int whichPart, const TCHAR* str=nullptr, const TCHAR* p=nullptr, const TCHAR* =nullptr);
	inline bool setOwnerDrawText(const TCHAR* str);
	generic_string beText;

};

/* bool StatusBar::ovLText(int whichPart, const TCHAR* str, int p)	{
	generic_string s=lastText[whichPart];
	if (str)	{
		if (p<0)	{
			s.resize(s.size()+p);
			s = s+&str[0];
		}
		else
			s = &str[0] + s.substr(p);
		return ::SendMessage(_hSelf, SB_SETTEXT, whichPart, reinterpret_cast<LPARAM>((lastText[whichPart]=s).c_str()));
	}
	return ::SendMessage(_hSelf, SB_SETTEXT, whichPart, reinterpret_cast<LPARAM>(lastText[whichPart].c_str()));
}

bool StatusBar::prepText(int whichPart, const TCHAR* str)	{
	generic_string s = lastText[whichPart];
	if (str)
		s = &str[0] + s;
	return ::SendMessage(_hSelf, SB_SETTEXT, whichPart, reinterpret_cast<LPARAM>(s.c_str()));
}

bool StatusBar::appText(int whichPart, const TCHAR* str)	{
	generic_string s=lastText[whichPart];
	if (str)
		s += &str[0];
	return ::SendMessage(_hSelf, SB_SETTEXT, whichPart, reinterpret_cast<LPARAM>(s.c_str()));
} */

bool StatusBar::setText(int whichPart, const TCHAR* str, const TCHAR* prep, const TCHAR* app)	{
	generic_string s;
	if (str)
		s = lastText[whichPart]	= str;
	else
		s = lastText[whichPart];
	if (prep)
		s = &prep[0] + s;
	if (app)
		s += &app[0];

	return ::SendMessage(_hSelf, SB_SETTEXT, whichPart, reinterpret_cast<LPARAM>(s.c_str()));
	// assert(false and "invalid status bar index");return false;
}

bool StatusBar::setOwnerDrawText(const TCHAR* str)	{
	return ::SendMessage(_hSelf, SB_SETTEXT, SBT_OWNERDRAW, reinterpret_cast<LPARAM>(str));
}