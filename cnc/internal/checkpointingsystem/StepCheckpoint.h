/*
 * StepCheckpoint.h
 *
 *  Created on: Apr 28, 2015
 *      Author: blackline
 */

#ifndef STEPCHECKPOINT_H_
#define STEPCHECKPOINT_H_

#include "TagLog.h"
#include "StepCheckpoint_i.h"
#include <tr1/unordered_map>


template< class StepTag >
class StepCheckpoint: public StepCheckpoint_i {
public:
	StepCheckpoint();
	virtual ~StepCheckpoint();

	void processStepPrescribe(StepTag prescriber, int prescriberColId, void * prescribedTagId, int tagCollectionId);
	void processStepDone(StepTag step, int stepColId, int puts, int prescribes);
	void processItemPut(StepTag producer, int stepProducerColId, void * itemId, int itemColId);


private:
	typedef std::tr1::unordered_map< StepTag, TagLog > hmap;


};

template< class StepTag >
StepCheckpoint< StepTag >::StepCheckpoint() {};

template< class StepTag >
StepCheckpoint< StepTag >::~StepCheckpoint() {};

template< class StepTag >
void StepCheckpoint< StepTag >::processStepPrescribe(StepTag prescriber, int prescriberColId, void * prescribedTagId, int tagCollectionId)
{

}

template< class StepTag >
void StepCheckpoint< StepTag >::processStepDone(StepTag step, int stepColId, int puts, int prescribes)
{

}

template< class StepTag >
void StepCheckpoint< StepTag >::processItemPut(StepTag producer, int stepProducerColId, void * itemId, int itemColId)
{

}



#endif /* STEPCHECKPOINT_H_ */
