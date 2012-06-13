#include <QtGui>
#include <vide/Core.hpp>

namespace vide
{

class QxMainWindow: public QMainWindow
{
public:
	QxMainWindow() {
	#ifdef Q_WS_MAC
		setUnifiedTitleAndToolBarOnMac(true);
	#endif
		QToolBar* bar = addToolBar(tr("Toolbar"));
		bar->setFloatable(false);
		bar->setMovable(false);
		
		QWidget* carrier = new QWidget(this);
		QVBoxLayout* cell0 = new QVBoxLayout;
		cell0->setSpacing(0);
		cell0->setMargin(0);
		carrier->setLayout(cell0);
		setCentralWidget(carrier);
		{
			QxControl* control = new QxControl(this);
			control->visual()->style()->setBackground(new QxTexture(Qt::white));
			control->visual()->setText("Hello, world!\nSecond line\nThird line\nAnother test string");
			control->setMode(QxControl::TextEditMode);
			control->visual()->style()->setMargins(2, 2, 2, 2);
			cell0->addWidget(control);
		}
		cell0->addWidget(styleManager()->hl(this));
		{
			QxControl* control = new QxControl(this);
			control->visual()->setText("[a very long absolutely nondescriptive test string]");
			control->setMode(QxControl::TextEditMode);
			control->visual()->style()->setMargins(2, 2, 2, 2);
			if (control->mode() == QxControl::TextEditMode)
				control->visual()->style()->setElideMode(Qt::ElideRight);
			cell0->addWidget(control);
		}
	}
};

int main(int argc, char** argv)
{
	QApplication app(argc, argv);
	QxMainWindow* win = new QxMainWindow;
	win->show();
	return app.exec();
}

} // namespace vide

int main(int argc, char** argv)
{
	return vide::main(argc, argv);
}
