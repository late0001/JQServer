#pragma once


// CLogView ��ͼ

class CLogView : public CListView
{
	DECLARE_DYNCREATE(CLogView)

public:
	CLogView();           // ��̬������ʹ�õ��ܱ����Ĺ��캯��
	virtual ~CLogView();

public:
	void InsertLogItem(LPCTSTR Text,int Mode, int Flag);
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	DECLARE_MESSAGE_MAP()
private:
	CListCtrl* m_pLogList;
	CImageList I_LogList;
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
};


