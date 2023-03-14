#ifndef MAINFRAME_H
#define MAINFRAME_H
#include "wxcrafter.h"
#include <cstdint>
#include <vector>
#include <memory>
#include <wx/dcmemory.h>
#include <wx/dataview.h>
#include "Block.h"
#include "Tileset.h"
#include "PaletteO.h"
#include "LSTilemapCmp.h"
#include "Rom.h"
#include "SpriteGraphic.h"
#include "SpriteFrame.h"
#include "Sprite.h"
#include "ImageBuffer.h"
#include "LSString.h"
#include "Images.h"
#include "ImageList.h"
#include "TilesetEditorFrame.h"
#include "TilesetManager.h"
#include "GameData.h"

#ifdef _WIN32
#include <winsock.h>
#else
#include <arpa/inet.h>
#endif

class wxImage;

class MainFrame : public MainFrameBaseClass
{
public:
    MainFrame(wxWindow* parent, const std::string& filename);
    virtual ~MainFrame();

protected:
    enum class ReturnCode
    {
        OK,
        ERR,
        CANCELLED
    };
    virtual void OnClose(wxCloseEvent& event);
    virtual void OnLayerOpacityChange(wxScrollEvent& event);
    virtual void OnLayerSelect(wxCommandEvent& event);
    virtual void OnLayerVisibilityChange(wxCommandEvent& event);
    virtual void OnPaint(wxPaintEvent& event);
    virtual void OnMousewheel(wxMouseEvent& event);
    virtual void OnKeyDown(wxKeyEvent& event);
    virtual void OnKeyUp(wxKeyEvent& event);
    virtual void OnOpen(wxCommandEvent& event);
    virtual void OnSaveAsAsm(wxCommandEvent& event);
    virtual void OnSaveToRom(wxCommandEvent& event);
    virtual void OnExport(wxCommandEvent& event);
    virtual void OnExit(wxCommandEvent& event);
    virtual void OnAbout(wxCommandEvent& event);
    virtual void OnBrowserSelect(wxTreeEvent& event);
    virtual void OnScrollWindowPaint(wxPaintEvent& event);
    virtual void OnScrollWindowMousewheel(wxMouseEvent& event);
    virtual void OnScrollWindowMouseMove(wxMouseEvent& event);
    virtual void OnScrollWindowLeftDown(wxMouseEvent& event);
    virtual void OnScrollWindowLeftUp(wxMouseEvent& event);
    virtual void OnScrollWindowRightDown(wxMouseEvent& event);
    virtual void OnScrollWindowRightUp(wxMouseEvent& event);
    virtual void OnScrollWindowKeyDown(wxKeyEvent& event);
    virtual void OnScrollWindowKeyUp(wxKeyEvent& event);
    virtual void OnScrollWindowResize(wxSizeEvent& event);
private:
    class TreeNodeData : public wxTreeItemData
    {
    public:
        enum NodeType {
            NODE_BASE,
            NODE_STRING,
            NODE_IMAGE,
            NODE_TILESET,
            NODE_ANIM_TILESET,
            NODE_BLOCKSET,
            NODE_ROOM_PAL,
            NODE_ROOM,
            NODE_ROOM_HEIGHTMAP,
            NODE_ROOM_WARPS,
            NODE_SPRITE,
            NODE_SPRITE_FRAME
        };
        TreeNodeData(NodeType nodeType = NODE_BASE, std::size_t value = 0) : m_nodeType(nodeType), m_value(value) {}
        std::size_t GetValue() const { return m_value; }
        NodeType GetNodeType() const { return m_nodeType; }
    private:
        const NodeType m_nodeType;
        const std::size_t m_value;
    };
    enum Mode
    {
        MODE_NONE,
        MODE_STRING,
        MODE_IMAGE,
        MODE_TILESET,
        MODE_BLOCKSET,
        MODE_PALETTE,
        MODE_ROOMMAP,
        MODE_SPRITE
    };
	void OnStatusBarInit(wxCommandEvent& event);
	void OnStatusBarUpdate(wxCommandEvent& event);
	void OnStatusBarClear(wxCommandEvent& event);
	void OnPropertiesInit(wxCommandEvent& event);
	void OnPropertiesUpdate(wxCommandEvent& event);
	void OnPropertiesClear(wxCommandEvent& event);
	void OnPropertyChange(wxPropertyGridEvent& event);
	void OnMenuInit(wxCommandEvent& event);
	void OnMenuClear(wxCommandEvent& event);
	void OnMenuClick(wxMenuEvent& event);
	void OnPaneClose(wxAuiManagerEvent& event);
    void DrawBlocks(const std::string& name, std::size_t row_width = -1, std::size_t scale = 1, uint8_t pal = 0);
    void DrawTilemap(uint16_t room, std::size_t scale=1);
    void DrawHeightmap(uint16_t room, std::size_t scale=1);
    void DrawWarps(uint16_t room, std::size_t scale=1);
    void DrawSprite(const Sprite& sprite, std::size_t animation, std::size_t frame, std::size_t scale = 4);
    void DrawImage(const std::string& image, std::size_t scale);
    void DrawWarp(wxGraphicsContext& gc, const WarpList::Warp& warp, std::shared_ptr<Tilemap3D> tilemap, int tile_width, int tile_height);
    void AddRoomLink(wxGraphicsContext* gc, const std::string& label, uint16_t room, int x, int y);
    void PopulatePalettes();
    void ShowStrings();
    void ShowTileset();
    void ShowBitmap();
    void ForceRepaint();
    void ClearScreen();
    void GoToRoom(uint16_t room);
    void PaintNow(wxDC& dc, std::size_t scale = 1);
    void InitPals(const wxTreeItemId& node);
    ReturnCode CloseFiles(bool force = false);
    bool CheckForFileChanges();
    void OpenFile(const wxString& path);
    void OpenRomFile(const wxString& path);
    void OpenAsmFile(const wxString& path);
    ReturnCode Save();
    ReturnCode SaveAsAsm(std::string path = std::string());
    ReturnCode SaveToRom(std::string path = std::string());
    void InitRoom(uint16_t room);
    void PopulateRoomProperties(uint16_t room);
    void EnableLayerControls(bool state);
    void SetMode(const Mode& mode);
    void Refresh();
    bool ExportPng(const std::string& filename);
    bool ExportTxt(const std::string& filename);
	ImageList& GetImageList();
    void ProcessSelectedBrowserItem(const wxTreeItemId& item);
    
    Rom m_rom;
    std::vector<uint8_t> m_gfxBuffer;
    std::size_t m_gfxSize;
    wxMemoryDC memDc;
    std::shared_ptr<wxBitmap> bmp;
    ImageBuffer m_imgbuf;
    wxImage m_img;
    std::size_t m_scale;
    uint16_t m_sprite_idx;
    uint16_t m_sprite_anim;
    uint16_t m_sprite_frame;
    uint16_t m_roomnum;
    int m_strtab;
    Mode m_mode;
    bool m_layer_controls_enabled;
    std::vector<std::vector<std::shared_ptr<LSString>>> m_strings;
    std::vector<SpriteFrame> m_spriteFrames;
    std::vector<SpriteGraphic> m_spriteGraphics;
    std::map<uint8_t, Sprite> m_sprites;
    std::string m_selImage;
    std::map<std::string, Images::Image> m_images;
    std::shared_ptr<std::map<std::string, PaletteO>> m_palettes;
    std::string m_selected_palette;
	ImageList* m_imgs;
    wxDataViewListCtrl* m_stringView;
    TilesetEditorFrame* m_tilesetEditor;
	EditorFrame* m_activeEditor;
    wxScrolledCanvas* m_canvas;
    std::vector<PaletteO> m_palette;
    std::list<std::pair<WarpList::Warp, std::vector<wxPoint2DDouble>>> m_warp_poly;
    std::list<std::pair<uint16_t, std::vector<wxPoint2DDouble>>> m_link_poly;

    bool m_asmfile;
    std::shared_ptr<TilesetManager> m_tsmgr;
    std::shared_ptr<GameData> m_g;

    std::string m_selname;
};
#endif // MAINFRAME_H
