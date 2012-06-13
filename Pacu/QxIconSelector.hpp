#ifndef PACU_QXICONSELECTOR_HPP
#define PACU_QXICONSELECTOR_HPP

#include "QxDialog.hpp"

class QToolButton;
class QListWidget;
class QListWidgetItem;

namespace pacu
{

class QxIconSelector: public QxDialog
{
	Q_OBJECT
	
public:
	QxIconSelector(QWidget* parent);
	QIcon icon() const;
	
private slots:
	void iconFromFile();
	void itemSelected(QListWidgetItem* item);
	
private:
	void loadBuiltinIcons();
	
	QToolButton* iconButton_;
	QListWidget* iconList_;
};

} // namespace pacu

#endif // PACU_QXICONSELECTOR_HPP
