/*
 * type.h
 *
 *  Created on: Nov 15, 2014
 *      Author: sunyc
 */

#ifndef SRC_LPC_TYPE_H_
#define SRC_LPC_TYPE_H_

#include <cstdint>
#include <limits>      // for max/min
#include <inttypes.h>  // for PRId64

/* type definiation for LPC types. */
typedef std::int64_t LPC_INT;
#define LPC_INT_MAX (std::numeric_limits<std::int64_t>::max())
#define LPC_INT_MIN (std::numeric_limits<std::int64_t>::min())
#define LPC_INT_FMTSTR_P PRId64

typedef double LPC_FLOAT;
#define LPC_FLOAT_MAX (std::numeric_limits<double>::max())
#define LPC_FLOAT_MIN (std::numeric_limits<double>::min())
#define LPC_FLOAT_FMTSTR_P "f"

#endif /* SRC_LPC_TYPE_H_ */
