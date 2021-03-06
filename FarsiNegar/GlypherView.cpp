// GlypherView.cpp : implementation of the CGlypherView class
//

#include "stdafx.h"
#include "Glypher.h"
#include "Wingdi.h"
#include "MainFrm.h"

#include "GlypherDoc.h"
#include "GlypherView.h"
#include "glypherview.h"

// CGlypherView
IMPLEMENT_DYNCREATE(CGlypherView, CScrollView)

BEGIN_MESSAGE_MAP(CGlypherView, CScrollView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CScrollView::OnFilePrintPreview)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_CONTEXTMENU()
	ON_WM_LBUTTONUP()
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEWHEEL()
END_MESSAGE_MAP()

// CGlypherView construction/destruction
CGlypherView::CGlypherView()
: m_bStartWrapSize(false)
{
	m_rcGuide.SetRectEmpty();
}

CGlypherView::~CGlypherView()
{
}

BOOL CGlypherView::PreCreateWindow(CREATESTRUCT& cs)
{
	BOOL ret = CScrollView::PreCreateWindow(cs);
	return ret;
}

void Test(HDC hdc)
{
	CString text = L"می";

	SCRIPT_ITEM si[100];
	int citem;
	HRESULT hr = ScriptItemize(L"", text.GetLength(), 100, NULL, NULL, si, &citem);

	SCRIPT_CACHE sc = NULL;
	WORD out[200]={0};
	WORD log[200]={0};
	SCRIPT_VISATTR sv[200] = {0};
	int clyph;
	hr = ScriptShape(hdc, &sc, text, text.GetLength(), 100, &si[0].a, out, log, sv, &clyph);
	CString ss((LPCWSTR)out, 2);
	m::Clipboard::CopyText(ss);
	UNREFERENCED_PARAMETER(hr);
}

void CGlypherView::Draw(HDC dcHandle)
{
	CRect rc(0,0,GetWrapSize(), 1000);
	rc.DeflateRect(5,0,5,0);
	rc.top += 20;
	CPoint pt(rc.left, 40);

	FontMapsPtr pfontMaps = GetManager()->FontMapsGet();
	CString text = GetInputText();

	POSITION pos = pfontMaps->GetHeadPosition();
	while (pos!=NULL)
	{
		FontMapPtr pfontMap = pfontMaps->GetNext(pos);
		bool quickMode = GetAppOptions()->QuickFontMapName==pfontMap->NameGet();
		bool generalMode = GetAppOptions()->QuickFontMapName.IsEmpty();
		bool bShow = quickMode || (generalMode && GetAppOptions()->IsFontMapEnabled(pfontMap));

		//check options
		if ( bShow )
		{
			CSize sizeSection = DrawFontMapSection(dcHandle, pt, pfontMap);
			pt.y += sizeSection.cy;
			CSize sizeText = DrawFontMapText(dcHandle, text, pt, pfontMap);
			pt.y += sizeText.cy + 20;
		}
	}

		//y+=100;
		//y += DrawFontMapSection(pdc, CPoint(0,y), pfontMap).cy;
		//ExtTextOut(*pdc, 0, y, 0*ETO_GLYPH_INDEX | ETO_IGNORELANGUAGE, NULL, (LPCWSTR)glyphs, glyphs.GetLength(), NULL);
		//pdc->SelectObject(odlFontHandle);

	//draw ruler
	DrawRuler(dcHandle);

	//draw guide line
	DrawWrapGuide(dcHandle); 
}

// CGlypherView drawing
void CGlypherView::OnDraw(CDC* pdc)
{
	//CSize size(5000,5000);
	//HDC memDC = CreateCompatibleDC(*pdc);
	//HBITMAP bmpHandle = CreateCompatibleBitmap(*pdc, size.cx, size.cy);
	//HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, bmpHandle);
	//m::DC_FillSolidRect(memDC, CRect(0,0,size.cx,size.cy), GetSysColor(COLOR_WINDOW));
	
	Draw(*pdc);
	
	//Draw(memDC);
	//BitBlt(*pdc, 0, 0, size.cx, size.cy, memDC, 0, 0, SRCCOPY);
	//SelectObject(memDC, oldBitmap);
	//DeleteObject(oldBitmap);
	//DeleteDC(memDC);
}

CGlypherDoc* CGlypherView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CGlypherDoc)));
	return (CGlypherDoc*)m_pDocument;
}

void CGlypherView::DrawWrapGuide(HDC dcHandle)
{
	if ( GetAppOptions()->AlignMode==GlypherManager::AlignNone )
		return;

	//draw guide line
	CRect rect(GetWrapSize(), 0, GetWrapSize()+1, GetTotalSize().cy);
	m::DC_FillSolidRect(dcHandle, rect, RGB(0,0,255));
}

void CGlypherView::DrawRulerGuide(HDC dcHandle, CPoint ptMouse)
{
	// clean old position
	m::DC_FillSolidRect(dcHandle, m_rcGuide, GetBkColor(dcHandle));

	// draw rulder
	OnPrepareDC(CDC::FromHandle(dcHandle));
	DrawRuler(dcHandle);

	// draw mouse guide
	CRect rcRuler = GetRulerRect();
	m_rcGuide.SetRect(ptMouse.x, rcRuler.top, ptMouse.x, rcRuler.bottom);
	Pos_ToClient(&m_rcGuide);
	m_rcGuide.right+=1;
	m::DC_FillSolidRect(dcHandle, m_rcGuide, RGB(255,0,0));
}

void CGlypherView::DrawRuler(HDC dcHandle)
{
	CDC* pdc = CDC::FromHandle(dcHandle);
	
	//draw rulder
	CRect rcLine = GetRulerRect();

	//pdc->MoveTo(rcLine.left, rcLine.bottom);
	//pdc->LineTo(rcLine.right, rcLine.bottom);

	pdc->SetMapMode(MM_LOMETRIC);
	pdc->DPtoLP(rcLine);
	pdc->FillSolidRect(rcLine, pdc->GetBkColor());

	//m::DC_FillSolidRect(dcHandle, rcLine, RGB(255,255,255));
	pdc->MoveTo(rcLine.left, rcLine.bottom);
	pdc->LineTo(rcLine.right, rcLine.bottom);

	rcLine.left=0;
	for (int i=0; i<500; i++)
	{

		pdc->MoveTo(rcLine.left+i*100, rcLine.bottom);
		pdc->LineTo(rcLine.left+i*100, rcLine.top);

		pdc->MoveTo(rcLine.left+i*50, rcLine.bottom);
		pdc->LineTo(rcLine.left+i*50, rcLine.top+rcLine.Height()/2);
	}
	pdc->SetMapMode(MM_TEXT);
}

void CGlypherView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();
	SetScrollSizes(MM_TEXT, CSize(5000, 5000));//, CSize(100,100));
}

void CGlypherView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	//update Layout_RTL
	bool viewRTL = _TOBOOL(GetExStyle() & WS_EX_LAYOUTRTL);
	bool editRTL = GetInputRtl();
	if (viewRTL!=editRTL) 
	{
		if (editRTL)
			ModifyStyleEx(0, WS_EX_LAYOUTRTL);
		else
			ModifyStyleEx(WS_EX_LAYOUTRTL, 0);
	}

	//call base member to take effect
	CScrollView::OnUpdate(pSender, lHint, pHint);
}

void CGlypherView::OnMouseMove(UINT nFlags, CPoint point)
{
	CScrollView::OnMouseMove(nFlags, point);
					    
	//draw rulder guide at top
	CClientDC dc(this);
	OnPrepareDC(&dc);
	Pos_ToLogical(&point);
	DrawRulerGuide(dc, point);

	//draw guide and reposition text if l-button is down
	GetWrapSize() = max(GetWrapSize(), 50);
	if ( m_bStartWrapSize && _TOBOOL(nFlags & MK_LBUTTON))
	{
		GetWrapSize() = max(point.x, 50);
		Invalidate();
	}

}

void CGlypherView::OnLButtonDown(UINT nFlags, CPoint point)
{
	CScrollView::OnLButtonDown(nFlags, point);
	if (point.y>GetRulerRect().Height()) return;
	m_bStartWrapSize = true;
	SetCapture();
	
	//update wrap position
	Pos_ToLogical(&point);
	GetWrapSize() = point.x;
    Invalidate();
}

int& CGlypherView::GetWrapSize()
{
	return GetAppOptions()->AlignSize;
}

void CGlypherView::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_bStartWrapSize = false;
	ReleaseCapture();
	CScrollView::OnLButtonUp(nFlags, point);
}

void CGlypherView::Pos_ToClient(CRect* pRect)
{
	CPoint pt1 = pRect->TopLeft();
	CPoint pt2 = pRect->BottomRight();
	Pos_ToClient(&pt1);
	Pos_ToClient(&pt2);
	pRect->SetRect(pt1, pt2);
}

void CGlypherView::Pos_ToClient(CPoint* pPoint)
{
	pPoint->x = pPoint->x - GetScrollPosition().x;
	pPoint->y = pPoint->y - GetScrollPosition().y;
}

void CGlypherView::Pos_ToLogical(CPoint* pPoint)
{
	//get position
	pPoint->x = pPoint->x + GetScrollPosition().x;
	pPoint->y = pPoint->y + GetScrollPosition().y;
}

CRect CGlypherView::GetRulerRect()
{
	CRect rc(0, 0, GetTotalSize().cx, 13);
	return rc;
}

void CGlypherView::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	if (point.x==-1 && point.y==-1)
	{
		CRect rc;
		GetWindowRect(rc);
		point=rc.CenterPoint();
		point.Offset(0,0);
	}

	UINT nFlags = IsAppRTL() ? TPM_RIGHTBUTTON : 0;
	GetMainWnd()->pMenuManager->GetContextView()->TrackPopupMenu(
		nFlags,
		point.x,
		point.y,
		GetMainWnd());
}

BOOL CGlypherView::OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll)
{
	BOOL ret = CScrollView::OnScroll(nScrollCode, nPos, bDoScroll);
	Invalidate();
	return ret;
}

CSize CGlypherView::DrawFontMapText(HDC dcHandle, CString text, CPoint point, FontMapPtr pfontMap)
{
	CSize ret(0,0);
	
	//create font
	HFONT fontHandle = GetAppOptions()->GetFontForFontMap(pfontMap)->HandleGet();
	HFONT odlFontHandle = (HFONT)SelectObject(dcHandle, fontHandle);
	
	CString glyphs = GetManager()->WindowsTextToFontMap(text, pfontMap->NameGet(), fontHandle, GetAppOptions()->AlignSize, GetAppOptions()->AlignMode, GetInputRtl());

	//draw lines
	StringListPtr plines = GlypherUtil::String_GetLines(glyphs);
	POSITION pos = plines->GetHeadPosition();
	while (pos!=NULL)
	{
		CString line = plines->GetNext(pos);
		CSize size = DrawGlyphsLine(dcHandle, line, point);
		point.y += size.cy;
		ret.cx = max(ret.cx, size.cx);
		ret.cy += size.cy;
	}

	//clean up
	SelectObject(dcHandle, odlFontHandle);
	return ret;
}

CSize CGlypherView::DrawGlyphsLine(HDC dcHandle, CString glyphs, CPoint point)
{	
	//process for empty line; empty line has y but hast not x
	if ( glyphs.IsEmpty() )
	{
		CSize size;
		GlypherUtil::String_GetTextSize(dcHandle, L" ", &size);
		size.cx = 0;
		return size;
	}

	//convert text to glyphs
	int cbOutGlyphs = glyphs.GetLength();
	WORD* pOutGlyphs = new WORD[cbOutGlyphs];
	memset(pOutGlyphs, 0, cbOutGlyphs*sizeof WORD);
	SCRIPT_CACHE sc=NULL;
	ScriptGetCMap(dcHandle, &sc, (LPCWSTR)glyphs, glyphs.GetLength(), 0*SGCM_RTL, pOutGlyphs);
	
	//draw text
	BOOL res = ExtTextOut(dcHandle, point.x, point.y, ETO_GLYPH_INDEX, NULL, (LPCWSTR)pOutGlyphs, cbOutGlyphs, NULL);
	CSize size(0,0);
	if ( res )
		res = GlypherUtil::String_GetTextSize(dcHandle, glyphs, &size);

	delete pOutGlyphs;
	return (res) ? size : CSize(0,0);;
}


CSize CGlypherView::DrawFontMapSection(HDC dcHandle, CPoint point, FontMapPtr pfontMap)
{
	CDC* pdc = CDC::FromHandle(dcHandle);
	CSize ret;
	COLORREF clrOldColor = pdc->SetTextColor( RGB(255,0,0) );

	//select tahoma font
	CFont font;
	font.CreatePointFont(80, _T("tahoma"), pdc);
	CFont* pOldFont = pdc->SelectObject( &font );

    //draw name
	CString string;
	CString strFormat; strFormat.LoadString(AfxGetInstanceHandle(), IDS_PREVIEW_FONTMAP);
	LOGFONT lf = {0};
	HFONT fontMapFontHandle = GetAppOptions()->GetFontForFontMap(pfontMap)->HandleGet();
	if (fontMapFontHandle!=NULL)
		mOld::Font_GetLogfont(fontMapFontHandle, &lf);
	string.Format(strFormat, (LPCTSTR)pfontMap->Title, (LPCTSTR)lf.lfFaceName, lf.lfHeight);
	pdc->TextOut(point.x, point.y, string );
	ret = pdc->GetTextExtent(string);

	CRect rcLine(point.x, point.y+ret.cy+3, point.x + ret.cx, point.y + ret.cy + 4);
	m::DC_FillSolidRect(dcHandle, rcLine, RGB(255,0,0));
	ret.cy += rcLine.Height() + 3;

	//clean up
	pdc->SelectObject( pOldFont );
	pdc->SetTextColor( clrOldColor );
	return ret;
}

DWORD CGlypherView::GetViewFlags(void)
{
	DWORD dwFlags=0;
	//
	//switch ( GetAppOptions()->AlignMode )
	//{
	//case GlypherManager::AlignKashida:
	//	dwFlags = UnicodeGlyphConverter::FlagBreak | UnicodeGlyphConverter::FlagKashida;
	//	break;
	//
	//case GlypherManager::AlignKashidaFull:
	//	dwFlags = UnicodeGlyphConverter::FlagBreak | UnicodeGlyphConverter::FlagKashida | UnicodeGlyphConverter::FlagKashidaFull;
	//	break;

	//case GlypherManager::AlignWrap:
	//	dwFlags = UnicodeGlyphConverter::FlagBreak;
	//	break;

	//case GlypherManager::AlignNone:
	//	break;
	//}

	//dwFlags |= 	GetAppOptions()->m_bRightToLeft ? UnicodeGlyphConverter::FlagRightToLeft : 0;
	return dwFlags;
}

BOOL CGlypherView::OnEraseBkgnd(CDC* pdc)
{
	CRect rc;
	GetClientRect(rc);
	HDC memDC = CreateCompatibleDC(*pdc);
	HBITMAP bmpHandle = CreateCompatibleBitmap(*pdc, rc.Width(), rc.Height());
	HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, bmpHandle);
	m::DC_FillSolidRect(memDC, rc, GetSysColor(COLOR_WINDOW));

	//draw ruler
	DrawRuler(memDC);

	//draw guide line
	DrawWrapGuide(memDC); 

	BitBlt(*pdc, 0, 0, rc.Width(), rc.Height(), memDC, 0, 0, SRCCOPY);

	SelectObject(memDC, oldBitmap);
	TESTLASTERR(DeleteObject(bmpHandle));
	TESTLASTERR(DeleteDC(memDC));

	//m::DC_FillSolidRect(*pdc, rc, GetSysColor(COLOR_WINDOW));
	//return __super::OnEraseBkgnd(pdc);
	return TRUE;
}

BOOL CGlypherView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	BOOL res = DoMouseWheel(nFlags, zDelta, pt);
	Invalidate();
	return res;
}
