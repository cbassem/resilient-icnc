/*
 * CheckpointManager.h
 *
 *  Created on: Mar 24, 2015
 *      Author: blackline
 */

#ifndef CHECKPOINTMANAGER_H_
#define CHECKPOINTMANAGER_H_

template<class Tag, class Item>
class CheckpointManager {
public:

	virtual void processStepPrescribe(Tag prescriber, int prescriberColId, Tag tag, int tagCollectionId) = 0;
	virtual void processStepDone(Tag step, int stepColId, int puts, int prescribes) = 0;
	virtual void processItemPut(Tag producer, int stepProducerId, Tag key, Item item, int itemColId) = 0;

private:

};

#endif /* CHECKPOINTMANAGER_H_ */
