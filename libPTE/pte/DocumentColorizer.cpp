// #include <ftl/stdio> // DEBUG
// #include <QDebug>
#include "Document.hpp"
#include "LanguageManager.hpp"
#include "LanguageLayer.hpp"
#include "LanguageStack.hpp"
#include "DocumentColorizer.hpp"

namespace pte
{

DocumentColorizer::DocumentColorizer(Ref<LanguageStack> languageStack, Ref<Document> document, int y)
	: languageManager_(languageStack->languageManager()),
	  languageStack_(languageStack),
	  document_(document),
	  highlighter_(document->highlighter()),
	  y_(y - (y != -1)), ci_(-1),
	  k0_(0), k1_(0),
	  needUpdate_(false),
	  singleLine_(y != -1)
{}

bool DocumentColorizer::nextLine()
{
	// if (stayOnLine_) qDebug() << "DocumentColorizer::nextLine()...";
	if (y_ < document_->cache()->length() - 1) {
		++y_;
		ci_ = -1;
		line_ = document_->cache()->get(y_);
		text_.clear(); // text_ = document_->copySpan(y_, 0, intMax);
		k0_ = k1_;
		// k1_ = k0_ + text_.length() + 1;
		k1_ = k0_ + 1;
		{
			Ref<ChunkList> chunkList = line_->chunkList_;
			for (int i = 0, n = chunkList->length(); i < n; ++i)
				k1_ += chunkList->get(i)->text_.length();
		}
		return true;
	}
	else {
		ci_ = -1;
		line_ = 0;
		text_.clear();
		k0_ = 0;
		k1_ = 0;
		return false;
	}
}

const QString& DocumentColorizer::text()
{
	if (text_.length() == 0)
		text_ = document_->copySpan(y_, 0, intMax);
	return text_;
}

void DocumentColorizer::appendSpan(int s0, int sn, Ref<pte::Style> style, Ref<Token> token)
{
	//if (stayOnLine_)
	//	qDebug() << "appendSpan.0: document_->filePath(), y_, s0, sn = " << document_->filePath() << y_ << s0 << sn;
	Ref<ChunkList> chunkList = line_->chunkList_;
	++ci_;
	
	// check for redundancy
	int n = sn, i = ci_;
	while ((n > 0) && (i < chunkList->length())) {
		// print("s");
		Ref<Chunk> chunk = chunkList->get(i);
		if (chunk->style_ != style) { /*print("b");*/ break; }
		chunk->token_ = token;
		// if (!chunk->token_) break;
		// if (chunk->token_->type() != token->type()) break;
		++i;
		n -= chunk->text_.length();
	}
	
	if (n == 0) {
		// overrun redundancy
		ci_ = i - 1;
		// print(".");
	}
	else {
		needUpdate_ = true;
		// print("x");
		QString span = text().mid(s0, sn);
		while (ci_ < chunkList->length())
			chunkList->remove(ci_);
		int x0 = (ci_ > 0) ? chunkList->get(ci_ - 1)->x1_ : 0;
		int x1 = x0;
		// qDebug() << "appendSpan.1.0";
		document_->pushChunk(y_, ci_, x0, &x1, span, style, token);
		ci_ = chunkList->length() - 1; // HACK for tab support
		// qDebug() << "appendSpan.1.1: x0, x1 = " << x0 << x1;
	}
	// qDebug() << "appendSpan.2";
}

bool DocumentColorizer::project(Ref<Token> token, int i0, int i1)
{
	while (true)
	{
		if (i0 < k1_) {
			int j1 = i1;
			if (i1 > k1_) j1 = k1_ - 1;
			int s0 = i0 - k0_, sn = j1 - i0; // index range of span
			int nl = k1_ - k0_ - 1; // text_.length(); // line length
			if (s0 + sn > nl) --sn; // strip newline
			if ((sn > 0) && (s0 < nl)) { // omit empty matches
				/*Ref<LanguageProvider> provider = languageManager_->languageProvider(token->definitionId());
				Ref<pte::Style> style = provider->style(token->rule());*/
				Ref<pte::Style> style;
				for (Ref<Token> candidate = token; (!style) && (candidate); candidate = candidate->parent()) {
					style = languageManager_->layerByDefinitionId(candidate->definition())->style(candidate->rule());
					// qDebug() << "up";
				}
				if (!style) style = document_->defaultStyle();
				appendSpan(s0, sn, style, token);
				/*qDebug() << "token->countChildren() =" << token->countChildren();
				qDebug() << "appendSpan("
					<< languageManager_->layerByDefinitionId(token->definition())->syntax()->name()
					<< languageManager_->layerByDefinitionId(token->definition())->syntax()->numRules()
					<< token->rule() << y_ << i0 << i1 << text().mid(s0, sn) << style.get() << (style == document_->defaultStyle() )<< ")";*/
			}
			/*else {
				Ref<LanguageProvider> provider = languageManager_->languageProvider(token->definition());
				Ref<pte::Style> style = provider->style(token->rule());
				// qDebug() << "dropSpan(" << token->ruleName() << y_ << i0 << i1 << text().mid(s0, sn) << style.get() << ")";
			}*/
			if (k1_ < i1) {
				i0 = k1_;
				continue;
			}
		}
		else {
			if (!singleLine_) { // implicit logic HACK, needs improvment
				if (highlighter_)
					if (!highlighter_->synchronise()) {
						// print("\n-- yielded --\n");
						return false;
					}
			}
			// if (stayOnLine_)
			//	qDebug() << "nextLine(" << text_ <<  y_  << k0_ << k1_ << ")";
			if (nextLine()) {
				continue;
				// print("\n%%:", y_);
			}
		}
		break;
	}
	return true;
}

} // namespace pte
