#ifndef PACU_QXFILESYSTEMVIEW_HPP
#define PACU_QXFILESYSTEMVIEW_HPP

#include <QTreeView>
#include <QKeyEvent>
#include <QResizeEvent>

namespace pacu
{

class QxFileSystemView: public QTreeView
{
	Q_OBJECT
	
public:
	QxFileSystemView(QWidget* parent = 0);
	
	void ignoreNextFocusOutEvent();
	
signals:
	void delKeyPressed();
	void escape();
	void cdUp();

public slots:
	void resizeColumnsToContents();
	
private:
	virtual void keyPressEvent(QKeyEvent* event);
	// virtual void resizeEvent(QResizeEvent* event);
	/*virtual void focusInEvent(QFocusEvent* event);
	virtual void focusOutEvent(QFocusEvent* event);*/
	virtual void contextMenuEvent(QContextMenuEvent* event);
	#ifdef QT_MAC_USE_COCOA
	virtual void mouseMoveEvent(QMouseEvent* event);
	#endif
	
	QModelIndex currentIndexSaved_;
	bool ignoreNextFocusOutEvent_;
	// virtual void rowsInserted(const QModelIndex & parent, int start, int end);
	// virtual void keyboardSearch(const QString& search);
	// virtual void dataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);
};

} // namespace pacu

#endif // PACU_QXFILESYSTEMVIEW_HPP
