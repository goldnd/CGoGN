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

#ifndef __ALGO_GEOMETRY_BASIC_H__
#define __ALGO_GEOMETRY_BASIC_H__

#include "Geometry/basic.h"

namespace CGoGN
{

namespace Algo
{

namespace Geometry
{

/**
 * vectorOutOfDart return a dart from the position of vertex attribute of d to the position of vertex attribute of phi1(d)
 */
template <typename PFP>
inline typename PFP::VEC3 vectorOutOfDart(typename PFP::MAP& map, Dart d, const VertexAttribute<typename PFP::VEC3, typename PFP::MAP>& position)
{
	typename PFP::VEC3 vec = position[map.phi1(d)] ;
	vec -= position[d] ;
	return vec ;
}

template <typename PFP>
inline typename PFP::REAL edgeLength(typename PFP::MAP& map, Dart d, const VertexAttribute<typename PFP::VEC3, typename PFP::MAP>& position)
{
	typename PFP::VEC3 v = vectorOutOfDart<PFP>(map, d, position) ;
	return typename PFP::REAL(v.norm());
}

namespace Parallel
{

template <typename PFP>
typename PFP::REAL meanEdgeLength(typename PFP::MAP& map, const VertexAttribute<typename PFP::VEC3, typename PFP::MAP>& position)
{
	std::vector<typename PFP::REAL> lengths;
	std::vector<unsigned int> nbedges;
	lengths.resize(CGoGN::Parallel::NumberOfThreads);
	nbedges.resize(CGoGN::Parallel::NumberOfThreads);

	CGoGN::Parallel::foreach_cell<EDGE>(map, [&] (Edge e, unsigned int thr)
	{
		lengths[thr-1] += edgeLength<PFP>(map, e, position);
		++nbedges[thr-1];
	}
	,AUTO);

	typename PFP::REAL length(0);
	unsigned int nbe = 0;
	for (int i = 0; i < CGoGN::Parallel::NumberOfThreads; ++i)
	{
		length += lengths[i];
		nbe += nbedges[i];
	}

	return length / nbe;
}

} // namespace Parallel

template <typename PFP>
inline typename PFP::REAL meanEdgeLength(typename PFP::MAP& map,const VertexAttribute<typename PFP::VEC3, typename PFP::MAP>& position)
{
	if (CGoGN::Parallel::NumberOfThreads > 1)
	{
		return Parallel::meanEdgeLength<PFP>(map, position);
	}

	typename PFP::REAL length(0);
	unsigned int nbe = 0;

	foreach_cell<EDGE>(map, [&] (Edge e)
	{
		length += edgeLength<PFP>(map, e, position);
		++nbe;
	}
	,AUTO);

	return length / nbe;
}

template <typename PFP>
inline typename PFP::REAL angle(typename PFP::MAP& map, Dart d1, Dart d2, const VertexAttribute<typename PFP::VEC3, typename PFP::MAP>& position)
{
	typename PFP::VEC3 v1 = vectorOutOfDart<PFP>(map, d1, position) ;
	typename PFP::VEC3 v2 = vectorOutOfDart<PFP>(map, d2, position) ;
	return Geom::angle(v1, v2) ;
}

template <typename PFP>
bool isTriangleObtuse(typename PFP::MAP& map, Dart d, const VertexAttribute<typename PFP::VEC3, typename PFP::MAP>& position)
{
	return Geom::isTriangleObtuse(position[d], position[map.phi1(d)], position[map.phi_1(d)]) ;
}

} // namespace Geometry

} // namespace Algo

} // namespace CGoGN

#endif
