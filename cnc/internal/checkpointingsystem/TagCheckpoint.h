/*
 * ItemCheckpoint.h
 *
 *  Created on: Apr 28, 2015
 *      Author: blackline
 */

#ifndef TAGCHECKPOINT_H_
#define TAGCHECKPOINT_H_

#include "TagCheckpoint_i.h"
#include "StepCheckpoint_i.h"
#include "StepCheckpoint.h"
#include <tr1/unordered_map>

namespace CnC {

template< typename Derived, typename Tag, typename Tuner, typename CheckpointTuner > class resilient_tag_collection;

template< typename Derived, typename Tag, typename Tuner, typename CheckpointTuner >
class TagCheckpoint: public TagCheckpoint_i {
public:
	TagCheckpoint(CnC::resilient_tag_collection< Derived, Tag, Tuner, CheckpointTuner > & owner, int col_id);
	virtual ~TagCheckpoint();

	void * put(const Tag & tag);

	int getId();

	void prescribeStepCheckpoint( StepCheckpoint<Tag> * cp);

	void calculate_checkpoint();

	void add_checkpoint_locally();

	void print();

private:
	typedef std::tr1::unordered_map< Tag, Tag * > tagMap;
    typedef tbb::scalable_allocator< Tag > tag_allocator_type;
    typedef std::vector< StepCheckpoint< Tag > * > step_checkpoints_type;

    tagMap m_tag_map;
	mutable tag_allocator_type m_allocator;
	step_checkpoints_type m_step_checkpoints;

	int m_col_id;
	CnC::resilient_tag_collection< Derived, Tag, Tuner, CheckpointTuner > & m_owner;

	Tag * create( const Tag & org ) const;
	void uncreate( Tag * item ) const;

	void cleanup();
};

template< typename Derived, typename Tag, typename Tuner, typename CheckpointTuner >
TagCheckpoint< Derived, Tag, Tuner, CheckpointTuner >::TagCheckpoint(CnC::resilient_tag_collection< Derived, Tag, Tuner, CheckpointTuner > & owner, int col_id):
m_tag_map(), m_allocator(), m_col_id(col_id), m_step_checkpoints(), m_owner(owner) {};

template< typename Derived, typename Tag, typename Tuner, typename CheckpointTuner >
TagCheckpoint< Derived, Tag, Tuner, CheckpointTuner >::~TagCheckpoint() { cleanup(); };

template< typename Derived, typename Tag, typename Tuner, typename CheckpointTuner >
void * TagCheckpoint< Derived, Tag, Tuner, CheckpointTuner >::put( const Tag & tag ) {
	typename tagMap::iterator it = m_tag_map.find(tag);
	if (it == m_tag_map.end()) {
		Tag * _i = create(tag);
		m_tag_map[tag] = _i;
		return static_cast<void*>(_i);
	} else {
		Tag * tmp = it->second;
		return static_cast<void*>(tmp);
	}
}

template< typename Derived, typename Tag, typename Tuner, typename CheckpointTuner >
int TagCheckpoint< Derived, Tag, Tuner, CheckpointTuner >::getId()
{
	return m_col_id;
}

template< typename Derived, typename Tag, typename Tuner, typename CheckpointTuner >
void TagCheckpoint< Derived, Tag, Tuner, CheckpointTuner >::prescribeStepCheckpoint( StepCheckpoint<Tag> * cp)
{
	m_step_checkpoints.push_back(cp);
}

template< typename Derived, typename Tag, typename Tuner, typename CheckpointTuner >
void TagCheckpoint< Derived, Tag, Tuner, CheckpointTuner >::add_checkpoint_locally() {
	for( typename tagMap::const_iterator it = m_tag_map.begin(); it != m_tag_map.end(); ++it) {
		m_owner.restart_put(it->first);
	}
}

template< typename Derived, typename Tag, typename Tuner, typename CheckpointTuner >
void TagCheckpoint< Derived, Tag, Tuner, CheckpointTuner >::calculate_checkpoint()
{
	for( typename step_checkpoints_type::const_iterator it = m_step_checkpoints.begin(); it != m_step_checkpoints.end(); ++it) {
		typename tagMap::const_iterator itt = m_tag_map.begin();
		while ( itt != m_tag_map.end() )
		{
			if ((*it)->isDone(itt->first)) {
				m_tag_map.erase(itt++);
			} else {
				++itt;
			}
		}
	}

}

template< typename Derived, typename Tag, typename Tuner, typename CheckpointTuner >
void TagCheckpoint< Derived, Tag, Tuner, CheckpointTuner >::print()
{
	std::cout << "Printing tag checkpoint [" << m_col_id << "]" << std::endl;
	for(typename tagMap::const_iterator it = m_tag_map.begin(); it != m_tag_map.end(); ++it)
	{
		std::cout << it->first << ", ";
	}
	std::cout << std::endl;
}


template< typename Derived, typename Tag, typename Tuner, typename CheckpointTuner >
Tag * TagCheckpoint< Derived, Tag, Tuner, CheckpointTuner >::create( const Tag & org ) const
{
    Tag * _tag = m_allocator.allocate( 1 );
    m_allocator.construct( _tag, org );
    return _tag;
}

template< typename Derived, typename Tag, typename Tuner, typename CheckpointTuner >
void TagCheckpoint< Derived, Tag, Tuner, CheckpointTuner >::uncreate( Tag * tag ) const
{
    if( tag ) {
        m_allocator.destroy( tag );
        m_allocator.deallocate( tag, 1 );
    }
}

template< typename Derived, typename Tag, typename Tuner, typename CheckpointTuner >
void TagCheckpoint< Derived, Tag, Tuner, CheckpointTuner >::cleanup()
{
	for( typename tagMap::const_iterator it = m_tag_map.begin(); it != m_tag_map.end(); ++it) {
		uncreate( it->second );
	}
}

}



#endif /* TAGCHECKPOINT_H_ */