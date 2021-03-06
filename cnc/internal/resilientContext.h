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
		m_step_checkpoints(1),
		m_item_checkpoints(1),
		m_tag_checkpoints(1),
		m_countdown_to_crash( -1 ),
		m_process_to_crash( -1 ),
		m_timer_sec(-1),
		m_last_time(),
		m_mutex()
		{ std::cout << " creating res ctxt " << std::endl; }

	template< class Derived >
	resilientContext< Derived >::resilientContext(int countdown, int processId):
		context< Derived >(),
		m_communicator( *this ),
		m_step_checkpoints(1),
		m_item_checkpoints(1),
		m_tag_checkpoints(1),
		m_countdown_to_crash( countdown ),
		m_process_to_crash( processId ),
		m_timer_sec(-1),
		m_last_time(),
		m_mutex()
		{  std::cout << " creating res ctxt " << std::endl; }

	template< class Derived >
	resilientContext< Derived >::resilientContext(int countdown, int processId, int sec):
		context< Derived >(),
		m_communicator( *this ),
		m_step_checkpoints(1),
		m_item_checkpoints(1),
		m_tag_checkpoints(1),
		m_countdown_to_crash( countdown ),
		m_process_to_crash( processId ),
		m_timer_sec(sec),
		m_last_time(),
		m_mutex()
	{
		gettimeofday(&m_last_time, 0);
	}

	template< class Derived >
	resilientContext< Derived >::resilientContext(int sec):
		context< Derived >(),
		m_communicator( *this ),
		m_step_checkpoints(1),
		m_item_checkpoints(1),
		m_tag_checkpoints(1),
		m_countdown_to_crash( -1 ),
		m_process_to_crash( -1 ),
		m_timer_sec(sec),
		m_last_time(),
		m_mutex()
	{
		gettimeofday(&m_last_time, 0);
	}

	template< class Derived >
	resilientContext< Derived >::~resilientContext() {
	}

	template< class Derived >
	void resilientContext< Derived >::registerStepCheckPoint( StepCheckpoint_i*  step_cp )
	{
		std::cout << "Registering step cp " << step_cp->getId();

		if (m_step_checkpoints.size() - 1 < step_cp->getId()){
			m_step_checkpoints.resize(step_cp->getId() + 1);
		}
		m_step_checkpoints[step_cp->getId()] = step_cp;
	}

	template< class Derived >
	void resilientContext< Derived >::registerTagCheckpoint( TagCheckpoint_i* tag_cp )
	{
		std::cout << "Registering tag cp " << tag_cp->getId();
		if (m_tag_checkpoints.size() - 1 < tag_cp->getId()){
			m_tag_checkpoints.resize(tag_cp->getId() + 1);
		}
		m_tag_checkpoints[tag_cp->getId()] = tag_cp;
	}

	template< class Derived >
	void resilientContext< Derived >::registerItemCheckpoint( ItemCheckpoint_i* item_cp)
	{
		std::cout << "Registering item cp " << item_cp->getId();
		if (m_item_checkpoints.size() - 1 < item_cp->getId()){
			m_item_checkpoints.resize(item_cp->getId() + 1);
		}
		m_item_checkpoints[item_cp->getId()] = item_cp;
	}

	template< class Derived >
	StepCheckpoint_i* resilientContext< Derived >::getStepCheckPoint( int id )
	{
		return m_step_checkpoints[id];
	}

	template< class Derived >
	TagCheckpoint_i* resilientContext< Derived >::getTagCheckpoint( int id )
	{
		return m_tag_checkpoints[id];
	}

	template< class Derived >
	ItemCheckpoint_i* resilientContext< Derived >::getItemCheckpoint( int id )
	{
		return m_item_checkpoints[id];
	}

	template< class Derived >
	void resilientContext< Derived >::add_checkpoint_data_locally() {
		//First calculate checkpoint
		calculate_checkpoint();
		print_checkpoint();

		for( typename std::vector< ItemCheckpoint_i * >::const_iterator it = m_item_checkpoints.begin(); it != m_item_checkpoints.end(); ++it) {
			(*it)->add_checkpoint_locally();
		}

		for( typename std::vector< TagCheckpoint_i * >::const_iterator it = m_tag_checkpoints.begin(); it != m_tag_checkpoints.end(); ++it) {
			(*it)->add_checkpoint_locally();
		}
	}

	template< class Derived >
	void resilientContext< Derived >::restarted() {
		std::cout << "Restarted and checking for data... " << Internal::distributor::myPid() << std::endl;
		serializer * ser = dist_context::new_serializer( &m_communicator );
		(*ser) & checkpoint_tuner_types::REQUEST_RESTART_DATA & CnC::Internal::distributor::myPid();
		dist_context::send_msg(ser, 0);
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
	void resilientContext< Derived >::print_checkpoint() {
		for( typename std::vector< TagCheckpoint_i * >::const_iterator it = m_tag_checkpoints.begin(); it != m_tag_checkpoints.end(); ++it) {
			(*it)->print();
		}
		for( typename std::vector< ItemCheckpoint_i * >::const_iterator it = m_item_checkpoints.begin(); it != m_item_checkpoints.end(); ++it) {
			(*it)->print();
		}
	}

	template< class Derived >
	void resilientContext< Derived >::calculate_checkpoint() {
		//loop over all tag checkpoints & call restart
		for( typename std::vector< TagCheckpoint_i * >::const_iterator it = m_tag_checkpoints.begin(); it != m_tag_checkpoints.end(); ++it ) {
			(*it)->calculate_checkpoint();
		}

		for( typename std::vector< StepCheckpoint_i * >::const_iterator it = m_step_checkpoints.begin(); it != m_step_checkpoints.end(); ++it ) {
			(*it)->decrement_get_counts();// This will results in items being removed from the checkpoint.
		}

//		for( typename std::vector< StepCheckpoint_i * >::const_iterator it = m_step_checkpoints.begin(); it != m_step_checkpoints.end(); ++it ) {
//			(*it)->;
//		}
	}


	template< class Derived >
	void resilientContext< Derived >::remote_wait_init( int recvr ) {
		CnC::Internal::context_base::m_scheduler->re_init_wait( recvr );
	}


	template< class Derived >
	bool resilientContext< Derived >::hasTimePassed() {
		timeval now;
		gettimeofday(&now, 0);
		long diff(now.tv_sec - m_last_time.tv_sec);
		return (diff >= m_timer_sec);
	}

	template< class Derived >
	void resilientContext< Derived >::resetTimer() {
		gettimeofday(&m_last_time, 0);
	}

	template< class Derived >
	void resilientContext< Derived >::calculateAndSendCheckpoint()
	{
		//place lock
		mutex_t::scoped_lock _l( m_mutex );
		calculate_checkpoint();
		//Loop over all tagLogs and:
		//If step done:
		//	-> Send all tags and items that are not "done" yet.
		//  -> Else do nothing
		for( typename std::vector< StepCheckpoint_i * >::const_iterator it = m_step_checkpoints.begin(); it != m_step_checkpoints.end(); ++it ) {
			(*it)->sendNotDone();// This will results in items being removed from the checkpoint.
		}


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
				m_resilientContext.remote_wait_init( requester );
				m_resilientContext.add_checkpoint_data_locally();
				break;
			}

			default:
				CNC_ABORT( "Protocol error: unexpected message tag." );
			}
		}

	template< class Derived >
	void resilientContext< Derived >::communicator::unsafe_reset( bool dist ) {}


} // namespace CnC

#endif /* CNC_INTERNAL_RESILIENTCONTEXT_H_ */
