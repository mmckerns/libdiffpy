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
* class QResolutionEnvelope -- Gaussian envelope due to limited Q resolution
*
* $Id$
*
*****************************************************************************/

#ifndef QRESOLUTIONENVELOPE_HPP_INCLUDED
#define QRESOLUTIONENVELOPE_HPP_INCLUDED

#include <diffpy/srreal/PDFEnvelope.hpp>

namespace diffpy {
namespace srreal {

/// @class QResolutionEnvelope
/// @brief wide Gaussian PDF scaling envelope caused by finite Q resolution

class QResolutionEnvelope : public PDFEnvelope
{
    public:

        // constructors
        QResolutionEnvelope();
        virtual PDFEnvelopePtr create() const;
        virtual PDFEnvelopePtr clone() const;

        // methods

        virtual const std::string& type() const;
        virtual double operator()(const double& r) const;
        void setQdamp(double sc);
        const double& getQdamp() const;

    private:

        // data
        double mqdamp;

        // serialization
        friend class boost::serialization::access;
        template<class Archive>
            void serialize(Archive& ar, const unsigned int version)
        {
            ar & boost::serialization::base_object<PDFEnvelope>(*this);
            ar & mqdamp;
        }

};  // class QResolutionEnvelope

}   // namespace srreal
}   // namespace diffpy

#endif  // QRESOLUTIONENVELOPE_HPP_INCLUDED
