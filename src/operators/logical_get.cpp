#include "logical_get.h"
#include "../optimizer/memo/memo.h"

extern "C" {
#include "access/relation.h"
#include "access/table.h"
#include "catalog/pg_attribute.h"
#include "postgres.h"
#include "utils/lsyscache.h"
#include "utils/rel.h"
}

namespace pg_carbon {

LogicalProperties *LogicalGet::DeriveLogicalProps(Memo *memo,
                                                  const PgVector<Group *> &input_groups) const {
    // For a Leaf Node (Scan), we get columns from the Catalog.
    ColSet output_columns;

    // Open relation to get tuple descriptor
    Relation rel = table_open(table_oid_, AccessShareLock);
    TupleDesc tupdesc = RelationGetDescr(rel);

    for (int i = 0; i < tupdesc->natts; i++) {
        Form_pg_attribute attr = TupleDescAttr(tupdesc, i);

        // Skip dropped columns
        if (attr->attisdropped)
            continue;

        // Create a TableColumn
        auto *col = new TableColumn(table_oid_, rtindex_, attr->attnum);

        // Allow column to outlive this function (Memo takes ownership)
        // Note: In real system we might use a pool or smart pointers.
        // Here we rely on Memo's vector to hold pointers.
        int col_id = memo->AddColumn(col);
        output_columns.Add(col_id);
    }

    table_close(rel, AccessShareLock);

    // Default cardinality for leaf
    double cardinality = 1000.0; // Simple default

    return new LogicalProperties(std::move(output_columns), cardinality);
}

} // namespace pg_carbon
