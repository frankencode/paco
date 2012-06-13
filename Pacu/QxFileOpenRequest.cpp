#include <pte/Document.hpp>
#include "QxTabBar.hpp"
#include "QxFileOpenRequest.hpp"

namespace pacu
{

QxFileOpenRequest::QxFileOpenRequest(int mode)
	: mode_(mode),
	  tabBar_(0),
	  tabIndex_(-1),
	  acceptBinary_(true),
	  failed_(false),
	  isBinary_(false),
	  edit_(0)
{}

QxFileOpenRequest::QxFileOpenRequest(int mode, QString path)
	: mode_(mode),
	  tabBar_(0),
	  tabIndex_(-1),
	  url_(QUrl::fromLocalFile(path)),
	  acceptBinary_(true),
	  failed_(false),
	  isBinary_(false),
	  edit_(0)
{}

QxFileOpenRequest::QxFileOpenRequest(int mode, Document* document)
	: mode_(mode),
	  tabBar_(0),
	  tabIndex_(-1),
	  url_(QUrl::fromLocalFile(document->filePath())),
	  acceptBinary_(true),
	  failed_(false),
	  isBinary_(false),
	  document_(document),
	  edit_(0)
{}

QxFileOpenRequest::QxFileOpenRequest(int mode, QxTabBar* tabBar, int tabIndex, QUrl url)
	: mode_(mode),
	  tabBar_(tabBar),
	  tabIndex_(tabIndex),
	  url_(url),
	  acceptBinary_(true),
	  failed_(false),
	  isBinary_(false),
	  edit_(0)
{}

} // namespace pacu
