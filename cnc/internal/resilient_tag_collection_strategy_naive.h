#ifndef _RESILIENT_TAG_COLLECTION_NAIVE_H_
#define _RESILIENT_TAG_COLLECTION_NAIVE_H_


namespace CnC {

template< typename ResilientTagCollection, typename Tag >
class resilient_tag_collection_strategy_naive:
		public resilient_tag_collection_strategy_i< resilient_tag_collection_strategy_naive< ResilientTagCollection, Tag >, Tag >,
		public virtual CnC::Internal::distributable
{
public:

	resilient_tag_collection_strategy_naive(ResilientTagCollection & resilient_tag_collection);
	~resilient_tag_collection_strategy_naive();

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
const char resilient_tag_collection_strategy_naive< ResilientTagCollection, Tag >::resilient_tag_collection_strategy_naive::PRESCRIBE;


template< typename ResilientTagCollection, typename Tag >
resilient_tag_collection_strategy_naive< ResilientTagCollection, Tag >::resilient_tag_collection_strategy_naive(ResilientTagCollection & resilient_tag_collection):
	resilient_tag_collection_strategy_i< resilient_tag_collection_strategy_naive< ResilientTagCollection, Tag >, Tag >(),
	m_resilient_tag_collection(resilient_tag_collection),
	m_tag_checkpoint(resilient_tag_collection, resilient_tag_collection.getId())
{
	m_resilient_tag_collection.getContext().subscribe(this);
	m_resilient_tag_collection.getContext().registerTagCheckpoint( &m_tag_checkpoint );
}

template< typename ResilientTagCollection, typename Tag >
resilient_tag_collection_strategy_naive< ResilientTagCollection, Tag >::~resilient_tag_collection_strategy_naive()
{
	m_resilient_tag_collection.getContext().unsubscribe(this);
}


template< typename ResilientTagCollection, typename Tag >
void resilient_tag_collection_strategy_naive< ResilientTagCollection, Tag >::processPrescribe(
		const Tag & t)
{
	void * itemid = m_tag_checkpoint.put( t );
}

template< typename ResilientTagCollection, typename Tag >
template < typename UserStepTag, typename PutterCollection >
void resilient_tag_collection_strategy_naive< ResilientTagCollection, Tag >::processPrescribe(
		const UserStepTag & putter,
		PutterCollection & putterColl,
		const Tag & t)
{
	if ( Internal::distributor::myPid() == 0) {
		//if (!putterColl.isStepDone(const_cast<UserStepTag&>(putter))) {
			void * tagid = m_tag_checkpoint.put( t );
			putterColl.processPrescribe( putter, tagid, m_resilient_tag_collection.getId());
		//}
		} else {
		serializer * ser = m_resilient_tag_collection.getContext().new_serializer( this );
		//Order is very important since we pass the serialized datastrc to the remote checkpoint object!
		(*ser) & PRESCRIBE & t & putterColl.getId() & putter;
		m_resilient_tag_collection.getContext().send_msg(ser, 0);
	}
}

template< typename ResilientTagCollection, typename Tag >
template < typename StepCheckpoint >
void resilient_tag_collection_strategy_naive< ResilientTagCollection, Tag >::prescribeStepCheckpoint( StepCheckpoint & s )
{
	m_tag_checkpoint.prescribeStepCheckpoint(s);
}


template< typename ResilientTagCollection, typename Tag >
void resilient_tag_collection_strategy_naive< ResilientTagCollection, Tag >::recv_msg( serializer * ser )
{
	char msg_tag;
	(* ser) & msg_tag;

	switch (msg_tag) {
		case PRESCRIBE:
		{
			Tag tag;
			int prescriber_collection_id;
			(* ser) & tag & prescriber_collection_id;
			//first copy ser obj
			//serializer ser_cpy = *ser;
			StepCheckpoint_i* i_ = m_resilient_tag_collection.getContext().getStepCheckPoint(prescriber_collection_id);
			//if (!i_->isDone(&ser_cpy)) {
				void * tagid = m_tag_checkpoint.put( tag );
				i_->processStepPrescribe(ser, tagid);
			//}

			break;
		}

		default:
			CNC_ABORT( "Protocol error: unexpected message tag." );
		}
}

template< typename ResilientTagCollection, typename Tag >
void resilient_tag_collection_strategy_naive< ResilientTagCollection, Tag >::unsafe_reset( bool dist ) {}




} //End namespace CnC



#endif // _RESILIENT_TAG_COLLECTION_NAIVE_H_
