#include <QLayout>
#include <QApplication>
#include <QLabel>
#include <QPushButton>
#include <QDateTime>
#include <QDesktopServices>
#include <QUrl>
#include <ftl/FileStatus.hpp>
#include <ftl/Process.hpp>
#include <ftl/Time.hpp>
#include <seal/Activation.hpp>
#include "UseFtl.hpp"
#include "QxAboutDialog.hpp"

namespace pacu
{

QxAboutDialog::QxAboutDialog(QWidget* parent)
	: QxDialog(parent)
{
	setWindowTitle(tr("About %1").arg(qApp->applicationName()));
	
	QVBoxLayout* col = new QVBoxLayout;
	col->setContentsMargins(40/*left*/, 4/*top*/, 40/*right*/, 20/*bottom*/);
	col->setSpacing(4);
	{
		QLabel* label = new QLabel;
		label->setPixmap(qApp->windowIcon().pixmap(512, 512).scaled(QSize(100, 100), Qt::KeepAspectRatio, Qt::SmoothTransformation));
		col->addWidget(label, 0, Qt::AlignCenter);
	}
	{
		Ref<FileStatus, Owner> exe = new FileStatus(Process::execPath());
		QString buildTime;
		{
			QDateTime h;
			h.setTime_t(exe->lastModified().s());
			buildTime = h.toString(Qt::ISODate);
			buildTime.replace("T", " / ");
		}
		QLabel* label = new QLabel;
		
		label->setText(
			tr(
				"<center>"
				"<h3>%1</h3>"
				"<font size=\"2\">"
				// "<p>The lightweight cross-platform code editor</p>"
				"<p>Version %2</p>"
				"<p>Build at %3</p>"
				"<p>Powered by <a href=\"http://qt.nokia.com/\">Qt</a> %4 (<a href=\"http://www.cyblogic.com/LGPL2.txt\">LGPLv2</a> edition)"
				"<br/>Copyright &copy; 2009-2011 Nokia Corporation</p>"
				"<p>Powered by %5</p>"
				"<p>Copyright &copy; 2008-2011 Frank Mertens<br/>All rights reserved.</p>"
				"</font>"
				"</center>"
			)
			.arg(QCoreApplication::applicationName())
			.arg(QCoreApplication::applicationVersion())
			.arg(buildTime)
			.arg(qVersion())
			.arg(QString::fromUtf8(seal::Activation::curlVersion())
				.replace("libcurl", "<a href=\"http://curl.haxx.se/\">libcurl</a>"))
		);
		col->addWidget(label, 0, Qt::AlignCenter);
		
		connect(label, SIGNAL(linkActivated(const QString&)), this, SLOT(openUrl(const QString&)));
	}
	#ifndef Q_WS_MAC
	{
		QPushButton* ok = new QPushButton;
		ok->setText("Close");
		connect(ok, SIGNAL(pressed()), this, SLOT(accept()));
		col->addSpacing(16);
		col->addWidget(ok, 0, Qt::AlignCenter);
	}
	#endif
	setLayout(col);
}

void QxAboutDialog::openUrl(const QString& url)
{
	QDesktopServices::openUrl(QUrl(url));
}

} // namespace pacu
