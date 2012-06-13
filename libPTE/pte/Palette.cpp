#include "PaletteManager.hpp"
#include "Palette.hpp"

namespace pte
{

Palette::Palette(Ref<PaletteManager> manager, QString name, QString displayName, int numColors)
	: name_(name),
	  displayName_(displayName),
	  colorByName_(new ColorByName),
	  colorByIndex_(new ColorByIndex(numColors))
{
	manager->addPalette(this);
}

QString Palette::name() const { return name_; }
QString Palette::displayName() const { return displayName_; }

QColor Palette::colorByName(const QString& name) const { return colorByName_->value(name); }
QColor Palette::colorByIndex(int index) const { return colorByIndex_->get(index); }
int Palette::numColors() const { return colorByIndex_->size(); }

} // namespace pte
