/*
 * TagLog.h
 *
 *  Created on: Mar 28, 2015
 *      Author: root
 */

#ifndef TAGLOG_H_
#define TAGLOG_H_

#include <vector>
#include <set>
#include <algorithm>
#include <tr1/unordered_map>


class TagLog {
	int currentPuts_;
	int currentPrescribes_;
	int totalPuts_;
	int totalPrescribes_;

	bool markedDone_;

	typedef std::set< void * > prescribes_t;
	typedef std::set< void * > items_t;

	prescribes_t prescribes_;
	items_t items_;

public:
	TagLog();
	virtual ~TagLog();

	void processPut(void * itemId );
	void processPrescribe(void * tagId);
	void processDone(int totalPuts, int totalPrescribes);

	bool isDone() const;
};

TagLog::TagLog() :
		currentPuts_(0), currentPrescribes_(0), totalPuts_(-1), totalPrescribes_(-1), markedDone_(false), prescribes_(), items_() {
}

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

void TagLog::processDone(int totalPuts, int totalPrescribes) {
	markedDone_ = true;
	totalPrescribes_ = totalPrescribes;
	totalPuts_ = totalPuts;
}

bool TagLog::isDone() const {
	return ( markedDone_ && currentPrescribes_ == totalPrescribes_ && currentPuts_ == totalPuts_ );
}


#endif /* TAGLOG_H_ */
