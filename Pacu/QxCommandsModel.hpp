#ifndef PACU_QXCOMMANDSMODEL_HPP
#define PACU_QXCOMMANDSMODEL_HPP

#include <pte/CustomList.hpp>
#include "UsePte.hpp"
#include "QxTableModelWrapper.hpp"
#include "QxCommand.hpp"

namespace pacu
{

class QxCommandsModel: public QxTableModelWrapper<QxCommand*>
{
public:
	typedef CustomList<QxCommand*> Commands;
	
	QxCommandsModel(Ref<Commands> commands);
	
private:
	virtual QVariant attribute(int i, int k) const;
	virtual QString attributeName(int k) const;
	virtual int numAttributes() const;
	virtual int length() const;
	
	Ref<Commands, Owner> commands_;
};

} // namespace pacu

#endif // PACU_QXCOMMANDSMODEL_HPP
