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
  Implementation of CnC::resilient_tag_collection
*/

#ifndef _CnC_RESILIENT_TAG_COLLECTION_H_
#define _CnC_RESILIENT_TAG_COLLECTION_H_

#include <cnc/internal/tag_collection_base.h>
#include <cnc/internal/step_launcher.h>
#include <cnc/internal/dist/distributor.h>

namespace CnC {

    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    template< typename Derived, typename Tag, typename Tuner, typename CheckpointTuner >
    resilient_tag_collection< Derived, Tag, Tuner, CheckpointTuner  >::resilient_tag_collection( resilientContext< Derived > & context, const std::string & name )
        : tag_collection< Tag, Tuner, CheckpointTuner >( context, name ), m_tag_checkpoint(super_type::getId()), m_resilient_contex(context), m_communicator(*this)
    {
    	m_resilient_contex.registerTagCheckpoint( &m_tag_checkpoint );
    }

    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    template< typename Derived, typename Tag, typename Tuner, typename CheckpointTuner  >
    resilient_tag_collection< Derived, Tag, Tuner, CheckpointTuner>::resilient_tag_collection( resilientContext< Derived > & context, const Tuner & tnr )
        : tag_collection< Tag, Tuner, CheckpointTuner >( context, tnr ), m_tag_checkpoint(super_type::getId()), m_resilient_contex(context), m_communicator(*this)
    {
    	m_resilient_contex.registerTagCheckpoint( &m_tag_checkpoint );
    }

    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    template<  typename Derived, typename Tag, typename Tuner, typename CheckpointTuner >
    resilient_tag_collection< Derived, Tag, Tuner, CheckpointTuner >::resilient_tag_collection( resilientContext< Derived > & context, const std::string & name, const Tuner & tnr )
        : tag_collection< Tag, Tuner, CheckpointTuner >( context, name, tnr ), m_tag_checkpoint(super_type::getId()), m_resilient_contex(context), m_communicator(*this)
    {
    	m_resilient_contex.registerTagCheckpoint( &m_tag_checkpoint );
    }

    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    template< typename Derived, typename Tag, typename Tuner, typename CheckpointTuner >
    resilient_tag_collection< Derived, Tag, Tuner, CheckpointTuner >::~resilient_tag_collection()
    {
    	m_resilient_contex.registerTagCheckpoint( &m_tag_checkpoint );
    }

    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    template< typename Derived, typename Tag, typename Tuner, typename CheckpointTuner >
    void resilient_tag_collection< Derived, Tag, Tuner, CheckpointTuner >::put( const Tag & t )
    {
    	void * id = m_tag_checkpoint.put( t );
        tag_collection< Tag, Tuner, CheckpointTuner >::put( t );
    }

    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    template< typename Derived, typename Tag, typename Tuner, typename CheckpointTuner >
    template< typename UserStepTag, typename UserStep, typename STuner, typename SCheckpointTuner >
    void resilient_tag_collection< Derived, Tag, Tuner, CheckpointTuner >::put(
    		const UserStepTag & prescriber,
    		CnC::resilient_step_collection< Derived, UserStepTag, UserStep, STuner, SCheckpointTuner> & prescriberCol ,
    		const Tag & t )
    {
    	if ( Internal::distributor::myPid() == 0) {
        	void * tagid = m_tag_checkpoint.put( t );
        	prescriberCol.processPrescribe( prescriber, tagid, super_type::getId());
    	} else {
    	    serializer * ser = m_resilient_contex.dist_context::new_serializer( &m_communicator );
    	    //Order is very important since we pass the serialized datastrc to the remote checkpoint object!
    	    (*ser) & checkpoint_tuner_types::PRESCRIBE & t & prescriberCol.getId() & prescriber;
    	    m_resilient_contex.dist_context::send_msg(ser, 0);
    	}
    	tag_collection< Tag, Tuner, CheckpointTuner >::put( prescriber, prescriberCol, t );
    }

    template< typename Derived, typename Tag, typename Tuner, typename CheckpointTuner >
    void resilient_tag_collection< Derived, Tag, Tuner, CheckpointTuner >::restart_put( const Tag & t ) {
    	tag_collection< Tag, Tuner, CheckpointTuner >::restart_put( t );
    }

    template< typename Derived, typename Tag, typename Tuner, typename CheckpointTuner >
    template< typename SDerived, typename UserStepTag, typename UserStep, typename STuner, typename Arg, typename SCheckpointTuner >
    error_type resilient_tag_collection< Derived, Tag, Tuner, CheckpointTuner >::prescribes( resilient_step_collection< SDerived, UserStepTag, UserStep, STuner, SCheckpointTuner > & s, Arg & arg )
	{
    	super_type::prescribes(s, arg);
    	StepCheckpoint<Tag> * cp_= s.getStepCheckpoint();
    	m_tag_checkpoint.prescribeStepCheckpoint(cp_);
	}


	//////////////////////////////////////////////////////////////////////
	/// Implementation of CnC::resilient_tag_collection::communicator ////
    //////////////////////////////////////////////////////////////////////

    template< typename Derived, typename Tag, typename Tuner, typename CheckpointTuner >
    resilient_tag_collection< Derived, Tag, Tuner, CheckpointTuner >::communicator::communicator(resilient_tag_collection< Derived, Tag, Tuner, CheckpointTuner > & r): m_resilient_tag_collection(r) {
    	m_resilient_tag_collection.m_resilient_contex.subscribe(this);
		std::cout << " creating res ctxt comm " << std::endl;
	}

    template< typename Derived, typename Tag, typename Tuner, typename CheckpointTuner >
    resilient_tag_collection< Derived, Tag, Tuner, CheckpointTuner >::communicator::~communicator() {
    	m_resilient_tag_collection.m_resilient_contex.unsubscribe(this);
	}

    template< typename Derived, typename Tag, typename Tuner, typename CheckpointTuner >
	void resilient_tag_collection< Derived, Tag, Tuner, CheckpointTuner >::communicator::recv_msg( serializer * ser ) {
		char msg_tag;
		(* ser) & msg_tag;

		switch (msg_tag) {
			case checkpoint_tuner_types::PRESCRIBE:
			{
				Tag tag;
				int prescriber_collection_id;
				(* ser) & tag & prescriber_collection_id;
	        	void * tagid = m_resilient_tag_collection.m_tag_checkpoint.put( tag );
	        	StepCheckpoint_i* i_ = m_resilient_tag_collection.m_resilient_contex.getStepCheckPoint(prescriber_collection_id);
				i_->processStepPrescribe(ser, tagid);
				break;
			}

			default:
				CNC_ABORT( "Protocol error: unexpected message tag." );
			}
		}

    template< typename Derived, typename Tag, typename Tuner, typename CheckpointTuner >
	void resilient_tag_collection< Derived, Tag, Tuner, CheckpointTuner >::communicator::unsafe_reset( bool dist ) {}



} // namespace CnC

#endif // _CnC_RESILIENT_TAG_COLLECTION_H_
