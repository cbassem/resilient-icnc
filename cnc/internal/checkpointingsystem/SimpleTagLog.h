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


template< class Tag, class Item>
class TagLog {
	int currentPuts_;
	int currentPrescribes_;
	int totalPuts_;
	int totalPrescribes_;

	bool markedDone_;

	typedef std::set< Tag > pres_type;
	typedef std::tr1::unordered_map<Tag, Item > itemMap;

	std::vector< pres_type > prescribes_;
	std::vector< itemMap > items_;

public:
	TagLog();
	TagLog(int tag_collections, int item_collections);
	virtual ~TagLog();

	void processPut(Tag key, Item item, int collId);
	void processPrescribe(Tag tag, int collId);
	void processDone(int totalPuts, int totalPrescribes);

	bool isDone() const;
};

template< class Tag, class Item >
TagLog<Tag, Item>::TagLog() :
		currentPuts_(0), currentPrescribes_(0), totalPuts_(-1), totalPrescribes_(-1), markedDone_(false) {
}

template< class Tag, class Item >
TagLog<Tag, Item>::TagLog(int tag_collections, int item_collections) :
		currentPuts_(0),
		currentPrescribes_(0),
		totalPuts_(-1),
		totalPrescribes_(-1),
		markedDone_(false),
		prescribes_( std::vector< pres_type >(tag_collections) ),
		items_( std::vector< itemMap >(item_collections) )
		{}

template<class Tag, class Item>
inline TagLog<Tag, Item>::~TagLog() {
}

template<class Tag, class Item>
void TagLog<Tag, Item>::processPut(Tag key, Item item, int collId) {
	typename itemMap::iterator it = items_[collId].find(key);
	if (it == items_[collId].end()) {
		currentPuts_++;
		items_[collId][key] = item;
	}
}

template<class Tag, class Item>
void TagLog<Tag, Item>::processPrescribe(Tag tag, int collId) {
	const bool is_in = prescribes_[collId].find(tag) != prescribes_[collId].end();
	if ( ! is_in ) {
		currentPrescribes_++;
		prescribes_[collId].insert(tag);
	};
}

template<class Tag, class Item>
void TagLog<Tag, Item>::processDone(int totalPuts, int totalPrescribes) {
	markedDone_ = true;
	totalPrescribes_ = totalPrescribes;
	totalPuts_ = totalPuts;
}

template<class Tag, class Item>
bool TagLog<Tag, Item>::isDone() const {
	return ( markedDone_ && currentPrescribes_ == totalPrescribes_ && currentPuts_ == totalPuts_ );
}


#endif /* TAGLOG_H_ */
