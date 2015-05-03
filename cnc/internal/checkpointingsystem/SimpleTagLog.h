/*
 * TagLog.h
 *
 *  Created on: Mar 28, 2015
 *      Author: root
 */

#ifndef SIMPLETAGLOG_H_
#define SIMPLETAGLOG_H_


#include "ItemCheckpoint_i.h"
#include <vector>
#include <set>
#include <algorithm>
#include <tr1/unordered_map>
#include <cnc/internal/tag_base.h>


template< class Tag, class Item>
class SimpleTagLog {
	int currentPuts_;
	int currentPrescribes_;
	int totalPuts_;
	int totalPrescribes_;

	bool markedDone_;

	typedef std::set< Tag > pres_type;
	typedef std::tr1::unordered_map< Tag, Item > itemMap;
	//Note the tag here is the tag for the item, this is not necessarily the tag of the step!!!
	typedef std::pair< ItemCheckpoint_i*, CnC::Internal::tag_base > getLog; //FIXME tag by ptr, ref or val?

	std::vector< pres_type > prescribes_;
	std::vector< itemMap > items_;
	std::vector< getLog > gets_;


	struct Compare
	{
	  Compare(ItemCheckpoint_i* val1, CnC::Internal::tag_base& val2) : val1_(val1), val2_(val2) {}
	  bool operator()(const getLog& elem) const {
	    return val1_ == elem.first && val2_ == elem.second;
	  }
	  private:
	  ItemCheckpoint_i* val1_;
	  CnC::Internal::tag_base&  val2_;
	};


public:
	SimpleTagLog();
	SimpleTagLog(int tag_collections, int item_collections);
	virtual ~SimpleTagLog();

	void processPut(Tag key, Item item, int collId);
	void processPrescribe(Tag tag, int collId);
	void processGet(ItemCheckpoint_i * item_cp, CnC::Internal::tag_base& tag); //FIXME tag by ptr, ref or val?
	void processDone(int totalPuts, int totalPrescribes);

	bool isDone() const;
};

template< class Tag, class Item >
SimpleTagLog<Tag, Item>::SimpleTagLog() :
		currentPuts_(0), currentPrescribes_(0), totalPuts_(-1), totalPrescribes_(-1), markedDone_(false) {
}

template< class Tag, class Item >
SimpleTagLog<Tag, Item>::SimpleTagLog(int tag_collections, int item_collections) :
		currentPuts_(0),
		currentPrescribes_(0),
		totalPuts_(-1),
		totalPrescribes_(-1),
		markedDone_(false),
		prescribes_( std::vector< pres_type >(tag_collections) ),
		items_( std::vector< itemMap >(item_collections) )
		{}

template<class Tag, class Item>
SimpleTagLog<Tag, Item>::~SimpleTagLog() {}

template<class Tag, class Item>
void SimpleTagLog<Tag, Item>::processPut(Tag key, Item item, int collId) {
	typename itemMap::iterator it = items_[collId].find(key);
	if (it == items_[collId].end()) {
		currentPuts_++;
		items_[collId][key] = item;
	}
}

template<class Tag, class Item>
void SimpleTagLog<Tag, Item>::processPrescribe(Tag tag, int collId) {
	const bool is_in = prescribes_[collId].find(tag) != prescribes_[collId].end();
	if ( ! is_in ) {
		currentPrescribes_++;
		prescribes_[collId].insert(tag);
	};
}


template<class Tag, class Item>
void SimpleTagLog<Tag, Item>::processGet(ItemCheckpoint_i * item_cp, CnC::Internal::tag_base& tag) {
	std::vector< getLog >::iterator it = std::find_if( gets_.begin(), gets_.end(), Compare(item_cp, tag) );
	if (it == gets_.end()) {
		gets_.push_back(getLog(item_cp, tag));
	}
}

template<class Tag, class Item>
void SimpleTagLog<Tag, Item>::processDone(int totalPuts, int totalPrescribes) {
	markedDone_ = true;
	totalPrescribes_ = totalPrescribes;
	totalPuts_ = totalPuts;
}

template<class Tag, class Item>
bool SimpleTagLog<Tag, Item>::isDone() const {
	if ( markedDone_ && currentPrescribes_ == totalPrescribes_ && currentPuts_ == totalPuts_ ) {
		for (typedef std::vector< getLog >::iterator it = gets_.begin(); it != gets_.end(); ++it) {
			//(it->first).decrement_get_count(it->second);
		}
		return true;
	} else {
		return false;
	}
}


#endif /* SIMPLETAGLOG_H_ */
