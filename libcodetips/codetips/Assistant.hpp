/*
 * Assistant.hpp -- language-specific codetips and autotext generator
 *
 * Copyright (c) 2010, Frank Mertens
 *
 * See ../COPYING for the license.
 */
#ifndef CODETIPS_ASSISTANT_HPP
#define CODETIPS_ASSISTANT_HPP

#include <ftl/Map.hpp>
#include <ftl/Variant.hpp>
#include <ftl/LineSink.hpp>
#include "codetips.hpp"

namespace codetips
{

typedef Map<String, Variant> ResourceMap;
typedef List<ResourceMap::Item> ResourceList;
typedef LineSink Log;

class Assistant: public Instance
{
public:
	Assistant();
	virtual ~Assistant() {}
	
	virtual String language() const = 0;
	virtual String name() const = 0;
	virtual String displayName() const = 0;
	virtual String description() const = 0;
	
	enum Modifiers { Shift = 1, Alt = 2, Control = 4, Meta = 8 };
	virtual Ref<Tip, Owner> assist(Ref<Context> context, int modifiers, uchar_t key) = 0;
	
	Variant resource(String key) const;
	void setResource(String key, Variant value);
	
	Ref<ResourceMap> resourceMap();
	Ref<ResourceList> resourceList();
	
	Ref<Log> log();
	virtual void update();
	
private:
	Ref<ResourceMap, Owner> resourceMap_;
	Ref<ResourceList, Owner> resourceList_;
	Ref<Log, Owner> log_;
};

#define CODETIPS_REGISTRATION(SpecialAssistant) \
class SpecialAssistant ## Registration { \
public: \
	SpecialAssistant ## Registration(); \
}; \
\
SpecialAssistant ## Registration::SpecialAssistant ## Registration() { \
	AssistantManager::instance()->registerAssistant(new SpecialAssistant); \
} \
namespace { SpecialAssistant ## Registration registration_; }

} // namespace codetips

#endif // CODETIPS_ASSISTANT_HPP
