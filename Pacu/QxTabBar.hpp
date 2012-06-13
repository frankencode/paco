#ifndef PACU_QXTABBAR_HPP
#define PACU_QXTABBAR_HPP

#include <QUrl>
#include "UseVide.hpp"

namespace pacu
{

class QxTabBar: public QxControl
{
	Q_OBJECT
	
public:
	QxTabBar(QWidget* parent = 0);
	
	void openTab(int index, QString label, QPixmap icon, QWidget* widget = 0);
	void closeTab(int index, bool closeWidget = true);
	int tabCount() const;
	
	void activate(int index);
	int indexActive() const;
	
	QString label(int index) const;
	void setLabel(int index, QString text);
	
	QPixmap icon(int index) const;
	void setIcon(int index, QPixmap icon);
	
	QWidget* widget(int index) const;
	void setWidget(int index, QWidget* widget);
	
	QString semanticType() const;
	void setSemanticType(QString name);
	
	bool acceptUriDrops() const;
	void setAcceptUriDrops(bool on);
	
	QMenu* contextMenu() const;
	void setContextMenu(QMenu* menu);
	
	QSize minimumSizeHint() const;
	QSize sizeHint() const;
	
signals:
	void addWidget(QWidget* widget);
	void removeWidget(QWidget* widget);
	void changeActiveWidget(QWidget* old, QWidget* now);
	
	void openRequest(int index, QUrl url);
	void closeRequest(int index);
	void lastTabClosed();
	
public slots:
	void dragTabLeft();
	void dragTabRight();
	void dragTab(int distance);
	void dropTab();
	
private slots:
	void closeTab();
	void moreTab();
	void activate(QAction* action);
	void stepLeft();
	void stepRight();
	void showContextMenu(QPoint pos);
	
private:
	QString internalMimeFormat() const;
	
	bool keepInView();
	bool keepInView(int tabIndex);
	void moveTab(int dragIndex, int dropIndex);
	void updateStyleHints();
	void layoutWidgets();
	
	int indexByPos(QPoint pos) const;
	
	virtual void resizeEvent(QResizeEvent* event);
	virtual void mousePressEvent(QMouseEvent* event);
	virtual void mouseReleaseEvent(QMouseEvent* event);
	virtual void mouseMoveEvent(QMouseEvent* event);
	void dragMove(QPoint pos);
	virtual void dragEnterEvent(QDragEnterEvent* event);
	virtual void leaveEvent(QEvent* event);
	virtual void dragMoveEvent(QDragMoveEvent* event);
	virtual void dropEvent(QDropEvent* event);
	
	virtual bool eventFilter(QObject* proxy, QEvent* event);
	
	Ref<QxStyle> style_;
	Ref<QxStyle> tabStyle_;
	int gap_;
	int tabMinWidth_;
	
	QList< QPointer<QxControl> > tabs_;
	QList< QPointer<QWidget> > widgets_;
	QMap<QAction*, int> actionToIndex_;
	
	QString semanticType_;
	bool acceptUriDrops_;
	QPointer<QMenu> contextMenu_;
	
	QxControl* carrier_;
	QxControl* closeButton_;
	QxControl* moreButton_;
	QxControl* dropMark_;
	QxControl* filler_;
	QxControl* scrollLeft_;
	QxControl* scrollRight_;
	
	int indexActive_;
	Qt::LayoutDirection layoutDirection_;
	int indexAlpha_;
	int indexBeta_;
	
	QPoint dragStartPos_;
	int dragIndex_;
	int dropIndex_;
};

} // namespace pacu

#endif // PACU_QXTABBAR_HPP
