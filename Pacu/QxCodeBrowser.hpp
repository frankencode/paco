#ifndef PACU_QXCODEBROWSER_HPP
#define PACU_QXCODEBROWSER_HPP

#include "UseVide.hpp"
#include "UsePte.hpp"
#include <pte/Document.hpp> // to please moc

class QStackedLayout;
class QTreeView;
class QModelIndex;

namespace pacu
{

class QxStatusBar;

class QxCodeBrowser: public QWidget
{
	Q_OBJECT
	
public:
	QxCodeBrowser(QWidget* parent);
	
public slots:
	void display(Ref<Document> document, bool reload = true);
	void refresh(bool reload = false);
	void reload();
	
signals:
	void gotoLine(int y);
	
private slots:
	void gotoEntity(const QModelIndex& index);
	
private:
	QTreeView* newInsightView();
	
	QxControl* panelHead_;
	QxControl* reloadButton_;
	QStackedLayout* insightStack_;
	QxStatusBar* statusBar_;
	QTreeView* noInsightView_;
};

} // namespace pacu

#endif // PACU_QXCODEBROWSER_HPP
