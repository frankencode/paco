#include <ftl/Utf8Decoder.hpp>
#include <QBuffer>
#include <QTextStream>
#include <QTextCodec>
#include "Edit.hpp"
#include "LanguageStack.hpp"
#include "LanguageLayer.hpp"
#include "Context.hpp"

namespace pte
{

Context::Context(Edit* edit)
	: edit_(edit),
	  cachedLine_(-1)
{
	connect(edit->document(), SIGNAL(changed(Ref<Delta, Owner>)), this, SLOT(changed(Ref<Delta, Owner>)));
}

Context::~Context()
{}

String Context::path() const { return string(edit_->document()->filePath()); }

String Context::language() const
{
	Ref<Document> doc = edit_->document();
	Ref<LanguageStack> stack = doc->highlighter()->languageStack();
	String name = stack->layer(0)->syntax()->name();
	Ref<Token> token;
	int cy = edit_->cursorLine();
	int cx = edit_->cursorColumn();
	doc->highlighterYield();
	doc->getCell(cy, cx, 0, 0, &token);
	if ((!token) && (cx > 0))
		doc->getCell(cy, cx - 1, 0, 0, &token);
	doc->highlighterResume();
	if (token)
		name = stack->languageManager()->layerByDefinitionId(token->definition())->syntax()->name();
	return name;
}

String Context::text() const
{
	if (text_ == "") {
		QByteArray bytes;
		QBuffer buffer(&bytes);
		buffer.open(QBuffer::WriteOnly);
		{
			QTextStream sink(&buffer);
			sink.setCodec(QTextCodec::codecForName("UTF-8"));
			Ref<Document> doc = edit_->document();
			bool flagSaved = doc->historyEnabled();
			doc->setHistoryEnabled(false);
			doc->save(&sink);
			doc->setHistoryEnabled(flagSaved);
		}
		text_ = String(bytes.constData(), bytes.size());
	}
	return text_;
}

String Context::copyLine(int line) const
{
	Ref<Document> doc = edit_->document();
	String text;
	if (line == Context::line()) {
		if (cachedLine_ != line) {
			doc->highlighterYield();
			cachedLineText_ = string(doc->copySpan(line, 0, intMax));
			doc->highlighterResume();
			cachedLine_ = line;
		}
		text = cachedLineText_;
	}
	else {
		doc->highlighterYield();
		text = string(doc->copySpan(line, 0, intMax));
		doc->highlighterResume();
	}
	return text;
}

int Context::numberOfLines() const
{
	return edit_->document()->numberOfLines();
}

String Context::indent() const
{
	return string(edit_->indent());
}

String Context::indentOf(int line) const
{
	String text = copyLine(line);
	int i = 0, n = text->length();
	while (i < n) {
		char ch = text->at(i);
		if ((ch == ' ') || (ch == '\t')) ++i;
		else break;
	}
	return text->head(i);
}

int Context::bytePos() const
{
	off_t x = 0;
	cursorByteAndCharOffset(&x, 0);
	return x;
}

int Context::pos() const
{
	off_t x = 0;
	cursorByteAndCharOffset(0, &x);
	return x;
}

int Context::line() const
{
	return edit_->cursorLine();
}

int Context::linePos() const
{
	Ref<Document> doc = edit_->document();
	doc->highlighterYield();
	int pos = doc->copySpan(line(), 0, edit_->cursorColumn()).length();
	doc->highlighterResume();
	return pos;
}

void Context::insert(String text)
{
	edit_->assistantBeginModify();
	edit_->insert(QString::fromUtf8(text));
	if (cachedLine_ != -1) {
		cachedLineText_ = "";
		cachedLine_ = -1;
	}
}

void Context::move(int delta)
{
	edit_->assistantBeginEdit();
	edit_->move(delta);
}

void Context::select(int delta)
{
	edit_->assistantBeginEdit();
	edit_->select(delta);
}

String Context::copy(int delta) const
{
	return string(edit_->copy(delta));
}

void Context::changed(Ref<Delta, Owner> delta)
{
	text_ = "";
	cachedLineText_ = "";
	cachedLine_ = -1;
}

void Context::cursorByteAndCharOffset(off_t* byteOffset, off_t* charOffset) const
{
	int cy = line();
	int cx = linePos();
	
	if ((cy == 0) && (cx == 0)) {
		if (byteOffset) *byteOffset = 0;
		if (charOffset) *charOffset = 0;
		return;
	}
	
	String text = Context::text();
	
	int y = 0, x = 0;
	off_t numCharRead = 0;
	Ref<Utf8Decoder, Owner> source = new Utf8Decoder(text->data(), text->size());
	for (uchar_t ch; source->read(&ch);) {
		++x;
		++numCharRead;
		if (ch == '\n') {
			if ((y == cy) && (x == cx))
				break;
			++y;
			x = 0;
		}
		if ((y == cy) && (x == cx))
			break;
	}
	
	if (byteOffset) *byteOffset = source->byteDecoder()->numBytesRead();
	if (charOffset) *charOffset = numCharRead;
}

String Context::string(const QString& s)
{
	QByteArray bytes = s.toUtf8();
	return String(bytes.constData(), bytes.size());
}

} // namespace pte
