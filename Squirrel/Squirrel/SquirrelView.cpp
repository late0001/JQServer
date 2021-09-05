// ��� MFC ʾ��Դ������ʾ���ʹ�� MFC Microsoft Office Fluent �û����� 
// (��Fluent UI��)����ʾ�������ο���
// ���Բ��䡶Microsoft ������ο����� 
// MFC C++ ������渽����ص����ĵ���  
// ���ơ�ʹ�û�ַ� Fluent UI ����������ǵ����ṩ�ġ�  
// ��Ҫ�˽��й� Fluent UI ��ɼƻ�����ϸ��Ϣ������� 
// http://go.microsoft.com/fwlink/?LinkId=238214��
//
// ��Ȩ����(C) Microsoft Corporation
// ��������Ȩ����

// SquirrelView.cpp : CSquirrelView ���ʵ��
//

#include "stdafx.h"

// SHARED_HANDLERS ������ʵ��Ԥ��������ͼ������ɸѡ�������
// ATL ��Ŀ�н��ж��壬�����������Ŀ�����ĵ����롣
#ifndef SHARED_HANDLERS
#include "Squirrel.h"
#endif

#include "SquirrelDoc.h"
#include "SquirrelView.h"
#include "PcView.h"
#include "LogView.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


extern CLogView*	g_pLogView;
// CSquirrelView

IMPLEMENT_DYNCREATE(CSquirrelView, CView)

BEGIN_MESSAGE_MAP(CSquirrelView, CView)
	// ��׼��ӡ����
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CSquirrelView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

// CSquirrelView ����/����

CSquirrelView::CSquirrelView()
{
	// TODO: �ڴ˴���ӹ������

}

CSquirrelView::~CSquirrelView()
{
}

BOOL CSquirrelView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: �ڴ˴�ͨ���޸�
	//  CREATESTRUCT cs ���޸Ĵ��������ʽ

	return CView::PreCreateWindow(cs);
}

// CSquirrelView ����

void CSquirrelView::OnDraw(CDC* /*pDC*/)
{
	CSquirrelDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: �ڴ˴�Ϊ����������ӻ��ƴ���
}


// CSquirrelView ��ӡ


void CSquirrelView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CSquirrelView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// Ĭ��׼��
	return DoPreparePrinting(pInfo);
}

void CSquirrelView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: ��Ӷ���Ĵ�ӡǰ���еĳ�ʼ������
}

void CSquirrelView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: ��Ӵ�ӡ����е��������
}

void CSquirrelView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CSquirrelView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CSquirrelView ���

#ifdef _DEBUG
void CSquirrelView::AssertValid() const
{
	CView::AssertValid();
}

void CSquirrelView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CSquirrelDoc* CSquirrelView::GetDocument() const // �ǵ��԰汾��������
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CSquirrelDoc)));
	return (CSquirrelDoc*)m_pDocument;
}
#endif //_DEBUG

BOOL CSquirrelView::AddView(CRuntimeClass* pViewClass, LPCTSTR lpszTitle)
{
	CCreateContext contextT;
	contextT.m_pCurrentDoc = GetDocument();
	contextT.m_pNewViewClass = pViewClass;
	contextT.m_pNewDocTemplate = GetDocument()->GetDocTemplate();

	CWnd* pWnd;
	TRY
	{
		pWnd = (CListView*)pViewClass->CreateObject();
		if (pWnd == NULL)
		{
			AfxThrowMemoryException();
		}
	}
	CATCH_ALL(e)
	{
		TRACE0("Out of memory creating a view.\n");
		// Note: DELETE_EXCEPTION(e) not required
		return FALSE;
	}
	END_CATCH_ALL

	DWORD dwStyle = AFX_WS_DEFAULT_VIEW;
	dwStyle &= ~WS_BORDER;
	//dwStyle |= WS_POPUP;

	int nTab = m_wndTabControl.GetItemCount();

	// Create with the right size (wrong position)
	CRect rect(0, 0, 0, 0);
	UINT nID = AFX_IDW_PANE_FIRST + nTab;	//ID_LIST_PAG + 100 + nTab; 
	if (!pWnd->Create(NULL, NULL, dwStyle, rect, &m_wndTabControl, nID, &contextT))
		//if (!pWnd->Create(NULL, NULL, dwStyle, rect, this, nID, &contextT))
		//if (!pWnd->Create(NULL, NULL, dwStyle, rect, &m_wndTabControl, nID))
	{
		TRACE0("Warning: couldn't create client tab for view.\n");
		// pWnd will be cleaned up by PostNcDestroy
		return NULL;
	}


	TCITEM   item;
	item.mask = TCIF_TEXT;
	item.pszText = (LPTSTR)lpszTitle;
	item.lParam = (UINT)pWnd->m_hWnd;		//����ָ��ľ��
	item.mask = TCIF_TEXT | TCIF_PARAM;
	//m_wndTabControl.InsertItem(nTab, lpszTitle, pWnd->GetSafeHwnd());
	m_wndTabControl.InsertItem(nTab, &item);
	pWnd->SetOwner(this);

	return TRUE;

}

BOOL CSquirrelView::AddGroup(LPCTSTR lpszTitle)
{
	BOOL Result = AddView(RUNTIME_CLASS(CPcView), lpszTitle);
	return Result;
}

HWND CSquirrelView::TCItem_GetHandle(int index)
{
	TCITEM item;
	item.mask = TCIF_PARAM;
	item.lParam = 0;
	m_wndTabControl.GetItem(index, &item);
	return (HWND)item.lParam;
}
// CSquirrelView ��Ϣ�������
void CSquirrelView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	// TODO: �ڴ˴������Ϣ����������


	if (m_wndTabControl.GetSafeHwnd())
		m_wndTabControl.MoveWindow(0, 0, cx, cy-l_Botton-l_Split_C);

	CWnd *pWnd1;
	HWND hWnd = TCItem_GetHandle(TabIndex);
	pWnd1 = CWnd::FromHandle(hWnd);
	CRect rect1;
	m_wndTabControl.GetItemRect(0, &rect1);
	int i_tit_h = rect1.bottom - rect1.top;//tabҳ�߶�
	if (pWnd1) 	
		pWnd1->SetWindowPos(NULL, 0, 0, cx, cy-l_Botton-l_Split_C-i_tit_h, SWP_NOZORDER|SWP_NOMOVE);

	//log
	CWnd *pWnd2; 
	pWnd2 = GetDlgItem(ID_LIST_LOG);
	if (pWnd2)
	{				
		//pWnd2->SetWindowPos(NULL, 0, 0, cx, l_Botton, SWP_NOZORDER|SWP_NOMOVE);
		pWnd2->SetWindowPos(NULL, 0, cy - l_Botton,0,0, SWP_NOZORDER|SWP_NOSIZE);
	}
}
int CSquirrelView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  �ڴ������ר�õĴ�������
	l_Botton = 200;
	l_Split_C = 1;
	TabIndex = 0;
	m_wndTabControl.Create(WS_CHILD|WS_VISIBLE|TCS_BOTTOM , CRect(0, 0, 0, 0), this, IDC_TABCONTROL);
	AddGroup(_T("Ĭ�Ϸ���(0)"));	//��ǩ����
	m_wndTabControl.SetCurSel(0);
	g_pLogView     = new CLogView();
	
	g_pLogView->Create(NULL, _T(""), WS_VISIBLE | WS_CHILD | LVS_REPORT, CRect(0, 0, 0, 0), this, ID_LIST_LOG);

	return 0;
}


void CSquirrelView::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	// TODO: �ڴ����ר�ô����/����û���
	DWORD d_tcs = m_wndTabControl.GetExtendedStyle();
	d_tcs |= TCS_BUTTONS;
	d_tcs |= TCS_BOTTOM;
	m_wndTabControl.SetExtendedStyle(d_tcs, 0);
}



