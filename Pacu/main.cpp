#include <QtCore>
#include <ftl/streams>
#ifdef Q_WS_X11
#include <ftl/Process.hpp>
#endif
// #include <ftl/utils>
#include "UseFtl.hpp"
#include "LogDisplay.hpp"
#include "QxPacuTheme.hpp"
#include "QxMainWindow.hpp"
#include "QxApplication.hpp"
#include "QxCustomStyle.hpp"

namespace pacu
{

void testVideoTerminal()
{
	print("normal palette:\n");
	for (int i = 30; i <= 37; ++i)
		print("  %%: \033[%%mColor test...\033[m\n", i, i);
	for (int i = 40; i <= 47; ++i)
		print("  %%: \033[%%mColor test...\033[m\n", i, i);
	print("bright palette:\n");
	for (int i = 30; i <= 37; ++i)
		print("  %%: \033[1;%%mColor test...\033[m\n", i, i);
	for (int i = 40; i <= 47; ++i)
		print("  %%: \033[1;%%mColor test...\033[m\n", i, i);
}

void testVideoTerminalHighLoad()
{
	for (int i = 0; true; ++i)
		print("High load %%...\n", i);
}

void testLongLine()
{
	print("long line test:\n");
	print("x");
	for (int i = 0; i < 8096; ++i)
		print("|.......");
	print("x");
	print("\n");
}

void testEcho()
{
	String s;
	while (true) {
		uint8_t byte;
		rawInput()->read(&byte, 1);
		if ((0x20 < byte) && (byte < 128))
			print("('%%':%%)", char(byte), int(byte));
		else
			print("(%%)", int(byte));
	}
}

int main(int argc, char** argv)
{
	// Time t = now();
	
	if (argc == 2) {
		if (QString(argv[1]) == "--follow") {
			Ref<LogDisplay, Owner> display_;
			for (String path; input()->read(&path);)
				display_ = new LogDisplay(path);
			return 0;
		}
	}
	
	bool styleOverload = false;
	for (int i = 1; i < argc; ++i)
		if (QString(argv[i]).contains("-style"))
			styleOverload = true;
	
	#ifdef Q_WS_X11
	bool needToUnsetGtk2RcFiles = false;
	if (!styleOverload) {
		String clearlooksRc = "/usr/share/themes/Clearlooks/gtk-2.0/gtkrc";
		String glossyRc = "/usr/share/themes/Glossy/gtk-2.0/gtkrc";
		if (File(clearlooksRc).exists()) {
			Process::setEnv("GTK2_RC_FILES", clearlooksRc);
			needToUnsetGtk2RcFiles = true;
		}
		else if (File(glossyRc).exists()) {
			Process::setEnv("GTK2_RC_FILES", glossyRc);
			needToUnsetGtk2RcFiles = true;
		}
	}
	#endif
	
	QxApplication app(argc, argv);
	
	if (argc == 2) {
		if (QString(argv[1]) == "--vttest") {
			testVideoTerminal();
			return 0;
		}
		else if (QString(argv[1]) == "--vtlong") {
			testLongLine();
			return 0;
		}
		else if (QString(argv[1]) == "--vtload") {
			testVideoTerminalHighLoad();
			return 0;
		}
		else if (QString(argv[1]) == "--echo") {
			testEcho();
			return 0;
		}
	}
	
	QCoreApplication::setApplicationName("Pacu");
	QCoreApplication::setApplicationVersion("1.2.4");
	QCoreApplication::setOrganizationDomain("cyblogic.com");
	QCoreApplication::setOrganizationName("Cyblogic");
	
	if (argc == 2) {
		if (QString(argv[1]) == "--reset")
			QSettings().clear();
	}
	else if (argc > 2) {
		if (QString(argv[1]) == "--open") {
			for (int i = 2; i < argc; ++i)
				print("\033]9;%%\007", argv[i]);
			return 0;
		}
	}
	
	// print("main(): t0 = %%ms\n", (now() - t).miliSeconds());
	
	QxPacuTheme::load();
	
	if (!styleOverload)
		QApplication::setStyle(new QxCustomStyle);
	
	// print("main(): t1 = %%ms\n", (now() - t).miliSeconds());
	
	#ifdef Q_WS_X11
	if (needToUnsetGtk2RcFiles) {
		Process::unsetEnv("GTK2_RC_FILES");
	}
	#endif
	
	return app.exec();
}

} // namespace pacu

int main(int argc, char** argv)
{
	return pacu::main(argc, argv);
}
