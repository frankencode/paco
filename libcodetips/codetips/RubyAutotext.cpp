/*
 * RubyAutotext.cpp -- Ruby autotext generator
 *
 * Copyright (c) 2010, Frank Mertens
 *
 * See ../COPYING for the license.
 */

#include <ftl/PrintDebug.hpp>
#include "AssistantManager.hpp"
#include "RubyAutotext.hpp"

namespace codetips
{

CODETIPS_REGISTRATION(RubyAutotext)

RubyAutotext::RubyAutotext()
{
	printTo(log(), "All fine.\n");
}

String RubyAutotext::language() const { return "ruby"; }
String RubyAutotext::name() const { return "autotext"; }
String RubyAutotext::displayName() const { return "Ruby Autotext"; }
String RubyAutotext::description() const {
	return "Speeds up typing source code by automatically inserting redundant text fragments.";
}

Ref<Tip, Owner> RubyAutotext::assist(Ref<Context> context, int modifiers, uchar_t key)
{
	Ref<Tip, Owner> tip;
	if (key == '\n') {
		String currLine = context->copyLine(context->line()).stripLeadingSpace();
		bool rescue = false;
		if ( (currLine.find("module ") == currLine.first()) ||
		     (currLine.find("class ") == currLine.first()) ||
		     (currLine.find("def ") == currLine.first()) ||
		     currLine.contains(" do") ||
		     (rescue = (currLine.trimmed() == "rescue")) ||
		     (currLine.find("begin") == currLine.first()) ||
		     (currLine.find("while") == currLine.first()) ) {
			String indent = context->indentOf(context->line());
			String nextIndent = context->indentOf(context->line() + 1);
			if ((!rescue) && (nextIndent->length() <= indent->length())) {
				String nextLine = context->copyLine(context->line() + 1);
				if (!(nextLine.contains("end") && (nextIndent->length() == indent->length()))) {
					context->insert("end");
					if (indent != "") context->insert(indent);
					context->insert("\n");
				}
			}
			String indentStep = context->indent();
			context->insert(indentStep);
			if (indent != "") context->insert(indent);
			context->insert("\n");
			context->move(1 + indent.length() + indentStep.length());
			tip = new Tip;
		}
		else {
			String currLine = context->copyLine(context->line());
			int len = currLine.length();
			int cx = context->linePos();
			if ((len > 0) && (cx > 0)) {
				uchar_t ch = currLine.get(currLine.first() + cx - 1);
				if (ch == '{') {
					String indent = context->indentOf(context->line());
					uchar_t ch2 = (cx < len) ? currLine.get(currLine.first() + cx) : uchar_t(0);
					if (ch2 == '}') {
						if (indent != "") context->insert(indent);
						context->insert("\n");
					}
					if (indent != "") context->insert(indent);
					context->insert("\n");
					context->move(1 + indent.length());
					String indentStep = context->indent();
					context->insert(indentStep);
					context->move(indentStep.length());
					tip = new Tip;
				}
			}
		}
	}
	else if (key == '{') {
		context->insert("{}");
		context->move(1);
		tip = new Tip;
	}
	return tip;
}

} // namespace codetips
