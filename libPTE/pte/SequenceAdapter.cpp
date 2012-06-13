#include <QDebug> // DEBUG
#include "Document.hpp"
#include "SequenceAdapter.hpp"

namespace pte
{

SequenceAdapter::SequenceAdapter(Ref<Document> document, bool synchronize)
	: highlighter_(synchronize ? document->highlighter() : 0),
	  cache_(document->cache()),
	  k1_(intMax),
	  y_(0), ci_(-1),
	  i0_(0), i1_(0),
	  nl_(false)
{
	Ref<Line> line = cache_->get(0);
	line->i0Hint_ = 0;
	chunkList_ = line->chunkList_;
	if (chunkList_->length() > 0) {
		ci_ = 0;
		chunk_ = chunkList_->get(0);
		i1_ = chunk_->text_.length();
		nl_ = (chunkList_->length() == 1);
	}
}

SequenceAdapter::SequenceAdapter(SequenceAdapter* parent, int k1)
	: highlighter_(parent->highlighter_),
	  cache_(parent->cache_),
	  chunkList_(parent->chunkList_),
	  chunk_(parent->chunk_),
	  k1_(k1),
	  y_(parent->y_), ci_(parent->ci_),
	  i0_(parent->i0_), i1_(parent->i1_),
	  nl_(parent->nl_)
{}

bool SequenceAdapter::has(int i)
{
	if (i >= k1_)
		return false;
	
	while (i < i0_) {
		// step backward
		if (ci_ > 0) {
			--ci_;
			chunk_ = chunkList_->get(ci_);
			i1_ = i0_;
			i0_ = i1_ - chunk_->text_.length();
			nl_ = false;
		}
		else {
			if (y_ <= 0) return false;
			--y_;
			chunkList_ = cache_->get(y_)->chunkList_;
			ci_ = chunkList_->length() - 1;
			if (ci_ >= 0)
				chunk_ = chunkList_->get(-1);
			else
				chunk_ = 0;
			i1_ = i0_ - 1;
			i0_ = i1_;
			if (chunk_) i0_ -= chunk_->text_.length();
			nl_ = true;
		}
	}
	while (i1_ + nl_ <= i) {
		// step forward
		if ((0 <= ci_) && (ci_ < chunkList_->length() - 1)) {
			++ci_;
			chunk_ = chunkList_->get(ci_);
			i0_ = i1_;
			i1_ = i0_ + chunk_->text_.length();
			nl_ = (ci_ == chunkList_->length() - 1);
		}
		else {
			if (y_ >= cache_->length() - 1) return false;
			if (highlighter_)
				if (!highlighter_->synchronise())
					return false;
			++y_;
			chunkList_ = cache_->get(y_)->chunkList_;
			ci_ = -(chunkList_->length() == 0);
			if (ci_ == 0)
				chunk_ = chunkList_->get(0);
			else
				chunk_ = 0;
			i0_ = i1_ + 1;
			i1_ = i0_;
			if (chunk_) i1_ += chunk_->text_.length();
			cache_->get(y_)->i0Hint_ = i0_;
			nl_ = (chunkList_->length() <= 1);
		}
	}
	return true;
}

QChar SequenceAdapter::get(int i)
{
	QChar ch = '\0';
	if (has(i)) {
		if (i == i1_)
			ch = '\n';
		else
			ch = chunk_->text_.at(i - i0_);
	}
	return ch;
}

QString SequenceAdapter::copy(int i0, int i1)
{
	int n = i1 - i0;
	if (n <= 0) return QString();
	QChar* buf = new QChar[n];
	/*bool someZero = false;
	int ySaved = y_;
	int ciSaved = ci_;
	int i0Saved = i0_;
	int i1Saved = i1_;
	bool nlSaved = nl_;*/
	for (int i = i0; i < i1; ++i) {
		buf[i - i0] = get(i);
		// if (buf[i - i0] == 0) someZero = true;
	}
	
	/*QString s(buf, n);
	if ((someZero) || (s == "RunCpp")) {
		qDebug() << "SequenceAdapter::copy(): some chars delivered where zero";
		qDebug() << "i0, i1 =" << i0 << "," << i1;
		qDebug() << "def(i0), def(i1-1) = " << def(i0) << "," << def(i1-1);
		qDebug() << "y_ =" << ySaved << "(" << y_ << ")";
		qDebug() << "ci_ =" << ciSaved << "(" << ci_ << ")";
		qDebug() << "i0_ =" << i0Saved << "(" << i0_ << ")";
		qDebug() << "i1_ =" << i1Saved << "(" << i1_ << ")";
		qDebug() << "nl_ =" << nlSaved << "(" << nl_ << ")";
		qDebug() << "nl_ =" << nlSaved << "(" << nl_ << ")";
	}
	return s;*/
	return QString(buf, n);
}

QString SequenceAdapter::copy(Ref<Token> token, int* yHint)
{
	Ref<YToken> yToken = token;
	if (yHint) *yHint = yToken->yHint_;
	y_ = yToken->yHint_;
	chunkList_ = cache_->get(y_)->chunkList_;
	ci_ = -(chunkList_->length() == 0);
	if (ci_ == 0)
		chunk_ = chunkList_->get(0);
	else
		chunk_ = 0;
	i0_ = cache_->get(y_)->i0Hint_;
	i1_ = i0_;
	if (chunk_) i1_ += chunk_->text_.length();
	nl_ = (chunkList_->length() <= 1);
	return copy(token->i0(), token->i1());
}

Token* SequenceAdapter::newToken()
{
	// cache_->get(y_)->i0Hint_ = i0Hint_;
	return new YToken(y_);
}

} // namespace pte
