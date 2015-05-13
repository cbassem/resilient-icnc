#ifndef _RESILIENT_TAG_COLLECTION_DIST_H_
#define _RESILIENT_TAG_COLLECTION_DIST_H_

#include <cnc/internal/checkpointingsystem/TagCheckpoint.h>
#include <cnc/internal/checkpointingsystem/TagCheckpoint_i.h>

namespace CnC {

template< typename ResilientTagCollection, typename Tag >
class resilient_tag_collection_strategy_dist:
		public resilient_tag_collection_strategy_i< resilient_tag_collection_strategy_dist< ResilientTagCollection, Tag >, Tag >,
		public virtual CnC::Internal::distributable
{
public:

	resilient_tag_collection_strategy_dist(ResilientTagCollection & resilient_tag_collection);
	~resilient_tag_collection_strategy_dist();

	void processPrescribe(
			const Tag & t);

	template < typename UserStepTag, typename PutterCollection >
	void processPrescribe(
			const UserStepTag & putter,
			PutterCollection & putterColl,
			const Tag & t);

	template < typename StepCheckpoint >
	void prescribeStepCheckpoint( StepCheckpoint & s );

	void recv_msg( serializer * ser );
	void unsafe_reset( bool dist );

private:
	typedef Internal::distributable_context dist_context;

	ResilientTagCollection & m_resilient_tag_collection;
	TagCheckpoint< ResilientTagCollection, Tag > m_tag_checkpoint;

	static const char PRESCRIBE = 0;

};


template< typename ResilientTagCollection, typename Tag >
const char resilient_tag_collection_strategy_dist< ResilientTagCollection, Tag >::resilient_tag_collection_strategy_dist::PRESCRIBE;


template< typename ResilientTagCollection, typename Tag >
resilient_tag_collection_strategy_dist< ResilientTagCollection, Tag >::resilient_tag_collection_strategy_dist(ResilientTagCollection & resilient_tag_collection):
	resilient_tag_collection_strategy_i< resilient_tag_collection_strategy_dist< ResilientTagCollection, Tag >, Tag >(),
	m_resilient_tag_collection(resilient_tag_collection),
	m_tag_checkpoint(resilient_tag_collection, resilient_tag_collection.getId())
{
	m_resilient_tag_collection.getContext().subscribe(this);
	m_resilient_tag_collection.getContext().registerTagCheckpoint( &m_tag_checkpoint );
}

template< typename ResilientTagCollection, typename Tag >
resilient_tag_collection_strategy_dist< ResilientTagCollection, Tag >::~resilient_tag_collection_strategy_dist()
{
	m_resilient_tag_collection.getContext().unsubscribe(this);
}


template< typename ResilientTagCollection, typename Tag >
void resilient_tag_collection_strategy_dist< ResilientTagCollection, Tag >::processPrescribe(
		const Tag & t)
{
	void * itemid = m_tag_checkpoint.put( t );
}

template< typename ResilientTagCollection, typename Tag >
template < typename UserStepTag, typename PutterCollection >
void resilient_tag_collection_strategy_dist< ResilientTagCollection, Tag >::processPrescribe(
		const UserStepTag & putter,
		PutterCollection & putterColl,
		const Tag & t)
{
	void * tagid = m_tag_checkpoint.put( t );
	putterColl.processPrescribe( putter, tagid, m_resilient_tag_collection.getId());
}

template< typename ResilientTagCollection, typename Tag >
template < typename StepCheckpoint >
void resilient_tag_collection_strategy_dist< ResilientTagCollection, Tag >::prescribeStepCheckpoint( StepCheckpoint & s )
{
	m_tag_checkpoint.prescribeStepCheckpoint(s);
}


template< typename ResilientTagCollection, typename Tag >
void resilient_tag_collection_strategy_dist< ResilientTagCollection, Tag >::recv_msg( serializer * ser )
{
	char msg_tag;
	(* ser) & msg_tag;

	switch (msg_tag) {
		case PRESCRIBE:
		{
			Tag tag;
			int prescriber_collection_id;
			(* ser) & tag & prescriber_collection_id;
			void * tagid = m_tag_checkpoint.put( tag );
			StepCheckpoint_i* i_ = m_resilient_tag_collection.getContext().getStepCheckPoint(prescriber_collection_id);
			i_->processStepPrescribe(ser, tagid);
			break;
		}

		default:
			CNC_ABORT( "Protocol error: unexpected message tag." );
		}
}

template< typename ResilientTagCollection, typename Tag >
void resilient_tag_collection_strategy_dist< ResilientTagCollection, Tag >::unsafe_reset( bool dist ) {}




} //End namespace CnC



#endif // _RESILIENT_TAG_COLLECTION_DIST_H_
