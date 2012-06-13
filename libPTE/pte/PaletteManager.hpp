#ifndef PTE_PALETTEMANAGER_HPP
#define PTE_PALETTEMANAGER_HPP

#include <ftl/Map.hpp>
#include <ftl/List.hpp>
#include "UseFtl.hpp"
#include "Palette.hpp"

namespace pte
{

class CharcoalDirectory;

class PaletteManager: public Instance
{
public:
	PaletteManager();
	
	Ref<Palette> paletteByName(const QString& name) const;
	Ref<Palette> paletteByIndex(int index) const;
	int numPalette() const;
	
private:
	friend class CharcoalDirectory;
	friend class Palette;
	
	void addPalette(Ref<Palette> palette);
	void sortPalettesByDisplayName();
	
	typedef Map<QString, Ref<Palette, Owner> > PaletteByName;
	typedef List< Ref<Palette, Owner> > PaletteByIndex;
	Ref<PaletteByName, Owner> paletteByName_;
	Ref<PaletteByIndex, Owner> paletteByIndex_;
};

} // namespace pte

#endif // PTE_PALETTEMANAGER_HPP
