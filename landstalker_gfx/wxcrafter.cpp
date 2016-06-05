//////////////////////////////////////////////////////////////////////
// This file was auto-generated by codelite's wxCrafter Plugin
// wxCrafter project file: wxcrafter.wxcp
// Do not modify this file by hand!
//////////////////////////////////////////////////////////////////////

#include "wxcrafter.h"


// Declare the bitmap loading function
extern void wxC9ED9InitBitmapResources();

static bool bBitmapLoaded = false;


MainFrameBaseClass::MainFrameBaseClass(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style)
    : wxFrame(parent, id, title, pos, size, style)
{
    if ( !bBitmapLoaded ) {
        // We need to initialise the default bitmap handler
        wxXmlResource::Get()->AddHandler(new wxBitmapXmlHandler);
        wxC9ED9InitBitmapResources();
        bBitmapLoaded = true;
    }
    
    m_menuBar = new wxMenuBar(0);
    this->SetMenuBar(m_menuBar);
    
    m_name6 = new wxMenu();
    m_menuBar->Append(m_name6, _("File"));
    
    m_menuItem109 = new wxMenuItem(m_name6, wxID_ANY, _("Open\tCtrl-O"), _("Open"), wxITEM_NORMAL);
    m_name6->Append(m_menuItem109);
    
    m_name6->AppendSeparator();
    
    m_menuItem7 = new wxMenuItem(m_name6, wxID_EXIT, _("Exit\tAlt-X"), _("Quit"), wxITEM_NORMAL);
    m_name6->Append(m_menuItem7);
    
    m_name8 = new wxMenu();
    m_menuBar->Append(m_name8, _("Help"));
    
    m_menuItem9 = new wxMenuItem(m_name8, wxID_ABOUT, _("About..."), wxT(""), wxITEM_NORMAL);
    m_name8->Append(m_menuItem9);
    
    m_statusBar29 = new wxStatusBar(this, wxID_ANY, wxSTB_DEFAULT_STYLE);
    m_statusBar29->SetFieldsCount(1);
    this->SetStatusBar(m_statusBar29);
    
    m_auimgr127 = new wxAuiManager;
    m_auimgr127->SetManagedWindow( this );
    m_auimgr127->SetFlags( wxAUI_MGR_LIVE_RESIZE|wxAUI_MGR_TRANSPARENT_HINT|wxAUI_MGR_TRANSPARENT_DRAG|wxAUI_MGR_ALLOW_ACTIVE_PANE|wxAUI_MGR_ALLOW_FLOATING);
    m_auimgr127->GetArtProvider()->SetMetric(wxAUI_DOCKART_GRADIENT_TYPE, wxAUI_GRADIENT_NONE);
    
    m_panel134 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(-1,-1), wxTAB_TRAVERSAL);
    
    m_auimgr127->AddPane(m_panel134, wxAuiPaneInfo().Caption(_("Browser")).Direction(wxAUI_DOCK_LEFT).Layer(0).Row(0).Position(0).BestSize(200,100).MinSize(100,100).MaxSize(100,100).CaptionVisible(true).MaximizeButton(false).CloseButton(true).MinimizeButton(false).PinButton(true));
    
    wxBoxSizer* boxSizer138 = new wxBoxSizer(wxVERTICAL);
    m_panel134->SetSizer(boxSizer138);
    
    m_treeCtrl101 = new wxTreeCtrl(m_panel134, wxID_ANY, wxDefaultPosition, wxSize(300,50), wxTR_DEFAULT_STYLE|wxTR_HIDE_ROOT|wxFULL_REPAINT_ON_RESIZE);
    
    boxSizer138->Add(m_treeCtrl101, 1, wxALL|wxEXPAND, 5);
    m_treeCtrl101->SetMinSize(wxSize(300,50));
    
    m_panel136 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(-1,-1), wxTAB_TRAVERSAL);
    
    m_auimgr127->AddPane(m_panel136, wxAuiPaneInfo().Caption(_("Properties")).Direction(wxAUI_DOCK_LEFT).Layer(0).Row(0).Position(0).BestSize(200,100).MinSize(100,100).MaxSize(100,100).CaptionVisible(true).MaximizeButton(false).CloseButton(true).MinimizeButton(false).PinButton(true));
    
    wxBoxSizer* boxSizer144 = new wxBoxSizer(wxVERTICAL);
    m_panel136->SetSizer(boxSizer144);
    
    wxArrayString m_pgMgr146Arr;
    wxUnusedVar(m_pgMgr146Arr);
    wxArrayInt m_pgMgr146IntArr;
    wxUnusedVar(m_pgMgr146IntArr);
    m_pgMgr146 = new wxPropertyGridManager(m_panel136, wxID_ANY, wxDefaultPosition, wxSize(300,50), wxPG_DESCRIPTION|wxPG_LIMITED_EDITING|wxPG_BOLD_MODIFIED|wxPG_AUTO_SORT);
    
    boxSizer144->Add(m_pgMgr146, 1, wxALL|wxEXPAND, 5);
    m_pgMgr146->SetMinSize(wxSize(300,50));
    
    m_scrollWin27 = new wxScrolledWindow(this, wxID_ANY, wxDefaultPosition, wxSize(-1,-1), wxFULL_REPAINT_ON_RESIZE|wxHSCROLL|wxVSCROLL);
    m_scrollWin27->SetBackgroundColour(wxColour(wxT("rgb(0,0,0)")));
    m_scrollWin27->SetScrollRate(5, 5);
    
    m_auimgr127->AddPane(m_scrollWin27, wxAuiPaneInfo().Direction(wxAUI_DOCK_CENTER).Layer(0).Row(0).Position(0).BestSize(100,100).MinSize(100,100).MaxSize(100,100).CaptionVisible(false).MaximizeButton(false).CloseButton(false).MinimizeButton(false).PinButton(false));
    m_auimgr127->Update();
    
    SetName(wxT("MainFrameBaseClass"));
    SetSize(800,600);
    if (GetSizer()) {
         GetSizer()->Fit(this);
    }
    if(GetParent()) {
        CentreOnParent(wxBOTH);
    } else {
        CentreOnScreen(wxBOTH);
    }
#if wxVERSION_NUMBER >= 2900
    if(!wxPersistenceManager::Get().Find(this)) {
        wxPersistenceManager::Get().RegisterAndRestore(this);
    } else {
        wxPersistenceManager::Get().Restore(this);
    }
#endif
    // Connect events
    this->Connect(m_menuItem109->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainFrameBaseClass::OnMenuitem109MenuSelected), NULL, this);
    this->Connect(m_menuItem7->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainFrameBaseClass::OnExit), NULL, this);
    this->Connect(m_menuItem9->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainFrameBaseClass::OnAbout), NULL, this);
    m_treeCtrl101->Connect(wxEVT_COMMAND_TREE_ITEM_ACTIVATED, wxTreeEventHandler(MainFrameBaseClass::OnTreectrl101TreeItemActivated), NULL, this);
    m_scrollWin27->Connect(wxEVT_PAINT, wxPaintEventHandler(MainFrameBaseClass::OnScrollwin27Paint), NULL, this);
    
}

MainFrameBaseClass::~MainFrameBaseClass()
{
    this->Disconnect(m_menuItem109->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainFrameBaseClass::OnMenuitem109MenuSelected), NULL, this);
    this->Disconnect(m_menuItem7->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainFrameBaseClass::OnExit), NULL, this);
    this->Disconnect(m_menuItem9->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainFrameBaseClass::OnAbout), NULL, this);
    m_treeCtrl101->Disconnect(wxEVT_COMMAND_TREE_ITEM_ACTIVATED, wxTreeEventHandler(MainFrameBaseClass::OnTreectrl101TreeItemActivated), NULL, this);
    m_scrollWin27->Disconnect(wxEVT_PAINT, wxPaintEventHandler(MainFrameBaseClass::OnScrollwin27Paint), NULL, this);
    
    m_auimgr127->UnInit();
    delete m_auimgr127;

}

ImgLst::ImgLst()
    : wxImageList(16, 16, true)
{
    if ( !bBitmapLoaded ) {
        // We need to initialise the default bitmap handler
        wxXmlResource::Get()->AddHandler(new wxBitmapXmlHandler);
        wxC9ED9InitBitmapResources();
        bBitmapLoaded = true;
    }
    
    {
        wxBitmap bmp;
        wxIcon icn;
        bmp = wxXmlResource::Get()->LoadBitmap(wxT("m_bmp57"));
        icn.CopyFromBitmap( bmp );
        this->Add( icn );
        m_bitmaps.insert( std::make_pair(wxT("m_bmp57"), bmp ) );
    }
    
    {
        wxBitmap bmp;
        wxIcon icn;
        bmp = wxXmlResource::Get()->LoadBitmap(wxT("m_bmp59"));
        icn.CopyFromBitmap( bmp );
        this->Add( icn );
        m_bitmaps.insert( std::make_pair(wxT("m_bmp59"), bmp ) );
    }
    
    {
        wxBitmap bmp;
        wxIcon icn;
        bmp = wxXmlResource::Get()->LoadBitmap(wxT("m_bmp61"));
        icn.CopyFromBitmap( bmp );
        this->Add( icn );
        m_bitmaps.insert( std::make_pair(wxT("m_bmp61"), bmp ) );
    }
    
}

ImgLst::~ImgLst()
{
}
