#ifndef CVSS_H
#define CVSS_H

namespace ns3
{   
    const int CVSSMetricsNumber = 22;
    enum CVSSMetric
    {
        AV, AC, PR, UI, S, C, I, A,
        E, RL, RC,
        CR, IR, AR,
        MAV, MAC, MPR, MUI, MS, MC, MI, MA
    };

    enum class CVSSMetricAV {Network, Adjacent, Local, Physical, NotDefined};
    enum class CVSSMetricAC {High, Low, NotDefined};
    enum class CVSSMetricPRCIA {None,  Low, High, NotDefined};
    enum class CVSSMetricUI {None, Required, NotDefined};
    enum class CVSSMetricS {Unchanged, Changed, NotDefined};

    enum class CVSSMetricE {NotDefined, Unproven, ProofOfConcept, Functional,High};
    enum class CVSSMetricRL {NotDefined, OfficialFix, TemporaryFix, Workaround, Unavailable};
    enum class CVSSMetricRC {NotDefined, Unknown, Reasonable, Confirmed};

    enum class CVSSMetricCIAR {NotDefined, Low, Medium, High};

    class CVSS
    {
    private:
        bool re_calc; // 是否重新计算
        double m_scores[3] = {0.0, 0.0, 0.0};
        int metrics[CVSSMetricsNumber];
        int M2I(CVSSMetric m);
        double GetWeight(CVSSMetric m);

        static double exp_coef;
        static double scope_coef;
        static double weight[11][5];
        static  inline double min(double a, double b);
        static double round_up1(double input);
    public:
        CVSS();
        CVSS(CVSSMetricAV av, CVSSMetricAC ac, CVSSMetricPRCIA pr, CVSSMetricUI ui, CVSSMetricS s, CVSSMetricPRCIA c, CVSSMetricPRCIA i, CVSSMetricPRCIA a,
        CVSSMetricE e, CVSSMetricRL rl, CVSSMetricRC rc, CVSSMetricCIAR cr, CVSSMetricCIAR ir, CVSSMetricCIAR ar,
        CVSSMetricAV mav, CVSSMetricAC mac, CVSSMetricPRCIA mpr, CVSSMetricUI mui, CVSSMetricS ms, CVSSMetricPRCIA mc, CVSSMetricPRCIA mi, CVSSMetricPRCIA ma);
        ~CVSS();
        void CalcScore(double scores[3]);
    };
}
#endif