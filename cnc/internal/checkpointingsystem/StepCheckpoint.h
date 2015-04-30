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

	TagLog& getTagLog(int colid, StepTag tag);


private:
	typedef std::tr1::unordered_map< StepTag, TagLog > tagMap_t;
	tagMap_t tagMap;

};

template< class StepTag >
StepCheckpoint< StepTag >::StepCheckpoint(): tagMap() {};

template< class StepTag >
StepCheckpoint< StepTag >::~StepCheckpoint() {};

template< class StepTag >
void StepCheckpoint< StepTag >::processStepPrescribe(StepTag prescriber, int prescriberColId, void * prescribedTagId, int tagCollectionId)
{
	TagLog& l_ = getTagLog(prescriberColId, prescriber);
	l_.processPrescribe(prescribedTagId);
}

template< class StepTag >
void StepCheckpoint< StepTag >::processStepDone(StepTag step, int stepColId, int puts, int prescribes)
{
	TagLog& l_ = getTagLog(stepColId, step);
	l_.processDone(puts, prescribes);
}

template< class StepTag >
void StepCheckpoint< StepTag >::processItemPut(StepTag producer, int stepProducerColId, void * itemId, int itemColId)
{
	TagLog& l_ = getTagLog(stepProducerColId, producer);
	l_.processPut(itemId);
}

template<class StepTag >
TagLog& StepCheckpoint< StepTag >::getTagLog(int colid, StepTag tag) {
	typename tagMap_t::iterator it = tagMap.find(tag);
	if (it == tagMap.end()) {
		TagLog & l_ = tagMap[tag] = TagLog();
		return l_;
	} else {
		TagLog & l_ = it->second;
		return l_;
	}
}



#endif /* STEPCHECKPOINT_H_ */
