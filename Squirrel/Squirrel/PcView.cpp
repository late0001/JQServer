// PcView.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Squirrel.h"
#include "PcView.h"


typedef struct
{
	TCHAR	*title;
	int		nWidth;
}COLUMNSTRUCT;

COLUMNSTRUCT g_Column_Data[20] =
{
	{_T("����"),			70	},  //56
	{_T("����IP"),			103	},
	{_T("����IP"),			103	},
	{_T("�������/��ע"),	100	},
	{_T("����ϵͳ"),		67	},
	{_T("CPU������"),		72	},
	{_T("Ӳ��/�ڴ�����"),	98  },
	{_T("��Ƶ"),			42	},
	{_T("DDOS״̬"),		60	},  //DOSS
	{_T("������ʱ"),		65	},
	{_T("����汾"),	    73	},
	{_T("����˰�װʱ��"),	110	},
	{_T("��������ʱ��"),	92	},
	{_T("����λ��"),		175	}
};

int g_Column_Width = 0;
int	g_Column_Count = (sizeof(g_Column_Data) / 8);
/////////////////////////////////////////////////////////////////////////////
// CPcView

IMPLEMENT_DYNCREATE(CPcView, CListView)

CPcView::CPcView()
{

}

CPcView::~CPcView()
{
}

BEGIN_MESSAGE_MAP(CPcView, CListView)
	ON_WM_CREATE()
	ON_WM_CREATE()
END_MESSAGE_MAP()


// CPcView ���

#ifdef _DEBUG
void CPcView::AssertValid() const
{
	CListView::AssertValid();
}

#ifndef _WIN32_WCE
void CPcView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}
#endif
#endif //_DEBUG


// CPcView ��Ϣ�������


BOOL CPcView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: �ڴ����ר�ô����/����û���
	cs.style |= LVS_REPORT;
	return CListView::PreCreateWindow(cs);
}


// int CPcView::OnCreate(LPCREATESTRUCT lpCreateStruct)
// {
// 	if (CListView::OnCreate(lpCreateStruct) == -1)
// 		return -1;
// 
// 	// TODO:  �ڴ������ר�õĴ�������
// 	m_pListCtrl = &GetListCtrl();
// 
// 	I_ImageList.Create(16, 16, ILC_COLOR32 | ILC_MASK, 10, 0);
// 	HICON hIcon = NULL;// ����Ϊ����3��ͼ����Դ
// 	hIcon = (HICON)::LoadImage(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_SHIPIN), IMAGE_ICON, 32, 32, 0);
// 	I_ImageList.Add(hIcon);
// 	hIcon = (HICON)::LoadImage(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_USER), IMAGE_ICON, 32, 32, 0);
// 	I_ImageList.Add(hIcon);
// 	ListView_SetImageList(m_pListCtrl->m_hWnd, I_ImageList, LVSIL_SMALL);
// 
// 	m_pListCtrl->SetExtendedStyle(LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT /*| LVS_EX_FLATSB*/ |
// 		LVS_EX_ONECLICKACTIVATE | LVS_EX_UNDERLINEHOT | LVS_EX_SUBITEMIMAGES | LVS_EX_GRIDLINES);
// 	/*
// 	m_pListCtrl->SetExtendedStyle(
// 	LVS_EX_FULLROWSELECT
// 	//|LVS_EX_HEADERDRAGDROP//����ͷ������ק
// 	//|LVS_EX_ONECLICKACTIVATE//��������
// 	//|LVS_EX_GRIDLINES//���Ʊ��
// 	//|LVS_EX_FLATSB//��ƽ������
// 	//|LVS_EX_MULTIWORKAREAS
// 	//| LVIF_IMAGE
// 	//| LVIF_PARAM //����ѡ��
// 	//| LVIF_TEXT
// 	//|LVIF_IMAGE
// 	//| LVIF_TEXT
// 	//LVS_EX_CHECKBOXES //����ѡ��
// 	//LVS_EX_TWOCLICKACTIVATE//˫������
// 	|LVIS_STATEIMAGEMASK //���»���
// 	);
// 	*/
// 	// �ı���������������ɫ
// 
// 	//m_pListCtrl->SetTextColor(RGB(237,96,61)); // ��ɫ��ʾ
// 	//m_pListCtrl->SetTextColor(RGB(240,0,150)); //�ۺ���ʾ
// 	m_pListCtrl->SetTextColor(RGB(0, 100, 255)); // ��ɫ��ʾ
// 
// 												 //	m_pListCtrl->SetTextBkColor(-1); 
// 
// 	for (int i = 0; i < g_Column_Count; i++)
// 	{
// 		m_pListCtrl->InsertColumn(i, g_Column_Data[i].title);
// 		m_pListCtrl->SetColumnWidth(i, g_Column_Data[i].nWidth);
// 		g_Column_Width += g_Column_Data[i].nWidth; // �ܿ��
// 	}
// 	
// 	return 0;
// }


int CPcView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CListView::OnCreate(lpCreateStruct) == -1)
		return -1;

	//TODO:  �ڴ������ר�õĴ�������
	m_pListCtrl = &GetListCtrl();

	I_ImageList.Create(16, 16, ILC_COLOR32 | ILC_MASK, 10, 0);
	HICON hIcon = NULL;// ����Ϊ����3��ͼ����Դ
	hIcon = (HICON)::LoadImage(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_SHIPIN), IMAGE_ICON, 32, 32, 0);
	I_ImageList.Add(hIcon);
	hIcon = (HICON)::LoadImage(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_USER), IMAGE_ICON, 32, 32, 0);
	I_ImageList.Add(hIcon);
	ListView_SetImageList(m_pListCtrl->m_hWnd, I_ImageList, LVSIL_SMALL);

	m_pListCtrl->SetExtendedStyle(LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT /*| LVS_EX_FLATSB*/ |
		LVS_EX_ONECLICKACTIVATE | LVS_EX_UNDERLINEHOT /*| LVS_EX_SUBITEMIMAGES*/ | LVS_EX_GRIDLINES);
	/*
	m_pListCtrl->SetExtendedStyle(
	LVS_EX_FULLROWSELECT
	//|LVS_EX_HEADERDRAGDROP//����ͷ������ק
	//|LVS_EX_ONECLICKACTIVATE//��������
	//|LVS_EX_GRIDLINES//���Ʊ��
	//|LVS_EX_FLATSB//��ƽ������
	//|LVS_EX_MULTIWORKAREAS
	//| LVIF_IMAGE
	//| LVIF_PARAM //����ѡ��
	//| LVIF_TEXT
	//|LVIF_IMAGE
	//| LVIF_TEXT
	//LVS_EX_CHECKBOXES //����ѡ��
	//LVS_EX_TWOCLICKACTIVATE//˫������
	|LVIS_STATEIMAGEMASK //���»���
	);
	*/
	// �ı���������������ɫ

	//m_pListCtrl->SetTextColor(RGB(237,96,61)); // ��ɫ��ʾ
	//m_pListCtrl->SetTextColor(RGB(240,0,150)); //�ۺ���ʾ
	m_pListCtrl->SetTextColor(RGB(0, 100, 255)); // ��ɫ��ʾ

												 //	m_pListCtrl->SetTextBkColor(-1); 

	for (int i = 0; i < g_Column_Count; i++)
	{
		m_pListCtrl->InsertColumn(i, g_Column_Data[i].title);
		m_pListCtrl->SetColumnWidth(i, g_Column_Data[i].nWidth);
		g_Column_Width += g_Column_Data[i].nWidth; // �ܿ��
	}
	return 0;
}
