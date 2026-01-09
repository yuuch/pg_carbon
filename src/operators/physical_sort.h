#ifndef PG_CARBON_PHYSICAL_SORT_H
#define PG_CARBON_PHYSICAL_SORT_H

#include "operator.h"

namespace pg_carbon {

class PhysicalSort : public PhysicalOperator {
public:
    explicit PhysicalSort(List *sort_clause) : sort_clause_(sort_clause) {}

    std::string ToString() const override { return "PhysicalSort"; }
    List *GetSortClause() const { return sort_clause_; }

    double ComputeCost(Memo *memo, const PgVector<Group *> &input_groups,
                       GroupExpression *expr = nullptr) const override;

private:
    List *sort_clause_;
};

} // namespace pg_carbon

#endif // PG_CARBON_PHYSICAL_SORT_H
