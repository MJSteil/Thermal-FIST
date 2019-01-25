/*
 * Thermal-FIST package
 * 
 * Copyright (c) 2014-2018 Volodymyr Vovchenko
 *
 * GNU General Public License (GPLv3 or later)
 */
#ifndef SIMPLEPARTICLE_H
#define SIMPLEPARTICLE_H

#include <cmath>

namespace thermalfist {
  /// Structure holding information about a single particle in the event generator.
  struct SimpleParticle {
    double px, py, pz; ///< 3-momentum components (in GeV)
    double m;          ///< Mass (in GeV)
    double p0;         ///< Energy (in GeV)
    long long PDGID;         ///< PDG code
    long long MotherPDGID;   ///< PDG code of a mother particle, if applicable
    bool processed;          ///< Used in event generator

    /// Default constructor
    SimpleParticle() { processed = false; }

    /// Constructs a particle from provided three-momentum, mass, and PDG code
    SimpleParticle(double inPx, double inPy, double inPz, double inM, long long inPDGID, long long inMotherPDGID = 0) :
      px(inPx),
      py(inPy),
      pz(inPz),
      m(inM),
      p0(sqrt(m*m + px * px + py * py + pz * pz)),
      PDGID(inPDGID), MotherPDGID(inMotherPDGID), processed(false) { }

    /// Absolute value of the 3-momentum (in GeV)
    double GetP() const {
      return sqrt(p0*p0 - m * m);
    }

    /// Transverse momentum (in GeV)
    double GetPt() const {
      return sqrt(px*px + py * py);
    }

    /// Transverse mass (in GeV)
    double GetMt() const {
      return sqrt(m*m + px * px + py * py);
    }

    /// The longitudinal rapidity
    double GetY() const {
      return 0.5*log((p0 + pz) / (p0 - pz));
    }
  };

} // namespace thermalfist

#endif
