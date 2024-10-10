#ifndef ADIOS_HPP
#define ADIOS_HPP

#include <string>
#include <vector>
#include <iostream>

namespace adios2
{
// Need std::vector<Box<Dims>>
// From adios2/common/ADIOSTypes.h
using Dims = std::vector<size_t>;
template <class T>
using Box = std::pair<T, T>;
// From adios2/core/ADIOS.h
// Form adios2/core/Engine.h
// Form adios2/core/IO.h
// Form adios2/core/Variable.h
// From source/adios2/toolkit/query/Query.h
namespace query
{
    
enum Op
{
    GT,
    LT,
    GE,
    LE,
    NE,
    EQ
};

enum Relation
{
    AND,
    OR,
    NOT
};

adios2::query::Relation strToRelation(std::string relationStr) noexcept;

adios2::query::Op strToQueryOp(std::string opStr) noexcept;

adios2::Dims split(const std::string &s, char delim);

//
// classes
//
class Range
{
public:
    adios2::query::Op m_Op;
    std::string m_StrValue;
    // void* m_Value = nullptr;

    // template<class T> bool Check(T val) const ;

    template <class T>
    bool CheckInterval(T &min, T &max) const;

  //void Print() { std::cout << "===> " << m_StrValue << std::endl; }
  // MODIFIED PRINT CODE
  void Print(std::string indent)
  {
    std::cout << indent << "===> " << m_StrValue << " (op ";
    switch (m_Op)
      {
      case adios2::query::Op::LT:
	std::cout << "<";
	break;
      case adios2::query::Op::LE:
	std::cout << "<=";
	break;
      case adios2::query::Op::GT:
	std::cout << ">";
	break;
      case adios2::query::Op::GE:
	std::cout << ">=";
	break;
      case adios2::query::Op::EQ:
	std::cout << "==";
	break;
      case adios2::query::Op::NE:
	std::cout << "!=";
	break;
      default:
	std::cout << "unknown";
      }

    std::cout << ")" << std::endl;
  }
}; // class Range

class RangeTree
{
public:
    void AddLeaf(adios2::query::Op op, std::string value)
    {
        Range range;
        range.m_Op = op;
        range.m_StrValue = value;

        m_Leaves.push_back(range);
    }

    void AddNode(RangeTree &node) { m_SubNodes.push_back(node); }

    void SetRelation(adios2::query::Relation r) { m_Relation = r; }

  /*
    void Print()
    {
        for (auto leaf : m_Leaves)
            leaf.Print();
        for (auto node : m_SubNodes)
            node.Print();
    }
  */
  // MODIFIED PRINT CODE
  void Print(std::string indent)
    {
        for (auto leaf : m_Leaves)
            leaf.Print(indent);
        for (auto node : m_SubNodes)
            node.Print(indent);
    }

    // template<class T>  bool Check(T value) const ;

    template <class T>
    bool CheckInterval(T &min, T &max) const;

    adios2::query::Relation m_Relation = adios2::query::Relation::AND;
    std::vector<Range> m_Leaves;
    std::vector<RangeTree> m_SubNodes;
}; // class RangeTree

struct BlockHit
{
    BlockHit(size_t id);
    BlockHit(size_t id, Box<Dims> &box);
    BlockHit(const BlockHit &cpy);

    size_t m_ID;

    // if no sublocks, m_Regions is start/count of block (if global array). size=1
    // with subblocks,
    //    if global array, m_Regions has all the touched sub blocks with (abs) start count
    //    if local array, because client needs to read whole block, subblocks is ignored
    //       size=0
    // items in this vector are assumed to have no intersection.
    std::vector<Box<Dims>> m_Regions;

    bool isLocalArrayBlock() const { return (0 == m_Regions.size()); }
    bool applyIntersection(const BlockHit &tmp);
    bool applyExtension(const BlockHit &tmp);
};

class QueryBase
{
public:
    virtual ~QueryBase(){};
    virtual bool IsCompatible(const adios2::Box<adios2::Dims> &box) = 0;
  //virtual void Print() = 0;
  // MODIFIED PRINT CODE
  virtual void Print(std::string indent) = 0;
  /*
    virtual void BlockIndexEvaluate(adios2::core::IO &, adios2::core::Engine &,
                                    std::vector<BlockHit> &touchedBlocks) = 0;
  */

    static Box<Dims> GetIntersection(const Box<Dims> &box1, const Box<Dims> &box2) noexcept
    {
      /*
        Box<Dims> b1 = adios2::helper::StartEndBox(box1.first, box1.second);
        Box<Dims> b2 = adios2::helper::StartEndBox(box2.first, box2.second);

        Box<Dims> result = adios2::helper::IntersectionBox(b1, b2);
        return adios2::helper::StartCountBox(result.first, result.second);
      */
      Box<Dims> ret;
      return ret;
    }

    bool UseOutputRegion(const adios2::Box<adios2::Dims> &region)
    {
        if (!IsCompatible(region))
            return false;

        m_OutputRegion = region;
        BroadcastOutputRegion(region);
        return true;
    }

    virtual void BroadcastOutputRegion(const adios2::Box<adios2::Dims> &region) = 0;

    void ApplyOutputRegion(std::vector<Box<Dims>> &touchedBlocks,
                           const adios2::Box<Dims> &referenceRegion);

    adios2::Box<adios2::Dims> m_OutputRegion;

private:
    // bool ResetToOutputRegion(Box<Dims>& block);
};

class QueryVar : public QueryBase
{
public:
    QueryVar(const std::string &varName) : m_VarName(varName) {}
    ~QueryVar() {}

    std::string &GetVarName() { return m_VarName; }
  /*
    void BlockIndexEvaluate(adios2::core::IO &, adios2::core::Engine &,
                            std::vector<BlockHit> &touchedBlocks);
  */

    void BroadcastOutputRegion(const adios2::Box<adios2::Dims> &region) { m_OutputRegion = region; }

  //void Print() { m_RangeTree.Print(); }
  // MODIFIED PRINT CODE
  void Print(std::string indent)
  {
    std::cout << indent << "- QueryVar (" << m_VarName << "):" << std::endl;
    m_RangeTree.Print(indent);
  }

    bool IsCompatible(const adios2::Box<adios2::Dims> &box)
    {
        if ((m_Selection.first.size() == 0) || (box.first.size() == 0))
            return true;

        if (box.first.size() != m_Selection.first.size())
            return false;

        for (size_t n = 0; n < box.second.size(); n++)
            if (box.second[n] != m_Selection.second[n])
                return false;

        return true;
    }

    void SetSelection(adios2::Dims &start, adios2::Dims &count)
    {
        m_Selection.first = start;
        m_Selection.second = count;
    }

    bool IsSelectionValid(adios2::Dims &varShape) const;

    bool TouchSelection(adios2::Dims &start, adios2::Dims &count) const;

    void LoadSelection(const std::string &startStr, const std::string &countStr);

    void LimitToSelection(std::vector<Box<Dims>> &touchedBlocks)
    {
        for (auto it = touchedBlocks.begin(); it != touchedBlocks.end(); it++)
        {
            Box<Dims> overlap = GetIntersection(m_Selection, *it);
            // adios2::helper::IntersectionBox(m_Selection, *it);
            it->first = overlap.first;
            it->second = overlap.second;
        }
    }

    // only applies to global arrays
    void LimitToSelection(std::vector<BlockHit> &blockHits)
    {
      /*
        for (auto i = blockHits.size(); i >= 1; i--)
        {
            if (blockHits[i - 1].isLocalArrayBlock())
                return;

            bool keepBlk = false;
            for (auto it = blockHits[i - 1].m_Regions.begin();
                 it != blockHits[i - 1].m_Regions.end(); it++)
            {
                Box<Dims> overlap = GetIntersection(m_Selection, *it);
                if (overlap.first.size() != 0)
                {
                    keepBlk = true;
                    it->first = overlap.first;
                    it->second = overlap.second;
                }
            }

            if (!keepBlk)
                blockHits.erase(blockHits.begin() + i - 1);
        }
      */
    }

    RangeTree m_RangeTree;
    adios2::Box<adios2::Dims> m_Selection;

    std::string m_VarName;

private:
}; // class QueryVar

class QueryComposite : public QueryBase
{
public:
    QueryComposite(adios2::query::Relation relation) : m_Relation(relation) {}
    ~QueryComposite()
    {
        for (auto n : m_Nodes)
            delete n;
        m_Nodes.clear();
    }

    void BroadcastOutputRegion(const adios2::Box<adios2::Dims> &region)
    {
        if (m_Nodes.size() == 0)
            return;

        for (auto n : m_Nodes)
            n->BroadcastOutputRegion(region);
    }

  /*
    void BlockIndexEvaluate(adios2::core::IO &, adios2::core::Engine &,
                            std::vector<BlockHit> &touchedBlocks);
  */
    // std::vector<Box<Dims>> &touchedBlocks);

  bool AddNode(QueryBase *var) // from Query.cpp
      {
    if (nullptr == var)
        return false;

    if (adios2::query::Relation::NOT == m_Relation)
    {
        // if (m_Nodes.size() > 0) return false;
        // don't want to support NOT for composite queries
        // return false;
      /*
        helper::Throw<std::ios_base::failure>("Toolkit", "query::QueryComposite", "AddNode",
                                              "Currently NOT is not suppprted for composite query");
      */
    }
    m_Nodes.push_back(var);
    return true;
}

  void Print(std::string indent)
    {
      //std::cout << " Composite query" << std::endl;
      // MODIFIED PRINT CODE
      std::cout << indent << "- Composite query (relation ";
      switch (m_Relation) {
      case adios2::query::Relation::OR:
	std::cout << "OR";
	break;
      case adios2::query::Relation::AND:
	std::cout << "AND";
	break;
      case adios2::query::Relation::NOT:
	std::cout << "NOT";
	break;
      default:
	std::cout << "unknown";
      }
      std::cout << ")" << std::endl;
	
        for (auto n : m_Nodes)
	  n->Print(indent + "    ");
    }

    bool IsCompatible(const adios2::Box<adios2::Dims> &box)
    {
        if (m_Nodes.size() == 0)
            return true;
        return (m_Nodes[0])->IsCompatible(box);
    }

  // MODIFICATION - new functions
  adios2::query::Relation GetRelation() { return m_Relation; }
  QueryBase* GetLastChild() { return m_Nodes.back(); }
  void RemoveLastChild()
  {
    if (m_Nodes.size() > 0)
      m_Nodes.pop_back();
  }
    std::vector<QueryBase *> m_Nodes;

private:
    adios2::query::Relation m_Relation = adios2::query::Relation::AND;

  //    std::vector<QueryBase *> m_Nodes;
}; // class QueryComposite

/*
 */

} // namespace query
} //  namespace adiso2

#endif
