#ifndef ARTY_RANDOM
#define ARTY_RANDOM

namespace Base
{
	class Random
	{
#define RANDN 624
		unsigned int state[RANDN];
		int left;
		int initf;
		unsigned int *next;
		void next_state();
	public:
		Random();
		void setSeed(unsigned int seed);
		unsigned int getSeed();
		operator unsigned int();
		//[0..1]
		float urnd();
		//[-1..1]
		float srnd();

	};

};//namespace Base

#endif