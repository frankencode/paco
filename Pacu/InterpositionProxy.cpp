#include <fcntl.h>
#include "QxMainWindow.hpp"
#include "InterpositionProxy.hpp"

namespace pacu
{

InterpositionProxy::InterpositionProxy(QxMainWindow* mainWin)
	: mainWin_(mainWin)
{}

String InterpositionProxy::redirectOpen(String path)
{
	return (mainWin_) ? mainWin_->openRedirect(path) : path;
		/* NB: We don't need to synchronise with the GUI thread here,
		 * because the GUI thread is invoking the client, which in turn
		 * connects with this server.
		 */
}

} // namespace pacu
