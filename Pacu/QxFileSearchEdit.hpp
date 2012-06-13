#ifndef PACU_QXFILESEARCHEDIT_HPP
#define PACU_QXFILESEARCHEDIT_HPP

#include "UseVide.hpp"
#include "QxFileFinder.hpp"
#include "QxListPopup.hpp"

namespace pacu
{

class QxFileSearchEdit: public QxControl
{
	Q_OBJECT
	
public:
	QxFileSearchEdit(QWidget* parent = 0, int popupAlign = Qt::AlignLeft);
	~QxFileSearchEdit();
	
signals:
	void openFile(QString path);
	
public slots:
	void setCurrentDir(QString path);
	
private slots:
	void lookup(QString prefix);
	void waiting();
	void found(QStringList paths);
	void finished();
	void autoComplete(QString fileName);
	void openFile();
	
private:
	virtual void keyPressEvent(QKeyEvent* event);
	virtual void focusInEvent(QFocusEvent* event);
	virtual void focusOutEvent(QFocusEvent* event);
	
	QxFileFinder* finder_;
	Ref<QxListPopup, Owner> results_;
	QTimer* responseTimer_;
};

} // namespace pacu


#endif // PACU_QXFILESEARCHEDIT_HPP
