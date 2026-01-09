#ifndef PG_CARBON_PHYSICAL_AGGREGATE_H
#define PG_CARBON_PHYSICAL_AGGREGATE_H

#include "operator.h"

namespace pg_carbon {

class PhysicalAggregate : public PhysicalOperator {
public:
    PhysicalAggregate(List *group_clause, Node *having_qual)
        : group_clause_(group_clause), having_qual_(having_qual) {}

    std::string ToString() const override { return "PhysicalAggregate"; }
    List *GetGroupClause() const { return group_clause_; }
    Node *GetHavingQual() const { return having_qual_; }

    double ComputeCost(Memo *memo, const PgVector<Group *> &input_groups,
                       GroupExpression *expr = nullptr) const override;

private:
    List *group_clause_;
    Node *having_qual_;
};

} // namespace pg_carbon

#endif // PG_CARBON_PHYSICAL_AGGREGATE_H
