/*****************************************************************************
*
* libdiffpy         by DANSE Diffraction group
*                   Simon J. L. Billinge
*                   (c) 2010 The Trustees of Columbia University
*                   in the City of New York.  All rights reserved.
*
* File coded by:    Pavol Juhas
*
* See AUTHORS.txt for a list of people who contributed.
* See LICENSE_DANSE.txt for license information.
*
******************************************************************************
*
* class NoMetaStructureAdapter -- StructureAdapter proxy class that disables
*     customPQConfig of another StructureAdapter instance.  This may be used
*     for preventing the adapter for diffpy.Structure class from applying
*     scale, spdiameter and such metadata to the PDFCalculator.
*
* nometa -- factory function that returns a NoMetaStructureAdapter
*     instance wrapped in StructureAdapterPtr
*
*****************************************************************************/

#include <diffpy/serialization.ipp>
#include <diffpy/srreal/StructureDifference.hpp>
#include <diffpy/srreal/NoMetaStructureAdapter.hpp>

namespace diffpy {
namespace srreal {

//////////////////////////////////////////////////////////////////////////////
// class NoMetaStructureAdapter
//////////////////////////////////////////////////////////////////////////////

// Constructor ---------------------------------------------------------------

NoMetaStructureAdapter::NoMetaStructureAdapter(
        StructureAdapterPtr srcstructure)
{
    boost::shared_ptr<NoMetaStructureAdapter> nmptr =
        boost::dynamic_pointer_cast<NoMetaStructureAdapter>(srcstructure);
    if (nmptr)  srcstructure = nmptr->getSourceStructure();
    msrcstructure = srcstructure.get() ?
        srcstructure : emptyStructureAdapter();
}

// Public Methods ------------------------------------------------------------

StructureAdapterPtr NoMetaStructureAdapter::clone() const
{
    boost::shared_ptr<NoMetaStructureAdapter> rv(new NoMetaStructureAdapter);
    if (msrcstructure)  rv->msrcstructure = msrcstructure->clone();
    return rv;
}


BaseBondGeneratorPtr NoMetaStructureAdapter::createBondGenerator() const
{
    BaseBondGeneratorPtr bnds = msrcstructure->createBondGenerator();
    return bnds;
}


int NoMetaStructureAdapter::countSites() const
{
    return msrcstructure->countSites();
}


double NoMetaStructureAdapter::numberDensity() const
{
    return msrcstructure->numberDensity();
}


const std::string& NoMetaStructureAdapter::siteAtomType(int idx) const
{
    return msrcstructure->siteAtomType(idx);
}


const R3::Vector& NoMetaStructureAdapter::siteCartesianPosition(
        int idx) const
{
    return msrcstructure->siteCartesianPosition(idx);
}


double NoMetaStructureAdapter::siteOccupancy(int idx) const
{
    return msrcstructure->siteOccupancy(idx);
}


bool NoMetaStructureAdapter::siteAnisotropy(int idx) const
{
    return msrcstructure->siteAnisotropy(idx);
}


const R3::Matrix& NoMetaStructureAdapter::siteCartesianUij(int idx) const
{
    return msrcstructure->siteCartesianUij(idx);
}


void NoMetaStructureAdapter::customPQConfig(PairQuantity* pq) const
{
    // disable customPQConfig of the wrapped PairQuantity
}


StructureDifference
NoMetaStructureAdapter::diff(StructureAdapterConstPtr other) const
{
    StructureDifference alldiffer;
    typedef boost::shared_ptr<const class NoMetaStructureAdapter> PPtr;
    PPtr pother = boost::dynamic_pointer_cast<PPtr::element_type>(other);
    if (!pother)  return alldiffer;
    StructureDifference sd =
        this->getSourceStructure()->diff(pother->getSourceStructure());
    assert(sd.stru0 == this->getSourceStructure());
    assert(sd.stru1 == pother->getSourceStructure());
    sd.stru0 = this->shared_from_this();
    sd.stru1 = other;
    return sd;
}


StructureAdapterPtr
NoMetaStructureAdapter::getSourceStructure()
{
    return msrcstructure;
}


StructureAdapterConstPtr
NoMetaStructureAdapter::getSourceStructure() const
{
    return msrcstructure;
}

// Routines ------------------------------------------------------------------

StructureAdapterPtr nometa(StructureAdapterPtr stru)
{
    StructureAdapterPtr rv =
        boost::dynamic_pointer_cast<NoMetaStructureAdapter>(stru) ? stru :
        StructureAdapterPtr(new NoMetaStructureAdapter(stru));
    return rv;
}

}   // namespace srreal
}   // namespace diffpy

// Serialization -------------------------------------------------------------

DIFFPY_INSTANTIATE_SERIALIZATION(diffpy::srreal::NoMetaStructureAdapter)
BOOST_CLASS_EXPORT_IMPLEMENT(diffpy::srreal::NoMetaStructureAdapter)

// End of file
