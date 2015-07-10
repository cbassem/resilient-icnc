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
  Implementation of CnC::tag_collection
  Mostly vey thin wrappers calling tag_collection_base.
  The wrappers hide internal functinality from the API.
*/

#ifndef _CnC_TAG_COLLECTION_H_
#define _CnC_TAG_COLLECTION_H_

#include <cnc/internal/tag_collection_base.h>
#include <cnc/internal/step_launcher.h>
#include <cnc/internal/dist/distributor.h>

namespace CnC {

    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    template< typename Tag, typename Tuner, typename CheckpointTuner >
    template< class Derived >
    tag_collection< Tag, Tuner, CheckpointTuner  >::tag_collection( context< Derived > & context, const std::string & name )
        : m_tagCollection( reinterpret_cast< Internal::context_base & >( context ), name )
    {
    }

    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    template< typename Tag, typename Tuner, typename CheckpointTuner  >
    template< class Derived >
    tag_collection< Tag, Tuner, CheckpointTuner>::tag_collection( context< Derived > & context, const Tuner & tnr )
        : m_tagCollection( reinterpret_cast< Internal::context_base & >( context ), std::string(), tnr )
    {
    }

    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    template< typename Tag, typename Tuner, typename CheckpointTuner >
    template< class Derived >
    tag_collection< Tag, Tuner, CheckpointTuner >::tag_collection( context< Derived > & context, const std::string & name, const Tuner & tnr )
        : m_tagCollection( reinterpret_cast< Internal::context_base & >( context ), name, tnr )
    {
    }

    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    template< typename Tag, typename Tuner, typename CheckpointTuner >
    tag_collection< Tag, Tuner, CheckpointTuner >::~tag_collection()
    {
        m_tagCollection.get_context().reset_distributables( true );
    }

    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    template< typename Tag, typename Tuner, typename CheckpointTuner >
    void tag_collection< Tag, Tuner, CheckpointTuner >::put( const Tag & t )
    {
        m_tagCollection.Put( t );
    }

    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    template< typename Tag, typename Tuner, typename CheckpointTuner >
    template< typename PTag, typename UserStep, typename STuner, typename SCheckpointTuner >
    void tag_collection< Tag, Tuner, CheckpointTuner >::put( const PTag & prescriber, const CnC::step_collection< UserStep, STuner, SCheckpointTuner> & prescriberCol , const Tag & t )
    {
        m_tagCollection.Put( prescriber, prescriberCol.getId(), t );
    }

    template< typename Tag, typename Tuner, typename CheckpointTuner >
    void tag_collection< Tag, Tuner, CheckpointTuner >::restart_put( const Tag & t ) {
    	m_tagCollection.restart_put( t );
    }

    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    template< typename Tag, typename Tuner, typename CheckpointTuner >
    void tag_collection< Tag, Tuner, CheckpointTuner >::put_range( const Internal::no_range & ) const
    {
        CNC_ABORT( "You need to specify a range type to put a range" );
    }

    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    template< typename Tag, typename Tuner, typename CheckpointTuner >
    void tag_collection< Tag, Tuner, CheckpointTuner >::put_range( const typename Tuner::range_type & r )
    {
        if( r.size() > 0 ) {
            m_tagCollection.create_range_instances( r );
        }
    }

    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 
    template< typename Tag, typename Tuner, typename CheckpointTuner >
    typename tag_collection< Tag, Tuner, CheckpointTuner >::const_iterator tag_collection< Tag, Tuner, CheckpointTuner >::begin() const
    {
        return m_tagCollection.begin();
    }
    
    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    template< typename Tag, typename Tuner, typename CheckpointTuner >
    typename tag_collection< Tag, Tuner, CheckpointTuner >::const_iterator tag_collection< Tag, Tuner, CheckpointTuner >::end() const
    {
        return m_tagCollection.end();
    }

    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    template< typename Tag, typename Tuner, typename CheckpointTuner >
    void tag_collection< Tag, Tuner, CheckpointTuner >::unsafe_reset()
    {
        m_tagCollection.unsafe_reset( Internal::distributor::numProcs() > 1 && m_tagCollection.get_context().subscribed() );
    }

    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    template< typename Tag, typename Tuner, typename CheckpointTuner >
    size_t tag_collection< Tag, Tuner, CheckpointTuner >::size()
    {
        return m_tagCollection.size();
    }
    
    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    template< typename Tag, typename Tuner, typename CheckpointTuner >
    bool tag_collection< Tag, Tuner, CheckpointTuner >::empty()
    {
        return m_tagCollection.empty();
    }
    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    template< typename Tag, typename Tuner, typename CheckpointTuner >
    template< typename Step, typename STuner, typename Arg, typename SCheckpointTuner >
    error_type tag_collection< Tag, Tuner, CheckpointTuner >::prescribes( step_collection< Step, STuner, SCheckpointTuner > & sc, Arg & arg )
    {
        typedef Internal::step_launcher< Tag, Step, Arg, Tuner, STuner, SCheckpointTuner > the_step_launcher;
        the_step_launcher * _sl = new the_step_launcher( &m_tagCollection, arg, sc );
        m_tagCollection.addPrescribedCollection( _sl );
        return 0;
    }

    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    template< typename Tag, typename Tuner, typename CheckpointTuner >
    void tag_collection< Tag, Tuner, CheckpointTuner >::on_put( callback_type * cb )
    {
        m_tagCollection.on_put( cb );
    }

    template< typename Tag, typename Tuner, typename CheckpointTuner >
    int tag_collection< Tag, Tuner, CheckpointTuner >::getId()
    {
        return m_tagCollection.getId();
    }

    template< typename Tag, typename Tuner, typename CheckpointTuner >
    void tag_collection< Tag, Tuner, CheckpointTuner >::reset_for_restart()
    {
        m_tagCollection.do_reset();
    }



} // namespace CnC

#endif // _CnC_TAG_COLLECTION_H_
