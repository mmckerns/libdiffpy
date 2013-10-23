/*****************************************************************************
*
* diffpy.srreal     Complex Modeling Initiative
*                   Pavol Juhas
*                   (c) 2013 Brookhaven National Laboratory,
*                   Upton, New York.  All rights reserved.
*
* File coded by:    Pavol Juhas
*
* See AUTHORS.txt for a list of people who contributed.
* See LICENSE.txt for license information.
*
******************************************************************************
*
* class PeriodicStructureAdapter -- universal adapter for structure with
*     periodic boundary conditions that has no space group symmetry
*
* class PeriodicStructureBondGenerator -- bond generator
*
*****************************************************************************/

#ifndef PERIODICSTRUCTUREADAPTER_HPP_INCLUDED
#define PERIODICSTRUCTUREADAPTER_HPP_INCLUDED

#include <boost/scoped_ptr.hpp>

#include <diffpy/srreal/forwardtypes.hpp>
#include <diffpy/srreal/AtomicStructureAdapter.hpp>
#include <diffpy/srreal/Lattice.hpp>

namespace diffpy {
namespace srreal {

class PointsInSphere;

class PeriodicStructureAdapter : public AtomicStructureAdapter
{
    public:

        // methods - overloaded
        virtual BaseBondGeneratorPtr createBondGenerator() const;
        virtual double numberDensity() const;
        virtual StructureDifference diff(StructureAdapterConstPtr other) const;

        // methods - own
        void setLatPar(
                double a, double b, double c,
                double alphadeg, double betadeg, double gammadeg);
        const Lattice& getLattice() const;
        void toCartesian(Atom&) const;
        void toFractional(Atom&) const;

    private:

        // data
        Lattice mlattice;

        // serialization
        friend class boost::serialization::access;
        template<class Archive>
            void serialize(Archive& ar, const unsigned int version)
        {
            ar & boost::serialization::base_object<AtomicStructureAdapter>(*this);
            ar & mlattice;
        }

};


class PeriodicStructureBondGenerator : public BaseBondGenerator
{
    public:

        // constructors
        PeriodicStructureBondGenerator(StructureAdapterConstPtr);

        // methods
        // loop control
        virtual void rewind();

        // configuration
        virtual void setRmin(double);
        virtual void setRmax(double);

    protected:

        // data
        const PeriodicStructureAdapter* mpstructure;

        // methods
        virtual bool iterateSymmetry();
        virtual void rewindSymmetry();
        virtual void getNextBond();

    private:

        // methods
        void updater1();

        // data
        boost::scoped_ptr<PointsInSphere> msphere;
        std::vector<R3::Vector> mcartesian_positions_uc;
        R3::Vector mrcsphere;
};

}   // namespace srreal
}   // namespace diffpy

#endif  // PERIODICSTRUCTUREADAPTER_HPP_INCLUDED
