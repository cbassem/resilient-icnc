/*
 * ItemCheckpoint.h
 *
 *  Created on: Apr 28, 2015
 *      Author: blackline
 */

#ifndef TAGCHECKPOINT_DIST_H_
#define TAGCHECKPOINT_DIST_H_

#include "../StepCheckpoint_i.h"
#include <tr1/unordered_map>
#include "tbb/concurrent_vector.h"
#include "StepCheckpointDist.h"
#include "../TagCheckpoint_i.h"

namespace CnC {

template< class ResilientTagCollection, class Tag >
class resilient_tag_collection_strategy_dist;

template< typename ResilientTagCollection, typename Tag >
class TagCheckpointDist: public TagCheckpoint_i {
public:
	TagCheckpointDist(ResilientTagCollection & owner, int col_id, resilient_tag_collection_strategy_dist< ResilientTagCollection, Tag > & strategy);
	virtual ~TagCheckpointDist();

	void * put(const Tag & tag);

	int getId();

	void prescribeStepCheckpoint( StepCheckpoint_i * cp);

	void calculate_checkpoint();

	void add_checkpoint_locally();

	void print();

	void sendIfNotDone(TagLog_i* f, void * tag);

private:
	typedef tbb::concurrent_hash_map< Tag, Tag * > tagMap;
    typedef tbb::scalable_allocator< Tag > tag_allocator_type;
    typedef tbb::concurrent_vector< StepCheckpointDist< Tag > * > step_checkpoints_type;

    tagMap m_tag_map;
	mutable tag_allocator_type m_allocator;
	step_checkpoints_type m_step_checkpoints;

	int m_col_id;
	ResilientTagCollection & m_owner;
	resilient_tag_collection_strategy_dist< ResilientTagCollection, Tag > & m_strategy;

	typedef tbb::spin_mutex mutex_t;
	mutex_t m_mutex;

	Tag * create( const Tag & org ) const;
	void uncreate( Tag * item ) const;

	void cleanup();
};

template< typename ResilientTagCollection, typename Tag >
TagCheckpointDist< ResilientTagCollection, Tag >::TagCheckpointDist(ResilientTagCollection & owner, int col_id, resilient_tag_collection_strategy_dist< ResilientTagCollection, Tag > & strategy):
m_tag_map(), m_allocator(), m_col_id(col_id), m_step_checkpoints(), m_owner(owner),m_mutex(), m_strategy(strategy) {};

template< typename ResilientTagCollection, typename Tag >
TagCheckpointDist< ResilientTagCollection, Tag >::~TagCheckpointDist() { cleanup(); };

template< typename ResilientTagCollection, typename Tag >
void * TagCheckpointDist< ResilientTagCollection, Tag >::put( const Tag & tag ) {
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
int TagCheckpointDist< ResilientTagCollection, Tag >::getId()
{
	return m_col_id;
}

template< typename ResilientTagCollection, typename Tag >
void TagCheckpointDist< ResilientTagCollection, Tag >::prescribeStepCheckpoint( StepCheckpoint_i * cp)
{
	m_step_checkpoints.push_back(static_cast<StepCheckpointDist<Tag> *>(cp));
}

template< typename ResilientTagCollection, typename Tag >
void TagCheckpointDist< ResilientTagCollection, Tag >::add_checkpoint_locally() {
	//mutex_t::scoped_lock _l( m_mutex );
	for( typename tagMap::const_iterator it = m_tag_map.begin(); it != m_tag_map.end(); ++it) {
		m_owner.restart_put(it->first);
	}
}

template< typename ResilientTagCollection, typename Tag >
void TagCheckpointDist< ResilientTagCollection, Tag >::calculate_checkpoint()
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
void TagCheckpointDist< ResilientTagCollection, Tag >::print()
{
	std::cout << "Printing tag checkpoint [" << m_col_id << "]" << std::endl;
	for(typename tagMap::const_iterator it = m_tag_map.begin(); it != m_tag_map.end(); ++it)
	{
		std::cout << it->first << ", ";
	}
	std::cout << std::endl;
}


template< typename ResilientTagCollection, typename Tag >
Tag * TagCheckpointDist< ResilientTagCollection, Tag >::create( const Tag & org ) const
{
    Tag * _tag = m_allocator.allocate( 1 );
    m_allocator.construct( _tag, org );
    return _tag;
}

template< typename ResilientTagCollection, typename Tag >
void TagCheckpointDist< ResilientTagCollection, Tag >::uncreate( Tag * tag ) const
{
    if( tag ) {
        m_allocator.destroy( tag );
        m_allocator.deallocate( tag, 1 );
    }
}

template< typename ResilientTagCollection, typename Tag >
void TagCheckpointDist< ResilientTagCollection, Tag >::cleanup()
{
	for( typename tagMap::const_iterator it = m_tag_map.begin(); it != m_tag_map.end(); ++it) {
		uncreate( it->second );
	}
}

template< typename ResilientTagCollection, typename Tag >
void TagCheckpointDist< ResilientTagCollection, Tag >::sendIfNotDone(TagLog_i * f, void * tag)
{
	Tag * t_ = static_cast< Tag * >(tag);
	typename tagMap::accessor _accr;
	bool found = m_tag_map.find(_accr, *t_);
	if (found) {
		m_strategy.sendPrescribe(f, *t_);
	}
}


}



#endif /* TAGCHECKPOINT_DIST_H_ */
