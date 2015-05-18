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
#include "tbb/concurrent_vector.h"
#include "TagLog_i.h"

namespace CnC {

template< typename ResilientTagCollection, typename Tag >
class TagCheckpoint: public TagCheckpoint_i {
public:
	TagCheckpoint(ResilientTagCollection & owner, int col_id);
	virtual ~TagCheckpoint();

	void * put(const Tag & tag);

	int getId();

	void prescribeStepCheckpoint( StepCheckpoint_i * cp);

	void calculate_checkpoint();

	void add_checkpoint_locally();

	void print();

	void sendIfNotDone(TagLog_i * t, void * tag) {};


private:
	typedef tbb::concurrent_hash_map< Tag, Tag * > tagMap;
    typedef tbb::scalable_allocator< Tag > tag_allocator_type;
    typedef tbb::concurrent_vector< StepCheckpoint< Tag > * > step_checkpoints_type;

    tagMap m_tag_map;
	mutable tag_allocator_type m_allocator;
	step_checkpoints_type m_step_checkpoints;

	int m_col_id;
	ResilientTagCollection & m_owner;

	typedef tbb::spin_mutex mutex_t;
	mutex_t m_mutex;

	Tag * create( const Tag & org ) const;
	void uncreate( Tag * item ) const;

	void cleanup();
};

template< typename ResilientTagCollection, typename Tag >
TagCheckpoint< ResilientTagCollection, Tag >::TagCheckpoint(ResilientTagCollection & owner, int col_id):
m_tag_map(), m_allocator(), m_col_id(col_id), m_step_checkpoints(), m_owner(owner),m_mutex() {};

template< typename ResilientTagCollection, typename Tag >
TagCheckpoint< ResilientTagCollection, Tag >::~TagCheckpoint() { cleanup(); };

template< typename ResilientTagCollection, typename Tag >
void * TagCheckpoint< ResilientTagCollection, Tag >::put( const Tag & tag ) {
	//mutex_t::scoped_lock _l( m_mutex );
	typename tagMap::accessor _accr;
	bool inserted = m_tag_map.insert(_accr, tag);
	if (inserted) {
		Tag * _i = create(tag);
		_accr->second = _i;
		return static_cast<void*>(_i);
	} else {
		Tag * tmp = _accr->second;
		return static_cast<void*>(tmp);
	}
}

template< typename ResilientTagCollection, typename Tag >
int TagCheckpoint< ResilientTagCollection, Tag >::getId()
{
	return m_col_id;
}

template< typename ResilientTagCollection, typename Tag >
void TagCheckpoint< ResilientTagCollection, Tag >::prescribeStepCheckpoint( StepCheckpoint_i * cp)
{
	m_step_checkpoints.push_back(static_cast< StepCheckpoint<Tag> *>(cp));
}

template< typename ResilientTagCollection, typename Tag >
void TagCheckpoint< ResilientTagCollection, Tag >::add_checkpoint_locally() {
	//mutex_t::scoped_lock _l( m_mutex );
	for( typename tagMap::const_iterator it = m_tag_map.begin(); it != m_tag_map.end(); ++it) {
		m_owner.restart_put(it->first);
	}
}

template< typename ResilientTagCollection, typename Tag >
void TagCheckpoint< ResilientTagCollection, Tag >::calculate_checkpoint()
{
	//mutex_t::scoped_lock _l( m_mutex );

	for( typename step_checkpoints_type::const_iterator it = m_step_checkpoints.begin(); it != m_step_checkpoints.end(); ++it) {
		std::vector<Tag> to_remove;
		typename tagMap::const_iterator itt = m_tag_map.begin();
		while ( itt != m_tag_map.end() )
		{
			if ((*it)->isDone(itt->first)) {
				to_remove.push_back(itt->first);
				//m_tag_map.erase(itt++);
				++itt;
			} else {
				++itt;
			}
		}
		for (typename std::vector<Tag>::iterator ittt = to_remove.begin(); ittt != to_remove.end(); ++ittt) {
			typename tagMap::accessor _accr;
			bool found = m_tag_map.find(_accr, *ittt);
			if (found) {
				uncreate(_accr->second);
				m_tag_map.erase(_accr);
			}
		}
	}

}

template< typename ResilientTagCollection, typename Tag >
void TagCheckpoint< ResilientTagCollection, Tag >::print()
{
	std::cout << "Printing tag checkpoint [" << m_col_id << "]" << std::endl;
	for(typename tagMap::const_iterator it = m_tag_map.begin(); it != m_tag_map.end(); ++it)
	{
		std::cout << it->first << ", ";
	}
	std::cout << std::endl;
}


template< typename ResilientTagCollection, typename Tag >
Tag * TagCheckpoint< ResilientTagCollection, Tag >::create( const Tag & org ) const
{
    Tag * _tag = m_allocator.allocate( 1 );
    m_allocator.construct( _tag, org );
    return _tag;
}

template< typename ResilientTagCollection, typename Tag >
void TagCheckpoint< ResilientTagCollection, Tag >::uncreate( Tag * tag ) const
{
    if( tag ) {
        m_allocator.destroy( tag );
        m_allocator.deallocate( tag, 1 );
    }
}

template< typename ResilientTagCollection, typename Tag >
void TagCheckpoint< ResilientTagCollection, Tag >::cleanup()
{
	for( typename tagMap::const_iterator it = m_tag_map.begin(); it != m_tag_map.end(); ++it) {
		uncreate( it->second );
	}
}


}



#endif /* TAGCHECKPOINT_H_ */
