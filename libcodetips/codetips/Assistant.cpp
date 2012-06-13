/*
 * Assistant.cpp -- language-specific codetips and autotext generator
 *
 * Copyright (c) 2010, Frank Mertens
 *
 * See ../COPYING for the license.
 */

#include <ftl/File.hpp>
#include "Assistant.hpp"

namespace codetips
{

Assistant::Assistant()
	: resourceMap_(new ResourceMap),
	  resourceList_(new ResourceList)
{}

Variant Assistant::resource(String key) const {
	return resourceMap_->value(key);
}

void Assistant::setResource(String key, Variant value) {
	if (resourceMap_->insert(key, value)) {
		resourceList_->append(ResourceList::Item(key, value));
	}
	else {
		resourceMap_->remove(key);
		resourceMap_->insert(key, value);
	}
}

Ref<Log> Assistant::log() {
	if (!log_) {
		Ref<File, Owner> tmp = File::temp();
		tmp->unlinkWhenDone();
		log_ = new Log(tmp);
		
	}
	return log_;
}

void Assistant::update()
{}

Ref<ResourceMap> Assistant::resourceMap() { return resourceMap_; }
Ref<ResourceList> Assistant::resourceList() { return resourceList_; }

} // namespace codetips
