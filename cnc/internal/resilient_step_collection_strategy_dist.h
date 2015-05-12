#ifndef _RESILIENT_STEP_COLLECTION_DIST_H_
#define _RESILIENT_STEP_COLLECTION_DIST_H_


namespace CnC {

template< typename ResilientStepCollection, typename Tag >
class resilient_step_collection_strategy_naive:
		public resilient_step_collection_strategy_i< resilient_step_collection_strategy_naive< ResilientStepCollection, Tag >, Tag >,
		public virtual CnC::Internal::distributable
{
public:

	resilient_step_collection_strategy_naive(ResilientStepCollection & resilient_step_collection);
	~resilient_step_collection_strategy_naive();

	void processPut(
			Tag putter,
			int stepProducerColId,
			void * itemid,
			int itemCollectionId);

	void processPrescribe(
			Tag prescriber,
			int prescriberColId,
			void * tagid,
			int tagCollectionId);

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

	StepCheckpoint<Tag> * getStepCheckpoint();


	void recv_msg( serializer * ser );
	void unsafe_reset( bool dist );

private:
	typedef Internal::distributable_context dist_context;

	ResilientStepCollection & m_resilient_step_collection;
	StepCheckpoint< Tag > m_step_checkpoint;

	static const char DONE = 0;

};


template< typename ResilientStepCollection, typename Tag >
const char resilient_step_collection_strategy_naive< ResilientStepCollection, Tag >::resilient_step_collection_strategy_naive::DONE;


template< typename ResilientStepCollection, typename Tag >
resilient_step_collection_strategy_naive< ResilientStepCollection, Tag >::resilient_step_collection_strategy_naive(ResilientStepCollection & resilient_step_collection):
	resilient_step_collection_strategy_i< resilient_step_collection_strategy_naive< ResilientStepCollection, Tag >, Tag >(),
	m_resilient_step_collection(resilient_step_collection),
	m_step_checkpoint(resilient_step_collection.getId())
{
	m_resilient_step_collection.getContext().subscribe(this);
	m_resilient_step_collection.getContext().registerStepCheckPoint( &m_step_checkpoint );
}

template< typename ResilientStepCollection, typename Tag >
resilient_step_collection_strategy_naive< ResilientStepCollection, Tag >::~resilient_step_collection_strategy_naive()
{
	m_resilient_step_collection.getContext().unsubscribe(this);
}

template< typename ResilientStepCollection, typename Tag >
void resilient_step_collection_strategy_naive< ResilientStepCollection, Tag >::processPut(
		Tag putter,
		int stepProducerColId,
		void * itemid,
		int itemCollectionId)
{
	m_step_checkpoint.processItemPut( putter, stepProducerColId, itemid, itemCollectionId);
}

template< typename ResilientStepCollection, typename Tag >
void resilient_step_collection_strategy_naive< ResilientStepCollection, Tag >::processPrescribe(
		Tag prescriber,
		int prescriberColId,
		void * tagid,
		int tagCollectionId)
{
	m_step_checkpoint.processStepPrescribe( prescriber, prescriberColId, tagid , tagCollectionId);
}

template< typename ResilientStepCollection, typename Tag >
void resilient_step_collection_strategy_naive< ResilientStepCollection, Tag >::processGet(
		Tag getter,
		ItemCheckpoint_i * item_cp,
		void* tag)
{
	m_step_checkpoint.processItemGet( getter, item_cp, tag );
}

template< typename ResilientStepCollection, typename Tag >
void resilient_step_collection_strategy_naive< ResilientStepCollection, Tag >::processStepDone(
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
CnC::StepCheckpoint<Tag> * resilient_step_collection_strategy_naive< ResilientStepCollection, Tag >::getStepCheckpoint() {
	return &m_step_checkpoint;
}


template< typename ResilientStepCollection, typename Tag >
void resilient_step_collection_strategy_naive< ResilientStepCollection, Tag >::recv_msg( serializer * ser )
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
void resilient_step_collection_strategy_naive< ResilientStepCollection, Tag >::unsafe_reset( bool dist ) {}




} //End namespace CnC



#endif // _RESILIENT_STEP_COLLECTION_DIST_H_
