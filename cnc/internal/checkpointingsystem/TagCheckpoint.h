/*
 * ItemCheckpoint.h
 *
 *  Created on: Apr 28, 2015
 *      Author: blackline
 */

#ifndef TAGCHECKPOINT_H_
#define TAGCHECKPOINT_H_

#include "TagCheckpoint_i.h"
#include <tr1/unordered_map>


template< class Tag >
class TagCheckpoint: public TagCheckpoint_i {
public:
	TagCheckpoint();
	virtual ~TagCheckpoint();

	void * put(const Tag & tag);

private:
	typedef std::tr1::unordered_map< Tag, Tag * > tagMap;

    typedef tbb::scalable_allocator< Tag > tag_allocator_type;

    tagMap m_tag_map;
	mutable tag_allocator_type m_allocator;

	Tag * create( const Tag & org ) const;
	void uncreate( Tag * item ) const;

	void cleanup();
};

template< class Tag >
TagCheckpoint< Tag >::TagCheckpoint(): m_tag_map(), m_allocator() {};

template< class Tag >
TagCheckpoint< Tag >::~TagCheckpoint() { cleanup(); };

template< class Tag >
void * TagCheckpoint< Tag >::put( const Tag & tag ) {
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

template< class Tag >
Tag * TagCheckpoint< Tag >::create( const Tag & org ) const
{
    Tag * _tag = m_allocator.allocate( 1 );
    m_allocator.construct( _tag, org );
    return _tag;
}

template< class Tag >
void TagCheckpoint< Tag >::uncreate( Tag * tag ) const
{
    if( tag ) {
        m_allocator.destroy( tag );
        m_allocator.deallocate( tag, 1 );
    }
}

template< class Tag >
void TagCheckpoint< Tag >::cleanup()
{
	for( typename tagMap::const_iterator it = m_tag_map.begin(); it != m_tag_map.end(); ++it) {
		uncreate( it->second );
	}
}

#endif /* TAGCHECKPOINT_H_ */
