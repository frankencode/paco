#ifndef PACU_QXAPPLICATION_HPP
#define PACU_QXAPPLICATION_HPP

#include <QPointer>
#include <QWidget>
#include <QApplication>

namespace pacu
{

class QxMainWindow;

class QxApplication: public QApplication
{
	Q_OBJECT
	
public:
	QxApplication(int& argc, char** argv);
	~QxApplication();
	
	int exec();
	QxMainWindow* mainWindow() const;
	
signals:
	void fileOpen(QString filePath);
	
private:
	bool event(QEvent* event);
	
	QPointer<QxMainWindow> mainWindow_;
};

extern QPointer<QxApplication> qxApp;

} // namespace pacu

#endif // PACU_QXAPPLICATION_HPP
