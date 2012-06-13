#include "PaletteManager.hpp"

namespace pte
{

PaletteManager::PaletteManager()
	: paletteByName_(new PaletteByName),
	  paletteByIndex_(new PaletteByIndex)
{}

Ref<Palette> PaletteManager::paletteByName(const QString& name) const
{
	Ref<Palette> palette = paletteByName_->value(name);
	if (!palette) palette = paletteByName_->value("Default");
	return palette;
}

Ref<Palette> PaletteManager::paletteByIndex(int index) const
{
	return paletteByIndex_->at(index);
}

int PaletteManager::numPalette() const
{
	return paletteByIndex_->length();
}

void PaletteManager::addPalette(Ref<Palette> palette)
{
	paletteByName_->insert(palette->name(), palette);
	paletteByIndex_->append(palette);
}

void PaletteManager::sortPalettesByDisplayName()
{
	typedef PaletteByName PaletteByDisplayName;
	Ref<PaletteByDisplayName, Owner> paletteByDisplayName = new PaletteByDisplayName;
	for (int i = 0; paletteByIndex_->has(i); ++i) {
		Ref<Palette> palette = paletteByIndex_->at(i);
		paletteByDisplayName->insert(palette->displayName(), palette);
	}
	paletteByIndex_ = new PaletteByIndex;
	for (int i = 0; i < paletteByDisplayName->length(); ++i)
		paletteByIndex_->append(paletteByDisplayName->get(i).value());
}

} // namespace pte
