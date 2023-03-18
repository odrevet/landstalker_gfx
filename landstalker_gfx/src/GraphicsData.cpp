#include "GraphicsData.h"
#include "AsmUtils.h"

GraphicsData::GraphicsData(const filesystem::path& asm_file)
	: DataManager(asm_file)
{
	if (!LoadAsmFilenames())
	{
		throw std::runtime_error(std::string("Unable to load file data from \'") + asm_file.str() + '\'');
	}
	if (!AsmLoadFonts())
	{
		throw std::runtime_error(std::string("Unable to load font data from \'") + asm_file.str() + '\'');
	}
	if (!AsmLoadStrings())
	{
		throw std::runtime_error(std::string("Unable to load string data from \'") + asm_file.str() + '\'');
	}
	if (!AsmLoadInventoryGraphics())
	{
		throw std::runtime_error(std::string("Unable to load inventory graphics data from \'") + m_inventory_graphics_filename.str() + '\'');
	}
	InitCache();
	UpdateTilesetRecommendedPalettes();
	ResetTilesetDefaultPalettes();
}

GraphicsData::GraphicsData(const Rom& rom)
	: DataManager(rom)
{
	SetDefaultFilenames();
	if (!RomLoadFonts(rom))
	{
		throw std::runtime_error(std::string("Unable to load font data from ROM"));
	}
	if (!RomLoadStrings(rom))
	{
		throw std::runtime_error(std::string("Unable to load string data from ROM"));
	}
	if (!RomLoadInventoryGraphics(rom))
	{
		throw std::runtime_error(std::string("Unable to load inventory graphics from ROM"));
	}
	InitCache();
	UpdateTilesetRecommendedPalettes();
	ResetTilesetDefaultPalettes();
}

bool GraphicsData::Save(const filesystem::path& dir)
{
	auto directory = dir;
	if (directory.exists() && directory.is_file())
	{
		directory = directory.parent_path();
	}
	if (!CreateDirectoryStructure(directory))
	{
		throw std::runtime_error(std::string("Unable to create directory structure at \'") + directory.str() + '\'');
	}
	if (!AsmSaveStrings(dir))
	{
		throw std::runtime_error(std::string("Unable to save string data to \'") + m_region_check_strings_filename.str() + '\'');
	}
	if (!AsmSaveGraphics(dir))
	{
		throw std::runtime_error(std::string("Unable to save graphics data to \'") + directory.str() + '\'');
	}
	if (!AsmSaveInventoryGraphics(dir))
	{
		throw std::runtime_error(std::string("Unable to save inventory graphics data to \'") + m_inventory_graphics_filename.str() + '\'');
	}
	return true;
}

bool GraphicsData::Save()
{
	return Save(GetBasePath());
}

bool GraphicsData::HasBeenModified() const
{
	auto entry_pred = [](const auto& e) {return e != nullptr && e->HasDataChanged(); };
	auto pair_pred = [](const auto& e) {return e.second != nullptr && e.second->HasDataChanged(); };
	if (std::any_of(m_fonts_by_name.begin(), m_fonts_by_name.end(), pair_pred))
	{
		return true;
	}
	if (m_fonts_by_name != m_fonts_by_name_orig)
	{
		return true;
	}
	if (m_system_strings != m_system_strings_orig)
	{
		return true;
	}
	if (m_palettes_by_name_orig != m_palettes_by_name)
	{
		return true;
	}
	if (m_misc_gfx_by_name_orig != m_misc_gfx_by_name)
	{
		return true;
	}
	return false;
}

void GraphicsData::RefreshPendingWrites(const Rom& rom)
{
	DataManager::RefreshPendingWrites(rom);
	if (!RomPrepareInjectFonts(rom))
	{
		throw std::runtime_error(std::string("Unable to prepare fonts for ROM injection"));
	}
	if (!RomPrepareInjectInvGraphics(rom))
	{
		throw std::runtime_error(std::string("Unable to prepare inventory graphics for ROM injection"));
	}
}

std::map<std::string, std::shared_ptr<TilesetEntry>> GraphicsData::GetAllTilesets() const
{
	std::map<std::string, std::shared_ptr<TilesetEntry>> result;
	for (auto& t : m_fonts_by_name)
	{
		result.insert(t);
	}
	for (auto& t : m_misc_gfx_by_name)
	{
		result.insert(t);
	}
	return result;
}

std::vector<std::shared_ptr<TilesetEntry>> GraphicsData::GetFonts() const
{
	std::vector<std::shared_ptr<TilesetEntry>> retval;
	for (const auto& t : m_fonts_by_name)
	{
		retval.push_back(t.second);
	}
	return retval;
}

std::vector<std::shared_ptr<TilesetEntry>> GraphicsData::GetMiscGraphics() const
{
	std::vector<std::shared_ptr<TilesetEntry>> retval;
	for (const auto& t : m_misc_gfx_by_name)
	{
		retval.push_back(t.second);
	}
	return retval;
}

std::map<std::string, std::shared_ptr<PaletteEntry>> GraphicsData::GetAllPalettes() const
{
	std::map<std::string, std::shared_ptr<PaletteEntry>> result;
	for (auto& t : m_palettes_by_name)
	{
		result.insert(t);
	}
	return result;
}

void GraphicsData::CommitAllChanges()
{
	auto entry_commit = [](const auto& e) {return e->Commit(); };
	auto pair_commit = [](const auto& e) {return e.second->Commit(); };
	std::for_each(m_fonts_by_name.begin(), m_fonts_by_name.end(), pair_commit);
	m_fonts_by_name_orig = m_fonts_by_name;
	m_system_strings_orig = m_system_strings;
	m_palettes_by_name_orig = m_palettes_by_name;
	m_misc_gfx_by_name_orig = m_misc_gfx_by_name;
	m_pending_writes.clear();
}

bool GraphicsData::LoadAsmFilenames()
{
	try
	{
		bool retval = true;
		AsmFile f(GetAsmFilename().str());
		retval = retval && GetFilenameFromAsm(f, RomOffsets::Strings::REGION_CHECK, m_region_check_filename);
		retval = retval && GetFilenameFromAsm(f, RomOffsets::Graphics::INV_SECTION, m_inventory_graphics_filename);
		AsmFile r(GetBasePath() / m_region_check_filename);
		retval = retval && GetFilenameFromAsm(r, RomOffsets::Strings::REGION_CHECK_ROUTINE, m_region_check_routine_filename);
		retval = retval && GetFilenameFromAsm(r, RomOffsets::Strings::REGION_CHECK_STRINGS, m_region_check_strings_filename);
		retval = retval && GetFilenameFromAsm(r, RomOffsets::Graphics::SYS_FONT, m_system_font_filename);
		
		return retval;
	}
	catch (...)
	{
	}
	return false;
}

void GraphicsData::SetDefaultFilenames()
{
	if (m_region_check_filename.empty()) m_region_check_filename = RomOffsets::Strings::REGION_CHECK_FILE;
	if (m_region_check_routine_filename.empty()) m_region_check_routine_filename = RomOffsets::Strings::REGION_CHECK_ROUTINE_FILE;
	if (m_region_check_strings_filename.empty()) m_region_check_strings_filename = RomOffsets::Strings::REGION_CHECK_STRINGS_FILE;
	if (m_system_font_filename.empty()) m_system_font_filename = RomOffsets::Graphics::SYS_FONT_FILE;
	if (m_inventory_graphics_filename.empty()) m_inventory_graphics_filename = RomOffsets::Graphics::INV_GRAPHICS_FILE;
}

bool GraphicsData::CreateDirectoryStructure(const filesystem::path& dir)
{
	bool retval = true;
	retval = retval && CreateDirectoryTree(dir / m_region_check_filename);
	retval = retval && CreateDirectoryTree(dir / m_region_check_routine_filename);
	retval = retval && CreateDirectoryTree(dir / m_region_check_strings_filename);
	retval = retval && CreateDirectoryTree(dir / m_system_font_filename);
	retval = retval && CreateDirectoryTree(dir / m_inventory_graphics_filename);
	for (const auto& f : m_fonts_by_name)
	{
		retval = retval && CreateDirectoryTree(dir / f.second->GetFilename());
	}
	for (const auto& g : m_misc_gfx_by_name)
	{
		retval = retval && CreateDirectoryTree(dir / g.second->GetFilename());
	}
	for (const auto& p : m_palettes_by_name)
	{
		retval = retval && CreateDirectoryTree(dir / p.second->GetFilename());
	}
	return retval;
}

void GraphicsData::InitCache()
{
	m_palettes_by_name_orig = m_palettes_by_name;
	m_system_strings_orig = m_system_strings;
	m_fonts_by_name_orig = m_fonts_by_name;
	m_misc_gfx_by_name_orig = m_misc_gfx_by_name;
}

bool GraphicsData::AsmLoadFonts()
{
	filesystem::path path = GetBasePath() / m_system_font_filename;
	auto e = TilesetEntry::Create(this, ReadBytes(path), RomOffsets::Graphics::SYS_FONT, m_system_font_filename, false, 8, 8);
	m_fonts_by_name.insert({ RomOffsets::Graphics::SYS_FONT, e });
	m_fonts_internal.insert({ RomOffsets::Graphics::SYS_FONT, e });
	return true;
}

bool GraphicsData::AsmLoadStrings()
{
	try
	{
		AsmFile file(GetBasePath() / m_region_check_strings_filename);
		auto read_str = [&](std::string& s)
		{
			char c;
			do
			{
				file >> c;
				if (c != 0)
				{
					s += c;
				}
			} while (c != 0);
		};
		file.Goto(RomOffsets::Strings::REGION_ERROR_LINE1);
		read_str(m_system_strings[0]);
		file.Goto(RomOffsets::Strings::REGION_ERROR_NTSC);
		read_str(m_system_strings[1]);
		file.Goto(RomOffsets::Strings::REGION_ERROR_PAL);
		read_str(m_system_strings[2]);
		file.Goto(RomOffsets::Strings::REGION_ERROR_LINE3);
		read_str(m_system_strings[3]);
		return true;
	}
	catch (const std::exception&)
	{
	}
	return false;
}

bool GraphicsData::AsmLoadInventoryGraphics()
{
	try
	{
		AsmFile file(GetBasePath() / m_inventory_graphics_filename);
		AsmFile::Label name;
		AsmFile::IncludeFile inc;
		std::vector<std::tuple<std::string, filesystem::path, ByteVectorPtr>> gfx;
		for(int i = 0; i < 7; ++i)
		{
			file >> name >> inc;
			auto path = GetBasePath() / inc.path;
			auto raw_data = std::make_shared<std::vector<uint8_t>>(ReadBytes(path));
			gfx.push_back({ name, inc.path, raw_data });
		}
		auto menufont = TilesetEntry::Create(this, *std::get<2>(gfx[0]), std::get<0>(gfx[0]),
			std::get<1>(gfx[0]), false, 8, 8, 1);
		auto menucursor = TilesetEntry::Create(this, *std::get<2>(gfx[1]), std::get<0>(gfx[1]),
			std::get<1>(gfx[1]), false, 8, 8, 2, Tileset::BLOCK4X4);
		auto arrow = TilesetEntry::Create(this, *std::get<2>(gfx[2]), std::get<0>(gfx[2]),
			std::get<1>(gfx[2]), false, 8, 8, 2, Tileset::BLOCK2X2);
		auto unused1 = TilesetEntry::Create(this, *std::get<2>(gfx[3]), std::get<0>(gfx[3]),
			std::get<1>(gfx[3]), false, 8, 11, 2);
		auto unused2 = TilesetEntry::Create(this, *std::get<2>(gfx[4]), std::get<0>(gfx[4]),
			std::get<1>(gfx[4]), false, 8, 10, 2);
		auto pal1 = PaletteEntry::Create(this, *std::get<2>(gfx[5]), std::get<0>(gfx[5]),
			std::get<1>(gfx[5]), Palette::Type::LOW8);
		auto pal2 = PaletteEntry::Create(this, *std::get<2>(gfx[6]), std::get<0>(gfx[6]),
			std::get<1>(gfx[6]), Palette::Type::FULL);
		m_fonts_by_name.insert({ menufont->GetName(), menufont });
		m_misc_gfx_by_name.insert({ menucursor->GetName(), menucursor });
		m_misc_gfx_by_name.insert({ arrow->GetName(), arrow });
		m_misc_gfx_by_name.insert({ unused1->GetName(), unused1 });
		m_misc_gfx_by_name.insert({ unused2->GetName(), unused2 });
		m_palettes_by_name.insert({ pal1->GetName(), pal1 });
		m_palettes_by_name.insert({ pal2->GetName(), pal2 });
		m_fonts_internal.insert({ menufont->GetName(), menufont });
		m_misc_gfx_internal.insert({ menucursor->GetName(), menucursor });
		m_misc_gfx_internal.insert({ arrow->GetName(), arrow });
		m_misc_gfx_internal.insert({ unused1->GetName(), unused1 });
		m_misc_gfx_internal.insert({ unused2->GetName(), unused2 });
		m_palettes_internal.insert({ pal1->GetName(), pal1 });
		m_palettes_internal.insert({ pal2->GetName(), pal2 });
		return true;
	}
	catch (const std::exception&)
	{
	}
	return false;
}

bool GraphicsData::RomLoadFonts(const Rom& rom)
{
	uint32_t sys_font_lea = rom.read<uint32_t>(RomOffsets::Graphics::SYS_FONT);
	uint32_t sys_font_begin = Disasm::LEA_PCRel(sys_font_lea, rom.get_address(RomOffsets::Graphics::SYS_FONT));
	uint32_t sys_font_size = rom.get_address(RomOffsets::Graphics::SYS_FONT_SIZE);
	auto sys_font_bytes = rom.read_array<uint8_t>(sys_font_begin, sys_font_size);
	auto sys_font = TilesetEntry::Create(this, sys_font_bytes, RomOffsets::Graphics::SYS_FONT,
		RomOffsets::Graphics::SYS_FONT_FILE, false);
	sys_font->SetStartAddress(sys_font_begin);
	m_fonts_by_name.insert({ sys_font->GetName(), sys_font });
	m_fonts_internal.insert({ sys_font->GetName(), sys_font });
	return true;
}

bool GraphicsData::RomLoadStrings(const Rom& rom)
{
	uint32_t line1_lea = rom.read<uint32_t>(RomOffsets::Strings::REGION_ERROR_LINE1);
	uint32_t line1_begin = Disasm::LEA_PCRel(line1_lea, rom.get_address(RomOffsets::Strings::REGION_ERROR_LINE1));
	m_system_strings[0] = rom.read_string(line1_begin);
	uint32_t line2_lea = rom.read<uint32_t>(RomOffsets::Strings::REGION_ERROR_NTSC);
	uint32_t line2_begin = Disasm::LEA_PCRel(line2_lea, rom.get_address(RomOffsets::Strings::REGION_ERROR_NTSC));
	m_system_strings[1] = rom.read_string(line2_begin);
	uint32_t line3_lea = rom.read<uint32_t>(RomOffsets::Strings::REGION_ERROR_PAL);
	uint32_t line3_begin = Disasm::LEA_PCRel(line3_lea, rom.get_address(RomOffsets::Strings::REGION_ERROR_PAL));
	m_system_strings[2] = rom.read_string(line3_begin);
	uint32_t line4_lea = rom.read<uint32_t>(RomOffsets::Strings::REGION_ERROR_LINE3);
	uint32_t line4_begin = Disasm::LEA_PCRel(line4_lea, rom.get_address(RomOffsets::Strings::REGION_ERROR_LINE3));
	m_system_strings[3] = rom.read_string(line4_begin);
	return true;
}

bool GraphicsData::RomLoadInventoryGraphics(const Rom& rom)
{
	auto load_bytes = [&](const std::string& lea_loc, const std::string& size)
	{
		uint32_t pc = rom.get_address(lea_loc);
		uint32_t lea = rom.read<uint32_t>(pc);
		uint32_t start = Disasm::LEA_PCRel(lea, pc);
		uint32_t sz = rom.get_address(size);
		return rom.read_array<uint8_t>(start, sz);
	};
	auto load_pal = [&](const std::string& lea_loc, Palette::Type type)
	{
		uint32_t pc = rom.get_address(lea_loc);
		uint32_t lea = rom.read<uint32_t>(pc);
		uint32_t start = Disasm::LEA_PCRel(lea, pc);
		uint32_t sz = Palette::GetSizeBytes(type);
		return rom.read_array<uint8_t>(start, sz);
	};
	auto menu_font = TilesetEntry::Create(this, load_bytes(RomOffsets::Graphics::INV_FONT, RomOffsets::Graphics::INV_FONT_SIZE),
		RomOffsets::Graphics::INV_FONT, RomOffsets::Graphics::INV_FONT_FILE, false, 8, 8, 1);
	auto cursor = TilesetEntry::Create(this, load_bytes(RomOffsets::Graphics::INV_CURSOR, RomOffsets::Graphics::INV_CURSOR_SIZE),
		RomOffsets::Graphics::INV_CURSOR, RomOffsets::Graphics::INV_CURSOR_FILE, false, 8, 8, 2, Tileset::BLOCK4X4);
	auto arrow = TilesetEntry::Create(this, load_bytes(RomOffsets::Graphics::INV_ARROW, RomOffsets::Graphics::INV_ARROW_SIZE),
		RomOffsets::Graphics::INV_ARROW, RomOffsets::Graphics::INV_ARROW_FILE, false, 8, 8, 2, Tileset::BLOCK2X2);
	auto unused1 = TilesetEntry::Create(this, load_bytes(RomOffsets::Graphics::INV_UNUSED1, RomOffsets::Graphics::INV_UNUSED1_SIZE),
		RomOffsets::Graphics::INV_UNUSED1, RomOffsets::Graphics::INV_UNUSED1_FILE, false, 8, 11, 2);
	auto unused2 = TilesetEntry::Create(this, load_bytes(RomOffsets::Graphics::INV_UNUSED2, RomOffsets::Graphics::INV_UNUSED2_SIZE),
		RomOffsets::Graphics::INV_UNUSED2, RomOffsets::Graphics::INV_UNUSED2_FILE, false, 8, 10, 2);
	auto pal1 = PaletteEntry::Create(this, load_pal(RomOffsets::Graphics::INV_PAL1, Palette::Type::LOW8),
		RomOffsets::Graphics::INV_PAL1, RomOffsets::Graphics::INV_PAL1_FILE, Palette::Type::LOW8);
	auto pal2 = PaletteEntry::Create(this, load_pal(RomOffsets::Graphics::INV_PAL2, Palette::Type::FULL),
		RomOffsets::Graphics::INV_PAL2, RomOffsets::Graphics::INV_PAL2_FILE, Palette::Type::FULL);
	m_fonts_by_name.insert({ menu_font->GetName(), menu_font });
	m_fonts_internal.insert({ RomOffsets::Graphics::INV_FONT, menu_font });
	m_misc_gfx_by_name.insert({ cursor->GetName(), cursor });
	m_misc_gfx_internal.insert({ RomOffsets::Graphics::INV_CURSOR, cursor });
	m_misc_gfx_by_name.insert({ arrow->GetName(), arrow });
	m_misc_gfx_internal.insert({ RomOffsets::Graphics::INV_ARROW, arrow });
	m_misc_gfx_by_name.insert({ unused1->GetName(), unused1 });
	m_misc_gfx_internal.insert({ RomOffsets::Graphics::INV_UNUSED1, unused1 });
	m_misc_gfx_by_name.insert({ unused2->GetName(), unused2 });
	m_misc_gfx_internal.insert({ RomOffsets::Graphics::INV_UNUSED2, unused2 });
	m_palettes_by_name.insert({ pal1->GetName(), pal1 });
	m_palettes_internal.insert({ RomOffsets::Graphics::INV_PAL1, pal1 });
	m_palettes_by_name.insert({ pal2->GetName(), pal2 });
	m_palettes_internal.insert({ RomOffsets::Graphics::INV_PAL2, pal2 });
	return true;
}

bool GraphicsData::AsmSaveGraphics(const filesystem::path& dir)
{
	bool retval = std::all_of(m_fonts_by_name.begin(), m_fonts_by_name.end(), [&](auto& f)
		{
			return f.second->Save(dir);
		});
	retval = retval && std::all_of(m_misc_gfx_by_name.begin(), m_misc_gfx_by_name.end(), [&](auto& f)
		{
			return f.second->Save(dir);
		});
	retval = retval && std::all_of(m_palettes_by_name.begin(), m_palettes_by_name.end(), [&](auto& f)
		{
			return f.second->Save(dir);
		});
	return retval;
}

bool GraphicsData::AsmSaveStrings(const filesystem::path& dir)
{
	try
	{
		AsmFile sfile, ifile;
		ByteVector str[4];
		ifile.WriteFileHeader(m_region_check_filename, "Region Check");
		ifile << AsmFile::Label(RomOffsets::Strings::REGION_CHECK_ROUTINE) << AsmFile::IncludeFile(m_region_check_routine_filename, AsmFile::FileType::ASSEMBLER);
		ifile << AsmFile::Label(RomOffsets::Strings::REGION_CHECK_STRINGS) << AsmFile::IncludeFile(m_region_check_strings_filename, AsmFile::FileType::ASSEMBLER);
		ifile << AsmFile::Align(2) << AsmFile::Label(RomOffsets::Graphics::SYS_FONT) << AsmFile::IncludeFile(m_system_font_filename, AsmFile::FileType::BINARY);
		ifile.WriteFile(dir / m_region_check_filename);
		sfile.WriteFileHeader(m_region_check_strings_filename, "Region Check System Strings");
		for (int i = 0; i < 4; ++i)
		{
			str[i].insert(str[i].end(), m_system_strings[i].cbegin(), m_system_strings[i].cend());
			str[i].push_back(0);
		}
		sfile << AsmFile::Label(RomOffsets::Strings::REGION_ERROR_LINE1) << str[0];
		sfile << AsmFile::Label(RomOffsets::Strings::REGION_ERROR_NTSC) << str[1];
		sfile << AsmFile::Label(RomOffsets::Strings::REGION_ERROR_PAL) << str[2];
		sfile << AsmFile::Label(RomOffsets::Strings::REGION_ERROR_LINE3) << str[3];
		sfile.WriteFile(dir / m_region_check_strings_filename);
		return true;
	}
	catch (const std::exception&)
	{
	}
	return false;
}

bool GraphicsData::AsmSaveInventoryGraphics(const filesystem::path& dir)
{

	try
	{
		AsmFile ifile;
		auto write_inc = [&](const std::string& name, const auto& container)
		{
			auto e = container.find(name)->second;
			ifile << AsmFile::Label(name) << AsmFile::IncludeFile(e->GetFilename(), AsmFile::FileType::BINARY);
		};
		ifile.WriteFileHeader(m_inventory_graphics_filename, "Inventory Graphics Data");
		write_inc(RomOffsets::Graphics::INV_FONT, m_fonts_internal);
		write_inc(RomOffsets::Graphics::INV_CURSOR, m_misc_gfx_internal);
		write_inc(RomOffsets::Graphics::INV_ARROW, m_misc_gfx_internal);
		write_inc(RomOffsets::Graphics::INV_UNUSED1, m_misc_gfx_internal);
		write_inc(RomOffsets::Graphics::INV_UNUSED2, m_misc_gfx_internal);
		write_inc(RomOffsets::Graphics::INV_PAL1, m_palettes_internal);
		write_inc(RomOffsets::Graphics::INV_PAL2, m_palettes_internal);
		ifile.WriteFile(dir / m_inventory_graphics_filename);
		return true;
	}
	catch (const std::exception&)
	{
	}
	return false;
}

bool GraphicsData::RomPrepareInjectFonts(const Rom& rom)
{
	auto system_font_bytes = m_fonts_by_name[RomOffsets::Graphics::SYS_FONT]->GetBytes();
	ByteVectorPtr bytes = std::make_shared<ByteVector>();
	auto write_string = [](ByteVectorPtr& bytes, const std::string& in)
	{
		for (char c : in)
		{
			bytes->push_back(c);
		}
		bytes->push_back(0);
	};
	uint32_t addrs[5];
	uint32_t begin = rom.get_section(RomOffsets::Strings::REGION_CHECK_DATA_SECTION).begin;
	for (int i = 0; i < 4; ++i)
	{
		addrs[i] = begin + bytes->size();
		write_string(bytes, m_system_strings[i]);
	}
	if ((bytes->size() & 1) == 1)
	{
		bytes->push_back(0xFF);
	}
	addrs[4] = begin + bytes->size();
	bytes->insert(bytes->end(), system_font_bytes->cbegin(), system_font_bytes->cend());
	m_pending_writes.push_back({ RomOffsets::Strings::REGION_CHECK_DATA_SECTION, bytes });
	uint32_t line1_lea = Asm::LEA_PCRel(AReg::A0, rom.get_address(RomOffsets::Strings::REGION_ERROR_LINE1), addrs[0]);
	uint32_t line2_lea = Asm::LEA_PCRel(AReg::A0, rom.get_address(RomOffsets::Strings::REGION_ERROR_NTSC), addrs[1]);
	uint32_t line3_lea = Asm::LEA_PCRel(AReg::A0, rom.get_address(RomOffsets::Strings::REGION_ERROR_PAL), addrs[2]);
	uint32_t line4_lea = Asm::LEA_PCRel(AReg::A0, rom.get_address(RomOffsets::Strings::REGION_ERROR_LINE3), addrs[3]);
	uint32_t font_lea = Asm::LEA_PCRel(AReg::A0, rom.get_address(RomOffsets::Graphics::SYS_FONT), addrs[4]);

	m_pending_writes.push_back({ RomOffsets::Strings::REGION_ERROR_LINE1, std::make_shared<std::vector<uint8_t>>(Split<uint8_t>(line1_lea)) });
	m_pending_writes.push_back({ RomOffsets::Strings::REGION_ERROR_NTSC, std::make_shared<std::vector<uint8_t>>(Split<uint8_t>(line2_lea)) });
	m_pending_writes.push_back({ RomOffsets::Strings::REGION_ERROR_PAL, std::make_shared<std::vector<uint8_t>>(Split<uint8_t>(line3_lea)) });
	m_pending_writes.push_back({ RomOffsets::Strings::REGION_ERROR_LINE3, std::make_shared<std::vector<uint8_t>>(Split<uint8_t>(line4_lea)) });
	m_pending_writes.push_back({ RomOffsets::Graphics::SYS_FONT, std::make_shared<std::vector<uint8_t>>(Split<uint8_t>(font_lea)) });

	return true;
}

bool GraphicsData::RomPrepareInjectInvGraphics(const Rom& rom)
{
	ByteVectorPtr bytes = std::make_shared<ByteVector>();
	uint32_t base = rom.get_section(RomOffsets::Graphics::INV_SECTION).begin;
	uint32_t addrs[7];
	addrs[0] = base + bytes->size();
	auto inv_font = m_fonts_internal[RomOffsets::Graphics::INV_FONT]->GetBytes();
	bytes->insert(bytes->end(), inv_font->cbegin(), inv_font->cend());
	addrs[1] = base + bytes->size();
	auto inv_cursor = m_misc_gfx_internal[RomOffsets::Graphics::INV_CURSOR]->GetBytes();
	bytes->insert(bytes->end(), inv_cursor->cbegin(), inv_cursor->cend());
	addrs[2] = base + bytes->size();
	auto inv_arrow = m_misc_gfx_internal[RomOffsets::Graphics::INV_ARROW]->GetBytes();
	bytes->insert(bytes->end(), inv_arrow->cbegin(), inv_arrow->cend());
	addrs[3] = base + bytes->size();
	auto inv_unused1 = m_misc_gfx_internal[RomOffsets::Graphics::INV_UNUSED1]->GetBytes();
	bytes->insert(bytes->end(), inv_unused1->cbegin(), inv_unused1->cend());
	addrs[4] = base + bytes->size();
	auto inv_unused2 = m_misc_gfx_internal[RomOffsets::Graphics::INV_UNUSED2]->GetBytes();
	bytes->insert(bytes->end(), inv_unused2->cbegin(), inv_unused2->cend());
	addrs[5] = base + bytes->size();
	auto inv_pal1 = m_palettes_internal[RomOffsets::Graphics::INV_PAL1]->GetBytes();
	bytes->insert(bytes->end(), inv_pal1->cbegin(), inv_pal1->cend());
	addrs[6] = base + bytes->size();
	auto inv_pal2 = m_palettes_internal[RomOffsets::Graphics::INV_PAL2]->GetBytes();
	bytes->insert(bytes->end(), inv_pal2->cbegin(), inv_pal2->cend());

	m_pending_writes.push_back({ RomOffsets::Graphics::INV_SECTION, bytes });

	uint32_t font_lea = Asm::LEA_PCRel(AReg::A1, rom.get_address(RomOffsets::Graphics::INV_FONT), addrs[0]);
	uint32_t cursor_lea = Asm::LEA_PCRel(AReg::A1, rom.get_address(RomOffsets::Graphics::INV_CURSOR), addrs[1]);
	uint32_t arrow_lea = Asm::LEA_PCRel(AReg::A1, rom.get_address(RomOffsets::Graphics::INV_ARROW), addrs[2]);
	uint32_t unused1_lea = Asm::LEA_PCRel(AReg::A1, rom.get_address(RomOffsets::Graphics::INV_UNUSED1), addrs[3]);
	uint32_t unused1p6_lea = Asm::LEA_PCRel(AReg::A1, rom.get_address(RomOffsets::Graphics::INV_UNUSED1_PLUS6), addrs[3] + 12);
	uint32_t unused2_lea = Asm::LEA_PCRel(AReg::A1, rom.get_address(RomOffsets::Graphics::INV_UNUSED2), addrs[4]);
	uint32_t unused2p4_lea = Asm::LEA_PCRel(AReg::A1, rom.get_address(RomOffsets::Graphics::INV_UNUSED2_PLUS4), addrs[4] + 8);
	uint32_t pal1_lea = Asm::LEA_PCRel(AReg::A0, rom.get_address(RomOffsets::Graphics::INV_PAL1), addrs[5]);
	uint32_t pal2_lea = Asm::LEA_PCRel(AReg::A0, rom.get_address(RomOffsets::Graphics::INV_PAL2), addrs[6]);

	m_pending_writes.push_back({ RomOffsets::Graphics::INV_FONT, std::make_shared<std::vector<uint8_t>>(Split<uint8_t>(font_lea)) });
	m_pending_writes.push_back({ RomOffsets::Graphics::INV_CURSOR, std::make_shared<std::vector<uint8_t>>(Split<uint8_t>(cursor_lea)) });
	m_pending_writes.push_back({ RomOffsets::Graphics::INV_ARROW, std::make_shared<std::vector<uint8_t>>(Split<uint8_t>(arrow_lea)) });
	m_pending_writes.push_back({ RomOffsets::Graphics::INV_UNUSED1, std::make_shared<std::vector<uint8_t>>(Split<uint8_t>(unused1_lea)) });
	m_pending_writes.push_back({ RomOffsets::Graphics::INV_UNUSED1_PLUS6, std::make_shared<std::vector<uint8_t>>(Split<uint8_t>(unused1p6_lea)) });
	m_pending_writes.push_back({ RomOffsets::Graphics::INV_UNUSED2, std::make_shared<std::vector<uint8_t>>(Split<uint8_t>(unused2_lea)) });
	m_pending_writes.push_back({ RomOffsets::Graphics::INV_UNUSED2_PLUS4, std::make_shared<std::vector<uint8_t>>(Split<uint8_t>(unused2p4_lea)) });
	m_pending_writes.push_back({ RomOffsets::Graphics::INV_PAL1, std::make_shared<std::vector<uint8_t>>(Split<uint8_t>(pal1_lea)) });
	m_pending_writes.push_back({ RomOffsets::Graphics::INV_PAL2, std::make_shared<std::vector<uint8_t>>(Split<uint8_t>(pal2_lea)) });

	return true;
}

void GraphicsData::UpdateTilesetRecommendedPalettes()
{
	std::vector<std::string> palettes;
	for (const auto& p : GetAllPalettes())
	{
		palettes.push_back(p.first);
	}
	auto set_pals = [&](auto& container)
	{
		for (auto& e : container)
		{
			e.second->SetAllPalettes(palettes);
			e.second->SetRecommendedPalettes(palettes);
		}
	};
	set_pals(m_fonts_by_name);
	set_pals(m_misc_gfx_by_name);
}

void GraphicsData::ResetTilesetDefaultPalettes()
{
	auto set_pals = [&](auto& container)
	{
		for (const auto& e : container)
		{
			if (e.second->GetRecommendedPalettes().size() == 0)
			{
				e.second->SetDefaultPalette(m_palettes_by_name.begin()->second->GetName());
			}
			else
			{
				e.second->SetDefaultPalette(e.second->GetRecommendedPalettes().front());
			}
		}
	};
	set_pals(m_fonts_by_name);
	set_pals(m_misc_gfx_by_name);
}
