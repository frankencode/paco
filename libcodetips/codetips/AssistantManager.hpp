/*
 * AssistantManager.hpp -- singleton which manages the assistants
 *
 * Copyright (c) 2010, Frank Mertens
 *
 * See ../COPYING for the license.
 */
#ifndef CODETIPS_SUPERVISOR_HPP
#define CODETIPS_SUPERVISOR_HPP

#include <ftl/List.hpp>
#include <ftl/Map.hpp>
#include <ftl/Singleton.hpp>
#include "codetips.hpp"
#include "Assistant.hpp"

namespace codetips
{

typedef List< Ref<Assistant, Owner> > AssistantList;
typedef Map<String, Ref<AssistantList, Owner> > AssistantListByLanguage;

class AssistantManager: public Instance, public Singleton<AssistantManager>
{
public:
	void registerAssistant(Ref<Assistant> assistant);
	Ref<AssistantListByLanguage> assistantListByLanguage() const;
	
	Ref<Instance, Owner> assist(Ref<Context> context, int modifiers, uchar_t key, const char* language = 0) const;
	
private:
	friend class Singleton<AssistantManager>;
	AssistantManager();
	
	Ref<AssistantListByLanguage, Owner> assistantListByLanguage_;
};

} // namespace codetips

#endif // CODETIPS_SUPERVISOR_HPP
