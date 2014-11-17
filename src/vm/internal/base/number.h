/*
 * type.h
 *
 *  Created on: Nov 15, 2014
 *      Author: sunyc
 */

#ifndef SRC_LPC_TYPE_H_
#define SRC_LPC_TYPE_H_

#include <cstdint>

/* type definiation for LPC types. */
typedef std::int64_t LPC_INT;
#define LPC_INT_MAX INT64_MAX
#define LPC_INT_MIN INT64_MIN
#define LPC_INT_FMTSTR_P PRId64

typedef double LPC_FLOAT;
#define LPC_FLOAT_MAX DBL_MAX
#define LPC_FLOAT_MIN DBL_MIN
#define LPC_FLOAT_FMTSTR_P "f"

#endif /* SRC_LPC_TYPE_H_ */
