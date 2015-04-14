/*
 * resilientContext.h
 *
 *  Created on: Apr 12, 2015
 *      Author: root
 */

#ifndef CNC_INTERNAL_RESILIENTCONTEXT_H_
#define CNC_INTERNAL_RESILIENTCONTEXT_H_

//Implementation of CnC::resilientContext
namespace CnC {

	template< class Derived, class Tag, class Item, class StepCollectionType, class TagCollectionType , class ItemCollectionType >
	resilientContext< Derived, Tag, Item, StepCollectionType, TagCollectionType, ItemCollectionType >::resilientContext():
		context< Derived >(),
		m_communicator( *this ),
		m_cmanager( 1, 0, 0 ),
		m_countdown_to_crash( -1 ),
		m_process_to_crash( -1 )
		{};

	template< class Derived, class Tag, class Item, class StepCollectionType, class TagCollectionType , class ItemCollectionType >
	resilientContext< Derived, Tag, Item, StepCollectionType, TagCollectionType, ItemCollectionType >::resilientContext(int stepColls, int tagColls, int itemColls):
		context< Derived >(),
		m_communicator( *this ),
		m_cmanager( stepColls + 1, tagColls, itemColls ),
		m_countdown_to_crash( -1 ),
		m_process_to_crash( -1 )
		{};

	template< class Derived, class Tag, class Item, class StepCollectionType, class TagCollectionType , class ItemCollectionType >
	resilientContext< Derived, Tag, Item, StepCollectionType, TagCollectionType, ItemCollectionType >::resilientContext(int stepColls, int tagColls, int itemColls, int countdown, int processId):
		context< Derived >(),
		m_communicator( *this ),
		m_cmanager( stepColls + 1, tagColls, itemColls ),
		m_countdown_to_crash( countdown ),
		m_process_to_crash( processId )
		{};

	template< class Derived, class Tag, class Item, class StepCollectionType, class TagCollectionType , class ItemCollectionType >
	resilientContext< Derived, Tag, Item, StepCollectionType, TagCollectionType, ItemCollectionType >::~resilientContext() {};

	template< class Derived, class Tag, class Item, class StepCollectionType, class TagCollectionType , class ItemCollectionType >
	void resilientContext< Derived, Tag, Item, StepCollectionType, TagCollectionType, ItemCollectionType >::registerStepCollection(StepCollectionType & step_col ) {
		m_step_collections.push_back(&step_col);
	};

	template< class Derived, class Tag, class Item, class StepCollectionType, class TagCollectionType , class ItemCollectionType >
	void resilientContext< Derived, Tag, Item, StepCollectionType, TagCollectionType, ItemCollectionType >::registerTagCollection(TagCollectionType & tag_col ) {
		m_tag_collections.push_back(&tag_col);
	};

	template< class Derived, class Tag, class Item, class StepCollectionType, class TagCollectionType , class ItemCollectionType >
	void resilientContext< Derived, Tag, Item, StepCollectionType, TagCollectionType, ItemCollectionType >::registerItemCollection(ItemCollectionType & item_col ) {
		m_item_collections.push_back(&item_col);
	};

	template< class Derived, class Tag, class Item, class StepCollectionType, class TagCollectionType , class ItemCollectionType >
	void resilientContext< Derived, Tag, Item, StepCollectionType, TagCollectionType, ItemCollectionType >::done(const Tag & tag, const int tagColId, const int nrOfPuts, const int nrOfPrescribes) const {
		serializer * ser = dist_context::new_serializer( &m_communicator );
		(*ser) & checkpoint_tuner_types::DONE & tag & tagColId & nrOfPuts & nrOfPrescribes;
		dist_context::send_msg(ser, 0);
	}

	template< class Derived, class Tag, class Item, class StepCollectionType, class TagCollectionType , class ItemCollectionType >
	void resilientContext< Derived, Tag, Item, StepCollectionType, TagCollectionType, ItemCollectionType >::prescribe(const Tag & prescriber, const int prescriberColId, const Tag & tag, const int tagColId) const {
		serializer * ser = dist_context::new_serializer( &m_communicator );
		(*ser) & checkpoint_tuner_types::DONE & prescriber & prescriberColId & tag & tagColId;
		dist_context::send_msg(ser, 0);
	}

	template< class Derived, class Tag, class Item, class StepCollectionType, class TagCollectionType , class ItemCollectionType >
	void resilientContext< Derived, Tag, Item, StepCollectionType, TagCollectionType, ItemCollectionType >::put(const Tag & putter, const int putterColId, const Tag & tag, const Item & item, const int itemColId) const {
    	serializer * ser = dist_context::new_serializer( &m_communicator );
    	(*ser) & checkpoint_tuner_types::PUT & putter & putterColId & tag & item & itemColId;
    	dist_context::send_msg(ser, 0); //zero is like the context on the main... right?
	}

	template< class Derived, class Tag, class Item, class StepCollectionType, class TagCollectionType , class ItemCollectionType >
	void resilientContext< Derived, Tag, Item, StepCollectionType, TagCollectionType, ItemCollectionType >::printCheckpoint() {
		m_cmanager.calculateCheckpoint();
		m_cmanager.printCheckpoint();
	}



//	template< class Derived, class Tag, class Item >
//    error_type resilientContext< Derived, Tag, Item >::wait()
//    {
//        CNC_ASSERT( !distributed() || CnC::Internal::distributor::distributed_env() || CnC::Internal::distributor::myPid() == 0 );
//        CnC::Internal::context_base::m_scheduler->wait_loop();
//        // Check if a node went down;
//        if (CnC::Internal::distributor::myPid() == 0) {
//            int res = CnC::Internal::distributor::flush();
//            std::cout << "interseting var "<< res << std::endl;
//        };
//        if( CnC::Internal::context_base::subscribed() && CnC::Internal::context_base::m_scheduler->subscribed() ) {
//            if( CnC::Internal::distributor::myPid() == 0 ) CnC::Internal::context_base::cleanup_distributables( true );
//            CnC::Internal::context_base::m_scheduler->wait_loop();
//        }
//        return 0;
//    }

	template< class Derived, class Tag, class Item, class StepCollectionType, class TagCollectionType , class ItemCollectionType >
	void resilientContext< Derived, Tag, Item, StepCollectionType, TagCollectionType, ItemCollectionType >::checkForCrash() {
		if (m_countdown_to_crash >= 0) {
			if (m_countdown_to_crash == 0) {
				crash();
				m_countdown_to_crash = -1;
			} else {
				m_countdown_to_crash--;
			}
		}
	}

	template< class Derived, class Tag, class Item, class StepCollectionType, class TagCollectionType , class ItemCollectionType >
	void resilientContext< Derived, Tag, Item, StepCollectionType, TagCollectionType, ItemCollectionType >::crash() const {
		int node_id = 1;
		serializer * ser = dist_context::new_serializer( &m_communicator );
		(* ser) & checkpoint_tuner_types::CRASH & node_id;
		dist_context::send_msg(ser, node_id);
	}

	template< class Derived, class Tag, class Item, class StepCollectionType, class TagCollectionType , class ItemCollectionType >
	void resilientContext< Derived, Tag, Item, StepCollectionType, TagCollectionType, ItemCollectionType >::remove_local() {
		Internal::distributor::remove_local(this);
		//Internal::distributor::undistribute(this);
	}


	// Implementation of CnC::resilientContext::communicator

//	template< class Derived, class Tag, class Item >
//	resilientContext< Derived, Tag, Item >::communicator::communicator(): m_resilientContext(NULL) {};

	template< class Derived, class Tag, class Item, class StepCollectionType, class TagCollectionType , class ItemCollectionType >
	resilientContext< Derived, Tag, Item, StepCollectionType, TagCollectionType, ItemCollectionType >::communicator::communicator(resilientContext< Derived, Tag, Item, StepCollectionType, TagCollectionType, ItemCollectionType  > & rctxt): m_resilientContext(rctxt) {
		m_resilientContext.subscribe(this);
	}

	template< class Derived, class Tag, class Item, class StepCollectionType, class TagCollectionType , class ItemCollectionType >
	resilientContext< Derived, Tag, Item, StepCollectionType, TagCollectionType, ItemCollectionType >::communicator::~communicator() {
		m_resilientContext.unsubscribe(this);
	}

	template< class Derived, class Tag, class Item, class StepCollectionType, class TagCollectionType , class ItemCollectionType >
	void resilientContext< Derived, Tag, Item, StepCollectionType, TagCollectionType, ItemCollectionType >::communicator::recv_msg( serializer * ser ) {
		//if everybody sends to the main one then this one will have a reference to the actual checkpoint
		char msg_tag;
		(* ser) & msg_tag;

		switch (msg_tag) {
			case checkpoint_tuner_types::PUT:
			{
				Tag putter;
				int putterColId;
				Tag tag;
				Item item;
				int itemCollectionUID;
				(* ser) & putter & putterColId & tag & item & itemCollectionUID;
				m_resilientContext.m_cmanager.processItemPut(putter, putterColId, tag, item, itemCollectionUID); //TODO refactor
				break;
			}
			case checkpoint_tuner_types::PRESCRIBE:
			{
				Tag prescriber;
				int prescriberColId;
				Tag tag;
				int tagCollectionUID;
				(* ser) & prescriber & prescriberColId & tag & tagCollectionUID;
				m_resilientContext.m_cmanager.processStepPrescribe(prescriber, prescriberColId, tag, tagCollectionUID); //TODO refactor
				break;
			}
			case checkpoint_tuner_types::DONE:
			{
				Tag tag;
				int stepCollectionUID, nr_of_puts, nr_of_prescribes;
				(* ser) & tag & stepCollectionUID & nr_of_puts & nr_of_prescribes;
				if (stepCollectionUID != 0) {
					m_resilientContext.m_cmanager.processStepDone( tag, stepCollectionUID, nr_of_puts, nr_of_prescribes);//TODO refactor
					m_resilientContext.checkForCrash();
				}
				break;
			}
			case checkpoint_tuner_types::CRASH:
			{
				std::cout << "Crashing " << Internal::distributor::myPid() << std::endl;
				m_resilientContext.remove_local();
				break;
			}
			default:
				CNC_ABORT( "Protocol error: unexpected message tag." );
		}
	}

	template< class Derived, class Tag, class Item, class StepCollectionType, class TagCollectionType , class ItemCollectionType >
	void resilientContext< Derived, Tag, Item, StepCollectionType, TagCollectionType, ItemCollectionType >::communicator::unsafe_reset( bool dist ) {

	}


} // namespace CnC

#endif /* CNC_INTERNAL_RESILIENTCONTEXT_H_ */
