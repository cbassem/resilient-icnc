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
#include <cnc/internal/resilient_item_collection_strategy_naive.h>

// Forward declarations
struct pres_context;
// The step classes
struct step_a
{
    int execute( const int & t, pres_context & c ) const;
};

struct step_b
{
    int execute( const int & t, pres_context & c ) const;
};

//typedef CnC::step_collection< step_a, CnC::step_tuner<>, fib_cr_tuner > step_a_type;
//typedef CnC::step_collection< step_b, CnC::step_tuner<>, fib_cr_tuner > step_b_type;
//
//typedef CnC::tag_collection< int, CnC::tag_tuner<>, fib_cr_tuner > tag_a_type;
//typedef CnC::tag_collection< int, CnC::tag_tuner<>, fib_cr_tuner > tag_b_type;


struct cr_step_a_tuner: public CnC::checkpoint_step_tuner< int >
{
	int getNrOfPuts(const int & tag) const;
	int getNrOfPrescribes(const int & tag) const;
	int getNrOfGets(const int & tag) const;
};

struct cr_step_b_tuner: public CnC::checkpoint_step_tuner< int >
{
	int getNrOfPuts(const int & tag) const;
	int getNrOfPrescribes(const int & tag) const;
	int getNrOfGets(const int & tag) const;
};

struct fib_cr_item_tuner: public CnC::checkpoint_item_tuner<int>
{
	int getNrOfgets(const int & tag) const;

};


// The context class
struct pres_context : public CnC::resilientContext< pres_context >
{
	//typedef CnC::resilient_item_collection_strategy_naive< CnC::resilient_item_collection< fib_context, int, fib_type, CnC::hashmap_tuner, fib_cr_item_tuner >, int, fib_type > t_;

    // step collections
    CnC::resilient_step_collection< pres_context, int, step_a, CnC::step_tuner<>, cr_step_a_tuner > m_steps_a;
    CnC::resilient_step_collection< pres_context, int, step_b, CnC::step_tuner<>, cr_step_b_tuner > m_steps_b;

    // Item collections
    CnC::resilient_item_collection< pres_context, int, int, CnC::hashmap_tuner, fib_cr_item_tuner > m_unused;
    // Tag collections
    CnC::resilient_tag_collection< pres_context, int, CnC::tag_tuner<>, cr_step_a_tuner > m_tags_a;
    CnC::resilient_tag_collection< pres_context, int, CnC::tag_tuner<>, cr_step_b_tuner > m_tags_b;


    // The context class constructor
    pres_context()
        : CnC::resilientContext< pres_context >(20, 1),
          m_steps_a( *this ),
          m_steps_b( *this ),
          m_tags_a( *this ),
          m_tags_b( *this ),
          m_unused( *this )
    {
        // Prescriptive relations
        m_tags_a.prescribes( m_steps_a, *this );
        m_tags_b.prescribes( m_steps_b, *this );

//        // Consumer relations
//        m_steps.consumes( m_fibs );
//        // Producer relations
//        m_steps.produces( m_fibs );

        CnC::debug::trace_all(*this, 1);
    }
};

#endif // fib_H_ALREADY_INCLUDED
