#ifndef __WALK_ENCODER_H__
#define __WALK_ENCODER_H__

#include "stdafx.h"

/******************************************************************************
  A walk is encoded by a 64-bit integer.

  0        8        16       24        32
  +--------+--------+--------+---------+
  | offset |          source           |
  +--------+--------+--------+-------+-+
  |        id       |   hop  |       | | <-- flag
  +-----------------+--------+-------+-+
                                       64

  Then sorting bucket by offset is equivalent to sort by destination nodes.
******************************************************************************/
class Encoder {
public:
    /**
     * Encoding a walk.
     *
     * Note, as sourceIdx is the highest order bits, the walks can be sorted by
     * source simply by sorting the list.
     *
     * source: starting node of the walks
     * hop: true if odd, false if even
     * offset: bucket offset
     */
    static long encode(const int offset, const int source, const int id,
                       const int hop = 0) {
        long walk = ((offset & 0xff) << 24) | (source & 0xffffff);
        walk = (walk << 16) | (id & 0xffff);
        walk = (walk << 8) | (hop & 0xff);
        return walk << 8;
    }

    static inline int getOffset(const long walk) { return walk >> 56 & 0xff; }

    static inline int getSource(const long walk) {
        return walk >> 32 & 0xffffff;
    }

    static inline int getId(const long walk) { return walk >> 16 & 0xffff; }

    static inline int getHop(const long walk) { return walk >> 8 & 0xff; }

    static inline bool isFlagged(const long walk) { return walk & 0x01; }

    // Resets the bucket offset to reflect the new destination vertex, and also
    // resets the track bit, according to the parameters.
    static long upcode(const long walk, const int to_node) {
        return encode(GET_OFFSET(to_node), getSource(walk), getId(walk),
                      getHop(walk) + 1);
    }

    static inline long wipeHop(const long walk) {
        return walk & 0xffffffffffff00ff;
    }

    static inline long addHop(const long walk, const int hop) {
        return walk | (hop & 0xff) << 8;
    }

    /**
     * return src:id
     */
    static inline long getWalkId(const long walk) {
        return walk & 0xffffffffff0000;
    }

    static inline long flag(const long walk) { return walk | 0x01; }

    static inline long unflag(const long walk) {
        return walk & 0xfffffffffffffffe;
    }

    /**
     * Return the node where the walker current resides
     */
    static int getNode(const int bucket, const long walk) {
        return FIRST_NODE_IN_BUCKET(bucket) + getOffset(walk);
    }

    static void decode(const long walk) {
        printf("offset: %d, source: %d, id: %d, hop: %d\n", getOffset(walk),
               getSource(walk), getId(walk), getHop(walk));
    }
};

#endif /* __WALK_ENCODER_H__ */
