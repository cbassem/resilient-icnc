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
//#include <cnc/serializer.h>

template< class StepTag >
class StepCheckpoint: public StepCheckpoint_i {
public:
	StepCheckpoint(int col_);
	virtual ~StepCheckpoint();

	void processStepPrescribe(StepTag prescriber, int prescriberColId, void * prescribedTagId, int tagCollectionId);
	void processStepDone(StepTag step, int stepColId, int puts, int prescribes);
	void processItemPut(StepTag producer, int stepProducerColId, void * itemId, int itemColId);

	//The serializer still contains the responsible one's tag, but this is ok since we know its type! :D
	void processStepPrescribe(CnC::serializer * ser, void * prescribedTagId);
	void processStepDone(CnC::serializer * ser);
	void processItemPut(CnC::serializer * ser, void * itemId);

	TagLog& getTagLog( StepTag tag );

	bool isDone(StepTag tag);

	int getId();


private:
	typedef std::tr1::unordered_map< StepTag, TagLog > tagMap_t;
	tagMap_t tagMap;
	int m_col_id;


};

template< class StepTag >
StepCheckpoint< StepTag >::StepCheckpoint(int col_id): tagMap(), m_col_id(col_id) {};

template< class StepTag >
StepCheckpoint< StepTag >::~StepCheckpoint() {};

template< class StepTag >
void StepCheckpoint< StepTag >::processStepPrescribe(StepTag prescriber, int prescriberColId, void * prescribedTagId, int tagCollectionId)
{
	TagLog& l_ = getTagLog( prescriber );
	l_.processPrescribe( prescribedTagId );
}

template< class StepTag >
void StepCheckpoint< StepTag >::processStepDone(StepTag step, int stepColId, int puts, int prescribes)
{
	TagLog& l_ = getTagLog( step );
	l_.processDone( puts, prescribes );
}

template< class StepTag >
void StepCheckpoint< StepTag >::processItemPut(StepTag producer, int stepProducerColId, void * itemId, int itemColId)
{
	TagLog& l_ = getTagLog( producer );
	l_.processPut( itemId );
}

template< class StepTag >
void StepCheckpoint< StepTag >::processStepPrescribe(CnC::serializer * ser, void * prescribedTagId)
{
	StepTag prescriber;
	(* ser) & prescriber;
	TagLog& l_ = getTagLog( prescriber );
	l_.processPrescribe( prescribedTagId );
}

template< class StepTag >
void StepCheckpoint< StepTag >::processStepDone(CnC::serializer * ser)
{
	StepTag step;
	int puts, prescribes;
	(* ser) & step & puts & prescribes;
	TagLog& l_ = getTagLog( step );
	l_.processDone( puts, prescribes );
}

template< class StepTag >
void StepCheckpoint< StepTag >::processItemPut(CnC::serializer * ser, void * itemId)
{
	StepTag producer;
	(* ser) & producer;
	TagLog& l_ = getTagLog( producer );
	l_.processPut( itemId );
}

template< class StepTag >
bool StepCheckpoint< StepTag >::isDone(StepTag tag)
{
	TagLog& l_ = getTagLog( tag );
	return l_.isDone();
}



template< class StepTag >
int StepCheckpoint< StepTag >::getId()
{
	return m_col_id;
}


template<class StepTag >
TagLog& StepCheckpoint< StepTag >::getTagLog( StepTag tag ) {
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
