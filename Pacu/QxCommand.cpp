#include <QDebug>
#include <QBuffer>
#include "QxCommand.hpp"

namespace pacu
{

QxCommand::QxCommand(QObject* parent)
	: QObject(parent),
	  target_(ActiveTerminal),
	  autoSaveFile_(false),
	  autoOpenNextLink_(true),
	  action_(0)
{}

QxCommand::~QxCommand()
{
	if (action_)
		delete action_;
}

void QxCommand::assignAction(QAction* action)
{
	action_ = action;
	connect(action_, SIGNAL(triggered()), this, SLOT(strike()));
}

void QxCommand::updateAction()
{
	action_->setIcon(icon_);
	action_->setToolTip(description_);
	if (description_ != QString())
		action_->setText(description_);
	else
		action_->setText(script_);
	QString keys = keysToString();
	#ifdef Q_WS_MAC
	// workaround HACK
	keys.replace("Ctrl", "HHHH");
	keys.replace("Meta", "Ctrl");
	keys.replace("HHHH", "Meta");
	#endif
	action_->setShortcut(keys);
}

QAction* QxCommand::action() const { return action_; }

void QxCommand::debug()
{
	QxCommand* cmd = this;
	qDebug() << "QxCommand {";
	qDebug() << "  icon: " << cmd->icon_.size();
	qDebug() << "  description: " << cmd->description_;
	qDebug() << "  keys: " << cmd->keys_;
	qDebug() << "  script: " << cmd->script_;
	qDebug() << "  target: " << cmd->target_;
	qDebug() << "}";
}

int QxCommand::numAttributes() { return 2; }

QString QxCommand::attributeName(int k)
{
	QString name;
	if (k == 0)
		name = tr("Shortcut");
	else if (k == 1)
		name = tr("Description");
	else if (k == 2)
		name = tr("Command");
	else if (k == 3)
		name = tr("Target");
	return name;
}

QVariant QxCommand::attribute(int k) const
{
	QVariant value;
	
	if (k == 0) {
		value = keysToString();
	}
	else if (k == 1) {
		value = (description_ != QString()) ? description_ : script_;
	}
	else if (k == 2) {
		value = script_;
	}
	else if (k == 3) {
		if (target_ == ActiveTerminal)
			value = tr("Active Terminal");
		else if (target_ == NewTerminal)
			value = tr("New Terminal");
		/*else if (target_ == TextEditor)
			value = tr("Text Editor");*/
		else if (target_ == WebBrowser)
			value = tr("Web Browser");
	}
	
	return value;
}

QString QxCommand::keysToString(QString sep, bool reverse) const
{
	QStringList l;
	for (int i = reverse ? keys_.count()-1 : 0; reverse ? (i >= 0) : (i < int(keys_.count())); i += reverse ? -1 : +1) {
		if (keys_[i] == Qt::Key_Meta)
			l << tr("Meta");
		else if (keys_[i] == Qt::Key_Control)
			l << tr("Ctrl");
		else if (keys_[i] == Qt::Key_Alt)
			l << tr("Alt");
		else if (keys_[i] == Qt::Key_Shift)
			l << tr("Shift");
		else
			l << QKeySequence(keys_[i]).toString();
	}
	return l.join(sep);
}

QVariantList QxCommand::save() const
{
	QByteArray encodedImage;
	{
		QBuffer buffer(&encodedImage);
		buffer.open(QIODevice::WriteOnly);
		icon_.save(&buffer, "PNG");
	}
	
	QVariantList list;
	list.append(encodedImage);
	list.append(description_);
	list.append(keysToString("+"));
	list.append(script_);
	list.append(target_);
	list.append(autoSaveFile_);
	list.append(autoOpenNextLink_);
	return list;
}

void QxCommand::restore(QVariantList list)
{
	int i = 0;
	icon_.loadFromData(list.at(i++).toByteArray(), "PNG");
	description_ = list.at(i++).toString();
	keys_ = list.at(i++).toString();
	script_ = list.at(i++).toString();
	target_ = list.at(i++).toInt();
	if (i < list.length()) {
		autoSaveFile_ = list.at(i++).toBool();
		if (i < list.length()) 
			autoOpenNextLink_ = list.at(i++).toBool();
		else
			autoOpenNextLink_ = false;
	}
	else
		autoSaveFile_ = false;
}

void QxCommand::strike()
{
	emit triggered(this);
}

} // namespace pacu
