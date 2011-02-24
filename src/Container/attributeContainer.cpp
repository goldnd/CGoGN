/*******************************************************************************
* CGoGN: Combinatorial and Geometric modeling with Generic N-dimensional Maps  *
* version 0.1                                                                  *
* Copyright (C) 2009, IGG Team, LSIIT, University of Strasbourg                *
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
* Web site: https://iggservis.u-strasbg.fr/CGoGN/                              *
* Contact information: cgogn@unistra.fr                                        *
*                                                                              *
*******************************************************************************/

#include <typeinfo>
#include <stdio.h>
#include <string.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include <iostream>

#include "Container/attributeContainer.h"

namespace CGoGN
{

AttributeContainer::AttributeContainer() :
	m_orbit(0),
	m_nbAttributes(0),
	m_nbUnknown(0),
	m_size(0),
	m_maxSize(0),
	m_lineCost(0),
	m_attributes_registry_map(NULL)
{
	m_holesBlocks.reserve(512);
}

AttributeContainer::~AttributeContainer()
{
}

/**************************************
 *          BASIC FEATURES            *
 **************************************/

bool AttributeContainer::removeAttribute(const std::string& attribName)
{
	unsigned int index = getAttributeIndex(attribName) ;

	if (index == UNKNOWN)
	{
		std::cerr << "removeAttribute by name: attribute not found" << std::endl ;
		return false ;
	}

	// delete the attribute
	delete m_tableAttribs[index] ;
	m_tableAttribs[index] = NULL ;

	if (index == m_tableAttribs.size() - 1)
		m_tableAttribs.pop_back() ;
	else
		m_freeIndices.push_back(index) ;

	--m_nbAttributes ;

	return true ;
}

bool AttributeContainer::removeAttribute(unsigned int index)
{
	if(m_tableAttribs[index] == NULL)
	{
		std::cerr << "removeAttribute by index: attribute not found" << std::endl ;
		return false ;
	}

	// delete the attribute
	delete m_tableAttribs[index] ;
	m_tableAttribs[index] = NULL ;

	if(index == m_tableAttribs.size() - 1)
		m_tableAttribs.pop_back() ;
	else
		m_freeIndices.push_back(index) ;

	--m_nbAttributes ;

	return true ;
}

/**************************************
 *       INFO ABOUT ATTRIBUTES        *
 **************************************/

unsigned int AttributeContainer::getAttributeIndex(const std::string& attribName)
{
	unsigned int index ;
	bool found = false ;
	for (index = 0; index < m_tableAttribs.size() && !found; ++index)
	{
		if(m_tableAttribs[index] != NULL && m_tableAttribs[index]->getName() == attribName)
			found = true ;
	}

	if (!found)
		return UNKNOWN;
	else
		return index - 1 ;
}

const std::string& AttributeContainer::getAttributeName(unsigned int index)
{
	assert(index < m_tableAttribs.size() || !"getAttributeName: attribute index out of bounds");
	assert(m_tableAttribs[index] != NULL || !"getAttributeName: attribute does not exist");

	if(m_tableAttribs[index] != NULL)
		return m_tableAttribs[index]->getName() ;
}

template <typename T>
unsigned int AttributeContainer::getAttributeBlocksPointers(unsigned int attrIndex, std::vector<T*>& vect_ptr, unsigned int& byteBlockSize)
{
	assert(attrIndex < m_tableAttribs.size() || !"getAttributeBlocksPointers: attribute index out of bounds");
	assert(m_tableAttribs[attrIndex] != NULL || !"getAttributeBlocksPointers: attribute does not exist");

	AttributeMultiVector<T>* atm = dynamic_cast<AttributeMultiVector<T>*>(m_tableAttribs[attrIndex]);
	assert((atm != NULL) || !"getAttributeBlocksPointers: wrong type");
	return atm->getBlocksPointers(vect_ptr, byteBlockSize);
}

unsigned int AttributeContainer::getAttributesNames(std::vector<std::string>& names)
{
	names.clear() ;
	names.reserve(m_nbAttributes) ;

	for (unsigned int i = 0; i < m_tableAttribs.size(); ++i)
	{
		if(m_tableAttribs[i] != NULL)
			names.push_back(m_tableAttribs[i]->getName()) ;
	}

	return m_nbAttributes ;
}

/**************************************
 *        CONTAINER MANAGEMENT        *
 **************************************/

void AttributeContainer::swap(AttributeContainer& cont)
{
	// swap everything but the orbit

	m_tableAttribs.swap(cont.m_tableAttribs);
	m_freeIndices.swap(cont.m_freeIndices);
	m_holesBlocks.swap(cont.m_holesBlocks);
	m_tableBlocksWithFree.swap(cont.m_tableBlocksWithFree);
	m_tableBlocksEmpty.swap(cont.m_tableBlocksEmpty);

	unsigned int temp = m_nbAttributes;
	m_nbAttributes = cont.m_nbAttributes;
	cont.m_nbAttributes = temp;

	temp = m_nbUnknown ;
	m_nbUnknown = cont.m_nbUnknown ;
	cont.m_nbUnknown = temp ;

	temp = m_size;
	m_size = cont.m_size;
	cont.m_size = temp;

	temp = m_maxSize;
	m_maxSize = cont.m_maxSize;
	cont.m_maxSize = temp;

	temp = m_lineCost;
	m_lineCost = cont.m_lineCost;
	cont.m_lineCost = temp;
}

void AttributeContainer::clear(bool removeAttrib)
{
	m_size = 0;
 	m_maxSize = 0;

	std::vector<HoleBlockRef*> bf;
	std::vector<unsigned int> bwf;

	// raz des cases libres
	for (std::vector<HoleBlockRef*>::iterator it = m_holesBlocks.begin(); it != m_holesBlocks.end(); ++it)
		delete (*it);

	m_holesBlocks.swap(bf);
	m_tableBlocksWithFree.swap(bwf);

	// detruit les données
	for (std::vector<AttributeMultiVectorGen*>::iterator it = m_tableAttribs.begin(); it != m_tableAttribs.end(); ++it)
	{
		if ((*it) != NULL)
			(*it)->clear();
	}

	// on enleve les attributs ?
	if (removeAttrib)
	{
		// nb a zero
		m_nbAttributes = 0;

		// detruit tous les attributs
		std::vector<AttributeMultiVectorGen*> amg;
		for (std::vector<AttributeMultiVectorGen*>::iterator it = m_tableAttribs.begin(); it != m_tableAttribs.end(); ++it)
		{
			if ((*it) != NULL)
				delete (*it);
		}
		m_tableAttribs.swap(amg);
	}
}

void AttributeContainer::compact(std::vector<unsigned int>& mapOldNew)
{
	unsigned int nbe = _BLOCKSIZE_ * m_holesBlocks.size();

	unsigned int nbb = m_holesBlocks.size() - 1;
	while ((m_holesBlocks[nbb])->empty())
	{
		--nbb;
		nbe -= _BLOCKSIZE_;
	}
	++nbb;

	mapOldNew.reserve(nbe);

	// now get the holes
	unsigned int baseAdr = 0;
	for (unsigned int i = 0; i < nbb; ++i)
	{
		HoleBlockRef* block = m_holesBlocks[i];

		for (unsigned int j = 0; j < _BLOCKSIZE_; ++j)
		{
			if (j < block->sizeTable())
			{
				if (block->used(j))
					mapOldNew.push_back(baseAdr);
				else
					mapOldNew.push_back(0xffffffff);
			}
			else
				mapOldNew.push_back(0xffffffff);
			baseAdr++;
		}
	}

	unsigned int last = mapOldNew.size() - 1;

 	for (unsigned int i = 0 ; i < last; ++i)
	{
		unsigned int val = mapOldNew[i];
		if (val == 0xffffffff)
		{
			// first find last element
			while (mapOldNew[last] == 0xffffffff)
				--last;

			// store it in the hole
			// find the blocks and indices
			unsigned int bi = i / _BLOCKSIZE_;
			unsigned int ib = i % _BLOCKSIZE_;
			unsigned int bj = last / _BLOCKSIZE_;
			unsigned int jb = last % _BLOCKSIZE_;

			//overwrite attributes
			for(unsigned int j = 0; j < m_tableAttribs.size(); ++j)
			{
				if (m_tableAttribs[j] != NULL)
					m_tableAttribs[j]->overwrite(bj, jb, bi, ib);
			}

			// overwrite emptyLine with last line in free blocks
			m_holesBlocks[bi]->overwrite(ib, m_holesBlocks[bj], jb);

			// set the map value
			mapOldNew[last] = i;
			--last;
		}
	}

	for (int i = m_holesBlocks.size() - 1; i >= 0; --i)
	{
		HoleBlockRef* ptr = m_holesBlocks[i];
		if (ptr->compressFree())
		{
			delete ptr;
			m_holesBlocks.pop_back();
		}
	}

	// maj de la table de block libre
	m_tableBlocksWithFree.clear();
	HoleBlockRef* block = m_holesBlocks.back();
	if (!block->full())
		m_tableBlocksWithFree.push_back(m_holesBlocks.size() - 1);

	// detruit les blocks de donnees inutiles
	for(unsigned int j = 0; j < m_tableAttribs.size(); ++j)
	{
		if (m_tableAttribs[j] != NULL)
			m_tableAttribs[j]->setNbBlocks(m_holesBlocks.size());
	}

	m_maxSize = (m_holesBlocks.back())->sizeTable() + (m_holesBlocks.size() - 1) * _BLOCKSIZE_;
}

/**************************************
 *          LINES MANAGEMENT          *
 **************************************/

unsigned int AttributeContainer::insertLine()
{
	// if no more rooms
	if (m_tableBlocksWithFree.empty())
	{
		HoleBlockRef* ptr = new HoleBlockRef();					// new block
		m_tableBlocksWithFree.push_back(m_holesBlocks.size());	// add its future position to block_free
		m_holesBlocks.push_back(ptr);							// and add it to block table

		for(unsigned int i = 0; i < m_tableAttribs.size(); ++i)
		{
			if (m_tableAttribs[i] != NULL)
				m_tableAttribs[i]->addBlock();					// add a block to every attribute
		}
	}

	// get the first free block index (last in vector)
	unsigned int bf = m_tableBlocksWithFree.back();
	// get the block
	HoleBlockRef* block = m_holesBlocks[bf];

	// add new element in block and compute index
	unsigned int ne = block->newRefElt(m_maxSize);
	unsigned int index = _BLOCKSIZE_ * bf + ne;

	// if no more room in block remove it from free_blocks
	if (block->full())
		m_tableBlocksWithFree.pop_back();

	++m_size;

//	initLine(index);

	return index;
}

void AttributeContainer::removeLine(unsigned int index)
{
	unsigned int bi = index / _BLOCKSIZE_;
	unsigned int j = index % _BLOCKSIZE_;

	HoleBlockRef* block = m_holesBlocks[bi];

	if (block->used(j))
	{
		if (block->full())		// block has no free elements before removal
			m_tableBlocksWithFree.push_back(bi);

		block->removeElt(j);

		--m_size;

		if (block->empty())		// block is empty after removal
			m_tableBlocksEmpty.push_back(bi);
	}
	else
		std::cerr << "Error removing non existing index " << index << std::endl;
}

/**************************************
 *            SAVE & LOAD             *
 **************************************/

bool AttributeContainer::loadXmlBWF(xmlNodePtr node)
{
	xmlChar* prop = xmlGetProp(node, BAD_CAST "nb");
	unsigned int nb = atoi((char*)prop);
	m_tableBlocksWithFree.clear();

	// charge et cree les  attributs
	for (xmlNode* x_node = node->children; x_node != NULL; x_node = x_node->next)
	{
		unsigned int ind = atoi((char*)(xmlNodeGetContent(x_node)));
		m_tableBlocksWithFree.push_back(ind);
	}
	if (m_tableBlocksWithFree.size() != nb)
	{
		std::cerr <<"Erreur lecture fichier XML incoherent"<< std::endl;
		return false;
	}
	return true;
}

bool AttributeContainer::loadXmlAN(xmlNodePtr node, unsigned int nbb)
{
	xmlChar* prop = xmlGetProp(node, BAD_CAST "nb");
//	unsigned int nb = atoi((char*)prop);
	prop = xmlGetProp(node, BAD_CAST "sv");
//	unsigned int sv = atoi((char*)prop);

	// Noooooooo!!!!
//	m_tableAttribs.resize(sv);
//	for (unsigned int i=0; i< sv; ++i)
//			m_tableAttribs[i]=NULL;

	// charge et cree les  attributs
	for (xmlNode* x_node = node->children; x_node != NULL; x_node = x_node->next)
	{
		prop = xmlGetProp(x_node, BAD_CAST "id");
//		unsigned int id = unsigned int(atoi((char*)prop);

		prop = xmlGetProp(x_node, BAD_CAST "type");
		// recupere l'attribut enregistrer par son type
		if (m_attributes_registry_map !=NULL)
		{
			std::map<std::string, RegisteredBaseAttribute*>::iterator itAtt = m_attributes_registry_map->find(std::string((char*)prop));
			if (itAtt == m_attributes_registry_map->end())
			{
				std::cout << "Skipping non registred attribute "<< std::string((char*)prop)<<std::endl;
			}
			else
			{
				RegisteredBaseAttribute* ra = itAtt->second;
				prop = xmlGetProp(x_node, BAD_CAST "name");
//				ra->addAttribute(*this, std::string((char*)prop), id);
				AttributeMultiVectorGen* amvg = ra->addAttribute(*this, std::string((char*)prop));
				amvg->setNbBlocks(nbb);
			}
		}
		else
		{
			std::cerr << "Attribute Registry non initialized"<< std::endl;
			return false;
		}
	}
//	if (m_attribNameMap.size() != nb)
//	{
//		std::cerr << "Pb lecture attributs"<< std::endl;
//		return false;
//	}
	return true;
}

bool AttributeContainer::loadXmlDL(xmlNodePtr node)
{
	// charge et cree les  attributs
	for (xmlNode* x_node = node->children; x_node != NULL; x_node = x_node->next)
	{
		// get index
		xmlChar* prop = xmlGetProp(x_node, BAD_CAST "id");
		unsigned int id = atoi((char*)prop);
		// get & set nbref
		prop = xmlGetProp(x_node, BAD_CAST "refs");
		unsigned int nbr = atoi((char*)prop);
		setNbRefs(id, nbr);

		if (nbr > 0)
		{
//			for (MapNameId::iterator it = m_attribNameMap.begin(); it != m_attribNameMap.end(); ++it)
//			{
//				prop = xmlGetProp(x_node, BAD_CAST (it->first).c_str());
//				// if name of data unkwown then error
//				if (prop == NULL)
//				{
//					std::cerr << "inconsistent xml file"<<std::endl;
//					return false;
//				}
//				m_tableAttribs[it->second]->input(id, std::string((char*)prop));
//			}
		}
	}
	return true;
}

void AttributeContainer::saveXml(xmlTextWriterPtr writer, unsigned int id)
{
	if (m_nbAttributes == 0)
		return;

	// noeud du container
	int rc = xmlTextWriterStartElement(writer, BAD_CAST "Attributes_Container");
	rc = xmlTextWriterWriteFormatAttribute(writer,  BAD_CAST "id","%u",id);
	rc = xmlTextWriterWriteFormatAttribute(writer,  BAD_CAST "BlockSize","%u",_BLOCKSIZE_);
	rc = xmlTextWriterWriteFormatAttribute(writer,  BAD_CAST "size","%u",m_maxSize);

	// recuperer le nombre d'attributs
	unsigned int nbAtt = m_nbAttributes;
	unsigned int sizeVectAtt = m_tableAttribs.size();

	// noeud avec la liste de attributs
	rc = xmlTextWriterStartElement(writer, BAD_CAST "Attributes_Names");
	rc = xmlTextWriterWriteFormatAttribute(writer,  BAD_CAST "nb","%u",nbAtt);
	rc = xmlTextWriterWriteFormatAttribute(writer,  BAD_CAST "sv","%u",sizeVectAtt);

	// recuperer les attributs dans la map et les sauver
//	for (std::map<std::string, unsigned int>::iterator it = m_attribNameMap.begin(); it!= m_attribNameMap.end(); ++it)
//	{
//		int rc = xmlTextWriterStartElement(writer, BAD_CAST "Attribute");
//		rc = xmlTextWriterWriteAttribute(writer,  BAD_CAST "name",BAD_CAST (it->first).c_str());
//		const std::string& str_type = m_tableAttribs[it->second]->getTypeName();
//		rc = xmlTextWriterWriteAttribute(writer,  BAD_CAST "type",BAD_CAST str_type.c_str());
//		rc = xmlTextWriterWriteFormatAttribute(writer,  BAD_CAST "id","%u",it->second);
//		rc = xmlTextWriterEndElement(writer);
//	}
	// fin du noeud
	rc = xmlTextWriterEndElement(writer);

	// parcourir le container et sauver les lignes
	rc = xmlTextWriterStartElement(writer, BAD_CAST "Data_Lines");
//	rc = xmlTextWriterWriteFormatAttribute(writer,  BAD_CAST "size","%u",m_maxSize);
	for (unsigned int i = 0; i != m_maxSize; ++i)
	{
		unsigned int nbr = getNbRefs(i);
		rc = xmlTextWriterStartElement(writer, BAD_CAST "Line");
		rc = xmlTextWriterWriteFormatAttribute(writer,  BAD_CAST "id","%u",i);
		rc = xmlTextWriterWriteFormatAttribute(writer,  BAD_CAST "refs","%u",nbr);
		if (nbr > 0)
		{
			// tous les attributs de la ligne
//			for (MapNameId::iterator it = m_attribNameMap.begin(); it!= m_attribNameMap.end(); ++it)
//			{
//				std::string st_att = m_tableAttribs[it->second]->output(i);
//				rc = xmlTextWriterWriteAttribute(writer,(xmlChar*)( (it->first).c_str()), (xmlChar*)( st_att.c_str()));
//			}
		}
		// fin du noeud Line
		rc = xmlTextWriterEndElement(writer);
	}
	// fin du noeud Data Lines
	rc = xmlTextWriterEndElement(writer);

	// fin du noeud Container
	rc = xmlTextWriterEndElement(writer);
}

unsigned int AttributeContainer::getIdXmlNode(xmlNodePtr node)
{
	xmlChar *prop = xmlGetProp(node, BAD_CAST "id");
	unsigned int id = atoi((char*)prop);
	return id;
}

bool AttributeContainer::loadXml(xmlNodePtr node)
{
	xmlChar *prop = xmlGetProp(node, BAD_CAST "BlockSize");
	unsigned int bs = atoi((char*)prop);

	if (bs != _BLOCKSIZE_)
	{
		std::cerr << "Chargement impossible, tailles de block differentes: "<<_BLOCKSIZE_<<" / " << bs << std::endl;
		return false;
	}

//	prop = xmlGetProp(node, BAD_CAST "id");
//	unsigned int id = atoi((char*)prop);

	prop = xmlGetProp(node, BAD_CAST "size");
	m_maxSize = atoi((char*)prop);

	char* ANnode = (char*)"Attributes_Names";
	char* DLnode= (char*)"Data_Lines";

	// calcul le nombre de block et les alloue
	unsigned int nbb = m_maxSize/_BLOCKSIZE_;
	if (m_maxSize%_BLOCKSIZE_)
			nbb++;

	m_holesBlocks.resize(nbb);
	for (unsigned int i=0; i<nbb; ++i)
		m_holesBlocks[i] = new HoleBlockRef;

	//load Attributes
	xmlNode* cur = node->children;
	while ( strcmp((char*)(cur->name),ANnode))
		cur = cur->next;
	loadXmlAN(cur,nbb);

	cur = node->children;
	while ( strcmp((char*)(cur->name),DLnode))
		cur = cur->next;
	loadXmlDL(cur);

	// recreate free holes in blocks
	nbb--;
	for (unsigned int i = 0; i < nbb; ++i)
	{
		if (m_holesBlocks[i]->updateHoles(_BLOCKSIZE_))
			m_tableBlocksWithFree.push_back(i);
	}
	m_holesBlocks[nbb]->updateHoles(m_maxSize - nbb * _BLOCKSIZE_);

	return true;
}

void AttributeContainer::saveBin(CGoGNostream& fs, unsigned int id)
{
	// en ascii id et taille les tailles

	std::vector<unsigned int> bufferui;
	bufferui.reserve(6);

	bufferui.push_back(id);
	bufferui.push_back(_BLOCKSIZE_);
	bufferui.push_back(m_holesBlocks.size());
	bufferui.push_back(m_tableBlocksWithFree.size());
	bufferui.push_back(m_nbAttributes);
	bufferui.push_back(m_size);
	bufferui.push_back(m_maxSize);

	std::cout << "Save Container: id:" <<id <<"  nbAtt:"<<m_nbAttributes << "  size:"<<m_size<<std::endl;

	fs.write(reinterpret_cast<const char*>(&bufferui[0]) ,bufferui.size()*sizeof(unsigned int));

	unsigned int i = 0;
	for(std::vector<AttributeMultiVectorGen*>::iterator it = m_tableAttribs.begin(); it != m_tableAttribs.end(); ++it)
	{
		if (*it != NULL)
			(*it)->saveBin(fs, i++);
	}

	//en binaire les blocks de ref
	for (std::vector<HoleBlockRef*>::iterator it = m_holesBlocks.begin(); it != m_holesBlocks.end(); ++it)
		(*it)->saveBin(fs);

	// les indices des blocks libres
	fs.write(reinterpret_cast<const char*>(&m_tableBlocksWithFree[0]), m_tableBlocksWithFree.size() * sizeof(unsigned int));
}

unsigned int AttributeContainer::loadBinId(CGoGNistream& fs)
{
	unsigned int id;
	fs.read(reinterpret_cast<char*>(&id), sizeof(unsigned int));
	return id;
}

bool AttributeContainer::loadBin(CGoGNistream& fs)
{
	if (m_attributes_registry_map == NULL)
	{
		std::cerr << "Attribute Registry non initialized"<< std::endl;
		return false;
	}

	std::vector<unsigned int> bufferui;
	bufferui.resize(256);

	fs.read(reinterpret_cast<char*>(&(bufferui[0])), 6*sizeof(unsigned int));

	unsigned int bs, szHB, szBWF, nbAtt;
	bs = bufferui[0];
	szHB = bufferui[1];
	szBWF = bufferui[2];
	nbAtt = bufferui[3];
	m_size = bufferui[4];
	m_maxSize = bufferui[5];

	if (bs != _BLOCKSIZE_)
	{
		std::cerr << "Chargement impossible, tailles de block differentes: "<<_BLOCKSIZE_<<" / " << bs << std::endl;
		return false;
	}

	for (unsigned int j = 0; j < nbAtt; ++j)
	{
		std::string nameAtt;
		std::string typeAtt;
		/*unsigned int id = */AttributeMultiVectorGen::loadBinInfos(fs,nameAtt, typeAtt);

		std::map<std::string, RegisteredBaseAttribute*>::iterator itAtt = m_attributes_registry_map->find(typeAtt);
		if (itAtt == m_attributes_registry_map->end())
		{
			std::cout << "Skipping non registred attribute of type name"<< typeAtt <<std::endl;
			AttributeMultiVectorGen::skipLoadBin(fs);
		}
		else
		{
			RegisteredBaseAttribute* ra = itAtt->second;
			AttributeMultiVectorGen* amvg = ra->addAttribute(*this, nameAtt);
			std::cout << "loading attribute " << nameAtt << " : " << typeAtt << std::endl;
			amvg->loadBin(fs);
			// no need the set the nb of block (done by binary read of attribmv)
			m_nbAttributes++;
		}
	}

	m_holesBlocks.resize(szHB);

	// blocks
	for (unsigned int i = 0; i < szHB; ++i)
	{
		m_holesBlocks[i] = new HoleBlockRef;
		m_holesBlocks[i]->loadBin(fs);
	}

	// les indices des blocks libres
	m_tableBlocksWithFree.resize(szBWF);
	fs.read(reinterpret_cast<char*>(&(m_tableBlocksWithFree[0])), szBWF*sizeof(unsigned int));

	return true;
}

}