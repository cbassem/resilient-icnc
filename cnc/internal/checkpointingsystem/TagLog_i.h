/*
 * StepCheckpoint.h
 *
 *  Created on: Apr 28, 2015
 *      Author: blackline
 */

#ifndef TAGLOG_I_H_
#define TAGLOG_I_H_


class TagLog_i {
public:
	virtual ~TagLog_i() {};
	virtual CnC::serializer * add_info(CnC::serializer * ser)=0;

};

#endif /* TAGLOG_I_H_ */
