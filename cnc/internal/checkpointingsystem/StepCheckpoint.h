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
#include "tbb/concurrent_hash_map.h"
//#include <cnc/serializer.h>

namespace CnC {

template< class StepTag >
class StepCheckpoint: public StepCheckpoint_i {
public:
	StepCheckpoint(int col_);
	virtual ~StepCheckpoint();

	void processStepPrescribe(StepTag prescriber, TagCheckpoint_i* chp, void * prescribedTagId);
	void processStepDone(StepTag step, int stepColId, int puts, int prescribes, int gets);
	void processItemPut(StepTag producer, ItemCheckpoint_i* ich, void * itemId);
	void processItemGet(StepTag getter, ItemCheckpoint_i* ich, void * tag);

	//The serializer still contains the responsible one's tag, but this is ok since we know its type! :D
	void processStepPrescribe(CnC::serializer * ser, TagCheckpoint_i* ich, void * prescribedTagId);
	void processStepDone(CnC::serializer * ser);
	void processItemPut(CnC::serializer * ser, ItemCheckpoint_i* ich, void * itemId);
	void processItemGet(CnC::serializer * ser, ItemCheckpoint_i* ich, void* tag);


	TagLog* getTagLog( StepTag & tag );

	bool isDone(StepTag tag);
	bool isDone(CnC::serializer * ser);

	int getId();

	void decrement_get_counts();

	void cleanup();

	void sendNotDone() {}; //TODO factor out


private:
	typedef tbb::concurrent_hash_map< StepTag, TagLog* > tagMap_t;
	typedef tbb::scalable_allocator< TagLog > taglog_allocator_type;
	tagMap_t tagMap;
	int m_col_id;
	taglog_allocator_type allocator;

	void uncreate_taglog( TagLog * t );
	TagLog * create_taglog( const TagLog & org );

};

template< class StepTag >
StepCheckpoint< StepTag >::StepCheckpoint(int col_id): tagMap(), m_col_id(col_id), allocator() {};

template< class StepTag >
StepCheckpoint< StepTag >::~StepCheckpoint() {
	cleanup();
}

template< class StepTag >
void StepCheckpoint< StepTag >::processStepPrescribe(StepTag prescriber, TagCheckpoint_i*, void * prescribedTagId)
{
	TagLog* l_ = getTagLog( prescriber );
	l_->processPrescribe( prescribedTagId );
}

template< class StepTag >
void StepCheckpoint< StepTag >::processStepDone(StepTag step, int stepColId, int puts, int prescribes, int gets)
{
	TagLog* l_ = getTagLog( step );
	l_->processDone( puts, prescribes, gets );
}

template< class StepTag >
void StepCheckpoint< StepTag >::processItemGet(StepTag getter, ItemCheckpoint_i* ich, void * tag) {
	TagLog* l_ = getTagLog( getter );
	l_->processGet(ich, tag);
}

template< class StepTag >
void StepCheckpoint< StepTag >::processItemPut(StepTag producer, ItemCheckpoint_i* ich, void * itemId)
{
	TagLog* l_ = getTagLog( producer );
	l_->processPut( itemId );
}

template< class StepTag >
void StepCheckpoint< StepTag >::processStepPrescribe(CnC::serializer * ser, TagCheckpoint_i* ich, void * prescribedTagId)
{
	StepTag prescriber;
	(* ser) & prescriber;
	TagLog* l_ = getTagLog( prescriber );
	l_->processPrescribe( prescribedTagId );
}

template< class StepTag >
void StepCheckpoint< StepTag >::processStepDone(CnC::serializer * ser)
{
	StepTag step;
	int puts, prescribes, gets;
	(* ser) & step & puts & prescribes & gets;
	TagLog* l_ = getTagLog( step );
	l_->processDone( puts, prescribes, gets );
}

template< class StepTag >
void StepCheckpoint< StepTag >::processItemGet(CnC::serializer * ser, ItemCheckpoint_i* ich, void* tag) {
	StepTag getter;
	(* ser) & getter;
	TagLog* l_ = getTagLog( getter );
	l_->processGet(ich, tag);
}

template< class StepTag >
void StepCheckpoint< StepTag >::processItemPut(CnC::serializer * ser, ItemCheckpoint_i* ich, void * itemId)
{
	StepTag producer;
	(* ser) & producer;
	TagLog* l_ = getTagLog( producer );
	l_->processPut( itemId );
}

template< class StepTag >
bool StepCheckpoint< StepTag >::isDone(StepTag tag)
{
	TagLog* l_ = getTagLog( tag );
	return l_->isDone();
}

template< class StepTag >
bool StepCheckpoint< StepTag >::isDone(CnC::serializer * ser)
{
	StepTag producer;
	(* ser) & producer;
	TagLog* l_ = getTagLog( producer );
	return l_->isDone();
}


template< class StepTag >
int StepCheckpoint< StepTag >::getId()
{
	return m_col_id;
}


template<class StepTag >
TagLog* StepCheckpoint< StepTag >::getTagLog( StepTag& tag ) {
	typename tagMap_t::accessor _accr;
	bool inserted = tagMap.insert(_accr, tag);
	if (inserted) {
		_accr->second = create_taglog(TagLog());
		return _accr->second;
	} else {
		return _accr->second;
	}
}

template<class StepTag >
void StepCheckpoint< StepTag >::decrement_get_counts()
{
	for (typename tagMap_t::iterator it = tagMap.begin(); it != tagMap.end(); ++it) {
		if ((it->second)->isDone()) (it->second)->decrement_get_counts();
	}
}

template<class StepTag >
void StepCheckpoint< StepTag >::uncreate_taglog( TagLog * t )
{
    if( t ) {
    	allocator.destroy( t );
        allocator.deallocate( t, 1 );
    }
}

template<class StepTag >
TagLog * StepCheckpoint< StepTag >::create_taglog( const TagLog & org )
{
    TagLog * _item = allocator.allocate( 1 );
    allocator.construct( _item, org );
    return _item;
}


template<class StepTag >
void StepCheckpoint< StepTag >::cleanup()
{
	for (typename tagMap_t::iterator it = tagMap.begin(); it != tagMap.end(); ++it) {
		uncreate_taglog(it->second);
	}
}

}//END CNC SCOPE


#endif /* STEPCHECKPOINT_H_ */
