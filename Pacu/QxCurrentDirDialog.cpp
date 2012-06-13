#include <QLayout>
#include <QDir>
#include <QDirModel>
#include <QLineEdit>
#include <QToolButton>
#include <QCompleter>
#include <QMessageBox>
#include <QFileDialog>
#include "QxCurrentDirDialog.hpp"

namespace pacu
{

QxCurrentDirDialog::QxCurrentDirDialog(QWidget* parent)
	: QxDialog(parent)
{
	setWindowTitle(tr("Current Working Directory"));
	input_ = new QLineEdit(this);
	input_->setText(QDir::currentPath());
	
	QCompleter* completer = new QCompleter(this);
	completer->setModel(new QDirModel(completer)); // or better use QFileSystemModel?
	completer->setCompletionMode(QCompleter::InlineCompletion);
	input_->setCompleter(completer);
	
	button_ = new QToolButton(this);
	button_->setText("...");
	#ifdef Q_WS_MAC
	connect(button_, SIGNAL(clicked()), this, SLOT(cascade()));
	#else
	connect(button_, SIGNAL(pressed()), this, SLOT(cascade()));
	#endif
	
	QHBoxLayout* layout = new QHBoxLayout;
	layout->addWidget(button_);
	layout->addWidget(input_);
	setLayout(layout);
}

QString QxCurrentDirDialog::exec()
{
	resize(300, sizeHint().height()); // visual HACK
	
	while (true)
	{
		if (QDialog::exec() == QDialog::Rejected)
			return QString();
		
		if (QDir(input_->text()).exists())
			break;
		
		QMessageBox info(this);
		info.setIcon(QMessageBox::Information);
		info.setText(tr("No such directory:"));
		info.setInformativeText(QString("\"%1\"").arg(input_->text()));
		info.setStandardButtons(QMessageBox::Ok);
		info.exec();
	}
	
	return input_->text();
}

void QxCurrentDirDialog::cascade()
{	
	QString path = QFileDialog::getExistingDirectory(this, QString()/*caption*/, QDir::currentPath());
	if (path != QString())
		input_->setText(path);
	button_->setChecked(false); // visual HACK, OS X
}

} // namespace pacu
