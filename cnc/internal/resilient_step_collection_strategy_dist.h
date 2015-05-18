#ifndef _RESILIENT_STEP_COLLECTION_DIST_H_
#define _RESILIENT_STEP_COLLECTION_DIST_H_

#include <cnc/internal/checkpointingsystem/StepCheckpoint_i.h>
#include "checkpointingsystem/checkpointingsystem_dist/StepCheckpointDist.h"

namespace CnC {

template< typename ResilientStepCollection, typename Tag >
class resilient_step_collection_strategy_dist:
		public resilient_step_collection_strategy_i< resilient_step_collection_strategy_dist< ResilientStepCollection, Tag >, Tag >,
		public virtual CnC::Internal::distributable
{
public:

	resilient_step_collection_strategy_dist(ResilientStepCollection & resilient_step_collection);
	~resilient_step_collection_strategy_dist();

	void processPut(
			Tag putter,
			ItemCheckpoint_i * item_cp,
			void * itemid);

	void processPrescribe(
			Tag prescriber,
			TagCheckpoint_i * tag_cp,
			void * tagid);

	void processGet(
			Tag getter,
			ItemCheckpoint_i * item_cp,
			void* tag);

	void processStepDone(
			Tag step,
			int stepColId,
			int puts,
			int prescribes,
			int gets);

	StepCheckpoint_i * getStepCheckpoint();


	void recv_msg( serializer * ser );
	void unsafe_reset( bool dist );

private:
	typedef Internal::distributable_context dist_context;

	ResilientStepCollection & m_resilient_step_collection;
	StepCheckpointDist< Tag > m_step_checkpoint;

	static const char DONE = 0;

};


template< typename ResilientStepCollection, typename Tag >
const char resilient_step_collection_strategy_dist< ResilientStepCollection, Tag >::resilient_step_collection_strategy_dist::DONE;


template< typename ResilientStepCollection, typename Tag >
resilient_step_collection_strategy_dist< ResilientStepCollection, Tag >::resilient_step_collection_strategy_dist(ResilientStepCollection & resilient_step_collection):
	resilient_step_collection_strategy_i< resilient_step_collection_strategy_dist< ResilientStepCollection, Tag >, Tag >(),
	m_resilient_step_collection(resilient_step_collection),
	m_step_checkpoint(resilient_step_collection.getId())
{
	m_resilient_step_collection.getContext().subscribe(this);
	m_resilient_step_collection.getContext().registerStepCheckPoint( &m_step_checkpoint );
}

template< typename ResilientStepCollection, typename Tag >
resilient_step_collection_strategy_dist< ResilientStepCollection, Tag >::~resilient_step_collection_strategy_dist()
{
	m_resilient_step_collection.getContext().unsubscribe(this);
}

template< typename ResilientStepCollection, typename Tag >
void resilient_step_collection_strategy_dist< ResilientStepCollection, Tag >::processPut(
		Tag putter,
		ItemCheckpoint_i * item_cp,
		void * itemid)
{
	m_step_checkpoint.processItemPut( putter, item_cp, itemid);
}

template< typename ResilientStepCollection, typename Tag >
void resilient_step_collection_strategy_dist< ResilientStepCollection, Tag >::processPrescribe(
		Tag prescriber,
		TagCheckpoint_i * tag_cp,
		void * tagid)
{
	m_step_checkpoint.processStepPrescribe( prescriber, tag_cp, tagid);
}

template< typename ResilientStepCollection, typename Tag >
void resilient_step_collection_strategy_dist< ResilientStepCollection, Tag >::processGet(
		Tag getter,
		ItemCheckpoint_i * item_cp,
		void* tag)
{
	m_step_checkpoint.processItemGet( getter, item_cp, tag );
}

template< typename ResilientStepCollection, typename Tag >
void resilient_step_collection_strategy_dist< ResilientStepCollection, Tag >::processStepDone(
		Tag step,
		int stepColId,
		int puts,
		int prescribes,
		int gets)
{
    m_step_checkpoint.processStepDone( step, stepColId, puts, prescribes, gets);
    m_resilient_step_collection.getContext().checkForCrash();
	if (m_resilient_step_collection.getContext().hasTimePassed() && Internal::distributor::myPid() != 0){
		m_resilient_step_collection.getContext().calculateAndSendCheckpoint();
		m_resilient_step_collection.getContext().resetTimer();
	}
}

template< typename ResilientStepCollection, typename Tag >
StepCheckpoint_i * resilient_step_collection_strategy_dist< ResilientStepCollection, Tag >::getStepCheckpoint() {
	return &m_step_checkpoint;
}


template< typename ResilientStepCollection, typename Tag >
void resilient_step_collection_strategy_dist< ResilientStepCollection, Tag >::recv_msg( serializer * ser )
{
	char msg_tag;
	(* ser) & msg_tag;

	switch (msg_tag) {
		case DONE:
		{
			int step_collection_id;
			(* ser) & step_collection_id;
			m_step_checkpoint.processStepDone(ser);
			if ( Internal::distributor::myPid() == 0) {
				m_resilient_step_collection.getContext().checkForCrash();
			}
			break;
		}

		default:
			CNC_ABORT( "Protocol error: unexpected message tag." );
		}
}

template< typename ResilientStepCollection, typename Tag >
void resilient_step_collection_strategy_dist< ResilientStepCollection, Tag >::unsafe_reset( bool dist ) {}




} //End namespace CnC



#endif // _RESILIENT_STEP_COLLECTION_DIST_H_
