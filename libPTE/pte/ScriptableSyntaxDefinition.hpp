#ifndef PTE_SCRIPTABLESYNTAXDEFINITION_HPP
#define PTE_SCRIPTABLESYNTAXDEFINITION_HPP

#include <QtScript>
#include "SyntaxDefinition.hpp"

namespace pte
{

/** Wrapper around ftl::Syntax<Media>::Definition, which
  * provides the syntax definition interface to QtScript.
  */
class ScriptableSyntaxDefinition: public QObject, public SyntaxDefinition
{
	Q_OBJECT
	
public:
	typedef SyntaxDefinition Super;
	
	static void init(QScriptEngine* engine);
	
	ScriptableSyntaxDefinition(Ref<Scope> scope, QString name);
	
private:
	static const char* constCharFromString(QString s);
	
	static QScriptValue nodeToScriptValue(QScriptEngine* engine, NODE const& node);
	static void nodeFromScriptValue(const QScriptValue& value, NODE& node);
	
	static QScriptValue ruleToScriptValue(QScriptEngine* engine, RULE const& rule);
	static void ruleFromScriptValue(const QScriptValue& value, RULE& rule);
	
	static QScriptValue constCharToScriptValue(QScriptEngine* engine, const char* const& cstr);
	static void constCharFromScriptValue(const QScriptValue& value, const char*& cstr);
	
	inline static ScriptableSyntaxDefinition* self(QScriptContext* context)
	{
		QScriptValue thisObject = context->parentContext()->thisObject();
		return qobject_cast<ScriptableSyntaxDefinition*>(thisObject.toQObject());
	}
	
	inline static SyntaxDefinition* super(QScriptContext* context)
	{
		return self(context);
	}
	
	static void checkNumberOfArguments(QScriptContext* context, int expected);
	static void checkNumberOfArguments(QScriptContext* context, int min, int max);
	
	static QScriptValue OPTION_(QScriptContext* context, QScriptEngine* engine);
	
	static QScriptValue CHAR_(QScriptContext* context, QScriptEngine* engine);
	static QScriptValue OTHER_(QScriptContext* context, QScriptEngine* engine);
	static QScriptValue ANY_(QScriptContext* context, QScriptEngine* engine);
	static QScriptValue RANGE_(QScriptContext* context, QScriptEngine* engine);
	static QScriptValue EXCEPT_(QScriptContext* context, QScriptEngine* engine);
	static QScriptValue STRING_(QScriptContext* context, QScriptEngine* engine);
	static QScriptValue REPEAT_(QScriptContext* context, QScriptEngine* engine);
	static QScriptValue BOI_(QScriptContext* context, QScriptEngine* engine);
	static QScriptValue EOI_(QScriptContext* context, QScriptEngine* engine);
	static QScriptValue PASS_(QScriptContext* context, QScriptEngine* engine);
	static QScriptValue FAIL_(QScriptContext* context, QScriptEngine* engine);
	static QScriptValue FIND_(QScriptContext* context, QScriptEngine* engine);
	static QScriptValue AHEAD_(QScriptContext* context, QScriptEngine* engine);
	static QScriptValue NOT_(QScriptContext* context, QScriptEngine* engine);
	static QScriptValue LENGTH_(QScriptContext* context, QScriptEngine* engine);
	
	static QScriptValue CHOICE_(QScriptContext* context, QScriptEngine* engine);
	static QScriptValue GLUE_(QScriptContext* context, QScriptEngine* engine);
	
	static QScriptValue KEYWORD_(QScriptContext* context, QScriptEngine* engine);
	static QScriptValue DEFINE_(QScriptContext* context, QScriptEngine* engine);
	static QScriptValue DEFINE_VOID_(QScriptContext* context, QScriptEngine* engine);
	static QScriptValue ENTRY_(QScriptContext* context, QScriptEngine* engine);
	
	static QScriptValue REF_(QScriptContext* context, QScriptEngine* engine);
	static QScriptValue INLINE_(QScriptContext* context, QScriptEngine* engine);
	static QScriptValue PREVIOUS_(QScriptContext* context, QScriptEngine* engine);
	
	static QScriptValue LINK_(QScriptContext* context, QScriptEngine* engine);
	
	static QScriptValue STATE_FLAG_(QScriptContext* context, QScriptEngine* engine);
	static QScriptValue STATE_CHAR_(QScriptContext* context, QScriptEngine* engine);
	static QScriptValue STATE_STRING_(QScriptContext* context, QScriptEngine* engine);
	
	static QScriptValue SET_(QScriptContext* context, QScriptEngine* engine);
	static QScriptValue IF_(QScriptContext* context, QScriptEngine* engine);
	static QScriptValue GETCHAR_(QScriptContext* context, QScriptEngine* engine);
	static QScriptValue SETCHAR_(QScriptContext* context, QScriptEngine* engine);
	static QScriptValue VARCHAR_(QScriptContext* context, QScriptEngine* engine);
	static QScriptValue VAROTHER_(QScriptContext* context, QScriptEngine* engine);
	static QScriptValue GETSTRING_(QScriptContext* context, QScriptEngine* engine);
	static QScriptValue SETSTRING_(QScriptContext* context, QScriptEngine* engine);
	static QScriptValue VARSTRING_(QScriptContext* context, QScriptEngine* engine);
	
	static QScriptValue INVOKE_(QScriptContext* context, QScriptEngine* engine);
};

} // namespace pte

Q_DECLARE_METATYPE(pte::SyntaxDefinition::NODE)
Q_DECLARE_METATYPE(pte::SyntaxDefinition::RULE)
Q_DECLARE_METATYPE(const char*)
Q_DECLARE_METATYPE(pte::ScriptableSyntaxDefinition*)

#endif // PTE_SCRIPTABLESYNTAXDEFINITION_HPP
