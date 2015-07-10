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

#include "pres.h"
#include <typeinfo>
#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>


int cr_step_a_tuner::getNrOfPrescribes(const int & tag) const {
	return 1;
};

int cr_step_a_tuner::getNrOfPuts(const int & tag) const {
	return 0;
};

int cr_step_a_tuner::getNrOfGets(const int & tag) const {
	return 0;
};

int cr_step_b_tuner::getNrOfPrescribes(const int & tag) const {
	return 0;
};

int cr_step_b_tuner::getNrOfPuts(const int & tag) const {
	return 1;
};

int cr_step_b_tuner::getNrOfGets(const int & tag) const {
	return 0;
};

int cr_item_tuner::getNrOfgets(const std::pair<int, long int> & tag) const {
    return 0;
}



int step_a::execute( const int & tag, pres_context & ctxt ) const
{
    ctxt.m_tags_b.put(tag, ctxt.m_steps_a, -tag);
    return CnC::CNC_Success;
}

int step_b::execute( const int & tag, pres_context & ctxt ) const
{
	struct timeval tp;
	gettimeofday(&tp, NULL);
	long int ms = tp.tv_sec * 1000 + tp.tv_usec / 1000;
	ctxt.m_side_effects.put(tag, ctxt.m_steps_b, std::pair<int, long int>(tag, ms), tag);
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

inline std::ostream & operator<<( std::ostream& os, const std::pair<int, long int> & t )
{
    os << "[ " << t.first << ", " << t.second << " ]";
    return os;
}

int main( int argc, char* argv[] )
{
	//signal(SIGSEGV, handler);
    CnC::dist_cnc_init<pres_context> _dinit;
    int n = 42;
    // eval command line args
    if( argc < 2 ) {
        std::cerr << "usage: " << argv[0] << " n\nUsing default value " << n << std::endl;
    } else n = atol( argv[1] );

    // create context
    pres_context ctxt;
    CnC::debug::collect_scheduler_statistics( ctxt );


    // put tags to initiate evaluation
    for( int i = 0; i <= n; ++i ) ctxt.m_tags_a.put( i );
    std::cout<<"start waiting" <<std::endl;
    // wait for completion
    ctxt.wait();
    std::cout<<"done waiting" <<std::endl;
    // get result
//    fib_type res2;
//    ctxt.m_fibs.get( n, res2 );

    // print result
//    std::cout << "fib (" << n << "): " << res2 << std::endl;

    ctxt.calculate_checkpoint();

    ctxt.print_checkpoint();

    return 0;
}
