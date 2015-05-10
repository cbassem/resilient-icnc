#ifndef _RESILIENT_ITEM_COLLECTION_I_H_
#define _RESILIENT_ITEM_COLLECTION_I_H_


template< typename Derived, typename Tag, typename Item >
class resilient_item_collection_strategy_i
{
public:

	void processPut(
			const Tag & t,
			const Item & i)
	{
		static_cast<Derived*>(this)->processPut(t, i);
	}

	template < typename UserStepTag, typename PutterCollection >
	void processPut(
			const UserStepTag & putter,
			PutterCollection & putterColl,
			const Tag & t,
			const Item & i)
	{
		static_cast<Derived*>(this)->processPut(putter, putterColl, t, i);
	}

	template < typename UserStepTag, typename GetterCollection >
	void processGet(
			const UserStepTag & getter,
    		GetterCollection & getterColl,
			const Tag & tag,
			Item & item)
	{
		static_cast<Derived*>(this)->processGet(getter, getterColl, tag, item);
	}
};


#endif // _RESILIENT_ITEM_COLLECTION_I_H_
