/*
 * iterator/iter_fwd.h - iterative resolver module forward zones.
 *
 * Copyright (c) 2007, NLnet Labs. All rights reserved.
 *
 * This software is open source.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * 
 * Neither the name of the NLNET LABS nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * \file
 *
 * This file contains functions to assist the iterator module.
 * Keep track of forward zones, and read those from config.
 */

#ifndef ITERATOR_ITER_FWD_H
#define ITERATOR_ITER_FWD_H
#include "util/rbtree.h"
struct config_file;
struct delegpt;
struct regional;

/**
 * Iterator forward zones structure
 */
struct iter_forwards {
	/** regional where forward zone server addresses are allocated */
	struct regional* region;
	/** 
	 * Zones are stored in this tree. Sort order is specially chosen.
	 * first sorted on qtype. Then on dname in nsec-like order, so that
	 * a lookup on class, name will return an exact match or the closest
	 * match which gives the ancestor needed.
	 * contents of type iter_forward_zone.
	 */
	rbtree_t* tree;
};

/**
 * Iterator forward servers for a particular zone.
 */
struct iter_forward_zone {
	/** redblacktree node, key is this structure: class and name */
	rbnode_t node;
	/** name */
	uint8_t* name;
	/** length of name */
	size_t namelen;
	/** number of labels in name */
	int namelabs;
	/** delegation point with forward server information for this zone. 
	 * If NULL then this forward entry is used to indicate that a
	 * stub-zone with the same name exists, and should be used. */
	struct delegpt* dp;
	/** pointer to parent in tree (or NULL if none) */
	struct iter_forward_zone* parent;
	/** class. host order. */
	uint16_t dclass;
};

/**
 * Create forwards 
 * @return new forwards or NULL on error.
 */
struct iter_forwards* forwards_create();

/**
 * Delete forwards.
 * @param fwd: to delete.
 */
void forwards_delete(struct iter_forwards* fwd);

/**
 * Process forwards config.
 * @param fwd: where to store.
 * @param cfg: config options.
 * @return 0 on error.
 */
int forwards_apply_cfg(struct iter_forwards* fwd, struct config_file* cfg);

/**
 * Find forward zone information
 * For this qname/qclass find forward zone information, returns delegation
 * point with server names and addresses, or NULL if no forwarding is needed.
 *
 * @param fwd: forward storage.
 * @param qname: The qname of the query.
 * @param qclass: The qclass of the query.
 * @return: A delegation point if the query has to be forwarded to that list,
 *         otherwise null.
 */
struct delegpt* forwards_lookup(struct iter_forwards* fwd, 
	uint8_t* qname, uint16_t qclass);

/**
 * Get memory in use by forward storage
 * @param fwd: forward storage.
 * @return bytes in use
 */
size_t forwards_get_mem(struct iter_forwards* fwd);

/** compare two fwd entries */
int fwd_cmp(const void* k1, const void* k2);

/**
 * Add zone to forward structure. For external use since it recalcs 
 * the tree parents.
 * @param fwd: the forward data structure
 * @param c: class of zone
 * @param dp: delegation point with name and target nameservers for new
 *	forward zone. This delegation point and all its data must be
 *	malloced in the fwd->region. (then it is freed when the fwd is
 *	deleted).
 * @return false on failure (out of memory);
 */
int forwards_add_zone(struct iter_forwards* fwd, uint16_t c, 
	struct delegpt* dp);

/**
 * Remove zone from forward structure. For external use since it 
 * recalcs the tree parents. Does not actually release any memory, the region 
 * is unchanged.
 * @param fwd: the forward data structure
 * @param c: class of zone
 * @param nm: name of zone (in uncompressed wireformat).
 */
void forwards_delete_zone(struct iter_forwards* fwd, uint16_t c, uint8_t* nm);

#endif /* ITERATOR_ITER_FWD_H */
