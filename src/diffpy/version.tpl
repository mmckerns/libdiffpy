/*****************************************************************************
*
* libdiffpy         Complex Modeling Initiative
*                   (c) 2014 Brookhaven Science Associates,
*                   Brookhaven National Laboratory.
*                   All rights reserved.
*
* File coded by:    Pavol Juhas
*
* See AUTHORS.txt for a list of people who contributed.
* See LICENSE.txt for license information.
*
******************************************************************************
*
* Macro definitions for
*   DIFFPY_VERSION,
*   DIFFPY_VERSION_MAJOR,
*   DIFFPY_VERSION_MINOR,
*   DIFFPY_VERSION_STR,
*   DIFFPY_VERSION_DATE
*   DIFFPY_GIT_SHA
*
*****************************************************************************/

#ifndef VERSION_HPP_INCLUDED
#define VERSION_HPP_INCLUDED

#define DIFFPY_VERSION_MAJOR ${DIFFPY_VERSION_MAJOR}
#define DIFFPY_VERSION_MINOR ${DIFFPY_VERSION_MINOR}

// DIFFPY_VERSION % 1000 is number of git commits since minor version
// DIFFPY_VERSION / 1000 % 1000 is the minor version
// DIFFPY_VERSION / 1000000 is the major version

#define DIFFPY_VERSION ${DIFFPY_VERSION}

// DIFFPY_VERSION_STR is a string form of DIFFPY_VERSION

#define DIFFPY_VERSION_STR "${DIFFPY_VERSION_STR}"

// DIFFPY_VERSION_DATE is the commit date of DIFFPY_VERSION

#define DIFFPY_VERSION_DATE "${DIFFPY_VERSION_DATE}"

// DIFFPY_GIT_SHA is a full git commit hash for the current version

#define DIFFPY_GIT_SHA "${DIFFPY_GIT_SHA}"

#endif  // VERSION_HPP_INCLUDED

// vim:ft=cpp:
