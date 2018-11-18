// FFT3DGPU plugin for Avisynth 2.5
// Copyright(C)2005 Tonny Petersen (tsp@person.dk) 
// Based on FFT3DFilter plugin for Avisynth 2.5 - 3D Frequency Domain filter
// Copyright(C)2004-2005 A.G.Balakhnin aka Fizick, email: bag@hotmail.ru, web: http://bag.hotmail.ru
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA, or visit
// http://www.gnu.org/copyleft/gpl.html .



#include "TexturePool.h"
#include "./core/Debug class.h"

#include "GPUCache.h"

GPUCache* GPUCache::first=0;

/*
 * GPUCache
 *
 *		Constructor of the cache.
 *
 * Inputs:
 *		cachesize:[in] number of objects to cache
 * Returns:
 *     None
 *
 *	Remarks:
 *		the cache is just a linked list of GPUCacheNode each containing an objech to cache
 *		It has this structure List->1->2->3->4 where the number is the age of the object(1=newest)
 */
GPUCache::GPUCache(int cachesize)
	:_maxcachesize(cachesize),_cachesize(0),List(0)
{
	if(first){
        first->prev->next=this;
		this->prev=first->prev;
		this->next=first;
		first->prev=this;
	}
	else
	{
		first=this;
		this->prev=this;
		this->next=this;
	}
}

/*
 * AddtoCache
 *
 *		Adds object to cache, using ID n
 *
 * Inputs:
 *		n:[in] ID of object
 *		stream:[in] object to add
 * Returns:
 *     None
 *
 *	Remarks:
 *		
 */
void GPUCache::AddtoCache(int n,TextureRT* stream){
	//List->1->2->3
    GPUCacheNode* temp=List;	//Temp->1->2->3
	List=NEW GPUCacheNode(stream,n,_StreamPool);//List->0
	List->next(temp);//List->0->1->2->3
	//if list length> cachesize
	if(_cachesize==_maxcachesize)
		deletelast(List);//delete last
	else
		_cachesize++;//else inc size
}

void GPUCache::AddtoCache(int n,pTextureRTpair* stream){
	//List->1->2->3
    GPUCacheNode* temp=List;	//Temp->1->2->3
	List=NEW GPUCacheNode(stream->first,stream->last,n,_StreamPool);//List->0
	List->next(temp);//List->0->1->2->3
	//if list length> cachesize
	if(_cachesize==_maxcachesize)
		deletelast(List);//delete last
	else
		_cachesize++;//else inc size
}
/*
 * deletelast
 *
 *		deletes the oldest object in the cache by recursively traveling the linked list
 *
 * Inputs:
 *		node:[in] node to test.
 * Returns:
 *     true if node=last node 
 *		else false
 *
 *	Remarks:
 *		
 */
bool GPUCache::deletelast(GPUCacheNode* node){
	if(node->next()==0){
		if(List==node)
			List=0;
		delete node;
        return true;
	}
	if(deletelast(node->next()))
		node->next(0);
	return false;
	
}

/*
 * deleteall
 *
 *		deletes all nodes from node and forward
 *
 * Inputs:
 *		node:[in] node to delete from.
 * Returns:
  *
 *	Remarks:
 *		
 */
void GPUCache::deleteall(GPUCacheNode* node){
	if(!node)
		return;
	deleteall(node->next());
	delete node;
}

/*
 * GetStream
 *
 *		get object from cache (don't delete it)
 *
 * Inputs:
 *		n:[in] object ID to get
 * Returns:
 *		object
 *	Remarks:
 *		
 */

bool GPUCache::GetStream(int n,pTextureRTpair* texture){
	GPUCacheNode* temp=List;
	if(texture->first)
		texture->first->Release();
	if(texture->last)
		texture->last->Release();
	while(temp!=0){
		if(temp->n()==n){
			temp->stream(texture->first,texture->last);
            texture->first->AddRef();
			texture->last->AddRef();
			return true;
		}
	temp=temp->next();
	}
	texture->first=0;
	texture->last=0;
	return false;
}

bool GPUCache::GetStream(int n,TextureRT* &texture){
	GPUCacheNode* temp=List;
	if(texture)
		texture->Release();
	while(temp!=0){
		if(temp->n()==n){
			temp->stream(texture);
            texture->AddRef();
			return true;
		}
	temp=temp->next();
	}
	texture=0;
	return false;
}



void GPUCache::FlushCache()
{
	deleteall(List);
	List=0;
	_cachesize=0;
}


void GPUCache::FlushAll()
{
	if(!first)
		return;
	GPUCache* temp=first;
	do
	{
		temp->FlushCache();
		temp=temp->next;
	}
	while(temp!=first);
}


GPUCache::~GPUCache(){deleteall(List);
if(next==this)
first=0;
else
{
	if(first==this)
		first=next;
	prev->next=next;
	next->prev=prev;
}
}