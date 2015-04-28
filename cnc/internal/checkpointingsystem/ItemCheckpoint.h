/*
 * ItemCheckpoint.h
 *
 *  Created on: Apr 28, 2015
 *      Author: blackline
 */

#ifndef ITEMCHECKPOINT_H_
#define ITEMCHECKPOINT_H_

#include <tbb/concurrent_hash_map.h>


template< class Key, class Item >
class ItemCheckpoint {
public:
	ItemCheckpoint();
	virtual ~ItemCheckpoint();

	void * put(const Key & tag, const Item & item);

private:
	typedef std::tr1::unordered_map< Key, Item * > itemMap;

	typedef tbb::scalable_allocator< Item > item_allocator_type;

	itemMap m_item_map;
	mutable item_allocator_type m_allocator;

	Item * create( const Item & org ) const;
	void uncreate( Item * item ) const;
};

template< class Key, class Item >
ItemCheckpoint< Key, Item >::ItemCheckpoint(): m_item_map(), m_allocator() {};

template< class Key, class Item >
ItemCheckpoint< Key, Item >::~ItemCheckpoint() {};

template< class Key, class Item >
void * ItemCheckpoint< Key, Item >::put( const Key & tag, const Item & item ) {
	typename itemMap::iterator it = m_item_map.find(tag);
	if (it == m_item_map.end()) {
		Item * _i = create(item);
		m_item_map[tag] = _i;
		return static_cast<void*>(_i);
	} else {
		Item * tmp = it->second;
		return static_cast<void*>(tmp);
	}
}

template< class Key, class Item >
Item * ItemCheckpoint< Key, Item >::create( const Item & org ) const
{
    Item * _item = m_allocator.allocate( 1 );
    m_allocator.construct( _item, org );
    return _item;
}

template< class Key, class Item >
void ItemCheckpoint< Key, Item >::uncreate( Item * item ) const
{
    if( item ) {
        m_allocator.destroy( item );
        m_allocator.deallocate( item, 1 );
    }
}

#endif /* ITEMCHECKPOINT_H_ */
