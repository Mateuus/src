#include "r3dPCH.h"
#include "random.h"

namespace Base
{

typedef unsigned int uint;

#define RANDM 397
#define MATRIX_A 0x9908b0dfUL   // constant vector a
#define UMASK 0x80000000UL // most significant w-r bits
#define LMASK 0x7fffffffUL // least significant r bits
#define MIXBITS(u,v) ( ((u) & UMASK) | ((v) & LMASK) )
#define TWIST(u,v) ((MIXBITS(u,v) >> 1) ^ ((v)&1UL ? MATRIX_A : 0UL))

void Random::next_state()
{
	uint *p=state;

	// if setSeed() has not been called, 
	// a default initial seed is used         
	if (initf==0) setSeed(5489UL);

	left = RANDN;
	next = state;

	int j;
	for (j=RANDN-RANDM+1; --j; p++) 
		*p = p[RANDM] ^ TWIST(p[0], p[1]);

	for (j=RANDM; --j; p++) 
		*p = p[RANDM-RANDN] ^ TWIST(p[0], p[1]);

	*p = p[RANDM-RANDN] ^ TWIST(p[0], state[0]);
}

// These real versions are due to Isaku Wada, 2002/01/09 added


Random::Random()
{
	int left = 1;
	int initf = 0;
	//setSeed( GetTickCount() );
	setSeed(0);
}
float Random::urnd()
{
	uint a=uint(*this)>>5, b=uint(*this)>>6; 
	return(a*67108864.0f+b)*(1.0f/9007199254740992.0f); 
}
//random number generator [-1..1]
float Random::srnd()
{
	return urnd()*2.0f-1.0f;
}

// initializes state[RANDN] with a seed
void Random::setSeed(uint s)
{
	state[0]= s & 0xffffffffUL;
	for (int j=1; j<RANDN; j++)
	{
		state[j] = (1812433253UL * (state[j-1] ^ (state[j-1] >> 30)) + j); 
		// See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. 
		// In the previous versions, MSBs of the seed affect   
		// only MSBs of the array state[].                        
		// 2002/01/09 modified by Makoto Matsumoto             
		state[j] &= 0xffffffffUL;  // for >32 bit machines 
	}
	left = 1; initf = 1;
}

uint Random::getSeed()
{
	return *this;
}

// generates a random number on [0,0xffffffff]-interval 
Random::operator uint ()
{
	if (--left == 0) next_state();
	uint y = *next++;

	// Tempering 
	y ^= (y >> 11);
	y ^= (y << 7) & 0x9d2c5680UL;
	y ^= (y << 15) & 0xefc60000UL;
	y ^= (y >> 18);

	return y;
}

}; // namespace Base
