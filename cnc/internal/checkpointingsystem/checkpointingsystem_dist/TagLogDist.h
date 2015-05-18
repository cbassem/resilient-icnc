/*
 * TagLog.h
 *
 *  Created on: Mar 28, 2015
 *      Author: root
 */

#ifndef TAGLOG_DIST_H_
#define TAGLOG_DIST_H_

#include "tbb/concurrent_vector.h"
#include <set>
#include <algorithm>
#include <tr1/unordered_map>
#include "tbb/concurrent_unordered_set.h"
#include "../TagCheckpoint_i.h"
#include "../TagLog_i.h"

template< class Tag >
class TagLogDist: public TagLog_i {
	int currentPuts_;
	int currentPrescribes_;
	int currentGets_;
	int totalPuts_;
	int totalPrescribes_;
	int totalGets_;
	int collection_id_;

	bool markedDone_;
	bool forceMarkedDone;

	typedef std::pair< TagCheckpoint_i*, void* > prescribesLog;
	typedef tbb::concurrent_unordered_set< prescribesLog > prescribes_t;
	typedef std::pair< ItemCheckpoint_i*, void* > putLog;
	typedef tbb::concurrent_unordered_set< putLog > items_t;
	typedef std::pair< ItemCheckpoint_i*, void* > getLog;
	typedef tbb::concurrent_unordered_set< getLog > gets_t;


	prescribes_t prescribes_;
	items_t items_;
	gets_t gets_;

	Tag me; // Has to be a copy :/

public:
	TagLogDist(int col_id, Tag & tag);
	virtual ~TagLogDist();

	void processPrescribe( TagCheckpoint_i * tag_cp, void * tagId);
	void processPut( ItemCheckpoint_i * item_cp, void * itemId );
	void processGet( ItemCheckpoint_i * item_cp, void* tag); //FIXME tag by ptr, ref or val?

	void processDone(int totalPuts, int totalPrescribes, int totalGets);

	bool isDone() const;

	void forceDone();

	void decrement_get_counts();

	void send_unprocessed_prescribes();

	void send_unprocessed_puts();

	void send_unprocessed_gets();

	void set_collection_id(int colid ) {collection_id_ = colid;};

	void set_my_tag(Tag t) {me = t;};

	CnC::serializer * add_info(CnC::serializer * ser);

};

template< class Tag >
TagLogDist< Tag >::TagLogDist(int col_id, Tag & tag) :
		currentPuts_(0),
		currentPrescribes_(0),
		currentGets_(0),
		totalPuts_(-1),
		totalPrescribes_(-1),
		totalGets_(-1),
		markedDone_(false),
		forceMarkedDone(false),
		prescribes_(),
		items_(),
		gets_(),
		collection_id_(col_id),
		me(tag)
		{}

template< class Tag >
TagLogDist< Tag >::~TagLogDist() {}

template< class Tag >
void TagLogDist< Tag >::processPut(ItemCheckpoint_i * item_cp, void * itemId) {
	std::pair<items_t::iterator, bool> res = items_.insert(putLog(item_cp, itemId));
	if (res.second) {
		currentPuts_++;
	}
}

template< class Tag >
void TagLogDist< Tag >::processPrescribe(TagCheckpoint_i * tag_cp, void * tagId) {
	std::pair<prescribes_t::iterator, bool> res = prescribes_.insert(prescribesLog(tag_cp, tagId));
	if (res.second) {
		currentPrescribes_++;
	}
}

template< class Tag >
void TagLogDist< Tag >::processGet( ItemCheckpoint_i * item_cp, void* tag) {
	std::pair<gets_t::iterator, bool> res = gets_.insert(getLog(item_cp, tag));
	if(res.second) {
		currentGets_++;
	}
}

template< class Tag >
void TagLogDist< Tag >::processDone(int totalPuts, int totalPrescribes, int totalGets) {
	markedDone_ = true;
	totalPrescribes_ = totalPrescribes;
	totalPuts_ = totalPuts;
	totalGets_ = totalGets;
}

template< class Tag >
void TagLogDist< Tag >::forceDone() {
	forceMarkedDone = true;
}

template< class Tag >
bool TagLogDist< Tag >::isDone() const {
	return (markedDone_
			&& currentPrescribes_ == totalPrescribes_
			&& currentPuts_ == totalPuts_
			&& currentGets_ == totalGets_)
			|| forceMarkedDone;
}


template< class Tag >
void TagLogDist< Tag >::decrement_get_counts() {
	for (typename gets_t::const_iterator it = gets_.begin(); it != gets_.end(); ++it) {
		(it->first)->decrement_get_count(it->second);
	}
}


template< class Tag >
void TagLogDist< Tag >::send_unprocessed_prescribes() {
	for (typename prescribes_t::const_iterator it = prescribes_.begin(); it != prescribes_.end(); ++it) {
		(it->first)->sendIfNotDone(this, it->second);
	}
}


template< class Tag >
void TagLogDist< Tag >::send_unprocessed_puts() {
	for (typename items_t::const_iterator it = items_.begin(); it != items_.end(); ++it) {
		//(it->first)->sendIfNotDone(get_tag_info_serializer(), it->second);
	}
}


template< class Tag >
void TagLogDist< Tag >::send_unprocessed_gets() {
	for (typename gets_t::const_iterator it = gets_.begin(); it != gets_.end(); ++it) {
		//(it->first)->sendIfNotDone(get_tag_info_serializer(), it->second);
	}
}


template< class Tag >
CnC::serializer * TagLogDist< Tag >::add_info(CnC::serializer * ser) {
    (*ser) & collection_id_ & me;
    return ser;
}

#endif /* TAGLOG_DIST_H_ */
