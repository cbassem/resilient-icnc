/*
 * SimpelCheckpointManager.h
 *
 *  Created on: Mar 24, 2015
 *      Author: blackline
 */

#ifndef SIMPELCHECKPOINTMANAGER_H_
#define SIMPELCHECKPOINTMANAGER_H_

#include "CheckpointManager.h"
#include "TagLog.h"
#include <vector>
#include <set>
#include <tr1/unordered_map>

/*
 * This simple checkpoint manager keeps every item in the checkpoint.
 * Only the tags that have to be re-added in case of failure are considered.
 * Also written very naively, w/o performance in mind...
 */

template<class Tag, class Item>
class SimpelCheckpointManager: public CheckpointManager<Tag, Item> {

public:
	SimpelCheckpointManager(int step_collections, int tag_collections, int item_collections);

	void processStepPrescribe(Tag prescriber, int prescriberColId, Tag tag, int tagCollectionId);
	void processStepDone(Tag step, int stepColId, int puts, int prescribes);
	void processItemPut(Tag producer, int stepProducerColId, Tag key, Item item, int itemColId);
	void calculateCheckpoint();
	void printCheckpoint();

private:
	typedef std::tr1::unordered_map<Tag, TagLog<Tag, Item> > hmap;
	typedef TagLog<Tag, Item> taglog;
	typedef std::tr1::unordered_map<Tag, Item > itemMap;

	int tag_collections_;
	int item_collections_;


	std::vector<hmap> tag_log_;
	//tag_checkpoint keeps a vector of vectors. One vector per step collection. Each containing all the tags that need to be restarted
	std::vector< std::set< Tag > > tag_checkpoint_;

	//item_checkpoint keeps a vector of vectors. One vector per item collection. Each containing all the items. (No gc in this version)
	std::vector< itemMap > item_checkpoint_;

	TagLog<Tag, Item>& getTagLog(int colid, Tag tag);
	void removeTagFromCheckpoint(Tag tag, int collection);

};



template<class Tag, class Item>
SimpelCheckpointManager<Tag, Item>::SimpelCheckpointManager(int step_collections, int tag_collections, int item_collections):
	tag_collections_(tag_collections), item_collections_(item_collections), tag_log_(step_collections), tag_checkpoint_( tag_collections ), item_checkpoint_( item_collections ) {};


template<class Tag, class Item>
void SimpelCheckpointManager<Tag, Item>::processStepPrescribe(Tag prescriber, int prescriberColId, Tag tag, int tagCollectionId) {
	getTagLog(prescriberColId, prescriber).processPrescribe(tag, tagCollectionId);
	tag_checkpoint_[tagCollectionId].insert(tag);
}

template<class Tag, class Item>
void SimpelCheckpointManager<Tag, Item>::processItemPut(Tag producer, int stepProducerColId, Tag key, Item item, int itemColId) {
	getTagLog(stepProducerColId, producer).processPut(key, item, itemColId);
	item_checkpoint_[itemColId][key] = item;
}

template<class Tag, class Item>
void SimpelCheckpointManager<Tag, Item>::processStepDone(Tag step, int stepColId, int puts, int prescribes) {
	getTagLog(stepColId, step).processDone(puts, prescribes);
}

template<class Tag, class Item>
void SimpelCheckpointManager<Tag, Item>::printCheckpoint() {
	std::cout << "Tags:" << std::endl;

	int currCol(0);
	for( typename std::vector< std::set< Tag > >::iterator it = tag_checkpoint_.begin(); it != tag_checkpoint_.end(); ++it ) {
		std::cout << "Tags for collection:" << currCol << std::endl;
		for( typename std::set< Tag >::iterator itt = it->begin(); itt != it->end(); ++itt ) {
			std::cout << *itt << ", " ;
		}
		currCol++;
	}

	std::cout << std::endl;
	std::cout << std::endl;
	std::cout << "Items:" << std::endl;

	currCol = 0;
	for( typename std::vector<itemMap>::iterator it = item_checkpoint_.begin(); it != item_checkpoint_.end(); ++it ) {
		for( typename itemMap::const_iterator itt = it->begin(); itt != it->end(); ++itt) {
			std::cout << "[" << itt->first << "," << itt->second << "] ";
		}
	}
	std::cout << std::endl;
}


template<class Tag, class Item>
void SimpelCheckpointManager<Tag, Item>::calculateCheckpoint() {
	int crrCol(0);
	for( typename std::vector<hmap>::iterator it = ++tag_log_.begin(); it != tag_log_.end(); ++it) { // Start from second, dirst is env, not sure what to do about this one yet
		for( typename hmap::const_iterator itt = it->begin(); itt != it->end(); ++itt) {
			const Tag & crrTag = itt->first;
			const taglog & crrLog = itt->second;
			if ( crrLog.isDone() ) {
				removeTagFromCheckpoint(crrTag, crrCol);
			}
		}
		crrCol++;
	}
}
template<class Tag, class Item>
void SimpelCheckpointManager<Tag, Item>::removeTagFromCheckpoint(Tag tag, int collection) {
	tag_checkpoint_[collection].erase(tag);
}

template<class Tag, class Item>
TagLog<Tag, Item>& SimpelCheckpointManager<Tag, Item>::getTagLog(int colid, Tag tag) {
	hmap & col = tag_log_[colid];
	typename hmap::iterator it = col.find(tag);
	if (it == col.end()) {
		taglog & a = col[tag] = taglog(tag_collections_, item_collections_);
		return a;
	} else {
		taglog & tmp = it->second;
		return tmp;
	}
}

#endif /* SIMPELCHECKPOINTMANAGER_H_ */