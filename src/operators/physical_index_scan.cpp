#include "physical_index_scan.h"
#include "../cost_model/cost_model.h"
#include "../metadata/metadata.h"
#include <cmath>

#include "../optimizer/memo/memo.h"

namespace pg_carbon {

double PhysicalIndexScan::ComputeCost(Memo * /*memo*/, const PgVector<Group *> & /*input_groups*/,
                                      GroupExpression *expr) const {
    pg_carbon::MetadataAccessor metadata;
    pg_carbon::CostModel cost_model(&metadata);

    // Get table stats
    auto table_stats = metadata.GetTableStats(table_oid_);
    // Get index stats
    auto index_stats = metadata.GetIndexStats(index_oid_);

    // Use cardinality from LogicalProperties if available (reflecting filters)
    double rows_out = 0.0;
    if (expr && expr->GetGroup() && expr->GetGroup()->GetLogicalProperties()) {
        rows_out = expr->GetGroup()->GetLogicalProperties()->GetCardinality();
    } else {
        // Fallback: Assume selectivity 1%
        rows_out = table_stats.rows * 0.01;
    }

    // Improve cost estimation by using selectivity for index pages
    double selectivity = (table_stats.rows > 0) ? (rows_out / table_stats.rows) : 0.01;
    double touched_index_pages = index_stats.pages * selectivity;
    if (touched_index_pages < 1.0)
        touched_index_pages = 1.0;

    // Also include tree height roughly (log of pages)
    if (index_stats.pages > 1) {
        touched_index_pages += std::log2(index_stats.pages);
    }

    return cost_model.CostIndexScan(rows_out, 0.0, table_stats.pages, touched_index_pages);
}

} // namespace pg_carbon
