#ifndef _RESILIENT_TAG_COLLECTION_I_H_
#define _RESILIENT_TAG_COLLECTION_I_H_


template< typename Derived, typename Tag >
class resilient_tag_collection_strategy_i
{
public:

	void processPrescribe(
			const Tag & t)
	{
		static_cast<Derived*>(this)->processPrescribe(t);
	}

	template < typename StepTag, typename PrescriberCollection >
	void processPrescribe(
			const StepTag & prescriber,
			PrescriberCollection & prescriberColl,
			const Tag & t)
	{
		static_cast<Derived*>(this)->processPrescribe(prescriber, prescriberColl, t);
	}

	template < typename StepCheckpoint >
	void prescribeStepCheckpoint( StepCheckpoint & s )
	{
		static_cast<Derived*>(this)->prescribeStepCheckpoint(s);
	}

};


#endif // _RESILIENT_TAG_COLLECTION_I_H_
