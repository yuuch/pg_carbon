#ifndef PG_CARBON_PHYSICAL_TABLE_SCAN_H
#define PG_CARBON_PHYSICAL_TABLE_SCAN_H

#include "operator.h"

namespace pg_carbon {

class PhysicalTableScan : public PhysicalOperator {
public:
    PhysicalTableScan(Oid table_oid, Index rtindex) : table_oid_(table_oid), rtindex_(rtindex) {}

    std::string ToString() const override {
        return "PhysicalTableScan(" + std::to_string(table_oid_) + ")";
    }
    Oid GetTableOid() const { return table_oid_; }
    Index GetRtIndex() const { return rtindex_; }

    double ComputeCost(Memo *memo, const PgVector<Group *> &input_groups,
                       GroupExpression *expr = nullptr) const override;

private:
    Oid table_oid_;
    Index rtindex_;
};

} // namespace pg_carbon

#endif // PG_CARBON_PHYSICAL_TABLE_SCAN_H
