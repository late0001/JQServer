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

// SquirrelView.h : CSquirrelView ��Ľӿ�
//

#pragma once


class CSquirrelView : public CView
{
protected: // �������л�����
	CSquirrelView();
	DECLARE_DYNCREATE(CSquirrelView)

// ����
public:
	CSquirrelDoc* GetDocument() const;
	CTabCtrl m_wndTabControl;
HWND	TCItem_GetHandle(int index);			//����ָ��ҳ�ľ��
	//listCWnd list_wnd;
	LONG l_Botton;
	LONG l_Split_C;
	int  TabIndex;
// ����
public:
	BOOL	AddGroup(LPCTSTR lpszTitle);
// ��д

public:
	virtual void OnDraw(CDC* pDC);  // ��д�Ի��Ƹ���ͼ
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// ʵ��
public:
	virtual ~CSquirrelView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// ���ɵ���Ϣӳ�亯��
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

#ifndef _DEBUG  // SquirrelView.cpp �еĵ��԰汾
inline CSquirrelDoc* CSquirrelView::GetDocument() const
   { return reinterpret_cast<CSquirrelDoc*>(m_pDocument); }
#endif

