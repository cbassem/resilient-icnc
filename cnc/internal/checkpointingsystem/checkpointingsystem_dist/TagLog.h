/*
 * TagLog.h
 *
 *  Created on: Mar 28, 2015
 *      Author: root
 */

#ifndef TAGLOG_H_
#define TAGLOG_H_

#include "tbb/concurrent_vector.h"
#include <set>
#include <algorithm>
#include <tr1/unordered_map>
#include "tbb/concurrent_unordered_set.h"


class TagLog {
	int currentPuts_;
	int currentPrescribes_;
	int currentGets_;
	int totalPuts_;
	int totalPrescribes_;
	int totalGets_;

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
	//tbb::concurrent_vector< getLog > gets_;
	gets_t gets_;

public:
	TagLog();
	virtual ~TagLog();

	void processPrescribe( TagCheckpoint_i * tag_cp, void * tagId);
	void processPut( ItemCheckpoint_i * item_cp, void * itemId );
	void processGet( ItemCheckpoint_i * item_cp, void* tag); //FIXME tag by ptr, ref or val?

	void processDone(int totalPuts, int totalPrescribes, int totalGets);

	bool isDone() const;

	void forceDone();

	void decrement_get_counts();

};

TagLog::TagLog() :
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
		gets_()
		{}

TagLog::~TagLog() {}

void TagLog::processPut(ItemCheckpoint_i * item_cp, void * itemId) {
	std::pair<items_t::iterator, bool> res = items_.insert(putLog(item_cp, itemId));
	if (res.second) {
		currentPuts_++;
	}
}

void TagLog::processPrescribe(TagCheckpoint_i * tag_cp, void * tagId) {
	std::pair<prescribes_t::iterator, bool> res = prescribes_.insert(prescribesLog(tag_cp, tagId));
	if (res.second) {
		currentPrescribes_++;
	}
}


void TagLog::processGet( ItemCheckpoint_i * item_cp, void* tag) {
	std::pair<gets_t::iterator, bool> res = gets_.insert(getLog(item_cp, tag));
	if(res.second) {
		currentGets_++;
	}
}

void TagLog::processDone(int totalPuts, int totalPrescribes, int totalGets) {
	markedDone_ = true;
	totalPrescribes_ = totalPrescribes;
	totalPuts_ = totalPuts;
	totalGets_ = totalGets;
}

void TagLog::forceDone() {
	forceMarkedDone = true;
}
bool TagLog::isDone() const {
	return (markedDone_
			&& currentPrescribes_ == totalPrescribes_
			&& currentPuts_ == totalPuts_
			&& currentGets_ == totalGets_)
			|| forceMarkedDone;
}

void TagLog::decrement_get_counts() {
	for (typename gets_t::const_iterator it = gets_.begin(); it != gets_.end(); ++it) {
		(it->first)->decrement_get_count(it->second);
	}
}

#endif /* TAGLOG_H_ */