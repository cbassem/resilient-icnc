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


    template< typename Derived, typename Tag, typename Item, typename Tuner, typename CheckpointTuner >
    resilient_item_collection< Derived, Tag, Item, Tuner, CheckpointTuner >::resilient_item_collection( resilientContext< Derived > & context, const std::string & name )
        : item_collection< Tag, Item, Tuner, CheckpointTuner >( context, name ), m_item_checkpoint(), m_resilient_contex(context)
    {
    	m_resilient_contex.registerItemCheckpoint( &m_item_checkpoint );
    }

    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    template< typename Derived, typename Tag, typename Item, typename Tuner, typename CheckpointTuner >
    resilient_item_collection< Derived, Tag, Item, Tuner, CheckpointTuner >::resilient_item_collection( resilientContext< Derived > & context, const std::string & name, const Tuner & tnr )
        : item_collection< Tag, Item, Tuner, CheckpointTuner >( context, name, tnr ), m_item_checkpoint(), m_resilient_contex(context)
    {
    	m_resilient_contex.registerItemCheckpoint( &m_item_checkpoint );
    } 

    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    template< typename Derived, typename Tag, typename Item, typename Tuner, typename CheckpointTuner >
    resilient_item_collection< Derived, Tag, Item, Tuner, CheckpointTuner >::resilient_item_collection( resilientContext< Derived > & context, const Tuner & tnr )
        : item_collection< Tag, Item, Tuner, CheckpointTuner >( context, tnr ), m_item_checkpoint(), m_resilient_contex(context)
    {
    	m_resilient_contex.registerItemCheckpoint( &m_item_checkpoint );
    } 

    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    template< typename Derived, typename Tag, typename Item, typename Tuner, typename CheckpointTuner >
    resilient_item_collection< Derived, Tag, Item, Tuner, CheckpointTuner >::~resilient_item_collection()
    {
    	m_resilient_contex.registerItemCheckpoint( &m_item_checkpoint );
    }


    template< typename Derived, typename Tag, typename Item, typename Tuner, typename CheckpointTuner >
    void resilient_item_collection< Derived, Tag, Item, Tuner, CheckpointTuner >::put( const Tag & t, const Item & i )
    {
    	void * id = m_item_checkpoint.put( t, i );
    	super_type::put( t, i );
    }

    template< typename Derived, typename Tag, typename Item, typename Tuner, typename CheckpointTuner >
    template< typename UserStepTag, typename UserStep, typename STuner, typename SCheckpointTuner >
    void resilient_item_collection< Derived, Tag, Item, Tuner, CheckpointTuner >::put(
    		const UserStepTag & putter,
    		CnC::resilient_step_collection< Derived, UserStepTag, UserStep, STuner, SCheckpointTuner>& putterColl,
    		const Tag & t,
    		const Item & i)
    {
    	void * itemid = m_item_checkpoint.put( t, i );
    	putterColl.processPut(putter, itemid, super_type::getId());
    	item_collection< Tag, Item, Tuner, CheckpointTuner >::put( putter, putterColl, t, i );
    }

    template< typename Derived, typename Tag, typename Item, typename Tuner, typename CheckpointTuner >
    void resilient_item_collection< Derived, Tag, Item, Tuner, CheckpointTuner >::restart_put(const Tag & user_tag, const Item & item)
    {
    	item_collection< Tag, Item, Tuner, CheckpointTuner >::restart_put( user_tag, item);
    }

	///////////////////////////////////////////////////////////////////////
	/// Implementation of CnC::resilient_item_collection::communicator ////
    ///////////////////////////////////////////////////////////////////////

    template< typename Derived, typename Tag, typename Item, typename Tuner, typename CheckpointTuner >
	resilient_item_collection< Derived, Tag, Item, Tuner, CheckpointTuner >::communicator::communicator(resilient_item_collection< Derived, Tag, Item, Tuner, CheckpointTuner > & r): m_resilient_item_collection(r) {
    	m_resilient_item_collection.m_resilient_contex.subscribe(this);
		std::cout << " creating res ctxt comm " << std::endl;
	}

    template< typename Derived, typename Tag, typename Item, typename Tuner, typename CheckpointTuner >
    resilient_item_collection< Derived, Tag, Item, Tuner, CheckpointTuner >::communicator::~communicator() {
    	m_resilient_item_collection.m_resilient_contex.unsubscribe(this);
	}

    template< typename Derived, typename Tag, typename Item, typename Tuner, typename CheckpointTuner >
	void resilient_item_collection< Derived, Tag, Item, Tuner, CheckpointTuner >::communicator::recv_msg( serializer * ser ) {
		char msg_tag;
		(* ser) & msg_tag;

		switch (msg_tag) {
//			case ???:
//			{
//
//			}

			default:
				CNC_ABORT( "Protocol error: unexpected message tag." );
			}
		}

	template< typename Derived, typename Tag, typename Item, typename Tuner, typename CheckpointTuner >
	void resilient_item_collection< Derived, Tag, Item, Tuner, CheckpointTuner >::communicator::unsafe_reset( bool dist ) {}

} // namespace CnC

#endif // _CnC_RESILIENT_ITEM_COLLECTION_H_
