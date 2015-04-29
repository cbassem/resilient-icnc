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
        : step_collection< UserStep, Tuner, CheckpointTuner >( ctxt ), m_step_checkpoint(), m_resilient_contex(ctxt)
    {
        // FIXME register with context
    }

    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    template< typename Derived, typename UserStepTag, typename UserStep, typename Tuner, typename CheckpointTuner >
    resilient_step_collection< Derived, UserStepTag, UserStep, Tuner, CheckpointTuner >::resilient_step_collection( resilientContext< Derived > & ctxt, const std::string & name )
        : step_collection< UserStep, Tuner, CheckpointTuner >( ctxt, name), m_step_checkpoint(), m_resilient_contex(ctxt)
    {
        // FIXME register with context
    }
    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    template< typename Derived, typename UserStepTag, typename UserStep, typename Tuner, typename CheckpointTuner >
    resilient_step_collection< Derived, UserStepTag, UserStep, Tuner, CheckpointTuner >::resilient_step_collection( resilientContext< Derived > & ctxt, const std::string & name, const step_type & userStep )
        : step_collection< UserStep, Tuner, CheckpointTuner >( ctxt, name, userStep ), m_step_checkpoint(), m_resilient_contex(ctxt)
    {
        // FIXME register with context
    }

    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    template< typename Derived, typename UserStepTag, typename UserStep, typename Tuner, typename CheckpointTuner >
    resilient_step_collection< Derived, UserStepTag, UserStep, Tuner, CheckpointTuner >::resilient_step_collection( resilientContext< Derived > & ctxt, const tuner_type & tnr, const std::string & name )
        : step_collection< UserStep, Tuner, CheckpointTuner >( ctxt, tnr, name ), m_step_checkpoint(), m_resilient_contex(ctxt)
    {
        // FIXME register with context
    }

    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    template< typename Derived, typename UserStepTag, typename UserStep, typename Tuner, typename CheckpointTuner >
    resilient_step_collection< Derived, UserStepTag, UserStep, Tuner, CheckpointTuner >::resilient_step_collection( resilientContext< Derived > & ctxt, const std::string & name,
                                                         const step_type & userStep, const tuner_type & tnr )
        : step_collection< UserStep, Tuner, CheckpointTuner >( ctxt, name, userStep, tnr ), m_step_checkpoint(), m_resilient_contex(ctxt)
    {
    }

    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    template< typename Derived, typename UserStepTag, typename UserStep, typename Tuner, typename CheckpointTuner >
    resilient_step_collection< Derived, UserStepTag, UserStep, Tuner, CheckpointTuner >::~resilient_step_collection()
    {
    }


} // end namespace CnC

#endif //RESILIENT_STEP_COLLECTION_HH_ALREADY_INCLUDED
