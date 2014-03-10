#ifndef __PROFILER__
#define __PROFILER__

#include <stdio.h>

		void syncStart();
		void writeTime(long int time);
		void speced();
		void specFailed();
		void setElsapedTimer();
		double getCurTimeInMs();

#endif
