/*
 * Thermal-FIST package
 * 
 * Copyright (c) 2015-2019 Volodymyr Vovchenko
 *
 * GNU General Public License (GPLv3 or later)
 */
#include "HRGEventGenerator/RandomGenerators.h"

#include "HRGBase/xMath.h"

namespace thermalfist {

  namespace RandomGenerators {

    MTRand randgenMT;

    void SetSeed(const unsigned int seed) {
      randgenMT.seed(seed);
    }

    int RandomPoisson(double mean) {
      int n;
      if (mean <= 0) return 0;
      if (mean < 25) {
        double expmean = exp(-mean);
        double pir = 1;
        n = -1;
        while (1) {
          n++;
          pir *= randgenMT.rand();
          if (pir <= expmean) break;
        }
        return n;
      }
      // for large value we use inversion method
      else {//if (mean < 1E9) {
        double em, t, y;
        double sq, alxm, g;
        double pi = xMath::Pi();

        sq = sqrt(2.0*mean);
        alxm = log(mean);
        g = mean * alxm - xMath::LogGamma(mean + 1.0);

        do {
          do {
            y = tan(pi*randgenMT.rand());
            em = sq * y + mean;
          } while (em < 0.0);

          em = floor(em);
          t = 0.9*(1.0 + y * y)* exp(em*alxm - xMath::LogGamma(em + 1.0) - g);
        } while (randgenMT.rand() > t);

        return static_cast<int> (em);

      }
      //else {
      //   // use Gaussian approximation vor very large values
      //   n = Int_t(Gaus(0,1)*TMath::Sqrt(mean) + mean + 0.5);
      //   return n;
      //}
    }


    int RandomPoisson(double mean, MTRand &rangen) {
      int n;
      if (mean <= 0) return 0;
      if (mean < 25) {
        double expmean = exp(-mean);
        double pir = 1;
        n = -1;
        while (1) {
          n++;
          pir *= rangen.rand();
          if (pir <= expmean) break;
        }
        return n;
      }
      // for large value we use inversion method
      else {//if (mean < 1E9) {
        double em, t, y;
        double sq, alxm, g;
        double pi = xMath::Pi();

        sq = sqrt(2.0*mean);
        alxm = log(mean);
        g = mean * alxm - xMath::LogGamma(mean + 1.0);

        do {
          do {
            y = tan(pi*rangen.rand());
            em = sq * y + mean;
          } while (em < 0.0);

          em = floor(em);
          t = 0.9*(1.0 + y * y)* exp(em*alxm - xMath::LogGamma(em + 1.0) - g);
        } while (rangen.rand() > t);

        return static_cast<int> (em);

      }
      //else {
      //   // use Gaussian approximation vor very large values
      //   n = Int_t(Gaus(0,1)*TMath::Sqrt(mean) + mean +0.5);
      //   return n;
      //}
    }

    double SkellamProbability(int k, double mu1, double mu2)
    {
      return exp(-(mu1 + mu2)) * pow(sqrt(mu1 / mu2), k) * xMath::BesselI(k, 2. * sqrt(mu1 * mu2));
    }


    double SiemensRasmussenMomentumGenerator::g(double x) const {
      double tp = -log(x);
      double talpha = alpha(tp);
      double en = w(tp);
      double sh = sinh(talpha);
      double shtalpha = 1.;
      if (talpha != 0.0)
        shtalpha = sh / talpha;
      double ch = sqrt(1. + sh * sh);
      return tp * tp * exp(-m_Gamma * en / m_T) * ((1. + m_T / m_Gamma / en)*shtalpha - m_T / m_Gamma / en * ch) / x;
    }

    double SiemensRasmussenMomentumGenerator::g2(double x, double tp) const {
      double talpha = alpha(tp);
      double en = w(tp);
      double sh = sinh(talpha);
      double shtalpha = 1.;
      if (talpha != 0.0)
        shtalpha = sh / talpha;
      double ch = sqrt(1. + sh * sh);
      return tp * tp * exp(-m_Gamma * en / m_T) * ((1. + m_T / m_Gamma / en)*shtalpha - m_T / m_Gamma / en * ch) / x;
    }

    void SiemensRasmussenMomentumGenerator::FixParameters() {
      double eps = 1e-8;
      double l = 0., r = 1.;
      double m1 = l + (r - l) / 3.;
      double m2 = r - (r - l) / 3.;
      int MAXITERS = 200;
      int iter = 0;
      while (fabs(m2 - m1) > eps && iter < MAXITERS) {
        if (g(m1) < g(m2)) {
          l = m1;
        }
        else {
          r = m2;
        }
        m1 = l + (r - l) / 3.;
        m2 = r - (r - l) / 3.;
        iter++;
      }
      m_Max = g((m1 + m2) / 2.);
    }

    double SiemensRasmussenMomentumGenerator::GetRandom() const {
      while (1) {
        double x0 = randgenMT.randDblExc();
        double y0 = m_Max * randgenMT.randDblExc();
        if (y0 < g(x0)) return -log(x0);
      }
      return 0.;
    }

    std::vector<double> SiemensRasmussenMomentumGenerator::GetMomentum() const {
      std::vector<double> ret(0);
      double tp = GetRandom();
      double tphi = 2. * xMath::Pi() * randgenMT.rand();
      double cthe = 2. * randgenMT.rand() - 1.;
      double sthe = sqrt(1. - cthe * cthe);
      ret.push_back(tp*cos(tphi)*sthe); //px
      ret.push_back(tp*sin(tphi)*sthe); //py
      ret.push_back(tp*cthe);           //pz
      return ret;
    }



    void SSHMomentumGenerator::FixParameters() {
      {
        double eps = 1e-8;
        double l = 0., r = 1.;
        double m1 = l + (r - l) / 3.;
        double m2 = r - (r - l) / 3.;
        int MAXITERS = 200;
        int iter = 0;
        while (fabs(m2 - m1) > eps && iter < MAXITERS) {
          if (g(m1) < g(m2)) {
            l = m1;
          }
          else {
            r = m2;
          }
          m1 = l + (r - l) / 3.;
          m2 = r - (r - l) / 3.;
          iter++;
        }
        m_MaxPt = g((m1 + m2) / 2.);

        m_dndpt.clear();
        double dx = m_dPt;
        for (double x = 0.5*dx; x <= 1.; x += dx) {
          m_dndpt.add_val(x, g(x));
        }
      }

      {
        m_dndy.resize(0);
        m_MaxYs.resize(0);
        double dx = m_dPt;
        for (double x = 0.5*dx; x <= 1.; x += dx) {

          double pt = -log(x);

          double eps = 1e-8;
          double l = -4. - m_EtaMax, r = 4. + m_EtaMax;
          double m1 = l + (r - l) / 3.;
          double m2 = r - (r - l) / 3.;
          int MAXITERS = 200;
          int iter = 0;
          while (fabs(m2 - m1) > eps && iter < MAXITERS) {
            if (m_distr.dndy(m1, pt) < m_distr.dndy(m2, pt)) {
              l = m1;
            }
            else {
              r = m2;
            }
            m1 = l + (r - l) / 3.;
            m2 = r - (r - l) / 3.;
            iter++;
          }
          m_MaxYs.push_back(m_distr.dndy((m1 + m2) / 2., pt));

          m_dndy.push_back(SplineFunction());
          double dy = m_dy;
          for (double ty = -4. - m_EtaMax + 0.5*dy; ty <= 4. + m_EtaMax; ty += dy) {
            m_dndy[m_dndy.size() - 1].add_val(ty, m_distr.dndy(ty, pt));
          }
        }
      }
    }

    void SSHMomentumGenerator::FixParameters2() {
      {
        double eps = 1e-8;
        double l = 0., r = 1.;
        double m1 = l + (r - l) / 3.;
        double m2 = r - (r - l) / 3.;
        int MAXITERS = 200;
        int iter = 0;
        while (fabs(m2 - m1) > eps && iter < MAXITERS) {
          if (g(m1) < g(m2)) {
            l = m1;
          }
          else {
            r = m2;
          }
          m1 = l + (r - l) / 3.;
          m2 = r - (r - l) / 3.;
          iter++;
        }
        m_MaxPt = g((m1 + m2) / 2.);

        m_dndpt.clear();
        double dx = m_dPt;
        for (double x = 0.5*dx; x <= 1.; x += dx) {
          m_dndpt.add_val(x, g(x));
        }
      }

      {
        m_dndy.resize(0);
        m_MaxYs.resize(0);
        double dx = m_dPt;
        for (double x = 0.5*dx; x <= 1.; x += dx) {

          double pt = -log(x);

          double eps = 1e-8;
          double l = -4. - m_EtaMax, r = 4. + m_EtaMax;
          double m1 = l + (r - l) / 3.;
          double m2 = r - (r - l) / 3.;
          int MAXITERS = 200;
          int iter = 0;
          while (fabs(m2 - m1) > eps && iter < MAXITERS) {
            if (m_distr.dndysingle(m1, pt) < m_distr.dndysingle(m2, pt)) {
              l = m1;
            }
            else {
              r = m2;
            }
            m1 = l + (r - l) / 3.;
            m2 = r - (r - l) / 3.;
            iter++;
          }
          m_MaxYs.push_back(m_distr.dndysingle((m1 + m2) / 2., pt));

          m_dndy.push_back(SplineFunction());
          double dy = m_dy;
          for (double ty = -4. + 0.5*dy; ty <= 4.; ty += dy) {
            m_dndy[m_dndy.size() - 1].add_val(ty, m_distr.dndysingle(ty, pt));
          }
        }
      }

    }

    void SSHMomentumGenerator::FindMaximumPt() {

      double eps = 1e-8;
      double l = 0., r = 1.;
      double m1 = l + (r - l) / 3.;
      double m2 = r - (r - l) / 3.;
      int MAXITERS = 200;
      int iter = 0;
      while (fabs(m2 - m1) > eps && iter < MAXITERS) {
        if (g(m1) < g(m2)) {
          l = m1;
        }
        else {
          r = m2;
        }
        m1 = l + (r - l) / 3.;
        m2 = r - (r - l) / 3.;
        iter++;
      }
      m_MaxPt = g((m1 + m2) / 2.);

      m_dndpt.clearall();
      double dx = 0.05;
      for (double x = 0.5*dx; x <= 1.; x += dx) {
        m_dndpt.add_val(x, g(x));
      }
    }

    void SSHMomentumGenerator::FindMaximumY(double pt) {
      double eps = 1e-8;
      double l = -4. - m_EtaMax, r = 4. + m_EtaMax;
      double m1 = l + (r - l) / 3.;
      double m2 = r - (r - l) / 3.;
      int MAXITERS = 200;
      int iter = 0;
      while (fabs(m2 - m1) > eps && iter < MAXITERS) {
        if (m_distr.dndy(m1, pt) < m_distr.dndy(m2, pt)) {
          l = m1;
        }
        else {
          r = m2;
        }
        m1 = l + (r - l) / 3.;
        m2 = r - (r - l) / 3.;
        iter++;
      }
      m_MaxY = m_distr.dndy((m1 + m2) / 2., pt);
    }

    void SSHMomentumGenerator::FindMaximumY2(double pt) {
      double eps = 1e-8;
      double l = -4. - m_EtaMax, r = 4. + m_EtaMax;
      double m1 = l + (r - l) / 3.;
      double m2 = r - (r - l) / 3.;
      int MAXITERS = 200;
      int iter = 0;
      while (fabs(m2 - m1) > eps && iter < MAXITERS) {
        if (m_distr.dndysingle(m1, pt) < m_distr.dndysingle(m2, pt)) {
          l = m1;
        }
        else {
          r = m2;
        }
        m1 = l + (r - l) / 3.;
        m2 = r - (r - l) / 3.;
        iter++;
      }
      m_MaxY = m_distr.dndysingle((m1 + m2) / 2., pt);
    }

    std::pair<double, double> SSHMomentumGenerator::GetRandom() {
      double tpt = 0., ty = 0.;
      while (1) {
        double x0 = randgenMT.randDblExc();
        double y0 = m_MaxPt * randgenMT.randDblExc();
        if (y0 < g2(x0)) {
          tpt = -log(x0);
          break;
        }
      }
      while (1) {
        int ind = (int)(exp(-tpt) / m_dPt);
        if (ind < 0) ind = 0;
        if (ind >= static_cast<int>(m_dndy.size())) ind = m_dndy.size() - 1;
        double x0 = -4. - m_EtaMax + (8. + 2. * m_EtaMax) * randgenMT.randDblExc();
        double y0 = m_MaxYs[ind] * randgenMT.randDblExc();
        if (y0 < m_dndy[ind].f(x0)) {
          ty = x0;
          break;
        }
      }
      return std::make_pair(tpt, ty);
    }

    std::pair<double, double> SSHMomentumGenerator::GetRandom2() const {
      double tpt = 0., ty = 0., teta = 0.;
      while (1) {
        double x0 = randgenMT.randDblExc();
        double y0 = m_MaxPt * randgenMT.randDblExc();
        if (y0 < g2(x0)) {
          tpt = -log(x0);
          break;
        }
      }
      while (1) {
        int ind = (int)(exp(-tpt) / m_dPt);
        if (ind < 0) ind = 0;
        if (ind >= static_cast<int>(m_dndy.size())) ind = m_dndy.size() - 1;
        double x0 = -4. + (8.) * randgenMT.randDblExc();
        double y0 = m_MaxYs[ind] * randgenMT.randDblExc();

        if (y0 < m_dndy[ind].f(x0)) {
          ty = x0;
          teta = -m_EtaMax + 2. * m_EtaMax * randgenMT.randDblExc();
          break;
        }
      }
      return std::make_pair(tpt, ty - teta);
    }

    std::vector<double> SSHMomentumGenerator::GetMomentum() const {
      std::vector<double> ret(0);
      std::pair<double, double> pty = GetRandom2();
      double tpt = pty.first;
      double ty = pty.second;
      double tphi = 2. * xMath::Pi() * randgenMT.rand();
      ret.push_back(tpt*cos(tphi));                          //px
      ret.push_back(tpt*sin(tphi));                          //py
      ret.push_back(sqrt(tpt*tpt + m_Mass * m_Mass)*sinh(ty)); //pz
      return ret;
    }

    double BreitWignerGenerator::f(double x) const {
      return x / ((x*x - m_M * m_M)*(x*x - m_M * m_M) + m_M * m_M*m_Gamma*m_Gamma);
    }

    void BreitWignerGenerator::FixParameters() {
      double eps = 1e-8;
      double l = m_Mthr, r = m_M + 2.*m_Gamma;
      double m1 = l + (r - l) / 3.;
      double m2 = r - (r - l) / 3.;
      int MAXITERS = 200;
      int iter = 0;
      while (fabs(m2 - m1) < eps && iter < MAXITERS) {
        if (f(m1) < f(m2)) {
          l = m1;
        }
        else {
          r = m2;
        }
        m1 = l + (r - l) / 3.;
        m2 = r - (r - l) / 3.;
        iter++;
      }
      m_Max = f((m1 + m2) / 2.);
    }

    double BreitWignerGenerator::GetRandom() const {
      //bool fl = true;
      if (m_Gamma < 1e-7) return m_M;
      while (1) {
        double x0 = m_Mthr + (m_M + 2.*m_Gamma - m_Mthr) * randgenMT.rand();
        double y0 = m_Max * randgenMT.rand();
        if (y0 < f(x0)) return x0;
      }
      return 0.;
    }

    void BreitWignerGenerator::SetParameters(double M, double gamma, double mthr) {
      m_M = M;
      m_Gamma = gamma;
      m_Mthr = mthr;
      FixParameters();
    }

    void ThermalBreitWignerGenerator::SetParameters(ThermalParticle *part, double T, double Mu)
    {
      m_part = part;
      m_T = T;
      m_Mu = Mu;
      FixParameters();
    }

    void ThermalBreitWignerGenerator::FixParameters()
    {
      double Threshold = m_part->DecayThresholdMass();
      double Width = m_part->ResonanceWidth();
      double Mass = m_part->Mass();

      double a = std::max(Threshold, Mass - 2.*Width);
      double b = Mass + 2.*Width;

      m_Xmin = a;
      m_Xmax = b + (b - a)*0.2;

      m_Max = 0.;

      int iters = 1000;
      double dM = (m_Xmax - m_Xmin) / (iters - 1.);
      for (int i = 0; i < iters; ++i) {
        double tM = m_Xmin + dM * i;
        m_Max = std::max(m_Max, f(tM));
      }
      m_Max *= 1.2;
    }

    double ThermalBreitWignerGenerator::f(double M) const
    {
      return m_part->ThermalMassDistribution(M, m_T, m_Mu, m_part->ResonanceWidth());
    }

    double ThermalBreitWignerGenerator::GetRandom() const
    {
      if (m_part->ResonanceWidth() / m_part->Mass() < 1.e-2)
        return m_part->Mass();
      while (true) {
        double x0 = m_Xmin + (m_Xmax - m_Xmin) * randgenMT.rand();
        double y0 = m_Max * randgenMT.rand();
        if (y0 < f(x0)) return x0;
      }
      return 0.;
    }

    void ThermalEnergyBreitWignerGenerator::FixParameters()
    {
      double Threshold = m_part->DecayThresholdMass();
      double Width = m_part->ResonanceWidth();
      double Mass = m_part->Mass();

      double a = std::max(Threshold, Mass - 2.*Width);
      double b = Mass + 2.*Width;

      if (m_part->Decays().size() == 0)
        a = Mass - 2.*Width + 1.e-6;
      else
        a = m_part->DecayThresholdMassDynamical();

      b = Mass + 2.*Width;

      m_Xmin = a;
      m_Xmax = b + 0.2 * (b - a);

      //if (m_part->PdgId() == 32214)
      //  printf("%lf %lf\n", m_Xmin, m_Xmax);

      m_Max = 0.;

      int iters = 1000;
      double dM = (m_Xmax - m_Xmin) / (iters - 1.);
      for (int i = 0; i < iters; ++i) {
        double tM = m_Xmin + dM * i;
        m_Max = std::max(m_Max, f(tM));
      }
      m_Max *= 1.2;
    }

    double ThermalEnergyBreitWignerGenerator::f(double M) const
    {
      return m_part->ThermalMassDistribution(M, m_T, m_Mu, m_part->TotalWidtheBW(M));
    }

    //double BesselDistributionGenerator::pn(int n, double a, int nu)
    //{
    //  if (nu < 0 || n < 0) return 0.0;
    //  double nfact = 1., nnufact = 1.;
    //  for (int i = 1; i <= n; ++i)
    //    nfact *= i;

    //  nnufact = nfact;
    //  for (int i = n + 1; i <= n + nu; ++i)
    //    nnufact *= i;

    //  return pow(a / 2., 2 * n + nu) / xMath::BesselI(nu, a) / nfact / nnufact;
    //}

    double BesselDistributionGenerator::pn(int n, double a, int nu)
    {
      if (nu < 0 || n < 0) return 0.0;

      double logret = (2. * n + nu) * log(a/2.) - a;
      for (int i = 1; i <= n; ++i)
        logret -= 2. * log(i);
      for (int i = n + 1; i <= n + nu; ++i)
        logret -= log(i);

      return exp(logret) / xMath::BesselIexp(nu, a);
    }

    double BesselDistributionGenerator::R(double x, int nu)
    {
      double hn2 = 1., hn1 = 0., hn = 0.;
      double kn2 = 0., kn1 = 1., kn = 0.;
      int nmax = 20;
      for (int n = 1; n <= nmax; ++n) {
        double an = 2. * (nu + n) / x;
        hn = an * hn1 + hn2;
        kn = an * kn1 + kn2;
        
        hn2 = hn1;
        hn1 = hn;

        kn2 = kn1;
        kn1 = kn;

        if (n == nmax && nmax <= 1000 && abs(hn2 / kn2 - hn1 / kn1) > 1.e-9)
          nmax *= 2;

        if (n == 1000) {
          printf("**WARNING** BesselDistributionGenerator::R(x,nu): Reached maximum iterations...\n");
        }
      }
      return hn / kn;
    }

    double BesselDistributionGenerator::chi2(double a, int nu)
    {
      return mu(a, nu) + (1. / 4.) * a * a * R(a, nu) * (R(a,nu+1) - R(a,nu));
    }

    double BesselDistributionGenerator::sig2(double a, int nu)
    {
      double tmp = m(a, nu) - mu(a, nu);
      return chi2(a, nu) + tmp * tmp;
    }

    double BesselDistributionGenerator::Q2(double a, int nu)
    {
      double A = sqrt(a*a + nu*nu);
      double B = sqrt(a*a + (nu+1.)*(nu+1.));
      double tmp = 1. + a * a * (1. + B - A) / 2. / (nu + A) / (nu + 1. + B);
      return a*a / 2. / (nu + A) + tmp * tmp;
    }

    int BesselDistributionGenerator::RandomBesselPoisson(double a, int nu, MTRand & rangen)
    {
      if (nu < 0 || a <= 0.) return 0;

      while (true) {
        int n1 = RandomPoisson(a / 2., rangen);
        int n2 = RandomPoisson(a / 2., rangen);
        if (n1 - n2 == nu)
          return n2;
      }

      return 0;
    }

    //double BesselDistributionGenerator::pmXmOverpm(int X, int tm, double a, int nu) {
    //  double tf1 = 1., tf2 = 1.;
    //  if (X > 0) {
    //    for (int i = 1; i <= X; ++i) {
    //      tf1 *= tm + i;
    //      tf2 *= tm + nu + i;
    //    }
    //  }
    //  else if (X < 0) {
    //    for (int i = 1; i <= -X; ++i) {
    //      tf1 *= tm + X + i;
    //      tf2 *= tm + nu + X + i;
    //    }
    //    tf1 = 1. / tf1;
    //    tf2 = 1. / tf2;
    //  }

    //  return pow(a / 2., 2 * X) / tf1 / tf2;
    //}

    double BesselDistributionGenerator::pmXmOverpm(int X, int tm, double a, int nu) {
      double ret = 1.;
      if (X > 0) {
        for (int i = 1; i <= X; ++i) {
          ret *= (a / 2.)/(tm + i);
          ret *= (a / 2.)/(tm + nu + i);
        }
      }
      else if (X < 0) {
        for (int i = 1; i <= -X; ++i) {
          ret *= (tm + X + i) / (a / 2.);
          ret *= (tm + nu + X + i) / (a / 2.);
        }
      }

      return ret;
    }

    int BesselDistributionGenerator::RandomBesselDevroye1(double a, int nu, MTRand & rangen)
    {
      int tm = m(a, nu);
      double pm = pn(tm, a, nu);
      double w = 1. + pm / 2.;
      while (true) {
        double U = rangen.rand();
        double W = rangen.rand();
        int S = 1;
        if (rangen.rand() < 0.5)
          S = -1;

        double Y = 0.;
        if (U <= w / (1. + w)) {
          double V = rangen.rand();
          Y = V * w / pm;
        }
        else {
          double E = -log(rangen.randDblExc());
          Y = (w + E) / pm;
        }
        int X = S * std::lround(Y);

        if (tm + X < 0)
          continue;

        double pratio = pmXmOverpm(X, tm, a, nu);

        if (pratio != pratio) {
          printf("**WARNING** BesselDistributionGenerator::RandomBesselDevroye1: Float problem!");
          continue;
        }

        if (W * std::min(1., exp(w - pm * Y)) <= pratio)
          return tm + X;
      }
      return 0;
    }

    int BesselDistributionGenerator::RandomBesselDevroye2(double a, int nu, MTRand & rangen)
    {
      int tm = m(a, nu);
      double tsig = sqrt(sig2(a, nu));
      double q = std::min(1. / tsig / sqrt(648.), 1. / 3.);
      while (true) {
        double U = rangen.rand();
        double W = rangen.rand();
        int S = 1;
        if (rangen.rand() < 0.5)
          S = -1;

        double Y = 0.;
        if (U <= (1. + 2. / q) / (1. + 4. / q)) {
          double V = rangen.rand();
          Y = V * (1. / 2. + 1. / q);
        }
        else {
          double E = -log(rangen.randDblExc());
          Y = 1. / 2. + 1. / q + E / q;
        }
        int X = S * std::lround(Y);

        if (tm + X < 0)
          continue;

        double pratio = pmXmOverpm(X, tm, a, nu);

        if (pratio != pratio) {
          printf("**WARNING** BesselDistributionGenerator::RandomBesselDevroye2: Float problem!");
          continue;
        }

        if (W * std::min(1., exp(1. + q/2. - q*Y)) <= pratio)
          return tm + X;
      }
      return 0;
    }

    int BesselDistributionGenerator::RandomBesselDevroye3(double a, int nu, MTRand & rangen)
    {
      int tm = m(a, nu);
      double tQ = sqrt(Q2(a, nu));
      double q = std::min(1. / tQ / sqrt(648.), 1. / 3.);
      while (true) {
        double U = rangen.rand();
        double W = rangen.rand();
        int S = 1;
        if (rangen.rand() < 0.5)
          S = -1;

        double Y = 0.;
        if (U <= (1. + 2. / q) / (1. + 4. / q)) {
          double V = rangen.rand();
          Y = V * (1. / 2. + 1. / q);
        }
        else {
          double E = -log(rangen.randDblExc());
          Y = 1. / 2. + 1. / q + E / q;
        }
        int X = S * std::lround(Y);

        if (tm + X < 0)
          continue;

        double pratio = pmXmOverpm(X, tm, a, nu);

        if (pratio != pratio) {
          printf("**WARNING** BesselDistributionGenerator::RandomBesselDevroye3: Float problem!");
          continue;
        }


        if (W * std::min(1., exp(1. + q/2. - q*Y)) <= pratio)
          return tm + X;
      }
      return 0;
    }

    int BesselDistributionGenerator::RandomBesselNormal(double a, int nu, MTRand & rangen)
    {
      double ret = -1.;
      while (ret <= -0.5) {
        ret = rangen.randNorm(mu(a,nu), sqrt(chi2(a,nu)));
      }
      return static_cast<int>(ret + 0.5);
    }

    int BesselDistributionGenerator::RandomBesselCombined(double a, int nu, MTRand & rangen)
    {
      int tm = m(a, nu);
      if (tm < 6) {
        if (nu <= a)
          return RandomBesselPoisson(a, nu, rangen);
        else
          return RandomBesselDevroye3(a, nu, rangen);
      }
      else
        return RandomBesselNormal(a, nu, rangen);
    }

}

} // namespace thermalfist
