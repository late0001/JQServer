#pragma once


// CPcView ��ͼ

class CPcView : public CListView
{
	DECLARE_DYNCREATE(CPcView)

protected:
	CPcView();           // ��̬������ʹ�õ��ܱ����Ĺ��캯��
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


