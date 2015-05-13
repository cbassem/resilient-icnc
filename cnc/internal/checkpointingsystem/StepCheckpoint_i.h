/*
 * StepCheckpoint.h
 *
 *  Created on: Apr 28, 2015
 *      Author: blackline
 */

#ifndef STEPCHECKPOINT_I_H_
#define STEPCHECKPOINT_I_H_

#include <cnc/internal/checkpointingsystem/ItemCheckpoint.h>

class StepCheckpoint_i {
public:
	virtual ~StepCheckpoint_i() {};
	virtual int getId() = 0;

	virtual void decrement_get_counts() = 0;
	virtual void cleanup() = 0;

	virtual void processStepPrescribe(CnC::serializer * ser, void * prescribedTagId) = 0;
	virtual void processStepDone(CnC::serializer * ser) = 0;
	virtual void processItemPut(CnC::serializer * ser, void * itemId) = 0;
	virtual void processItemGet(CnC::serializer * ser, ItemCheckpoint_i* ich, void * tag) = 0;

	virtual bool isDone(CnC::serializer * ser) = 0;

};

#endif /* STEPCHECKPOINT_I_H_ */
