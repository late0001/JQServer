// LogView.cpp : ʵ���ļ�
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
	{_T("����ʱ��"),	    182	},
	{_T("�¼���¼"),	    673	}
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


// CLogView ���

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


// CLogView ��Ϣ�������
BOOL CLogView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: �ڴ����ר�ô����/����û���
	cs.style |=  LVS_REPORT;
	return CListView::PreCreateWindow(cs);
}
void CLogView::InsertLogItem(LPCTSTR Text,int Mode, int Flag)
{
	char m_Text[512] = {0};
	CTime time = CTime::GetCurrentTime();		//����CTime���� 

	CString strTime = time.Format(" %Y-%m-%d %H:%M:%S");
	m_pLogList->InsertItem(0, strTime, Flag);//int InsertItem( int nItem, LPCTSTR lpszItem, int nImage );
	m_pLogList->SetItemText(0, 1, Text);  //BOOL SetItemText( int nItem, int nSubItem, LPTSTR lpszText );
	switch(Mode)
	{
	    case 0:
		{
			m_pLogList->SetTextColor(RGB(0,100,250));//��ɫ
		}
		break;	
		case 1:
		{
			m_pLogList->SetTextColor(RGB(255,0,0));//��ɫ
		}
		break;
		case 2:
		{
			m_pLogList->SetTextColor(RGB(0,180,250));//����ɫ
		}
		break;
		case 3:
		{
			m_pLogList->SetTextColor(RGB(200,0,200));//��ɫ
		}
		break;
		case 4:
		{
			m_pLogList->SetTextColor(RGB(0,20,100));//ò�ƺ�ɫ
		}
		break;
		
	default:
		m_pLogList->SetTextColor(RGB(0,100,255));//��ɫ
		break;
	}
}


int CLogView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CListView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  �ڴ������ר�õĴ�������
	m_pLogList = &GetListCtrl();

	I_LogList.Create(16, 16, ILC_COLOR32|ILC_MASK,10, 0);
	HICON hIcon = NULL;//����Ϊ����3��ͼ����Դ
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
		g_Log_Width += g_Log_Data[i].nWidth; // �ܿ��
	}
	return 0;
}



