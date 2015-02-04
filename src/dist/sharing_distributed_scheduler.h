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

#ifndef _CnC_SHARING_DISTRIBUTED_SCHEDULER_H
#define _CnC_SHARING_DISTRIBUTED_SCHEDULER_H

#include <src/dist/distributed_scheduler.h>
#include <tbb/atomic.h>
#include <tbb/concurrent_queue.h>

namespace CnC {

    class serializer;

    namespace Internal {

        class schedulable;

        class sharing_distributed_scheduler : public distributed_scheduler
        {
            public:
                sharing_distributed_scheduler( context_base & ctxt, scheduler_i & scheduler );
                virtual ~sharing_distributed_scheduler();
                virtual void loadBalanceCallback();
            virtual void on_received_workchunk( CnC::serializer* ser, int senderId, int n );
                bool postRequest();
                virtual bool migrate_step(unsigned int, schedulable* s);

            private:
                bool hasClientRequests(int& clientId);
                void putClientRequest(int clientId);
                virtual void recv_work_request( CnC::serializer* ser, int senderId );

                tbb::atomic<int> m_requestSent;
                tbb::atomic<int>* m_clientRequests;
                tbb::concurrent_bounded_queue<int> m_allClientRequests;
                static const int HOST = 0;
                static const int FEW_STEPS = (1<<8);
                static const int HOST_FEW_STEPS = FEW_STEPS>>0;
                static const int LOTS_OF_STEPS = FEW_STEPS<<2;
                tbb::atomic<int> m_last;
        };

    } // namespace Internal
} // namespace CnC

#endif // _CnC_SHARING_DISTRIBUTED_SCHEDULER_H
