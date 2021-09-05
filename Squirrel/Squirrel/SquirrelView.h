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

// SquirrelView.h : CSquirrelView 类的接口
//

#pragma once


class CSquirrelView : public CView
{
protected: // 仅从序列化创建
	CSquirrelView();
	DECLARE_DYNCREATE(CSquirrelView)

// 特性
public:
	CSquirrelDoc* GetDocument() const;
	CTabCtrl m_wndTabControl;
HWND	TCItem_GetHandle(int index);			//返回指定页的句柄
	//listCWnd list_wnd;
	LONG l_Botton;
	LONG l_Split_C;
	int  TabIndex;
// 操作
public:
	BOOL	AddGroup(LPCTSTR lpszTitle);
// 重写

public:
	virtual void OnDraw(CDC* pDC);  // 重写以绘制该视图
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// 实现
public:
	virtual ~CSquirrelView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 生成的消息映射函数
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
private:
	BOOL AddView(CRuntimeClass* pViewClass, LPCTSTR lpszTitle);
public:
	virtual void OnInitialUpdate();
	afx_msg void OnSize(UINT nType, int cx, int cy);
};

#ifndef _DEBUG  // SquirrelView.cpp 中的调试版本
inline CSquirrelDoc* CSquirrelView::GetDocument() const
   { return reinterpret_cast<CSquirrelDoc*>(m_pDocument); }
#endif

