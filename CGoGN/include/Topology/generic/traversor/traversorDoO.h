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

#ifndef __TRAVERSOR_DOO_H__
#define __TRAVERSOR_DOO_H__

#include "Topology/generic/dart.h"
#include "Topology/generic/traversor/traversorGen.h"

namespace CGoGN
{

template <typename MAP, unsigned int ORBIT>
class TraversorDartsOfOrbit //: public Traversor<MAP>
{
private:
	std::vector<Dart>::iterator m_current ;
	std::vector<Dart>* m_vd ;
	const GenericMap* m_map;

	TraversorDartsOfOrbit(const TraversorDartsOfOrbit<MAP,ORBIT>& /*tr*/){}

public:
	TraversorDartsOfOrbit(const MAP& map, Cell<ORBIT> c) ;

	 ~TraversorDartsOfOrbit();

	TraversorDartsOfOrbit(TraversorDartsOfOrbit<MAP,ORBIT>&& tr):
		 m_current(tr.m_current),m_vd(tr.m_vd)
	{
	}

	inline Dart begin() ;

	inline Dart end() ;

	inline Dart next() ;
} ;

template <typename MAP, unsigned int ORBIT>
class VTraversorDartsOfOrbit : public Traversor
{
private:
	std::vector<Dart>::iterator m_current ;
	std::vector<Dart>* m_vd ;
	const GenericMap* m_map;

	VTraversorDartsOfOrbit(const VTraversorDartsOfOrbit<MAP,ORBIT>& /*tr*/){}

public:
	VTraversorDartsOfOrbit(const MAP& map, Cell<ORBIT> c) ;

	~VTraversorDartsOfOrbit();

	inline Dart begin() ;

	inline Dart end() ;

	inline Dart next() ;
} ;

} // namespace CGoGN

#include "Topology/generic/traversor/traversorDoO.hpp"

#endif
