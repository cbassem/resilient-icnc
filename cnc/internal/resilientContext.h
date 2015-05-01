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

	template< class Derived >
	resilientContext< Derived >::resilientContext():
		context< Derived >(),
		m_communicator( *this ),
		m_countdown_to_crash( -1 ),
		m_process_to_crash( -1 )
//		m_mutex()
		{ std::cout << " creating res ctxt " << std::endl; };

	template< class Derived >
	resilientContext< Derived >::resilientContext(int stepColls, int tagColls, int itemColls):
		context< Derived >(),
		m_communicator( *this ),
		m_countdown_to_crash( -1 ),
		m_process_to_crash( -1 )
//		m_mutex()
		{  std::cout << " creating res ctxt " << std::endl; };

	template< class Derived >
	resilientContext< Derived >::resilientContext(int stepColls, int tagColls, int itemColls, int countdown, int processId):
		context< Derived >(),
		m_communicator( *this ),
		m_countdown_to_crash( countdown ),
		m_process_to_crash( processId )
//		m_mutex()
		{  std::cout << " creating res ctxt " << std::endl; };

	template< class Derived >
	resilientContext< Derived >::~resilientContext() {
	};

	template< class Derived >
	void resilientContext< Derived >::registerStepCheckPoint( StepCheckpoint_i*  step_col ) {
		m_step_checkpoints.push_back(step_col);
	};

	template< class Derived >
	void resilientContext< Derived >::registerTagCheckpoint( TagCheckpoint_i* tag_col ) {
		m_tag_checkpoints.push_back(tag_col);
	};

	template< class Derived >
	void resilientContext< Derived >::registerItemCheckpoint( ItemCheckpoint_i* item_col ) {
		m_item_checkpoints.push_back(item_col);
	};

	template< class Derived >
	void resilientContext< Derived >::add_checkpoint_data_locally() {

//		// We need a lock, the addition of data can launch new steps.
//		//mutex_t::scoped_lock _l( m_mutex ); //FIXME, we realy, realy need a lock here
//
//
//		//First calculate checkpoint
//		m_cmanager.calculateCheckpoint();
//		m_cmanager.printCheckpoint();
//
//		for ( typename std::vector< ItemCollectionType * >::reverse_iterator it = m_item_collections.rbegin(); it != m_item_collections.rend(); ++it ) {
//			int crrId = (*it)->getId();
//			std::tr1::unordered_map<Tag, Item>& map = m_cmanager.getItemCheckpoint( crrId );
//			std::cout << "adding items ... " << std::endl;
//			std::vector<typename std::tr1::unordered_map< Tag, Item >::const_iterator > tmp_;
//			for (typename std::tr1::unordered_map<Tag, Item>::const_iterator itt =map.begin(); itt != map.end(); ++itt) {
//				tmp_.push_back(itt);
//				//restart_put(itt->first, itt->second, crrId);
//			}
//
//			for (typename std::vector<typename std::tr1::unordered_map< Tag, Item >::const_iterator >::reverse_iterator itt=tmp_.rbegin();
//					itt != tmp_.rend();
//					++itt ) {
//				restart_put((*itt)->first, (*itt)->second, crrId);
//			}
//		}
//
//		//First put steps. Steps get triggered from waiting state by item puts
//		for( typename std::vector< TagCollectionType * >::const_iterator it = m_tag_collections.begin(); it != m_tag_collections.end(); ++it ) {
//			int crrId = (*it)->getId();
//			std::cout << "restart... adding tags to collecton with Id "<< crrId << std::endl;
//			std::set< Tag >& set = m_cmanager.getTagCheckpoint(crrId);
//			std::cout <<  "adding tags ... " << std::endl;
//			for( typename std::set< Tag >::const_iterator itt = set.begin(); itt != set.end(); ++itt) {
//				restart_prescribe(*itt, crrId);
//			}
//		}
//
//		//_l.release();


	}


	template< class Derived >
	void resilientContext< Derived >::checkForCrash() {
		if (m_countdown_to_crash >= 0) {
			if (m_countdown_to_crash == 0) {
				crash();
				m_countdown_to_crash = -1;
			} else {
				m_countdown_to_crash--;
			}
		}
	}

	template< class Derived >
	void resilientContext< Derived >::crash() const {
		Internal::distributor::send_crash_msg(this->gid(), m_process_to_crash);
	}


	template< class Derived >
	void resilientContext< Derived >::remote_wait_init( int recvr ) {
		CnC::Internal::context_base::m_scheduler->re_init_wait( recvr );
	}

	//////////////////////////////////////////////////////////////
	/// Implementation of CnC::resilientContext::communicator ////
    /////////////////////////////////////////////////////////////

	template< class Derived >
	resilientContext< Derived >::communicator::communicator(resilientContext< Derived > & rctxt): m_resilientContext(rctxt) {
		m_resilientContext.subscribe(this);
		std::cout << " creating res ctxt comm " << std::endl;
	}

	template< class Derived >
		resilientContext< Derived >::communicator::~communicator() {
		m_resilientContext.unsubscribe(this);
	}

	static bool _yield() { tbb::this_tbb_thread::sleep(tbb::tick_count::interval_t(0.0005)); return true; }

	template< class Derived >
	void resilientContext< Derived >::communicator::recv_msg( serializer * ser ) {
		char msg_tag;
		(* ser) & msg_tag;

		switch (msg_tag) {
			case checkpoint_tuner_types::REQUEST_RESTART_DATA:
			{
				std::cout << "Adding Checkpoint data " << Internal::distributor::myPid() << std::endl;
				int requester;
				(* ser) & requester;
				m_resilientContext.add_checkpoint_data_locally();
				m_resilientContext.remote_wait_init( requester );
				break;
			}

			default:
				CNC_ABORT( "Protocol error: unexpected message tag." );
			}
		}

	template< class Derived >
	void resilientContext< Derived >::communicator::unsafe_reset( bool dist ) {}





//
//	template< class Derived, class Tag, class Item, class StepCollectionType, class TagCollectionType , class ItemCollectionType >
//	void resilientContext< Derived, Tag, Item, StepCollectionType, TagCollectionType, ItemCollectionType >::communicator::recv_msg( serializer * ser ) {
//		//if everybody sends to the main one then this one will have a reference to the actual checkpoint
//		char msg_tag;
//		(* ser) & msg_tag;
//
//		switch (msg_tag) {
//			case checkpoint_tuner_types::PUT:
//			{
//				Tag putter;
//				int putterColId;
//				Tag tag;
//				Item item;
//				int itemCollectionUID;
//				(* ser) & putter & putterColId & tag & item & itemCollectionUID;
//				m_resilientContext.m_cmanager.processItemPut(putter, putterColId, tag, item, itemCollectionUID); //TODO refactor
//				break;
//			}
//			case checkpoint_tuner_types::PRESCRIBE:
//			{
//				Tag prescriber;
//				int prescriberColId;
//				Tag tag;
//				int tagCollectionUID;
//				(* ser) & prescriber & prescriberColId & tag & tagCollectionUID;
//				m_resilientContext.m_cmanager.processStepPrescribe(prescriber, prescriberColId, tag, tagCollectionUID); //TODO refactor
//				break;
//			}
//			case checkpoint_tuner_types::DONE:
//			{
//				Tag tag;
//				int stepCollectionUID, nr_of_puts, nr_of_prescribes;
//				(* ser) & tag & stepCollectionUID & nr_of_puts & nr_of_prescribes;
//				if (stepCollectionUID != 0) {
//					m_resilientContext.m_cmanager.processStepDone( tag, stepCollectionUID, nr_of_puts, nr_of_prescribes);//TODO refactor
//					if ( m_resilientContext.gid() == 0) {
//						m_resilientContext.checkForCrash();
//					}
//				}
//				break;
//			}
//			case checkpoint_tuner_types::REQUEST_RESTART_DATA:
//			{
//				std::cout << "Sending Restart Data " << Internal::distributor::myPid() << std::endl;
//				int requester;
//				(* ser) & requester;
//				//m_resilientContext.init_restart(requester);
//				//m_resilientContext.sendPing(10);
//
//				m_resilientContext.add_checkpoint_data_locally();
//
////				int _tmp = 100;
////				do {} while (_yield() && --_tmp > 0);
//
//				//m_resilientContext.remote_wait_init( requester );
//
//				//m_resilientContext.reset_suspended_steps();
//				break;
//			}

//			default:
//				CNC_ABORT( "Protocol error: unexpected message tag." );
//		}
//	}



} // namespace CnC

#endif /* CNC_INTERNAL_RESILIENTCONTEXT_H_ */
