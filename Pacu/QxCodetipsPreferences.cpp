#include <QComboBox>
#include <QLabel>
#include <QTabWidget>
#include <QLayout>
#include <QStackedLayout>
#include <QLineEdit>
#include <QToolButton>
#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QFileDialog>
#include <ftl/Process.hpp>
#include <ftl/Format.hpp>
#include <ftl/PrintDebug.hpp>
#include <pte/LanguageManager.hpp>
#include <pte/LanguageStack.hpp>
#include <codetips/codetips>
#include "QxPreferences.hpp"
#include "QxVideoTerminal.hpp"
#include "QxCodetipsPreferences.hpp"

namespace pacu
{

using namespace codetips;

QxCodetipsPreferences::QxCodetipsPreferences(QxPreferences* preferences, QWidget* parent)
	: QWidget(parent),
	  preferences_(preferences),
	  assistantByStackIndex_(new AssistantByStackIndex),
	  stackIndexByLocator_(new StackIndexByLocator),
	  locatorByWidget_(new LocatorByWidget)
{
	log_ = new QxVideoTerminal(QString::fromUtf8(Process::execPath()), "--follow");
	connect(preferences, SIGNAL(terminalPaletteChanged(Ref<Palette>)), log_, SLOT(setPalette(Ref<Palette>)));
	
	language_ = new QComboBox;
	{
		Ref<AssistantListByLanguage> assistantListByLanguage = AssistantManager::instance()->assistantListByLanguage();
		int plainIndex = 0;
		for (int i = 0; i < assistantListByLanguage->length(); ++i) {
			String name = assistantListByLanguage->get(i).key();
			if (name == "any") continue;
			if (name == "plain") plainIndex = language_->count();
			Ref<LanguageStack> stack = preferences->languageManager()->stackByName(name->data());
			language_->addItem(stack->displayName(), stack->name());
		}
		language_->setCurrentIndex(plainIndex);
	}
	connect(language_, SIGNAL(activated(int)), this, SLOT(selectLanguage(int)));
	
	assistant_ = new QComboBox;
	connect(assistant_, SIGNAL(activated(int)), this, SLOT(selectAssistant(int)));
	
	QVBoxLayout* col = new QVBoxLayout;
	{
		QHBoxLayout* row = new QHBoxLayout;
		row->addWidget(language_);
		row->addWidget(assistant_);
		col->addLayout(row);
	}
	{
		QTabWidget* tabs = new QTabWidget;
		tabs->setTabPosition(QTabWidget::West);
		tabs->setSizePolicy(tabs->sizePolicy().horizontalPolicy(), QSizePolicy::Expanding);
		{
			QWidget* widget = new QWidget;
			resourceStack_ = new QStackedLayout;
			widget->setLayout(resourceStack_);
			tabs->addTab(widget, tr("Resource"));
		}
		tabs->addTab(log_->widget(), tr("Log"));
		col->addWidget(tabs);
	}
	
	generateResourceStack();
	selectLanguage(language_->currentIndex());
	
	setLayout(col);
}

void QxCodetipsPreferences::selectLanguage(int index)
{
	QString name = language_->itemData(index).toString();
	// qDebug() << "QxCodetipsPreferences::selectLanguage(): name =" << name;
	assistant_->clear();
	Ref<AssistantList> list = AssistantManager::instance()->assistantListByLanguage()->value(name.toUtf8().data());
	for (int i = 0; i < list->length(); ++i) {
		Ref<Assistant> assistant = list->at(i);
		assistant_->addItem(QString::fromUtf8(assistant->displayName()->data()), assistant_->count());
	}
	selectAssistant(0);
}

void QxCodetipsPreferences::selectAssistant(int index)
{
	// print("QxCodetipsPreferences::selectAssistant(): index = %%\n", index);
	String languageName = language_->itemData(language_->currentIndex()).toString().toUtf8().data();
	Ref<AssistantList> list = AssistantManager::instance()->assistantListByLanguage()->value(languageName);
	Ref<Assistant> assistant = list->at(index);
	Ref<File> file = assistant->log()->stream();
	// print("QxCodetipsPreferences::selectAssistant(): log = %%\n", file->path());
	log_->shell()->rawInput()->write(file->path());
	log_->shell()->rawInput()->write("\n");
	resourceStack_->setCurrentIndex(stackIndexByLocator_->value(locator(languageName, assistant->name())));
}

String QxCodetipsPreferences::locator(String language, String assistant)
{
	return Format("%%.%%") << language << assistant;
}

String QxCodetipsPreferences::locator(String language, String assistant, String resource)
{
	return Format("%%.%%?%%") << language << assistant << resource;
}

void QxCodetipsPreferences::generateResourceStack()
{
	int stackIndex = 0;
	for (int i = 0; i < AssistantManager::instance()->assistantListByLanguage()->length(); ++i) {
		AssistantListByLanguage::Item item = AssistantManager::instance()->assistantListByLanguage()->get(i);
		String languageName = item.key();
		Ref<AssistantList> list = item.value();
		for (int j = 0; j < list->length(); ++j, ++stackIndex) {
			Ref<Assistant> assistant = list->get(j);
			assistantByStackIndex_->insert(stackIndex, assistant);
			stackIndexByLocator_->insert(locator(languageName, assistant->name()), stackIndex);
			Ref<ResourceList> resourceList = assistant->resourceList();
			QWidget* layer = new QWidget;
			QVBoxLayout* col = new QVBoxLayout;
			QFormLayout* form = new QFormLayout;
			form->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
			{
				QLabel* label = new QLabel;
				label->setTextFormat(Qt::RichText);
				label->setOpenExternalLinks(true);
				label->setWordWrap(true);
				label->setText(assistant->description()->data());
				col->addWidget(label, 0);
			}
			col->addLayout(form, 1);
			layer->setLayout(col);
			resourceStack_->addWidget(layer);
			for (int k = 0; k < resourceList->length(); ++k) {
				ResourceList::Item item = resourceList->at(k);
				String name = item.key();
				Variant value = item.value();
				QWidget* widget = 0;
				if (value.type() == Variant::IntType)
					widget = new QSpinBox;
				else if (value.type() == Variant::PathType)
					widget = pathEdit(value);
				else
					widget = new QLineEdit;
				form->addRow(name->data(), widget);
				locatorByWidget_->insert(widget, locator(languageName, assistant->name(), name));
			}
		}
	}
}

QWidget* QxCodetipsPreferences::pathEdit(Variant value)
{
	QLineEdit* edit = new QLineEdit;
	QToolButton* button = new QToolButton;
	
	edit->setText(value.toString()->data());
	button->setText("...");
	
	connect(edit, SIGNAL(editingFinished()), this, SLOT(pathEditingFinished()));
	connect(button, SIGNAL(pressed()), this, SLOT(pathChooser()));
	
	QWidget* widget = new QWidget;
	QHBoxLayout* row = new QHBoxLayout;
	row->setMargin(0);
	row->setSpacing(0);
	row->addWidget(edit);
	// edit->setSizePolicy(QSizePolicy::Expanding, edit->sizePolicy().verticalPolicy());
	row->addWidget(button);
	widget->setLayout(row);
	
	return widget;
}

void QxCodetipsPreferences::pathEditingFinished(QLineEdit* edit)
{
	if (!edit) edit = qobject_cast<QLineEdit*>(sender());
	String locator = locatorByWidget_->value(edit->parentWidget());
	Ref<StringList, Owner> parts = locator.split("?");
	if (parts->length() == 2) {
		String name = parts->at(1);
		Ref<Assistant> assistant = assistantByStackIndex_->value(stackIndexByLocator_->value(parts->at(0)));
		Path path = String(edit->text().toUtf8().data());
		// debug("QxCodetipsPreferences::pathEditingFinished(): %% = %%\n", name, path);
		if (assistant->resource(name) != path) {
			assistant->setResource(name, path);
			assistant->update();
		}
	}
}

void QxCodetipsPreferences::pathChooser()
{
	QLineEdit* edit = sender()->parent()->findChild<QLineEdit*>();
	QString path =
		QFileDialog::getOpenFileName(
			this,
			QString() /* caption */,
			QDir::currentPath() /* dir */,
			QString() /* filter */,
			0 /* selected filter */
		);
	if (path != "") {
		edit->setText(path);
		pathEditingFinished(edit);
	}
}

} // namespace pacu
