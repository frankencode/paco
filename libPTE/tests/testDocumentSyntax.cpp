#include <QtScript>
#include <QFile>
#include <pte/DocumentSyntax.hpp>
#include <pte/UsePona.hpp>
#include <ftl/time>
#include <ftl/stdio>

namespace pte
{

QString scriptPath() {
	QFileInfo fileInfo(__FILE__);
	return fileInfo.dir().filePath(fileInfo.baseName() + ".js");
}

QString logPath() {
	return QFileInfo(__FILE__).baseName() + ".log";
}

QString loadScript() {
	QFile file(scriptPath());
	file.open(QIODevice::ReadOnly);
	return file.readAll();
}

int main()
{
	Time t0, t1, t2, t3;
	t0 = now();
	QString script = loadScript();
	t1 = now();
	QScriptEngine engine;
	DocumentSyntax::init(&engine);
	t2 = now();
	qDebug() << engine.evaluate(script).toVariant();
	if (engine.hasUncaughtException()) {
		qDebug() << engine.uncaughtExceptionLineNumber() << ":" << engine.uncaughtException().toString();
	}
	t3 = now();
	
	print("load time: %%ms\n", (t1 - t0).us());
	print("init time: %%ms\n", (t2 - t0).us());
	print("eval time: %%ms\n", (t3 - t0).us());
	
	return 0;
}

} // namespace pte

int main()
{
	pte::main();
}
