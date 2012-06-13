/*
 * AssistantManager.cpp -- singleton which manages the assistants
 *
 * Copyright (c) 2010, Frank Mertens
 *
 * See ../COPYING for the license.
 */

#include <ftl/PrintDebug.hpp> // debug
#include "AssistantManager.hpp"

namespace codetips
{

AssistantManager::AssistantManager()
	: assistantListByLanguage_(new AssistantListByLanguage)
{}

void AssistantManager::registerAssistant(Ref<Assistant> assistant)
{
	Ref<AssistantList, Owner> list;
	if (!assistantListByLanguage_->lookup(assistant->language(), &list)) {
		list = new AssistantList;
		assistantListByLanguage_->insert(assistant->language(), list);
	}
	list->append(assistant);
}

Ref<AssistantListByLanguage> AssistantManager::assistantListByLanguage() const
{
	return assistantListByLanguage_;
}

Ref<Instance, Owner> AssistantManager::assist(Ref<Context> context, int modifiers, uchar_t key, const char* language) const
{
	Ref<Tip, Owner> tip;
	if (!language) {
		tip = assist(context, modifiers, key, context->language());
		if (!tip)
			tip = assist(context, modifiers, key, "any");
	}
	else {
		Ref<AssistantList, Owner> list;
		if (assistantListByLanguage_->lookup(language, &list)) {
			AssistantList::Iterator it = list->iterator();
			for (Ref<Assistant, Owner> assistant; it.read(&assistant) && (!tip);)
				tip = assistant->assist(context, modifiers, key);
		}
	}
	return tip;
}

} // namespace codetips
