/** @file blockhash.c  Hash that uses Block for keys and Object for values.

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

#include "the_Foundation/blockhash.h"
#include "the_Foundation/garbage.h"

#include <stdlib.h>
#include <stdarg.h>

iBlockHashNode *new_BlockHashNode(const iBlock *key, const iAnyObject *object) {
    iBlockHashNode *d = iMalloc(BlockHashNode);
    initCopy_Block(&d->keyBlock, key);
    d->object = ref_Object(object);
    return d;
}

void deinit_BlockHashNode(iBlockHashNode *d) {
    if (d) {
        deinit_Block(key_BlockHashNode(d));
        deref_Object(d->object);
    }
}

iHashKey hashKey_BlockHashNode(const iBlock *key) {
    return crc32_Block(key);
}

/*-------------------------------------------------------------------------------------*/

iDefineObjectConstruction(BlockHash)

void init_BlockHash(iBlockHash *d) {
    init_Hash(&d->hash);
    setNodeClass_BlockHash(d, &Class_BlockHashNode);
}

void deinit_BlockHash(iBlockHash *d) {
    clear_BlockHash(d); // delete everything
    deinit_Hash(&d->hash);
}

void setNodeClass_BlockHash(iBlockHash *d, const iBlockHashNodeClass *class) {
    d->nodeClass = class;
}

iBool contains_BlockHash(const iBlockHash *d, const iBlock *key) {
    return contains_Hash(&d->hash, d->nodeClass->hashKey(key));
}

const iAnyNode *constValue_BlockHash(const iBlockHash *d, const iBlock *key) {
    const iBlockHashNode *node = (const void *) value_Hash(&d->hash, d->nodeClass->hashKey(key));
    return (node? node->object : NULL);
}

iAnyNode *value_BlockHash(iBlockHash *d, const iBlock *key) {
    iBlockHashNode *node = (void *) value_Hash(&d->hash, d->nodeClass->hashKey(key));
    return (node? node->object : NULL);
}

void clear_BlockHash(iBlockHash *d) {
    iForEach(BlockHash, i, d) {
        remove_BlockHashIterator(&i);
    }
    clear_Hash(&d->hash);
}

iBool insert_BlockHash(iBlockHash *d, const iBlock *key, const iAnyObject *value) {
    /*
#if !defined (NDEBUG)
    iDebug("[%s] inserting (%zu)[", d->object.class->name, size_Block(key));
    for (size_t i = 0; i < size_Block(key); ++i) {
        iDebug(" %02x", (uint8_t) at_Block(key, i));
    }
    iDebug(" ] => %s %p\n", class_Object(value)->name, value);
#endif
    */
    iHashNode *node = (iHashNode *) d->nodeClass->newNode(key, value);
    node->key = d->nodeClass->hashKey(key);
    iAnyNode *old = insert_Hash(&d->hash, node);
    if (old) {
        delete_Class(d->nodeClass, old);
        return iFalse;
    }
    return iTrue;
}

void insertValues_BlockHash(iBlockHash *d, const iBlock *key, const iAnyObject *value, ...) {
    insert_BlockHash(d, key, value);
    va_list args;
    for (va_start(args, value);;) {
        key = va_arg(args, const iBlock *);
        if (!key) break;
        insert_BlockHash(d, key, va_arg(args, const iAnyObject *));
    }
    va_end(args);
}

void insertValuesCStr_BlockHash(iBlockHash *d, const char *key, const iAnyObject *value, ...) {
    iBeginCollect();
    insert_BlockHash(d, iClob(newCStr_Block(key)), value);
    va_list args;
    for (va_start(args, value);;) {
        key = va_arg(args, const char *);
        if (!key) break;
        insert_BlockHash(d, iClob(newCStr_Block(key)), va_arg(args, const iAnyObject *));
    }
    va_end(args);
    iEndCollect();
}

iBool remove_BlockHash(iBlockHash *d, const iBlock *key) {
    iHashNode *old = remove_Hash(&d->hash, d->nodeClass->hashKey(key));
    if (old) {
        delete_Class(d->nodeClass, old);
        return iTrue;
    }
    return iFalse;
}

/*-------------------------------------------------------------------------------------*/

void init_BlockHashIterator(iBlockHashIterator *d, iBlockHash *hash) {
    init_HashIterator(&d->iter, &hash->hash);
    d->blockHash = hash;
    d->value = (iBlockHashNode *) d->iter.value;
}

void next_BlockHashIterator(iBlockHashIterator *d) {
    next_HashIterator(&d->iter);
    d->value = (iBlockHashNode *) d->iter.value;
}

const iBlock *key_BlockHashIterator(iBlockHashIterator *d) {
    return key_BlockHashNode(d->value);
}

void remove_BlockHashIterator(iBlockHashIterator *d) {
    iAssert(d->blockHash->nodeClass);
    delete_Class(d->blockHash->nodeClass, remove_HashIterator(&d->iter));
}

void init_BlockHashConstIterator(iBlockHashConstIterator *d, const iBlockHash *hash) {
    init_HashConstIterator(&d->iter, &hash->hash);
    d->value = (const iBlockHashNode *) d->iter.value;
}

void next_BlockHashConstIterator(iBlockHashConstIterator *d) {
    next_HashConstIterator(&d->iter);
    d->value = (const iBlockHashNode *) d->iter.value;
}

const iBlock *key_BlockHashConstIterator(iBlockHashConstIterator *d) {
    return key_BlockHashNode(d->value);
}

iDefineClass(BlockHash)

iBeginDefineClass(BlockHashNode)
    .newNode = new_BlockHashNode,
    .hashKey = hashKey_BlockHashNode,
iEndDefineClass(BlockHashNode)
