/*
 * ItemCheckpoint.h
 *
 *  Created on: Apr 28, 2015
 *      Author: blackline
 */

#ifndef ITEMCHECKPOINT_H_
#define ITEMCHECKPOINT_H_

template< class Key, class Item, class Tuner >
class ItemCheckpoint {
public:
	ItemCheckpoint();
	virtual ~ItemCheckpoint();

	void put(const Key & tag, const Item & item);

private:
	typedef std::tr1::unordered_map< Key, Item * > itemMap;
	typedef std::tr1::unordered_map< int, Key > keyMap;

    typedef typename Tuner::template item_allocator< Item >::type item_allocator_type;

	itemMap m_item_map;
	keyMap m_key_map;
	mutable item_allocator_type m_allocator;

	Item * create( const Item & org ) const;
	void uncreate( Item * item ) const;
};

template< class Key, class Item, class Tuner >
ItemCheckpoint< Key, Item, Tuner >::ItemCheckpoint(): m_item_map(), m_key_map(), m_allocator() {};

template< class Key, class Item, class Tuner >
ItemCheckpoint< Key, Item, Tuner >::~ItemCheckpoint() {};

template< class Key, class Item, class Tuner >
void ItemCheckpoint< Key, Item, Tuner >::put( const Key & tag, const Item & item ) {
	Item * _i = create(item);
	ItemCheckpoint< Key, Item, Tuner >::m_item_map[tag] = _i;
}

template< class Key, class Item, class Tuner >
Item * ItemCheckpoint< Key, Item, Tuner >::create( const Item & org ) const
{
    Item * _item = m_allocator.allocate( 1 );
    m_allocator.construct( _item, org );
    return _item;
}

template< class Key, class Item, class Tuner >
void ItemCheckpoint< Key, Item, Tuner >::uncreate( Item * item ) const
{
    if( item ) {
        m_allocator.destroy( item );
        m_allocator.deallocate( item, 1 );
    }
}

#endif /* ITEMCHECKPOINT_H_ */
