/*
 * HaxeCodetips.cpp -- haxe codetips generator
 *
 * Copyright (c) 2010, Frank Mertens
 *
 * See ../COPYING for the license.
 */

#include <ftl/ProcessFactory.hpp>
#include <ftl/Dir.hpp>
#include <ftl/Path.hpp>
#include <ftl/Format.hpp>
#include <ftl/PrintDebug.hpp> // debug
#include "AssistantManager.hpp"
#include "HaxeMessageSyntax.hpp"
#include "InterpositionServer.hpp"
#include "HaxeCodetips.hpp"

namespace codetips
{

CODETIPS_REGISTRATION(HaxeCodetips)

HaxeCodetips::HaxeCodetips()
	: processFactory_(new ProcessFactory),
	  messageSyntax_(new HaxeMessageSyntax)
{
	Path haxe = Path::lookup(Process::env("PATH").split(":"), "haxe", File::Exists|File::Execute);
	
	processFactory_->setExecPath(haxe);
	processFactory_->setIoPolicy(Process::ForwardOutput|Process::ErrorToOutput);
	InterpositionServer::injectClient(processFactory_->envMap());
	
	setResource("haxe", haxe);
	
	update();
}

void HaxeCodetips::update()
{
	Path haxe = resource("haxe");
	printTo(log(), "HaxeCodetips::update(): haxe = %%\n", haxe);
	if (haxe != "") {
		if (File(haxe).access(File::Read|File::Execute))
			printTo(log(), "Found 'haxe' binary at \"%%\".\n", haxe);
		else
			printTo(log(), "Insufficiant permissions to 'haxe' binary.\n");
	}
	else {
		printTo(log(), "Can't find the 'haxe' binary.\n");
		printTo(log(), "Please provide the path to the 'haxe' binary.\n");
	}
}

String HaxeCodetips::language() const { return "haxe"; }
String HaxeCodetips::name() const { return "codetips"; }
String HaxeCodetips::displayName() const { return "haXe Code Tips"; }

String HaxeCodetips::description() const { return "Provides code tips for haxe. Press TAB key after \".\" or \"(\" for invocation."; }

Ref<Tip, Owner> HaxeCodetips::assist(Ref<Context> context, int modifiers, uchar_t key)
{
	if ( (key != '\t') ||
	     ((modifiers != Shift) && (modifiers != 0)) ||
	     (processFactory_->execPath() == "") )
	     return 0;
	
	if (modifiers == 0) {
		String line = context->copyLine(context->line());
		int x = context->linePos();
		if (x == 0) return 0;
		char ch = line.get(x - 1);
		if ((ch != '.') && (ch != '(')) return 0;
	}
	
	String className = Path(context->path()).fileNameSansExtension();
	String projectFile = "";
	String searchPath = context->path();
	
	while (searchPath != "/") {
		searchPath = Path(searchPath).reduce();
		Dir dir(searchPath);
		for (DirEntry entry; dir.read(&entry);) {
			Ref<StringList, Owner> parts = entry.name().split(".");
			if (parts->at(parts->length() - 1) == "hxml") {
				projectFile = Format("%%/%%") << searchPath << entry.name();
				break;
			}
		}
	}
	
	if (projectFile == "") return 0;
	
	processFactory_->setWorkingDirectory(Path(projectFile).reduce());
	// debug("HaxeCodetips::assist(): processFactory_->execPath() = \"%%\"\n", processFactory_->execPath());
	
	String options = Format("%%,%%,--display,%%@%%") << projectFile << className << context->path() << context->bytePos();
	// debug("HaxeCodetips::assist(): options = \"%%\"\n", options);
	
	processFactory_->setOptions(options.split(","));
	
	Ref<Process, Owner> process = processFactory_->produce();
	String message = process->rawOutput()->readAll();
	// debug("HaxeCodetips::assist(): message = \"%%\"\n", message);
	
	Ref<Tip, Owner> tip = messageSyntax_->parse(message);
	if ((!tip) && (message != ""))
		tip = new TypeTip(new Type(message));
	
	return tip;
}

} // namespace codetips
