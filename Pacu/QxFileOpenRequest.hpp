#ifndef PACU_QXFILEOPENREQUEST_HPP
#define PACU_QXFILEOPENREQUEST_HPP

#include <QUrl>
#include <QString>
#include "UseFtl.hpp"
#include "UsePte.hpp"

namespace pte {
	class Document;
	class Edit;
} // namespace pte


namespace pacu
{

class QxTabBar;

class QxFileOpenRequest: public Instance
{
public:
	enum Mode { Interactive = 1, CurrentTabWidget = 2 };
	
	QxFileOpenRequest(int mode);
	QxFileOpenRequest(int mode, QString path);
	QxFileOpenRequest(int mode, Document* document);
	QxFileOpenRequest(int mode, QxTabBar* tabBar, int tabIndex, QUrl url);
	
	int mode_;
	QxTabBar* tabBar_;
	int tabIndex_;
	QUrl url_;
	bool acceptBinary_;
	bool failed_;
	bool isBinary_;
	Ref<Document, Owner> document_;
	Edit* edit_;
	QString failureHint_;
};

} // namespace pacu

#endif // PACU_QXFILEOPENREQUEST_HPP
