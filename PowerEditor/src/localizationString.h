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

LocalizationSwitcher::LocalizationDefinition localizationDefs[] =
{
	{L"English", TEXT("english.xml")},
	{L"English (customizable)", TEXT("english_customizable.xml")},
	{L"Français", TEXT("french.xml")},
	{L"中文繁體", TEXT("chinese.xml")},
	{L"中文简体", TEXT("chineseSimplified.xml")},
	{L"한국어", TEXT("korean.xml")},
	{L"日本語", TEXT("japanese.xml")},
	{L"Deutsch", TEXT("german.xml")},
	{L"Español", TEXT("spanish.xml")},
	{L"Italiano", TEXT("italian.xml")},
	{L"Português", TEXT("portuguese.xml")},
	{L"Português brasileiro", TEXT("brazilian_portuguese.xml")},
	{L"Nederlands", TEXT("dutch.xml")},
	{L"Русский", TEXT("russian.xml")},
	{L"Polski", TEXT("polish.xml")},
	{L"Català", TEXT("catalan.xml")},
	{L"Česky", TEXT("czech.xml")},
	{L"Magyar", TEXT("hungarian.xml")},
	{L"Română", TEXT("romanian.xml")},
	{L"Türkçe", TEXT("turkish.xml")},
	{L"فارسی", TEXT("farsi.xml")},
	{L"Українська", TEXT("ukrainian.xml")},
	{L"עברית", TEXT("hebrew.xml")},
	{L"Nynorsk", TEXT("nynorsk.xml")},
	{L"Norsk", TEXT("norwegian.xml")},
	{L"Occitan", TEXT("occitan.xml")},
	{L"ไทย", TEXT("thai.xml")},
	{L"Furlan", TEXT("friulian.xml")},
	{L"العربية", TEXT("arabic.xml")},
	{L"Suomi", TEXT("finnish.xml")},
	{L"Lietuvių", TEXT("lithuanian.xml")},
	{L"Ελληνικά", TEXT("greek.xml")},
	{L"Svenska", TEXT("swedish.xml")},
	{L"Galego", TEXT("galician.xml")},
	{L"Slovenščina", TEXT("slovenian.xml")},
	{L"Slovenčina", TEXT("slovak.xml")},
	{L"Dansk", TEXT("danish.xml")},
	{L"Estremeñu", TEXT("extremaduran.xml")},
	{L"Žemaitiu ruoda", TEXT("samogitian.xml")},
	{L"Български", TEXT("bulgarian.xml")},
	{L"Bahasa Indonesia", TEXT("indonesian.xml")},
	{L"Gjuha shqipe", TEXT("albanian.xml")},
	{L"Hrvatski jezik", TEXT("croatian.xml")},
	{L"ქართული ენა", TEXT("georgian.xml")},
	{L"Euskara", TEXT("basque.xml")},
	{L"Español argentina", TEXT("spanish_ar.xml")},
	{L"Беларуская мова", TEXT("belarusian.xml")},
	{L"Srpski", TEXT("serbian.xml")},
	{L"Cрпски", TEXT("serbianCyrillic.xml")},
	{L"Bahasa Melayu", TEXT("malay.xml")},
	{L"Lëtzebuergesch", TEXT("luxembourgish.xml")},
	{L"Tagalog", TEXT("tagalog.xml")},
	{L"Afrikaans", TEXT("afrikaans.xml")},
	{L"Қазақша", TEXT("kazakh.xml")},
	{L"O‘zbekcha", TEXT("uzbek.xml")},
	{L"Ўзбекча", TEXT("uzbekCyrillic.xml")},
	{L"Кыргыз тили", TEXT("kyrgyz.xml")},
	{L"Македонски јазик", TEXT("macedonian.xml")},
	{L"latviešu valoda", TEXT("latvian.xml")},
	{L"தமிழ்", TEXT("tamil.xml")},
	{L"Azərbaycan dili", TEXT("azerbaijani.xml")},
	{L"Bosanski", TEXT("bosnian.xml")},
	{L"Esperanto", TEXT("esperanto.xml")},
	{L"Zeneize", TEXT("ligurian.xml")},
	{L"हिन्दी", TEXT("hindi.xml")},
	{L"Sardu", TEXT("sardinian.xml")},
	{L"ئۇيغۇرچە", TEXT("uyghur.xml")},
	{L"తెలుగు", TEXT("telugu.xml")},
	{L"aragonés", TEXT("aragonese.xml")},
	{L"বাংলা", TEXT("bengali.xml")},
	{L"සිංහල", TEXT("sinhala.xml")},
	{L"Taqbaylit", TEXT("kabyle.xml")},
	{L"मराठी", TEXT("marathi.xml")},
	{L"tiếng Việt", TEXT("vietnamese.xml")},
	{L"Aranés", TEXT("aranese.xml")},
	{L"ગુજરાતી", TEXT("gujarati.xml")},
	{L"Монгол хэл", TEXT("mongolian.xml")},
	{L"اُردُو‎", TEXT("urdu.xml")},
	{L"ಕನ್ನಡ‎", TEXT("kannada.xml")},
	{L"Cymraeg", TEXT("welsh.xml")},
	{L"eesti keel", TEXT("estonian.xml")},
	{L"Тоҷик", TEXT("tajikCyrillic.xml")},
	{L"татарча", TEXT("tatar.xml")},
	{L"ਪੰਜਾਬੀ", TEXT("punjabi.xml")},
	{L"Corsu", TEXT("corsican.xml")},
	{L"Brezhoneg", TEXT("breton.xml")},
	{L"کوردی‬", TEXT("kurdish.xml")},
	{L"Pig latin", TEXT("piglatin.xml")},
	{L"Zulu", TEXT("zulu.xml")},
	{L"Vèneto", TEXT("venetian.xml")},
	{L"Gaeilge", TEXT("irish.xml")}
};
