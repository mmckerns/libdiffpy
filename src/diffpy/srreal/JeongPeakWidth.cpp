/*****************************************************************************
*
* diffpy.srreal     by DANSE Diffraction group
*                   Simon J. L. Billinge
*                   (c) 2009 Trustees of the Columbia University
*                   in the City of New York.  All rights reserved.
*
* File coded by:    Christopher Farrow, Pavol Juhas
*
* See AUTHORS.txt for a list of people who contributed.
* See LICENSE.txt for license information.
*
******************************************************************************
*
* class DebyeWallerPeakWidth -- peak width model based on
*      I.-K. Jeong, et al., Phys. Rev. B 67, 104301 (2003)
*      http://link.aps.org/doi/10.1103/PhysRevB.67.104301
*
* $Id$
*
*****************************************************************************/

#include <diffpy/srreal/JeongPeakWidth.hpp>
#include <diffpy/mathutils.hpp>
#include <diffpy/serialization.hpp>

namespace diffpy {
namespace srreal {

using namespace std;

// Constructors --------------------------------------------------------------

JeongPeakWidth::JeongPeakWidth()
{
    this->setDelta1(0.0);
    this->setDelta2(0.0);
    this->setQbroad(0.0);
    this->registerDoubleAttribute("delta1",
            this, &JeongPeakWidth::getDelta1, &JeongPeakWidth::setDelta1);
    this->registerDoubleAttribute("delta2",
            this, &JeongPeakWidth::getDelta2, &JeongPeakWidth::setDelta2);
    this->registerDoubleAttribute("qbroad",
            this, &JeongPeakWidth::getQbroad, &JeongPeakWidth::setQbroad);
}


PeakWidthModelPtr JeongPeakWidth::create() const
{
    PeakWidthModelPtr rv(new JeongPeakWidth());
    return rv;
}


PeakWidthModelPtr JeongPeakWidth::clone() const
{
    PeakWidthModelPtr rv(new JeongPeakWidth(*this));
    return rv;
}

// Public Methods ------------------------------------------------------------

const string& JeongPeakWidth::type() const
{
    static const string rv = "jeong";
    return rv;
}


double JeongPeakWidth::calculate(const BaseBondGenerator& bnds) const
{
    using diffpy::mathutils::DOUBLE_EPS;
    double r = bnds.distance();
    // avoid division by zero
    double corr = (r < DOUBLE_EPS) ? 0.0 :
        (1.0 - this->getDelta1()/r - this->getDelta2()/pow(r, 2) +
         pow(this->getQbroad()*r, 2));
    // avoid calculating square root of negative value
    double fwhm = (corr <= 0) ? 0.0 :
        (sqrt(corr) * this->DebyeWallerPeakWidth::calculate(bnds));
    return fwhm;
}


const double& JeongPeakWidth::getDelta1() const
{
    return mdelta1;
}


void JeongPeakWidth::setDelta1(double delta1)
{
    mdelta1 = delta1;
}


const double& JeongPeakWidth::getDelta2() const
{
    return mdelta2;
}


void JeongPeakWidth::setDelta2(double delta2)
{
    mdelta2 = delta2;
}


const double& JeongPeakWidth::getQbroad() const
{
    return mqbroad;
}


void JeongPeakWidth::setQbroad(double qbroad)
{
    mqbroad = qbroad;
}

// Registration --------------------------------------------------------------

bool reg_JeongPeakWidth = JeongPeakWidth().registerThisType();

}   // namespace srreal
}   // namespace diffpy

// Serialization -------------------------------------------------------------

DIFFPY_INSTANTIATE_SERIALIZE(diffpy::srreal::JeongPeakWidth)
BOOST_CLASS_EXPORT(diffpy::srreal::JeongPeakWidth)

// End of file
