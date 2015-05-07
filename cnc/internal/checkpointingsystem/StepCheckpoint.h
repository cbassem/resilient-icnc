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
	void processStepDone(StepTag step, int stepColId, int puts, int prescribes, int gets);
	void processItemPut(StepTag producer, int stepProducerColId, void * itemId, int itemColId);
	void processItemGet(StepTag getter, ItemCheckpoint_i* ich, void * tag);

	//The serializer still contains the responsible one's tag, but this is ok since we know its type! :D
	void processStepPrescribe(CnC::serializer * ser, void * prescribedTagId);
	void processStepDone(CnC::serializer * ser);
	void processItemPut(CnC::serializer * ser, void * itemId);
	void processItemGet(CnC::serializer * ser, ItemCheckpoint_i* ich, void* tag);


	TagLog& getTagLog( StepTag & tag );

	bool isDone(StepTag tag);

	int getId();

	void decrement_get_counts();


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
void StepCheckpoint< StepTag >::processStepDone(StepTag step, int stepColId, int puts, int prescribes, int gets)
{
	TagLog& l_ = getTagLog( step );
	l_.processDone( puts, prescribes, gets );
}

template< class StepTag >
void StepCheckpoint< StepTag >::processItemGet(StepTag getter, ItemCheckpoint_i* ich, void * tag) {
	TagLog& l_ = getTagLog( getter );
	l_.processGet(ich, tag);
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
	int puts, prescribes, gets;
	(* ser) & step & puts & prescribes & gets;
	TagLog& l_ = getTagLog( step );
	l_.processDone( puts, prescribes, gets );
}

template< class StepTag >
void StepCheckpoint< StepTag >::processItemGet(CnC::serializer * ser, ItemCheckpoint_i* ich, void* tag) {
	StepTag getter;
	(* ser) & getter;
	TagLog& l_ = getTagLog( getter );
	l_.processGet(ich, tag);
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
TagLog& StepCheckpoint< StepTag >::getTagLog( StepTag& tag ) {
	typename tagMap_t::iterator it = tagMap.find(tag);
	if (it == tagMap.end()) {
		TagLog & l_ = tagMap[tag] = TagLog();
		return l_;
	} else {
		TagLog & l_ = it->second;
		return l_;
	}
}

template<class StepTag >
void StepCheckpoint< StepTag >::decrement_get_counts() {
	for (typename tagMap_t::iterator it = tagMap.begin(); it != tagMap.end(); ++it) {
		if ((it->second).isDone()) (it->second).decrement_get_counts();
	}
}



#endif /* STEPCHECKPOINT_H_ */
