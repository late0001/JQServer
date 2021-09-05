#pragma once


// CPcView 视图

class CPcView : public CListView
{
	DECLARE_DYNCREATE(CPcView)

protected:
	CPcView();           // 动态创建所使用的受保护的构造函数
	virtual ~CPcView();

public:
	CListCtrl *m_pListCtrl;
	CImageList   I_ImageList;
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	DECLARE_MESSAGE_MAP()
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
};


