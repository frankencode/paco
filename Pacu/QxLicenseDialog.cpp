#include <QApplication>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QTextCodec>
#include <QLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollBar>
#include <QFrame>
#include <pte/View.hpp>
#include "UsePte.hpp"
#include "QxLicenseDialog.hpp"

namespace pacu
{

QxLicenseDialog::QxLicenseDialog(QWidget* parent)
	: QxDialog(parent)
{
	QVBoxLayout* col = new QVBoxLayout;
	col->addWidget(new QLabel("Software License Agreement"));
	{
		Ref<Document, Owner> doc = new Document;
		
		{
			QString licensePath;
			QDir dir(qApp->applicationDirPath());
			dir.cdUp();
			#ifdef Q_WS_X11
			licensePath = dir.filePath("COPYING");
			#else
			dir.cd("Resources");
			licensePath = dir.filePath("License.txt");
			#endif
			QFile file(licensePath);
			file.open(QIODevice::ReadOnly);
			{
				QTextStream source(&file);
				source.setCodec(QTextCodec::codecForName("UTF-8"));
				doc->load(&source);
			}
		}
		
		QScrollBar* scroll = new QScrollBar;
		
		Ref<ViewMetrics, Owner> metrics = View::defaultMetrics();
		metrics->showLineNumbers_ = false;
		metrics->font_.setPixelSize(metrics->font_.pixelSize() - 1);
		View* view = new View(this, metrics);
		view->setVerticalScrollBar(scroll);
		view->setDocument(doc);
		view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		
		QHBoxLayout* row = new QHBoxLayout;
		row->setSpacing(0);
		row->setMargin(0);
		row->addWidget(view);
		row->addWidget(scroll);
		
		QFrame* frame = new QFrame;
		frame->setFrameShadow(QFrame::Sunken);
		frame->setFrameStyle(QFrame::StyledPanel);
		frame->setLayout(row);
		
		col->addWidget(frame);
	}
	{
		QPushButton* ok = new QPushButton;
		ok->setText("Close");
		connect(ok, SIGNAL(pressed()), this, SLOT(accept()));
		col->addWidget(ok, 0, Qt::AlignCenter);
	}
	setLayout(col);
	resize(800, 600);
}

} // namespace pacu
