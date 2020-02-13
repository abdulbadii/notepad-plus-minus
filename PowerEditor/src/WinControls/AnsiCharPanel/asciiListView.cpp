// This file is part of Notepad++ project
// Copyright (C)2016 Don HO <don.h@free.fr>
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


#include "asciiListView.h"
#include "Parameters.h"

void AsciiListView::resetValues(int codepage)
{
	if (codepage == -1)
		codepage = 0;

	if (_codepage == codepage)
		return;

	ListView_DeleteAllItems(_hSelf);
	setValues(codepage);
}

generic_string AsciiListView::getAscii(unsigned char value)
{
	switch (value)
	{
		case 0:
			return L"NULL";
		case 1:
			return L"SOH";
		case 2:
			return L"STX";
		case 3:
			return L"ETX";
		case 4:
			return L"EOT";
		case 5:
			return L"ENQ";
		case 6:
			return L"ACK";
		case 7:
			return L"BEL";
		case 8:
			return L"BS";
		case 9:
			return L"TAB";
		case 10:
			return L"LF";
		case 11:
			return L"VT";
		case 12:
			return L"FF";
		case 13:
			return L"CR";
		case 14:
			return L"SO";
		case 15:
			return L"SI";
		case 16:
			return L"DLE";
		case 17:
			return L"DC1";
		case 18:
			return L"DC2";
		case 19:
			return L"DC3";
		case 20:
			return L"DC4";
		case 21:
			return L"NAK";
		case 22:
			return L"SYN";
		case 23:
			return L"ETB";
		case 24:
			return L"CAN";
		case 25:
			return L"EM";
		case 26:
			return L"SUB";
		case 27:
			return L"ESC";
		case 28:
			return L"FS";
		case 29:
			return L"GS";
		case 30:
			return L"RS";
		case 31:
			return L"US";
		case 32:
			return L"Space";
		case 127:
			return L"DEL";
		default:
		{
			TCHAR charStr[10];
			char ascii[2];
			ascii[0] = value;
			ascii[1] = '\0';
			MultiByteToWideChar(_codepage, 0, ascii, -1, charStr, _countof(charStr));
			return charStr;
		}

	}
}

generic_string AsciiListView::getHtmlName(unsigned char value)
{
	switch (value)
	{
		case 34:
			return L"&quot;";
		case 38:
			return L"&amp;";
		case 60:
			return L"&lt;";
		case 62:
			return L"&gt;";
		case 128:
			return L"&euro;";
		case 160:
			return L"&nbsp;";
		case 161:
			return L"&iexcl;";
		case 162:
			return L"&cent;";
		case 163:
			return L"&pound;";
		case 164:
			return L"&curren;";
		case 165:
			return L"&yen;";
		case 166:
			return L"&brvbar;";
		case 167:
			return L"&sect;";
		case 168:
			return L"&uml;";
		case 169:
			return L"&copy;";
		case 170:
			return L"&ordf;";
		case 171:
			return L"&laquo;";
		case 172:
			return L"&not;";
		case 173:
			return L"&shy;";
		case 174:
			return L"&reg;";
		case 175:
			return L"&macr;";
		case 176:
			return L"&deg;";
		case 177:
			return L"&plusmn;";
		case 178:
			return L"&sup2;";
		case 179:
			return L"&sup3;";
		case 180:
			return L"&acute;";
		case 181:
			return L"&micro;";
		case 182:
			return L"&para;";
		case 183:
			return L"&middot;";
		case 184:
			return L"&cedil;";
		case 185:
			return L"&sup1;";
		case 186:
			return L"&ordm;";
		case 187:
			return L"&raquo;";
		case 188:
			return L"&frac14;";
		case 189:
			return L"&frac12;";
		case 190:
			return L"&frac34;";
		case 191:
			return L"&iquest;";
		case 192:
			return L"&Agrave;";
		case 193:
			return L"&Aacute;";
		case 194:
			return L"&Acirc;";
		case 195:
			return L"&Atilde;";
		case 196:
			return L"&Auml;";
		case 197:
			return L"&Aring;";
		case 198:
			return L"&AElig;";
		case 199:
			return L"&Ccedil;";
		case 200:
			return L"&Egrave;";
		case 201:
			return L"&Eacute;";
		case 202:
			return L"&Ecirc;";
		case 203:
			return L"&Euml;";
		case 204:
			return L"&Igrave;";
		case 205:
			return L"&Iacute;";
		case 206:
			return L"&Icirc;";
		case 207:
			return L"&Iuml;";
		case 208:
			return L"&ETH;";
		case 209:
			return L"&Ntilde;";
		case 210:
			return L"&Ograve;";
		case 211:
			return L"&Oacute;";
		case 212:
			return L"&Ocirc;";
		case 213:
			return L"&Otilde;";
		case 214:
			return L"&Ouml;";
		case 215:
			return L"&times;";
		case 216:
			return L"&Oslash;";
		case 217:
			return L"&Ugrave;";
		case 218:
			return L"&Uacute;";
		case 219:
			return L"&Ucirc;";
		case 220:
			return L"&Uuml;";
		case 221:
			return L"&Yacute;";
		case 222:
			return L"&THORN;";
		case 223:
			return L"&szlig;";
		case 224:
			return L"&agrave;";
		case 225:
			return L"&aacute;";
		case 226:
			return L"&acirc;";
		case 227:
			return L"&atilde;";
		case 228:
			return L"&auml;";
		case 229:
			return L"&aring;";
		case 230:
			return L"&aelig;";
		case 231:
			return L"&ccedil;";
		case 232:
			return L"&egrave;";
		case 233:
			return L"&eacute;";
		case 234:
			return L"&ecirc;";
		case 235:
			return L"&euml;";
		case 236:
			return L"&igrave;";
		case 237:
			return L"&iacute;";
		case 238:
			return L"&icirc;";
		case 239:
			return L"&iuml;";
		case 240:
			return L"&eth;";
		case 241:
			return L"&ntilde;";
		case 242:
			return L"&ograve;";
		case 243:
			return L"&oacute;";
		case 244:
			return L"&ocirc;";
		case 245:
			return L"&otilde;";
		case 246:
			return L"&ouml;";
		case 247:
			return L"&divide;";
		case 248:
			return L"&oslash;";
		case 249:
			return L"&ugrave;";
		case 250:
			return L"&uacute;";
		case 251:
			return L"&ucirc;";
		case 252:
			return L"&uuml;";
		case 253:
			return L"&yacute;";
		case 254:
			return L"&thorn;";
		case 255:
			return L"&yuml;";
		default:
		{
			return L"";
		}
		
	}
}

int AsciiListView::getHtmlNumber(unsigned char value)
{
	switch (value)
	{
		case 128:
			return 8364;
		case 130:
			return 8218;
		case 131:
			return 402;
		case 132:
			return 8222;
		case 133:
			return 8230;
		case 134:
			return 8224;
		case 135:
			return 8225;
		case 137:
			return 8240;
		case 138:
			return 352;
		case 140:
			return 338;
		case 145:
			return 8216;
		case 146:
			return 8217;
		case 147:
			return 8220;
		case 148:
			return 8221;
		case 149:
			return 8226;
		case 150:
			return 8211;
		case 151:
			return 8212;
		case 153:
			return 8482;
		case 154:
			return 353;
		case 156:
			return 339;
		case 159:
			return 376;
		default:
		{
			return -1;
		}
		
	}
}

void AsciiListView::setValues(int codepage)
{
	_codepage = codepage;

	for (int i = 0 ; i < 256 ; ++i)
	{
		TCHAR dec[8];
		TCHAR hex[8];
		TCHAR htmlNumber[8];
		generic_string htmlName;
		generic_sprintf(dec, L"%d", i);
		generic_sprintf(hex, L"%02X", i);
		generic_string s = getAscii(static_cast<unsigned char>(i));

		if (codepage == 0 || codepage == 1252)
		{
			if ((i >= 32 && i <= 126) || (i >= 160 && i <= 255))
			{
				generic_sprintf(htmlNumber, L"&#%d", i);
			}
			else
			{
				int n = getHtmlNumber(static_cast<unsigned char>(i));
				if (n > -1)
				{
					generic_sprintf(htmlNumber, L"&#%d", n);
				}
				else
				{
					generic_sprintf(htmlNumber, L"");
				}
			}

			htmlName = getHtmlName(static_cast<unsigned char>(i));
		}
		else
		{
			generic_sprintf(htmlNumber, L"");
			htmlName = L"";
		}

		std::vector<generic_string> values2Add;

		values2Add.push_back(dec);
		values2Add.push_back(hex);
		values2Add.push_back(s);
		values2Add.push_back(htmlNumber);
		values2Add.push_back(htmlName);

		addLine(values2Add);
	}
}
