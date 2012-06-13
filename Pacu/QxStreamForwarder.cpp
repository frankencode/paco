#include "QxStreamForwarder.hpp"

namespace pacu
{

QxStreamForwarder::QxStreamForwarder(Ref<Stream> source, int ioUnit, QObject* parent)
	: QThread(parent),
	  source_(source),
	  readyRead_(new Semaphore),
	  bufSize_(ioUnit),
	  bufFill_(0),
	  buf_(new char[bufSize_])
{}

QxStreamForwarder::~QxStreamForwarder()
{
	delete[] buf_;
	buf_ = 0;
}

Ref<Semaphore> QxStreamForwarder::readyRead() const { return readyRead_; }

void QxStreamForwarder::run()
{
	while (true) {
		bufFill_ = source_->readAvail(buf_, bufSize_);
		if (bufFill_ <= 0) break;
		readyRead_->acquire();
		// print("QxStreamForwarder::run(): bufFill = %%\n", bufFill_);
		emit forward(QByteArray(buf_, bufFill_));
	}
	emit eoi();
}

} // namespace pacu
