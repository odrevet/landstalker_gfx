#ifndef _TILE_SWAP_DATA_VIEW_MODEL_H_
#define _TILE_SWAP_DATA_VIEW_MODEL_H_

#include <cstdint>
#include <memory>
#include <map>
#include <wx/dataview.h>
#include <GameData.h>
#include <TileSwaps.h>

class TileSwapDataViewModel : public wxDataViewVirtualListModel
{
public:
    TileSwapDataViewModel(uint16_t roomnum, std::shared_ptr<GameData> gd);

    virtual void Initialise();

    virtual void CommitData();

    virtual unsigned int GetColumnCount() const override;

    virtual unsigned int GetRowCount() const;

    virtual wxString GetColumnHeader(unsigned int col) const;

    virtual wxArrayString GetColumnChoices(unsigned int col) const;

    virtual wxString GetColumnType(unsigned int col) const override;

    virtual void GetValueByRow(wxVariant& variant, unsigned int row, unsigned int col) const override;

    virtual bool GetAttrByRow(unsigned int row, unsigned int col, wxDataViewItemAttr& attr) const override;

    virtual bool SetValueByRow(const wxVariant& variant, unsigned int row, unsigned int col) override;

    virtual bool DeleteRow(unsigned int row);

    virtual bool AddRow(unsigned int row);

    virtual bool SwapRows(unsigned int r1, unsigned int r2);
private:
    std::vector<TileSwap> m_swaps;
    uint16_t m_roomnum;
    std::shared_ptr<GameData> m_gd;
};

#endif // _TILE_SWAP_DATA_VIEW_MODEL_H_
