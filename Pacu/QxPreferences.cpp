#include <QDebug> // DEBUG
#include <QItemDelegate>
#include <QStringListModel>
#include <QMessageBox>
#include <QFileDialog>
#include <QGridLayout>
#include <QTabWidget>
#include <QListView>
#include <QApplication>
#include <pte/CharcoalDirectory.hpp>
#include <pte/ThemeManager.hpp>
#include <pte/LanguageManager.hpp>
#include <pte/LanguageStack.hpp>
#include <pte/PaletteManager.hpp>
#include "QxVideoTerminal.hpp"
#include "QxThemesModel.hpp"
#include "QxCommandEditor.hpp"
#include "QxEditorPreferences.hpp"
#include "QxTerminalPreferences.hpp"
#include "QxPrintingPreferences.hpp"
#include "QxCodetipsPreferences.hpp"
#include "QxMiscPreferences.hpp"
#include "QxCommandsPreferences.hpp"
#include "QxPreferences.hpp"

namespace pacu
{

QxPreferences::QxPreferences(Ref<CharcoalDirectory> charcoalDirectory, QWidget* parent)
	: QxDialog(parent),
	  charcoalDirectory_(charcoalDirectory),
	  themeManager_(charcoalDirectory->themeManager()),
	  paletteManager_(charcoalDirectory->paletteManager())
{
	setWindowTitle(qApp->applicationName() + " - Preferences");
	#ifdef Q_WS_MAC
	#ifndef QT_MAC_USE_COCOA
	setAttribute(Qt::WA_MacShowFocusRect, false);
	#endif // QT_MAC_USE_COCOA
	#endif // Q_WS_MAC
	
	QGridLayout* grid = new QGridLayout(this);
	{
		tabs_ = new QTabWidget(this);
		tabs_->setElideMode(Qt::ElideNone);
		tabs_->setUsesScrollButtons(false);
		grid->addWidget(tabs_, 0, 0);
		// grid->setMargin(12);
	}
	
	{
		editor_ = new QxEditorPreferences(tabs_);
		tabs_->insertTab(tabs_->count(), editor_, tr("Editor"));
		
		editor_->fontSize_->setRange(8, 32);
		editor_->lineSpacing_->setRange(0, 32);
		editor_->tabWidth_->setRange(2, 32);
		editor_->indentWidth_->setRange(1, 32);
		
		connect(editor_->tabIndentMode_, SIGNAL(toggled(bool)), editor_->indentWidth_, SLOT(setDisabled(bool)));
		editor_->tabIndentMode_->setChecked(true);
		
		connect(editor_->font_, SIGNAL(currentFontChanged(const QFont&)), this, SLOT(updateEditorFontOption(const QFont&)));
		connect(editor_->fontSize_, SIGNAL(valueChanged(int)), this, SLOT(updateEditorIntOption(int)));
		connect(editor_->fontAntialiasing_, SIGNAL(toggled(bool)), this, SLOT(updateEditorBoolOption(bool)));
		connect(editor_->subpixelAntialiasing_, SIGNAL(toggled(bool)), this, SLOT(updateEditorBoolOption(bool)));
		connect(editor_->lineSpacing_, SIGNAL(valueChanged(int)), this, SLOT(updateEditorIntOption(int)));
		connect(editor_->showLineNumbers_, SIGNAL(toggled(bool)), this, SLOT(updateEditorBoolOption(bool)));
		connect(editor_->showWhitespace_, SIGNAL(toggled(bool)), this, SLOT(updateEditorBoolOption(bool)));
		connect(editor_->tabWidth_, SIGNAL(valueChanged(int)), this, SLOT(updateEditorIntOption(int)));
		
		connect(editor_->autoIndent_, SIGNAL(toggled(bool)), this, SIGNAL(editorAutoIndentChanged(bool)));
		connect(editor_->tabIndentMode_, SIGNAL(toggled(bool)), this, SLOT(updateEditorIndent(bool)));
		connect(editor_->indentWidth_, SIGNAL(valueChanged(int)), this, SLOT(updateEditorIndent(int)));
	}
	
	{
		terminal_ = new QxTerminalPreferences;
		tabs_->insertTab(tabs_->count(), terminal_, tr("Terminal"));
		
		terminal_->fontSize_->setRange(8, 32);
		terminal_->lineSpacing_->setRange(0, 32);
		terminal_->numberOfLines_->setRange(100000, 1000000);
		terminal_->numberOfLines_->setSingleStep(100000);
		terminal_->title_->addItems(
			QString(
				"$TITLE,"
				"$FG,"
				"$USER@$HOST,"
				"$USER@$FQHOST,"
				"$USER@$HOST:$CWD,"
				"$FG [$USER@$HOST],"
				"$FG [$USER@$HOST]:$CWD"
			).split(",")
		);
		terminal_->title_->setCurrentIndex(5);
		
		connect(terminal_->font_, SIGNAL(currentFontChanged(const QFont&)), this, SLOT(updateTerminalFontOption(const QFont&)));
		connect(terminal_->fontSize_, SIGNAL(valueChanged(int)), this, SLOT(updateTerminalIntOption(int)));
		connect(terminal_->fontAntialiasing_, SIGNAL(toggled(bool)), this, SLOT(updateTerminalBoolOption(bool)));
		connect(terminal_->subpixelAntialiasing_, SIGNAL(toggled(bool)), this, SLOT(updateTerminalBoolOption(bool)));
		connect(terminal_->lineSpacing_, SIGNAL(valueChanged(int)), this, SLOT(updateTerminalIntOption(int)));
		connect(terminal_->endlessLogging_, SIGNAL(toggled(bool)), this, SLOT(updateTerminalEndlessLogging(bool)));
		connect(terminal_->numberOfLines_, SIGNAL(valueChanged(int)), this, SIGNAL(terminalNumberOfLinesChanged(int)));
		connect(terminal_->endlessLogging_, SIGNAL(toggled(bool)), terminal_->numberOfLines_, SLOT(setDisabled(bool)));
		connect(terminal_->title_, SIGNAL(activated(const QString&)), this, SIGNAL(terminalTitleChanged(const QString&)));
		
		QStringList paletteNames;
		for (int i = 0, n = paletteManager_->numPalette(); i < n; ++i) {
			Ref<Palette> palette = paletteManager_->paletteByIndex(i);
			if (palette->numColors() >= QxVideoTerminal::NumColors) {
				paletteNames << paletteManager_->paletteByIndex(i)->displayName();
				paletteIndices_.append(i);
			}
		}
		
		QStringListModel* model = new QStringListModel(paletteNames, terminal_->palette_);
		terminal_->palette_->setModel(model);
		
		connect(terminal_->palette_, SIGNAL(activated(int)), this, SLOT(terminalPaletteActivated(int)));
	}
	
	{
		printing_ = new QxPrintingPreferences(tabs_);
		tabs_->insertTab(tabs_->count(), printing_, tr("Printing"));
		
		printing_->fontSize_->setRange(8, 32);
		printing_->lineSpacing_->setRange(0, 32);
	}
	
	/*
	{
		// performance HACK to reduce application launch time
		editor_->font_->setDisabled(true);
		terminal_->font_->setDisabled(true);
		printing_->font_->setDisabled(true);
		#ifdef Q_WS_MAC
		// visual HACK
		editor_->font_->setFixedWidth(editor_->font_->sizeHint().width());
		#endif
		#if 0
		editor_->font_->setMinimumContentsLength(20);
		editor_->font_->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
		terminal_->font_->setMinimumContentsLength(20);
		terminal_->font_->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
		printing_->font_->setMinimumContentsLength(20);
		printing_->font_->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
		#endif
		QTimer::singleShot(0, this, SLOT(startFontFilter()));
	}
	*/
	
	{
		themesView_ = new QListView(tabs_);
		themesView_->setViewMode(QListView::IconMode);
		themesView_->setMovement(QListView::Static);
		themesView_->setResizeMode(QListView::Adjust);
		themesView_->setFlow(QListView::LeftToRight);
		// themesView_->setSpacing(10);
		
		tabs_->insertTab(tabs_->count(), themesView_, tr("Theme"));
		
		themesView_->setModel(new QxThemesModel(charcoalDirectory_, themesView_));
		
		connect(
			themesView_->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex&, const QModelIndex&)),
			this, SLOT(themeActivate(const QModelIndex&, const QModelIndex&))
		);
	}
	
	{
		commands_ = new QxCommandsPreferences(tabs_);
		tabs_->insertTab(tabs_->count(), commands_, tr("Commands"));
		
		connect(commands_->add_, SIGNAL(clicked()), this, SLOT(addCommand()));
		connect(commands_->edit_, SIGNAL(clicked()), this, SLOT(editCommand()));
		connect(commands_->del_, SIGNAL(clicked()), this, SLOT(delCommand()));
		connect(commands_->import_, SIGNAL(clicked()), this, SLOT(importConfig()));
		connect(commands_->export_, SIGNAL(clicked()), this, SLOT(exportConfig()));
		
		commandsList_ = new QxCommandsList;
		commandsModel_ = new QxCommandsModel(commandsList_);
		commands_->view_->setModel(commandsModel_);
		commands_->view_->setHeaderHidden(false);
	}
	
	{
		codetips_ = new QxCodetipsPreferences(this, tabs_);
		tabs_->insertTab(tabs_->count(), codetips_, tr("Code Tips"));
	}
	
	{
		misc_ = new QxMiscPreferences(tabs_);
		tabs_->insertTab(tabs_->count(), misc_, tr("Misc"));
		misc_->scrollingSpeed_->setRange(1, 6);
		misc_->scrollingSpeed_->setValue(3);
		
		misc_->kinematicScrolling_->setChecked(true);
		misc_->scrollingSpeed_->setEnabled(false);
		connect(misc_->kinematicScrolling_, SIGNAL(toggled(bool)), misc_->scrollingSpeed_, SLOT(setDisabled(bool)));
		
		connect(misc_->kinematicScrolling_, SIGNAL(toggled(bool)), this, SLOT(updateEditorBoolOption(bool)));
		connect(misc_->kinematicScrolling_, SIGNAL(toggled(bool)), this, SLOT(updateTerminalBoolOption(bool)));
		connect(misc_->scrollingSpeed_, SIGNAL(valueChanged(int)), this, SLOT(updateEditorIntOption(int)));
		connect(misc_->scrollingSpeed_, SIGNAL(valueChanged(int)), this, SLOT(updateTerminalIntOption(int)));
		connect(misc_->scrollingSpeed_, SIGNAL(valueChanged(int)), this, SLOT(setWheelScrollLines(int)));
	}
	
	/*{
		config_ = new QxImportExportPreferences(tabs_);
		tabs_->insertTab(5, config_, tr("Import/Export"));
		
		config_->commands_->setChecked(true);
		connect(config_->import_, SIGNAL(pressed()), this, SLOT(importConfig()));
		connect(config_->export_, SIGNAL(pressed()), this, SLOT(exportConfig()));
	}*/
	
	QFontComboBox::FontFilters filters = QFontComboBox::MonospacedFonts|QFontComboBox::ScalableFonts;
	editor_->font_->setFontFilters(filters);
	terminal_->font_->setFontFilters(filters);
	printing_->font_->setFontFilters(filters);
	
	#ifndef QT_MAC_USE_COCOA
	resize(524, 390); // visual HACK
	#else
	resize(599, 421); // visual HACK
	#endif
}

QxPreferences::~QxPreferences()
{
	if (fontFilter_) {
		if (fontFilter_->isRunning()) {
			fontFilter_->stop();
			fontFilter_->wait();
		}
	}
}

Ref<LanguageManager> QxPreferences::languageManager() const
{
	return charcoalDirectory_->languageManager();
}

Ref<ThemeManager> QxPreferences::themeManager() const
{
	return themeManager_;
}

void QxPreferences::save(QSettings* settings, int selection)
{
	settings->beginGroup("preferences");
	
	if ((selection & Editor) != 0) {
		settings->beginGroup("editor");
		settings->setValue("font", editor_->font_->currentFont().family());
		settings->setValue("fontSize", editor_->fontSize_->value());
		settings->setValue("fontAntialiasing", editor_->fontAntialiasing_->isChecked());
		settings->setValue("speedOverQuality", !editor_->subpixelAntialiasing_->isChecked());
		settings->setValue("lineSpacing", editor_->lineSpacing_->value());
		settings->setValue("showLineNumbers", editor_->showLineNumbers_->isChecked());
		settings->setValue("showWhitespace", editor_->showWhitespace_->isChecked());
		settings->setValue("autoIndent", editor_->autoIndent_->isChecked());
		settings->setValue("tabIndentMode", editor_->tabIndentMode_->isChecked());
		settings->setValue("tabWidth", editor_->tabWidth_->value());
		settings->setValue("indentWidth", editor_->indentWidth_->value());
		settings->endGroup();
	}
	
	if ((selection & Terminal) != 0) {
		settings->beginGroup("terminal");
		settings->setValue("font", terminal_->font_->currentFont().family());
		settings->setValue("fontSize", terminal_->fontSize_->value());
		settings->setValue("fontAntialiasing", terminal_->fontAntialiasing_->isChecked());
		settings->setValue("speedOverQuality", !terminal_->subpixelAntialiasing_->isChecked());
		settings->setValue("lineSpacing", terminal_->lineSpacing_->value());
		settings->setValue("endlessLogging", terminal_->endlessLogging_->isChecked());
		settings->setValue("numberOfLines", terminal_->numberOfLines_->value());
		settings->setValue("palette", terminalPalette()->name());
		settings->setValue("title", terminal_->title_->currentText());
		settings->endGroup();
	}
	
	if ((selection & Printing) != 0) {
		settings->beginGroup("printing");
		settings->setValue("font", printing_->font_->currentFont().family());
		settings->setValue("fontSize", printing_->fontSize_->value());
		settings->setValue("fontAntialiasing", printing_->fontAntialiasing_->isChecked());
		settings->setValue("lineSpacing", printing_->lineSpacing_->value());
		settings->setValue("showLineNumbers", printing_->showLineNumbers_->isChecked());
		settings->setValue("showWhitespace", printing_->showWhitespace_->isChecked());
		settings->setValue("printingPageHeader", printing_->pageHeader_->isChecked());
		settings->setValue("printingPageBorder", printing_->pageBorder_->isChecked());
		settings->endGroup();
	}
	
	if ((selection & Theme) != 0) {
		settings->beginGroup("theme");
		settings->setValue("activeTheme", themeManager_->themeByIndex(themesView_->currentIndex().row())->name());
		settings->endGroup();
	}
	
	if ((selection & Commands) != 0) {
		settings->beginGroup("commands");
		{
			QVariantList list;
			for (int i = 0; i < commandsList_->length(); ++i)
				list.append(QVariant(commandsList_->get(i)->save()));
			settings->setValue("commands", list);
		}
		settings->endGroup();
	}
	
	/*if ((selection & Config) != 0) {
		settings->beginGroup("config");
		settings->setValue("editor", config_->editor_->isChecked());
		settings->setValue("printing", config_->printing_->isChecked());
		settings->setValue("terminal", config_->terminal_->isChecked());
		settings->setValue("commands", config_->commands_->isChecked());
		settings->endGroup();
	}*/
	
	if ((selection & Misc) != 0) {
		settings->beginGroup("misc");
		settings->setValue("kinematicScrolling", misc_->kinematicScrolling_->isChecked());
		settings->setValue("scrollingSpeed", misc_->scrollingSpeed_->value());
		settings->endGroup();
	}
	
	settings->endGroup();
}

void setFontFamily(QFontComboBox* combo, const QVariant& value, const QFont& fallback)
{
	QString family = ((value.type() == QVariant::Font) && (value.canConvert(QVariant::Font))) ? value.value<QFont>().family() : value.toString();
	bool found = false;
	if (family != "") {
		for (int i = 0, n = combo->count(); i < n; ++i) {
			if (combo->itemText(i) == family) {
				combo->setCurrentIndex(i);
				found = true;
				break;
			}
		}
	}
	if (!found)
		combo->setCurrentFont(fallback);
}

void QxPreferences::restore(QSettings* settings, int selection)
{
	blockSignals(true);
	settings->beginGroup("preferences");
	
	Ref<ViewMetrics, Owner> editorDefaults = Edit::defaultMetrics();
	Ref<ViewMetrics, Owner> terminalDefaults = QxVideoTerminal::defaultMetrics();
	
	if ((selection & Editor) != 0) {
		settings->beginGroup("editor");
		setFontFamily(editor_->font_, settings->value("font"), editorDefaults->font_);
		editor_->fontSize_->setValue(settings->value("fontSize", editorDefaults->font_.pixelSize()).toInt());
		editor_->fontAntialiasing_->setChecked(settings->value("fontAntialiasing", editorDefaults->fontAntialiasing_).toBool());
		editor_->subpixelAntialiasing_->setChecked(!settings->value("speedOverQuality", !editorDefaults->subpixelAntialiasing_).toBool());
		editor_->lineSpacing_->setValue(settings->value("lineSpacing", editorDefaults->lineSpacing_).toInt());
		editor_->showLineNumbers_->setChecked(settings->value("showLineNumbers", editorDefaults->showLineNumbers_).toBool());
		editor_->showWhitespace_->setChecked(settings->value("showWhitespace", editorDefaults->showWhitespace_).toBool());
		editor_->autoIndent_->setChecked(settings->value("autoIndent", true).toBool());
		{
			bool on = settings->value("tabIndentMode", true).toBool();
			if (on)
				editor_->tabIndentMode_->setChecked(true);
			else
				editor_->spaceIndentMode_->setChecked(true);
		}
		editor_->tabWidth_->setValue(settings->value("tabWidth", editorDefaults->tabWidth_).toInt());
		editor_->indentWidth_->setValue(settings->value("indentWidth", editorDefaults->tabWidth_).toInt());
		settings->endGroup();
	}
	
	if ((selection & Terminal) != 0) {
		settings->beginGroup("terminal");
		setFontFamily(terminal_->font_, settings->value("font"), terminalDefaults->font_);
		terminal_->fontSize_->setValue(settings->value("fontSize", terminalDefaults->font_.pixelSize()).toInt());
		terminal_->fontAntialiasing_->setChecked(settings->value("fontAntialiasing", terminalDefaults->fontAntialiasing_).toBool());
		terminal_->subpixelAntialiasing_->setChecked(!settings->value("speedOverQuality", !terminalDefaults->subpixelAntialiasing_).toBool());
		terminal_->lineSpacing_->setValue(settings->value("lineSpacing", terminalDefaults->lineSpacing_).toInt());
		terminal_->endlessLogging_->setChecked(settings->value("endlessLogging", true).toBool());
		terminal_->numberOfLines_->setValue(settings->value("numberOfLines", terminal_->numberOfLines_->maximum()).toInt());
		{
			Ref<Palette> palette = paletteManager_->paletteByName(settings->value("palette", "Default").toString());
			for (int i = 0, n = paletteIndices_.length(); i < n; ++i)
				if (paletteManager_->paletteByIndex(paletteIndices_.at(i)) == palette)
					terminal_->palette_->setCurrentIndex(i);
		}
		{
			QString title = settings->value("title", "$FG").toString();
			int titleIndex = terminal_->title_->findText(title);
			if (titleIndex != -1) terminal_->title_->setCurrentIndex(titleIndex);
			else terminal_->title_->setEditText(title);
		}
		settings->endGroup();
	}
	
	if ((selection & Printing) != 0) {
		settings->beginGroup("printing");
		setFontFamily(printing_->font_, settings->value("font"), editorDefaults->font_);
		printing_->fontSize_->setValue(settings->value("fontSize", 10).toInt());
		printing_->fontAntialiasing_->setChecked(settings->value("fontAntialiasing", editorDefaults->fontAntialiasing_).toBool());
		printing_->lineSpacing_->setValue(settings->value("lineSpacing", 1).toInt());
		printing_->showLineNumbers_->setChecked(settings->value("showLineNumbers", editorDefaults->showLineNumbers_).toBool());
		printing_->showWhitespace_->setChecked(settings->value("showWhitespace", false).toBool());
		printing_->pageHeader_->setChecked(settings->value("printingPageHeader", true).toBool());
		printing_->pageBorder_->setChecked(settings->value("printingPageBorder", true).toBool());
		settings->endGroup();
	}
	
	if ((selection & Theme) != 0) {
		settings->beginGroup("theme");
		if (settings->contains("activeTheme"))
			themesView_->setCurrentIndex(themesView_->model()->index(themeManager_->themeIndex(settings->value("activeTheme").toString()), 0));
		else
			themesView_->setCurrentIndex(themesView_->model()->index(themeManager_->defaultThemeIndex(), 0));
		settings->endGroup();
	}
	
	if ((selection & Commands) != 0) {
		settings->beginGroup("commands");
		{
			QVariantList list = settings->value("commands", QVariantList()).toList();
			commandsList_->clear();
			for (int i = 0; i < list.size(); ++i) {
				QxCommand* cmd = new QxCommand(parent());
				connect(cmd, SIGNAL(triggered(QxCommand*)), this, SIGNAL(commandTriggered(QxCommand*)));
				QAction* action = new QAction(parent());
				// parentWidget()->addAction(action);
				cmd->assignAction(action);
				QVariantList al = list.at(i).toList();
				cmd->restore(list.at(i).toList());
				cmd->updateAction();
				commandsList_->append(cmd);
			}
			// commands_->view_->resizeColumnToContents(0);
		}
		settings->endGroup();
	}
	
	/*if ((selection & Config) != 0) {
		settings->beginGroup("config");
		config_->editor_->setChecked(settings->value("editor", false).toBool());
		config_->printing_->setChecked(settings->value("printing", false).toBool());
		config_->terminal_->setChecked(settings->value("terminal", false).toBool());
		config_->commands_->setChecked(settings->value("commands", true).toBool());
		settings->endGroup();
	}*/
	
	if ((selection & Misc) != 0) {
		settings->beginGroup("misc");
		misc_->kinematicScrolling_->setChecked(settings->value("kinematicScrolling", false).toBool());
		misc_->scrollingSpeed_->setValue(settings->value("scrollingSpeed", qApp->wheelScrollLines()).toInt());
		settings->endGroup();
	}
	
	settings->endGroup();
	blockSignals(false);
	
	updateEditorMetrics();
	updateTerminalMetrics();
	emit commandsChanged(commandsList_);
}

Ref<ViewMetrics, Owner> QxPreferences::editorMetrics() const
{
	Ref<ViewMetrics, Owner> metrics = Edit::defaultMetrics();
	metrics->font_ = editor_->font_->currentFont();
	metrics->font_.setPixelSize(editor_->fontSize_->value());
	metrics->fontAntialiasing_ = editor_->fontAntialiasing_->isChecked();
	metrics->subpixelAntialiasing_ = editor_->subpixelAntialiasing_->isChecked();
	metrics->lineSpacing_ = editor_->lineSpacing_->value();
	metrics->showLineNumbers_ = editor_->showLineNumbers_->isChecked();
	metrics->showWhitespace_ = editor_->showWhitespace_->isChecked();
	metrics->tabWidth_ = editor_->tabWidth_->value();
	metrics->scrollingSpeed_ = misc_->scrollingSpeed_->value();
	metrics->kinematicScrolling_ = misc_->kinematicScrolling_->isChecked();
	return metrics;
}

QString QxPreferences::editorIndent() const {
	return editor_->tabIndentMode_->isChecked() ? QString("\t") : QString(editor_->indentWidth_->value(), ' ');
}

bool QxPreferences::editorAutoIndent() const { return editor_->autoIndent_->isChecked(); }

Ref<ViewMetrics, Owner> QxPreferences::printingMetrics() const
{
	Ref<ViewMetrics, Owner> metrics = editorMetrics();
	metrics->font_ = printing_->font_->currentFont();
	metrics->font_.setPixelSize(printing_->fontSize_->value());
	metrics->fontAntialiasing_ = printing_->fontAntialiasing_->isChecked();
	metrics->lineSpacing_ = printing_->lineSpacing_->value();
	metrics->showLineNumbers_ = printing_->showLineNumbers_->isChecked();
	metrics->showWhitespace_ = printing_->showWhitespace_->isChecked();
	return metrics;
}

bool QxPreferences::printingPageHeader() const { return printing_->pageHeader_->isChecked(); }
bool QxPreferences::printingPageBorder() const { return printing_->pageBorder_->isChecked(); }

Ref<ViewMetrics, Owner> QxPreferences::terminalMetrics() const
{
	Ref<ViewMetrics, Owner> metrics = QxVideoTerminal::defaultMetrics();
	metrics->font_ = terminal_->font_->currentFont();
	metrics->font_.setPixelSize(terminal_->fontSize_->value());
	metrics->fontAntialiasing_ = terminal_->fontAntialiasing_->isChecked();
	metrics->subpixelAntialiasing_ = terminal_->subpixelAntialiasing_->isChecked();
	metrics->lineSpacing_ = terminal_->lineSpacing_->value();
	metrics->scrollingSpeed_ = misc_->scrollingSpeed_->value();
	metrics->kinematicScrolling_ = misc_->kinematicScrolling_->isChecked();
	return metrics;
}

Ref<Palette> QxPreferences::terminalPalette() const
{
	return paletteManager_->paletteByIndex(paletteIndices_.at(terminal_->palette_->currentIndex()));
}

QString QxPreferences::terminalTitle() const { return terminal_->title_->currentText(); }

void QxPreferences::setup(Edit* edit)
{
	if (edit->document())
		edit->document()->setHistoryEnabled(true);
	edit->setMetrics(editorMetrics());
	edit->setAutoIndent(editorAutoIndent());
	edit->setIndent(editorIndent());
	connect(this, SIGNAL(editorMetricsChanged(Ref<ViewMetrics>)), edit, SLOT(setMetrics(Ref<ViewMetrics>)));
	connect(this, SIGNAL(editorAutoIndentChanged(bool)), edit, SLOT(setAutoIndent(bool)));
	connect(this, SIGNAL(editorIndentChanged(QString)), edit, SLOT(setIndent(QString)));
	
	edit->setColors(themeManager_->activeTheme()->viewColors());
	connect(themeManager_, SIGNAL(highlightersYield()),                edit, SLOT(highlighterYield()));
	connect(themeManager_, SIGNAL(viewColorsChanged(Ref<ViewColors>)), edit, SLOT(setColors(Ref<ViewColors>)));
	connect(themeManager_, SIGNAL(highlightersRestart()),              edit, SLOT(highlighterRestart()));
	connect(themeManager_, SIGNAL(highlightersSync()),                 edit, SLOT(highlighterSync()));
	connect(themeManager_, SIGNAL(highlightersResume()),               edit, SLOT(highlighterResume()));
}

void QxPreferences::showOrHide()
{
	setVisible(!isVisible());
	if (isVisible())
		tabs_->widget(tabs_->currentIndex())->setFocus();
}

void QxPreferences::showTabEditor()
{
	tabs_->setCurrentWidget(editor_);
	exec();
}

void QxPreferences::showTabCommands()
{
	tabs_->setCurrentWidget(commands_);
	exec();
}

void QxPreferences::startFontFilter()
{
	fontFilter_ = new QxMonospaceFilter(this);
	connect(fontFilter_, SIGNAL(finished(QStringList)), this, SLOT(fontFilterFinished(QStringList)));
	fontFilter_->start();
}

void QxPreferences::fontFilterFinished(QStringList fontNames)
{
	blockSignals(true);
	Ref<ViewMetrics, Owner> editorDefaults = Edit::defaultMetrics();
	Ref<ViewMetrics, Owner> terminalDefaults = Edit::defaultMetrics();
	fontFilterActivate(editor_->font_, editorDefaults->font_.family(), fontNames);
	fontFilterActivate(terminal_->font_, terminalDefaults->font_.family(), fontNames);
	fontFilterActivate(printing_->font_, editorDefaults->font_.family(), fontNames);
	blockSignals(false);
}

void QxPreferences::setWheelScrollLines(int value) { qApp->setWheelScrollLines(value); }

void QxPreferences::updateEditorFontOption(const QFont&) { updateEditorMetrics(); }
void QxPreferences::updateEditorIntOption(int) { updateEditorMetrics(); }
void QxPreferences::updateEditorBoolOption(bool) { updateEditorMetrics(); }
void QxPreferences::updateEditorIndent(bool) { editorIndentChanged(editorIndent()); }
void QxPreferences::updateEditorIndent(int) { editorIndentChanged(editorIndent()); }

void QxPreferences::updateTerminalFontOption(const QFont&) { updateTerminalMetrics(); }
void QxPreferences::updateTerminalIntOption(int) { updateTerminalMetrics();  }
void QxPreferences::updateTerminalBoolOption(bool) { updateTerminalMetrics(); }
void QxPreferences::updateTerminalEndlessLogging(bool on) {
	emit terminalNumberOfLinesChanged(on ? terminal_->numberOfLines_->value() : intMax);
}
void QxPreferences::terminalPaletteActivated(int index) {
	emit terminalPaletteChanged(terminalPalette());
	emit terminalPaletteChanged();
}

void QxPreferences::themeActivate(const QModelIndex& index, const QModelIndex& oldIndex) {
	if (isVisible())
		themeManager_->activateThemeByIndex(index.row());
	else
		themeManager_->activateThemeByIndexSilent(index.row());
}

void QxPreferences::openCommand(bool addNew)
{
	QxCommandEditor* editor = new QxCommandEditor(this);
	{
		Ref<LanguageStack> stack = charcoalDirectory_->languageManager()->stackByFileName("dummy.sh");
		if (stack) {
			editor->scriptEdit()->document()->setHighlighter(
				new Highlighter(stack, editor->scriptEdit()->document())
			);
		}
	}
	setup(editor->scriptEdit());
	
	if (addNew) {
		QxCommand* cmd = new QxCommand(parent());
		connect(cmd, SIGNAL(triggered(QxCommand*)), this, SIGNAL(commandTriggered(QxCommand*)));
		QAction* action = new QAction(parent());
		// parentWidget()->addAction(action);
		cmd->assignAction(action);
		editor->setCommand(cmd);
	}
	else {
		QModelIndex index = commands_->view_->currentIndex();
		if (!index.isValid()) return;
		editor->setCommand(commandsList_->get(index.row()));
	}
	while (editor->exec() == QDialog::Accepted) {
		QxCommand* cmd = editor->command();
		int i = 0;
		while (i < commandsList_->length()) {
			if (cmd->keysToString("+", true) < commandsList_->get(i)->keysToString("+", true)) break;
			else if ((cmd->keys_ == commandsList_->get(i)->keys_) && (addNew || (i != commands_->view_->currentIndex().row()))) {
				i = -1;
				break;
			}
			++i;
		}
		if (i != -1) {
			cmd->updateAction();
			if (addNew) {
				commandsList_->insert(i, cmd);
				addNew = false;
			}
			// commands_->view_->resizeColumnToContents(0);
			emit commandsChanged(commandsList_);
			break;
		}
		else {
			int choice =
			QMessageBox::warning(
				this,
				qApp->applicationName(),
				QString("Keyboard shortcut %1 already in use.").arg(cmd->keysToString()),
				addNew ? QMessageBox::Ok|QMessageBox::Cancel : QMessageBox::Ok,
				QMessageBox::Ok
			);
			if (choice == QMessageBox::Cancel)
				break;
		}
	}
	delete editor;
}

void QxPreferences::addCommand()
{
	openCommand(true);
}

void QxPreferences::editCommand()
{
	openCommand(false);
}

void QxPreferences::delCommand()
{
	QModelIndex index = commands_->view_->currentIndex();
	if (index.isValid()) {
		int y = index.row();
		commandsList_->remove(y);
		commands_->view_->setCurrentIndex(commandsModel_->index(y, 0)); // workaround HACK
		emit commandsChanged(commandsList_);
	}
}

void QxPreferences::importConfig()
{
	QString filePath =
		QFileDialog::getOpenFileName(
			this,
			QString() /* caption */,
			QDir::currentPath() /* dir */,
			tr("Config files (*.conf)") /* filter */,
			0 /* selected filter */,
			QFileDialog::DontUseSheet /* options, e.g. QFileDialog::DontUseNativeDialog */
		);
	
	if ((filePath != QString()) && (QFileInfo(filePath).exists())) {
		QSettings settings(filePath, QSettings::IniFormat);
		restore(&settings, configSelection());
	}
}

void QxPreferences::exportConfig()
{
	QString filePath =
		QFileDialog::getSaveFileName(
			this,
			QString() /* caption */,
			QDir::currentPath() /* dir */,
			tr("Config files (*.conf)") /* filter */,
			0 /* selected filter */,
			QFileDialog::DontUseSheet /* options, e.g. QFileDialog::DontUseNativeDialog */
		);
	
	if (filePath != QString()) {
		QSettings settings(filePath, QSettings::IniFormat);
		save(&settings, configSelection());
	}
}

void QxPreferences::fontFilterActivate(QFontComboBox* combo, QString defaultFamily, QStringList fontNames)
{
	combo->blockSignals(true);
	QString fontSaved = combo->currentFont().family();
	qobject_cast<QStringListModel*>(combo->model())->setStringList(fontNames);
	combo->setDisabled(false);
	int i = fontNames.indexOf(fontSaved);
	if (i == -1) {
		i = fontNames.indexOf(defaultFamily);
		if (i == -1)
			i = 0;
	}
	combo->setCurrentIndex(i);
	combo->blockSignals(false);
}

int QxPreferences::configSelection() const
{
	return Commands;
	/*return
		(config_->editor_->isChecked() * Editor) |
		(config_->printing_->isChecked() * Printing) |
		(config_->terminal_->isChecked() * Terminal) |
		(config_->commands_->isChecked() * Commands);*/
}

void QxPreferences::updateEditorMetrics()
{
	if (signalsBlocked()) return;
	emit editorMetricsChanged(editorMetrics());
}

void QxPreferences::updateTerminalMetrics()
{
	if (signalsBlocked()) return;
	emit terminalMetricsChanged(terminalMetrics());
}

/*void QxPreferences::resizeEvent(QResizeEvent* event)
{
	qDebug() << "QxPreferences::resizeEvent(): size =" << size();
}*/

} // namespace pacu
