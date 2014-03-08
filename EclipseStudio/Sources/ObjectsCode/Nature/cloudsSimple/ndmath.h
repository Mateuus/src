#ifndef ARTY_ND_MATH
#define ARTY_ND_MATH

namespace ndmath
{
	enum InitMode
	{
		IDENTITY,
		ZERO,
		INVALIDATE
	};

	template<class T, int dim>
	struct NDElement
	{
		NDElement(InitMode m)
		{
			if(m == IDENTITY) identity();
			else if(m == ZERO) zero();
		}

		NDElement(){}
		NDElement(T v){ set(v);	}
		NDElement(const T* d){ load(d);	}

		void store(T* d)
		{
			for(int i = 0; i < dim; ++i) 
				d[i] = data[i]; 
		}

		void load(const T* d)
		{
			for(int i = 0; i < dim; ++i) 
				data[i] = d[i]; 
		}

		void set(T v)
		{
			for(int i = 0; i < dim; ++i) 
				data[i] = v; 
		}

		T& operator[](int i) { return data[i]; }
		T operator[](int i) const { return data[i]; }

		NDElement operator +(const NDElement& e) const { NDElement res = *this; res += e; return res; }
		NDElement operator -(const NDElement& e) const { NDElement res = *this; res -= e; return res; }
		NDElement operator *(const NDElement& e) const { NDElement res = *this; res *= e; return res; }
		NDElement operator /(const NDElement& e) const { NDElement res = *this; res /= e; return res; }
		template<class scalar> NDElement operator *(scalar e) const { NDElement res = *this; res *= e; return res; }
		template<class scalar> NDElement operator /(scalar e) const { NDElement res = *this; res /= e; return res; }

		void operator +=(const NDElement& e) { for(int i = 0; i < dim; ++i) data[i] += e[i]; }
		void operator -=(const NDElement& e) { for(int i = 0; i < dim; ++i) data[i] -= e[i]; }
		void operator *=(const NDElement& e) { for(int i = 0; i < dim; ++i) data[i] *= e[i]; }
		void operator /=(const NDElement& e) { for(int i = 0; i < dim; ++i) data[i] /= e[i]; }

		template<class scalar> void operator *=(scalar e) { for(int i = 0; i < dim; ++i) data[i] *= e; }
		template<class scalar> void operator /=(scalar e) { for(int i = 0; i < dim; ++i) data[i] /= e; }

		inline void identity()
		{
			for(int i = 0; i < dim; ++i) 
				data[i] = 1.0f;
		}

		inline void zero()
		{
			for(int i = 0; i < dim; ++i) 
				data[i] = 0;
		}

		T data[dim];
	};

	template<class T, int dim>
	struct NDInterval
	{
		typedef NDElement<T, dim> Element;

		NDInterval(InitMode m)
		{
			if(m == IDENTITY) identity();
			else if(m == INVALIDATE) invalidate();
		}

		NDInterval(){}

		NDInterval(T v1, T v2):minValue(std::min(v1, v2)), maxValue(std::max(v1, v2))
		{

		}

		inline T size(int dimension) const { return maxValue[dimension] - minValue[dimension]; }

		inline T maxSize(int& d) const
		{
			d = 0;
			T res = size(0);
			for(int i = 1; i < dim; ++i)
			{
				T si = size(i);
				if(res < si)
				{
					res = si;
					d = i;
				}
			}

			return res;
		}

		inline Element size() const
		{
			return maxValue - minValue;
		}


		inline T measure() const
		{
			T res = size(0);
			for(int i = 1; i < dim; ++i)
				res *= size(i);
	
			return res;
		}

		inline bool inside(Element& e)
		{
			for(int i = 0; i < dim; ++i)
			{
				if(e[i] < minValue[i] || e[i] > maxValue[i])
					return false;
			}

			return true;
		}

		inline bool intersection(const NDInterval& interval, NDInterval& result) const
		{
			NDInterval r;
			for(int i = 0; i < dim; ++i)
			{
				r.minValue[i] = std::max(interval.minValue[i], minValue[i]);
				r.maxValue[i] = std::min(interval.maxValue[i], maxValue[i]);

				if(r.minValue[i] > r.maxValue[i])
					return false;
			}

			result = r;
			return true;
		}

		inline void normalize()
		{
			for(int i = 0; i < dim; ++i)
			{
				if(minValue[i] > maxValue[i])
					std::swap(minValue[i], maxValue[i]);
			}
		}

		inline void random(Element& e) const
		{
			static Base::Random rnd;
			for(int i = 0; i < dim; ++i)
			{
				e[i] = minValue[i] + rnd.urnd()*size(i);
			}
		}

		inline void merge(const NDInterval& interval)
		{
			for(int i = 0; i < dim; ++i)
			{
				minValue[i] = std::min(minValue[i], interval.minValue[i]);
				maxValue[i] = std::max(maxValue[i], interval.maxValue[i]);
			}
		}

		inline void merge(const Element& e)
		{
			for(int i = 0; i < dim; ++i)
			{
				minValue[i] = std::min(minValue[i], e[i]);
				maxValue[i] = std::max(maxValue[i], e[i]);
			}
		}

		inline Element center() const
		{
			return minValue + extent();
		}

		inline Element extent() const
		{
			return (maxValue - minValue)*0.5f;
		}

		inline void scale(const Element& s)
		{
			Element ext = extent();
			Element c = minValue + ext;
			ext *= s;
			minValue = c - ext;
			maxValue = c + ext;
		}

		inline void relative(Element& e) const
		{
			for(int i = 0; i < dim; ++i)
			{
				e[i] = (e[i] - minValue[i]) / size(i);
			}
		}

		inline void absolute(Element& e) const
		{
			for(int i = 0; i < dim; ++i)
			{
				e[i] = e[i] * size(i) + minValue[i];
			}
		}

		inline void identity()
		{
			for(int i = 0; i < dim; ++i)
			{
				minValue[i] = -0.5f;
				maxValue[i] = 0.5f;
			}
		}

		inline void invalidate()
		{
			for(int i = 0; i < dim; ++i)
			{
				minValue[i] = +FLT_MAX;
				maxValue[i] = -FLT_MAX;
			}
		}

		Element minValue;
		Element maxValue;
	};

	//
	//
	//
	template<class T, int dim>
	inline T dot(const NDElement<T, dim>& e1, const NDElement<T, dim>& e2)
	{
		T res = 0;
		for(int i = 0; i < dim; ++i)
		{
			res += e1[i]*e2[i];
		}

		return res;
	}

	template<class T, int dim>
	inline void random(NDElement<T, dim>& e, const NDElement<T, dim>& maxVal)
	{
		static Base::Random rnd;
		for(int i = 0; i < dim; ++i)
		{
			e[i] = rnd.urnd() * maxVal[i];
		}
	}

	template<class T, int dim>
	inline void random(NDInterval<T, dim>& result, const NDInterval<T, dim>& bound)
	{
		bound.random(result.minValue);
		bound.random(result.maxValue);
		result.normalize();
	}


	//
	//
	//
	template<class T, int dim>
	inline T summarySize(const NDInterval<T, dim>* intervals, int count)
	{
		T result = 0;
		for(int i = 0; i < count; ++i)
		{
			const NDInterval<T, dim>& interval = intervals[i];
			T isize = interval.measure();

			NDInterval<T, dim> intersection;
			for(int j = 0; j < i && isize > 0; ++j)
			{
				const NDInterval<T, dim>& ti = intervals[j];

				if(interval.intersection(ti, intersection))
				{
					isize -= intersection.measure();	
				}				
			}

			result += isize;
		}

		return result;
	}

	template<class T, int dim, template<class, int> class ND >
	inline void boundBox(const ND<T, dim>* e, int count, NDInterval<T, dim>& bbox)
	{
		bbox.invalidate();
		for(int i = 0; i < count; ++i)
		{
			bbox.merge(e[i]);
		}
	}
	
}

#endif