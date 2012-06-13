/*
 * XmlAutotext.cpp -- XML autotext generator
 *
 * Copyright (c) 2010, Frank Mertens
 *
 * See ../COPYING for the license.
 */

#include <ftl/PrintDebug.hpp>
#include "AssistantManager.hpp"
#include "XmlOpenTagSyntax.hpp"
#include "XmlAutotext.hpp"

namespace codetips
{

CODETIPS_REGISTRATION(XmlAutotext)

XmlAutotext::XmlAutotext()
	: openTagSyntax_(new XmlOpenTagSyntax)
{
	printTo(log(), "All fine.\n");
}

String XmlAutotext::language() const { return "xml"; }
String XmlAutotext::name() const { return "autotext"; }
String XmlAutotext::displayName() const { return "XML Autotext"; }
String XmlAutotext::description() const {
	return "Speeds up typing source code by automatically inserting redundant text fragments.";
}

Ref<Tip, Owner> XmlAutotext::assist(Ref<Context> context, int modifiers, uchar_t key)
{
	Ref<Tip, Owner> tip;
	
	if (key == '>') {
		String text = context->copyLine(context->line());
		int cx = context->linePos();
		int i0 = 0, i1 = -1;
		Ref<Token, Owner> token;
		while (true) {
			token = openTagSyntax_->find(text, &i0, &i1);
			if (token) {
				if (i1 < cx) {
					i0 = i1;
					continue;
				}
				if (i1 == cx) break;
				i0 = i1;
			}
			else {
				break;
			}
		}
		if ((token) && (i1 == cx)) {
			Ref<Token> tagName = token->firstChild();
			String name = text->copy(tagName->i0(), tagName->i1());
			context->insert(">");
			context->insert(name);
			context->insert("></");
			context->move(1);
			tip = new Tip;
		}
	}
	
	return tip;
}

} // namespace codetips
