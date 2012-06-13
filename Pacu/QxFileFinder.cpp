#include <QDebug> // DEBUG
#include <QDir>
#include "QxFileFinder.hpp"

namespace pacu
{

QxFileFinder::QxFileFinder(QObject* parent)
	: QThread(parent)
{}

void QxFileFinder::startSearch(QString dirPath, QString pattern, int maxResults)
{
	if (pattern.length() == 0) return;
	dirPath_ = dirPath;
	pattern_.clear();
	pattern_ << (pattern + "*");
	numResults_ = 0;
	maxResults_= maxResults;
	stop_ = false;
	results_.clear();
	depth_ = 0;
	start();
}

void QxFileFinder::stopSearch()
{
	stop_ = true;
}

void QxFileFinder::run()
{
	search(dirPath_);
	if (!stop_)
		emit found(results_);
}

void QxFileFinder::search(QString dirPath)
{
	QDir dir(dirPath);
	QStringList names = dir.entryList(pattern_, QDir::Files|QDir::Readable|QDir::Hidden, QDir::Name);
	for (int i = 0; (i < names.count()) && (numResults_ < maxResults_) && (!stop_); ++i) {
		results_.append(dir.filePath(names.at(i)));
		++numResults_;
	}
	if ((numResults_ < maxResults_) && (!stop_)) {
		++depth_;
		QStringList names = dir.entryList(QStringList(), QDir::Dirs|QDir::NoDotAndDotDot, QDir::Name);
		for (int i = 0; (i < names.count()) && (numResults_ < maxResults_) && (!stop_); ++i)
			search(dir.filePath(names.at(i)));
		--depth_;
	}
}

} // namespace pacu
