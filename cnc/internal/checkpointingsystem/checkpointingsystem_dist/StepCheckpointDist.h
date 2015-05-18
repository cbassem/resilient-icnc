/*
 * StepCheckpoint.h
 *
 *  Created on: Apr 28, 2015
 *      Author: blackline
 */

#ifndef STEPCHECKPOINT_DIST_H_
#define STEPCHECKPOINT_DIST_H_

#include "../StepCheckpoint_i.h"
#include <tr1/unordered_map>
#include "tbb/concurrent_hash_map.h"

#include "TagLogDist.h"
//#include <cnc/serializer.h>

namespace CnC {

template< class StepTag >
class StepCheckpointDist: public StepCheckpoint_i {
public:
	StepCheckpointDist(int col_);
	virtual ~StepCheckpointDist();

	void processStepPrescribe(StepTag prescriber, int prescriberColId, void * prescribedTagId, int tagCollectionId);
	void processStepDone(StepTag step, int stepColId, int puts, int prescribes, int gets);
	void processItemPut(StepTag producer, int stepProducerColId, void * itemId, int itemColId);
	void processItemGet(StepTag getter, ItemCheckpoint_i* ich, void * tag);

	//The serializer still contains the responsible one's tag, but this is ok since we know its type! :D
	void processStepPrescribe(CnC::serializer * ser, void * prescribedTagId);
	void processStepDone(CnC::serializer * ser);
	void processItemPut(CnC::serializer * ser, void * itemId);
	void processItemGet(CnC::serializer * ser, ItemCheckpoint_i* ich, void* tag);


	TagLogDist<StepTag>* getTagLog( StepTag & tag );

	bool isDone(StepTag tag);
	bool isDone(CnC::serializer * ser);

	int getId();

	void decrement_get_counts();

	void cleanup();

	void sendNotDone();


private:
	typedef tbb::concurrent_hash_map< StepTag, TagLogDist<StepTag>* > tagMap_t;
	typedef tbb::scalable_allocator< TagLogDist<StepTag> > taglog_allocator_type;
	tagMap_t tagMap;
	int m_col_id;
	taglog_allocator_type allocator;

	void uncreate_taglog( TagLogDist<StepTag> * t );
	TagLogDist<StepTag> * create_taglog( const TagLogDist<StepTag> & org );

};

template< class StepTag >
StepCheckpointDist< StepTag >::StepCheckpointDist(int col_id): tagMap(), m_col_id(col_id), allocator() {};

template< class StepTag >
StepCheckpointDist< StepTag >::~StepCheckpointDist() {
	cleanup();
}

template< class StepTag >
void StepCheckpointDist< StepTag >::processStepPrescribe(StepTag prescriber, int prescriberColId, void * prescribedTagId, int tagCollectionId)
{
	TagLogDist<StepTag>* l_ = getTagLog( prescriber );
	l_->processPrescribe( prescribedTagId );
}

template< class StepTag >
void StepCheckpointDist< StepTag >::processStepDone(StepTag step, int stepColId, int puts, int prescribes, int gets)
{
	TagLogDist<StepTag>* l_ = getTagLog( step );
	l_->processDone( puts, prescribes, gets );
}

template< class StepTag >
void StepCheckpointDist< StepTag >::processItemGet(StepTag getter, ItemCheckpoint_i* ich, void * tag) {
	TagLogDist<StepTag>* l_ = getTagLog( getter );
	l_->processGet(ich, tag);
}

template< class StepTag >
void StepCheckpointDist< StepTag >::processItemPut(StepTag producer, int stepProducerColId, void * itemId, int itemColId)
{
	TagLogDist<StepTag>* l_ = getTagLog( producer );
	l_->processPut( itemId );
}

template< class StepTag >
void StepCheckpointDist< StepTag >::processStepPrescribe(CnC::serializer * ser, void * prescribedTagId)
{
	StepTag prescriber;
	(* ser) & prescriber;
	TagLogDist<StepTag>* l_ = getTagLog( prescriber );
	l_->processPrescribe( prescribedTagId );
}

template< class StepTag >
void StepCheckpointDist< StepTag >::processStepDone(CnC::serializer * ser)
{
	StepTag step;
	int puts, prescribes, gets;
	(* ser) & step & puts & prescribes & gets;
	TagLogDist<StepTag>* l_ = getTagLog( step );
	l_->processDone( puts, prescribes, gets );
}

template< class StepTag >
void StepCheckpointDist< StepTag >::processItemGet(CnC::serializer * ser, ItemCheckpoint_i* ich, void* tag) {
	StepTag getter;
	(* ser) & getter;
	TagLogDist<StepTag>* l_ = getTagLog( getter );
	l_->processGet(ich, tag);
}

template< class StepTag >
void StepCheckpointDist< StepTag >::processItemPut(CnC::serializer * ser, void * itemId)
{
	StepTag producer;
	(* ser) & producer;
	TagLogDist<StepTag>* l_ = getTagLog( producer );
	l_->processPut( itemId );
}

template< class StepTag >
bool StepCheckpointDist< StepTag >::isDone(StepTag tag)
{
	TagLogDist<StepTag>* l_ = getTagLog( tag );
	return l_->isDone();
}

template< class StepTag >
bool StepCheckpointDist< StepTag >::isDone(CnC::serializer * ser)
{
	StepTag producer;
	(* ser) & producer;
	TagLogDist<StepTag>* l_ = getTagLog( producer );
	return l_->isDone();
}


template< class StepTag >
int StepCheckpointDist< StepTag >::getId()
{
	return m_col_id;
}


template<class StepTag >
TagLogDist<StepTag>* StepCheckpointDist< StepTag >::getTagLog( StepTag& tag ) {
	typename tagMap_t::accessor _accr;
	bool inserted = tagMap.insert(_accr, tag);
	if (inserted) {
		_accr->second = create_taglog(TagLogDist<StepTag>(m_col_id, tag));
		return _accr->second;
	} else {
		return _accr->second;
	}
}

template<class StepTag >
void StepCheckpointDist< StepTag >::decrement_get_counts()
{
	for (typename tagMap_t::iterator it = tagMap.begin(); it != tagMap.end(); ++it) {
		if ((it->second)->isDone()) (it->second)->decrement_get_counts();
	}
}

template<class StepTag >
void StepCheckpointDist< StepTag >::uncreate_taglog( TagLogDist<StepTag> * t )
{
    if( t ) {
    	allocator.destroy( t );
        allocator.deallocate( t, 1 );
    }
}

template<class StepTag >
TagLogDist<StepTag> * StepCheckpointDist< StepTag >::create_taglog( const TagLogDist<StepTag> & org )
{
	TagLogDist<StepTag> * _item = allocator.allocate( 1 );
    allocator.construct( _item, org );

    return _item;
}


template<class StepTag >
void StepCheckpointDist< StepTag >::cleanup()
{
	for (typename tagMap_t::iterator it = tagMap.begin(); it != tagMap.end(); ++it) {
		uncreate_taglog(it->second);
	}
}

template<class StepTag >
void StepCheckpointDist< StepTag >::sendNotDone() {
	for (typename tagMap_t::iterator it = tagMap.begin(); it != tagMap.end(); ++it) {
		if ((it->second)->isDone())
		{
			(it->second)->send_unprocessed_prescribes();
//			(it->second)->send_unprocessed_puts();
//			(it->second)->send_unprocessed_gets();
		}
	}
}

}//END CNC SCOPE


#endif /* STEPCHECKPOINT_DIST_H_ */
