#ifndef PG_CARBON_PHYSICAL_FILTER_H
#define PG_CARBON_PHYSICAL_FILTER_H

#include "operator.h"

namespace pg_carbon {

class PhysicalFilter : public PhysicalOperator {
public:
    explicit PhysicalFilter(Node *qual) : qual_(qual) {}

    std::string ToString() const override { return "PhysicalFilter"; }
    Node *GetQual() const { return qual_; }

    double ComputeCost(Memo *memo, const PgVector<Group *> &input_groups,
                       GroupExpression *expr = nullptr) const override;

private:
    Node *qual_;
};

} // namespace pg_carbon

#endif // PG_CARBON_PHYSICAL_FILTER_H
