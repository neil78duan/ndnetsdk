#pragma once


#include "nd_xml.h"
#include "ctrls/GridCtrl_src/GridCtrl.h"
#include "ctrls/GradShow.h"

BOOL ND_SetChildrenFont(HWND hwnd,bool bRedraw,int nSize, LPCTSTR lpszName)  ;

#include <string>
#include <vector>
using namespace std ;

// CXMLParserWnd window
class CXMLAlias {
public :
	struct sAlias {
		string str_var ;
		string str_alias ;
	};
	CXMLAlias(): m_num(0),m_pBuf(0) ,m_created(0){}
	virtual ~CXMLAlias() {
		if(m_pBuf) delete[] m_pBuf ;
	}

	sAlias &operator[] (int n){	return m_pBuf[n] ;	}
	int Create(ndxml *xmlAlias) ;
	char *GetAlia(char *valname) ;
	int m_num ;
	int m_created ;
	sAlias *m_pBuf ;

};

// CXmlCfg dialog

class CXmlCfg : public CDialog
{
	DECLARE_DYNAMIC(CXmlCfg)

	enum {E_COL_NUM=2};
public:
	CXmlCfg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CXmlCfg();

	void SetXML(ndxml_root *xml_root) ;
// Dialog Data
	enum { IDD = IDD_CFG_EDIT };

	ndxml_root *m_root ;
	CXMLAlias m_alias ;
	CTreeCtrl *m_pTree ;
	HTREEITEM m_hRootItem ;	//树根
	int m_stat ;			//0 xml is nothing, 1   2 changed

	CGradShow *m_grad ;
protected:
	bool CreateXmlTree(ndxml_root *xml_root) ;
	void InitTreeNode(ndxml *xml_node,HTREEITEM hParent); 
	int ShowXMLValue(ndxml *xml_node,CGradShow *show_ctrl, int expand =0) ;	//显示xml的内容
	int DisplaySelXml() ;
	ndxml* GetSelXml(HTREEITEM hItem);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	void CfgChanged() {m_stat = 1;}
	void ShowRow(char *name, char *val, ndxml* param) ;

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnNMDblclkTree1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMReturnTree1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTvnSelchangedTree1(NMHDR *pNMHDR, LRESULT *pResult);    
	afx_msg void OnGridEndEdit(NMHDR *pNotifyStruct, LRESULT* pResult);
	afx_msg void OnGridClick(NMHDR *pNotifyStruct, LRESULT* pResult);
	afx_msg void OnGridEndSelChange(NMHDR *pNotifyStruct, LRESULT* pResult);
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedOk();
};
