#ifndef PACU_QXSTREAMFORWARDER_HPP
#define PACU_QXSTREAMFORWARDER_HPP

#include <QtCore>
#include "UseFtl.hpp"
#include <ftl/Semaphore.hpp>
#include <ftl/Stream.hpp>

namespace pacu
{

class QxStreamForwarder: public QThread
{
	Q_OBJECT
	
public:
	QxStreamForwarder(Ref<Stream> source, int ioUnit, QObject* parent = 0);
	~QxStreamForwarder();
	
	Ref<Semaphore> readyRead() const;
	
signals:
	void forward(const QByteArray& data);
	void eoi();
	
private:
	void run();
	void commit();
	
	Ref<Stream, Owner> source_;
	Ref<Semaphore, Owner> readyRead_;
	int bufSize_, bufFill_;
	char* buf_;
};

} // namespace pacu

#endif // PACU_QXSTREAMFORWARDER_HPP
