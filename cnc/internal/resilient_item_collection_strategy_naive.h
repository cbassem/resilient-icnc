#ifndef _RESILIENT_ITEM_COLLECTION_NAIVE_H_
#define _RESILIENT_ITEM_COLLECTION_NAIVE_H_


namespace CnC {

template< typename ResilientItemCollection, typename Key, typename Item >
class resilient_item_collection_strategy_naive:
		public resilient_item_collection_strategy_i< resilient_item_collection_strategy_naive< ResilientItemCollection, Key, Item >, Key, Item >,
		public virtual CnC::Internal::distributable
{
public:

	resilient_item_collection_strategy_naive(ResilientItemCollection & resilient_item_collection);
	~resilient_item_collection_strategy_naive();

	void processPut(
			const Key & t,
			const Item & i);

	template < typename UserStepTag, typename PutterCollection >
	void processPut(
			const UserStepTag & putter,
			PutterCollection & putterColl,
			const Key & t,
			const Item & i);

	template < typename UserStepTag, typename GetterCollection >
	void processGet(
			const UserStepTag & getter,
    		GetterCollection & getterColl,
			const Key & tag,
			Item & item);

	void recv_msg( serializer * ser );
	void unsafe_reset( bool dist );

private:
	typedef Internal::distributable_context dist_context;

	ResilientItemCollection & m_resilient_item_collection;
	ItemCheckpoint< ResilientItemCollection, Key, Item > m_item_checkpoint;

	static const char PUT = 0;
	static const char GET = 1;

};

template< typename ResilientItemCollection, typename Key, typename Item >
const char resilient_item_collection_strategy_naive< ResilientItemCollection, Key, Item >::resilient_item_collection_strategy_naive::PUT;

template< typename ResilientItemCollection, typename Key, typename Item >
const char resilient_item_collection_strategy_naive< ResilientItemCollection, Key, Item >::resilient_item_collection_strategy_naive::GET;

template< typename ResilientItemCollection, typename Key, typename Item >
resilient_item_collection_strategy_naive< ResilientItemCollection, Key, Item >::resilient_item_collection_strategy_naive(ResilientItemCollection & resilient_item_collection):
	resilient_item_collection_strategy_i< resilient_item_collection_strategy_naive< ResilientItemCollection, Key, Item >, Key, Item >(),
	m_resilient_item_collection(resilient_item_collection),
	m_item_checkpoint(resilient_item_collection, resilient_item_collection.getId())
{
	m_resilient_item_collection.getContext().subscribe(this);
	m_resilient_item_collection.getContext().registerItemCheckpoint( &m_item_checkpoint );
}

template< typename ResilientItemCollection, typename Key, typename Item >
resilient_item_collection_strategy_naive< ResilientItemCollection, Key, Item >::~resilient_item_collection_strategy_naive()
{
	m_resilient_item_collection.getContext().unsubscribe(this);
}

template< typename ResilientItemCollection, typename Key, typename Item >
void resilient_item_collection_strategy_naive< ResilientItemCollection, Key, Item >::processPut(
		const Key & t,
		const Item & i)
{
	void * itemid = m_item_checkpoint.put( t, i );
}

template< typename ResilientItemCollection, typename Key, typename Item >
template < typename UserStepTag, typename PutterCollection >
void resilient_item_collection_strategy_naive< ResilientItemCollection, Key, Item >::processPut(
		const UserStepTag & putter,
		PutterCollection & putterColl,
		const Key & t,
		const Item & i)
{
	if ( Internal::distributor::myPid() == 0) {
		void * itemid = m_item_checkpoint.put( t, i );
		putterColl.processPut(putter, itemid, m_resilient_item_collection.getId());
	} else {
		serializer * ser = m_resilient_item_collection.getContext().new_serializer( this );
		//Order is very important since we pass the serialized datastrc to the remote checkpoint object!
		(*ser) & resilient_item_collection_strategy_naive::PUT & t & i & putterColl.getId() & putter;
		m_resilient_item_collection.getContext().send_msg(ser, 0);
	}

}

template< typename ResilientItemCollection, typename Key, typename Item >
template < typename UserStepTag, typename GetterCollection >
void resilient_item_collection_strategy_naive< ResilientItemCollection, Key, Item >::processGet(
		const UserStepTag & getter,
		GetterCollection & getterColl,
		const Key & tag,
		Item & item)
{
	if ( Internal::distributor::myPid() == 0) {
		void * t_ = m_item_checkpoint.getKeyId(tag);
		getterColl.processGet(getter, &m_item_checkpoint, t_);
	} else {
		serializer * ser = m_resilient_item_collection.getContext().new_serializer( this );
		//Order is very important since we pass the serialized datastrc to the remote checkpoint object!
		(*ser) & resilient_item_collection_strategy_naive::GET & tag & getterColl.getId() & getter;
		m_resilient_item_collection.getContext().send_msg(ser, 0);
	}
}

template< typename ResilientItemCollection, typename Key, typename Item >
void resilient_item_collection_strategy_naive< ResilientItemCollection, Key, Item >::recv_msg( serializer * ser )
{
	char msg_tag;
	(* ser) & msg_tag;

	switch (msg_tag) {
		case resilient_item_collection_strategy_naive::PUT:
		{
			Key tag;
			Item item;
			int putter_collection_id;
			(* ser) & tag & item & putter_collection_id;
			void * itemid = m_item_checkpoint.put( tag, item );
			//Get the step that made the item put
			StepCheckpoint_i* i_ = m_resilient_item_collection.getContext().getStepCheckPoint(putter_collection_id);
			i_->processItemPut(ser, itemid);
			break;
		}
		case resilient_item_collection_strategy_naive::GET:
		{
			Key tag;
			int getter_collection_id;
			(* ser) & tag & getter_collection_id;
			StepCheckpoint_i* i_ = m_resilient_item_collection.getContext().getStepCheckPoint(getter_collection_id);
			void * t_ = m_item_checkpoint.getKeyId(tag);
			i_->processItemGet(ser, &m_item_checkpoint, t_);
			break;
		}

		default:
			CNC_ABORT( "Protocol error: unexpected message tag." );
		}

}

template< typename ResilientItemCollection, typename Key, typename Item >
void resilient_item_collection_strategy_naive< ResilientItemCollection, Key, Item >::unsafe_reset( bool dist ) {}




} //End namespace CnC



#endif // _RESILIENT_ITEM_COLLECTION_NAIVE_H_
