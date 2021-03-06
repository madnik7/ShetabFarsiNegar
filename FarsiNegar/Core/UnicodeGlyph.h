#pragma once
#include "GlypherUtil.h"
#define CHAR_VIRTUALSPACE	(WCHAR)0x200C

namespace GlypherCore{

DEFINE_COMPTR(UnicodeGlyphSimulars);
class UnicodeGlyphSimulars : public IUnknownImpl<UnicodeGlyphSimulars>, public CAtlMap<WCHAR, WCHAR>
{
	Public void Serialize(MSXML2::IXMLDOMElementPtr pelement, bool save)
	{
		if (save)
		{
			POSITION pos = GetStartPosition();
			while (pos!=NULL)
			{
				MSXML2::IXMLDOMElementPtr pglyphElement = pelement->ownerDocument->createElement("Glyph");
				pelement->appendChild(pglyphElement);
				CPair* ppair = GetNext(pos);
				WCHAR key = ppair->m_key;
				WCHAR value = ppair->m_value;
				pglyphElement->setAttribute(L"Key", (LPCTSTR)Convert::ToString(key, L"%04x"));
				pglyphElement->setAttribute(L"Value", (LPCTSTR)Convert::ToString(value, L"%04x"));
				pglyphElement->setAttribute(L"KeySymbol", (LPCTSTR)Convert::ToString(key, L"%c"));
				pglyphElement->setAttribute(L"ValueSymbol", (LPCTSTR)Convert::ToString(value, L"%c"));
			}
		}
		else
		{
			MSXML2::IXMLDOMNodeListPtr psubElements = pelement!=NULL ? pelement->selectNodes("Glyph") : NULL;
			for (int i=0; psubElements!=NULL && i<(int)psubElements->length; i++)
			{
				MSXML2::IXMLDOMElementPtr pglyphElement = psubElements->Getitem(i);
				_variant_t attr;
				attr = pglyphElement->getAttribute(L"Key");
				WCHAR key = (WCHAR)Convert::ToInt(attr, 0, 16);
				attr = pglyphElement->getAttribute(L"Value");
				WCHAR value = (WCHAR)Convert::ToInt(attr, 0, 16);
				SetAt(key, value);
			}
		}
	}

	Public WCHAR Find(WCHAR code)
	{
		CPair* ppair = Lookup(code);
		if (ppair!=NULL)
			return ppair->m_value;
		return 0;
	}

	Protected ~UnicodeGlyphSimulars()
	{
	}
};

//UnicodeGlyphInfo keep unicode glyph data 
//(Unicode Glyph)->(Unicode Data)
DEFINE_COMPTR(UnicodeGlyphInfo);
class UnicodeGlyphInfo : public IUnknownImpl<UnicodeGlyphInfo>
{
	Public WCHAR Code;
	Public DWORD Flags;
    Public CString Data;
	Public CString Name;

	Public enum FlagEnum{
		FlagInvisible = 0x0001,
		FlagAdvanced =	0x0002,
	};

	Public static CString CodeToData(CString codes)
	{
		CString ret;
		int curPos = 0;
		CString resToken= codes.Tokenize(L",",curPos);
		while (resToken != L"")
		{
			WCHAR code = (WCHAR)Convert::ToInt(resToken, 16);
			if (code!=0)
				ret.AppendChar(code);
			resToken = codes.Tokenize(L",", curPos);
		}
		return ret;
	}

	Public static CString DataToCode(CString data)
	{
		CString ret;
		for (int i=0; i<data.GetLength(); i++)
		{
			if (i>0) ret += L",";
			ret += Convert::ToString(data[i], L"%04x");
		}
		return ret;
	}

	Public void Serialize(MSXML2::IXMLDOMElementPtr pelement, bool save)
	{
		if (save)
		{
			if (Code>32)
			{
				pelement->setAttribute(L"Code", (LPCTSTR)Convert::ToString(Code, L"%04x"));
				pelement->setAttribute(L"GlyphSymbol", (LPCTSTR)Convert::ToString(Code, L"%c"));
				pelement->setAttribute(L"Data", (LPCTSTR)Data);
				pelement->setAttribute(L"DataCode", (LPCTSTR)DataToCode(Data));
				pelement->setAttribute(L"Name", (LPCTSTR)Name);
				if (_TOBOOL(Flags&FlagInvisible))
					pelement->setAttribute(L"HideInTable", _TOBOOL(Flags&FlagInvisible));
				if (_TOBOOL(Flags&FlagAdvanced))
					pelement->setAttribute(L"Advanced", _TOBOOL(Flags&FlagAdvanced));
			}
		}
		else
		{
			_variant_t attr;
			attr = pelement->getAttribute(L"Code");
			Code = (WCHAR)Convert::ToInt(attr, 0, 16);

			attr = pelement->getAttribute(L"DataCode");
			Data = CodeToData(attr);

			attr = pelement->getAttribute(L"HideInTable");
			if (attr.vt!=VT_NULL) Flags |= FlagInvisible;

			attr = pelement->getAttribute(L"Advanced");
			if (attr.vt!=VT_NULL) Flags |= FlagAdvanced;

			attr = pelement->getAttribute(L"Name");
			if (attr.vt!=VT_NULL) Name = attr;
		}
	}

	Public explicit UnicodeGlyphInfo(MSXML2::IXMLDOMElementPtr pelement)
	{
		Code = 0;
		Flags = 0;
		Serialize(pelement, false);
	}

	Public UnicodeGlyphInfo(WCHAR code, CString data, DWORD flags=0)
	{
		Code = code;
		Data = data;
		Flags = flags;
	}

	Private UnicodeGlyphInfo();
	Protected ~UnicodeGlyphInfo(void)
	{
	}
};

DEFINE_COMPTR(UnicodeGlyphInfoArray);
class UnicodeGlyphInfoArray : public Array<UnicodeGlyphInfoArray, UnicodeGlyphInfoPtr>
{
	Public void Sort()
	{
		DWORD flags = 0;
		qsort_s(GetData(), GetCount(), sizeof UnicodeGlyphInfoPtr, CompareProc, &flags);
		Sort();
	}

	Private static int __cdecl CompareProc(void* /*context*/, const void* arg1, const void* arg2)
	{
		//DWORD flags = *((DWORD*)context);
		UnicodeGlyphInfoPtr pgi1 = (UnicodeGlyphInfo*)arg1;
		UnicodeGlyphInfoPtr pgi2 = (UnicodeGlyphInfo*)arg2;
		LCID lcid = MAKELCID(MAKELANGID(LANG_FARSI, SUBLANG_DEFAULT), SORT_DEFAULT);
		return CompareString(lcid,0, CString(pgi1->Code), CString(pgi1->Code).GetLength(), CString(pgi2->Code), CString(pgi2->Code).GetLength())-CSTR_EQUAL;
	}

	Protected ~UnicodeGlyphInfoArray()
	{
	}
};

// UnicodeGlyphInfos (Unicode Glyph)->(Unicode Data)
DEFINE_COMPTR(UnicodeGlyphInfos);
class UnicodeGlyphInfos : public IUnknownImpl<UnicodeGlyphInfos>, public CAtlMap<WCHAR, UnicodeGlyphInfoPtr>
{
	Private UnicodeGlyphSimularsPtr pGlyphSimulars;
	Public explicit UnicodeGlyphInfos(Path file)
	{
		pGlyphSimulars = new UnicodeGlyphSimulars();
		Load(file);
	}

	Public UnicodeGlyphInfoPtr Find(WCHAR glyphCode)
	{
		return Find(glyphCode, true);
	}

	Public UnicodeGlyphInfoPtr Find(WCHAR glyphCode, bool simular)
	{
		CPair* pair = Lookup(glyphCode);
		if ( pair!=NULL )
			return pair->m_value;

		if (simular)
		{
			WCHAR simularChar = pGlyphSimulars->Find(glyphCode);
			if (simularChar!=0)
				Find(simularChar, simular);
		}
		return NULL;
	}


	//@param simular if true then it return equalvaent glyph to another
	//eg: if arabic kaf not found it will retrun farsi kaf
	Public WCHAR FindCodeByData(CString data)
	{
		POSITION pos = GetStartPosition();
		while (pos!=NULL)
		{
			CPair* ppair = GetNext(pos);
			UnicodeGlyphInfoPtr pgi = ppair->m_value;
			if (pgi->Data==data)
				return ppair->m_key;
		}
		return 0;
	}

	Public void Save(Path fileName)
	{
		//initialize msxml
		MSXML2::IXMLDOMDocumentPtr pdocPtr;
		TESTHR(pdocPtr.CreateInstance("Msxml2.DOMDocument.3.0"));
		
		//initialize xml document
		MSXML2::IXMLDOMProcessingInstructionPtr pprocIns = pdocPtr->createProcessingInstruction(L"xml", L"version='1.0' encoding='utf-8'");
		pdocPtr->appendChild(pprocIns);

		//create UnicodeGlyphs element
		MSXML2::IXMLDOMElementPtr punicodeGlyphElement = pdocPtr->createElement(L"UnicodeGlyphs");
		pdocPtr->appendChild(punicodeGlyphElement);

		Serialize(punicodeGlyphElement, true);
		
		MSXML2::IXMLDOMDocumentPtr psortXST;
		TESTHR(psortXST.CreateInstance("Msxml2.DOMDocument.3.0"));
		Path sortXSD = fileName.GetFolder() + Path(L"UnicodeGlyphsSorter.xslt");
		if (psortXST->load((LPCTSTR)sortXSD)==VARIANT_FALSE)
			ThrowComError(L"Could not find " + sortXSD.ToString());

		MSXML2::IXMLDOMDocumentPtr pdocPtr2;
		TESTHR(pdocPtr2.CreateInstance("Msxml2.DOMDocument.3.0"));
		_bstr_t bstr = pdocPtr->transformNode(psortXST);
		pdocPtr2->loadXML(bstr);
		pdocPtr2->save((LPCTSTR)fileName);
	}

	Public void Serialize(MSXML2::IXMLDOMElementPtr pelement, bool save)
	{
		if (save)
		{
			MSXML2::IXMLDOMElementPtr psimularGlyphsElement = pelement->ownerDocument->createElement(L"SimularGlyphs");
			pelement->appendChild(psimularGlyphsElement);
			pGlyphSimulars->Serialize(psimularGlyphsElement, save);
			
			MSXML2::IXMLDOMElementPtr pglyphsElement = pelement->ownerDocument->createElement(L"Glyphs");
			pelement->appendChild(pglyphsElement);
			POSITION pos = GetStartPosition();
			while (pos!=NULL)
			{
				UnicodeGlyphInfoPtr pugi = GetNextValue(pos);
				MSXML2::IXMLDOMElementPtr pglyphElement = pglyphsElement->ownerDocument->createElement("Glyph");
				pglyphsElement->appendChild(pglyphElement);
				pugi->Serialize(pglyphElement, save);
			}
		}
		else
		{
			MSXML2::IXMLDOMElementPtr psimularGlyphsElement = pelement->selectSingleNode(L"SimularGlyphs");
			pGlyphSimulars = new UnicodeGlyphSimulars();
			if (psimularGlyphsElement!=NULL)
				pGlyphSimulars->Serialize(psimularGlyphsElement, save);

			MSXML2::IXMLDOMElementPtr pglyphsElement = pelement->selectSingleNode(L"Glyphs");
			MSXML2::IXMLDOMNodeListPtr psubElements = pglyphsElement!=NULL ? pglyphsElement->selectNodes("Glyph") : NULL;
			for (int i=0; psubElements!=NULL && i<(int)psubElements->length; i++)
			{
				MSXML2::IXMLDOMElementPtr pglyphElement = psubElements->Getitem(i);
				UnicodeGlyphInfoPtr pugi = new UnicodeGlyphInfo(pglyphElement);
				SetAt(pugi->Code, pugi);
			}
		}
	}

	Public void Load(Path fileName)
	{
		RemoveAll();
		InitHashTable(421);

		//initialize msxml
		MSXML2::IXMLDOMDocumentPtr pdocPtr;
		TESTHR(pdocPtr.CreateInstance("Msxml2.DOMDocument.3.0"));
		
		//initialize xml document
		if (pdocPtr->load((LPCTSTR)fileName)==VARIANT_FALSE)
			ThrowComError(L"Could not read " + fileName.ToString());

		//get UnicodeGlyphs element
		MSXML2::IXMLDOMElementPtr punicodeGlyphsElement = pdocPtr->selectSingleNode(L"UnicodeGlyphs");
		Serialize(punicodeGlyphsElement, false);
		InsertInlineChars();
	}

	Private UnicodeGlyphInfos();
	Protected ~UnicodeGlyphInfos()
	{
	}

	Private void InsertInlineChars()
	{
		SetAt(0x0A, new UnicodeGlyphInfo(0x0A, L"\n", UnicodeGlyphInfo::FlagInvisible)); //enter
		SetAt(0x0D, new UnicodeGlyphInfo(0x0D, L"\r", UnicodeGlyphInfo::FlagInvisible)); //line-feed

		//insert gylyph before 255
		//for (TCHAR i=33; i<255; i++)
		//	SetAt(i, new UnicodeGlyphInfo(i, CString((TCHAR)i), UnicodeGlyphInfo::FlagInvisible));

		//insert arabic-indic chars
		//for (WCHAR i=0x660; i<=0x669; i++)
		//	SetAt(i, new UnicodeGlyphInfo(i, Convert::ToString(i-0x660), UnicodeGlyphInfo::FlagInvisible));
	}

	//return seperate glyph that compose unicodeGlyph
	Public CString ParseComplexGlyph(WCHAR unicodeGlyph)
	{
		CString ret(unicodeGlyph);
		UnicodeGlyphInfoPtr pgi = Find(unicodeGlyph);
		if (pgi!=NULL && pgi->Data.GetLength()>1)
		{
			ret = L"";
			for (int i=0; i<pgi->Data.GetLength(); i++)
			{
				WCHAR glyph = FindCodeByData(CString(pgi->Data[i]));
				if (glyph)
					ret = CString(glyph) + ret;
			}
		}
		return ret;
	}

};

DEFINE_COMPTR(UnicodeGlyphConverter);
class UnicodeGlyphConverter : public IUnknownImpl<UnicodeGlyphConverter>
{
	Private static int const MaxGlyphsCount = 0xFFFF;

	Public enum Flags{
		FlagBreak		= 0x00001,
		FlagKashida		= 0x00002,
		FlagKashidaFull	= 0x00004,
		FlagRightToLeft	= 0x00008,
	};

	Private UnicodeGlyphInfosPtr pGlyphInfosField;
	Public UnicodeGlyphInfosPtr UnicodeGlyphInfosGet()
	{
		return pGlyphInfosField;
	}

	Public UnicodeGlyphConverter(UnicodeGlyphInfosPtr pglyphInfos)
	{
		pGlyphInfosField = pglyphInfos;
		memset(&gcpField, 0, sizeof GCP_RESULTS);
		gcpField.lStructSize = sizeof GCP_RESULTS;
		gcpField.lpGlyphs = new WCHAR[MaxGlyphsCount];
		gcpField.lpClass = new CHAR[MaxGlyphsCount];
		gcpField.lpDx = new int[MaxGlyphsCount];
		pUnicodeToGlyphIndex = new WORD[MaxGlyphsCount];
		pGlyphIndexToUnicode = new WORD[MaxGlyphsCount];

	}

	Private UnicodeGlyphConverter();
	Protected ~UnicodeGlyphConverter()
	{
		delete gcpField.lpGlyphs;
		delete gcpField.lpDx;
		delete gcpField.lpClass;
		delete pUnicodeToGlyphIndex;
		delete pGlyphIndexToUnicode;
	}

	Public CString StringToGlyphs(CString text, bool rightToLeft)
	{
		HFONT fontHandle = mOld::Font_Create(L"Name=Tahoma;Size=8");
		DWORD flags = rightToLeft ? FlagRightToLeft : 0;
		CString ret = StringToGlyphs(text, fontHandle, 0, flags);
		DeleteObject(fontHandle);
		return ret;
	}

	Public CString StringToGlyphs(CString text, HFONT fontHandle, int width, DWORD flags)
	{
		HDC dcHandle = CreateCompatibleDC(NULL);
		HFONT oldFont = (HFONT)SelectObject(dcHandle, fontHandle);
		CString ret = StringToGlyphs(text, dcHandle, width, flags);
		SelectObject(dcHandle, oldFont);
		DeleteDC(dcHandle);
		return ret;
	}

	Private CString StringToGlyphs(CString text, HDC dcHandle, int width, DWORD flags)
	{
		if (text.IsEmpty())
			return L"";

		DWORD oldLayout = SetLayout(dcHandle, _TOBOOL(flags&FlagRightToLeft) ?  LAYOUT_RTL : 0);
		text.Replace(L"\r\n", L"\n");
		text.Replace(L"\x200C", L"*$*");
		if (IsReInitCMapNeeded(dcHandle))
			ReInitCMap(dcHandle);

		//////////////////////////////////////////////
		//{process Wordwrap add line-feed("\n") to text and prepare for wrap
		if ( _TOBOOL(flags & FlagBreak)  )
		{
			CString stringWrap;
			int startIndex = 0;
			CString phase1;
			while (startIndex<text.GetLength())
			{
				//get next phrase
				WCHAR chBreakChar;
				CString phase2 = GetNextBreak(text, startIndex, &chBreakChar);

				//{calculate length
				//analyse text and get size
				CSize size;

    			int nPhase1=0; //phase 1 metric
				int res = GetTextExtentPoint32(dcHandle, phase1, phase1.GetLength(), &size);
				if ( res!=0 )
				{
					nPhase1 = size.cx;
				}

				int nPhase2=0; //phase 2 metric
				res = GetTextExtentPoint32(dcHandle, phase2, phase2.GetLength(), &size);
				if ( res!=0 )
				{
					nPhase2 = size.cx;
				}

				//}calculate length
				
				//add phase1 to phase2, and get another phase2
				if ( nPhase1+nPhase2<=width )
				{
					phase1 += phase2;
				}
				//add phase1 to stringWrap with line-feed and start next ine
				else
				{
					stringWrap += phase1 + L"\n";
					phase1 = phase2;
				}

				//process line-feed
				if ( chBreakChar==L'\n' )
				{
					stringWrap += phase1 + L"\n";
					phase1.Empty();
				}
			}

			//add last phase
			stringWrap += phase1;
			text = stringWrap; //update original text
		}
		//}process Wordwrap

		//////////////////////////////////////////////
		//Draw Multiline text by carriage return-line feed
		CString ret;
		CPoint ptText(0,0);
		int startIndex = 0;
		bool bFound = true;
		while (bFound)
		{
			//get next line
			int iEnd = text.Find(L'\n', startIndex);
			bFound = iEnd!=-1;
			if (iEnd==-1) iEnd = text.GetLength();
			CString line = text.Mid(startIndex, iEnd-startIndex);

			//analyse text and get size and draw line
			gcpField.nGlyphs = MaxGlyphsCount;
			memset(gcpField.lpGlyphs, 0, MaxGlyphsCount);
			memset(gcpField.lpClass, 0, line.GetLength());
			gcpField.lpClass[0] = GCPCLASS_POSTBOUNDRTL;
			
			//generate flags and call GetCharacterPlacement (prevent USE GCP_CLASSIN because notpad not use it)
			DWORD dwGcpFlags = 0*GCP_CLASSIN | GCP_GLYPHSHAPE | GCP_LIGATE | GCP_REORDER | GCP_JUSTIFY | GCP_DIACRITIC;
			if ( _TOBOOL(flags & FlagBreak) ) dwGcpFlags |= GCP_MAXEXTENT;
			if ( _TOBOOL(flags & FlagKashida) && text.Find(L'\n', iEnd)>=0) dwGcpFlags |= GCP_KASHIDA;
			if ( _TOBOOL(flags & FlagKashidaFull) ) dwGcpFlags |= GCP_KASHIDA;

			DWORD resGCP = GetCharacterPlacement(dcHandle, CString(line), line.GetLength(), width, &gcpField, dwGcpFlags);
			//ExtTextOut(dcHandle, 0, 100, ETO_GLYPH_INDEX, 0, (LPCWSTR)gcpField.lpGlyphs, gcpField.nGlyphs, NULL);


			if ( resGCP==0)
			{
				ret+=L"\r\n";
			}
			else
			{
				//convert GlyphIndex to unicode; if unicode not exist use GlyphIndex as code
				CString resGCPcode;
				for (UINT i=0; i<gcpField.nGlyphs; i++)
				{
					WCHAR glyph = gcpField.lpGlyphs[i];
					WCHAR code = glyph;
					if (pGlyphIndexToUnicode[glyph]!=0)
						code = pGlyphIndexToUnicode[glyph];
					resGCPcode.AppendChar(code);

					//switch(glyph){
					//case 1127:
					//	glyph=0x640;
					//	break;

					//case 0x2ef: // i dont know why "ء" has problem
					//	glyph=0xFE80;
					//	break;
					//}
				}
				
				if ( !ret.IsEmpty() ) ret+=L"\r\n";
				UnicodeGlyphInfoPtr pgi = pGlyphInfosField->Find(0x200C);
				resGCPcode.Replace(L"*$*", pgi!=NULL ? pgi->Data : L"");
				ret += resGCPcode;
			}
			
			//set index to next line
			startIndex = iEnd + 1; 
		}

		//clean up
		SetLayout(dcHandle, oldLayout);
		return ret;
	}

	// convert text of glyphs into unicode text
	// @param glyphs array of glyphs
	// @return windows standard wide text
	Public CString GlyphsToString(CString glyphs)
	{
		//seprate lines
		glyphs.Replace(L"\r\n", L"\n");
		glyphs.Replace(L'\r', '\n');

		//get lines
		StringListPtr plines = new StringList();
		int curPos = 0;
		CString line = glyphs.Tokenize(L"\n", curPos);
		while (!line.IsEmpty())
		{
			plines->AddTail(line);
			line = glyphs.Tokenize(L"\n", curPos);
		}

		//return
		CString ret;
		POSITION pos = plines->GetHeadPosition();
		while (pos!=NULL)
		{
			CString line2 = plines->GetNext(pos);
			ret += Glyphs_GetLine( line2 );
			if (pos!=NULL) 
				ret += L"\r\n";
		}
		return ret;
	}


	Private static CString GetDCFontName(HDC dcHandle)
	{
		HFONT fontHandle = (HFONT)::GetCurrentObject(dcHandle, OBJ_FONT);
		LOGFONT lf={0};
		if (GetObject( fontHandle, sizeof(LOGFONT), &lf)!=NULL)
			return lf.lfFaceName;
		return CString();
	}

	Private bool IsReInitCMapNeeded(HDC dcHandle)
	{
		return GetDCFontName(dcHandle).CompareNoCase(CMapFontNameField)!=0;
	}

	Private void ReInitCMap(HDC dcHandle)
	{
		memset(pUnicodeToGlyphIndex, 0, sizeof MaxGlyphsCount);
		memset(pGlyphIndexToUnicode, 0, sizeof MaxGlyphsCount);

		SCRIPT_CACHE sc=NULL;
		for (WCHAR i=0; i<0xFFFF; i++)
		{
			WORD indice;
			HRESULT hr = ScriptGetCMap(dcHandle, &sc, &i, 1, 0*SGCM_RTL, &indice);
			if (hr==S_OK)
			{
				pUnicodeToGlyphIndex[i] = indice;
				pGlyphIndexToUnicode[indice] = i;
			}
			else
			{
				pUnicodeToGlyphIndex[i] = 0;
			}
		}

		CMapFontNameField = GetDCFontName(dcHandle);
	}

	Private CString Glyphs_GetLine(CString glyphs)
	{
		CString ret;
		CString stack;
		for (int i=glyphs.GetLength()-1; i>=0; i--)
		{
			UnicodeGlyphInfoPtr pgi = pGlyphInfosField->Find(glyphs[i]);
			if ( pgi!=NULL )
			{
				if ( pgi->Data.GetLength()>0 && isltr(pgi->Data[0]) )
				{
					stack = pgi->Data + stack;
				}
				else if (!stack.IsEmpty())
				{
					ret = ret + stack + pgi->Data;
					stack.Empty();
				}
				else
				{
					ret = ret + pgi->Data;
				}
			}
		}

		ret = ret + stack;
		return ret;
	}

	Private CString GetNextBreak(CString text, int& startIndex, WCHAR* pbreakChar)
	{
		CString ret;

		for (int i=startIndex+1; i<text.GetLength(); i++)
		{
			WCHAR ch = text[i];
			if ( ch==L' ' || ch==L'-' || ch==L'\n' || 
				ch==L'.' || ch==L',')
			{
				if (pbreakChar!=NULL) *pbreakChar = ch;
				ret = (ch==L'\n') 
					? text.Mid(startIndex, i-startIndex)
					: text.Mid(startIndex, i-startIndex+1);
				startIndex = i+1;
				return ret;
			}
		}

		//retrun last phase if not any phase found
		if ( startIndex < text.GetLength() )
			ret = text.Mid(startIndex);

		//set break character
		if (pbreakChar!=NULL) *pbreakChar = 0;

		startIndex=text.GetLength();
		return ret;
	}

	Private static bool isltr(WCHAR wchar)
	{
		return iswdigit(wchar) || 
			(wchar>='A' && wchar<='Z') ||
			(wchar>='a' && wchar<='z');
	}

	Private WORD* pGlyphIndexToUnicode;
	Private WORD* pUnicodeToGlyphIndex;
	Private GCP_RESULTS gcpField;
	Private CString CMapFontNameField;
};

} //namespace