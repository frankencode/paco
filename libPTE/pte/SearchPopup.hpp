#ifndef PTE_SEARCHPOPUP_HPP
#define PTE_SEARCHPOPUP_HPP

#include <QFrame>
#include <QPointer>
#include <QTime>

class QListView;
class QModelIndex;
class QAbstractItemModel;

namespace pte
{

class SearchPopup: public QFrame
{
	Q_OBJECT
	
public:
	SearchPopup(QWidget* widget, QPoint pos, QAbstractItemModel* model, QWidget* quickInfo = 0, QString pattern = "");
	
	QWidget* quickInfo() const;
	
signals:
	void currentChanged(const QModelIndex& index);
	void searchPatternChanged(const QString& pattern);
	void accepted();
	void rejected();
	
public slots:
	void selectItem(int index = -1);
	void selectItemByPattern(const QString& pattern);
	
private slots:
	void commit(const QModelIndex& index);
	void currentChanged(const QModelIndex& current, const QModelIndex& previous);
	void updateInputContext(bool commit = false);
	
private:
	void commit();
	int selectedItem() const;
	bool eventFilter(QObject *obj, QEvent *event);
	virtual void keyPressEvent(QKeyEvent* event);
	virtual void closeEvent(QCloseEvent* event);
	
	QPointer<QWidget> widget_;
	QPointer<QWidget> window_;
	QListView* listView_;
	QWidget* quickInfo_;
	QPoint origMousePos_;
	QString preeditString_;
	int preeditCursor_;
	bool commited_;
};

} // namespace pte

#endif // PTE_SEARCHPOPUP_HPP
