// PcView.cpp : 实现文件
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
	{_T("网络"),			70	},  //56
	{_T("外网IP"),			103	},
	{_T("内网IP"),			103	},
	{_T("计算机名/备注"),	100	},
	{_T("操作系统"),		67	},
	{_T("CPU处理器"),		72	},
	{_T("硬盘/内存容量"),	98  },
	{_T("视频"),			42	},
	{_T("DDOS状态"),		60	},  //DOSS
	{_T("网络延时"),		65	},
	{_T("服务版本"),	    73	},
	{_T("服务端安装时间"),	110	},
	{_T("开机运行时间"),	92	},
	{_T("地理位置"),		175	}
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


// CPcView 诊断

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


// CPcView 消息处理程序


BOOL CPcView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此添加专用代码和/或调用基类
	cs.style |= LVS_REPORT;
	return CListView::PreCreateWindow(cs);
}


// int CPcView::OnCreate(LPCREATESTRUCT lpCreateStruct)
// {
// 	if (CListView::OnCreate(lpCreateStruct) == -1)
// 		return -1;
// 
// 	// TODO:  在此添加您专用的创建代码
// 	m_pListCtrl = &GetListCtrl();
// 
// 	I_ImageList.Create(16, 16, ILC_COLOR32 | ILC_MASK, 10, 0);
// 	HICON hIcon = NULL;// 以下为加入3个图标资源
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
// 	//|LVS_EX_HEADERDRAGDROP//报表头可以拖拽
// 	//|LVS_EX_ONECLICKACTIVATE//单击激活
// 	//|LVS_EX_GRIDLINES//绘制表格
// 	//|LVS_EX_FLATSB//扁平滚动条
// 	//|LVS_EX_MULTIWORKAREAS
// 	//| LVIF_IMAGE
// 	//| LVIF_PARAM //带复选框
// 	//| LVIF_TEXT
// 	//|LVIF_IMAGE
// 	//| LVIF_TEXT
// 	//LVS_EX_CHECKBOXES //带复选框
// 	//LVS_EX_TWOCLICKACTIVATE//双击激活
// 	|LVIS_STATEIMAGEMASK //带下划线
// 	);
// 	*/
// 	// 改变在线主机字体颜色
// 
// 	//m_pListCtrl->SetTextColor(RGB(237,96,61)); // 橘色显示
// 	//m_pListCtrl->SetTextColor(RGB(240,0,150)); //粉红显示
// 	m_pListCtrl->SetTextColor(RGB(0, 100, 255)); // 蓝色显示
// 
// 												 //	m_pListCtrl->SetTextBkColor(-1); 
// 
// 	for (int i = 0; i < g_Column_Count; i++)
// 	{
// 		m_pListCtrl->InsertColumn(i, g_Column_Data[i].title);
// 		m_pListCtrl->SetColumnWidth(i, g_Column_Data[i].nWidth);
// 		g_Column_Width += g_Column_Data[i].nWidth; // 总宽度
// 	}
// 	
// 	return 0;
// }


int CPcView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CListView::OnCreate(lpCreateStruct) == -1)
		return -1;

	//TODO:  在此添加您专用的创建代码
	m_pListCtrl = &GetListCtrl();

	I_ImageList.Create(16, 16, ILC_COLOR32 | ILC_MASK, 10, 0);
	HICON hIcon = NULL;// 以下为加入3个图标资源
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
	//|LVS_EX_HEADERDRAGDROP//报表头可以拖拽
	//|LVS_EX_ONECLICKACTIVATE//单击激活
	//|LVS_EX_GRIDLINES//绘制表格
	//|LVS_EX_FLATSB//扁平滚动条
	//|LVS_EX_MULTIWORKAREAS
	//| LVIF_IMAGE
	//| LVIF_PARAM //带复选框
	//| LVIF_TEXT
	//|LVIF_IMAGE
	//| LVIF_TEXT
	//LVS_EX_CHECKBOXES //带复选框
	//LVS_EX_TWOCLICKACTIVATE//双击激活
	|LVIS_STATEIMAGEMASK //带下划线
	);
	*/
	// 改变在线主机字体颜色

	//m_pListCtrl->SetTextColor(RGB(237,96,61)); // 橘色显示
	//m_pListCtrl->SetTextColor(RGB(240,0,150)); //粉红显示
	m_pListCtrl->SetTextColor(RGB(0, 100, 255)); // 蓝色显示

												 //	m_pListCtrl->SetTextBkColor(-1); 

	for (int i = 0; i < g_Column_Count; i++)
	{
		m_pListCtrl->InsertColumn(i, g_Column_Data[i].title);
		m_pListCtrl->SetColumnWidth(i, g_Column_Data[i].nWidth);
		g_Column_Width += g_Column_Data[i].nWidth; // 总宽度
	}
	return 0;
}
