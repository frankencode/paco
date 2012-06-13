#include <QWidget>
#include <pte/CharcoalDirectory.hpp>
#include <pte/ThemeManager.hpp>
#include "QxThemesModel.hpp"

namespace pacu
{

QxThemesModel::QxThemesModel(Ref<CharcoalDirectory> charcoalDirectory, QWidget* themesView)
	: QAbstractListModel(themesView),
	  charcoalDirectory_(charcoalDirectory),
	  themeManager_(charcoalDirectory->themeManager()),
	  previewCache_(new PreviewCache(themeManager_->numThemes())),
	  themesView_(themesView)
{}

int QxThemesModel::rowCount(const QModelIndex& parent) const
{
	return themeManager_->numThemes();
}

QVariant QxThemesModel::data(const QModelIndex& index, int role) const
{
	QVariant value;
	int themeIndex = index.row();
	if (role == Qt::DisplayRole) {
		value = themeManager_->themeByIndex(themeIndex)->displayName();
	}
	else if (role == Qt::DecorationRole) {
		if (previewCache_->get(themeIndex).isNull())
			previewCache_->set(themeIndex, charcoalDirectory_->themePreview(themeManager_->themeByIndex(themeIndex)));
		value = previewCache_->get(themeIndex);
	}
	return value;
}

} // namespace pacu
