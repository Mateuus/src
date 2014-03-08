#include "StdAfx.h"
#include "Exporter.h"
#include "VertexCandidate.h"

//----------------------------------------------------------------------------//
// Debug                                                                      //
//----------------------------------------------------------------------------//
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

	float	CVertex::POS_EPSILON = 0.00001f;
	float	CVertex::NRM_ANGLE_EPSILON = cosf(5.0f / 180.0f * 3.1415926f); // max 3 degrees

CVertex::CVertex()
{
  m_iVertexId    = -1;

  m_iMapChannels = 0;
  for(int i=0; i<4; i++) {
    m_tu[i].u = 0.0f;
    m_tu[i].v = 1.0f;
  }
  
  m_tangent   = Point3(1, 0, 0);
  m_normal    = Point3(0, 1, 0);
  m_bitangent = Point3(0, 0, 1);
  m_w_tangent = 1;
  
  m_color     = Point3(1, 1, 1);
}

CVertex::~CVertex()
{
}

int vtx_sort_debug = 0;

// return sort operand. 
// -1 if vertex is lower, >1 if higher, 0 if same
int CVertex::SortOperand(const CVertex& rhs, bool apply_epsilon) const
{
	float diff;
	#define DIFF(VAL, EPS) \
		diff = VAL - rhs.VAL; \
		if(diff > (apply_epsilon ? EPS : 0.0f)) { \
		  if(vtx_sort_debug) U_Log("%s, %f\n", #VAL, diff); \
		  return 1; \
		} \
		if(diff < (apply_epsilon ? -EPS : 0.0f)) { \
		  if(vtx_sort_debug) U_Log("%s, %f\n", #VAL, diff); \
		  return -1; \
		}

	#define DIFF_ANGLE(VAL, EPS) \
		diff = DotProd(VAL, rhs.VAL); \
		if(apply_epsilon && diff >= EPS) { \
		  ; \
		} else if(diff >= 0) { \
		  if(vtx_sort_debug) U_Log("%s, %f\n", #VAL, diff); \
		  return 1; \
		} \
		else if(diff < 0) { \
		  if(vtx_sort_debug) U_Log("%s, %f\n", #VAL, diff); \
		  return -1; \
		}

	int d = vtx_sort_debug;
	vtx_sort_debug = 0;
  // compare the position
	DIFF(m_position.x, POS_EPSILON)
	DIFF(m_position.y, POS_EPSILON)
	DIFF(m_position.z, POS_EPSILON)
	vtx_sort_debug = d;

  // compare the normal
	DIFF_ANGLE(m_normal, NRM_ANGLE_EPSILON)
	DIFF_ANGLE(m_tangent, NRM_ANGLE_EPSILON)
	DIFF_ANGLE(m_bitangent, NRM_ANGLE_EPSILON)

  // compare the texture coordinates

	//if(m_iMapChannels != vertex.m_iMapChannels) {
	//  return FALSE;
	//}

	int textureCoordinateId;
	for(textureCoordinateId = 0; textureCoordinateId < m_iMapChannels; textureCoordinateId++)
	{
		DIFF(m_tu[textureCoordinateId].u, POS_EPSILON);
		DIFF(m_tu[textureCoordinateId].v, POS_EPSILON);
	}
	
	// color
	DIFF(m_color.x, 1.0f / 255.0f);
	DIFF(m_color.y, 1.0f / 255.0f);
	DIFF(m_color.z, 1.0f / 255.0f);
	
	#undef DIFF
	#undef DIFF_ANGLE

	// no diferences found
	return 0;
}

//----------------------------------------------------------------------------//
// Add an influence to the vertex                                    //
//----------------------------------------------------------------------------//
void CVertex::AddInfluence(INode *pNode, float weight)
{
  int boneId = theSkeleton.FindBoneByNode(pNode);

  // check if there is already an influence for this bone ( weird 3ds max behaviour =P )
  int influenceId;
  for(influenceId = 0; influenceId < m_vectorInfluence.size(); influenceId++)
  {
    // compare bone id
    if(m_vectorInfluence[influenceId].boneId == boneId)
    {
      // just add the weights
      m_vectorInfluence[influenceId].weight += weight;
      break;
    }
  }

  // create an influence object if there is none for the given bone
  if(influenceId == m_vectorInfluence.size())
  {
    Influence influence;
    influence.boneId = boneId;
    influence.weight = weight;
    // add it to the influence vector
    m_vectorInfluence.push_back(influence);
  }
}

//----------------------------------------------------------------------------//
// Add a texture coordinate pair to the vertex                       //
//----------------------------------------------------------------------------//
void CVertex::SetTextureCoordinate(int ch, float u, float v)
{
  if(ch >= 4) 
    ThrowError("SetTextureCoordinate ch >= 4\n");

  m_tu[ch].u = u;
  m_tu[ch].v = v;
  if(ch >= m_iMapChannels) m_iMapChannels = ch + 1;
  return;
}

//----------------------------------------------------------------------------//
// Adjust the bone assignment for a given max bone count and weight threshold //
//----------------------------------------------------------------------------//
void CVertex::AdjustBoneInfluences(int maxBoneCount, float weightThreshold)
{
  // sort all the influences by weight
  std::sort(m_vectorInfluence.begin(), m_vectorInfluence.end(), CompareInfluenceWeight);

  // erase all influences below the weight threshold
  std::vector<Influence>::iterator iteratorInfluence;
  for(iteratorInfluence = m_vectorInfluence.begin(); iteratorInfluence != m_vectorInfluence.end(); )
  {
    // check against the weight threshold
    if((*iteratorInfluence).weight < weightThreshold)
    {
      iteratorInfluence = m_vectorInfluence.erase(iteratorInfluence);
    }
    else
    {
      ++iteratorInfluence;
    }
  }

  if(m_vectorInfluence.size() > 4)
    ThrowError("CVertex::AdjustBoneInfluences - more that 4 weights");

  // erase all but the first few influences specified by max bone count
  if(m_vectorInfluence.size() > maxBoneCount)
  {
    m_vectorInfluence.resize(maxBoneCount);
  }

  // get the total weight of the influence
  float fWeight = 0;
  int influenceId;
  for(influenceId = 0; influenceId < m_vectorInfluence.size(); influenceId++) {
    fWeight += m_vectorInfluence[influenceId].weight;
  }
  
  // if there is no total weight, distribute it evenly over all influences
  if((fWeight < POS_EPSILON) && (m_vectorInfluence.size() > 0)) {
    fWeight = 1.0f / (float)m_vectorInfluence.size();
  }

  // normalize all influence weights
  for(influenceId = 0; influenceId < m_vectorInfluence.size(); influenceId++)
  {
    m_vectorInfluence[influenceId].weight /= fWeight;
  }
  
  return;
}

//----------------------------------------------------------------------------//
// Compare the weight of two given influences                                 //
//----------------------------------------------------------------------------//

BOOL CVertex::CompareInfluenceWeight(const Influence& influence1, const Influence& influence2)
{
  return influence1.weight > influence2.weight;
}
