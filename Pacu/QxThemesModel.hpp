#ifndef PACU_QXTHEMESMODEL_HPP
#define PACU_QXTHEMESMODEL_HPP

#include <QAbstractListModel>
#include <ftl/Array.hpp>
#include "UsePte.hpp"

namespace pte { class CharcoalDirectory; }
class QWidget;

namespace pacu
{

class QxThemesModel: public QAbstractListModel
{
public:
	QxThemesModel(Ref<CharcoalDirectory> charcoalDirectory, QWidget* themesView);
	
private:
	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
	
	Ref<CharcoalDirectory, Owner> charcoalDirectory_;
	Ref<ThemeManager, Owner> themeManager_;
	typedef Array<QPixmap> PreviewCache;
	Ref<PreviewCache, Owner> previewCache_;
	QWidget* themesView_;
};

} // namespace pacu

#endif // PACU_QXTHEMESMODEL_HPP
