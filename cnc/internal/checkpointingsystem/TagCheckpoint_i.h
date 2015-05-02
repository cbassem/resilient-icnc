/*
 * ItemCheckpoint.h
 *
 *  Created on: Apr 28, 2015
 *      Author: blackline
 */

#ifndef TAGCHECKPOINT_I_H_
#define TAGCHECKPOINT_I_H_


class TagCheckpoint_i {
public:
	virtual ~TagCheckpoint_i() {};

	virtual int getId() = 0;
	virtual void calculate_checkpoint() = 0;
	virtual void add_checkpoint_locally() = 0;
	virtual void print() = 0;

};

#endif /* TAGCHECKPOINT_I_H_ */