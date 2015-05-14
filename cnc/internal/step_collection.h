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
  Implementation of CnC::step_collection.
  Has almost no functionality, mostly carries type information.
  The actual functionality is in step_launcher, which gets instantiated
  when the step_collection is prescribed.
*/

#ifndef STEP_COLLECTION_HH_ALREADY_INCLUDED
#define STEP_COLLECTION_HH_ALREADY_INCLUDED

namespace CnC {

    namespace Internal {
        template< typename Tuner > const Tuner & get_default_tuner();
    }

    template< typename UserStep, typename Tuner, typename CheckpointTuner >
    template< typename Derived >
    step_collection< UserStep, Tuner, CheckpointTuner >::step_collection( context< Derived > & ctxt )
        : Internal::traceable( std::string() ),
          m_userStep( UserStep() ),
          m_tuner( Internal::get_default_tuner< Tuner >() ),
          m_context( ctxt ),
          m_checkpoint_tuner( Internal::get_default_checkpoint_tuner< CheckpointTuner >() ),
          m_id(ctxt.get_next_step_col_id())
    {
        // FIXME register with context
    }

    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    template< typename UserStep, typename Tuner, typename CheckpointTuner >
    template< typename Derived >
    step_collection< UserStep, Tuner, CheckpointTuner >::step_collection( context< Derived > & ctxt, const std::string & name )
        : Internal::traceable( name ),
          m_userStep( UserStep() ),
          m_tuner( Internal::get_default_tuner< Tuner >() ),
          m_context( ctxt ),
          m_checkpoint_tuner( Internal::get_default_checkpoint_tuner< CheckpointTuner >() ),
          m_id(ctxt.get_next_step_col_id())
    {
        traceable::set_name( name );
        // FIXME register with context
    }
    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    template< typename UserStep, typename Tuner, typename CheckpointTuner >
    template< typename Derived >
    step_collection< UserStep, Tuner, CheckpointTuner >::step_collection( context< Derived > & ctxt, const std::string & name, const step_type & userStep )
        : Internal::traceable( name ),
          m_userStep( userStep ),
          m_tuner( Internal::get_default_tuner< Tuner >() ),
          m_context( ctxt ),
          m_checkpoint_tuner( Internal::get_default_checkpoint_tuner< CheckpointTuner >() ),
          m_id(ctxt.get_next_step_col_id())
    {
        traceable::set_name( name );
        // FIXME register with context
    }

    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    template< typename UserStep, typename Tuner, typename CheckpointTuner >
    template< typename Derived >
    step_collection< UserStep, Tuner, CheckpointTuner >::step_collection( context< Derived > & ctxt, const tuner_type & tnr, const std::string & name )
        : Internal::traceable( name ),
          m_userStep( UserStep() ),
          m_tuner( tnr ),
          m_context( ctxt ),
          m_checkpoint_tuner( Internal::get_default_checkpoint_tuner< CheckpointTuner >() ),
          m_id(ctxt.get_next_step_col_id())
    {
        traceable::set_name( name );
        // FIXME register with context
    }

    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    template< typename UserStep, typename Tuner, typename CheckpointTuner >
    template< typename Derived >
    step_collection< UserStep, Tuner, CheckpointTuner >::step_collection( context< Derived > & ctxt, const std::string & name,
                                                         const step_type & userStep, const tuner_type & tnr )
        : Internal::traceable( name ),
          m_userStep( userStep ),
          m_tuner( tnr ),
          m_context( ctxt ),
          m_checkpoint_tuner( Internal::get_default_checkpoint_tuner< CheckpointTuner >() ),
          m_id(ctxt.get_next_step_col_id())
    {
        traceable::set_name( name );
        // FIXME register with context
    }

    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    template< typename UserStep, typename Tuner, typename CheckpointTuner >
    step_collection< UserStep, Tuner, CheckpointTuner >::~step_collection()
    {
        m_context.reset_distributables( true );
    }

    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    template< typename UserStep, typename tuner, typename CheckpointTuner >
    template< typename DataTag, typename Item, typename ITuner, typename ICheckpointTuner >
    void step_collection< UserStep, tuner, CheckpointTuner >:: consumes( CnC::item_collection< DataTag, Item, ITuner, ICheckpointTuner > & )
    {
        // currently doing nothing
    } 

    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    template< typename UserStep, typename tuner, typename CheckpointTuner >
    template< typename DataTag, typename Item, typename ITuner, typename ICheckpointTuner >
    void step_collection< UserStep, tuner, CheckpointTuner >::produces( CnC::item_collection< DataTag, Item, ITuner, ICheckpointTuner > & )
    {
        // currently doing nothing
    } 

    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    template< typename UserStep, typename tuner, typename CheckpointTuner >
    template< typename ControlTag, typename TTuner, typename TCheckpointTuner >
    void step_collection< UserStep, tuner, CheckpointTuner >::controls( CnC::tag_collection< ControlTag, TTuner, TCheckpointTuner > & )
    {
        // currently doing nothing
    }

    template< typename UserStep, typename tuner, typename CheckpointTuner >
    void  step_collection< UserStep, tuner, CheckpointTuner >::processDone( void * step, int stepColId, int puts, int prescribes, int gets ) {
       std::cout << "oops we should not be here" <<std::cout;
    }

    template< typename UserStep, typename Tuner, typename CheckpointTuner >
    const int step_collection< UserStep, Tuner, CheckpointTuner >::getId() const
	{
    	return m_id;
	}

} // end namespace CnC

#endif //STEP_COLLECTION_HH_ALREADY_INCLUDED
