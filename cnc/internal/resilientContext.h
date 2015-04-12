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

	template< class Derived, class Tag, class Item >
	resilientContext< Derived, Tag, Item >::resilientContext():
		context< Derived >(),
		m_communicator( *this ),
		m_cmanager( 1, 0, 0 ),
		m_countdown_to_crash( -1 ),
		m_process_to_crash( -1 )
		{};

	template< class Derived, class Tag, class Item >
	resilientContext< Derived, Tag, Item >::resilientContext(int stepColls, int tagColls, int itemColls):
		context< Derived >(),
		m_communicator( *this ),
		m_cmanager( stepColls + 1, tagColls, itemColls ),
		m_countdown_to_crash( -1 ),
		m_process_to_crash( -1 )
		{};

	template< class Derived, class Tag, class Item >
	resilientContext< Derived, Tag, Item >::resilientContext(int stepColls, int tagColls, int itemColls, int countdown, int processId):
		context< Derived >(),
		m_communicator( *this ),
		m_cmanager( stepColls + 1, tagColls, itemColls ),
		m_countdown_to_crash( countdown ),
		m_process_to_crash( processId )
		{};

	template< class Derived, class Tag, class Item >
	resilientContext< Derived, Tag, Item >::~resilientContext() {};


	template< class Derived, class Tag, class Item >
	void resilientContext< Derived, Tag, Item >::done(const Tag & tag, const int tagColId, const int nrOfPuts, const int nrOfPrescribes) const {
		serializer * ser = dist_context::new_serializer( &m_communicator );
		(*ser) & checkpoint_tuner_types::DONE & tag & tagColId & nrOfPuts & nrOfPrescribes;
		dist_context::send_msg(ser, 0);
	}

	template< class Derived, class Tag, class Item >
	void resilientContext< Derived, Tag, Item >::prescribe(const Tag & prescriber, const int prescriberColId, const Tag & tag, const int tagColId) const {
		serializer * ser = dist_context::new_serializer( &m_communicator );
		(*ser) & checkpoint_tuner_types::DONE & prescriber & prescriberColId & tag & tagColId;
		dist_context::send_msg(ser, 0);
	}

	template< class Derived, class Tag, class Item >
	void resilientContext< Derived, Tag, Item >::put(const Tag & putter, const int putterColId, const Tag & tag, const Item & item, const int itemColId) const {
    	serializer * ser = dist_context::new_serializer( &m_communicator );
    	(*ser) & checkpoint_tuner_types::PUT & putter & putterColId & tag & item & itemColId;
    	dist_context::send_msg(ser, 0); //zero is like the context on the main... right?
	}

	template< class Derived, class Tag, class Item >
	void resilientContext< Derived, Tag, Item >::printCheckpoint() {
		m_cmanager.calculateCheckpoint();
		m_cmanager.printCheckpoint();
	}

	template< class Derived, class Tag, class Item >
	void resilientContext< Derived, Tag, Item >::checkForCrash() {
		if (m_countdown_to_crash >= 0) {
			if (m_countdown_to_crash == 0) {
				crash();
				m_countdown_to_crash = -1;
			} else {
				m_countdown_to_crash--;
			}
		}
	}

	template< class Derived, class Tag, class Item >
	void resilientContext< Derived, Tag, Item >::crash() const {
		int node_id = 1;
		serializer * ser = dist_context::new_serializer( this );
		(* ser) & checkpoint_tuner_types::CRASH & node_id;
		dist_context::send_msg(ser, node_id);
	}


	// Implementation of CnC::resilientContext::communicator

//	template< class Derived, class Tag, class Item >
//	resilientContext< Derived, Tag, Item >::communicator::communicator(): m_resilientContext(NULL) {};

	template< class Derived, class Tag, class Item >
	resilientContext< Derived, Tag, Item >::communicator::communicator(resilientContext< Derived, Tag, Item > & rctxt): m_resilientContext(rctxt) {
		m_resilientContext.subscribe(this);
	}

	template< class Derived, class Tag, class Item >
	resilientContext< Derived, Tag, Item >::communicator::~communicator() {
		m_resilientContext.unsubscribe(this);
	}

	template< class Derived, class Tag, class Item >
	void resilientContext< Derived, Tag, Item >::communicator::recv_msg( serializer * ser ) {
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
				m_resilientContext.m_cmanager.processStepDone( tag, stepCollectionUID, nr_of_puts, nr_of_prescribes);//TODO refactor
				m_resilientContext.checkForCrash();
				break;
			}
			case checkpoint_tuner_types::CRASH:
			{
				std::cout << "Crashing " << std::endl;
				m_resilientContext.reset_distributables(true);
				break;
			}
			default:
				CNC_ABORT( "Protocol error: unexpected message tag." );
		}
	}

	template< class Derived, class Tag, class Item >
	void resilientContext< Derived, Tag, Item >::communicator::unsafe_reset( bool dist ) {

	}


} // namespace CnC

#endif /* CNC_INTERNAL_RESILIENTCONTEXT_H_ */
