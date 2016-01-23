/*******************************************************************************
 * CGoGN: Combinatorial and Geometric modeling with Generic N-dimensional Maps  *
 * version 0.1                                                                  *
 * Copyright (C) 2009-2012, IGG Team, LSIIT, University of Strasbourg           *
 *                                                                              *
 * This library is free software; you can redistribute it and/or modify it      *
 * under the terms of the GNU Lesser General Public License as published by the *
 * Free Software Foundation; either version 2.1 of the License, or (at your     *
 * option) any later version.                                                   *
 *                                                                              *
 * This library is distributed in the hope that it will be useful, but WITHOUT  *
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or        *
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License  *
 * for more details.                                                            *
 *                                                                              *
 * You should have received a copy of the GNU Lesser General Public License     *
 * along with this library; if not, write to the Free Software Foundation,      *
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA.           *
 *                                                                              *
 * Web site: http://cgogn.unistra.fr/                                           *
 * Contact information: cgogn@unistra.fr                                        *
 *                                                                              *
 *******************************************************************************/

#include "Algo/Geometry/basic.h"
#include "Algo/Geometry/area.h"

#include "Topology/generic/traversor/traversorCell.h"
#include "Topology/generic/traversor/traversor2.h"

#include <cmath>

namespace CGoGN
{

namespace Algo
{

namespace Surface
{

namespace Geometry
{

template<typename PFP, typename V_ATT>
typename V_ATT::DATA_TYPE triangleNormal(typename PFP::MAP& map, Face f, const V_ATT& position)
{
	CHECK_ATTRIBUTEHANDLER_ORBIT(V_ATT, VERTEX);

	typename V_ATT::DATA_TYPE N = Geom::triangleNormal(
		position[f.dart],
		position[map.phi1(f)],
		position[map.phi_1(f)]
	) ;
	N.normalize() ;
	return N ;
}

template<typename PFP, typename V_ATT>
typename V_ATT::DATA_TYPE newellNormal(typename PFP::MAP& map, Face f, const V_ATT& position)
{
	CHECK_ATTRIBUTEHANDLER_ORBIT(V_ATT, VERTEX);

	typedef typename V_ATT::DATA_TYPE VEC3;
	VEC3 N(0);

	foreach_incident2<VERTEX>(map, f, [&] (Vertex v)
	{
		const VEC3& P = position[v];
		const VEC3& Q = position[map.phi1(v)];
		N[0] += (P[1] - Q[1]) * (P[2] + Q[2]);
		N[1] += (P[2] - Q[2]) * (P[0] + Q[0]);
		N[2] += (P[0] - Q[0]) * (P[1] + Q[1]);
	});

	N.normalize();
	return N;
}

template<typename PFP, typename V_ATT>
typename V_ATT::DATA_TYPE faceNormal(typename PFP::MAP& map, Face f, const V_ATT& position)
{
	CHECK_ATTRIBUTEHANDLER_ORBIT(V_ATT, VERTEX);

	if(map.faceDegree(f) == 3)
		return triangleNormal<PFP>(map, f, position) ;
	else
		return newellNormal<PFP>(map, f, position) ;
}

template<typename PFP, typename V_ATT>
typename V_ATT::DATA_TYPE vertexNormal(typename PFP::MAP& map, Vertex v, const V_ATT& position)
{
	CHECK_ATTRIBUTEHANDLER_ORBIT(V_ATT, VERTEX);

	typedef typename V_ATT::DATA_TYPE VEC3 ;

	VEC3 N(0) ;

	foreach_incident2<FACE>(map, v, [&] (Face f)
	{
		VEC3 n = faceNormal<PFP>(map, f, position) ;
		if(!n.hasNan())
		{
			VEC3 v1 = Algo::Geometry::vectorOutOfDart<PFP>(map, f.dart, position) ;
			VEC3 v2 = Algo::Geometry::vectorOutOfDart<PFP>(map, map.phi_1(f), position);
			typename VEC3::DATA_TYPE l = (v1.norm2() * v2.norm2());
			if (l > (typename VEC3::DATA_TYPE(0.0)) )
			{
				n *= convexFaceArea<PFP>(map, f, position) / l ;
				N += n ;
			}
		}
	});

	N.normalize() ;
	return N ;
}

template<typename PFP, typename V_ATT>
typename V_ATT::DATA_TYPE vertexBorderNormal(typename PFP::MAP& map, Vertex v, const V_ATT& position)
{
	CHECK_ATTRIBUTEHANDLER_ORBIT(V_ATT, VERTEX);
	assert(map.dimension() == 3);

	typedef typename V_ATT::DATA_TYPE VEC3 ;

	VEC3 N(0) ;

	std::vector<Dart> faces;
	faces.reserve(16);
	map.foreach_dart_of_vertex(v, [&] (Dart d) { faces.push_back(d); });

	CellMarker<typename PFP::MAP, FACE> f(map);

	for(std::vector<Dart>::iterator it = faces.begin() ; it != faces.end() ; ++it)
	{
//		if(!f.isMarked(*it) && map.isBoundaryIncidentFace(*it))
		if (!f.isMarked(*it) && (map.template isBoundaryMarked<3>(map.phi3(*it))))
		{
			f.mark(*it);
			VEC3 n = faceNormal<PFP>(map, *it, position);
			if(!n.hasNan())
			{
				VEC3 v1 = Algo::Geometry::vectorOutOfDart<PFP>(map, *it, position);
				VEC3 v2 = Algo::Geometry::vectorOutOfDart<PFP>(map, map.phi_1(*it), position);
				n *= convexFaceArea<PFP>(map, *it, position) / (v1.norm2() * v2.norm2());
				N += n ;
			}
		}
	}

	N.normalize() ;
	return N ;
}

template <typename PFP, typename V_ATT, typename F_ATT>
void computeNormalFaces(typename PFP::MAP& map, const V_ATT& position, F_ATT& face_normal)
{
	CHECK_ATTRIBUTEHANDLER_ORBIT(V_ATT, VERTEX);
	CHECK_ATTRIBUTEHANDLER_ORBIT(F_ATT, FACE);

	if (CGoGN::Parallel::NumberOfThreads > 1)
	{
		Parallel::computeNormalFaces<PFP,V_ATT,F_ATT>(map, position, face_normal);
		return;
	}

	foreach_cell<FACE>(map, [&] (Face f)
	{
		face_normal[f] = faceNormal<PFP>(map, f, position) ;
	}, AUTO);
}

template <typename PFP, typename V_ATT>
void computeNormalVertices(typename PFP::MAP& map, const V_ATT& position, V_ATT& normal)
{
	CHECK_ATTRIBUTEHANDLER_ORBIT(V_ATT, VERTEX);

	if (CGoGN::Parallel::NumberOfThreads > 1)
	{
		Parallel::computeNormalVertices<PFP,V_ATT>(map, position, normal);
		return;
	}

	foreach_cell<VERTEX>(map, [&] (Vertex v)
	{
		normal[v] = vertexNormal<PFP>(map, v, position) ;
	}, FORCE_CELL_MARKING);
}

template <typename PFP, typename V_ATT>
typename PFP::REAL computeAngleBetweenNormalsOnEdge(typename PFP::MAP& map, Edge e, const V_ATT& position)
{
	CHECK_ATTRIBUTEHANDLER_ORBIT(V_ATT, VERTEX);

	typedef typename V_ATT::DATA_TYPE VEC3 ;

	if(map.isBoundaryEdge(e))
		return 0 ;

	Vertex v1(e.dart);
	Vertex v2 = map.phi2(e) ;
	const VEC3 n1 = faceNormal<PFP>(map, Face(v1), position) ;
	const VEC3 n2 = faceNormal<PFP>(map, Face(v2), position) ;
	VEC3 edge = position[v2] - position[v1] ;
	edge.normalize() ;
	typename PFP::REAL s = edge * (n1 ^ n2) ;
	typename PFP::REAL c = n1 * n2 ;
	typename PFP::REAL a(0) ;

	// the following trick is useful for avoiding NaNs (due to floating point errors)
	if (c > 0.5) a = asin(s) ;
	else
	{
		if(c < -1) c = -1 ;
		if (s >= 0) a = acos(c) ;
		else a = -acos(c) ;
	}
	//	if (isnan(a))
	if(a != a)
		std::cerr<< "Warning : computeAngleBetweenNormalsOnEdge returns NaN on edge " << v1 << "-" << v2 << std::endl ;

	return a ;
}

template <typename PFP, typename V_ATT, typename E_ATT>
void computeAnglesBetweenNormalsOnEdges(typename PFP::MAP& map, const V_ATT& position, E_ATT& angles)
{
	CHECK_ATTRIBUTEHANDLER_ORBIT(V_ATT, VERTEX);
	CHECK_ATTRIBUTEHANDLER_ORBIT(E_ATT, EDGE);

	if (CGoGN::Parallel::NumberOfThreads > 1)
	{
		Parallel::computeAnglesBetweenNormalsOnEdges<PFP,V_ATT,E_ATT>(map, position, angles);
		return;
	}

	foreach_cell<EDGE>(map, [&] (Edge e)
	{
		angles[e] = computeAngleBetweenNormalsOnEdge<PFP>(map, e, position) ;
	}, AUTO);
}


namespace Parallel
{

template <typename PFP, typename V_ATT>
void computeNormalVertices(typename PFP::MAP& map, const V_ATT& position, V_ATT& normal)
{
	CHECK_ATTRIBUTEHANDLER_ORBIT(V_ATT, VERTEX);

	CGoGN::Parallel::foreach_cell<VERTEX>(map, [&] (Vertex v, unsigned int /*thr*/)
	{
		normal[v] = vertexNormal<PFP>(map, v, position) ;
	}, FORCE_CELL_MARKING);
}

template <typename PFP, typename V_ATT, typename F_ATT>
void computeNormalFaces(typename PFP::MAP& map, const V_ATT& position, F_ATT& normal)
{
	CHECK_ATTRIBUTEHANDLER_ORBIT(V_ATT, VERTEX);
	CHECK_ATTRIBUTEHANDLER_ORBIT(F_ATT, FACE);

	CGoGN::Parallel::foreach_cell<FACE>(map, [&] (Face f, unsigned int /*thr*/)
	{
		normal[f] = faceNormal<PFP>(map, f, position) ;
	});
}

template <typename PFP, typename V_ATT, typename E_ATT>
void computeAnglesBetweenNormalsOnEdges(typename PFP::MAP& map, const V_ATT& position, E_ATT& angles)
{
	CHECK_ATTRIBUTEHANDLER_ORBIT(V_ATT, VERTEX);
	CHECK_ATTRIBUTEHANDLER_ORBIT(E_ATT, EDGE);

	CGoGN::Parallel::foreach_cell<EDGE>(map,[&](Edge e, unsigned int /*thr*/)
	{
		angles[e] = computeAngleBetweenNormalsOnEdge<PFP>(map, e, position) ;
	});
}

} // namespace Parallel


} // namespace Geometry

} // namespace Surface

} // namespace Algo

} // namespace CGoGN
