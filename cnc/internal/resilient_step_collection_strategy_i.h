#ifndef _RESILIENT_STEP_COLLECTION_I_H_
#define _RESILIENT_STEP_COLLECTION_I_H_

template< typename Derived, typename UserStepTag >
class resilient_step_collection_strategy_i
{
public:

	void processPut(
    		UserStepTag putter,
			ItemCheckpoint_i * item_cp,
			void * itemid)
	{
		static_cast<Derived*>(this)->processPut(putter, item_cp, itemid);
	}


	void processPrescribe(
			UserStepTag prescriber,
			TagCheckpoint_i * tag_cp,
			void * tagid)
	{
		static_cast<Derived*>(this)->processPrescribe(prescriber, tag_cp, tagid);
	}


	void processGet(
    		UserStepTag getter,
			ItemCheckpoint_i * item_cp,
			void* tag)
	{
		static_cast<Derived*>(this)->processGet(getter, item_cp, tag);
	}

	void processStepDone(
    		UserStepTag step,
    		int stepColId,
    		int puts,
    		int prescribes,
    		int gets)
	{
		static_cast<Derived*>(this)->processStepDone(step, stepColId, puts, prescribes, gets);
	}

	StepCheckpoint_i * getStepCheckpoint()
	{
		return static_cast<Derived*>(this)->getStepCheckpoint();
	}

	bool isStepDone(UserStepTag & step)
	{
		return static_cast<Derived*>(this)->isStepDone(step);
	}
};


#endif // _RESILIENT_STEP_COLLECTION_I_H_
