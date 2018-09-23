/*
 * PROJECT:     PAINT for ReactOS
 * LICENSE:     LGPL
 * FILE:        base/applications/mspaint/scrollbox.cpp
 * PURPOSE:     Functionality surrounding the scroll box window class
 * PROGRAMMERS: Benedikt Freisen
 */

/* INCLUDES *********************************************************/

#include "precomp.h"
#include <atltypes.h>

/*
 * Scrollbar functional modes:
 * 0  view < canvas
 * 1  view < canvas + scroll
 * 2  view >= canvas + scroll
 *
 * Matrix of scrollbar presence (VERTICAL,HORIZONTAL) given by
 * vertical x horizontal scrollbar modes (view:whole report):
 *
 *           horizontal mode
 *             |      0      |      1      |      2
 * vertical ---+-------------+-------------+------------
 *   mode    0 |  TRUE,TRUE  |  TRUE,TRUE  |  TRUE,FALSE
 *          ---+-------------+-------------+------------
 *           1 |  TRUE,TRUE  | FALSE,FALSE | FALSE,FALSE
 *          ---+-------------+-------------+------------
 *           2 | FALSE,TRUE  | FALSE,FALSE | FALSE,FALSE
 */

struct ScrollbarPresence
{
    BOOL bVert;
    BOOL bHoriz;
};

CONST ScrollbarPresence sp_mx[3][3] =
{
    { {  TRUE,TRUE  }, {  TRUE,TRUE  }, {  TRUE,FALSE } },
    { {  TRUE,TRUE  }, { FALSE,FALSE }, { FALSE,FALSE } },
    { { FALSE,TRUE  }, { FALSE,FALSE }, { FALSE,FALSE } }
};


/* FUNCTIONS ********************************************************/

void
UpdateScrollbox()
{
    CONST INT EXTRASIZE = 5; /* 3 px of selection markers + 2 px of border */

    CRect tempRect;
    CSize sizeImageArea;
    CSize sizeScrollBox;
    INT vmode, hmode;
    SCROLLINFO si;

    scrollboxWindow.GetWindowRect(&tempRect);
    sizeScrollBox = CSize(tempRect.Width(), tempRect.Height());

    imageArea.GetClientRect(&tempRect);
    sizeImageArea = CSize(tempRect.Width(), tempRect.Height());
    sizeImageArea += CSize(EXTRASIZE * 2, EXTRASIZE * 2);

    /* show/hide the scrollbars */
    vmode = (sizeScrollBox.cy < sizeImageArea.cy? 0 :
                (sizeScrollBox.cy < sizeImageArea.cy +
                    ::GetSystemMetrics(SM_CYHSCROLL) ? 1 : 2));
    hmode = (sizeScrollBox.cx < sizeImageArea.cx ? 0 :
                (sizeScrollBox.cx < sizeImageArea.cx +
                    ::GetSystemMetrics(SM_CXVSCROLL) ? 1 : 2));
    scrollboxWindow.ShowScrollBar(SB_VERT, sp_mx[vmode][hmode].bVert);
    scrollboxWindow.ShowScrollBar(SB_HORZ, sp_mx[vmode][hmode].bHoriz);

    si.cbSize = sizeof(SCROLLINFO);
    si.fMask  = SIF_PAGE | SIF_RANGE;
    si.nMin   = 0;

    si.nMax   = sizeImageArea.cx +
                    (sp_mx[vmode][hmode].bVert == TRUE ?
                        ::GetSystemMetrics(SM_CYHSCROLL) : 0);
    si.nPage  = sizeScrollBox.cx;
    scrollboxWindow.SetScrollInfo(SB_HORZ, &si);

    si.nMax   = sizeImageArea.cy +
                    (sp_mx[vmode][hmode].bHoriz == TRUE ?
                        ::GetSystemMetrics(SM_CXHSCROLL) : 0);
    si.nPage  = sizeScrollBox.cy;
    scrollboxWindow.SetScrollInfo(SB_VERT, &si);

    scrlClientWindow.MoveWindow(-scrollboxWindow.GetScrollPos(SB_HORZ),
                                -scrollboxWindow.GetScrollPos(SB_VERT),
                                sizeImageArea.cx, sizeImageArea.cy, TRUE);
}

LRESULT CScrollboxWindow::OnSize(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if (m_hWnd == scrollboxWindow.m_hWnd)
    {
        UpdateScrollbox();
    }
    return 0;
}

LRESULT CScrollboxWindow::OnHScroll(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if (m_hWnd == scrollboxWindow.m_hWnd)
    {
        SCROLLINFO si;
        si.cbSize = sizeof(SCROLLINFO);
        si.fMask = SIF_ALL;
        scrollboxWindow.GetScrollInfo(SB_HORZ, &si);
        switch (LOWORD(wParam))
        {
            case SB_THUMBTRACK:
            case SB_THUMBPOSITION:
                si.nPos = HIWORD(wParam);
                break;
            case SB_LINELEFT:
                si.nPos -= 5;
                break;
            case SB_LINERIGHT:
                si.nPos += 5;
                break;
            case SB_PAGELEFT:
                si.nPos -= si.nPage;
                break;
            case SB_PAGERIGHT:
                si.nPos += si.nPage;
                break;
        }
        scrollboxWindow.SetScrollInfo(SB_HORZ, &si);
        scrlClientWindow.MoveWindow(-scrollboxWindow.GetScrollPos(SB_HORZ),
                   -scrollboxWindow.GetScrollPos(SB_VERT), imageModel.GetWidth() * toolsModel.GetZoom() / 1000 + 6,
                   imageModel.GetHeight() * toolsModel.GetZoom() / 1000 + 6, TRUE);
    }
    return 0;
}

LRESULT CScrollboxWindow::OnVScroll(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if (m_hWnd == scrollboxWindow.m_hWnd)
    {
        SCROLLINFO si;
        si.cbSize = sizeof(SCROLLINFO);
        si.fMask = SIF_ALL;
        scrollboxWindow.GetScrollInfo(SB_VERT, &si);
        switch (LOWORD(wParam))
        {
            case SB_THUMBTRACK:
            case SB_THUMBPOSITION:
                si.nPos = HIWORD(wParam);
                break;
            case SB_LINEUP:
                si.nPos -= 5;
                break;
            case SB_LINEDOWN:
                si.nPos += 5;
                break;
            case SB_PAGEUP:
                si.nPos -= si.nPage;
                break;
            case SB_PAGEDOWN:
                si.nPos += si.nPage;
                break;
        }
        scrollboxWindow.SetScrollInfo(SB_VERT, &si);
        scrlClientWindow.MoveWindow(-scrollboxWindow.GetScrollPos(SB_HORZ),
                   -scrollboxWindow.GetScrollPos(SB_VERT), imageModel.GetWidth() * toolsModel.GetZoom() / 1000 + 6,
                   imageModel.GetHeight() * toolsModel.GetZoom() / 1000 + 6, TRUE);
    }
    return 0;
}

LRESULT CScrollboxWindow::OnLButtonDown(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    selectionWindow.ShowWindow(SW_HIDE);
    pointSP = 0;    // resets the point-buffer of the polygon and bezier functions
    return 0;
}
