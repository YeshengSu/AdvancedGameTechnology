#include "Hull.h"
#include <algorithm>
#include <nclgl\NCLDebug.h>

Hull::Hull()
{

}

Hull::~Hull()
{

}

void Hull::Clear()
{
	m_vVertices.clear();
	m_vEdges.clear();
	m_vFaces.clear();
}

int Hull::AddVertex(const Vector3& v)
{
	HullVertex new_vertex;
	new_vertex._idx = m_vVertices.size();
	new_vertex._pos = v;

	m_vVertices.push_back(new_vertex);

	return new_vertex._idx;
}

int Hull::FindEdge(int v0_idx, int v1_idx)
{
	for (const HullEdge& edge : m_vEdges)
	{
		if ((edge._vStart == v0_idx && edge._vEnd == v1_idx)
			|| (edge._vStart == v1_idx && edge._vEnd == v0_idx))
		{
			return edge._idx;
		}
	}

	return -1; //Not Found
}

int Hull::ConstructNewEdge(int parent_face_idx, int vert_start, int vert_end)
{
	int out_idx = FindEdge(vert_start, vert_end);


	//Edge not already within the Hull, 
	if (out_idx == -1)
	{
		out_idx = m_vEdges.size();

		HullEdge new_edge;
		new_edge._idx = m_vEdges.size();
		new_edge._vStart = vert_start;
		new_edge._vEnd = vert_end;
		m_vEdges.push_back(new_edge);

		HullEdge* new_edge_ptr = &m_vEdges[new_edge._idx];

		//Find Adjacent Edges
		for (int i = 0; i < new_edge._idx; ++i)
		{
			if (m_vEdges[i]._vStart == vert_start
				|| m_vEdges[i]._vStart == vert_end
				|| m_vEdges[i]._vEnd == vert_start
				|| m_vEdges[i]._vEnd == vert_end)
			{
				m_vEdges[i]._adjoining_edge_ids.push_back(new_edge._idx);
				new_edge_ptr->_adjoining_edge_ids.push_back(i);
			}
		}

		//Update Contained Vertices
		m_vVertices[vert_start]._enclosing_edges.push_back(new_edge._idx);
		m_vVertices[vert_end]._enclosing_edges.push_back(new_edge._idx);
	}

	m_vEdges[out_idx]._enclosing_faces.push_back(parent_face_idx);
	return out_idx;
}

int Hull::AddFace(const Vector3& _normal, int nVerts, const int* verts)
{
	HullFace new_face;
	new_face._idx = m_vFaces.size();
	new_face._normal = _normal;
	new_face._normal.Normalise();

	m_vFaces.push_back(new_face);
	HullFace* new_face_ptr = &m_vFaces[new_face._idx];



	//Construct all contained edges
	int p0 = nVerts - 1;
	for (int p1 = 0; p1 < nVerts; ++p1)
	{
		new_face_ptr->_vert_ids.push_back(verts[p1]);
		new_face_ptr->_edge_ids.push_back(ConstructNewEdge(new_face._idx, verts[p0], verts[p1]));
		p0 = p1;
	}


	//Find Adjacent Faces
	for (int i = 0; i < new_face._idx; ++i)
	{
		HullFace& cFace = m_vFaces[i];
		bool found = false;
		for (size_t j = 0; found == false && j < cFace._edge_ids.size(); ++j)
		{
			for (int k = 0; found == false && k < nVerts; ++k)
			{
				if (new_face_ptr->_edge_ids[k] == cFace._edge_ids[j])
				{
					found = true;
					cFace._adjoining_face_ids.push_back(new_face._idx);
					new_face_ptr->_adjoining_face_ids.push_back(i);
				}
			}
		}
	}

	//Update Contained Vertices
	for (int i = 0; i < nVerts; ++i)
	{
		HullVertex* cVertStart	= &m_vVertices[m_vEdges[new_face_ptr->_edge_ids[i]]._vStart];
		HullVertex* cVertEnd	= &m_vVertices[m_vEdges[new_face_ptr->_edge_ids[i]]._vEnd];

		auto foundLocStart = std::find(cVertStart->_enclosing_faces.begin(), cVertStart->_enclosing_faces.end(), new_face._idx);
		if (foundLocStart == cVertStart->_enclosing_faces.end())
		{
			cVertStart->_enclosing_faces.push_back(new_face._idx);
		}

		auto foundLocEnd = std::find(cVertEnd->_enclosing_faces.begin(), cVertEnd->_enclosing_faces.end(), new_face._idx);
		if (foundLocEnd == cVertEnd->_enclosing_faces.end())
		{
			cVertEnd->_enclosing_faces.push_back(new_face._idx);
		}
	}

	return new_face._idx;
}

void Hull::RemoveFace(int faceidx)
{
	HullFace* face = &m_vFaces[faceidx];

	for (int edge_idx : face->_edge_ids)
	{
		HullEdge* edge = &m_vEdges[edge_idx];
		if (edge->_enclosing_faces.size() == 1)
		{
			for (int adj_edge_idx : edge->_adjoining_edge_ids)
			{
				HullEdge* adj_edge = &m_vEdges[adj_edge_idx];
				adj_edge->_adjoining_edge_ids.erase(std::remove(adj_edge->_adjoining_edge_ids.begin(), adj_edge->_adjoining_edge_ids.end(), edge_idx));
			}
			edge->_enclosing_faces.clear();
			edge->_adjoining_edge_ids.clear();
		}		
		else
			edge->_enclosing_faces.erase(std::remove(edge->_enclosing_faces.begin(), edge->_enclosing_faces.end(), faceidx));
	}
	face->_edge_ids.clear();

	for (int adj_face_idx : face->_adjoining_face_ids)
	{
		HullFace* face = &m_vFaces[adj_face_idx];
		face->_adjoining_face_ids.erase(std::remove(face->_adjoining_face_ids.begin(), face->_adjoining_face_ids.end(), faceidx));
	}
	face->_adjoining_face_ids.clear();

	for (int vert_idx : face->_vert_ids)
	{
		HullVertex* vert = &m_vVertices[vert_idx];
		//if (vert->_enclosing_faces.size() == 1)
		//	RemoveVertex(edge_idx);
		//else
		vert->_enclosing_faces.erase(std::remove(vert->_enclosing_faces.begin(), vert->_enclosing_faces.end(), faceidx));
	}
	face->_vert_ids.clear();
}


void Hull::GetMinMaxVerticesInAxis(const Vector3& local_axis, int* out_min_vert, int* out_max_vert)
{
	float cCorrelation;
	int minVertex, maxVertex;

	float minCorrelation = FLT_MAX, maxCorrelation = -FLT_MAX;

	for (size_t i = 0; i < m_vVertices.size(); ++i)
	{
		cCorrelation = Vector3::Dot(local_axis, m_vVertices[i]._pos);

		if (cCorrelation > maxCorrelation)
		{
			maxCorrelation = cCorrelation;
			maxVertex = i;
		}

		if (cCorrelation <= minCorrelation)
		{
			minCorrelation = cCorrelation;
			minVertex = i;
		}
	}

	if (out_min_vert) *out_min_vert = minVertex;
	if (out_max_vert) *out_max_vert = maxVertex;
}


void Hull::DebugDraw(const Matrix4& transform)
{
	//Draw all Hull Polygons
	for (HullFace& face : m_vFaces)
	{
		//Render Polygon as triangle fan
		if (face._vert_ids.size() > 2)
		{
			Vector3 polygon_start = transform * m_vVertices[face._vert_ids[0]]._pos;
			Vector3 polygon_last = transform * m_vVertices[face._vert_ids[1]]._pos;

			for (size_t idx = 2; idx < face._vert_ids.size(); ++idx)
			{
				Vector3 polygon_next = transform * m_vVertices[face._vert_ids[idx]]._pos;

				NCLDebug::DrawTriangleNDT(polygon_start, polygon_last, polygon_next, Vector4(1.0f, 1.0f, 1.0f, 0.2f));
				polygon_last = polygon_next;
			}
		}
	}

	//Draw all Hull Edges
	for (HullEdge& edge : m_vEdges)
	{
		NCLDebug::DrawThickLineNDT(transform * m_vVertices[edge._vStart]._pos, transform * m_vVertices[edge._vEnd]._pos, 0.02f, Vector4(1.0f, 0.2f, 1.0f, 1.0f));
	}
}