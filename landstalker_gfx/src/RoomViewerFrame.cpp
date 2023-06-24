#include "RoomViewerFrame.h"

#include <fstream>
#include <sstream>

enum MENU_IDS
{
	ID_FILE_EXPORT_BIN = 20000,
	ID_FILE_EXPORT_CSV,
	ID_FILE_EXPORT_PNG,
	ID_FILE_IMPORT_BIN,
	ID_FILE_IMPORT_CSV,
	ID_EDIT,
	ID_EDIT_ENTITY_PROPERTIES,
	ID_EDIT_FLAGS,
	ID_EDIT_CHESTS,
	ID_TOOLS,
	ID_TOOLS_LAYERS,
	ID_TOOLS_ENTITIES,
	ID_TOOLS_WARPS,
	ID_VIEW,
	ID_VIEW_NORMAL,
	ID_VIEW_HEIGHTMAP,
	ID_VIEW_MAP,
	ID_VIEW_ENTITIES,
	ID_VIEW_ENTITY_HITBOX,
	ID_VIEW_WARPS,
	ID_VIEW_ERRORS
};

enum TOOL_IDS
{
	TOOL_TOGGLE_ENTITIES = 30000,
	TOOL_TOGGLE_ENTITY_HITBOX,
	TOOL_TOGGLE_WARPS,
	TOOL_NORMAL_MODE,
	TOOL_HEIGHTMAP_MODE,
	TOOL_MAP_MODE,
	TOOL_SHOW_LAYERS_PANE,
	TOOL_SHOW_ENTITIES_PANE,
	TOOL_SHOW_WARPS_PANE,
	TOOL_SHOW_FLAGS,
	TOOL_SHOW_CHESTS,
	TOOL_SHOW_SELECTION_PROPERTIES,
	TOOL_SHOW_ERRORS
};

wxBEGIN_EVENT_TABLE(RoomViewerFrame, wxWindow)
EVT_KEY_DOWN(RoomViewerFrame::OnKeyDown)
EVT_COMMAND(wxID_ANY, EVT_ZOOM_CHANGE, RoomViewerFrame::OnZoomChange)
EVT_COMMAND(wxID_ANY, EVT_OPACITY_CHANGE, RoomViewerFrame::OnOpacityChange)
EVT_COMMAND(wxID_ANY, EVT_ENTITY_UPDATE, RoomViewerFrame::OnEntityUpdate)
EVT_COMMAND(wxID_ANY, EVT_ENTITY_SELECT, RoomViewerFrame::OnEntitySelect)
EVT_COMMAND(wxID_ANY, EVT_ENTITY_OPEN_PROPERTIES, RoomViewerFrame::OnEntityOpenProperties)
EVT_COMMAND(wxID_ANY, EVT_ENTITY_ADD, RoomViewerFrame::OnEntityAdd)
EVT_COMMAND(wxID_ANY, EVT_ENTITY_DELETE, RoomViewerFrame::OnEntityDelete)
EVT_COMMAND(wxID_ANY, EVT_ENTITY_MOVE_UP, RoomViewerFrame::OnEntityMoveUp)
EVT_COMMAND(wxID_ANY, EVT_ENTITY_MOVE_DOWN, RoomViewerFrame::OnEntityMoveDown)
EVT_COMMAND(wxID_ANY, EVT_WARP_UPDATE, RoomViewerFrame::OnWarpUpdate)
EVT_COMMAND(wxID_ANY, EVT_WARP_SELECT, RoomViewerFrame::OnWarpSelect)
EVT_COMMAND(wxID_ANY, EVT_WARP_OPEN_PROPERTIES, RoomViewerFrame::OnWarpOpenProperties)
EVT_COMMAND(wxID_ANY, EVT_WARP_ADD, RoomViewerFrame::OnWarpAdd)
EVT_COMMAND(wxID_ANY, EVT_WARP_DELETE, RoomViewerFrame::OnWarpDelete)
wxEND_EVENT_TABLE()

RoomViewerFrame::RoomViewerFrame(wxWindow* parent, ImageList* imglst)
	: EditorFrame(parent, wxID_ANY, imglst),
	  m_title(""),
	  m_mode(RoomViewerCtrl::Mode::NORMAL),
	  m_reset_props(false),
	  m_layerctrl_visible(true),
	  m_entityctrl_visible(true),
	  m_warpctrl_visible(true)
{
	m_mgr.SetManagedWindow(this);

	m_roomview = new RoomViewerCtrl(this);
	m_layerctrl = new LayerControlFrame(this);
	m_entityctrl = new EntityControlFrame(this, GetImageList());
	m_warpctrl = new WarpControlFrame(this, GetImageList());
	m_mgr.AddPane(m_layerctrl, wxAuiPaneInfo().Right().Layer(1).Resizable(false).MinSize(220, 200)
		.BestSize(220, 200).FloatingSize(220, 200).Caption("Layers"));
	m_mgr.AddPane(m_entityctrl, wxAuiPaneInfo().Right().Layer(1).Resizable(false).MinSize(220, 200)
		.BestSize(220, 200).FloatingSize(220, 200).Caption("Entities"));
	m_mgr.AddPane(m_warpctrl, wxAuiPaneInfo().Right().Layer(1).Resizable(false).MinSize(220, 200)
		.BestSize(220, 200).FloatingSize(220, 200).Caption("Warps"));
	m_mgr.AddPane(m_roomview, wxAuiPaneInfo().CenterPane());

	// tell the manager to "commit" all the changes just made
	m_mgr.Update();
	UpdateUI();

	m_roomview->SetZoom(m_layerctrl->GetZoom());
	m_roomview->SetLayerOpacity(RoomViewerCtrl::Layer::BACKGROUND1, m_layerctrl->GetLayerOpacity(LayerControlFrame::Layer::BG1));
	m_roomview->SetLayerOpacity(RoomViewerCtrl::Layer::BACKGROUND2, m_layerctrl->GetLayerOpacity(LayerControlFrame::Layer::BG2));
	m_roomview->SetLayerOpacity(RoomViewerCtrl::Layer::BG_SPRITES, m_layerctrl->GetLayerOpacity(LayerControlFrame::Layer::SPRITES));
	m_roomview->SetLayerOpacity(RoomViewerCtrl::Layer::FOREGROUND, m_layerctrl->GetLayerOpacity(LayerControlFrame::Layer::FG));
	m_roomview->SetLayerOpacity(RoomViewerCtrl::Layer::FG_SPRITES, m_layerctrl->GetLayerOpacity(LayerControlFrame::Layer::SPRITES));
	m_roomview->SetLayerOpacity(RoomViewerCtrl::Layer::HEIGHTMAP, m_layerctrl->GetLayerOpacity(LayerControlFrame::Layer::HM));
	m_roomview->Connect(wxEVT_CHAR, wxKeyEventHandler(RoomViewerFrame::OnKeyDown), nullptr, this);
	m_entityctrl->Connect(wxEVT_CHAR, wxKeyEventHandler(RoomViewerFrame::OnKeyDown), nullptr, this);
	m_warpctrl->Connect(wxEVT_CHAR, wxKeyEventHandler(RoomViewerFrame::OnKeyDown), nullptr, this);
}

RoomViewerFrame::~RoomViewerFrame()
{
	m_roomview->Disconnect(wxEVT_CHAR, wxKeyEventHandler(RoomViewerFrame::OnKeyDown), nullptr, this);
	m_entityctrl->Disconnect(wxEVT_CHAR, wxKeyEventHandler(RoomViewerFrame::OnKeyDown), nullptr, this);
	m_warpctrl->Disconnect(wxEVT_CHAR, wxKeyEventHandler(RoomViewerFrame::OnKeyDown), nullptr, this);
}

void RoomViewerFrame::SetMode(RoomViewerCtrl::Mode mode)
{
	m_mode = mode;
	if (mode == RoomViewerCtrl::Mode::NORMAL)
	{
		SetPaneVisibility(m_layerctrl, m_layerctrl_visible);
		SetPaneVisibility(m_entityctrl, m_entityctrl_visible);
		SetPaneVisibility(m_entityctrl, m_warpctrl_visible);
	}
	else
	{
		m_layerctrl_visible = IsPaneVisible(m_layerctrl);
		m_entityctrl_visible = IsPaneVisible(m_entityctrl);
		m_warpctrl_visible = IsPaneVisible(m_warpctrl);
		SetPaneVisibility(m_layerctrl, false);
		SetPaneVisibility(m_entityctrl, false);
		SetPaneVisibility(m_warpctrl, false);
	}
	UpdateFrame();
	UpdateUI();
}

void RoomViewerFrame::UpdateFrame()
{
	m_layerctrl->EnableLayers(m_mode != RoomViewerCtrl::Mode::HEIGHTMAP);
	m_roomview->SetRoomNum(m_roomnum, m_mode);
	FireEvent(EVT_STATUSBAR_UPDATE);
	FireEvent(EVT_PROPERTIES_UPDATE);
}

void RoomViewerFrame::SetGameData(std::shared_ptr<GameData> gd)
{
	m_g = gd;
	if (m_roomview)
	{
		m_roomview->SetGameData(gd);
	}
	m_mode = RoomViewerCtrl::Mode::NORMAL;
	UpdateFrame();
}

void RoomViewerFrame::ClearGameData()
{
	m_g = nullptr;
	if (m_roomview)
	{
		m_roomview->ClearGameData();
	}
	m_mode = RoomViewerCtrl::Mode::NORMAL;
	UpdateFrame();
}

void RoomViewerFrame::SetRoomNum(uint16_t roomnum, RoomViewerCtrl::Mode mode)
{
	if (m_roomnum != roomnum)
	{
		m_reset_props = true;
	}
	m_roomnum = roomnum;
	m_mode = mode;
	UpdateFrame();
}

bool RoomViewerFrame::ExportBin(const std::string& path)
{
	auto data = m_g->GetRoomData()->GetMapForRoom(m_roomnum);
	WriteBytes(*data->GetBytes(), path);
	return true;
}

bool RoomViewerFrame::ExportCsv(const std::array<std::string, 3>& paths)
{
	auto data = m_g->GetRoomData()->GetMapForRoom(m_roomnum)->GetData();
	std::ofstream bg(paths[0], std::ios::out | std::ios::trunc);
	std::ofstream fg(paths[1], std::ios::out | std::ios::trunc);
	std::ofstream hm(paths[2], std::ios::out | std::ios::trunc);


	for (int i = 0; i < data->GetWidth() * data->GetHeight(); ++i)
	{
		fg << StrPrintf("%04X", data->GetBlock(i, Tilemap3D::Layer::FG).value);
		bg << StrPrintf("%04X", data->GetBlock(i, Tilemap3D::Layer::BG).value);
		if ((i + 1) % data->GetWidth() == 0)
		{
			fg << std::endl;
			bg << std::endl;
		}
		else
		{
			fg << ",";
			bg << ",";
		}
	}
	hm << StrPrintf("%02X", data->GetLeft()) << "," << StrPrintf("%02X",data->GetTop()) << std::endl;
	for (int i = 0; i < data->GetHeightmapHeight(); ++i)
		for (int j = 0; j < data->GetHeightmapWidth(); ++j)
		{
			hm << StrPrintf("%X%X%02X", data->GetCellProps({ j, i }), data->GetHeight({ j, i }), data->GetCellType({j, i}));
			if ((j + 1) % data->GetHeightmapWidth() == 0)
			{
				hm << std::endl;
			}
			else
			{
				hm << ",";
			}
		}

	return true;
}

bool RoomViewerFrame::ExportPng(const std::string& path)
{
	auto map = m_g->GetRoomData()->GetMapForRoom(m_roomnum)->GetData();
	auto blocksets = m_g->GetRoomData()->GetBlocksetsForRoom(m_roomnum);
	auto palette = std::vector<std::shared_ptr<Palette>>{ m_g->GetRoomData()->GetPaletteForRoom(m_roomnum)->GetData() };
	auto tileset = m_g->GetRoomData()->GetTilesetForRoom(m_roomnum)->GetData();
	auto blockset = m_g->GetRoomData()->GetCombinedBlocksetForRoom(m_roomnum);

	int width = map->GetPixelWidth();
	int height = map->GetPixelHeight();
	ImageBuffer buf(width, height);

	buf.Insert3DMapLayer(0, 0, 0, Tilemap3D::Layer::BG, map, tileset, blockset);
	buf.Insert3DMapLayer(0, 0, 0, Tilemap3D::Layer::FG, map, tileset, blockset);
	return buf.WritePNG(path, palette, false);
}

bool RoomViewerFrame::ImportBin(const std::string& path)
{
	auto bytes = ReadBytes(path);
	auto data = m_g->GetRoomData()->GetMapForRoom(m_roomnum);
	data->GetData()->Decode(bytes.data());
	return true;
}

bool RoomViewerFrame::ImportCsv(const std::array<std::string, 3>& paths)
{
	auto data = m_g->GetRoomData()->GetMapForRoom(m_roomnum)->GetData();
	std::ifstream bg(paths[0], std::ios::in);
	std::ifstream fg(paths[1], std::ios::in);
	std::ifstream hm(paths[2], std::ios::in);
	
	int w, h, t, l, hw, hh;
	std::vector<std::vector<uint16_t>> foreground, background, heightmap;

	auto read_csv = [](auto& iss, auto& data)
	{
		std::string row;
		std::string cell;
		while (std::getline(iss, row))
		{
			data.push_back(std::vector<uint16_t>());
			std::istringstream rss(row);
			while (std::getline(rss, cell, ','))
			{
				data.back().push_back(std::stoi(cell, nullptr, 16));
			}
		}
	};
	
	read_csv(fg, foreground);
	read_csv(bg, background);
	read_csv(hm, heightmap);

	if (heightmap.size() < 2 || heightmap.front().size() != 2)
	{
		return false;
	}
	if (foreground.size() == 0 || foreground.front().size() == 0)
	{
		return false;
	}
	if (background.size() == 0 || background.front().size() == 0)
	{
		return false;
	}
	w = foreground.front().size();
	h = foreground.size();
	hw = heightmap[1].size();
	hh = heightmap.size() - 1;
	l = heightmap[0][0];
	t = heightmap[0][1];

	if (background.size() != h)
	{
		return false;
	}

	for (int i = 0; i < h; ++i)
	{
		if (background[i].size() != w || foreground[i].size() != w)
		{
			return false;
		}
	}
	for (int i = 1; i <= hh; ++i)
	{
		if (heightmap[i].size() != hw)
		{
			return false;
		}
	}

	data->Resize(w, h);
	data->ResizeHeightmap(hw, hh);
	data->SetLeft(l);
	data->SetTop(t);
	
	int i = 0;
	for (int y = 0; y < h; ++y)
	{
		for (int x = 0; x < w; ++x)
		{
			data->SetBlock(background[y][x], i, Tilemap3D::Layer::BG);
			data->SetBlock(foreground[y][x], i++, Tilemap3D::Layer::FG);
		}
	}
	for (int y = 0; y < hh; ++y)
	{
		for (int x = 0; x < hw; ++x)
		{
			data->SetCellProps({ x, y }, (heightmap[y + 1][x] >> 12) & 0xF);
			data->SetHeight({ x, y }, (heightmap[y + 1][x] >> 8) & 0xF);
			data->SetCellType({ x, y }, heightmap[y + 1][x] & 0xFF);
		}
	}

	return true;
}

bool RoomViewerFrame::HandleKeyDown(unsigned int key, unsigned int modifiers)
{
	if (m_roomview != nullptr)
	{
		return m_roomview->HandleKeyDown(key, modifiers);
	}
	return true;
}

void RoomViewerFrame::InitStatusBar(wxStatusBar& status) const
{
	status.SetFieldsCount(3);
	status.SetStatusText("", 0);
	status.SetStatusText("", 1);
	status.SetStatusText("", 1);
}

void RoomViewerFrame::UpdateStatusBar(wxStatusBar& status) const
{
	if (status.GetFieldsCount() != 3)
	{
		return;
	}
	status.SetStatusText(m_roomview->GetStatusText(), 0);
	if (m_roomview->IsEntitySelected())
	{
		auto& entity = m_roomview->GetSelectedEntity();
		int idx = m_roomview->GetSelectedEntityIndex();
		status.SetStatusText(StrPrintf("Selected Entity %d (%04.1f, %04.1f, %04.1f) - %s",
			idx, entity.GetXDbl(), entity.GetYDbl(), entity.GetZDbl(), entity.GetTypeName().c_str()), 1);
	}
	else
	{
		status.SetStatusText("", 1);
	}
	if (m_roomview->GetErrorCount() > 0)
	{
		status.SetStatusText(StrPrintf("Total Errors: %d, Error #1 : %s",
			m_roomview->GetErrorCount(), m_roomview->GetErrorText(0).c_str()), 2);
	}
	else
	{
		status.SetStatusText("", 2);
	}
}

void RoomViewerFrame::InitProperties(wxPropertyGridManager& props) const
{
	if (m_g && ArePropsInitialised() == false)
	{
		RefreshLists();
		props.GetGrid()->Clear();
		const auto rd = m_g->GetRoomData()->GetRoom(m_roomnum);
		auto tm = m_g->GetRoomData()->GetMapForRoom(m_roomnum);

		props.Append(new wxPropertyCategory("Main", "Main"));
		props.Append(new wxStringProperty("Name", "Name", rd->name));
		props.Append(new wxIntProperty("Room Number", "RN", rd->index))->Enable(false);
		props.Append(new wxEnumProperty("Tileset", "TS", m_tilesets));
		props.Append(new wxEnumProperty("Room Palette", "RP", m_palettes))->SetChoiceSelection(rd->room_palette);
		props.Append(new wxEnumProperty("Primary Blockset", "PBT", m_pri_blocksets));
		props.Append(new wxEnumProperty("Secondary Blockset", "SBT", m_sec_blocksets));
		props.Append(new wxEnumProperty("BGM", "BGM", m_bgms))->SetChoiceSelection(rd->bgm);
		props.Append(new wxEnumProperty("Map", "M", m_maps));
		props.Append(new wxIntProperty("Unknown Parameter 1", "UP1", rd->unknown_param1));
		props.Append(new wxIntProperty("Unknown Parameter 2", "UP2", rd->unknown_param2));
		props.Append(new wxIntProperty("Z Begin", "ZB", rd->room_z_begin));
		props.Append(new wxIntProperty("Z End", "ZE", rd->room_z_end));
		props.Append(new wxPropertyCategory("Map", "Map"));
		props.Append(new wxIntProperty("Tilemap Left Offset", "TLO", tm->GetData()->GetLeft()));
		props.Append(new wxIntProperty("Tilemap Top Offset", "TTO", tm->GetData()->GetTop()));
		props.Append(new wxIntProperty("Tilemap Width", "TW", tm->GetData()->GetWidth()))->Enable(false);
		props.Append(new wxIntProperty("Tilemap Height", "TH", tm->GetData()->GetHeight()))->Enable(false);
		props.Append(new wxIntProperty("Heightmap Width", "HW", tm->GetData()->GetHeightmapWidth()))->Enable(false);
		props.Append(new wxIntProperty("Heightmap Height", "HH", tm->GetData()->GetHeightmapHeight()))->Enable(false);
		props.Append(new wxPropertyCategory("Warps", "Warps"));
		props.Append(new wxEnumProperty("Fall Destination", "FD", m_rooms));
		props.Append(new wxEnumProperty("Climb Destination", "CD",m_rooms));
		props.Append(new wxPropertyCategory("Flags", "Flags"));
		props.Append(new wxIntProperty("Visit Flag", "VF", m_g->GetStringData()->GetRoomVisitFlag(m_roomnum)));
		props.Append(new wxPropertyCategory("Misc", "Misc"));
		props.Append(new wxEnumProperty("Save Location String", "SLS", m_menustrings));
		props.Append(new wxEnumProperty("Island Location String", "ILS", m_menustrings));
		props.Append(new wxIntProperty("Island Position", "ILP", -1));
		EditorFrame::InitProperties(props);
		RefreshProperties(props);
	}
}

void RoomViewerFrame::RefreshLists() const
{
	const auto rd = m_g->GetRoomData()->GetRoom(m_roomnum);

	m_bgms.Clear();
	m_bgms.Add("[00] The Marquis' Invitation");
	m_bgms.Add("[01] Premonition of Trouble");
	m_bgms.Add("[02] Overworld");
	m_bgms.Add("[03] Bustling Street");
	m_bgms.Add("[04] Torchlight");
	m_bgms.Add("[05] Prayers to God");
	m_bgms.Add("[06] Gumi");
	m_bgms.Add("[07] Beneath the Mysterious Tree");
	m_bgms.Add("[08] Overworld");
	m_bgms.Add("[09] The King's Chamber");
	m_bgms.Add("[0A] Mysterious Island");
	m_bgms.Add("[0B] The Death God's Invitation");
	m_bgms.Add("[0C] Deserted Street Corner");
	m_bgms.Add("[0D] Labrynth");
	m_bgms.Add("[0E] The Silence, the Darkness, and...");
	m_bgms.Add("[0F] Light of the Setting Sun");
	m_bgms.Add("[10] Friday and a Soft Breeze");
	m_bgms.Add("[11] Divine Guardian of the Maze");
	m_bgms.Add("[12] Fade Out");

	m_palettes.Clear();
	for (const auto& p : m_g->GetRoomData()->GetRoomPalettes())
	{
		m_palettes.Add(_(p->GetName()));
	}

	m_tilesets.Clear();
	for (const auto& p : m_g->GetRoomData()->GetTilesets())
	{
		m_tilesets.Add(_(p->GetName()));
	}
	m_pri_blocksets.Clear();
	m_sec_blocksets.Clear();
	for (const auto& p : m_g->GetRoomData()->GetAllBlocksets())
	{
		if (p.second->GetTileset() == rd->tileset)
		{
			if (p.second->GetSecondary() == 0)
			{
				m_pri_blocksets.Add(_(p.first));
			}
			else if (p.second->GetPrimary() == rd->pri_blockset)
			{
				m_sec_blocksets.Add(_(p.first));
			}
		}
	}
	m_maps.Clear();
	for (const auto& map : m_g->GetRoomData()->GetMaps())
	{
		m_maps.Add(map.first);
	}
	m_rooms.Clear();
	m_rooms.Add("<NONE>");
	for (const auto& room : m_g->GetRoomData()->GetRoomlist())
	{
		m_rooms.Add(room->name);
	}
	m_menustrings.Clear();
	m_menustrings.Add("<NONE>");
	for (int i = 0; i < m_g->GetStringData()->GetItemNameCount(); ++i)
	{
		m_menustrings.Add(m_g->GetStringData()->GetItemName(i));
	}
	for (int i = 0; i < m_g->GetStringData()->GetMenuStrCount(); ++i)
	{
		m_menustrings.Add(m_g->GetStringData()->GetMenuStr(i));
	}
}

void RoomViewerFrame::UpdateProperties(wxPropertyGridManager& props) const
{
	EditorFrame::UpdateProperties(props);
	if (ArePropsInitialised() == true)
	{
		if (m_reset_props)
		{
			props.GetGrid()->ClearModifiedStatus();
			m_reset_props = false;
		}
		RefreshProperties(props);
	}
}

void RoomViewerFrame::RefreshProperties(wxPropertyGridManager& props) const
{
	if (m_g != nullptr)
	{
		RefreshLists();
		props.GetGrid()->Freeze();
		props.GetGrid()->GetProperty("PBT")->SetChoices(m_pri_blocksets);
		props.GetGrid()->GetProperty("SBT")->SetChoices(m_sec_blocksets);

		const auto rd = m_g->GetRoomData()->GetRoom(m_roomnum);
		auto tm = m_g->GetRoomData()->GetMapForRoom(m_roomnum);

		props.GetGrid()->SetPropertyValue("Name", _(rd->name));
		props.GetGrid()->SetPropertyValue("RN", rd->index);
		props.GetGrid()->GetProperty("TS")->SetChoiceSelection(rd->tileset);
		props.GetGrid()->GetProperty("RP")->SetChoiceSelection(rd->room_palette);
		props.GetGrid()->GetProperty("BGM")->SetChoiceSelection(rd->bgm);
		props.GetGrid()->GetProperty("PBT")->SetChoiceSelection(rd->pri_blockset);
		props.GetGrid()->GetProperty("SBT")->SetChoiceSelection(rd->sec_blockset);
		props.GetGrid()->GetProperty("M")->SetChoiceSelection(m_maps.Index(rd->map));
		props.GetGrid()->SetPropertyValue("UP1", rd->unknown_param1);
		props.GetGrid()->SetPropertyValue("UP2", rd->unknown_param2);
		props.GetGrid()->SetPropertyValue("ZB", rd->room_z_begin);
		props.GetGrid()->SetPropertyValue("ZE", rd->room_z_end);
		props.GetGrid()->SetPropertyValue("TLO", tm->GetData()->GetLeft());
		props.GetGrid()->SetPropertyValue("TTO", tm->GetData()->GetTop());
		props.GetGrid()->SetPropertyValue("TW", tm->GetData()->GetWidth());
		props.GetGrid()->SetPropertyValue("TH", tm->GetData()->GetHeight());
		props.GetGrid()->SetPropertyValue("HW", tm->GetData()->GetHeightmapWidth());
		props.GetGrid()->SetPropertyValue("HH", tm->GetData()->GetHeightmapHeight());
		int fall = m_g->GetRoomData()->HasFallDestination(m_roomnum) ? (m_g->GetRoomData()->GetFallDestination(m_roomnum) + 1) : 0;
		int climb = m_g->GetRoomData()->HasClimbDestination(m_roomnum) ? (m_g->GetRoomData()->GetClimbDestination(m_roomnum) + 1) : 0;
		props.GetGrid()->GetProperty("FD")->SetChoiceSelection(fall);
		props.GetGrid()->GetProperty("CD")->SetChoiceSelection(climb);
		props.GetGrid()->SetPropertyValue("VF", m_g->GetStringData()->GetRoomVisitFlag(m_roomnum));
		int save_loc = m_g->GetStringData()->GetSaveLocation(m_roomnum);
		save_loc = save_loc == 0xFF ? 0 : save_loc + 1;
		int map_loc = m_g->GetStringData()->GetMapLocation(m_roomnum);
		map_loc = map_loc == 0xFF ? 0 : map_loc + 1;
		props.GetGrid()->GetProperty("SLS")->SetChoiceSelection(save_loc);
		props.GetGrid()->GetProperty("ILS")->SetChoiceSelection(map_loc);
		props.GetGrid()->SetPropertyValue("ILP", m_g->GetStringData()->GetMapPosition(m_roomnum));
		props.GetGrid()->Thaw();
	}
}

void RoomViewerFrame::OnPropertyChange(wxPropertyGridEvent& evt)
{
	auto* ctrl = static_cast<wxPropertyGridManager*>(evt.GetEventObject());
	ctrl->GetGrid()->Freeze();
	wxPGProperty* property = evt.GetProperty();
	if (property == nullptr)
	{
		return;
	}
	const auto rd = m_g->GetRoomData()->GetRoom(m_roomnum);
	auto tm = m_g->GetRoomData()->GetMapForRoom(m_roomnum);

	const wxString& name = property->GetName();
	if (name == "TS")
	{
		if (property->GetChoiceSelection() != rd->tileset)
		{
			rd->tileset = property->GetChoiceSelection();
			if (m_g->GetRoomData()->GetBlockset(rd->tileset, rd->pri_blockset, 0) == nullptr)
			{
				rd->pri_blockset = 0;
			}
			if (m_g->GetRoomData()->GetBlockset(rd->tileset, rd->pri_blockset, rd->sec_blockset + 1) == nullptr)
			{
				rd->sec_blockset = 0;
			}
			

			UpdateFrame();
		}
	}
	else if (name == "RP")
	{
		if (property->GetChoiceSelection() != rd->room_palette)
		{
			rd->room_palette = property->GetChoiceSelection();
			UpdateFrame();
		}
	}
	else if (name == "PBT")
	{
		if (property->GetChoiceSelection() != rd->pri_blockset)
		{
			rd->pri_blockset = property->GetChoiceSelection();
			if (m_g->GetRoomData()->GetBlockset(rd->tileset, rd->pri_blockset, rd->sec_blockset + 1) == nullptr)
			{
				rd->sec_blockset = 0;
			}
			UpdateFrame();
		}
	}
	else if (name == "SBT")
	{
		if (property->GetChoiceSelection() != rd->sec_blockset)
		{
			rd->sec_blockset = property->GetChoiceSelection();
			UpdateFrame();
		}
	}
	else if (name == "BGM")
	{
		if (property->GetChoiceSelection() != rd->bgm)
		{
			rd->bgm = property->GetChoiceSelection();
			FireEvent(EVT_STATUSBAR_UPDATE);
			FireEvent(EVT_PROPERTIES_UPDATE);
		}
	}
	else if (name == "M")
	{
		const auto map_name = m_maps.GetLabel(property->GetChoiceSelection());
		if (map_name != rd->map)
		{
			rd->map = map_name;
			UpdateFrame();
		}
	}
	else if (name == "ZB")
	{
		if (property->GetValuePlain().GetLong() != rd->room_z_begin)
		{
			rd->room_z_begin = std::clamp<uint8_t>(property->GetValuePlain().GetLong(), 0, 15);
			FireEvent(EVT_PROPERTIES_UPDATE);
		}
	}
	else if (name == "ZE")
	{
		if (property->GetValuePlain().GetLong() != rd->room_z_end)
		{
			rd->room_z_end = std::clamp<uint8_t>(property->GetValuePlain().GetLong(), 0, 15);
			FireEvent(EVT_PROPERTIES_UPDATE);
		}
	}
	else if (name == "UP1")
	{
		if (property->GetValuePlain().GetLong() != rd->unknown_param1)
		{
			rd->unknown_param1 = std::clamp<uint8_t>(property->GetValuePlain().GetLong(), 0, 3);
			FireEvent(EVT_PROPERTIES_UPDATE);
		}
	}
	else if (name == "UP2")
	{
		if (property->GetValuePlain().GetLong() != rd->unknown_param2)
		{
			rd->unknown_param2 = std::clamp<uint8_t>(property->GetValuePlain().GetLong(), 0, 3);
			FireEvent(EVT_PROPERTIES_UPDATE);
		}
	}
	else if (name == "TLO")
	{
		if (property->GetValuePlain().GetLong() != tm->GetData()->GetLeft())
		{
			tm->GetData()->SetLeft(static_cast<uint8_t>(property->GetValuePlain().GetLong()));
			UpdateFrame();
		}
	}
	else if (name == "TTO")
	{
		if (property->GetValuePlain().GetLong() != tm->GetData()->GetTop())
		{
			tm->GetData()->SetTop(static_cast<uint8_t>(property->GetValuePlain().GetLong()));
			UpdateFrame();
		}
	}
	else if (name == "VF")
	{
		if (property->GetValuePlain().GetLong() != m_g->GetStringData()->GetRoomVisitFlag(m_roomnum))
		{
			m_g->GetStringData()->SetRoomVisitFlag(m_roomnum, static_cast<uint16_t>(property->GetValuePlain().GetLong()));
			UpdateFrame();
		}
	}
	else if (name == "FD")
	{
		bool enabled = property->GetChoiceSelection() != 0;
		int room = property->GetChoiceSelection() - 1;
		if (m_g->GetRoomData()->HasFallDestination(m_roomnum) != enabled ||
			(enabled && m_g->GetRoomData()->GetFallDestination(m_roomnum) != room))
		{
			m_g->GetRoomData()->SetHasFallDestination(m_roomnum, enabled);
			if (enabled)
			{
				m_g->GetRoomData()->SetFallDestination(m_roomnum, room);
			}
			UpdateFrame();
		}
	}
	else if (name == "CD")
	{
		bool enabled = property->GetChoiceSelection() != 0;
		int room = property->GetChoiceSelection() - 1;
		if (m_g->GetRoomData()->HasClimbDestination(m_roomnum) != enabled ||
			(enabled && m_g->GetRoomData()->GetClimbDestination(m_roomnum) != room))
		{
			m_g->GetRoomData()->SetHasClimbDestination(m_roomnum, enabled);
			if (enabled)
			{
				m_g->GetRoomData()->SetClimbDestination(m_roomnum, room);
			}
			UpdateFrame();
		}
	}
	else if (name == "SLS")
	{
		bool enabled = property->GetChoiceSelection() != 0;
		uint8_t string = enabled ? property->GetChoiceSelection() - 1 : 0xFF;
		if (m_g->GetStringData()->GetSaveLocation(m_roomnum) != string)
		{
			m_g->GetStringData()->SetSaveLocation(m_roomnum, string);
		}
	}
	else if (name == "ILS")
	{
		bool enabled = property->GetChoiceSelection() != 0;
		uint8_t string = enabled ? property->GetChoiceSelection() - 1 : 0xFF;
		auto position = enabled ? m_g->GetStringData()->GetMapPosition(m_roomnum) : 0xFF;
		if (m_g->GetStringData()->GetMapLocation(m_roomnum) != string)
		{
			m_g->GetStringData()->SetMapLocation(m_roomnum, string, position);
		}
	}
	else if (name == "ILP")
	{
		uint8_t position = property->GetValuePlain().GetLong();
		auto string = m_g->GetStringData()->GetMapLocation(m_roomnum);
		if (string != 0xFF && m_g->GetStringData()->GetMapPosition(m_roomnum) != position)
		{
			m_g->GetStringData()->SetMapLocation(m_roomnum, string, position);
		}
	}
	ctrl->GetGrid()->Thaw();
}

void RoomViewerFrame::InitMenu(wxMenuBar& menu, ImageList& ilist) const
{
	auto* parent = m_mgr.GetManagedWindow();

	ClearMenu(menu);
	auto& fileMenu = *menu.GetMenu(menu.FindMenu("File"));
	AddMenuItem(fileMenu, 0, wxID_ANY, "", wxITEM_SEPARATOR);
	AddMenuItem(fileMenu, 1, ID_FILE_EXPORT_BIN, "Export Map as Binary...");
	AddMenuItem(fileMenu, 2, ID_FILE_EXPORT_CSV, "Export Map as CSV Set...");
	AddMenuItem(fileMenu, 3, ID_FILE_EXPORT_PNG, "Export Map as PNG...");
	AddMenuItem(fileMenu, 4, ID_FILE_IMPORT_BIN, "Import Map from Binary...");
	AddMenuItem(fileMenu, 5, ID_FILE_IMPORT_CSV, "Import Map from CSV...");

	auto& editMenu = AddMenu(menu, 1, ID_EDIT, "Edit");
	AddMenuItem(editMenu, 0, ID_EDIT_ENTITY_PROPERTIES, "Entity Properties...");
	AddMenuItem(editMenu, 1, ID_EDIT_FLAGS, "Flags...");
	AddMenuItem(editMenu, 1, ID_EDIT_CHESTS, "Chests...");

	auto& viewMenu = AddMenu(menu, 2, ID_VIEW, "View");
	AddMenuItem(viewMenu, 0, ID_VIEW_NORMAL, "Normal", wxITEM_RADIO);
	AddMenuItem(viewMenu, 1, ID_VIEW_HEIGHTMAP, "Heightmap Editor", wxITEM_RADIO);
	AddMenuItem(viewMenu, 1, ID_VIEW_MAP, "Map Editor", wxITEM_RADIO);
	AddMenuItem(viewMenu, 2, wxID_ANY, "", wxITEM_SEPARATOR);
	AddMenuItem(viewMenu, 3, ID_VIEW_ENTITIES, "Show Entities", wxITEM_CHECK);
	AddMenuItem(viewMenu, 4, ID_VIEW_ENTITY_HITBOX, "Show Entity Hitboxes", wxITEM_CHECK);
	AddMenuItem(viewMenu, 5, ID_VIEW_WARPS, "Show Warps", wxITEM_CHECK);

	auto& toolsMenu = AddMenu(menu, 3, ID_TOOLS, "Tools");
	AddMenuItem(toolsMenu, 0, ID_TOOLS_LAYERS, "Layers", wxITEM_CHECK);
	AddMenuItem(toolsMenu, 1, ID_TOOLS_ENTITIES, "Entity List", wxITEM_CHECK);
	AddMenuItem(toolsMenu, 2, ID_TOOLS_WARPS, "Warp List", wxITEM_CHECK);

	wxAuiToolBar* main_tb = new wxAuiToolBar(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE | wxAUI_TB_HORIZONTAL);
	main_tb->SetToolBitmapSize(wxSize(16, 16));
	main_tb->AddTool(TOOL_NORMAL_MODE, "Normal Mode", ilist.GetImage("room"), "Normal Mode", wxITEM_RADIO);
	main_tb->AddTool(TOOL_HEIGHTMAP_MODE, "Heightmap Edit Mode", ilist.GetImage("heightmap"), "Heightmap Edit Mode", wxITEM_RADIO);
	main_tb->AddTool(TOOL_MAP_MODE, "Map Edit Mode", ilist.GetImage("map"), "Map Edit Mode", wxITEM_RADIO);
	main_tb->AddSeparator();
	main_tb->AddTool(TOOL_TOGGLE_ENTITIES, "Entities Visible", ilist.GetImage("entity"), "Entities Visible", wxITEM_CHECK);
	main_tb->AddTool(TOOL_TOGGLE_ENTITY_HITBOX, "Entity Hitboxes Visible", ilist.GetImage("ehitbox"), "Entity Hitboxes Visible", wxITEM_CHECK);
	main_tb->AddTool(TOOL_TOGGLE_WARPS, "Warps Visible", ilist.GetImage("warp"), "Warps Visible", wxITEM_CHECK);
	main_tb->AddSeparator();
	main_tb->AddTool(TOOL_SHOW_FLAGS, "Flags", ilist.GetImage("flags"), "Flags");
	main_tb->AddTool(TOOL_SHOW_CHESTS, "Chests", ilist.GetImage("chest16"), "Chests");
	main_tb->AddTool(TOOL_SHOW_SELECTION_PROPERTIES, "Selection Properties", ilist.GetImage("properties"), "Selection Properties");
	main_tb->AddSeparator();
	main_tb->AddTool(TOOL_SHOW_LAYERS_PANE, "Layers Pane", ilist.GetImage("layers"), "Layers Pane", wxITEM_CHECK);
	main_tb->AddTool(TOOL_SHOW_ENTITIES_PANE, "Entities Pane", ilist.GetImage("epanel"), "Entities Pane", wxITEM_CHECK);
	main_tb->AddTool(TOOL_SHOW_WARPS_PANE, "Warps Pane", ilist.GetImage("wpanel"), "Warps Pane", wxITEM_CHECK);
	main_tb->AddSeparator();
	main_tb->AddTool(TOOL_SHOW_ERRORS, "Show Errors", ilist.GetImage("warning"), "Show Errors");
	AddToolbar(m_mgr, *main_tb, "Main", "Main Tools", wxAuiPaneInfo().ToolbarPane().Top().Row(1).Position(1));

	UpdateUI();

	m_mgr.Update();
}

void RoomViewerFrame::OnMenuClick(wxMenuEvent& evt)
{
	const auto id = evt.GetId();
	if ((id >= 20000) && (id < 31000))
	{
		switch (id)
		{
		case ID_FILE_EXPORT_BIN:
			OnExportBin();
			break;
		case ID_FILE_EXPORT_CSV:
			OnExportCsv();
			break;
		case ID_FILE_EXPORT_PNG:
			OnExportPng();
			break;
		case ID_FILE_IMPORT_BIN:
			OnImportBin();
			break;
		case ID_FILE_IMPORT_CSV:
			OnImportCsv();
			break;
		case ID_VIEW_NORMAL:
		case TOOL_NORMAL_MODE:
			SetMode(RoomViewerCtrl::Mode::NORMAL);
			break;
		case ID_VIEW_HEIGHTMAP:
		case TOOL_HEIGHTMAP_MODE:
			SetMode(RoomViewerCtrl::Mode::HEIGHTMAP);
			break;
		case ID_VIEW_ENTITIES:
		case TOOL_TOGGLE_ENTITIES:
			m_roomview->SetEntitiesVisible(!m_roomview->GetEntitiesVisible());
			break;
		case ID_VIEW_ENTITY_HITBOX:
		case TOOL_TOGGLE_ENTITY_HITBOX:
			m_roomview->SetEntitiesHitboxVisible(!m_roomview->GetEntitiesHitboxVisible());
			break;
		case ID_VIEW_WARPS:
		case TOOL_TOGGLE_WARPS:
			m_roomview->SetWarpsVisible(!m_roomview->GetWarpsVisible());
			break;
		case ID_TOOLS_LAYERS:
		case TOOL_SHOW_LAYERS_PANE:
			SetPaneVisibility(m_layerctrl, !IsPaneVisible(m_layerctrl));
			break;
		case ID_TOOLS_ENTITIES:
		case TOOL_SHOW_ENTITIES_PANE:
			SetPaneVisibility(m_entityctrl, !IsPaneVisible(m_entityctrl));
			break;
		case ID_TOOLS_WARPS:
		case TOOL_SHOW_WARPS_PANE:
			SetPaneVisibility(m_warpctrl, !IsPaneVisible(m_warpctrl));
			break;
		case ID_EDIT_ENTITY_PROPERTIES:
		case TOOL_SHOW_SELECTION_PROPERTIES:
			if (m_roomview->IsEntitySelected())
			{
				m_roomview->UpdateEntityProperties(m_roomview->GetSelectedEntityIndex());
			}
			else if (m_roomview->IsWarpSelected())
			{
				m_roomview->UpdateWarpProperties(m_roomview->GetSelectedWarpIndex());
			}
			break;
		default:
			wxMessageBox(wxString::Format("Unrecognised Event %d", evt.GetId()));
		}
		UpdateUI();
	}
}

void RoomViewerFrame::OnExportBin()
{
	auto rd = m_g->GetRoomData()->GetRoom(m_roomnum);
	const wxString default_file = rd->name + ".bin";
	wxFileDialog fd(this, _("Export Map As Binary"), "", default_file, "Room Map (*.cmp)|*.cmp|All Files (*.*)|*.*", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if (fd.ShowModal() != wxID_CANCEL)
	{
		ExportBin(fd.GetPath().ToStdString());
	}
}

void RoomViewerFrame::OnExportCsv()
{
	auto rd = m_g->GetRoomData()->GetRoom(m_roomnum);
	std::array<std::string, 3> fnames;
	wxString default_file = rd->name + "_background.csv";
	wxFileDialog fd(this, _("Export Background Layer as CSV"), "", default_file, "CSV File (*.csv)|*.csv|All Files (*.*)|*.*", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if (fd.ShowModal() != wxID_CANCEL)
	{
		fnames[0] = fd.GetPath().ToStdString();
	}
	default_file = rd->name + "_foreground.csv";
	fd.Create(this, _("Export Foreground Layer as CSV"), "", default_file, "CSV File (*.csv)|*.csv|All Files (*.*)|*.*", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if (fd.ShowModal() != wxID_CANCEL)
	{
		fnames[1] = fd.GetPath().ToStdString();
	}
	default_file = rd->name + "_heightmap.csv";
	fd.Create(this, _("Export Heightmap Layer as CSV"), "", default_file, "CSV File (*.csv)|*.csv|All Files (*.*)|*.*", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if (fd.ShowModal() != wxID_CANCEL)
	{
		fnames[2] = fd.GetPath().ToStdString();
	}
	ExportCsv(fnames);
}

void RoomViewerFrame::OnExportPng()
{
	auto rd = m_g->GetRoomData()->GetRoom(m_roomnum);
	const wxString default_file = rd->name + ".png";
	wxFileDialog fd(this, _("Export Map As PNG"), "", default_file, "PNG Image (*.png)|*.png|All Files (*.*)|*.*", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if (fd.ShowModal() != wxID_CANCEL)
	{
		ExportPng(fd.GetPath().ToStdString());
	}
}

void RoomViewerFrame::OnImportBin()
{
	wxFileDialog fd(this, _("Import Map From Binary"), "", "", "Room Map (*.cmp)|*.cmp|All Files (*.*)|*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if (fd.ShowModal() != wxID_CANCEL)
	{
		std::string path = fd.GetPath().ToStdString();
		ImportBin(path);
	}
	UpdateFrame();
}

void RoomViewerFrame::OnImportCsv()
{
	std::array<std::string, 3> filenames;
	wxFileDialog fd(this, _("Import Background Layer"), "", "", "CSV File (*.csv)|*.csv|All Files (*.*)|*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if (fd.ShowModal() != wxID_CANCEL)
	{
		filenames[0] = fd.GetPath().ToStdString();
	}
	fd.Create(this, _("Import Foreground Layer"), "", "", "CSV File (*.csv)|*.csv|All Files (*.*)|*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if (fd.ShowModal() != wxID_CANCEL)
	{
		filenames[1] = fd.GetPath().ToStdString();
	}
	fd.Create(this, _("Import Heightmap Data"), "", "", "CSV File (*.csv)|*.csv|All Files (*.*)|*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if (fd.ShowModal() != wxID_CANCEL)
	{
		filenames[2] = fd.GetPath().ToStdString();
	}
	ImportCsv(filenames);
	UpdateFrame();
}


void RoomViewerFrame::UpdateUI() const
{
	EnableMenuItem(ID_EDIT_FLAGS, false);
	EnableToolbarItem("Main", TOOL_SHOW_FLAGS, false);
	EnableMenuItem(ID_EDIT_CHESTS, false);
	EnableToolbarItem("Main", TOOL_SHOW_CHESTS, false);
	EnableMenuItem(ID_VIEW_MAP, false);
	EnableToolbarItem("Main", TOOL_MAP_MODE, false);

	if (m_mode == RoomViewerCtrl::Mode::NORMAL)
	{
		CheckMenuItem(ID_VIEW_NORMAL, true);
		CheckToolbarItem("Main", TOOL_NORMAL_MODE, true);

		EnableMenuItem(ID_VIEW_ENTITIES, true);
		CheckMenuItem(ID_VIEW_ENTITIES, m_roomview->GetEntitiesVisible());
		EnableToolbarItem("Main", TOOL_TOGGLE_ENTITIES, true);
		CheckToolbarItem("Main", TOOL_TOGGLE_ENTITIES, m_roomview->GetEntitiesVisible());

		EnableMenuItem(ID_VIEW_ENTITY_HITBOX, true);
		CheckMenuItem(ID_VIEW_ENTITY_HITBOX, m_roomview->GetEntitiesHitboxVisible());
		EnableToolbarItem("Main", TOOL_TOGGLE_ENTITY_HITBOX, true);
		CheckToolbarItem("Main", TOOL_TOGGLE_ENTITY_HITBOX, m_roomview->GetEntitiesHitboxVisible());

		EnableMenuItem(ID_EDIT_ENTITY_PROPERTIES, m_roomview->IsEntitySelected() || m_roomview->IsWarpSelected());
		EnableToolbarItem("Main", TOOL_SHOW_SELECTION_PROPERTIES, m_roomview->IsEntitySelected() || m_roomview->IsWarpSelected());

		EnableMenuItem(ID_TOOLS_LAYERS, true);
		EnableToolbarItem("Main", TOOL_SHOW_LAYERS_PANE, true);
		CheckMenuItem(ID_TOOLS_LAYERS, IsPaneVisible(m_layerctrl));
		CheckToolbarItem("Main", TOOL_SHOW_LAYERS_PANE, IsPaneVisible(m_layerctrl));

		EnableMenuItem(ID_TOOLS_ENTITIES, true);
		EnableToolbarItem("Main", TOOL_SHOW_ENTITIES_PANE, true);
		CheckMenuItem(ID_TOOLS_ENTITIES, IsPaneVisible(m_entityctrl));
		CheckToolbarItem("Main", TOOL_SHOW_ENTITIES_PANE, IsPaneVisible(m_entityctrl));

		EnableMenuItem(ID_TOOLS_WARPS, true);
		EnableToolbarItem("Main", TOOL_SHOW_WARPS_PANE, true);
		CheckMenuItem(ID_TOOLS_WARPS, IsPaneVisible(m_warpctrl));
		CheckToolbarItem("Main", TOOL_SHOW_WARPS_PANE, IsPaneVisible(m_warpctrl));
	}
	else
	{
		CheckMenuItem(ID_VIEW_HEIGHTMAP, true);
		CheckToolbarItem("Main", TOOL_HEIGHTMAP_MODE, true);

		CheckMenuItem(ID_VIEW_ENTITIES, false);
		CheckToolbarItem("Main", TOOL_TOGGLE_ENTITIES, false);
		EnableMenuItem(ID_VIEW_ENTITIES, false);
		EnableToolbarItem("Main", TOOL_TOGGLE_ENTITIES, false);

		CheckMenuItem(ID_VIEW_ENTITY_HITBOX, false);
		CheckToolbarItem("Main", TOOL_TOGGLE_ENTITY_HITBOX, false);
		EnableMenuItem(ID_VIEW_ENTITY_HITBOX, false);
		EnableToolbarItem("Main", TOOL_TOGGLE_ENTITY_HITBOX, false);

		EnableMenuItem(ID_EDIT_ENTITY_PROPERTIES, false);
		EnableToolbarItem("Main", ID_EDIT_ENTITY_PROPERTIES, false);

		CheckMenuItem(ID_TOOLS_LAYERS, false);
		CheckToolbarItem("Main", TOOL_SHOW_LAYERS_PANE, false);
		EnableMenuItem(ID_TOOLS_LAYERS, false);
		EnableToolbarItem("Main", TOOL_SHOW_LAYERS_PANE, false);

		CheckMenuItem(ID_TOOLS_ENTITIES, false);
		CheckToolbarItem("Main", TOOL_SHOW_ENTITIES_PANE, false);
		EnableMenuItem(ID_TOOLS_ENTITIES, false);
		EnableToolbarItem("Main", TOOL_SHOW_ENTITIES_PANE, false);

		CheckMenuItem(ID_TOOLS_WARPS, false);
		CheckToolbarItem("Main", TOOL_SHOW_WARPS_PANE, false);
		EnableMenuItem(ID_TOOLS_WARPS, false);
		EnableToolbarItem("Main", TOOL_SHOW_WARPS_PANE, false);
	}
	CheckMenuItem(ID_VIEW_WARPS, m_roomview->GetWarpsVisible());
	CheckToolbarItem("Main", TOOL_TOGGLE_WARPS, m_roomview->GetWarpsVisible());
}

void RoomViewerFrame::OnKeyDown(wxKeyEvent& evt)
{
	if (m_roomview != nullptr)
	{
		evt.Skip(m_roomview->HandleKeyDown(evt.GetKeyCode(), evt.GetModifiers()));
		return;
	}
	evt.Skip();
}

void RoomViewerFrame::OnZoomChange(wxCommandEvent& evt)
{
	m_roomview->SetZoom(m_layerctrl->GetZoom());
}

void RoomViewerFrame::OnOpacityChange(wxCommandEvent& evt)
{
	LayerControlFrame::Layer layer = static_cast<LayerControlFrame::Layer>(reinterpret_cast<intptr_t>(evt.GetClientData()));
	switch (layer)
	{
	case LayerControlFrame::Layer::BG1:
		m_roomview->SetLayerOpacity(RoomViewerCtrl::Layer::BACKGROUND1, m_layerctrl->GetLayerOpacity(layer));
		break;
	case LayerControlFrame::Layer::BG2:
		m_roomview->SetLayerOpacity(RoomViewerCtrl::Layer::BACKGROUND2, m_layerctrl->GetLayerOpacity(layer));
		break;
	case LayerControlFrame::Layer::FG:
		m_roomview->SetLayerOpacity(RoomViewerCtrl::Layer::FOREGROUND, m_layerctrl->GetLayerOpacity(layer));
		break;
	case LayerControlFrame::Layer::SPRITES:
		m_roomview->SetLayerOpacity(RoomViewerCtrl::Layer::BG_SPRITES, m_layerctrl->GetLayerOpacity(layer));
		m_roomview->SetLayerOpacity(RoomViewerCtrl::Layer::FG_SPRITES, m_layerctrl->GetLayerOpacity(layer));
		break;
	case LayerControlFrame::Layer::HM:
		m_roomview->SetLayerOpacity(RoomViewerCtrl::Layer::HEIGHTMAP, m_layerctrl->GetLayerOpacity(layer));
		break;
	}
	m_roomview->RefreshGraphics();
}

void RoomViewerFrame::OnEntityUpdate(wxCommandEvent& evt)
{
	m_entityctrl->SetEntities(m_roomview->GetEntities());
	m_entityctrl->SetSelected(m_roomview->GetSelectedEntityIndex());
	m_warpctrl->SetSelected(m_roomview->GetSelectedWarpIndex());
	UpdateUI();
}

void RoomViewerFrame::OnEntitySelect(wxCommandEvent& evt)
{
	m_warpctrl->SetSelected(m_roomview->GetSelectedWarpIndex());
	m_roomview->SelectEntity(m_entityctrl->GetSelected());
	UpdateUI();
}

void RoomViewerFrame::OnEntityOpenProperties(wxCommandEvent& evt)
{
	m_warpctrl->SetSelected(m_roomview->GetSelectedWarpIndex());
	m_roomview->SelectEntity(m_entityctrl->GetSelected());
	m_roomview->UpdateEntityProperties(m_entityctrl->GetSelected());
}

void RoomViewerFrame::OnEntityAdd(wxCommandEvent& evt)
{
	m_warpctrl->SetSelected(m_roomview->GetSelectedWarpIndex());
	m_roomview->AddEntity();
}

void RoomViewerFrame::OnEntityDelete(wxCommandEvent& evt)
{
	m_warpctrl->SetSelected(m_roomview->GetSelectedWarpIndex());
	m_roomview->SelectEntity(m_entityctrl->GetSelected());
	m_roomview->DeleteSelectedEntity();
}

void RoomViewerFrame::OnEntityMoveUp(wxCommandEvent& evt)
{
	m_warpctrl->SetSelected(m_roomview->GetSelectedWarpIndex());
	m_roomview->SelectEntity(m_entityctrl->GetSelected());
	m_roomview->MoveSelectedEntityUp();
}

void RoomViewerFrame::OnEntityMoveDown(wxCommandEvent& evt)
{
	m_warpctrl->SetSelected(m_roomview->GetSelectedWarpIndex());
	m_roomview->SelectEntity(m_entityctrl->GetSelected());
	m_roomview->MoveSelectedEntityDown();
}

void RoomViewerFrame::OnWarpUpdate(wxCommandEvent& evt)
{
	m_warpctrl->SetWarps(m_roomview->GetWarps());
	m_warpctrl->SetSelected(m_roomview->GetSelectedWarpIndex());
	m_entityctrl->SetSelected(m_roomview->GetSelectedEntityIndex());
}

void RoomViewerFrame::OnWarpSelect(wxCommandEvent& evt)
{
	m_roomview->SelectWarp(m_warpctrl->GetSelected());
	m_entityctrl->SetSelected(m_roomview->GetSelectedEntityIndex());
}

void RoomViewerFrame::OnWarpOpenProperties(wxCommandEvent& evt)
{
	m_roomview->SelectWarp(m_warpctrl->GetSelected());
	m_roomview->UpdateWarpProperties(m_roomview->GetSelectedWarpIndex());
}

void RoomViewerFrame::OnWarpAdd(wxCommandEvent& evt)
{
	m_roomview->AddWarp();
}

void RoomViewerFrame::OnWarpDelete(wxCommandEvent& evt)
{
	m_roomview->SelectWarp(m_warpctrl->GetSelected());
	m_roomview->DeleteSelectedWarp();
}

void RoomViewerFrame::FireEvent(const wxEventType& e)
{
	wxCommandEvent evt(e);
	evt.SetClientData(this);
	wxPostEvent(this, evt);
}

void RoomViewerFrame::FireEvent(const wxEventType& e, const std::string& userdata)
{
	wxCommandEvent evt(e);
	evt.SetString(userdata);
	evt.SetClientData(this);
	wxPostEvent(this, evt);
}
