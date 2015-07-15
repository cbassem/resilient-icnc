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
  Implementation of CnC::resilient_item_collection
  Mostly very thin wrappers calling item_collection_base.
  The wrappers hide internal functionality from the API.
*/

#ifndef _CnC_RESILIENT_ITEM_COLLECTION_H_
#define _CnC_RESILIENT_ITEM_COLLECTION_H_

namespace CnC {


    template< typename Derived, typename Tag, typename Item, typename Tuner, typename CheckpointTuner, typename Strategy >
    resilient_item_collection< Derived, Tag, Item, Tuner, CheckpointTuner, Strategy >::resilient_item_collection( resilientContext< Derived > & context, const std::string & name )
        : item_collection< Tag, Item, Tuner, CheckpointTuner >( context, name ),
		  m_resilient_contex(context),
		  m_ctuner(Internal::get_default_checkpoint_tuner<CheckpointTuner>()),
		  m_strategy(new strategy_t(*this))
    {}

    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    template< typename Derived, typename Tag, typename Item, typename Tuner, typename CheckpointTuner, typename Strategy >
    resilient_item_collection< Derived, Tag, Item, Tuner, CheckpointTuner, Strategy >::resilient_item_collection( resilientContext< Derived > & context, const std::string & name, const Tuner & tnr )
        : item_collection< Tag, Item, Tuner, CheckpointTuner >( context, name, tnr ),
		  m_resilient_contex(context),
		  m_ctuner(Internal::get_default_checkpoint_tuner<CheckpointTuner>()),
		  m_strategy(new strategy_t(*this))
    {}

    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    template< typename Derived, typename Tag, typename Item, typename Tuner, typename CheckpointTuner, typename Strategy >
    resilient_item_collection< Derived, Tag, Item, Tuner, CheckpointTuner, Strategy >::resilient_item_collection( resilientContext< Derived > & context, const Tuner & tnr )
        : item_collection< Tag, Item, Tuner, CheckpointTuner >( context, tnr ),
		  m_resilient_contex(context),
		  m_ctuner(Internal::get_default_checkpoint_tuner<CheckpointTuner>()),
		  m_strategy(new strategy_t(*this))
    {}

    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    template< typename Derived, typename Tag, typename Item, typename Tuner, typename CheckpointTuner, typename Strategy >
    resilient_item_collection< Derived, Tag, Item, Tuner, CheckpointTuner, Strategy >::~resilient_item_collection()
    {
    	delete m_strategy;
    }


    template< typename Derived, typename Tag, typename Item, typename Tuner, typename CheckpointTuner, typename Strategy >
    void resilient_item_collection< Derived, Tag, Item, Tuner, CheckpointTuner, Strategy >::put( const Tag & t, const Item & i )
    {
    	m_strategy->processPut(t, i);
    	super_type::put( t, i );
    }

    template< typename Derived, typename Tag, typename Item, typename Tuner, typename CheckpointTuner, typename Strategy >
    template< typename UserStepTag, typename UserStep, typename STuner, typename SCheckpointTuner, typename SStrategy >
    void resilient_item_collection< Derived, Tag, Item, Tuner, CheckpointTuner, Strategy >::put(
    		const UserStepTag & putter,
    		CnC::resilient_step_collection< Derived, UserStepTag, UserStep, STuner, SCheckpointTuner, SStrategy>& putterColl,
    		const Tag & t,
    		const Item & i)
    {
    	m_strategy->processPut(putter, putterColl, t, i);
    	super_type::put( putter, putterColl, t, i );
    }

    template< typename Derived, typename Tag, typename Item, typename Tuner, typename CheckpointTuner, typename Strategy >
    void resilient_item_collection< Derived, Tag, Item, Tuner, CheckpointTuner, Strategy >::restart_put(const Tag & user_tag, const Item & item)
    {
    	item_collection< Tag, Item, Tuner, CheckpointTuner >::restart_put( user_tag, item);
    }

    template< typename Derived, typename Tag, typename Item, typename Tuner, typename CheckpointTuner, typename Strategy >
    void resilient_item_collection< Derived, Tag, Item, Tuner, CheckpointTuner, Strategy >::get( const Tag & tag, Item & item ) const
    {
    	super_type::get(tag, item);
    }

    template< typename Derived, typename Tag, typename Item, typename Tuner, typename CheckpointTuner, typename Strategy >
    template< typename UserStepTag, typename UserStep, typename STuner, typename SCheckpointTuner, typename SStrategy >
    void resilient_item_collection< Derived, Tag, Item, Tuner, CheckpointTuner, Strategy >::get(
    		const UserStepTag & getter,
    		CnC::resilient_step_collection< Derived, UserStepTag, UserStep, STuner, SCheckpointTuner, SStrategy > & getterColl,
			const Tag & tag, Item & item )
    {
    	m_strategy->processGet(getter, getterColl, tag, item);
		item_collection< Tag, Item, Tuner, CheckpointTuner >::get( tag, item );
    }

    template< typename Derived, typename Tag, typename Item, typename Tuner, typename CheckpointTuner, typename Strategy >
    typename CnC::item_collection<Tag, Item, Tuner, CheckpointTuner>::const_iterator resilient_item_collection< Derived, Tag, Item, Tuner, CheckpointTuner, Strategy >::begin() const
    {
    	typename CnC::item_collection<Tag, Item, Tuner, CheckpointTuner>::const_iterator _tmp( this, super_type::m_itemCollection.begin() );
    	typename CnC::item_collection<Tag, Item, Tuner, CheckpointTuner>::const_iterator _e( end() );
        while( _tmp != _e && ! _tmp.valid() ) ++_tmp;
        return _tmp;
    }

    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    template< typename Derived, typename Tag, typename Item, typename Tuner, typename CheckpointTuner, typename Strategy >
    typename CnC::item_collection<Tag, Item, Tuner, CheckpointTuner>::const_iterator resilient_item_collection< Derived, Tag, Item, Tuner, CheckpointTuner, Strategy >::end() const
    {
        return const_iterator( this, super_type::m_itemCollection.end() );
    }

} // namespace CnC

#endif // _CnC_RESILIENT_ITEM_COLLECTION_H_
