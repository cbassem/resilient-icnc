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
	typedef std::pair< ItemCheckpoint_i*, CnC::Internal::tag_base* > getLog; //FIXME tag by ptr, ref or val? No choice has to be a ptr, object slicing problem//
	typedef tbb::scalable_allocator< CnC::Internal::tag_base > tag_allocator_type;

	tag_allocator_type m_allocator;


	prescribes_t prescribes_;
	items_t items_;
	std::vector< getLog > gets_;

	struct Compare
	{
	  Compare(ItemCheckpoint_i* val1, CnC::Internal::tag_base& val2) : val1_(val1), val2_(val2) {}
	  bool operator()(const getLog& elem) const {
	    return val1_ == elem.first && val2_ == (*elem.second);
	  }
	  private:
	  ItemCheckpoint_i* val1_;
	  CnC::Internal::tag_base&  val2_;
	};

public:
	TagLog();
	virtual ~TagLog();

	void processPut(void * itemId );
	void processPrescribe(void * tagId);

	template< typename ItemTagType >
	void processGet(ItemCheckpoint_i * item_cp, CnC::Internal::typed_tag< ItemTagType >& tag); //FIXME tag by ptr, ref or val?

	void processDone(int totalPuts, int totalPrescribes);

	bool isDone() const;

	template< typename ItemTagType >
	CnC::Internal::tag_base * create( const CnC::Internal::typed_tag< ItemTagType > & org ) const;

	void uncreate( CnC::Internal::tag_base * item ) const;

	void decrement_get_counts();

};

TagLog::TagLog() :
		currentPuts_(0),
		currentPrescribes_(0),
		totalPuts_(-1),
		totalPrescribes_(-1),
		markedDone_(false),
		prescribes_(),
		items_(),
		gets_(),
		m_allocator() {}

TagLog::~TagLog() {
	for (typename std::vector< getLog >::iterator it = gets_.begin(); it != gets_.end(); ++it) {
		  delete it->second;
		  it = gets_.erase(it);
		}
	}

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

template< typename ItemTagType >
void TagLog::processGet(ItemCheckpoint_i * item_cp, CnC::Internal::typed_tag< ItemTagType >& tag) {
	std::vector< getLog >::iterator it = std::find_if( gets_.begin(), gets_.end(), Compare(item_cp, tag) );
	if (it == gets_.end()) {
		CnC::Internal::tag_base* t_ = new CnC::Internal::typed_tag< ItemTagType >(tag);
		gets_.push_back(getLog(item_cp, t_));
	}
}

void TagLog::processDone(int totalPuts, int totalPrescribes) {
	markedDone_ = true;
	totalPrescribes_ = totalPrescribes;
	totalPuts_ = totalPuts;
}

bool TagLog::isDone() const {
	return markedDone_ && currentPrescribes_ == totalPrescribes_ && currentPuts_ == totalPuts_;
}

void TagLog::decrement_get_counts() {
	for (typename std::vector< getLog >::const_iterator it = gets_.begin(); it != gets_.end(); ++it) {
		(it->first)->decrement_get_count(it->second);
	}
}

#endif /* TAGLOG_H_ */
