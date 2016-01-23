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

#include "Topology/generic/dartmarker.h"

namespace CGoGN
{

/****************************************
 *         THREAD ID MANAGEMENT         *
 ****************************************/

inline unsigned int GenericMap::getCurrentThreadIndex() const
{
	std::thread::id id = std::this_thread::get_id();
	for (unsigned int i = 0; i < m_thread_ids.size(); ++i)
	{
		if (id == m_thread_ids[i])
			return i;
	}
	assert(m_thread_ids.size() < NB_THREADS + 1);
	if (m_authorizeExternalThreads)
	{
		m_thread_ids.push_back(id);
		return m_thread_ids.size() - 1;
	}
	else
	{
		return -1;
	}
}

//inline void GenericMap::addThreadId(const std::thread::id id)
//{
//	for (unsigned int i = 0; i < m_thread_ids.size(); ++i)
//	{
//		if (m_thread_ids[i] == id)
//			return;
//	}
//	assert(m_thread_ids.size() < NB_THREADS + 1);
//	if (m_authorizeExternalThreads)
//		m_thread_ids.push_back(id);
//}

inline void GenericMap::removeThreadId(const std::thread::id id)
{
	for (unsigned int i = 0; i < m_thread_ids.size(); ++i)
	{
		if (m_thread_ids[i] == id)
		{
			m_thread_ids[i] = m_thread_ids.back();
			m_thread_ids.pop_back();
			break;
		}
	}
}

inline std::thread::id& GenericMap::addEmptyThreadId()
{
	assert(m_thread_ids.size() < NB_THREADS + 1);
	unsigned int size = uint32(m_thread_ids.size());
	m_thread_ids.resize(size + 1);
	return m_thread_ids.back();
}

inline std::thread::id GenericMap::getThreadId(unsigned int index) const
{
	assert(index < m_thread_ids.size());
	return m_thread_ids[index];
}

inline void GenericMap::setExternalThreadsAuthorization(bool b)
{
	m_authorizeExternalThreads = b;
	if (!m_authorizeExternalThreads)
	{
		// keep only the thread that created the map
		while (m_thread_ids.size() > 1)
			m_thread_ids.pop_back();
	}
}


/****************************************
 *         BUFFERS MANAGEMENT           *
 ****************************************/

inline std::vector<Dart>* GenericMap::askDartBuffer() const
{
	unsigned int thread = getCurrentThreadIndex();

	if (s_vdartsBuffers[thread].empty())
	{
		std::vector<Dart>* vd = new std::vector<Dart>;
		vd->reserve(128);
		return vd;
	}

	std::vector<Dart>* vd = s_vdartsBuffers[thread].back();
	s_vdartsBuffers[thread].pop_back();
	return vd;
}

inline void GenericMap::releaseDartBuffer(std::vector<Dart>* vd) const
{
	unsigned int thread = getCurrentThreadIndex();

	if (vd->capacity() > 1024)
	{
		std::vector<Dart> v;
		vd->swap(v);
		vd->reserve(128);
	}
	vd->clear();
	s_vdartsBuffers[thread].push_back(vd);
}

inline std::vector<unsigned int>* GenericMap::askUIntBuffer() const
{
	unsigned int thread = getCurrentThreadIndex();

	if (s_vintsBuffers[thread].empty())
	{
		std::vector<unsigned int>* vui = new std::vector<unsigned int>;
		vui->reserve(128);
		return vui;
	}

	std::vector<unsigned int>* vui = s_vintsBuffers[thread].back();
	s_vintsBuffers[thread].pop_back();
	return vui;
}

inline void GenericMap::releaseUIntBuffer(std::vector<unsigned int>* vui) const
{
	unsigned int thread = getCurrentThreadIndex();

	if (vui->capacity() > 1024)
	{
		std::vector<unsigned int> v;
		vui->swap(v);
		vui->reserve(128);
	}
	vui->clear();
	s_vintsBuffers[thread].push_back(vui);
}


/****************************************
 *           DARTS MANAGEMENT           *
 ****************************************/

inline Dart GenericMap::newDart()
{
	unsigned int di = m_attribs[DART].insertLine();		// insert a new dart line
	m_attribs[DART].initMarkersOfLine(di);
	for(unsigned int i = 0; i < NB_ORBITS; ++i)
	{
		if (m_embeddings[i])							// set all its embeddings
		{
			(*m_embeddings[i])[di] = EMBNULL ;			// to EMBNULL
		}
	}

	return Dart::create(di) ;
}

inline void GenericMap::deleteDartLine(unsigned int index)
{
	m_attribs[DART].removeLine(index) ;	// free the dart line

	for(unsigned int orbit = 0; orbit < NB_ORBITS; ++orbit)
	{
		if (m_embeddings[orbit])									// for each embedded orbit
		{
			unsigned int emb = (*m_embeddings[orbit])[index] ;		// get the embedding of the dart
			if(emb != EMBNULL)
				m_attribs[orbit].unrefLine(emb);					// and unref the corresponding line
		}
	}
}

inline unsigned int GenericMap::copyDartLine(unsigned int index)
{
	unsigned int newindex = m_attribs[DART].insertLine() ;	// create a new dart line
	m_attribs[DART].copyLine(newindex, index) ;				// copy the given dart line
	for(unsigned int orbit = 0; orbit < NB_ORBITS; ++orbit)
	{
		if (m_embeddings[orbit])
		{
			unsigned int emb = (*m_embeddings[orbit])[newindex] ;	// add a ref to the cells pointed
			if(emb != EMBNULL)										// by the new dart line
				m_attribs[orbit].refLine(emb) ;
		}
	}
	return newindex ;
}

//inline bool GenericMap::isDartValid(Dart d)
//{
//	return !d.isNil() && m_attribs[DART].used(dartIndex(d)) ;
//}

/****************************************
 *         EMBEDDING MANAGEMENT         *
 ****************************************/

template <unsigned int ORBIT>
inline bool GenericMap::isOrbitEmbedded() const
{
	return (ORBIT == DART) || (m_embeddings[ORBIT] != NULL) ;
}

inline bool GenericMap::isOrbitEmbedded(unsigned int orbit) const
{
	return (orbit == DART) || (m_embeddings[orbit] != NULL) ;
}

template <unsigned int ORBIT>
inline unsigned int GenericMap::newCell()
{
	assert(isOrbitEmbedded<ORBIT>() || !"Invalid parameter: orbit not embedded");

	unsigned int c = m_attribs[ORBIT].insertLine();
	m_attribs[ORBIT].initMarkersOfLine(c);
	return c;
}

template <unsigned int ORBIT>
inline void GenericMap::copyCell(unsigned int i, unsigned int j)
{
	assert(isOrbitEmbedded<ORBIT>() || !"Invalid parameter: orbit not embedded");
	m_attribs[ORBIT].copyLine(i, j) ;
}

template <unsigned int ORBIT>
inline void GenericMap::initCell(unsigned int i)
{
	assert(isOrbitEmbedded<ORBIT>() || !"Invalid parameter: orbit not embedded");
	m_attribs[ORBIT].initLine(i) ;
}

/****************************************
 *   ATTRIBUTES CONTAINERS MANAGEMENT   *
 ****************************************/

inline unsigned int GenericMap::getNbCells(unsigned int orbit)
{
	return m_attribs[orbit].size() ;
}

template <unsigned int ORBIT>
inline AttributeContainer& GenericMap::getAttributeContainer()
{
	return m_attribs[ORBIT] ;
}

template <unsigned int ORBIT>
inline const AttributeContainer& GenericMap::getAttributeContainer() const
{
	return m_attribs[ORBIT] ;
}

inline AttributeContainer& GenericMap::getAttributeContainer(unsigned int orbit)
{
	return m_attribs[orbit] ;
}

inline const AttributeContainer& GenericMap::getAttributeContainer(unsigned int orbit) const
{
	return m_attribs[orbit] ;
}

inline AttributeMultiVectorGen* GenericMap::getAttributeVectorGen(unsigned int orbit, const std::string& nameAttr)
{
	return m_attribs[orbit].getVirtualDataVector(nameAttr) ;
}


template <unsigned int ORBIT>
AttributeMultiVector<MarkerBool>* GenericMap::askMarkVector()
{
	assert(isOrbitEmbedded<ORBIT>() || !"Invalid parameter: orbit not embedded") ;

	// get current thread index for table of markers
	unsigned int thread = getCurrentThreadIndex();

	if (!m_markVectors_free[ORBIT][thread].empty())
	{
		AttributeMultiVector<MarkerBool>* amv = m_markVectors_free[ORBIT][thread].back();
		m_markVectors_free[ORBIT][thread].pop_back();
		return amv;
	}
	else
	{
		std::lock_guard<std::mutex> lockMV(m_MarkerStorageMutex[ORBIT]);

		unsigned int x = m_nextMarkerId++;
		std::string number("___");
		number[2] = '0'+x%10;
		x = x/10;
		number[1] = '0'+x%10;
		x = x/10;
		number[0] = '0'+x%10;

		AttributeMultiVector<MarkerBool>* amv = m_attribs[ORBIT].addMarkerAttribute("marker_" + orbitName(ORBIT) + number);
		return amv;
	}
}

template <unsigned int ORBIT>
inline void GenericMap::releaseMarkVector(AttributeMultiVector<MarkerBool>* amv)
{
	assert(isOrbitEmbedded<ORBIT>() || !"Invalid parameter: orbit not embedded") ;

	unsigned int thread = getCurrentThreadIndex();
	m_markVectors_free[ORBIT][thread].push_back(amv);
}



template <unsigned int ORBIT>
inline AttributeMultiVector<unsigned int>* GenericMap::getEmbeddingAttributeVector()
{
	return m_embeddings[ORBIT] ;
}

template <typename R>
bool GenericMap::registerAttribute(const std::string &nameType)
{
	RegisteredBaseAttribute* ra = new RegisteredAttribute<R>;
	if (ra == NULL)
	{
		CGoGNerr << "Erreur enregistrement attribut" << CGoGNendl;
		return false;
	}

	ra->setTypeName(nameType);

	m_attributes_registry_map->insert(std::pair<std::string, RegisteredBaseAttribute*>(nameType,ra));
	return true;
}

/****************************************
 *   EMBEDDING ATTRIBUTES MANAGEMENT    *
 ****************************************/

template <unsigned int ORBIT>
void GenericMap::addEmbedding()
{
	if (!isOrbitEmbedded<ORBIT>())
	{
		std::ostringstream oss;
		oss << "EMB_" << ORBIT;

		AttributeContainer& dartCont = m_attribs[DART] ;
		AttributeMultiVector<unsigned int>* amv = dartCont.addAttribute<unsigned int>(oss.str()) ;
		m_embeddings[ORBIT] = amv ;

		// set new embedding to EMBNULL for all the darts of the map
		for(unsigned int i = dartCont.begin(); i < dartCont.end(); dartCont.next(i))
			(*amv)[i] = EMBNULL ;
	}
}

/****************************************
 *          ORBITS TRAVERSALS           *
 ****************************************/

/****************************************
 *  TOPOLOGICAL ATTRIBUTES MANAGEMENT   *
 ****************************************/

inline AttributeMultiVector<Dart>* GenericMap::addRelation(const std::string& name)
{
	AttributeContainer& cont = m_attribs[DART] ;
	AttributeMultiVector<Dart>* amv = cont.addAttribute<Dart>(name) ;

	// set new relation to fix point for all the darts of the map
	for(unsigned int i = cont.begin(); i < cont.end(); cont.next(i))
		(*amv)[i] = Dart(i) ;

	return amv ;
}

inline AttributeMultiVector<Dart>* GenericMap::getRelation(const std::string& name)
{
	AttributeContainer& cont = m_attribs[DART] ;
	AttributeMultiVector<Dart>* amv = cont.getDataVector<Dart>(cont.getAttributeIndex(name)) ;
	return amv ;
}

inline float GenericMap::fragmentation(unsigned int orbit)
{
	if (isOrbitEmbedded(orbit))
		return m_attribs[orbit].fragmentation();
	return 1.0f;
}

} //namespace CGoGN
