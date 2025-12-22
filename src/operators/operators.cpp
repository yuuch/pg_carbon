#include "operators.h"
#include "../optimizer/memo.h"

extern "C" {
#include "access/relation.h"
#include "access/table.h"
#include "catalog/pg_attribute.h"
#include "postgres.h"
#include "utils/lsyscache.h"
#include "utils/rel.h"
}

namespace pg_carbon {

// --- LogicalGet ---

LogicalProperties *
LogicalGet::DeriveLogicalProps(Memo *memo,
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

// --- Other Logical Operators (Pass-through or Union) ---

LogicalProperties *LogicalInnerJoin::DeriveLogicalProps(
    Memo *memo, const PgVector<Group *> &input_groups) const {
  // Join: Union of child output columns.
  ColSet output_columns;
  double cardinality = 1000.0;

  for (Group *child_group : input_groups) {
    if (!child_group)
      continue;

    LogicalProperties *child_props = child_group->GetLogicalProperties();
    if (child_props) {
      output_columns.Union(child_props->GetOutputColumns());
    }
  }

  return new LogicalProperties(std::move(output_columns), cardinality);
}

LogicalProperties *
LogicalFilter::DeriveLogicalProps(Memo *memo,
                                  const PgVector<Group *> &input_groups) const {
  // Filter: Preserves input columns of the single child.
  if (input_groups.empty())
    return new LogicalProperties(ColSet(), 0.0);

  Group *child = input_groups[0];
  if (child && child->GetLogicalProperties()) {
    const auto *child_props = child->GetLogicalProperties();
    double cardinality =
        child_props->GetCardinality() * 0.5; // Default selectivity
    ColSet output_columns(child_props->GetOutputColumns());
    return new LogicalProperties(std::move(output_columns), cardinality);
  }
  return new LogicalProperties(ColSet(), 0.0);
}

LogicalProperties *LogicalProjection::DeriveLogicalProps(
    Memo *memo, const PgVector<Group *> &input_groups) const {
  // Projection: Defines new output columns based on TargetList.
  ColSet output_columns;

  if (target_list_) {
    ListCell *lc;
    foreach (lc, target_list_) {
      TargetEntry *tle = (TargetEntry *)lfirst(lc);
      // Create ExprColumn for the expression in TLE
      auto *col = new ExprColumn((Node *)tle->expr);
      int col_id = memo->AddColumn(col);
      output_columns.Add(col_id);
    }
  }

  // Projection preserves cardinality
  double cardinality = 0.0;

  if (!input_groups.empty() && input_groups[0]->GetLogicalProperties()) {
    cardinality = input_groups[0]->GetLogicalProperties()->GetCardinality();
  }

  return new LogicalProperties(std::move(output_columns), cardinality);
}

LogicalProperties *
LogicalSort::DeriveLogicalProps(Memo *memo,
                                const PgVector<Group *> &input_groups) const {
  // Sort: Preserves input columns.
  if (input_groups.empty())
    return new LogicalProperties(ColSet(), 0.0);

  Group *child = input_groups[0];
  if (child && child->GetLogicalProperties()) {
    const auto *child_props = child->GetLogicalProperties();
    return new LogicalProperties(ColSet(child_props->GetOutputColumns()),
                                 child_props->GetCardinality());
  }
  return new LogicalProperties(ColSet(), 0.0);
}

LogicalProperties *
LogicalLimit::DeriveLogicalProps(Memo *memo,
                                 const PgVector<Group *> &input_groups) const {
  // Limit: Preserves input columns.
  if (input_groups.empty())
    return new LogicalProperties(ColSet(), 0.0);

  Group *child = input_groups[0];
  if (child && child->GetLogicalProperties()) {
    const auto *child_props = child->GetLogicalProperties();
    // Limit changes cardinality
    double cardinality = 10.0; // Mock limit
    return new LogicalProperties(ColSet(child_props->GetOutputColumns()),
                                 cardinality);
  }
  return new LogicalProperties(ColSet(), 0.0);
}

} // namespace pg_carbon
