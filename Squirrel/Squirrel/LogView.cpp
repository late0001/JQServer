// LogView.cpp : 实现文件
//

#include "stdafx.h"
#include "Squirrel.h"
#include "LogView.h"


CLogView* g_pLogView;

typedef struct
{
	TCHAR	*title;
	int		nWidth;
}COLUMNSTRUCT;

COLUMNSTRUCT g_Log_Data[] = 
{
	{_T("发生时间"),	    182	},
	{_T("事件记录"),	    673	}
};

int g_Log_Width = 0;
int	g_Log_Count = (sizeof(g_Log_Data) / 8);
/////////////////////////////////////////////////////////////////////////////
// CLogView

IMPLEMENT_DYNCREATE(CLogView, CListView)

CLogView::CLogView()
{

}

CLogView::~CLogView()
{
}

BEGIN_MESSAGE_MAP(CLogView, CListView)
	ON_WM_CREATE()
END_MESSAGE_MAP()


// CLogView 诊断

#ifdef _DEBUG
void CLogView::AssertValid() const
{
	CListView::AssertValid();
}

#ifndef _WIN32_WCE
void CLogView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}
#endif
#endif //_DEBUG


// CLogView 消息处理程序
BOOL CLogView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此添加专用代码和/或调用基类
	cs.style |=  LVS_REPORT;
	return CListView::PreCreateWindow(cs);
}
void CLogView::InsertLogItem(LPCTSTR Text,int Mode, int Flag)
{
	char m_Text[512] = {0};
	CTime time = CTime::GetCurrentTime();		//构造CTime对象 

	CString strTime = time.Format(" %Y-%m-%d %H:%M:%S");
	m_pLogList->InsertItem(0, strTime, Flag);//int InsertItem( int nItem, LPCTSTR lpszItem, int nImage );
	m_pLogList->SetItemText(0, 1, Text);  //BOOL SetItemText( int nItem, int nSubItem, LPTSTR lpszText );
	switch(Mode)
	{
	    case 0:
		{
			m_pLogList->SetTextColor(RGB(0,100,250));//蓝色
		}
		break;	
		case 1:
		{
			m_pLogList->SetTextColor(RGB(255,0,0));//红色
		}
		break;
		case 2:
		{
			m_pLogList->SetTextColor(RGB(0,180,250));//蓝绿色
		}
		break;
		case 3:
		{
			m_pLogList->SetTextColor(RGB(200,0,200));//粉色
		}
		break;
		case 4:
		{
			m_pLogList->SetTextColor(RGB(0,20,100));//貌似黑色
		}
		break;
		
	default:
		m_pLogList->SetTextColor(RGB(0,100,255));//蓝色
		break;
	}
}


int CLogView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CListView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  在此添加您专用的创建代码
	m_pLogList = &GetListCtrl();

	I_LogList.Create(16, 16, ILC_COLOR32|ILC_MASK,10, 0);
	HICON hIcon = NULL;//以下为加入3个图标资源
	hIcon = (HICON)::LoadImage(::AfxGetInstanceHandle(),MAKEINTRESOURCE(IDI_EVENT_INFO), IMAGE_ICON, 16, 16, 0);
	I_LogList.Add(hIcon);
	hIcon = (HICON)::LoadImage(::AfxGetInstanceHandle(),MAKEINTRESOURCE(IDI_EVENT_ERROR), IMAGE_ICON, 16, 16, 0);
	I_LogList.Add(hIcon);
	hIcon = (HICON)::LoadImage(::AfxGetInstanceHandle(),MAKEINTRESOURCE(IDI_BE_CAREFUL), IMAGE_ICON, 16, 16, 0);
	I_LogList.Add(hIcon);
	ListView_SetImageList(m_pLogList->m_hWnd, I_LogList, LVSIL_SMALL);

	m_pLogList->SetExtendedStyle(/*LVIF_PARAM |*/ LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP|/*LVS_EX_FLATSB|*/
		LVS_EX_ONECLICKACTIVATE | LVS_EX_UNDERLINEHOT |LVS_EX_SUBITEMIMAGES |LVS_EX_GRIDLINES);

	for (int i = 0; i < g_Log_Count; i++)
	{	
		m_pLogList->InsertColumn(i, g_Log_Data[i].title);
		m_pLogList->SetColumnWidth(i, g_Log_Data[i].nWidth);
		g_Log_Width += g_Log_Data[i].nWidth; // 总宽度
	}
	return 0;
}



