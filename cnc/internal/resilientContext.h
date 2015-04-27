/*
 * resilientContext.h
 *
 *  Created on: Apr 12, 2015
 *      Author: root
 */

#ifndef CNC_INTERNAL_RESILIENTCONTEXT_H_
#define CNC_INTERNAL_RESILIENTCONTEXT_H_

#include <cnc/internal/scheduler_i.h>

//Implementation of CnC::resilientContext
namespace CnC {

	template< class Derived, class Tag, class Item, class StepCollectionType, class TagCollectionType , class ItemCollectionType >
	resilientContext< Derived, Tag, Item, StepCollectionType, TagCollectionType, ItemCollectionType >::resilientContext():
		context< Derived >(),
		m_communicator( *this ),
		m_cmanager( 1, 0, 0 ),
		m_countdown_to_crash( -1 ),
		m_process_to_crash( -1 ),
		m_mutex()
		{ std::cout << " creating res ctxt " << std::endl; };

	template< class Derived, class Tag, class Item, class StepCollectionType, class TagCollectionType , class ItemCollectionType >
	resilientContext< Derived, Tag, Item, StepCollectionType, TagCollectionType, ItemCollectionType >::resilientContext(int stepColls, int tagColls, int itemColls):
		context< Derived >(),
		m_communicator( *this ),
		m_cmanager( stepColls + 1, tagColls, itemColls ),
		m_countdown_to_crash( -1 ),
		m_process_to_crash( -1 ),
		m_mutex()
		{  std::cout << " creating res ctxt " << std::endl; };

	template< class Derived, class Tag, class Item, class StepCollectionType, class TagCollectionType , class ItemCollectionType >
	resilientContext< Derived, Tag, Item, StepCollectionType, TagCollectionType, ItemCollectionType >::resilientContext(int stepColls, int tagColls, int itemColls, int countdown, int processId):
		context< Derived >(),
		m_communicator( *this ),
		m_cmanager( stepColls + 1, tagColls, itemColls ),
		m_countdown_to_crash( countdown ),
		m_process_to_crash( processId ),
		m_mutex()
		{  std::cout << " creating res ctxt " << std::endl; };

	template< class Derived, class Tag, class Item, class StepCollectionType, class TagCollectionType , class ItemCollectionType >
	resilientContext< Derived, Tag, Item, StepCollectionType, TagCollectionType, ItemCollectionType >::~resilientContext() {
	};

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
	void resilientContext< Derived, Tag, Item, StepCollectionType, TagCollectionType, ItemCollectionType >::done(const Tag & tag, const int tagColId, const int nrOfPuts, const int nrOfPrescribes) {
	    //mutex_t::scoped_lock _l( m_mutex );
		serializer * ser = dist_context::new_serializer( &m_communicator );
		(*ser) & checkpoint_tuner_types::DONE & tag & tagColId & nrOfPuts & nrOfPrescribes;
		if (Internal::distributor::myPid() == 0) { //sockets cannot send to themselves
			m_cmanager.processStepDone( tag, tagColId, nrOfPuts, nrOfPrescribes);
		} else {
			dist_context::send_msg(ser, 0);
		}
		//_l.release();
	}

	template< class Derived, class Tag, class Item, class StepCollectionType, class TagCollectionType , class ItemCollectionType >
	void resilientContext< Derived, Tag, Item, StepCollectionType, TagCollectionType, ItemCollectionType >::prescribe(const Tag & prescriber, const int prescriberColId, const Tag & tag, const int tagColId) {
	    //mutex_t::scoped_lock _l( m_mutex );
		serializer * ser = dist_context::new_serializer( &m_communicator );
		(*ser) & checkpoint_tuner_types::PRESCRIBE & prescriber & prescriberColId & tag & tagColId;
		if (Internal::distributor::myPid() == 0) {
			m_cmanager.processStepPrescribe( prescriber, prescriberColId, tag, tagColId );
		} else {
			dist_context::send_msg(ser, 0);
		}
		//_l.release();
	}

	template< class Derived, class Tag, class Item, class StepCollectionType, class TagCollectionType , class ItemCollectionType >
	void resilientContext< Derived, Tag, Item, StepCollectionType, TagCollectionType, ItemCollectionType >::put(const Tag & putter, const int putterColId, const Tag & tag, const Item & item, const int itemColId) {
	    //mutex_t::scoped_lock _l( m_mutex );
		serializer * ser = dist_context::new_serializer( &m_communicator );
    	(*ser) & checkpoint_tuner_types::PUT & putter & putterColId & tag & item & itemColId;
		if (Internal::distributor::myPid() == 0) {
			m_cmanager.processItemPut(putter, putterColId, tag, item, itemColId);
		} else {
			dist_context::send_msg(ser, 0);
		}
		//_l.release();
	}

	template< class Derived, class Tag, class Item, class StepCollectionType, class TagCollectionType , class ItemCollectionType >
	void resilientContext< Derived, Tag, Item, StepCollectionType, TagCollectionType, ItemCollectionType >::printCheckpoint() {
		m_cmanager.calculateCheckpoint();
		m_cmanager.printCheckpoint();
	}

	template< class Derived, class Tag, class Item, class StepCollectionType, class TagCollectionType , class ItemCollectionType >
	void resilientContext< Derived, Tag, Item, StepCollectionType, TagCollectionType, ItemCollectionType >::restarted() {
		std::cout << "Restarted and checking for data... " << Internal::distributor::myPid() << std::endl;
		CnC::Internal::context_base::m_scheduler->start_dist();
		CnC::Internal::scheduler_i::restarted_safe = false;
		serializer * ser = dist_context::new_serializer( &m_communicator );
		(*ser) & checkpoint_tuner_types::REQUEST_RESTART_DATA & CnC::Internal::distributor::myPid();
		dist_context::send_msg(ser, 0);

	}

	template< class Derived, class Tag, class Item, class StepCollectionType, class TagCollectionType , class ItemCollectionType >
	void resilientContext< Derived, Tag, Item, StepCollectionType, TagCollectionType, ItemCollectionType >::sendItem(Tag key, Item value, int item_coll, int receiver_pid) {
		serializer * ser = dist_context::new_serializer( &m_communicator );
		(*ser) & checkpoint_tuner_types::REQUESTED_ITEM & key & value & item_coll;
		dist_context::send_msg(ser, receiver_pid);
	}

	template< class Derived, class Tag, class Item, class StepCollectionType, class TagCollectionType , class ItemCollectionType >
	void resilientContext< Derived, Tag, Item, StepCollectionType, TagCollectionType, ItemCollectionType >::sendTag(Tag tag, int tag_coll, int receiver_pid) {
		serializer * ser = dist_context::new_serializer( &m_communicator );
		(*ser) & checkpoint_tuner_types::REQUESTED_TAG & tag & tag_coll;
		dist_context::send_msg(ser, receiver_pid);
	}


	template< class Derived, class Tag, class Item, class StepCollectionType, class TagCollectionType , class ItemCollectionType >
	void resilientContext< Derived, Tag, Item, StepCollectionType, TagCollectionType, ItemCollectionType >::restart_put(Tag key, Item value, int item_coll) {
		//mutex_t::scoped_lock _l( m_mutex );
		m_item_collections[item_coll]->restart_put(key, value); //TODO perhaps we should carry over the putter data... local checkpoint data might get corrupted
		//_l.release();
	}

	template< class Derived, class Tag, class Item, class StepCollectionType, class TagCollectionType , class ItemCollectionType >
	void resilientContext< Derived, Tag, Item, StepCollectionType, TagCollectionType, ItemCollectionType >::restart_prescribe(Tag tag, int tag_coll) {
		//mutex_t::scoped_lock _l( m_mutex );
		m_tag_collections[tag_coll]->restart_put(tag); //TODO perhaps we should carry over the putter data... local checkpoint data might get corrupted
		//_l.release();
	}

	template< class Derived, class Tag, class Item, class StepCollectionType, class TagCollectionType , class ItemCollectionType >
	void resilientContext< Derived, Tag, Item, StepCollectionType, TagCollectionType, ItemCollectionType >::init_restart(int requester_pid) {
		//First calculate checkpoint
		m_cmanager.calculateCheckpoint();

		for( typename std::vector< ItemCollectionType * >::const_iterator it = m_item_collections.begin(); it != m_item_collections.end(); ++it ) {
			int crrId = (*it)->getId();
			std::tr1::unordered_map<Tag, Item >& map = m_cmanager.getItemCheckpoint(crrId);
			for( typename std::tr1::unordered_map<Tag, Item >::const_iterator itt = map.begin(); itt != map.end(); ++itt) {
				sendItem(itt->first, itt->second, crrId, requester_pid);
			}
		}

		for( typename std::vector< TagCollectionType * >::const_iterator it = m_tag_collections.begin(); it != m_tag_collections.end(); ++it ) {
			int crrId = (*it)->getId();
			std::set< Tag >& set = m_cmanager.getTagCheckpoint(crrId);
			for( typename std::set< Tag >::const_iterator itt = set.begin(); itt != set.end(); ++itt) {
				sendTag(*itt, crrId, requester_pid);
			}
		}
	}


	template< class Derived, class Tag, class Item, class StepCollectionType, class TagCollectionType , class ItemCollectionType >
	void resilientContext< Derived, Tag, Item, StepCollectionType, TagCollectionType, ItemCollectionType >::add_checkpoint_data_locally() {

		// We need a lock, the addition of data can launch new steps.
		//mutex_t::scoped_lock _l( m_mutex );


		//First calculate checkpoint
		m_cmanager.calculateCheckpoint();
		m_cmanager.printCheckpoint();

		for ( typename std::vector< ItemCollectionType * >::reverse_iterator it = m_item_collections.rbegin(); it != m_item_collections.rend(); ++it ) {
			int crrId = (*it)->getId();
			std::tr1::unordered_map<Tag, Item>& map = m_cmanager.getItemCheckpoint( crrId );
			std::cout << "adding items ... " << std::endl;
			std::vector<typename std::tr1::unordered_map< Tag, Item >::const_iterator > tmp_;
			for (typename std::tr1::unordered_map<Tag, Item>::const_iterator itt =map.begin(); itt != map.end(); ++itt) {
				tmp_.push_back(itt);
				//restart_put(itt->first, itt->second, crrId);
			}

			for (typename std::vector<typename std::tr1::unordered_map< Tag, Item >::const_iterator >::reverse_iterator itt=tmp_.rbegin();
					itt != tmp_.rend();
					++itt ) {
				restart_put((*itt)->first, (*itt)->second, crrId);
			}
		}

		//First put steps. Steps get triggered from waiting state by item puts
		for( typename std::vector< TagCollectionType * >::const_iterator it = m_tag_collections.begin(); it != m_tag_collections.end(); ++it ) {
			int crrId = (*it)->getId();
			std::cout << "restart... adding tags to collecton with Id "<< crrId << std::endl;
			std::set< Tag >& set = m_cmanager.getTagCheckpoint(crrId);
			std::cout <<  "adding tags ... " << std::endl;
			for( typename std::set< Tag >::const_iterator itt = set.begin(); itt != set.end(); ++itt) {
				restart_prescribe(*itt, crrId);
			}
		}

		//_l.release();


	}

	template< class Derived, class Tag, class Item, class StepCollectionType, class TagCollectionType , class ItemCollectionType >
	void resilientContext< Derived, Tag, Item, StepCollectionType, TagCollectionType, ItemCollectionType >::sendPing(int nr_to_go) {
		serializer * ser = dist_context::new_serializer( &m_communicator );
		(*ser) & checkpoint_tuner_types::KEEP_ALIVE_PING & nr_to_go;
		dist_context::bcast_msg(ser);
	}

	template< class Derived, class Tag, class Item, class StepCollectionType, class TagCollectionType , class ItemCollectionType >
	void resilientContext< Derived, Tag, Item, StepCollectionType, TagCollectionType, ItemCollectionType >::sendPong(int nr_to_go) {
		serializer * ser = dist_context::new_serializer( &m_communicator );
		(*ser) & checkpoint_tuner_types::KEEP_ALIVE_PONG & nr_to_go;
		dist_context::send_msg(ser, 0);
	}

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
		Internal::distributor::send_crash_msg(this->gid(), m_process_to_crash);
	}

	template< class Derived, class Tag, class Item, class StepCollectionType, class TagCollectionType , class ItemCollectionType >
	void resilientContext< Derived, Tag, Item, StepCollectionType, TagCollectionType, ItemCollectionType >::stop_wait_loop() {
		CnC::Internal::context_base::m_scheduler->stop_wait_task();
	}

	template< class Derived, class Tag, class Item, class StepCollectionType, class TagCollectionType , class ItemCollectionType >
	void resilientContext< Derived, Tag, Item, StepCollectionType, TagCollectionType, ItemCollectionType >::remote_wait_init( int recvr ) {
		CnC::Internal::context_base::m_scheduler->re_init_wati( recvr );
	}


	// Implementation of CnC::resilientContext::communicator

//	template< class Derived, class Tag, class Item >
//	resilientContext< Derived, Tag, Item >::communicator::communicator(): m_resilientContext(NULL) {};

	template< class Derived, class Tag, class Item, class StepCollectionType, class TagCollectionType , class ItemCollectionType >
	resilientContext< Derived, Tag, Item, StepCollectionType, TagCollectionType, ItemCollectionType >::communicator::communicator(resilientContext< Derived, Tag, Item, StepCollectionType, TagCollectionType, ItemCollectionType  > & rctxt): m_resilientContext(rctxt) {
		m_resilientContext.subscribe(this);
		std::cout << " creating res ctxt comm " << std::endl;
	}

	template< class Derived, class Tag, class Item, class StepCollectionType, class TagCollectionType , class ItemCollectionType >
	resilientContext< Derived, Tag, Item, StepCollectionType, TagCollectionType, ItemCollectionType >::communicator::~communicator() {
		m_resilientContext.unsubscribe(this);
	}

	static bool _yield() { tbb::this_tbb_thread::sleep(tbb::tick_count::interval_t(0.0005)); return true; }

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
					if ( m_resilientContext.gid() == 0) {
						m_resilientContext.checkForCrash();
					}
				}
				break;
			}
			case checkpoint_tuner_types::REQUEST_RESTART_DATA:
			{
				std::cout << "Sending Restart Data " << Internal::distributor::myPid() << std::endl;
				int requester;
				(* ser) & requester;
				//m_resilientContext.init_restart(requester);
				//m_resilientContext.sendPing(10);
				m_resilientContext.add_checkpoint_data_locally();

//				int _tmp = 100;
//				do {} while (_yield() && --_tmp > 0);

				m_resilientContext.remote_wait_init( requester );
				//m_resilientContext.reset_suspended_steps();
				break;
			}
			case checkpoint_tuner_types::REQUESTED_ITEM:
			{
				Tag tag;
				Item item;
				int item_col;
				(* ser) & tag & item & item_col;
				m_resilientContext.restart_put(tag, item, item_col);
				break;
			}
			case checkpoint_tuner_types::REQUESTED_TAG:
			{
				Tag tag;
				int tag_col;
				(* ser) & tag & tag_col;
				m_resilientContext.restart_prescribe(tag, tag_col);
				break;
			}
			case checkpoint_tuner_types::KEEP_ALIVE_PING: //During restart it can happen that there is a brief moment when there are no messages being passed. This triggers the termination of the program.
			{
				int nr_of_pings_to_go;
				(* ser) & nr_of_pings_to_go;
				m_resilientContext.sendPong(nr_of_pings_to_go);
				break;
			}
			case checkpoint_tuner_types::KEEP_ALIVE_PONG:
				int nr_of_pings_to_go;
				(* ser) & nr_of_pings_to_go;
				if (nr_of_pings_to_go > 0) {
					m_resilientContext.sendPing(nr_of_pings_to_go - 1);
				}
				break;

			default:
				CNC_ABORT( "Protocol error: unexpected message tag." );
		}
	}

	template< class Derived, class Tag, class Item, class StepCollectionType, class TagCollectionType , class ItemCollectionType >
	void resilientContext< Derived, Tag, Item, StepCollectionType, TagCollectionType, ItemCollectionType >::communicator::unsafe_reset( bool dist ) {

	}


} // namespace CnC

#endif /* CNC_INTERNAL_RESILIENTCONTEXT_H_ */
