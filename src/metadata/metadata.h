#ifndef PG_CARBON_METADATA_H
#define PG_CARBON_METADATA_H

#include "../common/memory.h"

extern "C" {
#include "access/htup_details.h"
#include "access/relation.h"
#include "postgres.h"
#include "utils/rel.h"
}

namespace pg_carbon {

// Stats for a relation
struct RelationStats {
    double rows;
    double pages;
};

// Stats for an index
struct IndexStats {
    double pages;
    double tree_height;
};

class MetadataAccessor {
public:
    virtual ~MetadataAccessor() = default;

    // Query Context
    virtual void SetQuery(Query *query) { pg_query_ = query; }

    // Table Stats
    virtual RelationStats GetTableStats(Oid table_oid);

    // Index Stats
    virtual IndexStats GetIndexStats(Oid index_oid);

    // Selectivity Estimation
    virtual double GetSelectivity(const Node *quals, int range_table_index);

private:
    Query *pg_query_ = nullptr;
};

} // namespace pg_carbon

#endif // PG_CARBON_METADATA_H
