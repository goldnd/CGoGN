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

#include "Utils/threadbarrier.h"
#include <vector>

namespace CGoGN
{

template <typename MAP, unsigned int ORBIT, TraversalOptim OPT>
TraversorCell<MAP, ORBIT, OPT>::TraversorCell(const MAP& map, bool forceDartMarker) :
	m(map),
	dmark(NULL),
	cmark(NULL),
	quickTraversal(NULL),
	current(NIL),
	firstTraversal(true)
{
	dimension = map.dimension();

	switch(OPT)
	{
		case FORCE_DART_MARKING:
			dmark = new DartMarker<MAP>(map) ;
			break;
		case FORCE_CELL_MARKING:
			cmark = new CellMarker<MAP, ORBIT>(map) ;
			break;
		case FORCE_QUICK_TRAVERSAL:
			quickTraversal = map.template getQuickTraversal<ORBIT>() ;
			assert(quickTraversal != NULL);
			cont = &(map.template getAttributeContainer<ORBIT>()) ;
			break;
		case AUTO:
			if(forceDartMarker)
				dmark = new DartMarker<MAP>(map) ;
			else
			{
				quickTraversal = map.template getQuickTraversal<ORBIT>() ;
				if(quickTraversal != NULL)
				{
					cont = &(map.template getAttributeContainer<ORBIT>()) ;

				}
				else
				{
					if(map.template isOrbitEmbedded<ORBIT>())
						cmark = new CellMarker<MAP, ORBIT>(map) ;
					else
						dmark = new DartMarker<MAP>(map) ;
				}
			}
			break;
		default:
			break;
	}
}

template <typename MAP, unsigned int ORBIT, TraversalOptim OPT>
TraversorCell<MAP, ORBIT, OPT>::TraversorCell(const TraversorCell<MAP, ORBIT, OPT>& tc) :
	m(tc.m),
	dimension(tc.dimension),
	cont(tc.cont),
	qCurrent(tc.qCurrent),
	dmark(tc.dmark),
	cmark(tc.cmark),
	quickTraversal(tc.quickTraversal),
	current(tc.current),
	firstTraversal(tc.firstTraversal)
{}

template <typename MAP, unsigned int ORBIT, TraversalOptim OPT>
TraversorCell<MAP, ORBIT,OPT>::~TraversorCell()
{
	switch(OPT)
	{
		case FORCE_DART_MARKING:
			delete dmark ;
			break;
		case FORCE_CELL_MARKING:
			delete cmark ;
			break;
		case AUTO:
			if(dmark)
				delete dmark ;
			else if(cmark)
				delete cmark ;
			break;
		default:
			break;
	}
}

template <typename MAP, unsigned int ORBIT, TraversalOptim OPT>
Cell<ORBIT> TraversorCell<MAP, ORBIT, OPT>::begin()
{
	switch(OPT)
	{
		case FORCE_DART_MARKING:
		{
			if(!firstTraversal)
				dmark->unmarkAll() ;

			current.dart = m.begin() ;
			while(current.dart != m.end() && (m.isBoundaryMarked(dimension, current.dart)))
				m.next(current.dart) ;

			if(current.dart == m.end())
				current.dart = NIL ;
			else
				dmark->markOrbit(current) ;
		}
			break;
		case FORCE_CELL_MARKING:
		{
			if(!firstTraversal)
				cmark->unmarkAll() ;

			current.dart = m.begin() ;
			while(current.dart != m.end() && (m.isBoundaryMarked(dimension, current.dart)))
				m.next(current.dart) ;

			if(current.dart == m.end())
				current.dart = NIL ;
			else
				cmark->mark(current) ;
		}
			break;
		case FORCE_QUICK_TRAVERSAL:
		{
			qCurrent = cont->begin() ;
			current.dart = (*quickTraversal)[qCurrent] ;
		}
			break;
		case AUTO:
		{
			if(quickTraversal != NULL)
			{
				qCurrent = cont->begin() ;
				current.dart = (*quickTraversal)[qCurrent] ;
			}
			else
			{
				if(!firstTraversal)
				{
					if(dmark)
						dmark->unmarkAll() ;
					else
						cmark->unmarkAll() ;
				}

				current.dart = m.begin() ;
				while(current.dart != m.end() && (m.isBoundaryMarked(dimension, current.dart)))
					m.next(current.dart) ;

				if(current.dart == m.end())
					current.dart = NIL ;
				else
				{
					if(dmark)
						dmark->markOrbit(current) ;
					else
						cmark->mark(current) ;
				}
			}
		}
			break;
		default:
			break;
	}

	firstTraversal = false ;
	return current ;
}

template <typename MAP, unsigned int ORBIT, TraversalOptim OPT>
Cell<ORBIT> TraversorCell<MAP, ORBIT, OPT>::end()
{
	return Cell<ORBIT>(NIL) ;
}

template <typename MAP, unsigned int ORBIT, TraversalOptim OPT>
Cell<ORBIT> TraversorCell<MAP, ORBIT, OPT>::next()
{
	assert(current.dart != NIL);

	switch(OPT)
	{
		case FORCE_DART_MARKING:
		{
			bool ismarked = dmark->isMarked(current.dart) ;
			while(current.dart != NIL && (ismarked || m.isBoundaryMarked(dimension, current.dart)))
			{
				m.next(current.dart) ;
				if(current.dart == m.end())
					current.dart = NIL ;
				else
					ismarked = dmark->isMarked(current.dart) ;
			}
			if(current.dart != NIL)
				dmark->markOrbit(current) ;
		}
			break;
		case FORCE_CELL_MARKING:
		{
			bool ismarked = cmark->isMarked(current) ;
			while(current.dart != NIL && (ismarked || m.isBoundaryMarked(dimension, current.dart)))
			{
				m.next(current.dart) ;
				if(current.dart == m.end())
					current.dart = NIL ;
				else
					ismarked = cmark->isMarked(current) ;
			}
			if(current.dart != NIL)
				cmark->mark(current) ;
		}
			break;
		case FORCE_QUICK_TRAVERSAL:
		{
			cont->next(qCurrent) ;
			if (qCurrent != cont->end())
				current.dart = (*quickTraversal)[qCurrent] ;
			else current.dart = NIL;
		}
			break;
		case AUTO:
		{
			if(quickTraversal != NULL)
			{
				cont->next(qCurrent) ;
				if (qCurrent != cont->end())
					current.dart = (*quickTraversal)[qCurrent] ;
				else current.dart = NIL;
			}
			else
			{
				if(dmark)
				{
					bool ismarked = dmark->isMarked(current.dart) ;
					while(current.dart != NIL && (ismarked || m.isBoundaryMarked(dimension, current.dart)))
					{
						m.next(current.dart) ;
						if(current.dart == m.end())
							current.dart = NIL ;
						else
							ismarked = dmark->isMarked(current.dart) ;
					}
					if(current.dart != NIL)
						dmark->markOrbit(current) ;
				}
				else
				{
					bool ismarked = cmark->isMarked(current) ;
					while(current.dart != NIL && (ismarked || m.isBoundaryMarked(dimension, current.dart) ))
					{
						m.next(current.dart) ;
						if(current.dart == m.end())
							current.dart = NIL ;
						else
							ismarked = cmark->isMarked(current) ;
					}
					if(current.dart != NIL)
						cmark->mark(current) ;
				}
			}
		}
			break;
		default:
			break;
	}
	return current ;
}

template <typename MAP, unsigned int ORBIT, TraversalOptim OPT>
void TraversorCell<MAP, ORBIT, OPT>::skip(Cell<ORBIT> c)
{
	switch(OPT)
	{
		case FORCE_DART_MARKING:
			dmark->markOrbit(c) ;
			break;
		case FORCE_CELL_MARKING:
			cmark->mark(c) ;
			break;
		case FORCE_QUICK_TRAVERSAL:
			break;
		case AUTO:
			if(dmark)
				dmark->markOrbit(c) ;
			else
				cmark->mark(c) ;
			break;
		default:
			break;
	}
}




template <typename MAP, unsigned int ORBIT, TraversalOptim OPT>
Cell<ORBIT> TraversorCellEven<MAP, ORBIT, OPT>::begin()
{
	Cell<ORBIT> c = TraversorCell<MAP, ORBIT, OPT>::begin();
	this->firstTraversal = true;
	return c;
}



template <typename MAP, unsigned int ORBIT, TraversalOptim OPT>
Cell<ORBIT> TraversorCellOdd<MAP, ORBIT, OPT>::begin()
{
	switch(OPT)
	{
		case FORCE_DART_MARKING:
		{
			this->current.dart = this->m.begin() ;
			while(this->current.dart != this->m.end() && (this->m.isBoundaryMarked(this->dimension, this->current.dart) ))
				this->m.next(this->current.dart) ;

			if(this->current.dart == this->m.end())
				this->current.dart = NIL ;
			else
				this->dmark->unmarkOrbit(this->current) ;
		}
			break;
		case FORCE_CELL_MARKING:
		{
			this->current.dart = this->m.begin() ;
			while(this->current.dart != this->m.end() && (this->m.isBoundaryMarked(this->dimension, this->current.dart) ))
				this->m.next(this->current.dart) ;

			if(this->current.dart == this->m.end())
				this->current.dart = NIL ;
			else
				this->cmark->unmark(this->current) ;
		}
			break;
		case FORCE_QUICK_TRAVERSAL:
		{
			this->qCurrent = this->cont->begin() ;
			this->current.dart = this->quickTraversal->operator[](this->qCurrent);
		}
			break;
		case AUTO:
		{
			if(this->quickTraversal != NULL)
			{
				this->qCurrent = this->cont->begin() ;
				this->current.dart = this->quickTraversal->operator[](this->qCurrent);
			}
			else
			{
				this->current.dart = this->m.begin() ;
				while(this->current.dart != this->m.end() && (this->m.isBoundaryMarked(this->dimension, this->current.dart) ))
					this->m.next(this->current.dart) ;

				if(this->current.dart == this->m.end())
					this->current.dart = NIL ;
				else
				{
					if(this->dmark)
						this->dmark->unmarkOrbit(this->current) ;
					else
						this->cmark->unmark(this->current) ;
				}
			}
		}
			break;
		default:
			break;
	}
	return this->current ;
}

template <typename MAP, unsigned int ORBIT, TraversalOptim OPT>
Cell<ORBIT> TraversorCellOdd<MAP, ORBIT, OPT>::next()
{
	assert(this->current.dart != NIL);

	switch(OPT)
	{
		case FORCE_DART_MARKING:
		{
			bool ismarked = this->dmark->isMarked(this->current.dart) ;
			while(this->current.dart != NIL && (!ismarked || this->m.isBoundaryMarked(this->dimension,this->current.dart)))
			{
				this->m.next(this->current.dart) ;
				if(this->current.dart == this->m.end())
					this->current.dart = NIL ;
				else
					ismarked = this->dmark->isMarked(this->current.dart) ;
			}
			if(this->current.dart != NIL)
				this->dmark->unmarkOrbit(this->current) ;
		}
			break;
		case FORCE_CELL_MARKING:
		{
			bool ismarked = this->cmark->isMarked(this->current) ;
			while(this->current.dart != NIL && (!ismarked || this->m.isBoundaryMarked(this->dimension, this->current.dart) ))
			{
				this->m.next(this->current.dart) ;
				if(this->current.dart == this->m.end())
					this->current.dart = NIL ;
				else
					ismarked = this->cmark->isMarked(this->current) ;
			}
			if(this->current.dart != NIL)
				this->cmark->unmark(this->current) ;
		}
			break;
		case FORCE_QUICK_TRAVERSAL:
		{
			this->cont->next(this->qCurrent) ;
			if (this->qCurrent != this->cont->end())
				this->current.dart = this->quickTraversal->operator[](this->qCurrent) ;
			else this->current.dart = NIL;
		}
			break;
		case AUTO:
		{
			if(this->quickTraversal != NULL)
			{
				this->cont->next(this->qCurrent) ;
				if (this->qCurrent != this->cont->end())
					this->current.dart = this->quickTraversal->operator[](this->qCurrent) ;
				else this->current.dart = NIL;
			}
			else
			{
				if(this->dmark)
				{
					bool ismarked = this->dmark->isMarked(this->current.dart) ;
					while(this->current.dart != NIL && (!ismarked || this->m.isBoundaryMarked(this->dimension,this->current.dart)))
					{
						this->m.next(this->current.dart) ;
						if(this->current.dart == this->m.end())
							this->current.dart = NIL ;
						else
							ismarked = this->dmark->isMarked(this->current.dart) ;
					}
					if(this->current.dart != NIL)
						this->dmark->unmarkOrbit(this->current) ;
				}
				else
				{
					bool ismarked = this->cmark->isMarked(this->current) ;
					while(this->current.dart != NIL && (!ismarked || this->m.isBoundaryMarked(this->dimension, this->current.dart) ))
					{
						this->m.next(this->current.dart) ;
						if(this->current.dart == this->m.end())
							this->current.dart = NIL ;
						else
							ismarked = this->cmark->isMarked(this->current) ;
					}
					if(this->current.dart != NIL)
						this->cmark->unmark(this->current) ;
				}
			}
		}
			break;
		default:
			break;
	}

	return this->current ;
}




template <unsigned int ORBIT, typename MAP, typename FUNC>
inline void foreach_cell(const MAP& map, FUNC f, TraversalOptim opt)
{
	switch(opt)
	{
		case FORCE_DART_MARKING:
		{
			TraversorCell<MAP, ORBIT,FORCE_DART_MARKING> trav(map, false);
			for (Cell<ORBIT> c = trav.begin(), e = trav.end(); c.dart != e.dart; c = trav.next())
				f(c);
		}
			break;
		case FORCE_CELL_MARKING:
		{
			TraversorCell<MAP, ORBIT,FORCE_CELL_MARKING> trav(map, false);
			for (Cell<ORBIT> c = trav.begin(), e = trav.end(); c.dart != e.dart; c = trav.next())
				f(c);
		}
			break;
		case FORCE_QUICK_TRAVERSAL:
		{
			TraversorCell<MAP, ORBIT,FORCE_QUICK_TRAVERSAL> trav(map, false);
			for (Cell<ORBIT> c = trav.begin(), e = trav.end(); c.dart != e.dart; c = trav.next())
				f(c);
		}
			break;
		case AUTO:
		default:
		{
			TraversorCell<MAP, ORBIT,AUTO> trav(map, false);
			for (Cell<ORBIT> c = trav.begin(), e = trav.end(); c.dart != e.dart; c = trav.next())
				f(c);
		}
			break;
	}
}

template <unsigned int ORBIT, typename MAP, typename FUNC>
inline void foreach_cell_until(const MAP& map, FUNC f, TraversalOptim opt)
{
	switch(opt)
	{
		case FORCE_DART_MARKING:
		{
			TraversorCell<MAP, ORBIT,FORCE_DART_MARKING> trav(map, false);
			for (Cell<ORBIT> c = trav.begin(), e = trav.end(); c.dart != e.dart; c = trav.next())
				if (!f(c))
					break;
		}
			break;
		case FORCE_CELL_MARKING:
		{
			TraversorCell<MAP, ORBIT,FORCE_CELL_MARKING> trav(map, false);
			for (Cell<ORBIT> c = trav.begin(), e = trav.end(); c.dart != e.dart; c = trav.next())
				if (!f(c))
					break;
		}
			break;
		case FORCE_QUICK_TRAVERSAL:
		{
			TraversorCell<MAP, ORBIT,FORCE_QUICK_TRAVERSAL> trav(map, false);
			for (Cell<ORBIT> c = trav.begin(), e = trav.end(); c.dart != e.dart; c = trav.next())
				if (!f(c))
					break;
		}
			break;
		case AUTO:
		default:
		{
			TraversorCell<MAP, ORBIT,AUTO> trav(map, false);
			for (Cell<ORBIT> c = trav.begin(), e = trav.end(); c.dart != e.dart; c = trav.next())
				if (!f(c))
					break;
		}
			break;
	}
}


//template <unsigned int ORBIT, typename MAP, typename FUNC, typename FUNC2>
//inline void foreach_cell_EvenOdd(const MAP& map, FUNC f, FUNC2 g, unsigned int nbpasses, TraversalOptim opt)
//{
//	switch(opt)
//	{
//	case FORCE_DART_MARKING:
//	{
//		TraversorCell<MAP,ORBIT,FORCE_DART_MARKING> trav(map, false);
//		TraversorCellEven<MAP,ORBIT,FORCE_DART_MARKING> tr1(trav);
//		TraversorCellOdd<MAP,ORBIT,FORCE_DART_MARKING> tr2(trav);

//		for (unsigned int i=0; i<nbpasses; ++i)
//		{
//			for (Cell<ORBIT> c = trav.begin(), e = trav.end(); c.dart != e.dart; c = trav.next())
//				f(c);
//			for (Cell<ORBIT> c = trav.begin(), e = trav.end(); c.dart != e.dart; c = trav.next())
//				g(c);
//		}
//	}
//	break;
//	case FORCE_CELL_MARKING:
//	{
//		TraversorCell<MAP,ORBIT,FORCE_CELL_MARKING> trav(map, false);
//		TraversorCellEven<MAP,ORBIT,FORCE_CELL_MARKING> tr1(trav);
//		TraversorCellOdd<MAP,ORBIT, FORCE_CELL_MARKING> tr2(trav);

//		for (unsigned int i=0; i<nbpasses; ++i)
//		{
//			for (Cell<ORBIT> c = trav.begin(), e = trav.end(); c.dart != e.dart; c = trav.next())
//				f(c);
//			for (Cell<ORBIT> c = trav.begin(), e = trav.end(); c.dart != e.dart; c = trav.next())
//				g(c);
//		}
//	}
//	break;
//	case FORCE_QUICK_TRAVERSAL:
//	{
//		TraversorCell<MAP,ORBIT,FORCE_QUICK_TRAVERSAL> trav(map, false);
//		TraversorCellEven<MAP,ORBIT,FORCE_QUICK_TRAVERSAL> tr1(trav);
//		TraversorCellOdd<MAP,ORBIT,FORCE_QUICK_TRAVERSAL> tr2(trav);

//		for (unsigned int i=0; i<nbpasses; ++i)
//		{
//			for (Cell<ORBIT> c = trav.begin(), e = trav.end(); c.dart != e.dart; c = trav.next())
//				f(c);
//			for (Cell<ORBIT> c = trav.begin(), e = trav.end(); c.dart != e.dart; c = trav.next())
//				g(c);
//		}
//	}
//	break;
//	case AUTO:
//	default:
//	{
//		TraversorCell<MAP,ORBIT,AUTO> trav(map, false);
//		TraversorCellEven<MAP,ORBIT,AUTO> tr1(trav);
//		TraversorCellOdd<MAP,ORBIT,AUTO> tr2(trav);

//		for (unsigned int i=0; i<nbpasses; ++i)
//		{
//			for (Cell<ORBIT> c = trav.begin(), e = trav.end(); c.dart != e.dart; c = trav.next())
//				f(c);
//			for (Cell<ORBIT> c = trav.begin(), e = trav.end(); c.dart != e.dart; c = trav.next())
//				g(c);
//		}
//	}
//	break;
//	}
//}





namespace Parallel
{

/// internal functor for boost call
template <unsigned int ORBIT, typename FUNC>
class ThreadFunction
{
protected:
	typedef Cell<ORBIT> CELL;
	std::vector<CELL>& m_cells;
	Utils::Barrier& m_sync1;
	Utils::Barrier& m_sync2;
	bool& m_finished;
	unsigned int m_id;
	FUNC m_lambda;
	std::thread::id& m_threadId; // ref on thread::id in table of threads in genericMap for init at operator()

public:
	ThreadFunction(FUNC func, std::vector<CELL>& vd, Utils::Barrier& s1, Utils::Barrier& s2, bool& finished, unsigned int id, std::thread::id& threadId) :
		m_cells(vd), m_sync1(s1), m_sync2(s2), m_finished(finished), m_id(id), m_lambda(func), m_threadId(threadId)
	{
	}

	ThreadFunction(const ThreadFunction<ORBIT, FUNC>& tf):
		m_cells(tf.m_cells), m_sync1(tf.m_sync1), m_sync2(tf.m_sync2), m_finished(tf.m_finished), m_id(tf.m_id), m_lambda(tf.m_lambda){}

	std::thread::id& getThreadId() { return m_threadId; }

	void operator()()
	{
		// first thing to do set the thread id in genericMap
		m_threadId = std::this_thread::get_id();

		while (!m_finished)
		{
			for (typename std::vector<CELL>::const_iterator it = m_cells.begin(); it != m_cells.end(); ++it)
				m_lambda(*it, m_id);
			m_cells.clear();
			m_sync1.wait(); // wait every body has finished
			m_sync2.wait(); // wait vectors has been refilled
		}
	}
};


template <TraversalOptim OPT, unsigned int ORBIT, typename MAP, typename FUNC>
void foreach_cell_tmpl(MAP& map, FUNC func, unsigned int nbth)
{
	// buffer for cell traversing
	std::vector< Cell<ORBIT> >* vd = new std::vector< Cell<ORBIT> >[nbth];
	for (unsigned int i = 0; i < nbth; ++i)
		vd[i].reserve(SIZE_BUFFER_THREAD);

	unsigned int nb = 0;
	TraversorCell<MAP, ORBIT, OPT> trav(map);
	Cell<ORBIT> cell = trav.begin();
	Cell<ORBIT> c_end = trav.end();
	while ((cell.dart != c_end.dart) && (nb < nbth*SIZE_BUFFER_THREAD) )
	{
		vd[nb%nbth].push_back(cell);
		nb++;
		cell = trav.next();
	}
	Utils::Barrier sync1(nbth+1);
	Utils::Barrier sync2(nbth+1);
	bool finished=false;

	// launch threads
	std::thread** threads = new std::thread*[nbth];
	ThreadFunction<ORBIT,FUNC>** tfs = new ThreadFunction<ORBIT,FUNC>*[nbth];

	// add place for nbth new threads in the table of threadId in genericmap
//	unsigned int firstThread = map.addEmptyThreadIds(nbth);

	for (unsigned int i = 0; i < nbth; ++i)
	{
		std::thread::id& threadId = map.addEmptyThreadId();
		tfs[i] = new ThreadFunction<ORBIT,FUNC>(func, vd[i], sync1, sync2, finished, 1+i, threadId);
		threads[i] = new std::thread( std::ref( *(tfs[i]) ) );
	}

	// and continue to traverse the map
	std::vector< Cell<ORBIT> >* tempo = new std::vector< Cell<ORBIT> >[nbth];
	for (unsigned int i = 0; i < nbth; ++i)
		tempo[i].reserve(SIZE_BUFFER_THREAD);

	while (cell.dart != c_end.dart)
	{
		for (unsigned int i = 0; i < nbth; ++i)
			tempo[i].clear();
		unsigned int nb = 0;

		while ((cell.dart != c_end.dart) && (nb < nbth*SIZE_BUFFER_THREAD) )
		{
			tempo[nb%nbth].push_back(cell);
			nb++;
			cell = trav.next();
		}
		sync1.wait();// wait for all thread to finish its vector
		for (unsigned int i = 0; i < nbth; ++i)
			vd[i].swap(tempo[i]);
		sync2.wait();// everybody refilled then go
	}

	sync1.wait(); // wait for all thread to finish its vector
	finished = true; // say finsih to everyone
	sync2.wait(); // just wait for last barrier wait !

	// wait for all theads to be finished
	for (unsigned int i = 0; i < nbth; ++i)
	{
		threads[i]->join();
		delete threads[i];
		map.removeThreadId(tfs[i]->getThreadId());
		delete tfs[i];
	}

	delete[] tfs;
	delete[] threads;
	delete[] vd;
	delete[] tempo;
}

template <unsigned int ORBIT, typename MAP, typename FUNC>
void foreach_cell(MAP& map, FUNC func, TraversalOptim opt, unsigned int nbth)
{
	if (nbth < 2)
	{
		CGoGNerr << "Warning number of threads must be > 1 for //" << CGoGNendl;
		nbth = 2;
	}
	switch(opt)
	{
		case FORCE_DART_MARKING:
			foreach_cell_tmpl<FORCE_DART_MARKING,ORBIT,MAP,FUNC>(map,func,nbth-1);
			break;
		case FORCE_CELL_MARKING:
			foreach_cell_tmpl<FORCE_CELL_MARKING,ORBIT,MAP,FUNC>(map,func,nbth-1);
			break;
		case FORCE_QUICK_TRAVERSAL:
			foreach_cell_tmpl<FORCE_QUICK_TRAVERSAL,ORBIT,MAP,FUNC>(map,func,nbth-1);
			break;
		case AUTO:
		default:
			foreach_cell_tmpl<AUTO,ORBIT,MAP,FUNC>(map,func,nbth-1);
			break;
	}
}

} // namespace Parallel

} // namespace CGoGN
