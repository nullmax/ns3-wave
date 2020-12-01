#include "cvss.h"
#include <cmath>

using namespace ns3;

double CVSS::exp_coef = 8.22;
double CVSS::scope_coef = 1.08;
double CVSS::weight[11][5] = {{0.85, 0.62, 0.55, 0.2},
                                                               {0.44, 0.77},
                                                               {0.85, 0.62, 0.27},
                                                               {0.85, 0.68, 0.5},
                                                               {0.85, 0.62},
                                                               {6.42, 7.52},
                                                               {0, 0.22, 0.56},
                                                               {1, 0.91, 0.94, 0.97, 1},
                                                               {1, 0.95, 0.96, 0.97, 1},
                                                               {1, 0.92, 0.96, 1},
                                                               {1, 0.5, 1, 1.5}};
                                                        
 CVSS::CVSS()
{   
}

CVSS::CVSS(CVSSMetricAV av, CVSSMetricAC ac, CVSSMetricPRCIA pr, CVSSMetricUI ui, CVSSMetricS s, CVSSMetricPRCIA c, CVSSMetricPRCIA i, CVSSMetricPRCIA a,
        CVSSMetricE e, CVSSMetricRL rl, CVSSMetricRC rc, CVSSMetricCIAR cr, CVSSMetricCIAR ir, CVSSMetricCIAR ar,
        CVSSMetricAV mav, CVSSMetricAC mac, CVSSMetricPRCIA mpr, CVSSMetricUI mui, CVSSMetricS ms, CVSSMetricPRCIA mc, CVSSMetricPRCIA mi, CVSSMetricPRCIA ma)
{
    re_calc = true;
    metrics[AV] = static_cast<int>(av);
    metrics[AC] = static_cast<int>(ac);
    metrics[PR] = static_cast<int>(pr);
    metrics[UI] = static_cast<int>(ui);
    metrics[S] = static_cast<int>(s);
    metrics[C] = static_cast<int>(c);
    metrics[I] = static_cast<int>(i);
    metrics[A] = static_cast<int>(a);

    metrics[E] = static_cast<int>(e);
    metrics[RL] = static_cast<int>(rl);
    metrics[RC] = static_cast<int>(rc);

    metrics[CR] = static_cast<int>(cr);
    metrics[IR] = static_cast<int>(ir); 
    metrics[AR] = static_cast<int>(ar);

    metrics[MAV] = mav == CVSSMetricAV::NotDefined ? metrics[AV] : static_cast<int>(mav);
    metrics[MAC] = mac == CVSSMetricAC::NotDefined ? metrics[AC] : static_cast<int>(mac);
    metrics[MPR] = mpr == CVSSMetricPRCIA::NotDefined ? metrics[PR] : static_cast<int>(mpr); 
    metrics[MUI] = mui == CVSSMetricUI::NotDefined ? metrics[UI] : static_cast<int>(mui);
    metrics[MS] =  ms == CVSSMetricS::NotDefined ? metrics[S] :static_cast<int>(ms); 
    metrics[MC] =  mc == CVSSMetricPRCIA::NotDefined ? metrics[C] :static_cast<int>(mc); 
    metrics[MI] =  mi == CVSSMetricPRCIA::NotDefined ? metrics[I] :static_cast<int>(mi);
    metrics[MA] =  ma == CVSSMetricPRCIA::NotDefined ? metrics[A] :static_cast<int>(ma);
   }

CVSS::~CVSS()
{
}

int CVSS::M2I(CVSSMetric m)
{
    switch (m)
    {
    case AV: return 0;
    case AC: return 1;
    case PR: return static_cast<CVSSMetricS>(metrics[S]) == CVSSMetricS::Unchanged ? 2 : 3;
    case UI: return 4;
    case S : return 5;
    case C:
    case I:
    case A: return 6;
    case E: return 7;
    case RL: return 8;
    case RC: return 9;
    case CR:
    case IR:
    case AR: return 10;
    case MAV: return 0;
    case MAC: return 1;
    case MPR: return static_cast<CVSSMetricS>(metrics[MS]) == CVSSMetricS::Unchanged ? 2 : 3;
    case MUI: return 4;
    case MS : return 5;
    case MC:
    case MI:
    case MA: return 6;
    default:
        return -1;
    }
}

double CVSS::GetWeight(CVSSMetric m)
{   
    return weight[M2I(m)][metrics[m]];
}

double CVSS::round_up1(double input)
{
    int int_input =  round(input * 1e5);
    if (int_input % 10000 == 0)
    {
        return int_input / 1e5;
    }
    else
    {
        return (floor(int_input / 1e4) + 1) / 10;
    }
}
    

inline double CVSS::min(double a, double b)
{
    return a < b ? a : b;
}

void CVSS::CalcScore(double scores[3])
{   
    if (!re_calc)
    {
        scores[0]  = m_scores[0];
        scores[1]  = m_scores[1];
        scores[2]  = m_scores[2];
        return;
    }

    const double eps = 1e-5;
    CVSSMetricS scope_cast = static_cast<CVSSMetricS>(metrics[S]);

    // base score
    double iss = 1 - ((1 - GetWeight(C)) * (1 - GetWeight(I)) * (1 - GetWeight(A)));
    double impact = 0.0;
    if (scope_cast == CVSSMetricS::Unchanged) 
    {
        impact = GetWeight(S) * iss;
    }
    else
    {
        impact = GetWeight(S) * (iss - 0.029) - 3.25 * pow(iss - 0.02, 15);
    }

    double exp = exp_coef * GetWeight(AV) * GetWeight(AC) * GetWeight(PR) * GetWeight(UI);

    if (impact < 0 || impact < eps )
    {
        m_scores[0] = 0;
    }
    else
    {
        if (scope_cast == CVSSMetricS::Unchanged)
        {
            m_scores[0] = round_up1(min(exp + impact, 10));
        }
        else
        {
            m_scores[0] = round_up1(min(scope_coef * (exp + impact), 10));
        }
    }
    
    // temportal score
    m_scores[1] = round_up1(m_scores[0] * GetWeight(E) * GetWeight(RL) * GetWeight(RC));

    // evnironmental score
    double miss = min(1 - (
        (1 - GetWeight(MC) * GetWeight(CR)) * 
        (1 - GetWeight(MI) * GetWeight(IR)) *
        (1 - GetWeight(MA) * GetWeight(AR))
    ), 0.915);
    CVSSMetricS m_scope_cast = static_cast<CVSSMetricS>(metrics[MS]);
    double m_impact = 0;
    if (m_scope_cast == CVSSMetricS::Unchanged)
    {
        m_impact = GetWeight(MS) * miss;
    }
    else
    {
        m_impact = GetWeight(MS) * (miss - 0.029) - 3.25 * pow(miss * 0.9731 - 0.02, 13);
    }
    
    double m_exp = exp_coef * GetWeight(MAV) * GetWeight(MAC) * GetWeight(MPR) * GetWeight(MUI);

    if (m_impact < 0 || m_impact < eps)
    {
        m_scores[2] = 0;
    }
    else if (m_scope_cast == CVSSMetricS::Unchanged)
    {
        m_scores[2] = round_up1(
            round_up1(min(m_impact + m_exp, 10)) * 
            GetWeight(E) * GetWeight(RL) * GetWeight(RC)
        );
    }
    else
    {
        m_scores[2] = round_up1(
            round_up1(min(scope_coef * (m_impact + m_exp), 10)) * 
            GetWeight(E) * GetWeight(RL) * GetWeight(RC)
        );
    }
    scores[0] = m_scores[0];
    scores[1] = m_scores[1];
    scores[2] = m_scores[2];
    re_calc = false;
}