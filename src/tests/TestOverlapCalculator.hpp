/*****************************************************************************
*
* libdiffpy         by DANSE Diffraction group
*                   Simon J. L. Billinge
*                   (c) 2011 The Trustees of Columbia University
*                   in the City of New York.  All rights reserved.
*
* File coded by:    Pavol Juhas
*
* See AUTHORS.txt for a list of people who contributed.
* See LICENSE_DANSE.txt for license information.
*
******************************************************************************
*
* class TestOverlapCalculator -- unit tests for the OverlapCalculator class
*
*****************************************************************************/

#include <cxxtest/TestSuite.h>

#include <diffpy/srreal/AtomicStructureAdapter.hpp>
#include <diffpy/srreal/PeriodicStructureAdapter.hpp>
#include <diffpy/srreal/OverlapCalculator.hpp>
#include <diffpy/serialization.ipp>
#include "test_helpers.hpp"
#include "serialization_helpers.hpp"

using namespace std;
using namespace diffpy::srreal;

class TestOverlapCalculator : public CxxTest::TestSuite
{
    private:

        boost::shared_ptr<OverlapCalculator> molc;
        StructureAdapterPtr mnacl;
        double meps;

    public:

        void setUp()
        {
            CxxTest::setAbortTestOnFail(true);
            meps = diffpy::mathutils::SQRT_DOUBLE_EPS;
            molc.reset(new OverlapCalculator);
            molc->getAtomRadiiTable()->setCustom("", 1.0);
            molc->getAtomRadiiTable()->setCustom("Na1+", 1.5);
            molc->getAtomRadiiTable()->setCustom("Cl1-", 1.8);
            if (!mnacl)  mnacl = loadTestPeriodicStructure("NaCl.stru");
            CxxTest::setAbortTestOnFail(true);
        }


        void test_lineNoTouch()
        {
            AtomicStructureAdapterPtr stru(new AtomicStructureAdapter);
            Atom a0, a1;
            a0.xyz_cartn = R3::Vector(0.0, 0.0, 0.0);
            a1.xyz_cartn = R3::Vector(0.0, 0.0, 2.0);
            stru->append(a0);
            stru->append(a1);
            molc->eval(stru);
            TS_ASSERT_EQUALS(0.0, molc->totalSquareOverlap());
            QuantityType sqolps = molc->siteSquareOverlaps();
            TS_ASSERT_EQUALS(2u, sqolps.size());
            TS_ASSERT_EQUALS(0.0, sqolps[0]);
            TS_ASSERT_EQUALS(0.0, sqolps[1]);
            TS_ASSERT_EQUALS(0.0, molc->flipDiffTotal(0, 0));
            TS_ASSERT_EQUALS(0.0, molc->flipDiffTotal(0, 1));
            std::vector<R3::Vector> g = molc->gradients();
            TS_ASSERT_EQUALS(2u, g.size());
            TS_ASSERT_EQUALS(0.0, R3::norm(g[0]));
            TS_ASSERT_EQUALS(0.0, R3::norm(g[1]));
        }


        void test_lineTouch()
        {
            AtomicStructureAdapterPtr stru(new AtomicStructureAdapter);
            Atom a0, a1;
            a0.xyz_cartn = R3::Vector(0.0, 0.0, 0.0);
            a1.xyz_cartn = R3::Vector(0.0, 0.0, 1.5);
            stru->append(a0);
            stru->append(a1);
            molc->eval(stru);
            TS_ASSERT_EQUALS(0.25, molc->totalSquareOverlap());
            QuantityType sqolps = molc->siteSquareOverlaps();
            TS_ASSERT_EQUALS(2u, sqolps.size());
            TS_ASSERT_EQUALS(0.125, sqolps[0]);
            TS_ASSERT_EQUALS(0.125, sqolps[1]);
            TS_ASSERT_EQUALS(0.0, molc->flipDiffTotal(0, 1));
            std::vector<R3::Vector> g = molc->gradients();
            TS_ASSERT_EQUALS(2u, g.size());
            R3::Vector g0(0.0, 0.0, +1);
            TS_ASSERT_EQUALS(0.0, R3::distance(g0, g[0]));
            R3::Vector g1(0.0, 0.0, -1);
            TS_ASSERT_EQUALS(0.0, R3::distance(g1, g[1]));
        }


        void test_gradients()
        {
            AtomicStructureAdapterPtr stru(new AtomicStructureAdapter);
            Atom a0, a1;
            a0.xyz_cartn = R3::Vector(0.0, 0.0, 0.0);
            a1.xyz_cartn = R3::Vector(0.17, 0.31, 0.37);
            stru->append(a0);
            stru->append(a1);
            molc->eval(stru);
            std::vector<R3::Vector> g = molc->gradients();
            double dx = 1e-6;
            double tsqo0 = molc->totalSquareOverlap();
            R3::Vector r1 = stru->siteCartesianPosition(1);
            R3::Vector gn1;
            for (int k = 0; k != R3::Ndim; ++k)
            {
                R3::Vector& xyz1 = stru->at(1).xyz_cartn;
                xyz1 = r1;
                xyz1[k] += dx;
                molc->eval(stru);
                gn1[k] = (molc->totalSquareOverlap() - tsqo0) / dx;
            }
            TS_ASSERT(R3::norm(gn1) > 1.0);
            TS_ASSERT_DELTA(0.0, R3::norm(gn1 - g[1]), 1e-5);
            TS_ASSERT_DELTA(0.0, R3::norm(g[0] + g[1]), 1e-5);
        }


        void test_bccTouch()
        {
            using namespace boost;
            PeriodicStructureAdapterPtr bcc(new PeriodicStructureAdapter);
            bcc->setLatPar(2.0, 2.0, 2.0, 90, 90, 90);
            Atom a;
            a.atomtype = "C";
            bcc->append(a);
            a.xyz_cartn = R3::Vector(0.5, 0.5, 0.5);
            bcc->toCartesian(a);
            bcc->append(a);
            molc->getAtomRadiiTable()->setCustom("C", 1.0);
            molc->eval(bcc);
            TS_ASSERT_EQUALS(2, molc->getStructure()->countSites());
            TS_ASSERT_EQUALS(8 * pow(2.0 - sqrt(3.0), 2),
                    molc->totalSquareOverlap());
            // gradient is zero due to bcc symmetry
            std::vector<R3::Vector> g = molc->gradients();
            TS_ASSERT_DELTA(0.0, R3::norm(g[0]), meps);
            TS_ASSERT_DELTA(0.0, R3::norm(g[1]), meps);
            // move the second atom so it touches only one ball
            bcc->at(1).xyz_cartn = R3::Vector(-0.25, 0.0, 0.0);
            bcc->toCartesian(bcc->at(1));
            molc->eval(bcc);
            double tsqo = 1.5 * 1.5 + 0.5 * 0.5;
            TS_ASSERT_DELTA(tsqo, molc->totalSquareOverlap(), meps);
            g = molc->gradients();
            TS_ASSERT_EQUALS(2u, g.size());
            double gx = 1.5 + 0.5;
            TS_ASSERT_DELTA(-gx, g[0][0], meps);
            TS_ASSERT_DELTA(0.0, g[0][1], meps);
            TS_ASSERT_DELTA(0.0, g[0][2], meps);
            TS_ASSERT_DELTA(+gx, g[1][0], meps);
            TS_ASSERT_DELTA(0.0, g[1][1], meps);
            TS_ASSERT_DELTA(0.0, g[1][2], meps);
        };


        void test_NaCl_overlap()
        {
            double olp = pow(3.3 - 5.62 / 2, 2) * 6 / 2;
            molc->eval(mnacl);
            TS_ASSERT_DELTA(olp, molc->meanSquareOverlap(), meps);
            QuantityType sqolps = molc->siteSquareOverlaps();
            TS_ASSERT_EQUALS(8u, sqolps.size());
            for (int i = 0; i < 8; ++i)
            {
                TS_ASSERT_DELTA(olp, sqolps[i], meps);
            }
        }


        void test_NaCl_flips()
        {
            molc->eval(mnacl);
            // flipping the same type should not change the cost
            TS_ASSERT_EQUALS(0.0, molc->flipDiffTotal(0, 0));
            TS_ASSERT_EQUALS(0.0, molc->flipDiffTotal(0, 1));
            TS_ASSERT_EQUALS(0.0, molc->flipDiffTotal(0, 2));
            TS_ASSERT_EQUALS(0.0, molc->flipDiffTotal(0, 3));
            // flipping with the second Cl neighbor
            TS_ASSERT_DELTA(1.08, molc->flipDiffTotal(0, 4), meps);
            TS_ASSERT_DELTA(1.08 / 8, molc->flipDiffMean(0, 4), meps);
            // flipping with the nearest Cl neighbor
            TS_ASSERT_DELTA(0.72, molc->flipDiffTotal(0, 5), meps);
            TS_ASSERT_DELTA(0.72, molc->flipDiffTotal(0, 6), meps);
            TS_ASSERT_DELTA(0.72, molc->flipDiffTotal(0, 7), meps);
        }


        void test_NaCl_gradient()
        {
            using namespace boost;
            molc->eval(mnacl);
            // default gradients are all zero
            std::vector<R3::Vector> g = molc->gradients();
            TS_ASSERT_EQUALS(8u, g.size());
            for (int i = 0; i < 8; ++i)
            {
                TS_ASSERT_DELTA(0.0, R3::norm(g[i]), meps);
            }
            StructureAdapterPtr nacl1 = mnacl->clone();
            PeriodicStructureAdapter& nacl1ref =
                static_cast<PeriodicStructureAdapter&>(*nacl1);
            R3::Vector& xyzc0 = nacl1ref[0].xyz_cartn;
            xyzc0 = R3::Vector(0.02, 0.03, 0.07);
            molc->eval(nacl1);
            double c0 = molc->totalSquareOverlap();
            R3::Vector g0 = molc->gradients()[0];
            TS_ASSERT(R3::norm(g0) > meps);
            R3::Vector g0n;
            const double dx = 1e-8;
            for (int i = 0; i < R3::Ndim; ++i)
            {
                xyzc0[i] += dx;
                molc->eval(nacl1);
                g0n[i] = (molc->totalSquareOverlap() - c0) / dx;
                xyzc0[i] -= dx;
            }
            TS_ASSERT_DELTA(0.0, R3::norm(g0 - g0n), 1e-6);
        }


        void test_NaCl_mixed_overlap()
        {
            StructureAdapterPtr nacl_mixed;
            nacl_mixed = loadTestPeriodicStructure("NaCl_mixed.stru");
            molc->eval(nacl_mixed);
            double olp = pow(3.3 - 5.62 / 2, 2) * 6 / 2;
            TS_ASSERT_DELTA(olp, molc->meanSquareOverlap(), meps);
        }


        void test_serialization()
        {
            // build customized PDFCalculator
            AtomicStructureAdapterPtr stru(new AtomicStructureAdapter);
            Atom a0, a1;
            a0.xyz_cartn = R3::Vector(0.0, 0.0, 0.0);
            a1.xyz_cartn = R3::Vector(0.0, 0.0, 1.5);
            stru->append(a0);
            stru->append(a1);
            molc->eval(stru);
            boost::shared_ptr<OverlapCalculator> olc1;
            olc1 = dumpandload(molc);
            TS_ASSERT_DIFFERS(molc.get(), olc1.get());
            TS_ASSERT_EQUALS(2, olc1->getStructure()->countSites());
            TS_ASSERT_EQUALS(0.25, olc1->totalSquareOverlap());
            QuantityType sqolps = molc->siteSquareOverlaps();
            TS_ASSERT_EQUALS(2u, olc1->siteSquareOverlaps().size());
            TS_ASSERT_EQUALS(0.125, olc1->siteSquareOverlaps()[0]);
            TS_ASSERT_EQUALS(0.125, olc1->siteSquareOverlaps()[1]);
        }

};  // class TestOverlapCalculator

// End of file
