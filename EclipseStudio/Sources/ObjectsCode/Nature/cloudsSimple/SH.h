#ifndef ARTY_SH
#define ARTY_SH

#include "types.h"

namespace Light{
namespace SH{

	enum BasisType
	{
		SPHERE,
	};

	struct Vector3
	{
		template<class V3>
		Vector3(const V3& v):x(v.x), y(v.y), z(v.z){}

		double x;
		double y;
		double z;
	};

	double SH(int l, int m, double theta, double phi);

	template<BasisType basis> struct evaluator;

	template<>	struct evaluator<SPHERE>		{ static inline double eval(int l, int m, double theta, double phi) { return SH(l, m, theta, phi); } };

	inline void thetaPhi(const Vector3& dir, double& theta, double& phi)
	{
		theta = acos(dir.y);
		phi = atan2(dir.z, dir.x);
	}

	template<int Order, BasisType basis>
	struct Sample
	{
		static const int order = Order;
		static const int n = (order + 1)*(order + 1);

		Sample();

		template<class Vector3Type>
		Sample(const Vector3Type& dir);

		Sample(double theta, double phi);

		void scale(double v);

		void add(const Sample& shs);				
		void add(const Sample& shs, double v);		
		void add(double theta, double phi);			
		void add(double theta, double phi, double v);

		template<class Vector3Type>	void add(const Vector3Type& dir);
		template<class Vector3Type>	void add(const Vector3Type& dir, double v);

		void evaluate(const Vector3& dir);
		void evaluate(double theta, double phi);

		template<class scalar>
		void extract(scalar* target, int count);

		void zero();

		double coeff[n];
	};

	template<int order>	struct SHSample : public Sample<order, SPHERE>
	{
		SHSample():Sample(){}

		template<class Vector3Type>
		SHSample(const Vector3Type& dir): Sample(dir) {}
	};

	//
	//
	//
	template<int order, BasisType basis>
	double dot(const Sample<order, basis>& a, const Sample<order, basis>& b)
	{
		double sum = 0;
		for(int i = 0; i < Sample<order, basis>::n; ++i) 
		{
			sum += a.coeff[i] * b.coeff[i];
		}
		return sum;
	}

	//
	//
	//
	template<int Order, BasisType basis>
	Sample<Order, basis>::Sample()
	{
		for (int i = 0; i < n; ++i)
			coeff[i] = 0.0;
	}

	template<int Order, BasisType basis>
	template<class Vector3Type>
	Sample<Order, basis>::Sample(const Vector3Type& dir)
	{
		evaluate(SH::Vector3(dir));
	}

	template<int Order, BasisType basis>
	Sample<Order, basis>::Sample(double theta, double phi)
	{
		evaluate(theta, phi);
	}

	template<int Order, BasisType basis>
	void Sample<Order, basis>::zero()
	{
		for (int i = 0; i < n; ++i) coeff[i] = 0.0; 		
	}

	template<int Order, BasisType basis>
	void Sample<Order, basis>::scale(double v)		
	{ 
		for (int i = 0; i < n; ++i) coeff[i] *= v; 
	}

	template<int Order, BasisType basis>
	void Sample<Order, basis>::add(const Sample& shs)
	{	
		for (int i = 0; i < n; ++i)	coeff[i] += shs.coeff[i];		
	}

	template<int Order, BasisType basis>
	void Sample<Order, basis>::add(const Sample& shs, double v)			
	{	
		for (int i = 0; i < n; ++i)	coeff[i] += shs.coeff[i] * v;	
	}

	template<int Order, BasisType basis>
	void Sample<Order, basis>::add(double theta, double phi)
	{	
		add(Sample(theta, phi));	
	}

	template<int Order, BasisType basis>
	void Sample<Order, basis>::add(double theta, double phi, double v)	
	{	
		add(Sample(theta, phi), v);	
	}

	template<int Order, BasisType basis>
	template<class Vector3Type>	
	void Sample<Order, basis>::add(const Vector3Type& dir)			
	{ 
		add(Sample(dir));		
	}

	template<int Order, BasisType basis>
	template<class Vector3Type>	
	void Sample<Order, basis>::add(const Vector3Type& dir, double v)	
	{ 
		add(Sample(dir), v);	
	}

	template<int Order, BasisType basis>
	void Sample<Order, basis>::evaluate(const Vector3& dir)
	{
		double theta, phi;
		thetaPhi(dir, theta, phi);
		evaluate(theta, phi);
	}

	template<int Order, BasisType basis>
	void Sample<Order, basis>::evaluate(double theta, double phi)
	{
		for(int l = 0; l <= order; ++l) 
		{
			for(int m = -l; m <= l; ++m) 
			{
				int i = l * l + l + m;
				coeff[i] = evaluator<basis>::eval(l, m, theta, phi);
			}
		}
	}

	template<int Order, BasisType basis>
	template<class scalar>
	void Sample<Order, basis>::extract(scalar* target, int count)
	{
		for (int i = 0; i < n && i < count; ++i)
			target[i] = (scalar)coeff[i];
	}
}}


#endif