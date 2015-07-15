/*
 * ItemCheckpoint.h
 *
 *  Created on: Apr 28, 2015
 *      Author: blackline
 */

#ifndef ITEMCHECKPOINT_H_
#define ITEMCHECKPOINT_H_

#include "ItemCheckpoint_i.h"
#include "tbb/concurrent_hash_map.h"
#include <tr1/unordered_map>
#include <cnc/internal/typed_tag.h>


namespace CnC {

template< typename ResilientItemCollection, typename Key, typename Item >
class ItemCheckpoint: public ItemCheckpoint_i {
public:
	ItemCheckpoint(ResilientItemCollection & owner, int col_id);
	virtual ~ItemCheckpoint();

	void * put(const Key & tag, const Item & item);
	void * getKeyId(const Key & k);

	void add_checkpoint_locally();

	void decrement_get_count(void * tag);

	int getId();

	void send_get(CnC::serializer * ser, void * tag);

	void print();

private:
	typedef std::pair< Item*, int > checkpoint_item_type;
	typedef tbb::concurrent_hash_map< Key, checkpoint_item_type > itemMap;
	typedef tbb::scalable_allocator< Item > item_allocator_type;
	typedef tbb::concurrent_hash_map< Key, Key* > keyMap;
	typedef tbb::scalable_allocator< Key > key_allocator_type;

	itemMap m_item_map;
	mutable item_allocator_type m_item_allocator;

	keyMap m_key_map;
	mutable key_allocator_type m_key_allocator;

	int m_coll_id;
	ResilientItemCollection & m_owner;

	Item * create_item( const Item & org ) const;
	void uncreate_item( Item * item ) const;

	Key * create_key( const Key & org ) const;
	void uncreate_key( Key * key ) const;

	void cleanup();
};

template< typename ResilientItemCollection, typename Key, typename Item >
ItemCheckpoint< ResilientItemCollection, Key, Item >::ItemCheckpoint(
		ResilientItemCollection & owner,
		int col_id):
	m_item_map(), m_item_allocator(), m_coll_id(col_id), m_owner(owner), m_key_map(), m_key_allocator() {};

template< typename ResilientItemCollection, typename Key, typename Item >
ItemCheckpoint< ResilientItemCollection, Key, Item >::~ItemCheckpoint() { cleanup(); };

//Make sure to return the address of the item!
template< typename ResilientItemCollection, typename Key, typename Item >
void * ItemCheckpoint< ResilientItemCollection, Key, Item >::put( const Key & tag, const Item & item )
{
	typename itemMap::accessor _accr;
	bool inserted = m_item_map.insert(_accr, tag);
	if (inserted) {
		Item * _i = create_item(item);
		int get_count = m_owner.getCTuner().getNrOfgets(tag);
		_accr->second = checkpoint_item_type(_i, get_count);
		return static_cast<void*>(_i);
	} else {
		Item * tmp = (_accr->second).first;
		return static_cast<void*>(tmp);
	}
}

template< typename ResilientItemCollection, typename Key, typename Item >
void * ItemCheckpoint< ResilientItemCollection, Key, Item >::getKeyId(const Key & key)
{
	typename keyMap::accessor _accr;
	bool inserted = m_key_map.insert(_accr, key);
	if (inserted) {
		Key * _k = create_key(key);
		_accr->second = _k;
		return static_cast<void*>(_k);
	} else {
		Key * tmp = _accr->second;
		return static_cast<void*>(tmp);
	}
}

template< typename ResilientItemCollection, typename Key, typename Item >
void ItemCheckpoint< ResilientItemCollection, Key, Item >::decrement_get_count(void * const tag)
{
	Key* t_ = static_cast< Key * >(tag);
	typename itemMap::accessor _i_accr;
	bool i_found = m_item_map.find(_i_accr, * t_);
	if (i_found) {
		if (--((_i_accr->second).second) == 0) {
			typename keyMap::accessor _k_accr;
			bool k_found = m_key_map.find(_k_accr, * t_);
			if (k_found) { //Should not occur
				uncreate_key(_k_accr->second);
				m_key_map.erase(_k_accr);
			}
			uncreate_item( (_i_accr->second).first );
			m_item_map.erase(_i_accr);
		}
	}
}

template< typename ResilientItemCollection, typename Key, typename Item >
int ItemCheckpoint< ResilientItemCollection, Key, Item >::getId()
{
	return m_coll_id;
}

template< typename ResilientItemCollection, typename Key, typename Item >
Item * ItemCheckpoint< ResilientItemCollection, Key, Item >::create_item( const Item & org ) const
{
    Item * _item = m_item_allocator.allocate( 1 );
    m_item_allocator.construct( _item, org );
    return _item;
}

template< typename ResilientItemCollection, typename Key, typename Item >
Key * ItemCheckpoint< ResilientItemCollection, Key, Item >::create_key( const Key & org ) const
{
    Key * _key = m_key_allocator.allocate( 1 );
    m_key_allocator.construct( _key, org );
    return _key;
}

template< typename ResilientItemCollection, typename Key, typename Item >
void ItemCheckpoint< ResilientItemCollection, Key, Item >::add_checkpoint_locally() {
	for( typename itemMap::const_iterator it = m_item_map.begin(); it != m_item_map.end(); ++it) {
		m_owner.restart_put(it->first, *it->second.first);
	}
}

template< typename ResilientItemCollection, typename Key, typename Item >
void ItemCheckpoint< ResilientItemCollection, Key, Item >::print() {
	std::cout << "Printing Item Checkpoint: " << std::endl;
	for( typename itemMap::const_iterator it = m_item_map.begin(); it != m_item_map.end(); ++it) {
		std::cout << "(" << it->first <<", " << *it->second.first << ", " << it->second.second << "), ";
	}
	std::cout << std::endl;
}

template< typename ResilientItemCollection, typename Key, typename Item >
void ItemCheckpoint< ResilientItemCollection, Key, Item >::uncreate_item( Item * item ) const
{
    if( item ) {
        m_item_allocator.destroy( item );
        m_item_allocator.deallocate( item, 1 );
    }
}

template< typename ResilientItemCollection, typename Key, typename Item >
void ItemCheckpoint< ResilientItemCollection, Key, Item >::uncreate_key( Key * key ) const
{
    if( key ) {
        m_key_allocator.destroy( key );
        m_key_allocator.deallocate( key, 1 );
    }
}


template< typename ResilientItemCollection, typename Key, typename Item >
void ItemCheckpoint< ResilientItemCollection, Key, Item >::cleanup()
{
	for( typename itemMap::const_iterator it = m_item_map.begin(); it != m_item_map.end(); ++it) {
		uncreate_item( it->second.first );
	}
	for( typename keyMap::const_iterator it = m_key_map.begin(); it != m_key_map.end(); ++it) {
		uncreate_key( it->second );
	}
}

template< typename ResilientItemCollection, typename Key, typename Item >
void ItemCheckpoint< ResilientItemCollection, Key, Item >::send_get(CnC::serializer * ser, void * tag) {
	m_owner.getStrategy()->sendGet(ser, tag);
}

}

#endif /* ITEMCHECKPOINT_H_ */
