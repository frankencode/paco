#ifndef PTE_CHARCOALEXCEPTION_HPP
#define PTE_CHARCOALEXCEPTION_HPP

#include <QString>
#include "UseFtl.hpp"

namespace pte
{

class CharcoalException: public StdException
{
public:
	CharcoalException(QString message)
		: message_(message),
		  messageUtf8_(message_.toUtf8())
	{}
	
	CharcoalException(QString path, int line, QString message)
		: message_(QString("%1:%2: %3").arg(addExt(path)).arg(line).arg(message)),
		  messageUtf8_(message_.toUtf8())
	{}
	
	CharcoalException(QString path, int line, int column, QString message)
		: message_(QString("%1:%2:%3: %4").arg(addExt(path)).arg(line).arg(column).arg(message)),
		  messageUtf8_(message_.toUtf8())
	{}
	
	~CharcoalException() throw()
	{}
	
	// workaround HACK
	static QString addExt(QString path) { return (path.contains(".js")) ? path : path + ".js"; }
	
	QString message() const { return message_; }
	const char* what() const throw() { return messageUtf8_; }
	
private:
	QString message_;
	QByteArray messageUtf8_;
};

} // namespace pte

#endif // PTE_CHARCOALEXCEPTION_HPP
