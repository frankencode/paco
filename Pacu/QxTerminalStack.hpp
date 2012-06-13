#ifndef PACU_QXVIDEOTERMINALMANAGER_HPP
#define PACU_QXVIDEOTERMINALMANAGER_HPP

#include <QObject>
#include <QPointer>
#include <pte/Atoms.hpp> // Link
#include "UsePte.hpp"
#include "QxTabWidget.hpp"

class QAction;
class QMenu;
class QWidget;
class QStackedLayout;

namespace pacu
{

class QxPreferences;
class QxVideoTerminal;

class QxTerminalStack: public QxTabWidget
{
	Q_OBJECT
	
public:
	QxTerminalStack(QxPreferences* preferences, QWidget* parent = 0);
	
	QxVideoTerminal* activeTerminal() const;
	
	bool activate(Ref<Document> document);
	
signals:
	void activeTerminalChanged(QxVideoTerminal*);
	void gotoLinkRequest(Ref<Link, Owner> link);
	void openFile(QString path);
	void fullscreenMode(bool on);
	
public slots:
	void openTerminal(int index = -1);
	void closeTerminal(int index = -1);
	
private slots:
	void changeActiveWidget(QWidget* old, QWidget* now);
	void shellTerminated();
	void activeProcessChanged(const QString& job);
	void windowTitleChanged(const QString& title);
	void locationInfoChanged(const QString& user, const QString& host, const QString& cwd);
	void sendSignal(QAction* action);
	void terminalTitleChanged(const QString& title);
	
private:
	int tabIndexOfTerminal(QxVideoTerminal* vt);
	QString generateTabLabel(QxVideoTerminal* vt) const;
	void updateTabLabel(QxVideoTerminal* vt);
	QWidget* createTerminalWidget();
	QMenu* createSignalsMenu(QWidget* vt);
	
	QPointer<QxPreferences> preferences_;
};

} // namespace pacu

#endif // PACU_QXVIDEOTERMINALMANAGER_HPP
