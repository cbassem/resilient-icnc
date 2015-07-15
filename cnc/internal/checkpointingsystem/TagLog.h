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
#include "cnc/serializer.h"


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
	typedef std::pair< ItemCheckpoint_i*, void* > getLog;
	typedef tbb::concurrent_unordered_set< getLog > gets_t;


	prescribes_t prescribes_;
	items_t items_;
	//tbb::concurrent_vector< getLog > gets_;
	gets_t gets_;

public:
	TagLog();
	virtual ~TagLog();

	void processPut(void * itemId );
	void processPrescribe(void * tagId);
	void processGet( ItemCheckpoint_i * item_cp, void* tag); //FIXME tag by ptr, ref or val?

	void processDone(int totalPuts, int totalPrescribes, int totalGets);

	bool isDone() const;

	template < typename UserStepTag >
	void sendGets(const UserStepTag & getter, int getterColl);


	void decrement_get_counts();

};

inline TagLog::TagLog() :
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

inline TagLog::~TagLog() {}

inline void TagLog::processPut(void * itemId) {
	std::pair<items_t::iterator, bool> res = items_.insert(itemId);
	if (res.second) {
		currentPuts_++;
	}
}

inline void TagLog::processPrescribe(void * tagId) {
	std::pair<prescribes_t::iterator, bool> res = prescribes_.insert(tagId);
	if (res.second) {
		currentPrescribes_++;
	}
}


inline void TagLog::processGet( ItemCheckpoint_i * item_cp, void* tag) {
	std::pair<gets_t::iterator, bool> res = gets_.insert(getLog(item_cp, tag));
	if(res.second) {
		currentGets_++;
	}
}

inline void TagLog::processDone(int totalPuts, int totalPrescribes, int totalGets) {
	markedDone_ = true;
	totalPrescribes_ = totalPrescribes;
	totalPuts_ = totalPuts;
	totalGets_ = totalGets;
}

inline bool TagLog::isDone() const {
	return markedDone_
			&& currentPrescribes_ == totalPrescribes_
			&& currentPuts_ == totalPuts_
			&& currentGets_ == totalGets_;
}

inline void TagLog::decrement_get_counts() {
	for (typename gets_t::const_iterator it = gets_.begin(); it != gets_.end(); ++it) {
		(it->first)->decrement_get_count(it->second);
	}
}

template < typename UserStepTag >
inline void TagLog::sendGets(const UserStepTag & getter, int  getterColl) {
    CnC::serializer * _serlzr = new CnC::serializer( false, true );
    _serlzr->set_mode_pack();
    (*_serlzr) & getterColl & getter ;
	for (typename gets_t::const_iterator it = gets_.begin(); it != gets_.end(); ++it) {
		(it->first)->send_get(_serlzr, it->second);
	}
	delete _serlzr;
}

#endif /* TAGLOG_H_ */
