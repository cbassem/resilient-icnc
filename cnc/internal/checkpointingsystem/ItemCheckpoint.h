/*
 * ItemCheckpoint.h
 *
 *  Created on: Apr 28, 2015
 *      Author: blackline
 */

#ifndef ITEMCHECKPOINT_H_
#define ITEMCHECKPOINT_H_

#include "ItemCheckpoint_i.h"
#include <tr1/unordered_map>
#include <cnc/internal/typed_tag.h>


namespace CnC {

template< typename Derived, typename Tag, typename Item, typename Tuner, typename CheckpointTuner > class resilient_item_collection;

template< typename Derived, typename Key, typename Item, typename Tuner, typename CheckpointTuner >
class ItemCheckpoint: public ItemCheckpoint_i {
public:
	ItemCheckpoint(resilient_item_collection< Derived, Key, Item, Tuner, CheckpointTuner > & owner, int col_id);
	virtual ~ItemCheckpoint();

	void * put(const Key & tag, const Item & item);

	void add_checkpoint_locally();

	void decrement_get_count(CnC::Internal::tag_base& tag);

	int getId();

	void print();

private:
	typedef std::pair< Item*, int > checkpoint_item_type;

	typedef std::tr1::unordered_map< Key, checkpoint_item_type > itemMap;

	typedef tbb::scalable_allocator< Item > item_allocator_type;

	itemMap m_item_map;
	mutable item_allocator_type m_allocator;

	int m_coll_id;
	resilient_item_collection< Derived, Key, Item, Tuner, CheckpointTuner > & m_owner;

	Item * create( const Item & org ) const;
	void uncreate( Item * item ) const;

	void cleanup();
};

template< typename Derived, typename Key, typename Item, typename Tuner, typename CheckpointTuner >
ItemCheckpoint< Derived, Key, Item, Tuner, CheckpointTuner >::ItemCheckpoint(resilient_item_collection< Derived, Key, Item, Tuner, CheckpointTuner > & owner, int col_id):
	m_item_map(), m_allocator(), m_coll_id(col_id), m_owner(owner) {};

template< typename Derived, typename Key, typename Item, typename Tuner, typename CheckpointTuner >
ItemCheckpoint< Derived, Key, Item, Tuner, CheckpointTuner >::~ItemCheckpoint() { cleanup(); };

//Make sure to return the address of the item!
template< typename Derived, typename Key, typename Item, typename Tuner, typename CheckpointTuner >
void * ItemCheckpoint< Derived, Key, Item, Tuner, CheckpointTuner >::put( const Key & tag, const Item & item ) {
	typename itemMap::iterator it = m_item_map.find(tag);
	if (it == m_item_map.end()) {
		Item * _i = create(item);
		m_item_map[tag] = checkpoint_item_type(_i, 0);
		return static_cast<void*>(_i);
	} else {
		Item * tmp = it->second.first;
		return static_cast<void*>(tmp);
	}
}

template< typename Derived, typename Key, typename Item, typename Tuner, typename CheckpointTuner >
void ItemCheckpoint< Derived, Key, Item, Tuner, CheckpointTuner >::decrement_get_count(CnC::Internal::tag_base& tag)
{
	Internal::typed_tag< Key >& t_ = static_cast< Internal::typed_tag< Key >& >(tag);
	typename itemMap::iterator it = m_item_map.find(t_.getValue());
	if (it != m_item_map.end()) {
		it->second.second--;
	}

}

template< typename Derived, typename Key, typename Item, typename Tuner, typename CheckpointTuner >
int ItemCheckpoint< Derived, Key, Item, Tuner, CheckpointTuner >::getId()
{
	return m_coll_id;
}

template< typename Derived, typename Key, typename Item, typename Tuner, typename CheckpointTuner >
Item * ItemCheckpoint< Derived, Key, Item, Tuner, CheckpointTuner >::create( const Item & org ) const
{
    Item * _item = m_allocator.allocate( 1 );
    m_allocator.construct( _item, org );
    return _item;
}

template< typename Derived, typename Key, typename Item, typename Tuner, typename CheckpointTuner >
void ItemCheckpoint< Derived, Key, Item, Tuner, CheckpointTuner >::add_checkpoint_locally() {
	for( typename itemMap::const_iterator it = m_item_map.begin(); it != m_item_map.end(); ++it) {
		m_owner.restart_put(it->first, *it->second.first);
	}
}

template< typename Derived, typename Key, typename Item, typename Tuner, typename CheckpointTuner >
void ItemCheckpoint< Derived, Key, Item, Tuner, CheckpointTuner >::print() {
	std::cout << "Printing Item Checkpoint: " << std::endl;
	for( typename itemMap::const_iterator it = m_item_map.begin(); it != m_item_map.end(); ++it) {
		std::cout << "(" << it->first <<", " << *it->second.first << "), ";
	}
	std::cout << std::endl;
}

template< typename Derived, typename Key, typename Item, typename Tuner, typename CheckpointTuner >
void ItemCheckpoint< Derived, Key, Item, Tuner, CheckpointTuner >::uncreate( Item * item ) const
{
    if( item ) {
        m_allocator.destroy( item );
        m_allocator.deallocate( item, 1 );
    }
}

template< typename Derived, typename Key, typename Item, typename Tuner, typename CheckpointTuner >
void ItemCheckpoint< Derived, Key, Item, Tuner, CheckpointTuner >::cleanup()
{
	for( typename itemMap::const_iterator it = m_item_map.begin(); it != m_item_map.end(); ++it) {
		uncreate( it->second.first );
	}
}

}

#endif /* ITEMCHECKPOINT_H_ */
