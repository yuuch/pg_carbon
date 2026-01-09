#include "metadata.h"

extern "C" {
#include "access/genam.h"
#include "access/relation.h"
#include "access/table.h"
#include "nodes/pathnodes.h"
#include "optimizer/clauses.h"
#include "optimizer/cost.h" // For GUC variables
#include "utils/selfuncs.h" // For estimate_rel_size if needed, usually in relation->rd_rel

extern Selectivity clauselist_selectivity(PlannerInfo *root, List *clauses, int varRelid,
                                          JoinType jointype, SpecialJoinInfo *sjinfo);
}

// Ensure we pick up the GUC variables from postgres
// cpu_tuple_cost, etc. are global variables in Postgres.

namespace pg_carbon {

RelationStats MetadataAccessor::GetTableStats(Oid table_oid) {
    RelationStats stats = {0.0, 0.0};

    // Open relation with AccessShareLock
    Relation rel = table_open(table_oid, AccessShareLock);

    // Use values from pg_class (rd_rel)
    // Note: In a full optimizer we might want to use estimate_rel_size()
    // but accessing rd_rel directly is a safe start for now.
    if (rel->rd_rel) {
        stats.rows = (double)rel->rd_rel->reltuples;
        stats.pages = (double)rel->rd_rel->relpages;

        // Safety check for empty/new tables
        if (stats.rows < 0)
            stats.rows = 0;
        if (stats.pages < 0)
            stats.pages = 0;
    }

    table_close(rel, AccessShareLock);
    return stats;
}

IndexStats MetadataAccessor::GetIndexStats(Oid index_oid) {
    IndexStats stats = {0.0, 0.0};

    Relation index_rel = index_open(index_oid, AccessShareLock);

    if (index_rel->rd_rel) {
        stats.pages = (double)index_rel->rd_rel->relpages;
        if (stats.pages < 0)
            stats.pages = 0;
    }

    // Tree height is not directly in rd_rel.
    // B-tree logic usually gets it from metapage, but that's complex.
    // For now, assume a default or calculate log based on pages?
    // Let's just mock it or leave as 0 (for cost models that ignore it or assume 1-2).
    // The Standard planner checks metapage. We'll skip deep inspection for stability.
    stats.tree_height = 1.0;

    index_close(index_rel, AccessShareLock);
    return stats;
}

double MetadataAccessor::GetSelectivity(const Node *quals, int range_table_index) {
    if (!quals || !pg_query_)
        return 1.0;

    // Construct a minimal PlannerInfo and root to call standard estimation
    PlannerInfo *root = makeNode(PlannerInfo);
    root->parse = pg_query_;
    root->glob = makeNode(PlannerGlobal);
    root->glob->boundParams = NULL;
    root->planner_cxt = CurrentMemoryContext;

    // We need a simple_rte_array for estimation to look up relation stats
    // Size is rtable length + 1 (1-based indexing)
    int lockmode = AccessShareLock; // Safety
    int rtable_len = list_length(pg_query_->rtable);
    root->simple_rte_array = (RangeTblEntry **)palloc0((rtable_len + 1) * sizeof(RangeTblEntry *));
    root->simple_rel_array = (RelOptInfo **)palloc0((rtable_len + 1) * sizeof(RelOptInfo *));
    root->simple_rel_array_size = rtable_len + 1;

    int i = 1;
    ListCell *lc;
    foreach (lc, pg_query_->rtable) {
        RangeTblEntry *rte = (RangeTblEntry *)lfirst(lc);
        root->simple_rte_array[i] = rte;

        // Construct basic RelOptInfo
        if (rte->rtekind == RTE_RELATION) {
            RelOptInfo *rel = makeNode(RelOptInfo);
            rel->reloptkind = RELOPT_BASEREL;
            rel->relid = i; // Index in array
            rel->rtekind = rte->rtekind;
            rel->min_attr = 0;
            rel->max_attr = 0;
            rel->attr_needed = NULL;
            rel->lateral_vars = NIL;
            rel->indexlist = NIL;
            rel->statlist = NIL;

            // Get stats
            RelationStats stats = GetTableStats(rte->relid);
            rel->tuples = stats.rows;
            rel->pages = stats.pages;

            root->simple_rel_array[i] = rel;
        }
        i++;
    }

    // Wrap single qual in a list if needed, or assume it is a List of quals?
    // Carbon LogicalFilter usually holds 'quals' as a List* (implicitly AND).
    // If it's a single Node*, make a list.
    List *qual_list = NIL;
    if (IsA(quals, List)) {
        qual_list = (List *)quals;
    } else {
        qual_list = list_make1((Node *)quals);
    }

    // SpecialType = JOIN_INNER (0) usually for base rels restriction
    // JoinType = JOIN_INNER
    double selectivity =
        clauselist_selectivity(root, qual_list, range_table_index, JOIN_INNER, NULL);

    // Clean up if massive allocation? Memo allocator handles pallocs usually reset per query
    // context.

    return selectivity;
}

} // namespace pg_carbon
