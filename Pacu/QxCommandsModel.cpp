#include "QxCommandsModel.hpp"

namespace pacu
{

QxCommandsModel::QxCommandsModel(Ref<Commands> commands)
	: commands_(commands)
{
	commands->setObserver(this);
}

QVariant QxCommandsModel::attribute(int i, int k) const { return commands_->get(i)->attribute(k); }
QString QxCommandsModel::attributeName(int k) const { return QxCommand::attributeName(k); }
int QxCommandsModel::numAttributes() const { return QxCommand::numAttributes(); }
int QxCommandsModel::length() const { return commands_->length(); }

} // namespace pacu
