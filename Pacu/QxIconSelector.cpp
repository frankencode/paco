#include <QToolButton>
#include <QListWidget>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QLayout>
#include <QApplication>
#include "QxIconSelector.hpp"

namespace pacu
{

QxIconSelector::QxIconSelector(QWidget* parent)
	: QxDialog(parent)
{
	setWindowTitle(qApp->applicationName() + tr("- Select Icon"));
	
	QVBoxLayout* col = new QVBoxLayout;
	{
		QHBoxLayout* row = new QHBoxLayout;
		
		iconButton_ = new QToolButton(this);
		iconButton_->setText("...");
		iconButton_->setToolButtonStyle(Qt::ToolButtonIconOnly);
		iconButton_->setIconSize(QSize(24, 24));
		connect(iconButton_, SIGNAL(pressed()), this, SLOT(iconFromFile()));
		
		QToolButton* customButton = new QToolButton(this);
		customButton->setText("Custom");
		connect(customButton, SIGNAL(pressed()), this, SLOT(iconFromFile()));
		
		row->addWidget(iconButton_);
		row->addWidget(customButton);
		row->addStretch();
		
		col->addLayout(row);
	}
	{
		iconList_ = new QListWidget(this);
		iconList_->setViewMode(QListView::IconMode);
		loadBuiltinIcons();
		connect(iconList_, SIGNAL(itemPressed(QListWidgetItem*)), this, SLOT(itemSelected(QListWidgetItem*)));
		col->addWidget(iconList_);
	}
	{
		QDialogButtonBox* buttonBox = new QDialogButtonBox(this);
		buttonBox->setOrientation(Qt::Horizontal);
		buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
		connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
		connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
		col->addWidget(buttonBox);
	}
	setLayout(col);
}

QIcon QxIconSelector::icon() const { return iconButton_->icon(); }

void QxIconSelector::iconFromFile()
{
	QString path =
		QFileDialog::getOpenFileName(
			this,
			"Import icon (ideally 24x24)" /* caption */,
			QString() /* dir */,
			"Images (*.png *.xpm *.jpg)" /* filter */,
			0 /* selected filter */,
			QFileDialog::DontUseSheet /* options, e.g. QFileDialog::DontUseNativeDialog */
		);
	if (path != QString()) {
		QPixmap pm;
		if (pm.load(path)) {
			iconButton_->setIcon(pm);
		}
	}
}

void QxIconSelector::itemSelected(QListWidgetItem* item)
{
	iconButton_->setIcon(item->icon());
}

void QxIconSelector::loadBuiltinIcons()
{
	#include "sky/24x24/filenew.c"
	#include "sky/24x24/fileverify.c"
	#include "sky/24x24/wwwlookup.c"
	#include "sky/24x24/wwwbrowser.c"
	#include "sky/24x24/book.c"
	#include "sky/24x24/wwwbook.c"
	#include "sky/24x24/script.c"
	#include "sky/24x24/scriptstart.c"
	#include "sky/24x24/scriptstop.c"
	#include "sky/24x24/scriptreset.c"
	#include "sky/24x24/scripthalt.c"
	#include "sky/24x24/question.c"
	#include "sky/24x24/print.c"
	#include "sky/24x24/pencil.c"
	#include "sky/24x24/idea.c"
	#include "sky/24x24/chat.c"
	#include "sky/24x24/protect.c"
	#include "sky/24x24/piechart.c"
	#include "sky/24x24/barchart.c"
	#include "sky/24x24/reload.c"
	{ QPixmap pm; pm.loadFromData(filenew,     filenewSize);     new QListWidgetItem(pm, QString(), iconList_); }
	{ QPixmap pm; pm.loadFromData(fileverify,  fileverifySize);  new QListWidgetItem(pm, QString(), iconList_); }
	{ QPixmap pm; pm.loadFromData(wwwlookup,   wwwlookupSize);   new QListWidgetItem(pm, QString(), iconList_); }
	{ QPixmap pm; pm.loadFromData(wwwbrowser,  wwwbrowserSize);  new QListWidgetItem(pm, QString(), iconList_); }
	{ QPixmap pm; pm.loadFromData(book,        bookSize);        new QListWidgetItem(pm, QString(), iconList_); }
	{ QPixmap pm; pm.loadFromData(wwwbook,     wwwbookSize);     new QListWidgetItem(pm, QString(), iconList_); }
	{ QPixmap pm; pm.loadFromData(script,      scriptSize);      new QListWidgetItem(pm, QString(), iconList_); }
	{ QPixmap pm; pm.loadFromData(scriptstart, scriptstartSize); new QListWidgetItem(pm, QString(), iconList_); }
	{ QPixmap pm; pm.loadFromData(scriptstop,  scriptstopSize);  new QListWidgetItem(pm, QString(), iconList_); }
	{ QPixmap pm; pm.loadFromData(scriptreset, scriptresetSize); new QListWidgetItem(pm, QString(), iconList_); }
	{ QPixmap pm; pm.loadFromData(scripthalt,  scripthaltSize);  new QListWidgetItem(pm, QString(), iconList_); }
	{ QPixmap pm; pm.loadFromData(question,    questionSize);    new QListWidgetItem(pm, QString(), iconList_); }
	{ QPixmap pm; pm.loadFromData(print,       printSize);       new QListWidgetItem(pm, QString(), iconList_); }
	{ QPixmap pm; pm.loadFromData(pencil,      pencilSize);      new QListWidgetItem(pm, QString(), iconList_); }
	{ QPixmap pm; pm.loadFromData(idea,        ideaSize);        new QListWidgetItem(pm, QString(), iconList_); }
	{ QPixmap pm; pm.loadFromData(chat,        chatSize);        new QListWidgetItem(pm, QString(), iconList_); }
	{ QPixmap pm; pm.loadFromData(protect,     protectSize);     new QListWidgetItem(pm, QString(), iconList_); }
	{ QPixmap pm; pm.loadFromData(piechart,    piechartSize);    new QListWidgetItem(pm, QString(), iconList_); }
	{ QPixmap pm; pm.loadFromData(barchart,    barchartSize);    new QListWidgetItem(pm, QString(), iconList_); }
	{ QPixmap pm; pm.loadFromData(reload,      reloadSize);      new QListWidgetItem(pm, QString(), iconList_); }
}

} // namespace pacu
