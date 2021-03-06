/* *******************************************************************************
 *  Copyright (c) 2010-2014, Intel Corporation
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

#include <src/dist/distributed_scheduler.h>
#include <cnc/serializer.h>
#include <cnc/internal/context_base.h>
#include <cnc/internal/scheduler_i.h>
#include <cnc/internal/dist/distributor.h>
#include <cnc/internal/step_instance_base.h>
#include <cnc/internal/dist/distributable_context.h>
#include <cnc/internal/dist/distributable.h>

#include <iostream>
#include <sstream>

namespace CnC {
    namespace Internal
    {
        distributed_scheduler* distributed_scheduler::instance;

        distributed_scheduler::distributed_scheduler( context_base & ctxt, scheduler_i & scheduler )
            : m_scheduler( scheduler ),
              m_context( ctxt )
              
        {
            instance = this;
            m_context.subscribe( this );
            std::cerr << "my rank = " << distributor::myPid() << std::endl;
            m_topo.init_ring(distributor::numProcs(),distributor::myPid());
        }

        distributed_scheduler::~distributed_scheduler()
        {
            instance = NULL;
            m_context.unsubscribe( this );
        }

        /* virtual */ void distributed_scheduler::recv_msg( CnC::serializer* ser )
        {
            char marker;
            int senderId;
            (*ser) & marker & senderId;
            if (marker == WORK_REQUEST) {
                recv_work_request( ser, senderId );
            } else if (marker == WORK_CHUNK){
                int n = recv_steps( *ser );
                on_received_workchunk( ser, senderId, n );
            } else if (marker == STATE_UPDATE){
                recv_state_update( ser, senderId );
            } else {
                CNC_ASSERT("Impossible code path" == 0);
            }
        }

        bool distributed_scheduler::bcast_work_request()
        {
            if(false){Speaker spkr; spkr << "distributed_scheduler::bcast_work_request()";}
            CNC_ASSERT( m_topo.neighbors().size() );

            CnC::serializer* ser = m_context.new_serializer( this );
            char marker = WORK_REQUEST;
            int myId = distributor::myPid();
            (*ser) & marker & myId;
            m_context.bcast_msg( ser, m_topo.neighbors().data(), m_topo.neighbors().size() );
            //m_context.bcast_msg( ser );
            return true;
        }

        bool distributed_scheduler::send_work_request( int receiver )
        {
            CnC::serializer* ser = m_context.new_serializer( this );
            char marker = WORK_REQUEST;
            int myId = distributor::myPid();
            (*ser) & marker & myId;
            m_context.send_msg( ser, receiver );
            return true;
        }

        void distributed_scheduler::serialize_step( CnC::serializer & ser, schedulable & step )
        {
            CNC_ASSERT(dynamic_cast<step_instance_base*>(&step));
            step.serialize( ser );
        }

        // /* virtual */ void distributed_scheduler::activate()
        // {
        //     Eo("activation");
        //     loadBalanceCallback();
        // }

        void distributed_scheduler::send_steps_to_client( int clientId, schedulable ** steps, size_t n )
        {
            CnC::serializer* ser = m_context.new_serializer( this );
            char marker = WORK_CHUNK;
            int myId = distributor::myPid();
            (*ser) & marker & myId & n;
            while( n-- ) {
                serialize_step( *ser, *steps[n] );
            }
            m_context.send_msg( ser, clientId );
        }

        int distributed_scheduler::recv_steps( CnC::serializer & ser )
        {
            step_vec_type::size_type sz;
            ser & sz;
            for( size_type i = 0; i < sz; ++i ) {
                m_context.recv_msg( &ser );
            }
            return sz;
        }

        void distributed_scheduler::bcast_state_update( int me, int value )
        {
            // what's this gid business for?
            int g = m_context.gid();
            if (g < 0) {
                return;
            }

            CnC::serializer* ser = m_context.new_serializer( this );
            char marker = STATE_UPDATE;
            (*ser) & marker & me & value;
            m_context.bcast_msg( ser, m_topo.neighbors().data(), m_topo.neighbors().size() );
            //m_context.bcast_msg( ser );
        }

    } // namespace Internal
} // namespace CnC
