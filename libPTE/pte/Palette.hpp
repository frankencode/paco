#ifndef PTE_PALETTE_HPP
#define PTE_PALETTE_HPP

#include <QColor>
#include <ftl/Map.hpp>
#include <ftl/Array.hpp>
#include "UseFtl.hpp"

namespace pte
{

class CharcoalDirectory;
class PaletteManager;

class Palette: public Instance
{
public:
	Palette(Ref<PaletteManager> manager, QString name, QString displayName, int numColors);
	
	QString name() const;
	QString displayName() const;
	
	QColor colorByName(const QString& name) const;
	QColor colorByIndex(int index) const;
	int numColors() const;
	
private:
	friend class CharcoalDirectory;
	
	QString name_;
	QString displayName_;
	
	typedef Map<QString, QColor> ColorByName;
	typedef Array<QColor> ColorByIndex;
	Ref<ColorByName, Owner> colorByName_;
	Ref<ColorByIndex, Owner> colorByIndex_;
};

} // namespace pte

#endif // PTE_PALETTE_HPP
