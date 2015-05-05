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
	void * getKeyId(const Key & k);

	void add_checkpoint_locally();

	void decrement_get_count(void * tag);

	int getId();

	void print();

private:
	typedef std::pair< Item*, int > checkpoint_item_type;
	typedef std::tr1::unordered_map< Key, checkpoint_item_type > itemMap;
	typedef tbb::scalable_allocator< Item > item_allocator_type;
	typedef std::tr1::unordered_map< Key, Key* > keyMap;
	typedef tbb::scalable_allocator< Key > key_allocator_type;

	itemMap m_item_map;
	mutable item_allocator_type m_item_allocator;

	keyMap m_key_map;
	mutable key_allocator_type m_key_allocator;

	int m_coll_id;
	resilient_item_collection< Derived, Key, Item, Tuner, CheckpointTuner > & m_owner;

	Item * create_item( const Item & org ) const;
	void uncreate_item( Item * item ) const;

	Key * create_key( const Key & org ) const;
	void uncreate_key( Key * key ) const;

	void cleanup();
};

template< typename Derived, typename Key, typename Item, typename Tuner, typename CheckpointTuner >
ItemCheckpoint< Derived, Key, Item, Tuner, CheckpointTuner >::ItemCheckpoint(
		resilient_item_collection< Derived, Key, Item, Tuner, CheckpointTuner > & owner,
		int col_id):
	m_item_map(), m_item_allocator(), m_coll_id(col_id), m_owner(owner), m_key_map(), m_key_allocator() {};

template< typename Derived, typename Key, typename Item, typename Tuner, typename CheckpointTuner >
ItemCheckpoint< Derived, Key, Item, Tuner, CheckpointTuner >::~ItemCheckpoint() { cleanup(); };

//Make sure to return the address of the item!
template< typename Derived, typename Key, typename Item, typename Tuner, typename CheckpointTuner >
void * ItemCheckpoint< Derived, Key, Item, Tuner, CheckpointTuner >::put( const Key & tag, const Item & item )
{
	typename itemMap::iterator it = m_item_map.find(tag);
	if (it == m_item_map.end()) {
		Item * _i = create_item(item);
		int get_count = m_owner.m_ctuner.getNrOfgets(tag);
		m_item_map[tag] = checkpoint_item_type(_i, get_count);
		return static_cast<void*>(_i);
	} else {
		Item * tmp = it->second.first;
		return static_cast<void*>(tmp);
	}
}

template< typename Derived, typename Key, typename Item, typename Tuner, typename CheckpointTuner >
void * ItemCheckpoint< Derived, Key, Item, Tuner, CheckpointTuner >::getKeyId(const Key & key)
{
	typename keyMap::iterator it = m_key_map.find(key);
	if (it == m_key_map.end()) {
		Key * _k = create_key(key);
		m_key_map[key] = _k;
		return static_cast<void*>(_k);
	} else {
		Key * tmp = it->second;
		return static_cast<void*>(tmp);
	}
}

template< typename Derived, typename Key, typename Item, typename Tuner, typename CheckpointTuner >
void ItemCheckpoint< Derived, Key, Item, Tuner, CheckpointTuner >::decrement_get_count(void * const tag)
{
	Key* t_ = static_cast< Key * >(tag);
	typename itemMap::iterator it = m_item_map.find(* t_);
	if (it != m_item_map.end()) {
		if (--it->second.second == 0) {
			typename keyMap::iterator itt = m_key_map.find(* t_);
			if (itt != m_key_map.end()) { //Should not occur
				uncreate_key(itt->second);
				m_key_map.erase(itt);

			}
			uncreate_item( it->second.first );
			m_item_map.erase(it);
		}
	}
}

template< typename Derived, typename Key, typename Item, typename Tuner, typename CheckpointTuner >
int ItemCheckpoint< Derived, Key, Item, Tuner, CheckpointTuner >::getId()
{
	return m_coll_id;
}

template< typename Derived, typename Key, typename Item, typename Tuner, typename CheckpointTuner >
Item * ItemCheckpoint< Derived, Key, Item, Tuner, CheckpointTuner >::create_item( const Item & org ) const
{
    Item * _item = m_item_allocator.allocate( 1 );
    m_item_allocator.construct( _item, org );
    return _item;
}

template< typename Derived, typename Key, typename Item, typename Tuner, typename CheckpointTuner >
Key * ItemCheckpoint< Derived, Key, Item, Tuner, CheckpointTuner >::create_key( const Key & org ) const
{
    Key * _key = m_key_allocator.allocate( 1 );
    m_key_allocator.construct( _key, org );
    return _key;
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
void ItemCheckpoint< Derived, Key, Item, Tuner, CheckpointTuner >::uncreate_item( Item * item ) const
{
    if( item ) {
        m_item_allocator.destroy( item );
        m_item_allocator.deallocate( item, 1 );
    }
}

template< typename Derived, typename Key, typename Item, typename Tuner, typename CheckpointTuner >
void ItemCheckpoint< Derived, Key, Item, Tuner, CheckpointTuner >::uncreate_key( Key * key ) const
{
    if( key ) {
        m_key_allocator.destroy( key );
        m_key_allocator.deallocate( key, 1 );
    }
}


template< typename Derived, typename Key, typename Item, typename Tuner, typename CheckpointTuner >
void ItemCheckpoint< Derived, Key, Item, Tuner, CheckpointTuner >::cleanup()
{
	for( typename itemMap::const_iterator it = m_item_map.begin(); it != m_item_map.end(); ++it) {
		uncreate_item( it->second.first );
	}
	for( typename keyMap::const_iterator it = m_key_map.begin(); it != m_key_map.end(); ++it) {
		uncreate_key( it->second );
	}
}

}

#endif /* ITEMCHECKPOINT_H_ */
