#ifndef ARTY_TYPES
#define ARTY_TYPES

typedef D3DXVECTOR2 Vector2;
typedef D3DXVECTOR3 Vector3;
typedef D3DXVECTOR4 Vector4;

inline float magnitude(const Vector3& v)
{
	return D3DXVec3Length(&v);
}

inline float magnitudeSq(const Vector3& v)
{
	return D3DXVec3LengthSq(&v);
}


inline float magnitude(const Vector2& v)
{
	return D3DXVec2Length(&v);
}

inline float magnitudeSq(const Vector2& v)
{
	return D3DXVec2LengthSq(&v);
}

#endif
