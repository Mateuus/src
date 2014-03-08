#include "r3dPCH.h"
#include "cloudBoxesGeneration.h"
#include "random.h"

namespace CloudBoxes
{

	inline float Sign(float value)
	{
		return (value > 0.0f ? 1.0f : ( value < 0.0f ? -1.0f : 0.0f ));
	}
	Base::Random rnd;
	int randomIndex(int count)
	{
		return (int)(rnd.urnd() * count);
	}

	Box identityBox(ndmath::IDENTITY);
	Element identityElement(ndmath::IDENTITY);

	void generateBox(int currentCount, int targetCount, const Vector3& size, Box* boxes)
	{
		if(currentCount == targetCount)
			return;

		Element scale(&size.x);
		if(currentCount == 0)
		{
			boxes[0] = identityBox;
			boxes[0].scale(scale);
		}
		else
		{
			Box baseBox = boxes[ randomIndex(currentCount) ];

			Element relativePos;
			baseBox.random(relativePos);

			float sizeScaleX = rnd.srnd()*1.5f;// + 0.5f;
			float sizeScaleY = rnd.srnd()*1.5f;// + 0.5f;
			float sizeScaleZ = rnd.srnd()*1.5f;// + 0.5f;
			sizeScaleX += Sign(sizeScaleX)*0.5f;
			sizeScaleY += Sign(sizeScaleY)*0.5f;
			sizeScaleZ += Sign(sizeScaleZ)*0.5f;

					
 			Element bbs = boxes[0].size();
 			bbs.data[0] *= sizeScaleX;
 			bbs.data[1] *= sizeScaleY;
 			bbs.data[2] *= sizeScaleZ;
			ndmath::random(bbs, bbs);

			Box& newBox = boxes[currentCount];
			newBox.minValue = relativePos;
			newBox.maxValue = newBox.minValue + bbs;
			newBox.normalize();
		}

		generateBox(currentCount + 1, targetCount, size, boxes);
	}

	//
	void generate(int count, const Vector3& size, Box* boxes)
	{
		generateBox(0, count, size, boxes);	
	}

	void remap(Element* elements, int count, const Box& from, const Box& to_)
	{
		int idx;
		Element normalizedSize = from.size() / from.maxSize(idx);


		Box to = to_;
		Element toSize = to.size() * normalizedSize;
		to.maxValue = to.minValue + toSize;
		

		for(int i = 0; i < count; ++i)
		{
			from.relative( elements[i] );
			to.absolute( elements[i] );
		}
	}

	int refine(Element* elements, int count, float radius)
	{
		radius *= radius;

		int np = count;
		for(int w = 0; w < count; ++w)
		{
			for(int p = 0; p < count; ++p)
			{
				if(p != w)
				{
					Element e = elements[w] - elements[p];
					float dp = dot(e, e);
					if(dp < radius)
					{
						//delete particle
						count--;
						elements[p] = elements[count];
					}					
				}
			}
		}

		//LOG_MSG("refine: " << np << ", " << count);
		return count;
	}

	int generatePoints(float density, int maxPointsCount, const Vector3& size, Vector3* points, const Box& normalizationBox, Box& boundBox)
	{
		int boxesCount = 50;//(rand() % 1000) % (int)( std::max(size.x, std::max(size.y, size.z) ), std::min(size.x, std::min(size.y, size.z) ) );
		Box* boxes = new Box[boxesCount];
		Element* elements = new Element[maxPointsCount];

		Box boxesBB; boxesBB.invalidate();
		generate(boxesCount, size, boxes);
		ndmath::boundBox(boxes, boxesCount, boxesBB);
		float volume = ndmath::summarySize(boxes, boxesCount);

		float boxesBBVol = boxesBB.measure();
		float nBoxVol = normalizationBox.measure();
		float scaledVolume = volume * nBoxVol / boxesBBVol;
		
		boundBox.invalidate();

		int pointsCount = std::min((int)ceilf( scaledVolume * density ), maxPointsCount);

		int generated = 0;
		while(true)
		{
			if(generated == pointsCount)
				break;

			Element& point = elements[generated];
			boxesBB.random(point);

			for(int i = 0; i < boxesCount; ++i)
			{
				if(boxes[i].inside(point))
				{
					boundBox.merge(point);
					++generated;
					break;
				}
			}
		}

		remap(elements, generated, boundBox, normalizationBox);
		//int count = refine(elements, generated, 1.5f);
		int count = generated;
		

		boundBox.invalidate();
		for(int i = 0; i < count; ++i)
		{
			boundBox.merge( elements[i] );
			elements[i].store( &points[i].x );
		}

		delete [] boxes;
		delete [] elements;

		return count;
	}
}
