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
#ifndef fib_H_ALREADY_INCLUDED
#define fib_H_ALREADY_INCLUDED

#include <cnc/dist_cnc.h> 
#include <cnc/debug.h>

// Forward declarations
struct fib_context;
struct fib_cr_tuner;
// The step classes
struct fib_step
{
    int execute( const int & t, fib_context & c ) const;
};

typedef CnC::step_collection< fib_step, CnC::step_tuner<>, fib_cr_tuner > step_type;
typedef CnC::item_collection< int, fib_type, CnC::hashmap_tuner, fib_cr_tuner> item_type;
typedef CnC::tag_collection< int, CnC::tag_tuner<>, fib_cr_tuner > tag_type;


// let's use a tuner to pre-declare dependencies
struct fib_cr_tuner: public CnC::checkpoint_tuner<fib_context, int, fib_type, step_type, tag_type, item_type>
{
	fib_cr_tuner(CnC::Internal::distributable_context&);

	int getNrOfPuts() const;
	int getNrOfPrescribes() const;
};


// The context class
struct fib_context : public CnC::resilientContext< fib_context, int, fib_type, step_type, tag_type, item_type>
{
    // step collections
    CnC::step_collection< fib_step, CnC::step_tuner<>, fib_cr_tuner > m_steps;
    // Item collections
    CnC::item_collection< int, fib_type, CnC::hashmap_tuner, fib_cr_tuner> m_fibs;
    // Tag collections
    CnC::tag_collection< int, CnC::tag_tuner<>, fib_cr_tuner > m_tags;

    // The context class constructor
    fib_context()
        : CnC::resilientContext< fib_context, int, fib_type, step_type, tag_type, item_type >(1, 1, 1, 6, 1),
          // Initialize each step collection
          m_steps( *this ),
          // Initialize each item collection
          m_fibs( *this ),
          // Initialize each tag collection
          m_tags( *this )
    {
        // Prescriptive relations
        m_tags.prescribes( m_steps, *this );
        // Consumer relations
        m_steps.consumes( m_fibs );
        // Producer relations
        m_steps.produces( m_fibs );

        CnC::resilientContext< fib_context, int, fib_type, step_type, tag_type, item_type>::registerStepCollection(m_steps);
        CnC::resilientContext< fib_context, int, fib_type, step_type, tag_type, item_type>::registerTagCollection(m_tags);
        CnC::resilientContext< fib_context, int, fib_type, step_type, tag_type, item_type>::registerItemCollection(m_fibs);

        CnC::debug::trace_all(*this, 3);


    }
};

#endif // fib_H_ALREADY_INCLUDED
