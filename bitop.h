/*
 * bitop.h
 *
 *  Created on: 2016Äê5ÔÂ26ÈÕ
 *      Author: leiml
 */
#ifndef SRC_BITOP_H_
#define SRC_BITOP_H_
#include  <stdlib.h>
#ifdef X86_64
#define INT_BITSIZE 64
#else
#define INT_BITSIZE 32
#endif
inline int test_bit(int bitint, int bitnum)
{
	if (bitnum >= INT_BITSIZE)
	{
		abort();
	}

	return bitint & (1<<bitnum);

}
inline int clear_bit(int bitint, int bitnum)
{
	if (bitnum >= INT_BITSIZE)
	{
		abort();
	}

	return bitint & ~(1<<bitnum);

}
inline int set_bit(int bitint, int bitnum)
{
	if (bitnum >= INT_BITSIZE)
	{
		abort();
	}

	return bitint | (1<<bitnum);
}


#endif /* SRC_BITOP_H_ */
