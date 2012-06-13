#include <QHBoxLayout>
#include <QStackedLayout>
#include <QScrollBar>
#include <QTimer>
#include <QMenu>
#include <ftl/Process.hpp>
#include <ftl/ProcessStatus.hpp>
#include "QxTabBar.hpp"
#include "QxPreferences.hpp"
#include "QxVideoTerminal.hpp"
#include "QxWidgetStack.hpp"
#include "QxTerminalStack.hpp"

namespace pacu
{

QxTerminalStack::QxTerminalStack(QxPreferences* preferences, QWidget* parent)
	: QxTabWidget(parent),
	  preferences_(preferences)
{
	connect(preferences, SIGNAL(terminalTitleChanged(const QString&)), this, SLOT(terminalTitleChanged(const QString&)));
	
	tabBar()->setSemanticType("terminal");
	connect(tabBar(), SIGNAL(changeActiveWidget(QWidget*, QWidget*)), this, SLOT(changeActiveWidget(QWidget*, QWidget*)));
	connect(tabBar(), SIGNAL(closeRequest(int)), this, SLOT(closeTerminal(int)));
	tabBar()->setVisible(false);
	QTimer::singleShot(0, this, SLOT(openTerminal()));
}

QxVideoTerminal* QxTerminalStack::activeTerminal() const
{
	return tabBar()->widget(tabBar()->indexActive())->findChild<QxVideoTerminal*>();
}

bool QxTerminalStack::activate(Ref<Document> document)
{
	for (int i = 0, n = tabBar()->tabCount(); i < n; ++i) {
		QxVideoTerminal* vt = tabBar()->widget(i)->findChild<QxVideoTerminal*>();
		if (vt->document() == document) {
			if (vt != activeTerminal())
				tabBar()->activate(i);
			return true;
		}
	}
	return false;
}

void QxTerminalStack::openTerminal(int index)
{
	if (index == -1) index = tabBar()->tabCount();
	QString label = "<none>";
	QWidget* widget = createTerminalWidget();
	tabBar()->openTab(index, widget->findChild<QxVideoTerminal*>()->defaultShellName(), QPixmap(), widget);
	if ((tabBar()->tabCount() > 1) && (!tabBar()->isVisible()))
		tabBar()->show();
}

void QxTerminalStack::closeTerminal(int index)
{
	if (tabBar()->tabCount() > 1) {
		if (index == -1) index = tabBar()->indexActive();
		tabBar()->closeTab(index);
	}
	if (tabBar()->tabCount() == 1)
		tabBar()->hide();
}

void QxTerminalStack::changeActiveWidget(QWidget* old, QWidget* now)
{
	setFocusProxy(now);
	emit activeTerminalChanged(activeTerminal());
}

void QxTerminalStack::shellTerminated()
{
	QxVideoTerminal* vt = qobject_cast<QxVideoTerminal*>(sender());
	closeTerminal(tabIndexOfTerminal(vt));
}

void QxTerminalStack::activeProcessChanged(const QString& job)
{
	QxVideoTerminal* vt = qobject_cast<QxVideoTerminal*>(sender());
	updateTabLabel(vt);
}

void QxTerminalStack::windowTitleChanged(const QString& title)
{
	QxVideoTerminal* vt = qobject_cast<QxVideoTerminal*>(sender());
	updateTabLabel(vt);
}

void QxTerminalStack::locationInfoChanged(const QString& user, const QString& host, const QString& cwd)
{
	QxVideoTerminal* vt = qobject_cast<QxVideoTerminal*>(sender());
	updateTabLabel(vt);
}

void QxTerminalStack::sendSignal(QAction* action)
{
	QxVideoTerminal* vt = activeTerminal();
	if (vt) {
		ProcessStatus status(vt->shell()->id());
		try  { Process::kill(status.foregroundProcessGroupId(), action->data().toInt()); } catch(...) {}
	}
}

void QxTerminalStack::terminalTitleChanged(const QString& title)
{
	for (int i = 0, n = tabBar()->tabCount(); i < n; ++i)
		updateTabLabel(tabBar()->widget(i)->findChild<QxVideoTerminal*>());
}

int QxTerminalStack::tabIndexOfTerminal(QxVideoTerminal* vt)
{
	int index = -1;
	for (int i = 0, n = tabBar()->tabCount(); i < n; ++i) {
		if (tabBar()->widget(i)->findChild<QxVideoTerminal*>() == vt) {
			index = i;
			break;
		}
	}
	return index;
}

QWidget* QxTerminalStack::createTerminalWidget()
{
	QxVideoTerminal* vt = new QxVideoTerminal("", "", preferences_->terminalPalette());
	
	connect(vt, SIGNAL(shellTerminated()), this, SLOT(shellTerminated()));
	connect(vt, SIGNAL(activeProcessChanged(const QString&)), this, SLOT(activeProcessChanged(const QString&)));
	connect(vt, SIGNAL(windowTitleChanged(const QString&)), this, SLOT(windowTitleChanged(const QString&)));
	connect(
		vt, SIGNAL(locationInfoChanged(const QString&, const QString&, const QString&)),
		this, SLOT(locationInfoChanged(const QString&, const QString&, const QString&))
	);
	connect(vt, SIGNAL(openFile(QString)), this, SIGNAL(openFile(QString)));
	connect(vt, SIGNAL(fullscreenMode(bool)), this, SIGNAL(fullscreenMode(bool)));
	
	connect(vt, SIGNAL(gotoLinkRequest(Ref<Link, Owner>)), this, SIGNAL(gotoLinkRequest(Ref<Link, Owner>)));
	
	vt->setMetrics(preferences_->terminalMetrics());
	connect(preferences_, SIGNAL(terminalMetricsChanged(Ref<ViewMetrics>)), vt, SLOT(setMetrics(Ref<ViewMetrics>)));
	connect(preferences_, SIGNAL(terminalNumberOfLinesChanged(int)), vt, SLOT(setNumberOfLines(int)));
	connect(preferences_, SIGNAL(terminalPaletteChanged(Ref<Palette>)), vt, SLOT(setPalette(Ref<Palette>)));
	
	/*vt->setPaletteColor(000, Qt::white);     // black, 000 == QxVideoTerminal::DefaultBgColor
	vt->setPaletteColor(001, Qt::red);     // red
	vt->setPaletteColor(002, QColor::fromRgbF(0., 0.9, 0.));     // green
	vt->setPaletteColor(003, Qt::darkYellow);     // yellow
	vt->setPaletteColor(004, Qt::blue);     // blue
	vt->setPaletteColor(005, Qt::magenta);     // magenta
	vt->setPaletteColor(006, Qt::darkCyan);     // cyan
	vt->setPaletteColor(007, Qt::black);  // white, 007 == QxVideoTerminal::DefaultFgColor
	vt->setPaletteColor(010, Qt::darkCyan);     // bright black
	vt->setPaletteColor(011, Qt::darkRed);     // bright red
	vt->setPaletteColor(012, QColor::fromRgbF(0., 0.8, 0.));     // bright green
	vt->setPaletteColor(013, Qt::darkYellow);     // bright yellow
	vt->setPaletteColor(014, Qt::darkBlue);     // bright blue
	vt->setPaletteColor(015, Qt::magenta);     // bright magenta
	vt->setPaletteColor(016, Qt::darkCyan);     // bright cyan
	vt->setPaletteColor(017, Qt::darkCyan);     // bright white
	*/
	// vt->setBlanking(5);
	
	{
		QAction* copy = new QAction(tr("Copy"), vt);
		QAction* paste = new QAction(tr("Paste"), vt);
		QAction* selectAll = new QAction(tr("Select All"), vt);
		QAction* clear = new QAction(tr("Clear"), vt);
		QAction* cintr = new QAction(tr("CINTR"), vt);
		QAction* csusp = new QAction(tr("CSUSP"), vt);
		QAction* ceof = new QAction(tr("CEOF"), vt);
		QAction* links = new QAction(tr("Detect Links"), vt);
		
		copy->setShortcut(tr("Ctrl+C"));
		paste->setShortcut(tr("Ctrl+V"));
		selectAll->setShortcut(tr("Ctrl+A"));
		clear->setShortcut(tr("Ctrl+Del"));
		
		#ifdef Q_WS_MAC
		cintr->setShortcut(tr("Meta+C"));
		csusp->setShortcut(tr("Meta+Z"));
		ceof->setShortcut(tr("Meta+D"));
		#else
		cintr->setShortcut(tr("Ctrl+C"));
		csusp->setShortcut(tr("Ctrl+Z"));
		ceof->setShortcut(tr("Ctrl+D"));
		#endif
		
		vt->addAction(copy);
		vt->addAction(paste);
		vt->addAction(selectAll);
		vt->addAction(clear);
		vt->addAction(cintr);
		vt->addAction(csusp);
		vt->addAction(ceof);
		vt->addAction(links);
		
		connect(copy, SIGNAL(triggered()), vt, SLOT(copy()));
		connect(paste, SIGNAL(triggered()), vt, SLOT(paste()));
		connect(selectAll, SIGNAL(triggered()), vt, SLOT(selectAll()));
		connect(clear, SIGNAL(triggered()), vt, SLOT(shellClear()));
		connect(cintr, SIGNAL(triggered()), vt, SLOT(cintr()));
		connect(csusp, SIGNAL(triggered()), vt, SLOT(csusp()));
		connect(ceof, SIGNAL(triggered()), vt, SLOT(ceof()));
		
		#ifdef Q_WS_X11 // because Ctrl-A is much to othen used by term apps itself, e.g. 'screen'
		connect(this, SIGNAL(fullscreenMode(bool)), selectAll, SLOT(setDisabled(bool)));
		#endif
		/*
		connect(this, SIGNAL(fullscreenMode(bool)), cintr, SLOT(setDisabled(bool)));
		connect(this, SIGNAL(fullscreenMode(bool)), csusp, SLOT(setDisabled(bool)));
		connect(this, SIGNAL(fullscreenMode(bool)), ceof, SLOT(setDisabled(bool)));
		*/
		
		copy->setEnabled(false);
		connect(vt, SIGNAL(hasSelectionChanged(bool)), copy, SLOT(setEnabled(bool)));
		#ifdef Q_WS_X11
		connect(vt, SIGNAL(hasSelectionChanged(bool)), cintr, SLOT(setDisabled(bool)));
		#endif
		
		links->setCheckable(true);
		links->setChecked(vt->commitLines());
		connect(links, SIGNAL(toggled(bool)), vt, SLOT(setCommitLines(bool)));
		
		{
			QList<QAction*> actions = vt->actions();
			for (int i = 0, n = actions.count(); i < n; ++i)
				actions.at(i)->setShortcutContext(Qt::WidgetShortcut);
		}
		
		QMenu* menu = new QMenu(vt);
		menu->setCursor(Qt::ArrowCursor);
		menu->addAction(copy);
		menu->addAction(paste);
		menu->addAction(selectAll);
		menu->addSeparator();
		menu->addAction(clear);
		menu->addSeparator();
		menu->addAction(cintr);
		menu->addAction(csusp);
		menu->addAction(ceof);
		menu->addSeparator();
		menu->addMenu(createSignalsMenu(vt));
		menu->addSeparator();
		menu->addAction(links);
		
		vt->setContextMenu(menu);
	}
	
	return vt->widget();
}

QMenu* QxTerminalStack::createSignalsMenu(QWidget* vt)
{
	QMenu* sigMenu = new QMenu(tr("Send Signal"), vt);
	connect(sigMenu, SIGNAL(triggered(QAction*)), this, SLOT(sendSignal(QAction*)));
	{
		QMenu* menu = new QMenu(tr("Job Control"), vt);
		#ifdef SIGINT
		QAction* sigint    = new QAction("SIGINT",    vt); sigint   ->setData(SIGINT);
		menu->addAction(sigint);
		#endif
		#ifdef SIGHUP
		QAction* sighup    = new QAction("SIGHUP",    vt); sighup   ->setData(SIGHUP);
		menu->addAction(sighup);
		#endif
		#if SIGTSTP
		QAction* sigtstp   = new QAction("SIGTSTP",   vt); sigtstp  ->setData(SIGTSTP);
		menu->addAction(sigtstp);
		#endif
		#ifdef SIGSTOP
		QAction* sigstop   = new QAction("SIGSTOP",   vt); sigstop  ->setData(SIGSTOP);
		menu->addAction(sigstop);
		#endif
		#ifdef SIGCONT
		QAction* sigcont   = new QAction("SIGCONT",   vt); sigcont  ->setData(SIGCONT);
		menu->addAction(sigcont);
		#endif
		#ifdef SIGCHLD
		QAction* sigchld   = new QAction("SIGCHLD",   vt); sigchld  ->setData(SIGCHLD);
		menu->addAction(sigchld);
		#endif
		sigMenu->addMenu(menu);
	}
	{
		QMenu* menu = new QMenu(tr("Termination"), vt);
		#ifdef SIGQUIT
		QAction* sigquit   = new QAction("SIGQUIT",   vt); sigquit  ->setData(SIGQUIT);
		menu->addAction(sigquit);
		#endif
		#ifdef SIGTERM
		QAction* sigterm   = new QAction("SIGTERM",   vt); sigterm  ->setData(SIGTERM);
		menu->addAction(sigterm);
		#endif
		#ifdef SIGKILL
		QAction* sigkill   = new QAction("SIGKILL",   vt); sigkill  ->setData(SIGKILL);
		menu->addAction(sigkill);
		#endif
		sigMenu->addMenu(menu);
	}
	{
		QMenu* menu = new QMenu(tr("Non-Fatal Conditions"), vt);
		#ifdef SIGALRM
		QAction* sigalrm   = new QAction("SIGALRM",   vt); sigalrm  ->setData(SIGALRM);
		menu->addAction(sigalrm);
		#endif
		#ifdef SIGVTALRM
		QAction* sigvtalrm = new QAction("SIGVTALRM", vt); sigvtalrm->setData(SIGVTALRM);
		menu->addAction(sigvtalrm);
		#endif
		#ifdef SIGPROF
		QAction* sigprof   = new QAction("SIGPROF",   vt); sigprof  ->setData(SIGPROF);
		menu->addAction(sigprof);
		#endif
		menu->addSeparator(); // --------------------------------------------
		#ifdef SIGWINCH
		QAction* sigwinch  = new QAction("SIGWINCH",  vt); sigwinch ->setData(SIGWINCH);
		menu->addAction(sigwinch);
		#endif
		#ifdef SIGUSR1
		QAction* sigusr1   = new QAction("SIGUSR1",   vt); sigusr1  ->setData(SIGUSR1);
		menu->addAction(sigusr1);
		#endif
		#ifdef SIGUSR2
		QAction* sigusr2   = new QAction("SIGUSR2",   vt); sigusr2  ->setData(SIGUSR2);
		menu->addAction(sigusr2);
		#endif
		menu->addSeparator(); // --------------------------------------------
		#ifdef SIGURG
		QAction* sigurg    = new QAction("SIGURG",    vt); sigurg   ->setData(SIGURG);
		menu->addAction(sigurg);
		#endif
		#ifdef SIGIO
		QAction* sigio     = new QAction("SIGIO",     vt); sigio    ->setData(SIGIO);
		menu->addAction(sigio);
		#else // SIGIO
		#ifdef SIGPOLL
		QAction* sigpoll   = new QAction("SIGPOLL",   vt); sigpoll  ->setData(SIGPOLL);
		menu->addAction(sigpoll);
		#endif // SIGPOLL
		#endif // SIGIO
		#ifdef SIGTTIN
		QAction* sigttin   = new QAction("SIGTTIN",   vt); sigttin  ->setData(SIGTTIN);
		menu->addAction(sigttin);
		#endif
		#ifdef SIGTTOU
		QAction* sigttou   = new QAction("SIGTTOU",   vt); sigttou  ->setData(SIGTTOU);
		menu->addAction(sigttou);
		#endif
		menu->addSeparator(); // --------------------------------------------
		#ifdef SIGTRAP
		QAction* sigtrap   = new QAction("SIGTRAP",   vt); sigtrap  ->setData(SIGTRAP);
		menu->addAction(sigtrap);
		#endif
		sigMenu->addMenu(menu);
	}
	{
		QMenu* menu = new QMenu(tr("Fatal Conditions"), vt);
		#ifdef SIGPIPE
		QAction* sigpipe   = new QAction("SIGPIPE",   vt); sigpipe  ->setData(SIGPIPE);
		menu->addAction(sigpipe);
		#endif
		#ifdef SIGABRT
		QAction* sigabrt   = new QAction("SIGABRT",   vt); sigabrt  ->setData(SIGABRT);
		menu->addAction(sigabrt);
		#endif
		#ifdef SIGIOT
		QAction* sigiot    = new QAction("SIGIOT",    vt); sigiot   ->setData(SIGIOT);
		menu->addAction(sigiot);
		#endif
		menu->addSeparator(); // --------------------------------------------
		#ifdef SIGPFE
		QAction* sigfpe    = new QAction("SIGFPE",    vt); sigfpe   ->setData(SIGFPE);
		menu->addAction(sigfpe);
		#endif
		#ifdef SIGBUS
		QAction* sigbus    = new QAction("SIGBUS",    vt); sigbus   ->setData(SIGBUS);
		menu->addAction(sigbus);
		#endif
		#ifdef SIGSEGV
		QAction* sigsegv   = new QAction("SIGSEGV",   vt); sigsegv  ->setData(SIGSEGV);
		menu->addAction(sigsegv);
		#endif
		#ifdef SIGSTKFLT
		QAction* sigstkflt = new QAction("SIGSTKFLT", vt); sigstkflt->setData(SIGSTKFLT);
		menu->addAction(sigstkflt);
		#endif
		#ifdef SIGILL
		QAction* sigill    = new QAction("SIGILL",    vt); sigill   ->setData(SIGILL);
		menu->addAction(sigill);
		#endif
		menu->addSeparator(); // --------------------------------------------
		#ifdef SIGSYS
		QAction* sigsys    = new QAction("SIGSYS",    vt); sigsys   ->setData(SIGSYS);
		menu->addAction(sigsys);
		#endif
		sigMenu->addMenu(menu);
		#ifdef SIGPWR
		QAction* sigpwr    = new QAction("SIPWR",     vt); sigpwr   ->setData(SIGPWR);
		menu->addAction(sigpwr);
		#endif
		#ifdef SIGXCPU
		QAction* sigxcpu   = new QAction("SIGXCPU",   vt); sigxcpu  ->setData(SIGXCPU);
		menu->addAction(sigxcpu);
		#endif
		#ifdef SIGXFSZ
		QAction* sigxfsz   = new QAction("SIGXFSZ",   vt); sigxfsz  ->setData(SIGXFSZ);
		menu->addAction(sigxfsz);
		#endif
		sigMenu->addMenu(menu);
	}
	return sigMenu;
}

QString QxTerminalStack::generateTabLabel(QxVideoTerminal* vt) const
{
	QString label = preferences_->terminalTitle();
	label = label.replace("$FG", vt->activeProcess());
	label = label.replace("$TITLE", vt->windowTitle());
	label = label.replace("$USER", vt->user());
	label = label.replace("$HOST", vt->host());
	label = label.replace("$FQHOST", vt->fqHost());
	label = label.replace("$CWD", vt->cwdSimplified());
	return label;
}

void QxTerminalStack::updateTabLabel(QxVideoTerminal* vt)
{
	tabBar()->setLabel(tabIndexOfTerminal(vt), generateTabLabel(vt));
}

} // namespace pacu
