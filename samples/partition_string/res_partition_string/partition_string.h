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
#ifndef partStr_H_ALREADY_INCLUDED
#define partStr_H_ALREADY_INCLUDED

#include <cnc/cnc.h>
#include <cnc/debug.h>

// Forward declaration of the context class (also known as graph)
struct partStr_context;

// The step classes

struct CreateSpan
{
    int execute( const int & t, partStr_context & c ) const;
};

struct ProcessSpan
{
    int execute( const int & t, partStr_context & c ) const;
};

struct create_step_tuner: public CnC::checkpoint_step_tuner< int >
{
	int getNrOfPuts(const int & tag) const;
	int getNrOfPrescribes(const int & tag) const;
	int getNrOfGets(const int & tag) const;
};

struct process_step_tuner: public CnC::checkpoint_step_tuner< int >
{
	int getNrOfPuts(const int & tag) const;
	int getNrOfPrescribes(const int & tag) const;
	int getNrOfGets(const int & tag) const;
};

struct i_item_tuner: public CnC::checkpoint_item_tuner<int>
{
	int getNrOfgets(const int & tag) const;

};

struct r_item_tuner: public CnC::checkpoint_item_tuner<int>
{
	int getNrOfgets(const int & tag) const;

};

struct s_item_tuner: public CnC::checkpoint_item_tuner<int>
{
	int getNrOfgets(const int & tag) const;

};

// The context class
struct partStr_context : public CnC::resilientContext< partStr_context >
{
    // Step collections
    CnC::resilient_step_collection<partStr_context, int, CreateSpan, CnC::step_tuner<>, create_step_tuner > createSpan;
    CnC::resilient_step_collection<partStr_context, int, ProcessSpan, CnC::step_tuner<>, process_step_tuner > processSpan;

    // Item collections
    CnC::resilient_item_collection<partStr_context, int, string, CnC::hashmap_tuner, i_item_tuner > input;
    CnC::resilient_item_collection<partStr_context, int, string, CnC::hashmap_tuner, r_item_tuner > results;
    CnC::resilient_item_collection<partStr_context, int, string, CnC::hashmap_tuner, s_item_tuner > span;

    // Tag collections
    CnC::resilient_tag_collection<partStr_context, int, CnC::tag_tuner<>, create_step_tuner > singletonTag;
    CnC::resilient_tag_collection<partStr_context, int, CnC::tag_tuner<>, process_step_tuner > spanTags;

    // The context class constructor
    partStr_context()
        : CnC::resilientContext< partStr_context >(),
	  // Initialize each step collection
	  createSpan( *this ),
	  processSpan( *this ),
          // Initialize each item collection
          input( *this ),
          results( *this ),
          span( *this ),
          // Initialize each tag collection
          singletonTag( *this ),
          spanTags( *this )
    {
        // Prescriptive relations
        singletonTag.prescribes( createSpan, *this );
        spanTags.prescribes( processSpan, *this );

	createSpan.consumes( input );
	createSpan.controls( spanTags );
	createSpan.produces( span );

	processSpan.consumes( span );
	processSpan.produces( results );

    }
};

#endif // partStr_H_ALREADY_INCLUDED
