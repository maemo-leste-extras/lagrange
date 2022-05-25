#pragma once

/** @file the_Foundation/map.h  Map of sorted unique integer nodes.

Map is implemented as a red-black tree, i.e., a balanced BST.

Map does not have ownership of the nodes. This means the nodes can be any type of object
as long as they are derived from MapNode.

@authors Copyright (c) 2017 Jaakko Keränen <jaakko.keranen@iki.fi>

@par License

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

<small>THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.</small>
*/

#include "defs.h"
#include "list.h"
#include "class.h"

iBeginPublic

iDeclareType(Map)
iDeclareType(MapNode)
iDeclareType(Block)

typedef int64_t iMapKey;
typedef int (*iMapNodeCmpFunc)(iMapKey, iMapKey);

struct Impl_Map {
    size_t size;
    iMapNode *root;
    iMapNodeCmpFunc cmp;
};

/// Nodes inserted to the map must be based on iMapNode.
struct Impl_MapNode {
    iMapNode *parent;
    iMapNode *child[2];
    int flags;
    iMapKey key;
};

iDeclareTypeConstructionArgs(Map, iMapNodeCmpFunc cmp)

iLocalDef size_t    size_Map    (const iMap *d) { return d->size; }
iLocalDef iBool     isEmpty_Map (const iMap *d) { return size_Map(d) == 0; }

iBool       contains_Map    (const iMap *, iMapKey key);
iMapNode *  value_Map       (const iMap *, iMapKey key);

void        clear_Map   (iMap *);

/**
 * Inserts a node into the map.
 *
 * @param node  Node to be inserted. Ownership not taken. The `key` member must
 *              be set to the key value.
 *
 * @return Previous node with the same key that had to be removed from the hash to
 * make room for the new node. The caller should delete the node or take any other
 * necessary actions, since it is no longer part of the hash.
 */
iMapNode *  insert_Map      (iMap *, iMapNode *node);

iMapNode *  remove_Map      (iMap *, iMapKey key);

iMapNode *  removeNode_Map  (iMap *, iMapNode *node);

/** @name Iterators */
///@{
iDeclareIterator(Map, iMap *)
iMapNode *remove_MapIterator(iMapIterator *d);
iMapNode *remove_MapReverseIterator(iMapReverseIterator *d);
struct IteratorImpl_Map {
    iMapNode *value;
    iMapNode *next;
    iMap *map;
};

iDeclareConstIterator(Map, const iMap *)
struct ConstIteratorImpl_Map {
    const iMapNode *value;
    const iMap *map;
};
///@}

iEndPublic
