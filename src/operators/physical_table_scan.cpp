#include "physical_table_scan.h"
#include "../cost_model/cost_model.h"
#include "../metadata/metadata.h"

namespace pg_carbon {

double PhysicalTableScan::ComputeCost(Memo * /*memo*/, const PgVector<Group *> & /*input_groups*/,
                                      GroupExpression * /*expr*/) const {
    pg_carbon::MetadataAccessor metadata;
    pg_carbon::CostModel cost_model(&metadata);

    // Get stats
    auto stats = metadata.GetTableStats(table_oid_);

    // Compute Cost
    return cost_model.CostSeqScan(stats.rows, 0.0, stats.pages);
}

} // namespace pg_carbon
