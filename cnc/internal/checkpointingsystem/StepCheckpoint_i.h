/*
 * StepCheckpoint.h
 *
 *  Created on: Apr 28, 2015
 *      Author: blackline
 */

#ifndef STEPCHECKPOINT_I_H_
#define STEPCHECKPOINT_I_H_

class StepCheckpoint_i {
public:
	virtual ~StepCheckpoint_i() {};
	virtual int getId() = 0;

	virtual void processStepPrescribe(CnC::serializer * ser, void * prescribedTagId) = 0;
	virtual void processStepDone(CnC::serializer * ser, int puts, int prescribes) = 0;
	virtual void processItemPut(CnC::serializer * ser, void * itemId) = 0;

};

#endif /* STEPCHECKPOINT_I_H_ */
