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
        : tag_collection< Tag, Tuner, CheckpointTuner >( context, name ),
		  m_resilient_contex(context),
		  m_strategy(new resilient_tag_collection_strategy_naive<resilient_tag_collection< Derived, Tag, Tuner, CheckpointTuner >, Tag >(*this))
    {}

    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    template< typename Derived, typename Tag, typename Tuner, typename CheckpointTuner  >
    resilient_tag_collection< Derived, Tag, Tuner, CheckpointTuner>::resilient_tag_collection( resilientContext< Derived > & context, const Tuner & tnr )
        : tag_collection< Tag, Tuner, CheckpointTuner >( context, tnr ),
		  m_resilient_contex(context),
		  m_strategy(new resilient_tag_collection_strategy_naive<resilient_tag_collection< Derived, Tag, Tuner, CheckpointTuner >, Tag >(*this))
    {}

    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    template<  typename Derived, typename Tag, typename Tuner, typename CheckpointTuner >
    resilient_tag_collection< Derived, Tag, Tuner, CheckpointTuner >::resilient_tag_collection( resilientContext< Derived > & context, const std::string & name, const Tuner & tnr )
        : tag_collection< Tag, Tuner, CheckpointTuner >( context, name, tnr ),
		  m_resilient_contex(context),
		  m_strategy(new resilient_tag_collection_strategy_naive<resilient_tag_collection< Derived, Tag, Tuner, CheckpointTuner >, Tag >(*this))
    {}

    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    template< typename Derived, typename Tag, typename Tuner, typename CheckpointTuner >
    resilient_tag_collection< Derived, Tag, Tuner, CheckpointTuner >::~resilient_tag_collection()
    {
    	delete m_strategy;
    }

    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    template< typename Derived, typename Tag, typename Tuner, typename CheckpointTuner >
    void resilient_tag_collection< Derived, Tag, Tuner, CheckpointTuner >::put( const Tag & t )
    {
    	m_strategy->processPrescribe( t );
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

    	m_strategy->processPrescribe( prescriber, prescriberCol, t);
    	tag_collection< Tag, Tuner, CheckpointTuner >::put( prescriber, prescriberCol, t );
    }

    template< typename Derived, typename Tag, typename Tuner, typename CheckpointTuner >
    void resilient_tag_collection< Derived, Tag, Tuner, CheckpointTuner >::restart_put( const Tag & t ) {
    	//tag_collection< Tag, Tuner, CheckpointTuner >::restart_put( t );
    	tag_collection< Tag, Tuner, CheckpointTuner >::put( t );
    }

    template< typename Derived, typename Tag, typename Tuner, typename CheckpointTuner >
    template< typename SDerived, typename UserStepTag, typename UserStep, typename STuner, typename Arg, typename SCheckpointTuner >
    error_type resilient_tag_collection< Derived, Tag, Tuner, CheckpointTuner >::prescribes( resilient_step_collection< SDerived, UserStepTag, UserStep, STuner, SCheckpointTuner > & s, Arg & arg )
	{
    	super_type::prescribes(s, arg);
    	StepCheckpoint<Tag> * cp_= s.getStepCheckpoint();
    	m_strategy->prescribeStepCheckpoint(cp_);
	}





} // namespace CnC

#endif // _CnC_RESILIENT_TAG_COLLECTION_H_
