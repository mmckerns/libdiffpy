/*****************************************************************************
*
* diffpy.srreal     by DANSE Diffraction group
*                   Simon J. L. Billinge
*                   (c) 2010 Trustees of the Columbia University
*                   in the City of New York.  All rights reserved.
*
* File coded by:    Pavol Juhas, Chris Farrow
*
* See AUTHORS.txt for a list of people who contributed.
* See LICENSE.txt for license information.
*
******************************************************************************
*
* class DebyePDFCalculator -- calculate PDF from the Debye equation.
*
* $Id$
*
*****************************************************************************/

#include <cassert>
#include <valarray>
#include <stdexcept>

#include <gsl/gsl_errno.h>
#include <gsl/gsl_fft_complex.h>

#include <diffpy/srreal/DebyePDFCalculator.hpp>
#include <diffpy/srreal/PDFUtils.hpp>
#include <diffpy/srreal/GaussianProfile.hpp>
#include <diffpy/srreal/BaseBondGenerator.hpp>
#include <diffpy/mathutils.hpp>
#include <diffpy/validators.hpp>

using namespace std;
using namespace diffpy::validators;
using diffpy::mathutils::eps_gt;

namespace diffpy {
namespace srreal {

// Constructor ---------------------------------------------------------------

DebyePDFCalculator::DebyePDFCalculator()
{
    // default configuration
    this->setScatteringFactorTable("SFTperiodictableXray");
    this->setRstep(0.01);
    this->setRmax(10.0);
    this->setMaxExtension(10.0);
    // attributes
    this->registerDoubleAttribute("qmin", this,
            &DebyePDFCalculator::getQmin, &DebyePDFCalculator::setQmin);
    this->registerDoubleAttribute("qmax", this,
            &DebyePDFCalculator::getQmax, &DebyePDFCalculator::setQmax);
    this->registerDoubleAttribute("qstep", this,
            &DebyePDFCalculator::getQstep, &DebyePDFCalculator::setQstep);
    this->registerDoubleAttribute("rmin", this,
            &DebyePDFCalculator::getRmin, &DebyePDFCalculator::setRmin);
    this->registerDoubleAttribute("rmax", this,
            &DebyePDFCalculator::getRmax, &DebyePDFCalculator::setRmax);
    this->registerDoubleAttribute("rstep", this,
            &DebyePDFCalculator::getRmax, &DebyePDFCalculator::setRmax);
    this->registerDoubleAttribute("maxextension", this,
            &DebyePDFCalculator::getMaxExtension,
            &DebyePDFCalculator::setMaxExtension);
    this->registerDoubleAttribute("extendedrmin", this,
            &DebyePDFCalculator::rcalclo);
    this->registerDoubleAttribute("extendedrmax", this,
            &DebyePDFCalculator::rcalchi);
}

// Public Methods ------------------------------------------------------------

QuantityType DebyePDFCalculator::getPDF() const
{
    int nfromdr = M_PI / this->getRstep() / this->getQstep();
    int nrequired = max(this->totalQPoints(), nfromdr);
    int nlog2 = int(floor(log2(nrequired))) + 1;
    int padlen = int(pow(2, nlog2));
    // complex valarray needs to have twice as many elements
    valarray<double> ftog(0.0, 2 * padlen);
    QuantityType fqext = this->getExtendedF();
    assert(fqext.size() * 2 <= ftog.size());
    QuantityType::const_iterator fe = fqext.begin();
    double* pfc = &(ftog[0]);
    for (; fe != fqext.end(); ++fe, pfc += 2)  { *pfc = *fe; }
    // apply inverse fft
    int status;
    status = gsl_fft_complex_radix2_inverse(&(ftog[0]), 1, padlen);
    if (status != GSL_SUCCESS)
    {
        const char* emsgft = "Fourier Transformation failed.";
        throw invalid_argument(emsgft);
    }
    // normalize complex part by drpad = 2 * pi / Qmax_padded
    double drpad = 2 * M_PI / (padlen * this->getQstep());
    valarray<double> gpad(0.0, padlen / 2);
    for (int i = 0; i < padlen / 2; ++i)
    {
        gpad[i] = drpad * ftog[2 * i + 1];
    }
    // interpolate to the output r-grid
    QuantityType rgrid = this->getRgrid();
    QuantityType pdf(rgrid.size());
    QuantityType::const_iterator ri = rgrid.begin();
    QuantityType::iterator pdfi = pdf.begin();
    for (; ri != rgrid.end(); ++ri, ++pdfi)
    {
        double xdrp = *ri / drpad;
        int iplo = int(xdrp);
        int iphi = iplo + 1;
        double wphi = xdrp - iplo;
        double wplo = 1.0 - wphi;
        assert(iphi < int(gpad.size()));
        *pdfi = wplo * gpad[iplo] + wphi * gpad[iphi];
    }
    return pdf;
}


QuantityType DebyePDFCalculator::getRgrid() const
{
    double ndrmin = int(this->getRmin() / this->getRstep());
    double ndrmax = int(ceil(this->getRmax() / this->getRstep()));
    QuantityType rv;
    rv.reserve(ndrmax - ndrmin);
    for (int ndr = ndrmin; ndr < ndrmax; ++ndr)
    {
        rv.push_back(ndr * this->getRstep());
    }
    return rv;
}


void DebyePDFCalculator::setQstep(double qstep)
{
    moptimumqstep = false;
    this->BaseDebyeSum::setQstep(qstep);
}


void DebyePDFCalculator::setOptimumQstep()
{
    moptimumqstep = true;
    this->updateQstep();
}

// R-range configuration

void DebyePDFCalculator::setRmin(double rmin)
{
    ensureNonNegative("Rmin", rmin);
    this->PairQuantity::setRmin(rmin);
}


void DebyePDFCalculator::setRmax(double rmax)
{
    ensureNonNegative("Rmax", rmax);
    this->PairQuantity::setRmax(rmax);
    this->updateQstep();
}


void DebyePDFCalculator::setRstep(double rstep)
{
    ensureEpsilonPositive("Rstep", rstep);
    mrstep = rstep;
}


const double& DebyePDFCalculator::getRstep() const
{
    return mrstep;
}


void DebyePDFCalculator::setMaxExtension(double maxextension)
{
    ensureNonNegative("maxextension", maxextension);
    mmaxextension = maxextension;
}


const double& DebyePDFCalculator::getMaxExtension() const
{
    return mmaxextension;
}

// Protected Methods ---------------------------------------------------------

// attributes overload to direct visitors around data structures

void DebyePDFCalculator::accept(diffpy::BaseAttributesVisitor& v)
{
    using ::diffpy::Attributes;
    this->getPeakWidthModel().accept(v);
    // finally call standard accept
    this->Attributes::accept(v);
}


void DebyePDFCalculator::accept(diffpy::BaseAttributesVisitor& v) const
{
    using ::diffpy::Attributes;
    this->getPeakWidthModel().accept(v);
    // finally call standard accept
    this->Attributes::accept(v);
}

// BaseDebyeSum overloads

void DebyePDFCalculator::resetValue()
{
    this->cacheRlimitsData();
    this->updateQstep();
    this->BaseDebyeSum::resetValue();
}


void DebyePDFCalculator::configureBondGenerator(BaseBondGenerator& bnds)
{
    bnds.setRmin(this->rcalclo());
    bnds.setRmax(this->rcalchi());
}


double DebyePDFCalculator::sfSiteAtQ(int siteidx, const double& Q) const
{
    const ScatteringFactorTable& sftable = this->getScatteringFactorTable();
    const string& smbl = mstructure->siteAtomType(siteidx);
    double rv = sftable.lookup(smbl) * mstructure->siteOccupancy(siteidx);
    return rv;
}

// Private Methods -----------------------------------------------------------

void DebyePDFCalculator::updateQstep()
{
    if (!moptimumqstep)  return;
    double rmaxext = this->rcalchi();
    // Use at least 4 steps to Qmax even for tiny rmaxext.
    // Avoid division by zero.
    double qstep = (this->getQmax() * rmaxext / M_PI > 4) ?
        (M_PI / rmaxext) : (this->getQmax() / 4);
    this->BaseDebyeSum::setQstep(qstep);
}


double DebyePDFCalculator::rcalclo() const
{
    double rv = this->getRmin() - mrlimits_cache.totalextension;
    rv = max(rv, 0.0);
    return rv;
}


double DebyePDFCalculator::rcalchi() const
{
    double rv = this->getRmax() + mrlimits_cache.totalextension;
    return rv;
}


double DebyePDFCalculator::extFromTerminationRipples() const
{
    // number of termination ripples for extending the r-range
    const int nripples = 6;
    // extension due to termination ripples
    double rv = (this->getQmax() > 0.0) ?
        (nripples*2*M_PI / this->getQmax()) : 0.0;
    return rv;
}


double DebyePDFCalculator::extFromPeakTails() const
{
    double maxmsd = 2 * maxUii(mstructure);
    double maxfwhm = this->getPeakWidthModel().calculateFromMSD(maxmsd);
    // assume Gaussian peak profile
    GaussianProfile pkf;
    pkf.setPrecision(this->getDebyePrecision());
    // Gaussian function is symmetric, no need to use xboundlo
    double rv = pkf.xboundhi(maxfwhm);
    return rv;
}


void DebyePDFCalculator::cacheRlimitsData()
{
    double totext =
        this->extFromTerminationRipples() + this->extFromPeakTails();
    mrlimits_cache.totalextension = min(totext, this->getMaxExtension());
}

}   // namespace srreal
}   // namespace diffpy

// End of file
