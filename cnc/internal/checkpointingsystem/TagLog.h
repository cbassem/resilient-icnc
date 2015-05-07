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

	typedef tbb::concurrent_unordered_set< void * > prescribes_t;
	typedef tbb::concurrent_unordered_set< void * > items_t;
	typedef std::pair< ItemCheckpoint_i*, void* > getLog; //FIXME tag by ptr, ref or val? No choice has to be a ptr, object slicing problem//


	prescribes_t prescribes_;
	items_t items_;
	tbb::concurrent_vector< getLog > gets_;

	struct Compare
	{
	  Compare(ItemCheckpoint_i* val1, void * val2) : val1_(val1), val2_(val2) {}
	  bool operator()(const getLog& elem) const {
	    return val1_ == elem.first && val2_ == elem.second;
	  }
	  private:
	  ItemCheckpoint_i* val1_;
	  void*  val2_;
	};

public:
	TagLog();
	virtual ~TagLog();

	void processPut(void * itemId );
	void processPrescribe(void * tagId);
	void processGet( ItemCheckpoint_i * item_cp, void* tag); //FIXME tag by ptr, ref or val?

	void processDone(int totalPuts, int totalPrescribes, int totalGets);

	bool isDone() const;


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
		prescribes_(),
		items_(),
		gets_()
		{}

TagLog::~TagLog() {}

void TagLog::processPut(void * itemId) {
	typename items_t::iterator it = items_.find(itemId);
	if (it == items_.end()) {
		currentPuts_++;
		items_.insert(itemId);
	}
}

void TagLog::processPrescribe(void * tagId) {
	typename prescribes_t::iterator it = prescribes_.find(tagId);
	if (it == items_.end()) {
		currentPrescribes_++;
		prescribes_.insert(tagId);
	}
}


void TagLog::processGet( ItemCheckpoint_i * item_cp, void* tag) {
	//I don't think this is thread safe, is this a random access iterator? Yet my bug seems to have disappeared... //TODO
	tbb::concurrent_vector< getLog >::iterator it = std::find_if( gets_.begin(), gets_.end(), Compare(item_cp, tag) );
	if (it == gets_.end()) {
		currentGets_++;
		gets_.push_back(getLog(item_cp, tag));
	}
}

void TagLog::processDone(int totalPuts, int totalPrescribes, int totalGets) {
	markedDone_ = true;
	totalPrescribes_ = totalPrescribes;
	totalPuts_ = totalPuts;
	totalGets_ = totalGets;
}

bool TagLog::isDone() const {
	return markedDone_
			&& currentPrescribes_ == totalPrescribes_
			&& currentPuts_ == totalPuts_
			&& currentGets_ == totalGets_;
}

void TagLog::decrement_get_counts() {
	for (typename tbb::concurrent_vector< getLog >::const_iterator it = gets_.begin(); it != gets_.end(); ++it) {
		(it->first)->decrement_get_count(it->second);
	}
}

#endif /* TAGLOG_H_ */
