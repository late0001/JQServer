// 这段 MFC 示例源代码演示如何使用 MFC Microsoft Office Fluent 用户界面 
// (“Fluent UI”)。该示例仅供参考，
// 用以补充《Microsoft 基础类参考》和 
// MFC C++ 库软件随附的相关电子文档。  
// 复制、使用或分发 Fluent UI 的许可条款是单独提供的。  
// 若要了解有关 Fluent UI 许可计划的详细信息，请访问 
// http://go.microsoft.com/fwlink/?LinkId=238214。
//
// 版权所有(C) Microsoft Corporation
// 保留所有权利。

// SquirrelView.cpp : CSquirrelView 类的实现
//

#include "stdafx.h"

// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
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
	// 标准打印命令
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CSquirrelView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

// CSquirrelView 构造/析构

CSquirrelView::CSquirrelView()
{
	// TODO: 在此处添加构造代码

}

CSquirrelView::~CSquirrelView()
{
}

BOOL CSquirrelView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式

	return CView::PreCreateWindow(cs);
}

// CSquirrelView 绘制

void CSquirrelView::OnDraw(CDC* /*pDC*/)
{
	CSquirrelDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: 在此处为本机数据添加绘制代码
}


// CSquirrelView 打印


void CSquirrelView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CSquirrelView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 默认准备
	return DoPreparePrinting(pInfo);
}

void CSquirrelView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加额外的打印前进行的初始化过程
}

void CSquirrelView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加打印后进行的清理过程
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


// CSquirrelView 诊断

#ifdef _DEBUG
void CSquirrelView::AssertValid() const
{
	CView::AssertValid();
}

void CSquirrelView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CSquirrelDoc* CSquirrelView::GetDocument() const // 非调试版本是内联的
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
	item.lParam = (UINT)pWnd->m_hWnd;		//保存指针的句柄
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
// CSquirrelView 消息处理程序
void CSquirrelView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	// TODO: 在此处添加消息处理程序代码


	if (m_wndTabControl.GetSafeHwnd())
		m_wndTabControl.MoveWindow(0, 0, cx, cy-l_Botton-l_Split_C);

	CWnd *pWnd1;
	HWND hWnd = TCItem_GetHandle(TabIndex);
	pWnd1 = CWnd::FromHandle(hWnd);
	CRect rect1;
	m_wndTabControl.GetItemRect(0, &rect1);
	int i_tit_h = rect1.bottom - rect1.top;//tab页高度
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

	// TODO:  在此添加您专用的创建代码
	l_Botton = 200;
	l_Split_C = 1;
	TabIndex = 0;
	m_wndTabControl.Create(WS_CHILD|WS_VISIBLE|TCS_BOTTOM , CRect(0, 0, 0, 0), this, IDC_TABCONTROL);
	AddGroup(_T("默认分组(0)"));	//标签名称
	m_wndTabControl.SetCurSel(0);
	g_pLogView     = new CLogView();
	
	g_pLogView->Create(NULL, _T(""), WS_VISIBLE | WS_CHILD | LVS_REPORT, CRect(0, 0, 0, 0), this, ID_LIST_LOG);

	return 0;
}


void CSquirrelView::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	// TODO: 在此添加专用代码和/或调用基类
	DWORD d_tcs = m_wndTabControl.GetExtendedStyle();
	d_tcs |= TCS_BUTTONS;
	d_tcs |= TCS_BOTTOM;
	m_wndTabControl.SetExtendedStyle(d_tcs, 0);
}



