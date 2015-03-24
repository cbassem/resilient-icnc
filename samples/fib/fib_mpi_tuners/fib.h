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

// Forward declaration of the context class (also known as graph)
struct fib_context;


// let's use a tuner to pre-declare dependencies
struct cr_step_tuner : public CnC::step_tuner<>
{
    void prescribe( const int & tag ) const;
    void done( const int & tag ) const;
};

struct cr_item_tuner : public CnC::hashmap_tuner
{
    void put_local( const int & tag, fib_type * arg ) const;
};


// The step classes

struct fib_step
{
    int execute( const int & t, fib_context & c ) const;
};

// The context class
struct fib_context : public CnC::context< fib_context >
{
    // step collections
    CnC::step_collection< fib_step, cr_step_tuner > m_steps;
    // Item collections
    CnC::item_collection< int, fib_type, cr_item_tuner > m_fibs;
    // Tag collections
    CnC::tag_collection< int > m_tags;

    // The context class constructor
    fib_context()
        : CnC::context< fib_context >(),
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
    }
};

#endif // fib_H_ALREADY_INCLUDED
