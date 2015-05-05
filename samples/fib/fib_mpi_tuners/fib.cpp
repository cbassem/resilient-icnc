//********************************************************************************
// Copyright (c) 2007-2014 Intel Corporation. All rights reserved.              **
//                                                                              **
// Redistribution and use in source and binary forms, with or without           **
// modification, are permitted provided that the following conditions are met:  **
//   * Redistributions of source code must retain the above copyright notice,   **
//     this list of conditions and the following disclaimer.                    **
//   * Redistributions in binary form must reproduce the above copyright        **
//     notice, this list of conditions and the following disclaimer in the      **
//     documentation and/or other materials provided with the distribution.     **
//   * Neither the name of Intel Corporation nor the names of its contributors  **
//     may be used to endorse or promote products derived from this software    **
//     without specific prior written permission.                               **
//                                                                              **
// This software is provided by the copyright holders and contributors "as is"  **
// and any express or implied warranties, including, but not limited to, the    **
// implied warranties of merchantability and fitness for a particular purpose   **
// are disclaimed. In no event shall the copyright owner or contributors be     **
// liable for any direct, indirect, incidental, special, exemplary, or          **
// consequential damages (including, but not limited to, procurement of         **
// substitute goods or services; loss of use, data, or profits; or business     **
// interruption) however caused and on any theory of liability, whether in      **
// contract, strict liability, or tort (including negligence or otherwise)      **
// arising in any way out of the use of this software, even if advised of       **
// the possibility of such damage.                                              **
//********************************************************************************

// compute fibonacci numbers
//

#define _CRT_SECURE_NO_DEPRECATE // to keep the VS compiler happy with TBB

// let's use a large type to store fib numbers
typedef unsigned long long fib_type;

#include "fib.h"
#include <typeinfo>
#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>


int fib_cr_step_tuner::getNrOfPrescribes() const {
	return 0;
};

int fib_cr_step_tuner::getNrOfPuts() const {
	return 1;
};

int fib_cr_item_tuner::getNrOfgets(const int & tag) const {
	if (tag > 0) {
		return 2;
	} else {
		return 1;
	}
}


// the actual step code computing the fib numbers goes here
int fib_step::execute( const int & tag, fib_context & ctxt ) const
{
    switch( tag ) {
        case 0 : ctxt.m_fibs.put(tag, ctxt.m_steps, tag, 0 ); break;
        case 1 : ctxt.m_fibs.put(tag, ctxt.m_steps, tag, 1 ); break;
        default :
            // get previous 2 results
            fib_type f_1; ctxt.m_fibs.get( tag, ctxt.m_steps, tag - 1, f_1 );
            fib_type f_2; ctxt.m_fibs.get( tag, ctxt.m_steps, tag - 2, f_2 );
            // put our result
            ctxt.m_fibs.put( tag, ctxt.m_steps, tag, f_1 + f_2 );
    }
    return CnC::CNC_Success;
}

void handler(int sig) {
  void *array[10];
  size_t size;

  // get void*'s for all entries on the stack
  size = backtrace(array, 10);

   //print out all the frames to stderr
  fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, STDERR_FILENO);
  exit(1);
}

int main( int argc, char* argv[] )
{
	signal(SIGSEGV, handler);
    CnC::dist_cnc_init<fib_context> _dinit;
    int n = 42;
    // eval command line args
    if( argc < 2 ) {
        std::cerr << "usage: " << argv[0] << " n\nUsing default value " << n << std::endl;
    } else n = atol( argv[1] );

    // create context
    fib_context ctxt;
    CnC::debug::collect_scheduler_statistics( ctxt );


    // put tags to initiate evaluation
    for( int i = 0; i <= n; ++i ) ctxt.m_tags.put( i );
    std::cout<<"start waiting" <<std::endl;
    // wait for completion
    ctxt.wait();
    std::cout<<"done waiting" <<std::endl;
    // get result
    fib_type res2;
    ctxt.m_fibs.get( n, res2 );

    // print result
    std::cout << "fib (" << n << "): " << res2 << std::endl;

    ctxt.print_checkpoint();

    return 0;
}
