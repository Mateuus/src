#ifndef	TL_TFIXEDARRAY_H
#define	TL_TFIXEDARRAY_H

namespace r3dTL
{
	template < typename T, uint32_t N>
	class TFixedArray
	{
	public:
		static const uint32_t COUNT = N;

	public:
		TFixedArray();

	public:
		T& operator [] ( uint32_t idx );
		const T& operator [] ( uint32_t idx ) const;

		T* operator + ( uint32_t idx ) ;
		const T* operator + ( uint32_t idx ) const ;

		void Move( uint32_t targIdx, uint32_t srcIdx, uint32_t count ) ;

	private:
		T			mElems[ N ];
	};

	//------------------------------------------------------------------------

	template < typename T, uint32_t N >
	TFixedArray< T, N > :: TFixedArray()
	{
		for( uint32_t i = 0, e = N; i < e; i ++ )
		{
			mElems[ i ] = T();
		}
	}

	//------------------------------------------------------------------------

	template < typename T, uint32_t N >
	T&
	TFixedArray< T, N > :: operator [] ( uint32_t idx )
	{
		r3d_assert( idx < N );
		return mElems[ idx ];
	}

	//------------------------------------------------------------------------

	template < typename T, uint32_t N >
	const T&
	TFixedArray< T, N > :: operator [] ( uint32_t idx ) const
	{
		r3d_assert( idx < N );
		return mElems[ idx ];
	}

	//------------------------------------------------------------------------

	template < typename T, uint32_t N >
	T* 
	TFixedArray< T, N > :: operator + ( uint32_t idx )
	{
		r3d_assert( idx < N );
		return mElems + idx ;
	}

	//------------------------------------------------------------------------

	template < typename T, uint32_t N >
	const T*
	TFixedArray< T, N > :: operator + ( uint32_t idx ) const
	{
		r3d_assert( idx < N ) ;
		return mElems + idx ;
	}

	//------------------------------------------------------------------------

	template < typename T, uint32_t N >
	void
	TFixedArray< T, N > :: Move( uint32_t targIdx, uint32_t srcIdx, uint32_t count )
	{
		for( uint32_t i = 0, e = count, t = targIdx, s = srcIdx ; i < e ; i ++, t ++, s ++ )
		{
			mElems[ t ] = mElems[ s ] ;
		}
	}

}

#endif