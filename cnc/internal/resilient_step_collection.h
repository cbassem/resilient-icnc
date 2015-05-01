/* *******************************************************************************
 *  Copyright (c) 2007-2014, Intel Corporation
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of Intel Corporation nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ********************************************************************************/

/*
  Implementation of CnC::resilient_step_collection.
*/

#ifndef RESILIENT_STEP_COLLECTION_HH_ALREADY_INCLUDED
#define RESILIENT_STEP_COLLECTION_HH_ALREADY_INCLUDED

namespace CnC {


    template< typename Derived, typename UserStepTag, typename UserStep, typename Tuner, typename CheckpointTuner >
    resilient_step_collection< Derived, UserStepTag, UserStep, Tuner, CheckpointTuner >::resilient_step_collection( resilientContext< Derived > & ctxt )
        : step_collection< UserStep, Tuner, CheckpointTuner >( ctxt ), m_step_checkpoint(super_type::getId()), m_resilient_contex(ctxt), m_communicator(*this)
    {
        m_resilient_contex.registerStepCheckPoint( &m_step_checkpoint );
    }

    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    template< typename Derived, typename UserStepTag, typename UserStep, typename Tuner, typename CheckpointTuner >
    resilient_step_collection< Derived, UserStepTag, UserStep, Tuner, CheckpointTuner >::resilient_step_collection( resilientContext< Derived > & ctxt, const std::string & name )
        : step_collection< UserStep, Tuner, CheckpointTuner >( ctxt, name), m_step_checkpoint(super_type::getId()), m_resilient_contex(ctxt), m_communicator(*this)
    {
        m_resilient_contex.registerStepCheckPoint( &m_step_checkpoint );
    }
    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    template< typename Derived, typename UserStepTag, typename UserStep, typename Tuner, typename CheckpointTuner >
    resilient_step_collection< Derived, UserStepTag, UserStep, Tuner, CheckpointTuner >::resilient_step_collection( resilientContext< Derived > & ctxt, const std::string & name, const step_type & userStep )
        : step_collection< UserStep, Tuner, CheckpointTuner >( ctxt, name, userStep ), m_step_checkpoint(super_type::getId()), m_resilient_contex(ctxt), m_communicator(*this)
    {
        m_resilient_contex.registerStepCheckPoint( &m_step_checkpoint );
    }

    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    template< typename Derived, typename UserStepTag, typename UserStep, typename Tuner, typename CheckpointTuner >
    resilient_step_collection< Derived, UserStepTag, UserStep, Tuner, CheckpointTuner >::resilient_step_collection( resilientContext< Derived > & ctxt, const tuner_type & tnr, const std::string & name )
        : step_collection< UserStep, Tuner, CheckpointTuner >( ctxt, tnr, name ), m_step_checkpoint(super_type::getId()), m_resilient_contex(ctxt), m_communicator(*this)
    {
        m_resilient_contex.registerStepCheckPoint( &m_step_checkpoint );
    }

    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    template< typename Derived, typename UserStepTag, typename UserStep, typename Tuner, typename CheckpointTuner >
    resilient_step_collection< Derived, UserStepTag, UserStep, Tuner, CheckpointTuner >::resilient_step_collection( resilientContext< Derived > & ctxt, const std::string & name,
                                                         const step_type & userStep, const tuner_type & tnr )
        : step_collection< UserStep, Tuner, CheckpointTuner >( ctxt, name, userStep, tnr ), m_step_checkpoint(), m_resilient_contex(ctxt), m_communicator(*this)
    {
    }

    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    template< typename Derived, typename UserStepTag, typename UserStep, typename Tuner, typename CheckpointTuner >
    resilient_step_collection< Derived, UserStepTag, UserStep, Tuner, CheckpointTuner >::~resilient_step_collection() {}


    template< typename Derived, typename UserStepTag, typename UserStep, typename Tuner, typename CheckpointTuner >
    void resilient_step_collection< Derived, UserStepTag, UserStep, Tuner, CheckpointTuner >::processPut( UserStepTag putter, void * itemid, int itemCollectionId)
    {
    	m_step_checkpoint.processItemPut( putter, super_type::getId(), itemid, itemCollectionId);
    }

    template< typename Derived, typename UserStepTag, typename UserStep, typename Tuner, typename CheckpointTuner >
    void resilient_step_collection< Derived, UserStepTag, UserStep, Tuner, CheckpointTuner >::processPrescribe( UserStepTag prescriber, void * tagid, int tagCollectionId)
    {
    	m_step_checkpoint.processStepPrescribe( prescriber, super_type::getId(), tagid , tagCollectionId);
    }

    template< typename Derived, typename UserStepTag, typename UserStep, typename Tuner, typename CheckpointTuner >
    void resilient_step_collection< Derived, UserStepTag, UserStep, Tuner, CheckpointTuner >::processDone( UserStepTag step, int stepColId, int puts, int prescribes ) const
    {
    	if ( Internal::distributor::myPid() == 0) {
        	m_step_checkpoint.processStepDone( step, stepColId, puts, prescribes);
    	} else {
    	    serializer * ser = m_resilient_contex.dist_context::new_serializer( &m_communicator );
    	    //Order is very important since we pass the serialized datastrc to the remote checkpoint object!
    	    (*ser) & checkpoint_tuner_types::DONE & stepColId & step & puts & prescribes;
    	    m_resilient_contex.dist_context::send_msg(ser, 0);
    	}
    }

	///////////////////////////////////////////////////////////////////////
	/// Implementation of CnC::resilient_step_collection::communicator ////
    ///////////////////////////////////////////////////////////////////////

    template< typename Derived, typename UserStepTag, typename UserStep, typename Tuner, typename CheckpointTuner >
    resilient_step_collection< Derived, UserStepTag, UserStep, Tuner, CheckpointTuner >::communicator::communicator(resilient_step_collection< Derived, UserStepTag, UserStep, Tuner, CheckpointTuner > & r): m_resilient_step_collection(r) {
    	m_resilient_step_collection.m_resilient_contex.subscribe(this);
		std::cout << " creating res ctxt comm " << std::endl;
	}

    template< typename Derived, typename UserStepTag, typename UserStep, typename Tuner, typename CheckpointTuner >
    resilient_step_collection< Derived, UserStepTag, UserStep, Tuner, CheckpointTuner >::communicator::~communicator() {
    	m_resilient_step_collection.m_resilient_contex.unsubscribe(this);
	}

    template< typename Derived, typename UserStepTag, typename UserStep, typename Tuner, typename CheckpointTuner >
	void resilient_step_collection< Derived, UserStepTag, UserStep, Tuner, CheckpointTuner >::communicator::recv_msg( serializer * ser ) {
		char msg_tag;
		(* ser) & msg_tag;

		switch (msg_tag) {
			case checkpoint_tuner_types::DONE:
			{
				int step_collection_id;
				(* ser) & step_collection_id;
				StepCheckpoint_i& i_ = m_resilient_step_collection.m_step_checkpoint;
				i_.processStepDone(ser);
				if ( Internal::distributor::myPid() == 0) {
					m_resilient_step_collection.m_resilient_contex.checkForCrash();
				}
				break;
			}

			default:
				CNC_ABORT( "Protocol error: unexpected message tag." );
			}
		}

    template< typename Derived, typename UserStepTag, typename UserStep, typename Tuner, typename CheckpointTuner >
	void resilient_step_collection< Derived, UserStepTag, UserStep, Tuner, CheckpointTuner >::communicator::unsafe_reset( bool dist ) {}



} // end namespace CnC

#endif //RESILIENT_STEP_COLLECTION_HH_ALREADY_INCLUDED
