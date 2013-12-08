#pragma once

#include <iostream>
#include <sstream>

#define ASSERT_LOG(_a,_b)																\
	do {																				\
		if(!(_a)) {																		\
			std::ostringstream _s;														\
			_s << __FILE__ << ":" << __LINE__ << " FATAL: ASSERTION FAILED: " << _b << "\n";	\
			std::cerr << _s.str();														\
			exit(1);																	\
		}																				\
	} while(0)
