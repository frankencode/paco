#ifndef PACU_QXFINDTEXT_HPP
#define PACU_QXFINDTEXT_HPP

#include <QPointer>
#include <QComboBox>
#include <QPushButton>
#include <QCheckBox>
#include <pte/Document.hpp>
#include <pte/View.hpp>
#include "UseVide.hpp"
#include "UsePte.hpp"
#include "UseFtl.hpp"
#include "QxFileOpenRequest.hpp"

class QSettings;
class QxPreferences;

namespace ftl {
	class Semaphore;
}

namespace pacu
{

class QxMatchView;
class QxVideoTerminal;

class QxFindText: public QxControl
{
	Q_OBJECT
	
public:
	QxFindText(QxPreferences* preferences, QWidget* parent);
	
	void saveState(QSettings* settings);
	void restoreState(const QSettings* settings);
	
	enum Scope { CurrentFile, CurrentDirectory, Terminal };
	void setScope(int scope);
	
	void selectPatternText();
	
signals:
	void escape();
	void showFindStatusChanged(bool on);
	void gotoMatch(Ref<Document> document, int my, int mx0, int mx1);
	void openFile(Ref<QxFileOpenRequest, Owner> request);
	void showMessage(QString messge);
	
public slots:
	void setTextView(View* view);
	void setVideoTerminal(QxVideoTerminal* vt);
	void setViewColors(Ref<ViewColors> colors);
	void setViewMetrics(Ref<ViewMetrics> metrics);
	void terminalPaletteChanged();
	void updateMatchStatus(Ref<Delta, Owner> delta);
	void updateMatchStatus();
	
	void openLink();
	void firstLink();
	void lastLink();
	void previousLink();
	void nextLink();
	
private slots:
	void patternChanged(const QString&);
	void scopeChanged(int scope);
	void find();
	void replace();
	
private:
	void findAndReplace(bool replace);
	void dismiss();
	void updateStatusMessage(bool afterMatch = false);
	void showEvent(QShowEvent* event);
	void hideEvent(QHideEvent* event);
	
	QPointer<QxPreferences> preferences_;
	
	QComboBox* pattern_;
	QPushButton* findButton_;
	QCheckBox* useRegex_;
	QCheckBox* caseSensitive_;
	QCheckBox* wholeWord_;
	QCheckBox* indentation_;
	QComboBox* replacement_;
	QPushButton* replaceButton_;
	QComboBox* scope_;
	
	QxMatchView* matchView_;
	QPointer<View> textView_;
	QPointer<QxVideoTerminal> vt_;
	
	Ref<MatchStatus, Owner> matchStatus_;
	Ref<Document, Owner> matchStatusDocument_;
	int scopeSaved_;
	QStringList paths_;
	
	Ref<Semaphore, Owner> findAndReplaceIsRunning_;
};

} // namespace pacu

#endif // PACU_QXFINDTEXT_HPP
