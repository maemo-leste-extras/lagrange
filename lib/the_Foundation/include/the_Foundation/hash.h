#pragma once

/** @file the_Foundation/hash.h  Hash of unsorted unique integer keys.

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
#include "class.h"

iBeginPublic

/**
 * Hash does not have ownership of the nodes. This means the nodes can be
 * any type of object as long as they are derived from HashNode.
 */
iDeclareType(Hash)
iDeclareType(HashNode)
iDeclareType(HashBucket)

typedef uint32_t iHashKey;

struct Impl_Hash {
    size_t size;
    iHashBucket *root;
};

/// Base class for nodes inserted into the hash.
struct Impl_HashNode {
    iHashNode *next;
    iHashKey key;
};

typedef void iAnyNode;

iDeclareTypeConstruction(Hash)

iBool       contains_Hash   (const iHash *, iHashKey key);
iHashNode * value_Hash      (const iHash *, iHashKey key);

void        clear_Hash  (iHash *);

iLocalDef size_t    size_Hash       (const iHash *d) { return d->size; }
iLocalDef iBool     isEmpty_Hash    (const iHash *d) { return size_Hash(d) == 0; }

/**
 * Inserts a node into the hash.
 *
 * @param node  Node to be inserted. Ownership not taken. The `key` member must
 *              be set to a valid hash key.
 *
 * @return Previous node with the same key that had to be removed from the hash to
 * make room for the new nodes. The caller should delete the node or take any other
 * necessary actions, since it is no longer part of the hash.
 */
iHashNode * insert_Hash (iHash *, iHashNode *node);

iHashNode * remove_Hash (iHash *, iHashKey key);

/** @name Iterators */
///@{
iDeclareIterator(Hash, iHash *)
iHashNode *remove_HashIterator(iHashIterator *d);
struct IteratorImpl_Hash {
    iHashNode *value;
    iHashNode *next;
    iHashBucket *bucket;
    iHash *hash;
};

iDeclareConstIterator(Hash, const iHash *)
struct ConstIteratorImpl_Hash {
    const iHashNode *value;
    const iHashBucket *bucket;
    const iHash *hash;
};
///@}

iEndPublic
