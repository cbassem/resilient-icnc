/*
 * ItemCheckpoint.h
 *
 *  Created on: Apr 28, 2015
 *      Author: blackline
 */

#ifndef ITEMCHECKPOINT_I_H_
#define ITEMCHECKPOINT_I_H_


class ItemCheckpoint_i {
public:
	virtual ~ItemCheckpoint_i() {};
	virtual int getId() = 0;
	virtual void add_checkpoint_locally() = 0;

};


#endif /* ITEMCHECKPOINT_I_H_ */
