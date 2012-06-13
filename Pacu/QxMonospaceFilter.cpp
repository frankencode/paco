#include <QFontDatabase>
#include <QDebug> // DEBUG
#include <ftl/utils>
#include <ftl/streams>
#include "UseFtl.hpp"
#include "QxMonospaceFilter.hpp"

namespace pacu
{

QxMonospaceFilter::QxMonospaceFilter(QObject* parent)
	: QThread(parent),
	  stop_(false)
{}

void QxMonospaceFilter::stop()
{
	stop_ = true;
}

void QxMonospaceFilter::run()
{
	QFontDatabase db;
	QStringList input = db.families();
	QStringList output;
	for (int i = 0, n = input.size(); i < n && (!stop_); ++i) {
		QString name = input.at(i);
		if (name.contains("Italic") || name.contains("Bold") || (name == ".LastResort") || (name.contains("Wingdings")))
			;
		else {
			// qDebug() << "name =" << name;
			if (db.isFixedPitch(name)) {
				// have to keep "Regular" to get "Menlo Regular" rendered right
				// if (name.contains("Regular"))
				//	name = name.replace("Regular", "");
				output.append(name);
			}
			// qDebug() << "done.";
		}
	}
	if (!stop_)
		emit finished(output);
}

} // namespace pacu
