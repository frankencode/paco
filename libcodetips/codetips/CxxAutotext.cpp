/*
 * CxxAutotext.cpp -- C++ autotext generator
 *
 * Copyright (c) 2010, Frank Mertens
 *
 * See ../COPYING for the license.
 */

#include <ftl/PrintDebug.hpp>
#include "AssistantManager.hpp"
#include "CxxAutotext.hpp"

namespace codetips
{

CODETIPS_REGISTRATION(CxxAutotext)

CxxAutotext::CxxAutotext()
{
	printTo(log(), "All fine.\n");
}

String CxxAutotext::language() const { return "cxx"; }
String CxxAutotext::name() const { return "autotext"; }
String CxxAutotext::displayName() const { return "C++ Autotext"; }

String CxxAutotext::description() const
{
	return "Speeds up typing source code by automatically inserting redundant text fragments.";
}

Ref<Tip, Owner> CxxAutotext::assist(Ref<Context> context, int modifiers, uchar_t key)
{
	Ref<Tip, Owner> tip;
	
	if (key == '{') {
		bool appendSemicolon = false;
		if (context->line() > 0) {
			String currLine = context->copyLine(context->line());
			String prevLine = context->copyLine(context->line() - 1);
			appendSemicolon = currLine.contains("class ") || currLine.contains("struct ") || currLine.contains("enum ");
			if (appendSemicolon)
				appendSemicolon = !currLine->contains('(');
			if (!appendSemicolon)
				if (currLine == "")
					appendSemicolon = prevLine.contains("class ") || prevLine.contains("struct ") || currLine.contains("enum ");
		}
		if (appendSemicolon)
			context->insert("{};");
		else
			context->insert("{}");
		context->move(1);
		tip = new Tip;
	}
	/*else if (key == '(') {
		context->insert("()");
		context->move(1);
	}
	else if (key == '[') {
		context->insert("[]");
		context->move(1);
	}*/
	else if (key == '\n') {
		String currLine = context->copyLine(context->line());
		int len = currLine.length();
		int cx = context->linePos();
		if ((len > 0) && (cx > 0)) {
			uchar_t ch = currLine.get(currLine.first() + cx - 1);
			if (ch == '{') {
				String prevLine = context->copyLine(context->line() - 1);
				uchar_t ch2 = (cx < len) ? currLine.get(currLine.first() + cx) : uchar_t(0);
				String indent = context->indentOf(context->line());
				if (ch2 == '}') {
					if (indent != "") context->insert(indent);
					context->insert("\n");
				}
				if (indent != "") context->insert(indent);
				context->insert("\n");
				context->move(1 + indent.length());
				if ( (!( currLine.contains("class ") || currLine.contains("namespace ") ||
				         prevLine.contains("class ") || prevLine.contains("namespace ") )) ||
				     currLine->contains(')') ) {
					String indentStep = context->indent();
					context->insert(indentStep);
					context->move(indentStep.length());
				}
				tip = new Tip;
			}
			else if (ch == ':') {
				String indent = context->indentOf(context->line());
				String indentStep = context->indent();
				context->insert(indentStep);
				if (indent != "") context->insert(indent);
				context->insert("\n");
				context->move(1 + indent.length() + indentStep.length());
				tip = new Tip;
			}
		}
	}
	#if 0
	else if (key == '*') {
		String currLine = context->copyLine(context->line());
		int len = currLine.length();
		int cx = context->linePos();
		if ((len > 0) && (cx > 0)) {
			if (currLine.get(currLine.first() + cx - 1) == '/') {
				context->insert("**/");
				context->move(1);
			}
		}
	}
	#endif
	
	return tip;
}

} // namespace codetips
