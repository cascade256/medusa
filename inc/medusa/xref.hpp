#ifndef MEDUSA_XREF_HPP
#define MEDUSA_XREF_HPP

#include "medusa/namespace.hpp"
#include "medusa/export.hpp"
#include "medusa/types.hpp"
#include "medusa/address.hpp"

#include <list>
#include <boost/bimap.hpp>
#include <boost/bimap/multiset_of.hpp>

MEDUSA_NAMESPACE_BEGIN

//! XRefs handles a cross-reference.
class Medusa_EXPORT XRefs
{
public:
   XRefs(void) {}
  ~XRefs(void) {}

  typedef boost::bimap<
    boost::bimaps::multiset_of<Address>,
    Address
  > TXRefs;

  //! Add new cross-reference
  bool AddXRef(Address const& rTo, Address const& rFrom);

  //! Remove a cross-reference
  bool RemoveRef(Address const& rFrom);

  //! Get all addresses which access to a specific address
  bool From(Address const& rTo, Address::List& rFromList) const;

  //! Get accessed address from a specific address
  bool To(Address const& rFrom, Address& rTo) const;

  //! Test if a specific address has a cross-reference
  bool HasXRefTo(Address const& rFrom) const;

  //! Test if a specific address has a cross-reference
  bool HasXRefFrom(Address const& rTo) const;

  //! Erase all cross-references
  void EraseAll(void);

  //! Get all cross-references
  TXRefs const& GetAllXRefs(void) const { return m_XRefs; }

private:
  TXRefs m_XRefs;
};

MEDUSA_NAMESPACE_END

#endif // !MEDUSA_XREF_HPP