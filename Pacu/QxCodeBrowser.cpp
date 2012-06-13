#include <QDebug> // DEBUG
#include <QStyle>
// #include <QHeaderView>
#ifdef Q_WS_MAC
#include <QMacStyle>
#include <QProxyStyle>
#endif
#include <QLayout>
#include <QDockWidget>
#include <QStackedLayout>
#include <QTreeView>
#include <pte/Edit.hpp>
#include <pte/LanguageStack.hpp>
#include <pte/InsightParser.hpp>
#include <pte/InsightModel.hpp>
#include "QxStatusBar.hpp"
#include "QxCodeBrowser.hpp"

namespace pacu
{

QxCodeBrowser::QxCodeBrowser(QWidget* parent)
	: QWidget(parent)
{
	panelHead_ = new QxControl(this, new QxVisual(styleManager()->style("codeBrowserPanelHead")));
	reloadButton_ = new QxControl(this, new QxVisual(styleManager()->style("codeBrowserReloadButton")));
	reloadButton_->setMode(QxControl::TouchMode);
	connect(reloadButton_, SIGNAL(pressed()), this, SLOT(reload()));
	
	insightStack_ = new QStackedLayout;
	statusBar_ = new QxStatusBar(this);
	noInsightView_ = newInsightView();
	
	QxControl* navBar = new QxControl(this, new QxVisual(styleManager()->style("codeBrowserNavCarrier")));
	{
		QHBoxLayout* row = new QHBoxLayout;
		row->setSpacing(0);
		row->setMargin(0);
		row->addWidget(panelHead_);
		row->addStretch();
		row->addWidget(reloadButton_);
		navBar->setLayout(row);
	}
	
	QDockWidget* dock = qobject_cast<QDockWidget*>(parent);
	if (dock) {
		dock->setTitleBarWidget(navBar);
		dock->setWidget(this);
	}
	
	QVBoxLayout* col = new QVBoxLayout;
	col->setSpacing(0);
	col->setMargin(0);
	if (!dock)
		col->addWidget(navBar);
	col->addLayout(insightStack_);
	col->addWidget(styleManager()->hl(this));
	col->addWidget(statusBar_);
	setLayout(col);
}

void QxCodeBrowser::display(Ref<Document> document, bool reload)
{
	bool hasInsight = document->highlighter()->languageStack()->insightParser();
	// qDebug() << "QxCodeBrowser::display(): document->filePath(), hasInsight =" << document->filePath() << "," << hasInsight;
	if (!hasInsight) {
		insightStack_->setCurrentWidget(noInsightView_);
		panelHead_->visual()->setText("<unavailable>");
		return;
	}
	
	InsightModel* insightModel = document->insightModel();
	QTreeView* insightView = 0;
	
	if (insightModel) {
		for (int i = 0, n = insightStack_->count(); i < n; ++i) {
			QTreeView* view = qobject_cast<QTreeView*>(insightStack_->widget(i));
			if (view->model() == insightModel) {
				insightView = view;
				break;
			}
		}
	}
	
	if (!insightView) insightView = newInsightView();
	
	if (!insightModel) {
		insightModel = new InsightModel(document);
		document->setInsightModel(insightModel);
		insightView->setModel(insightModel);
	}
	
	insightStack_->setCurrentWidget(insightView);
	
	refresh(reload);
}

void QxCodeBrowser::refresh(bool reload)
{
	QTreeView* insightView = qobject_cast<QTreeView*>(insightStack_->currentWidget());
	if (!insightView) return;
	InsightModel* insightModel = qobject_cast<InsightModel*>(insightView->model());
	if (!insightModel) return;
	
	if (reload)
		insightModel->reload();
	
	QModelIndex rootIndex = insightModel->index(0, 0, QModelIndex());
	QString moduleName = "";
	if (rootIndex.isValid()) {
		if (insightView->rootIndex() != rootIndex) {
			insightView->setRootIndex(rootIndex);
			insightView->expandToDepth(0);
		}
		moduleName = insightModel->data(rootIndex).toString();
	}
	
	if (moduleName == "")
		moduleName = "<global>";
	panelHead_->visual()->setText(moduleName);
}

void QxCodeBrowser::reload()
{
	refresh(true);
}

QTreeView* QxCodeBrowser::newInsightView()
{
	QTreeView* view = new QTreeView(this);
	connect(view, SIGNAL(activated(const QModelIndex&)), this, SLOT(gotoEntity(const QModelIndex&)));
	view->setHeaderHidden(true);
	view->setFrameStyle(QFrame::NoFrame);
	view->setLineWidth(0);
	view->setMidLineWidth(0);
	view->setUniformRowHeights(true); // performance HACK
	{
		QPalette pal = view->palette();
		pal.setColor(QPalette::Base, styleManager()->color("codeBrowserInsightViewBgColor"));
		view->setPalette(pal);
	}
	#ifdef Q_WS_MAC
	{
		QProxyStyle* proxyStyle = qobject_cast<QProxyStyle*>(style());
		QMacStyle* macStyle = qobject_cast<QMacStyle*>((proxyStyle) ? proxyStyle->baseStyle() : style());
		if (macStyle) {
			macStyle->setFocusRectPolicy(view, QMacStyle::FocusDisabled);
		}
	}
	#endif
	insightStack_->addWidget(view);
	return view;
}

void QxCodeBrowser::gotoEntity(const QModelIndex& index)
{
	QTreeView* view = qobject_cast<QTreeView*>(insightStack_->currentWidget());
	if (view == noInsightView_) return;
	InsightModel* model = qobject_cast<InsightModel*>(view->model());
	Ref<Document> document = model->document();
	InsightTree* tree = static_cast<InsightTree*>(index.internalPointer());
	int y = tree->yHint_;
	if (y >= document->numberOfLines()) y = document->numberOfLines() - 1;
	int y0 = y;
	bool found = false;
	for (int n = 50; (n > 0) && (y < document->numberOfLines()); --n) {
		if (document->copySpan(y, 0, intMax).contains(tree->identifier_)) {
			emit gotoLine(y);
			found = true;
			break;
		}
		++y;
	}
	if (!found) {
		y = y0;
		for (int n = 50; (n > 0) && (y > 0); --n) {
			--y;
			if (document->copySpan(y, 0, intMax).contains(tree->identifier_)) {
				emit gotoLine(y);
				break;
			}
		}
	}
}

} // namespace pacu
