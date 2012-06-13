/*
 * PythonAutotext.cpp -- Python autotext generator
 *
 * Copyright (c) 2010, Frank Mertens
 *
 * See ../COPYING for the license.
 */

#include <ftl/PrintDebug.hpp>
#include "AssistantManager.hpp"
#include "PythonAutotext.hpp"

namespace codetips
{

CODETIPS_REGISTRATION(PythonAutotext)

PythonAutotext::PythonAutotext()
{
	printTo(log(), "All fine.\n");
}

String PythonAutotext::language() const { return "python"; }
String PythonAutotext::name() const { return "autotext"; }
String PythonAutotext::displayName() const { return "Python Autotext"; }
String PythonAutotext::description() const {
	return "Speeds up typing source code by automatically inserting redundant text fragments.";
}

Ref<Tip, Owner> PythonAutotext::assist(Ref<Context> context, int modifiers, uchar_t key)
{
	Ref<Tip, Owner> tip;
	if (key == '\n') {
		String currLine = context->copyLine(context->line());
		int len = currLine.length();
		int cx = context->linePos();
		if ((len > 0) && (cx > 0)) {
			uchar_t ch = currLine.get(currLine.first() + cx - 1);
			if (ch == ':') {
				String indent = context->indentOf(context->line());
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
	return tip;
}

} // namespace codetips
