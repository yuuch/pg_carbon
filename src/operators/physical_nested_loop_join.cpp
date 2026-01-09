#include "physical_nested_loop_join.h"
#include "../cost_model/cost_model.h"
#include "../metadata/metadata.h"
#include "../optimizer/memo/memo.h"

namespace pg_carbon {

double PhysicalNestedLoopJoin::ComputeCost(Memo * /*memo*/, const PgVector<Group *> &input_groups,
                                           GroupExpression * /*expr*/) const {
    double outer_cost = 0.0;
    double inner_cost = 0.0;
    double outer_rows = 1000.0; // Default
    double inner_rows = 1000.0; // Default

    if (input_groups.size() >= 2) {
        if (auto *outer_best = input_groups[0]->GetBestExpression())
            outer_cost = outer_best->GetCost();
        if (auto *inner_best = input_groups[1]->GetBestExpression())
            inner_cost = inner_best->GetCost();

        if (input_groups[0]->GetLogicalProperties())
            outer_rows = input_groups[0]->GetLogicalProperties()->GetCardinality();
        if (input_groups[1]->GetLogicalProperties())
            inner_rows = input_groups[1]->GetLogicalProperties()->GetCardinality();
    }

    pg_carbon::MetadataAccessor metadata;
    pg_carbon::CostModel cost_model(&metadata);
    return cost_model.CostNestedLoopJoin(outer_rows, outer_cost, inner_rows, inner_cost);
}

} // namespace pg_carbon
