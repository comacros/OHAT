#include <Origin.h>
#include <../OriginLab/DialogEx.h>
#include <../OriginLab/HTMLDlg.h>

#define APP_DIALOG_TITLE      "APP"
#define APP_INDEX_PAGE_NAME   "index.html"
#define APP_CLASS_NAME        AppDialog
#define APP_LAUNCH_FUNCTION   LaunchAppDialog
#define APP_DIALOG_MIN_WIDTH  600
#define APP_DIALOG_MIN_HEIGHT 400
#define APP_HAS_ORIGIN_GRAPH  false

#if APP_HAS_ORIGIN_GRAPH
#define APP_DIALOG_MODELESS
#endif // APP_HAS_ORIGIN_GRAPH

#define USER_LANGUAGE_TRUE  true

#define SHOW_APP_CALLSTACK_INFO false
#define LOGCS if(SHOW_APP_CALLSTACK_INFO){printf("%d: %d\n", GetTickCount(), __LINE__);}

#ifdef APP_DIALOG_MODELESS
class APP_CLASS_NAME;
static APP_CLASS_NAME *s_pDlg = NULL;
#endif // APP_DIALOG_MODELESS

#define APP_WITH_ORIGIN_GRAPH_CONTROL

#define ID_GRAPH_CONTROL 1

class APP_CLASS_NAME : public HTMLDlg
{
public:
	
	int DoModalEx(HWND hParent, DWORD dwDlgBits=0)
	{
		return HTMLDlg::DoModalEx(hParent, dwDlgBits);
	}
	
#ifdef APP_DIALOG_MODELESS
	int Create(HWND hParent = NULL, DWORD dwOptions = 0)
	{
		InitMsgMap();
		return HTMLDlg::Create(hParent, dwOptions);
	}
#endif // APP_DIALOG_MODELESS

protected:
	
	string GetDialogTitle() {
		return APP_DIALOG_TITLE;
	}
	void   SetDialogTitle() {
		Text = APP_DIALOG_TITLE;
	}
	
	string GetInitURL() {
		string strPath = __FILE__ "\\..\\" APP_INDEX_PAGE_NAME;
		return strPath;
	}
	string GetCurrentLanguage() {
		char lang = okutil_get_current_lang(USER_LANGUAGE_TRUE);
		if(lang == 'C')
			return "zh";
		else if(lang == 'E')
			return "en";
		else if(lang == 'G')
			return "de";
		else if(lang == 'J')
			return "ja";
		else
			return "en";
	}
	
	void UpdateDialogFromOC() {
		LOGCS
		
		if( !m_dhtml )
			return;

		if(!m_js)
			m_js = m_dhtml.GetScript();

		if( !m_js )
			return;

		try
		{
			m_js.updateColumns();
		}
		catch(int err)
		{
			Quit();
		}
	}
	void ConsoleLog(string log) {
		LOGCS
		
		printf("%s\n", log);
	}
	void Quit() {
		HTMLDlg::Close(IDCANCEL);
	}
public:
	
	EVENTS_BEGIN_DERIV(HTMLDlg)
		ON_INIT(OnInitDialog)
		ON_DESTROY(OnDestroy)
		ON_GETMINMAXINFO(OnMinMaxInfo)
		
#ifdef APP_DIALOG_MODELESS
		ON_SIZE(OnDlgResize)
#endif // APP_DIALOG_MODELESS

	EVENTS_END_DERIV	

	DECLARE_DISPATCH_MAP
	
protected:
	
	bool OnDestroy() {
		LOGCS
		
#ifdef APP_DIALOG_MODELESS
		HTMLDlg::OnDestroy();
		s_pDlg = NULL;
		delete this;
#endif // APP_DIALOG_MODELESS
		return true;
	}
	
	BOOL OnInitDialog()
	{
		LOGCS
		
		if( !HTMLDlg::OnInitDialog() )
			return FALSE;

		// Modeless dialog should be allowed to be minimized.
		ModifyStyle(0, WS_MAXIMIZEBOX|WS_MINIMIZEBOX);
		ModifyStyleEx(0, WS_EX_DLGMODALFRAME);
		
#ifdef APP_WITH_ORIGIN_GRAPH_CONTROL
		RECT rect;
		m_gcCntrl.CreateControl(GetSafeHwnd(), &rect, ID_GRAPH_CONTROL, WS_CHILD|WS_VISIBLE|WS_BORDER);
		m_graph.Create("Origin", CREATE_HIDDEN);
		// m_graph.LT_execute("page -vnw -w 500");
		// m_graph.LT_execute("page.aa = 1");
		DWORD dwAttachOptions = 0xFF;
		m_gcCntrl.AttachPage(m_graph, dwAttachOptions);
#endif // APP_WITH_ORIGIN_GRAPH_CONTROL

		SetDialogTitle();
		
		return TRUE
	}
	
#ifdef APP_WITH_ORIGIN_GRAPH_CONTROL
	BOOL OnDlgResize(int nType, int cx, int cy) // when you resize the dialog, need to reinit the size and position of each control in dialog
	{
		LOGCS
		
		if( !IsInitReady() )
			return false;

		// MoveControlsHelper _temp(this); // you can uncomment this line, if the dialog flickers when you resize it
		HTMLDlg::OnDlgResize(nType, cx, cy); //place html control in dialog

		if( !IsHTMLDocumentCompleted() ) //check the state of HTML control
			return FALSE;

		RECT rectGraph;
		if( !GetGraphRECT(rectGraph) )
			return FALSE;
		//overlap the GraphControl on the top of HTML control
		m_gcCntrl.SetWindowPos(HWND_TOP, rectGraph.left, rectGraph.top, RECT_WIDTH(rectGraph), RECT_HEIGHT(rectGraph), 0);

		return TRUE;
	}
	
	BOOL GetGraphRECT(RECT &rect) //this is the function to call JavaScript and get the position of GraphControl
	{
		LOGCS
		
		if( !m_dhtml )
			return FALSE;

		if(!m_js)
			m_js = m_dhtml.GetScript();

		if( !m_js ) //check the validity of returned COM object is always recommended
			return FALSE;
		//return true;
		string str;
		try
		{
			str = m_js.getGraphRect();
		}
		catch(int err)
		{
			return false;
		}
		JSON.FromString(rect, str); //convert a string to a structure
	#if _OC_VER >= 0x0950
		check_convert_rect_with_DPI(GetWindow(), rect, true);
	#endif
	
		LOGCS
	
		return TRUE;
	}
	
	void DOMResize(string strRect) {
		LOGCS
		
		RECT rectGraph;
		JSON.FromString(rectGraph, strRect);
		
#if _OC_VER >= 0x0950
		check_convert_rect_with_DPI(GetWindow(), rectGraph, true);
#endif

		m_gcCntrl.SetWindowPos(HWND_TOP, rectGraph.left, rectGraph.top, RECT_WIDTH(rectGraph), RECT_HEIGHT(rectGraph), 0);
	}
#endif // APP_WITH_ORIGIN_GRAPH_CONTROL
	
	void OnMinMaxInfo(MINMAXINFO* lpMMI) {
		lpMMI->ptMinTrackSize.y = CheckConvertDlgSizeWithDPI(APP_DIALOG_MIN_HEIGHT, false);
		lpMMI->ptMinTrackSize.x = CheckConvertDlgSizeWithDPI(APP_DIALOG_MIN_WIDTH, true);
	}
	

private:
#ifdef APP_WITH_ORIGIN_GRAPH_CONTROL
	GraphControl m_gcCntrl;
	GraphPage	 m_graph;
#endif
	Object		 m_js;
};

BEGIN_DISPATCH_MAP(APP_CLASS_NAME, HTMLDlg)
	DISP_FUNCTION(APP_CLASS_NAME, Quit, VTS_VOID, VTS_I4)
	DISP_FUNCTION(APP_CLASS_NAME, GetCurrentLanguage, VTS_STR, VTS_VOID)
	DISP_FUNCTION(APP_CLASS_NAME, ConsoleLog, VTS_VOID, VTS_BSTRA)
	
#ifdef APP_WITH_ORIGIN_GRAPH_CONTROL
	DISP_FUNCTION(APP_CLASS_NAME, DOMResize, VTS_VOID, VTS_BSTRA)
#endif

END_DISPATCH_MAP

void APP_LAUNCH_FUNCTION()
{
	DWORD dwOptions = 0; // DLG_NO_LOAD_TOP_LEFT | DLG_NO_LOAD_WIDTH_HEIGHT;
#ifdef APP_DIALOG_MODELESS
	if(s_pDlg == NULL)
	{
		s_pDlg = new APP_CLASS_NAME;
		if(s_pDlg)
		{
			if(!s_pDlg->Create(GetWindow(), dwOptions))
			{
				delete s_pDlg;
				s_pDlg = NULL;
			}
		}
	}
#else // APP_DIALOG_MODELESS
	APP_CLASS_NAME dlg;
	dlg.DoModalEx(GetWindow(), dwOptions);
#endif // APP_DIALOG_MODELESS
}

