#ifndef PACU_QXPREFERENCES_HPP
#define PACU_QXPREFERENCES_HPP

#include <QPointer>
#include <pte/Edit.hpp>
#include <pte/Palette.hpp>
#include "UsePte.hpp"
#include "QxDialog.hpp"
#include "QxMonospaceFilter.hpp"
#include "QxCommandsModel.hpp"

class QSettings;
class QTabWidget;
class QListView;
class QFontComboBox;

namespace pte
{

class CharcoalDirectory;
class LanguageManager;
class ThemeManager;
class PaletteManager;

} // namespace pte

namespace pacu
{

class QxEditorPreferences;
class QxTerminalPreferences;
class QxPrintingPreferences;
class QxCommandsPreferences;
class QxCodetipsPreferences;
class QxMiscPreferences;

typedef CustomList<QxCommand*> QxCommandsList;

class QxPreferences: public QxDialog
{
	Q_OBJECT
	
public:
	QxPreferences(Ref<CharcoalDirectory> charcoalDirectory, QWidget* parent = 0);
	~QxPreferences();
	
	Ref<LanguageManager> languageManager() const;
	Ref<ThemeManager> themeManager() const;
	
	enum Selection {
		Editor = 1, Printing = 2, Terminal = 4, Commands = 16, Misc = 32, Theme = 64,
		All = Editor|Printing|Terminal|Commands|Misc|Theme
	};
	
	void save(QSettings* settings, int selection = All);
	void restore(QSettings* settings, int selection = All);
	
	Ref<ViewMetrics, Owner> editorMetrics() const;
	bool editorAutoIndent() const;
	QString editorIndent() const;
	
	Ref<ViewMetrics, Owner> printingMetrics() const;
	bool printingPageHeader() const;
	bool printingPageBorder() const;
	
	Ref<ViewMetrics, Owner> terminalMetrics() const;
	Ref<Palette> terminalPalette() const;
	QString terminalTitle() const;
	
	void setup(Edit* edit);
	
signals:
	void editorMetricsChanged(Ref<ViewMetrics> metrics);
	void editorAutoIndentChanged(bool on);
	void editorIndentChanged(QString indent);
	void terminalMetricsChanged(Ref<ViewMetrics> metrics);
	void terminalNumberOfLinesChanged(int value);
	void terminalPaletteChanged(Ref<Palette> palette);
	void terminalPaletteChanged();
	void terminalTitleChanged(const QString& title);
	
	void commandTriggered(QxCommand* cmd);
	void commandsChanged(Ref<QxCommandsList> commands);
	
public slots:
	void showOrHide();
	void showTabEditor();
	void showTabCommands();
	
private slots:
	void startFontFilter();
	void fontFilterFinished(QStringList fontNames);
	
	void setWheelScrollLines(int value);
	
	void updateEditorFontOption(const QFont&);
	void updateEditorIntOption(int);
	void updateEditorBoolOption(bool);
	void updateEditorIndent(bool);
	void updateEditorIndent(int);
	
	void updateTerminalFontOption(const QFont&);
	void updateTerminalIntOption(int);
	void updateTerminalBoolOption(bool);
	void updateTerminalEndlessLogging(bool on);
	void terminalPaletteActivated(int);
	
	void themeActivate(const QModelIndex& index, const QModelIndex& oldIndex);
	
	void openCommand(bool addNew);
	void addCommand();
	void editCommand();
	void delCommand();
	
	void importConfig();
	void exportConfig();
	
private:
	void fontFilterActivate(QFontComboBox* combo, QString defaultFamily, QStringList fontNames);
	int configSelection() const;
	
	void updateEditorMetrics();
	void updateTerminalMetrics();
	
	// virtual void resizeEvent(QResizeEvent* event);
	
	QTabWidget* tabs_;
	QPointer<QxEditorPreferences> editor_;
	QPointer<QxTerminalPreferences> terminal_;
	QPointer<QxPrintingPreferences> printing_;
	QPointer<QxCodetipsPreferences> codetips_;
	QPointer<QxMiscPreferences> misc_;
	QPointer<QListView> themesView_;
	QPointer<QxCommandsPreferences> commands_;
	// QPointer<QxImportExportPreferences> config_;
	
	Ref<CharcoalDirectory, Owner> charcoalDirectory_;
	Ref<ThemeManager, Owner> themeManager_;
	Ref<PaletteManager, Owner> paletteManager_;
	
	QPointer<QxMonospaceFilter> fontFilter_;
	
	Ref<QxCommandsList, Owner> commandsList_;
	Ref<QxCommandsModel, Owner> commandsModel_;
	QList<int> paletteIndices_;
};

} // namespace pacu

#endif // PACU_QXPREFERENCES_HPP
