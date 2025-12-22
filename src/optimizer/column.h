#ifndef PG_CARBON_COLUMN_H
#define PG_CARBON_COLUMN_H

#include "../common/memory.h"
#include <string>

// clang-format off
extern "C" {
#include "postgres.h"
#include "nodes/primnodes.h"
}
// clang-format on

namespace pg_carbon {

enum class CarbonColumnType { TABLE_COLUMN, EXPR_COLUMN, UNKNOWN };

class CarbonColumn : public PgObject {
public:
  virtual ~CarbonColumn() = default;
  virtual CarbonColumnType GetType() const = 0;
  virtual std::string ToString() const = 0;

  // Unique ID assigned by Memo when registered
  void SetId(int id) { id_ = id; }
  int GetId() const { return id_; }

private:
  int id_ = -1;
};

class TableColumn : public CarbonColumn {
public:
  TableColumn(Oid table_oid, Index rt_index, AttrNumber attr_num)
      : table_oid_(table_oid), rt_index_(rt_index), attr_num_(attr_num) {}

  CarbonColumnType GetType() const override {
    return CarbonColumnType::TABLE_COLUMN;
  }

  std::string ToString() const override {
    return "TableColumn(OID=" + std::to_string(table_oid_) +
           ", RT=" + std::to_string(rt_index_) +
           ", Attr=" + std::to_string(attr_num_) + ")";
  }

  Oid GetTableOid() const { return table_oid_; }
  Index GetRtIndex() const { return rt_index_; }
  AttrNumber GetAttrNum() const { return attr_num_; }

private:
  Oid table_oid_;
  Index rt_index_;
  AttrNumber attr_num_;
};

class ExprColumn : public CarbonColumn {
public:
  explicit ExprColumn(Node *expr) : expr_(expr) {}

  CarbonColumnType GetType() const override {
    return CarbonColumnType::EXPR_COLUMN;
  }

  std::string ToString() const override {
    return "ExprColumn"; // Simplified for now
  }

  Node *GetExpr() const { return expr_; }

private:
  Node *expr_;
};

} // namespace pg_carbon

#endif // PG_CARBON_COLUMN_H
