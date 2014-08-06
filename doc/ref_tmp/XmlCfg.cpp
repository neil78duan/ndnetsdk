// XmlCfg.cpp : implementation file
//

#include "stdafx.h"
#include "excel_exp.h"
#include "XmlCfg.h"
#include "ND_file.h"

#include "ctrls/NewCellTypes/GridURLCell.h"
#include "ctrls/NewCellTypes/GridCellCombo.h"
#include "ctrls/NewCellTypes/GridCellCheck.h"
#include "ctrls/NewCellTypes/GridCellNumeric.h"
#include "ctrls/NewCellTypes/GridCellDateTime.h"

enum eDataType{			//数据类型
	EDT_NONE = 0 ,		//无类型
	EDT_ENUM,			//枚举类型
	EDT_BOOL ,			//bool类型
	EDT_NUMERAL,		//数字类型
	EDT_STRING	,		//字符类型
	EDT_PASSWORD,		//密码类型
	EDT_HIDE,			//隐藏节点
	EDT_IN_FILE,		//输入文件
	EDT_OUT_FILE,		//输出文件
	EDT_DIR				//目录
} ;
char *_szDataKindsDesc[] =		//数据类型对应的字符串
{
	"none"  ,		//无类型
	"enum",			//枚举类型
	"bool" ,		//bool类型
	"numeral",		//数字类型
	"string" ,		//字符类型
	"password",		//显示密码
	"hide",			//隐藏类型(此node节点不可见
	"in_file",		//输入文件类型
	"out_file",		//输出文件类型
	"dir"			//目录
};
enum eReservedType {
	ERT_KIND =0 ,
	ERT_VALUE, 
	ERT_NAME ,
	ERT_CREATETIME,
	ERT_DESC,
	ERT_EXPAND,
	ERT_RW ,
	ERT_PARAM
} ;
char *_szReserved[] = {		//保留字
	"kinds",				//制定XML node 的类型 eDataType
	"enum_value",			//枚举值,指定枚举值的内容
	"name",
	"createtime" ,
	"desc" ,
	"expand",				//张开所有子项 
	"rw_stat",				//读写状态(只读"read" 读写"write" default )
	"param"					//参数
} ;

//检查XML 的attribute 是否是保留字
int CheckReserved(char *param)
{
	int num = sizeof(_szReserved) /sizeof(char*) ;
	for(int i=0; i<num; i++) {
		if(0==stricmp(param,_szReserved[i]))
			return i ;
	}
	return -1 ;
}

//得到数据类型 eDataType
int CheckDataType(char *param)
{
	int num = sizeof(_szDataKindsDesc) /sizeof(char*) ;
	for(int i=0; i<num; i++) {
		if(0==stricmp(param,_szDataKindsDesc[i]))
			return i ;
	}
	return -1 ;
}

//得到xml值的类型
int GetXmlValType(ndxml *xml)
{
	char *kinds = ndxml_getattr_val(xml, _szReserved[ERT_KIND]) ;
	if(kinds) {
		return  CheckDataType(kinds) ;
	}
	return EDT_NONE  ;
}

char* GetXmlParam(ndxml *xml)
{
	return ndxml_getattr_val(xml, _szReserved[ERT_PARAM]) ;
}
char *_GetXmlName(ndxml *xml)
{
	char *name = ndxml_getattr_val(xml, _szReserved[ERT_NAME]) ;
	if(!name) {
		name = ndxml_getname(xml) ;
	}
	return name ;
}

char *_GetXmlDesc(ndxml *xml)
{
	char *name = ndxml_getattr_val(xml, _szReserved[ERT_DESC]) ;
	if(!name) {
		name = "NULL";
	}
	return name ;
}
//检测节点是否隐藏
BOOL CheckHide(ndxml *xml)
{
	char *kinds = ndxml_getattr_val(xml, _szReserved[ERT_KIND]) ;
	if(kinds) {
		if(EDT_HIDE==CheckDataType(kinds) )
			return TRUE ;
	}
	return FALSE ;
}

//检测是否是扩展显示所有子节点
BOOL CheckExpand(ndxml *xml)
{
	char *kinds = ndxml_getattr_val(xml, _szReserved[ERT_EXPAND]) ;
	if(kinds) {
		if(0==stricmp(kinds,"yes")) 
			return TRUE ;

	}
	return FALSE ;

}

//检测是否是扩展显示所有子节点
BOOL CheckReadOnly(ndxml *xml)
{

	char *kinds = ndxml_getattr_val(xml, _szReserved[ERT_RW]) ;
	if(kinds) {
		if(0==stricmp(kinds,"read")) 
			return TRUE ;	
	}
	//XML parent ;
	//if(0==xml.Parent(parent) )
	//	return CheckReadOnly(parent) ;
	return FALSE ;	
}
/////////////////////////////////////////////////////////////////////////////

int CXMLAlias::Create(ndxml *xmlAlias) 
{
	ndxml *node;
	if(m_created)
		return 0 ;
	
	int num = ndxml_getsub_num(xmlAlias) ;
	if(num <=0 )
		return -1 ;

	m_pBuf = new sAlias[num] ;

	for(int i=0; i<num; i++) {
		node = ndxml_refsubi(xmlAlias,i) ;
		if(node ) {
			m_pBuf[m_num].str_var = ndxml_getname(node);
			m_pBuf[m_num].str_alias = ndxml_getval(node) ;
			m_num++ ;
		}		
	}
	m_created = 1 ;
	return m_num ;
}

char* CXMLAlias::GetAlia(char *valname) 
{
	for (int i=0; i<m_num; i++)
	{
		if(0==stricmp(valname,m_pBuf[i].str_var.c_str()))
			return (char*)m_pBuf[i].str_alias.c_str() ;
	}
	return NULL ;
}
//////////////////////////////////////////////////////////////////////////
// CXmlCfg dialog

IMPLEMENT_DYNAMIC(CXmlCfg, CDialog)

CXmlCfg::CXmlCfg(CWnd* pParent /*=NULL*/)
	: CDialog(CXmlCfg::IDD, pParent)
{

	m_root = 0;
	m_pTree = 0;
	m_hRootItem = 0;	//树根
	m_stat =0;			//0 xml is empyt, 1 input xml,  2 changed

	m_grad = 0;
}

CXmlCfg::~CXmlCfg()
{

	if(m_grad) {
		delete m_grad ;
		m_grad = 0 ;
	}
}

void CXmlCfg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CXmlCfg, CDialog)
	ON_NOTIFY(NM_DBLCLK, IDC_TREE1, &CXmlCfg::OnNMDblclkTree1)
	ON_NOTIFY(NM_RETURN, IDC_TREE1, &CXmlCfg::OnNMReturnTree1)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE1, &CXmlCfg::OnTvnSelchangedTree1)

	ON_NOTIFY(GVN_ENDLABELEDIT, IDC_GRID_CTRL, OnGridEndEdit)
	ON_NOTIFY(NM_CLICK, IDC_GRID_CTRL, OnGridClick)
	ON_BN_CLICKED(IDCANCEL, &CXmlCfg::OnBnClickedCancel)
	ON_BN_CLICKED(IDOK, &CXmlCfg::OnBnClickedOk)
	ON_NOTIFY(GVN_SELCHANGED, IDC_GRID_CTRL, OnGridEndSelChange)
END_MESSAGE_MAP()


// CXmlCfg message handlers

#define SHOW_VAL(_name, _val, _op, _desc, _grid, _param)	\
	do {											\
		CString str_buf[E_COL_NUM] ;				\
		str_buf[0] = _name ;						\
		str_buf[1] = _val ;							\
		_grid->ShowText(str_buf,E_COL_NUM) ;		\
		int row =_grid->GetRowCount()-1 ;					\
		_grid->SetItemData(row, 1, (LPARAM)_param);	\
		if(_param) {									\
			_grid->SetItemState(row, 1, _grid->GetItemState(row, 1) & ~GVIS_READONLY);	\
		}												\
	} while (0)

BOOL CXmlCfg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_pTree = (CTreeCtrl *)GetDlgItem(IDC_TREE1) ;

	if(!m_pTree)
		return FALSE ;

	CRect rc;	
	GetDlgItem(IDC_GRID_FRAME)->GetWindowRect(rc);
	ScreenToClient(&rc);

	m_grad = new CGradShow ;

	m_grad->Create(rc, this, IDC_GRID_CTRL);
	m_grad->SetEditable(FALSE);


	m_hRootItem = m_pTree->InsertItem("XML配置信息") ;
	m_pTree->SetItemData(m_hRootItem, (DWORD)0) ;			//root = 0 ;
	CreateXmlTree(m_root) ;
	//InitTree(m_xmlroot,m_hRootItem) ;

	m_pTree->Expand(m_hRootItem, TVE_EXPAND);

	SHOW_VAL(_T("名字") ,_T("值") ,_T("操作") , ("备注"),m_grad, 0)	;
	/*
	CString str_buf[10] ;
	for(int i=0; i<10; i++) {
		str_buf[i].Format( "hello world %d", i) ;
	}

	for(int i=0; i<10; i++)
		m_grad->ShowText(str_buf,10) ;
*/
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

#define BACKUP_NAME "xmlcfgbackup.xml"
void CXmlCfg::SetXML(ndxml_root *xml_root) 
{
	m_root = xml_root ;
	ND_CFile::chdir((char*)ND_CFile::getworkingdir()) ;
	ndxml_save(xml_root, BACKUP_NAME) ;
	ndxml *alias = ndxml_getnode(xml_root, "alias") ;
	if(alias)
		m_alias.Create(alias) ;
}
bool CXmlCfg::CreateXmlTree(ndxml_root *xml_root) 
{

	HTREEITEM hNode ;
	int num =ndxml_num(xml_root) ;
	for(int i=0;i<num; i++) {
		ndxml *node = ndxml_getnodei(xml_root,i) ;
		if(!node)
			continue ;
		
		if(CheckHide(node))
			continue ;

		hNode = m_pTree->InsertItem(_GetXmlName(node),m_hRootItem,TVI_LAST) ;
		m_pTree->SetItemData(hNode,(DWORD)node) ;
		if(ndxml_getsub_num(node)>0 && !CheckExpand(node)){
			InitTreeNode(node,hNode) ;
		}
	}
	return true ;
}

void CXmlCfg::InitTreeNode(ndxml *xml_node,HTREEITEM hParent)
{

	HTREEITEM hNode ;
	int num =ndxml_getsub_num(xml_node);
	for(int i=0;i<num; i++) {
		ndxml *node = ndxml_refsubi(xml_node,i) ;
		if(!node)
			continue ;

		if(CheckHide(node))
			continue ;

		hNode = m_pTree->InsertItem(_GetXmlName(node),hParent,TVI_LAST) ;
		m_pTree->SetItemData(hNode,(DWORD)node) ;
		if(ndxml_getsub_num(node)&& !CheckExpand(node)){
			InitTreeNode(node,hNode) ;
		}
	}
}

//显示一行xml
void CXmlCfg::ShowRow(char *name, char *val, ndxml* param) 
{
	CString str_buf[E_COL_NUM] ;
	str_buf[0] = name ;
	str_buf[1] = val ;
	m_grad->ShowText(str_buf,E_COL_NUM) ;
	int row =m_grad->GetRowCount()-1 ;
	m_grad->SetItemData(row, 1, (LPARAM)param);

	if(param) {	
		m_grad->SetItemState(row, 1, m_grad->GetItemState(row, 1) & ~GVIS_READONLY);
		int type =  GetXmlValType(param) ;
		if(type==EDT_BOOL) {
			if(m_grad->SetCellType(row,1, RUNTIME_CLASS(CGridCellCheck))) {
				CGridCellCheck *pcell =(CGridCellCheck *) m_grad->GetCell(row,1) ;
				pcell->SetCheck('0'!=val[0]) ;
				m_grad->SetItemText(row,1, NULL);
				m_grad->RedrawCell(row,1);
			}
		}
		else if(type==EDT_ENUM) {
			if (!m_grad->SetCellType(row,1, RUNTIME_CLASS(CGridCellCombo)))
				return;
			char *pValue =ndxml_getattr_val(param, _szReserved[ERT_VALUE]) ;
			if(!pValue)
				return  ;
			char subtext[128] ;

			CStringArray options;
			int num = 0;
			do 	{
				pValue = ndstr_parse_word_n(pValue,subtext,sizeof(subtext)) ;				
				options.Add(subtext) ;
				if(pValue) {
					pValue = strchr(pValue, ',') ;
					if(pValue)
						++pValue;
				}
				num++ ;
			} while(pValue);

			int sel = atoi(val) ;
			if(sel >=0 || sel <num)
				m_grad->SetItemText(row,1, (LPCTSTR) options[sel]);


			CGridCellCombo *pCell = (CGridCellCombo*) m_grad->GetCell(row,1);
			pCell->SetOptions(options);
			pCell->SetStyle(CBS_DROPDOWN); 
		}
	}

}
//显示xml的内容
int CXmlCfg::ShowXMLValue(ndxml *xml_node,CGradShow *show_ctrl, int expand ) 
{
	int i;
	char *pval , *name;
	//size_t param ;
	pval = ndxml_getval(xml_node) ;
	if(pval) {
		char *desc =_GetXmlDesc(xml_node);
		name = _GetXmlName(xml_node) ;

		char *alias = m_alias.GetAlia(name) ;
		if(alias)
			name = alias ;

		ShowRow(name, pval, xml_node) ;
		//SHOW_VAL(name ,pval ,_T("NULL") , desc,show_ctrl, xml_node) ;

	}
	for(i=0; i<ndxml_getattr_num(xml_node); i++) {

		name = ndxml_getattr_name(xml_node,i) ;
		if(!name || CheckReserved(name)>=0)
			continue;
		pval = ndxml_getattr_vali(xml_node,i) ;
		if(pval) {
			char *alias = m_alias.GetAlia(name) ;
			if(alias)
				name = alias ;

			ShowRow(name, pval, xml_node) ;
			//SHOW_VAL(name ,pval ,_T("NULL") , _T("NULL"),show_ctrl,0) ;
		}
	}

	if(expand) {
		for(i=0; i<ndxml_getsub_num(xml_node); i++) {
			ndxml *sub_node = ndxml_refsubi(xml_node,i) ;
			if(sub_node && !CheckHide(sub_node) ){
				ShowXMLValue(sub_node,show_ctrl,expand ) ;
			}
		}
	}
	
	return 0;

}
void CXmlCfg::OnNMDblclkTree1(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here

	DisplaySelXml() ;
	*pResult = 0;
}

void CXmlCfg::OnNMReturnTree1(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here

	DisplaySelXml() ;
	*pResult = 0;
}

void CXmlCfg::OnTvnSelchangedTree1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

	DisplaySelXml() ;
	*pResult = 0;
}

int CXmlCfg::DisplaySelXml()
{
	HTREEITEM hItem = m_pTree->GetSelectedItem();
	if(!hItem) 
		return -1;
	ndxml* xml = GetSelXml(hItem) ;
	if(!xml)
		return -1;

	m_grad->Clear();

	ShowXMLValue(xml,m_grad, (int)CheckExpand(xml)) ; 

	ND_SetChildrenFont(m_grad->GetSafeHwnd(),false,12, _T("宋体"))  ;
	
	return 0;
}

//通过选中的TREE item来得到XML 节点
ndxml* CXmlCfg::GetSelXml(HTREEITEM hItem)
{
	if(hItem==m_hRootItem) {
		return NULL ;
	}

	return (ndxml*) (m_pTree->GetItemData(hItem) );
}

// GVN_ENDLABELEDIT
void CXmlCfg::OnGridEndEdit(NMHDR *pNotifyStruct, LRESULT* pResult)
{
	NM_GRIDVIEW* pItem = (NM_GRIDVIEW*) pNotifyStruct;
	//Trace(_T("End Edit on row %d, col %d\n"), pItem->iRow, pItem->iColumn);
	ndxml *xml =(ndxml *) m_grad->GetItemData(pItem->iRow, pItem->iColumn) ;
	int ret = -1 ;
	if(xml){
		CString str1 = m_grad->GetItemText(pItem->iRow, pItem->iColumn) ;
		if(GetXmlValType(xml)==EDT_ENUM) {
			int sel = -1 ;
			if(!str1.IsEmpty()) {
				char *pValue =ndxml_getattr_val(xml, _szReserved[ERT_VALUE]) ;
				if(!pValue)
					return  ;
				char subtext[128] ;
				int index = 0 ;
				do 	{
					pValue = ndstr_parse_word_n(pValue,subtext,sizeof(subtext)) ;
					if(0==ndstricmp(subtext,(char*)(LPCTSTR)str1)) {
						sel = index ;
						break ;
					}
					if(pValue) {
						pValue = strchr(pValue, ',') ;
						if(pValue)
							++pValue;
					}
					index++ ;
				} while(pValue);
			}
			if(-1 != sel) {
				str1.Format("%d", sel) ;
				ret = ndxml_setval(xml, (char*)(LPCTSTR)str1) ;
			}

		}
		else if(!str1.IsEmpty()) {
			ret = ndxml_setval(xml, (char*)(LPCTSTR)str1) ;
			m_grad->Invalidate();
			CfgChanged() ;
		}
	}
	*pResult = ret;
}

// GVN_SELCHANGED
void CXmlCfg::OnGridEndSelChange(NMHDR *pNotifyStruct, LRESULT* /*pResult*/)
{
	NM_GRIDVIEW* pItem = (NM_GRIDVIEW*) pNotifyStruct;
	//Trace(_T("End Selection Change on row %d, col %d (%d Selected)\n"), 
	//	pItem->iRow, pItem->iColumn, m_Grid.GetSelectedCount());
	ndxml *xml =(ndxml *) m_grad->GetItemData(pItem->iRow, pItem->iColumn) ;
	if(xml){
		if(GetXmlValType(xml)==EDT_BOOL) {
			CGridCellCheck *pcell =(CGridCellCheck *) m_grad->GetCell(pItem->iRow, pItem->iColumn) ;
			if(pcell) {
				char *val = pcell->GetCheck() ? "0":"1" ;
				ndxml_setval(xml, val) ;
				CfgChanged() ;
			}
		}
	}

}


// 得到文件名
bool _GetFileName(bool bOpen, CString & strFile, char *default_file)
{
	CString defext=".xls";
	CString filter;	
	if(default_file) {
		filter.Format("default Files (*.%s)| *.%s| All Files (*.*)| *.*||", default_file,default_file) ;
	}
	else 
		filter =" All Files (*.*)| *.*||";

	CFileDialog myfile(bOpen,_T("*.*"),NULL,NULL,(LPCTSTR)filter,NULL);
	if(myfile.DoModal()==IDOK)	{
		strFile = myfile.GetPathName() ;
		return true ;
	}
	return false;
}

bool _OpenFilter(CString &strPath, CString &tip, HWND hOwner)
{
	char   szDir[MAX_PATH];   
	BROWSEINFO   bi;   
	ITEMIDLIST   *pidl;   

	bi.hwndOwner   =   hOwner;   
	bi.pidlRoot   =   NULL;   
	bi.pszDisplayName   =   szDir;   
	bi.lpszTitle   =   (LPCTSTR)tip ; //"请选择目录";//strDlgTitle;   
	bi.ulFlags   =   BIF_RETURNONLYFSDIRS;   
	bi.lpfn   =   NULL;   
	bi.lParam   =   0;   
	bi.iImage   =   0;   

	pidl   =   SHBrowseForFolder(&bi);   
	if(pidl   ==   NULL)     
		return false;   
	if(!SHGetPathFromIDList(pidl,   szDir))     
		return false;
	strPath = szDir ;
	return TRUE;
}

void CXmlCfg::OnGridClick(NMHDR *pNotifyStruct, LRESULT* /*pResult*/)
{
	NM_GRIDVIEW* pItem = (NM_GRIDVIEW*) pNotifyStruct;
	//Trace(_T("Clicked on row %d, col %d\n"), pItem->iRow, pItem->iColumn);
	
	ndxml *xml =(ndxml *) m_grad->GetItemData(pItem->iRow, pItem->iColumn) ;
	if(xml){
		int type =  GetXmlValType(xml) ;
		if(type== EDT_IN_FILE || type==EDT_OUT_FILE) {
			CString  strFile ;
			if(_GetFileName(type==EDT_IN_FILE,strFile ,GetXmlParam(xml)) ) {
				if(0==ndxml_setval(xml, (char*)(LPCTSTR)strFile) ) {
					m_grad->SetItemText(pItem->iRow, pItem->iColumn, (LPCTSTR)strFile);
					m_grad->RedrawCell(pItem->iRow, pItem->iColumn);
					m_grad->Invalidate();
					CfgChanged();
				}
			}
		}
		else if(EDT_DIR==type) {
			CString path ;
			if(_OpenFilter(path, CString(_T("先择目录")),GetSafeHwnd()) ){
				if(path.Right(1)!=_T("\\")) {
					path += _T("\\") ;
				}
				if(0==ndxml_setval(xml, (char*)(LPCTSTR)path) ) {
					m_grad->SetItemText(pItem->iRow, pItem->iColumn, (LPCTSTR)path);
					m_grad->RedrawCell(pItem->iRow, pItem->iColumn);
					m_grad->Invalidate();
					CfgChanged();
				}
			}
		}
	}
}
void CXmlCfg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	if(m_stat) {
		ND_CFile::chdir((char*)ND_CFile::getworkingdir()) ;
		ndxml_destroy(m_root) ;
		ndxml_load(BACKUP_NAME,m_root) ;
	}
	ND_CFile::rmfile(BACKUP_NAME) ;
	OnCancel();
}

void CXmlCfg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	ND_CFile::rmfile(BACKUP_NAME) ;
	OnOK();
}
