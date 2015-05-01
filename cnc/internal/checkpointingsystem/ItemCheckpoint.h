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


template< class Key, class Item >
class ItemCheckpoint: public ItemCheckpoint_i {
public:
	ItemCheckpoint(int col_id);
	virtual ~ItemCheckpoint();

	void * put(const Key & tag, const Item & item);

	int getId();

private:
	typedef std::tr1::unordered_map< Key, Item * > itemMap;

	typedef tbb::scalable_allocator< Item > item_allocator_type;

	itemMap m_item_map;
	mutable item_allocator_type m_allocator;

	int m_coll_id;

	Item * create( const Item & org ) const;
	void uncreate( Item * item ) const;

	void cleanup();
};

template< class Key, class Item >
ItemCheckpoint< Key, Item >::ItemCheckpoint(int col_id): m_item_map(), m_allocator(), m_coll_id(col_id) {};

template< class Key, class Item >
ItemCheckpoint< Key, Item >::~ItemCheckpoint() { cleanup(); };

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
int ItemCheckpoint< Key, Item >::getId()
{
	return m_coll_id;
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

template< class Key, class Item >
void ItemCheckpoint< Key, Item >::cleanup()
{
	for( typename itemMap::const_iterator it = m_item_map.begin(); it != m_item_map.end(); ++it) {
		uncreate( it->second );
	}
}

#endif /* ITEMCHECKPOINT_H_ */
