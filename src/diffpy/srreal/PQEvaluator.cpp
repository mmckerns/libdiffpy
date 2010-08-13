/*****************************************************************************
*
* diffpy.srreal     by DANSE Diffraction group
*                   Simon J. L. Billinge
*                   (c) 2009 Trustees of the Columbia University
*                   in the City of New York.  All rights reserved.
*
* File coded by:    Pavol Juhas
*
* See AUTHORS.txt for a list of people who contributed.
* See LICENSE.txt for license information.
*
******************************************************************************
*
* class PQEvaluatorBasic -- robust PairQuantity evaluator, the result
*     is always calculated from scratch.

* class PQEvaluatorOptimized -- optimized PairQuantity evaluator with fast
*     quantity updates
*
* $Id$
*
*****************************************************************************/


#include <stdexcept>
#include <sstream>

#include <diffpy/srreal/PQEvaluator.hpp>
#include <diffpy/srreal/PairQuantity.hpp>
#include <diffpy/srreal/StructureAdapter.hpp>

using namespace std;

// Local Constants -----------------------------------------------------------

namespace {
// tolerated load variance for splitting outer loop for parallel evaluation
const double CPU_LOAD_VARIANCE = 0.1;
}

namespace diffpy {
namespace srreal {

//////////////////////////////////////////////////////////////////////////////
// class PQEvaluatorBasic
//////////////////////////////////////////////////////////////////////////////

PQEvaluatorType PQEvaluatorBasic::typeint() const
{
    return BASIC;
}


void PQEvaluatorBasic::updateValue(PairQuantity& pq)
{
    auto_ptr<BaseBondGenerator> bnds;
    bnds.reset(pq.mstructure->createBondGenerator());
    pq.configureBondGenerator(*bnds);
    int cntsites = pq.mstructure->countSites();
    // loop counter
    long n = mcpuindex;
    // split outer loop for many atoms.  The CPUs should have similar load.
    bool chop_outer = (mncpu <= ((cntsites - 1) * CPU_LOAD_VARIANCE + 1));
    bool chop_inner = !chop_outer;
    for (int i0 = 0; i0 < cntsites; ++i0)
    {
        if (chop_outer && (n++ % mncpu))    continue;
        bnds->selectAnchorSite(i0);
        bnds->selectSiteRange(0, i0 + 1);
        for (bnds->rewind(); !bnds->finished(); bnds->next())
        {
            if (chop_inner && (n++ % mncpu))    continue;
            if (!pq.getPairMask(bnds->site0(), bnds->site1()))  continue;
            int summationscale = (bnds->site0() == bnds->site1()) ? 1 : 2;
            pq.addPairContribution(*bnds, summationscale);
        }
    }
}


void PQEvaluatorBasic::setupParallelRun(int cpuindex, int ncpu)
{
    // make sure ncpu is at least one
    if (ncpu < 1)
    {
        const char* emsg = "Number of CPU ncpu must be at least 1.";
        throw invalid_argument(emsg);
    }
    mcpuindex = cpuindex;
    mncpu = ncpu;
}

//////////////////////////////////////////////////////////////////////////////
// class PQEvaluatorOptimized
//////////////////////////////////////////////////////////////////////////////

PQEvaluatorType PQEvaluatorOptimized::typeint() const
{
    return OPTIMIZED;
}


void PQEvaluatorOptimized::updateValue(PairQuantity& pq)
{
}

// Factory for PairQuantity evaluators ---------------------------------------

PQEvaluatorBasic* createPQEvaluator(PQEvaluatorType pqtp)
{
    PQEvaluatorBasic* rv = NULL;
    switch (pqtp)
    {
        case BASIC:
            rv = new PQEvaluatorBasic();
            break;

        case OPTIMIZED:
            rv = new PQEvaluatorOptimized();
            break;

        default:
            ostringstream emsg;
            emsg << "Invalid PQEvaluatorType value " << pqtp;
            throw invalid_argument(emsg.str());
    }
    return rv;
}


}   // namespace srreal
}   // namespace diffpy

// End of file
