#include <math.h>

#include <Eigen/Dense>
#include <Eigen/Geometry>
#include <Eigen/Eigenvalues>


#include <iostream>

using namespace Eigen;
using namespace std;

namespace Kronrod
{
Array<double,Dynamic,2> multiPrecisionKronrod(const unsigned int nNodes);
Array<double,Dynamic,2> jacobiRecurrenceCoeff(const unsigned int nNodes);
Array<double,Dynamic,2> jacobiRecurrenceCoeff(const unsigned int nNodes,double alpha);
Array<double,Dynamic,2> jacobiRecurrenceCoeff(const unsigned int nNodes, double alpha, double beta);
Array<double,Dynamic,2> jacobiRecurrenceCoeffZeroToOne(const unsigned int nNodes);
Array<double,Dynamic,2> jacobiRecurrenceCoeffZeroToOne(const unsigned int nNodes,double alpha);
Array<double,Dynamic,2> jacobiRecurrenceCoeffZeroToOne(const unsigned int nNodes, double alpha, double beta);
Array<double,Dynamic,2> kronrod(const unsigned int nNodes, Array<double,Dynamic,2> ab);
Array<double,Dynamic,2> kronrodRecurrenceCoeff(const unsigned int nNodes, Array<double,Dynamic,2> ab0);
}

/**
*   multiPrecisionKronrod Arbitrary precision Kronrod abscissae & weights.
*
*   xGK = multiPrecisionKronrod(N) computes Kronrod points for (-1,1) with any required precision
*
*   Based on work of Dirk Laurie and Walter Gautschi.
*   Created by Pavel Holoborodko, November 7, 2011.
*   Ported to C++/Eigen Grey Point Corporation September 2014
*/
Array<double,Dynamic,2> Kronrod::multiPrecisionKronrod(const unsigned int nNodes)
{
    Array<double,Dynamic,2> alphaBeta = jacobiRecurrenceCoeffZeroToOne(2 * nNodes);
    Array<double,Dynamic,2> xwGK = kronrod(nNodes, alphaBeta);

    Array<double,Dynamic,2> xGK = ArrayX2d::Zero(2 * nNodes + 1, 2);
    xGK.col(0) = 2. * xwGK.col(0) - 1.;
    xGK.col(1) = 2. * xwGK.col(1);
    return xGK;
}

/**
*   kronrod Gauss-Kronrod quadrature formula.
*
*   xwGK = kronrod(n, alphaBeta) generates the (2n+1)-point Gauss-Kronrod
*   quadrature rule for the weight function w encoded by the
*   recurrence matrix alphaBeta of order [ceil(3*n/2)+1]x2 containing
*   in its first and second column respectively the alpha- and
*   beta-coefficients in the three-term recurrence relation
*   for w. The 2n+1 nodes, (abscissae), in increasing order, are output
*   into the first column. The corresponding weights are output to the
*   second column of the (2n+1)x2 array xwGK.
*
*   Created by Dirk Laurie, June 22, 1998.
*   Edited by Pavel Holoborodko, November 7, 2011:
*   Ported to C++/Eigen Grey Point Corporation September 2014
*/
Array<double,Dynamic,2> Kronrod::kronrod(const unsigned int nNodes, Array<double,Dynamic,2> alphaBeta)
{
    Array<double,Dynamic,2> ab0 = kronrodRecurrenceCoeff(nNodes, alphaBeta);
    MatrixXd J = MatrixXd::Zero(2*nNodes + 1, 2*nNodes + 1);

    for(size_t k = 0; k < 2 * nNodes; ++k)
    {
        J(k,k) = ab0(k,0);
        J(k,k + 1) = sqrt(ab0(k + 1, 1));
        J(k + 1, k) = J(k, k + 1);
    }

    J(2 * nNodes, 2 * nNodes) = ab0(2 * nNodes, 0);

    EigenSolver<MatrixXd> eigenSol(J);
    VectorXcd d = eigenSol.eigenvalues();
    MatrixXcd V = eigenSol.eigenvectors();

    //Insertion sort
    bool sorted = false;
    int i = 0;
    while(!sorted)
    {
        double di = d(i).real();
        double di1 = d(i+1).real();
        if(di1 < di)
        {
            complex<double> tmpD = d(i);
            d(i) = d(i+1);
            d(i+1) = tmpD;
            VectorXcd tmpV = V.col(i);
            V.col(i) = V.col(i+1);
            V.col(i+1) = tmpV;
            i = max(i-1, 0);
            continue;
        }
        else
        {
            ++i;
            if(i == d.size() - 1)
            {
                sorted = true;
            }
        }
    }

    ArrayXd tempV = V.real().row(0).array();
    ArrayXd e = ab0(0,1) * tempV * tempV;

    Array<double,Dynamic,2> xwGK = ArrayX2d::Zero(2*nNodes + 1, 2);
    xwGK.col(0) = d.real();
    xwGK.col(1) = e;

    return xwGK;
}

/**
*   jacobiRecurrenceCoeff Recurrence coefficients for monic Jacobi polynomials.
*
*   alphaBeta = jacobiRecurrenceCoeff(n, alpha, beta) generates the first n
*   recurrence coefficients for monic Jacobi polynomials with parameters
*   alpha and beta. These are orthogonal on [-1,1] relative to the
*   weight function w(t) = (1 - t)^a(1 + t)^b. The n alpha-coefficients
*   are stored in the first column, the n beta-coefficients in the second column,
*   of the nx2 array ab. The call ab = jacobiRecurrenceCoeff(n,a)
*   is the same as ab = jacobiRecurrenceCoeff(n,a,a) and
*   ab = jacobiRecurrenceCoeff(n) is the same as ab = jacobiRecurrenceCoeff(n,0,0).
*
*   Created by Dirk Laurie, 6-22-1998; edited by Walter Gautschi, 4-4-2002.
*   Ported to C++/Eigen Grey Point Corporation September 2014
*/

Array<double,Dynamic,2> Kronrod::jacobiRecurrenceCoeff(const unsigned int nNodes)
{
    return jacobiRecurrenceCoeff(nNodes, 0, 0);
}

Array<double,Dynamic,2> Kronrod::jacobiRecurrenceCoeff(const unsigned int nNodes, double alpha)
{
    return jacobiRecurrenceCoeff(nNodes, alpha, alpha);
}

Array<double,Dynamic,2> Kronrod::jacobiRecurrenceCoeff(const unsigned int nNodes, double alpha, double beta)
{
    double nu = (beta - alpha) / (alpha + beta + 2.);
    double mu = pow(2, alpha + beta + 1) * tgamma(alpha + 1) * tgamma(beta + 1)
                / tgamma(alpha + beta + 2);

    if (nNodes == 1)
    {
        Array<double,Dynamic,2> alphaBeta = ArrayXXd::Zero(1,2);
        alphaBeta(0,0) = nu;
        alphaBeta(0,1) = mu;
        return alphaBeta;
    }

    double  nAlphaBeta;
    ArrayXd A(nNodes);
    ArrayXd B(nNodes);

    for(size_t n = 1; n < nNodes; ++n)
    {
        nAlphaBeta = 2. * n + alpha + beta;
        A(n) = (pow(beta, 2.) - pow(alpha, 2.)) / (nAlphaBeta * (nAlphaBeta + 2.));

        B(n) = 4. * (alpha + n) * (beta + n) * n * (alpha + beta + n)
            / ((nAlphaBeta * nAlphaBeta) * (nAlphaBeta + 1.) * (nAlphaBeta - 1.));
    }

    B(0) = mu;
    B(1) = 4. * (alpha + 1) * (beta + 1) / (pow(alpha + beta + 2, 2) * (alpha + beta + 3));

    Array<double,Dynamic,2> alphaBeta = ArrayXXd::Zero(nNodes,2);
    alphaBeta.col(0) = A;
    alphaBeta.col(1) = B;

    return alphaBeta;
}

/**
*   jacobiRecurrenceCoeffZeroToOne Recurrence coefficients for monic Jacobi polynomials on [0,1].
*
*   alphaBeta = jacobiRecurrenceCoeffZeroToOne(n, a, b) generates the first n recurrence
*   coefficients for monic Jacobi polynomials on [0,1] with parameters alpha and beta.
*
*   These are orthogonal on [0,1] relative to the weight function w(t) = (1-t)^a t^b.
*   The n alpha-coefficients are stored in the first column, the n beta-coefficients
*   in the second column, of the nx2 array alphaBeta. The call
*   alphaBeta = jacobiRecurrenceCoeffZeroToOne(n, alpha) is the same as the call
*   alphaBeta = jacobiRecurrenceCoeffZeroToOne(n, alpha, alpha), and the call
*   alphaBeta = jacobiRecurrenceCoeffZeroToOne(n) is the same as the call
*   alphaBeta = jacobiRecurrenceCoeffZeroToOne(n, 0, 0).
*
*   Created by Dirk Laurie, 6-22-1998; edited by Walter Gautschi, 4-4-2002.
*   Ported to C++/Eigen Grey Point Corporation September 2014
*/
Array<double,Dynamic,2> Kronrod::jacobiRecurrenceCoeffZeroToOne(const unsigned int nNodes)
{
    return jacobiRecurrenceCoeffZeroToOne(nNodes,0,0);
}

Array<double,Dynamic,2> Kronrod::jacobiRecurrenceCoeffZeroToOne(const unsigned int nNodes, double alpha)
{
    return jacobiRecurrenceCoeffZeroToOne(nNodes, alpha, alpha);
}

Array<double,Dynamic,2> Kronrod::jacobiRecurrenceCoeffZeroToOne(const unsigned int nNodes, double alpha, double beta)
{
    Array<double,Dynamic,2> coeffs = jacobiRecurrenceCoeff(nNodes, alpha, beta);
    Array<double,Dynamic,2> alphaBeta = ArrayXXd::Zero(nNodes, 2);

    for (size_t i = 0; i < nNodes; ++i)
    {
        alphaBeta(i, 0) = (1 + coeffs(i, 0)) / 2;
    }

    alphaBeta(0, 1) = coeffs(0, 1) / pow(2., alpha + beta + 1);

    for (size_t i = 1; i < nNodes; ++i)
    {
        alphaBeta(i, 1) = coeffs(i, 1) / 4.;
    }

    return alphaBeta;
}

/**
*   kronrodRecurrenceCoeff Jacobi-Kronrod matrix.
*
*   alphaBeta = kronrodRecurrenceCoeff(n,ab0) produces the alpha- and beta-elements in
*   the Jacobi-Kronrod matrix of order 2n+1 for the weight
*   function (or measure) w. The input data for the weight
*   function w are the recurrence coefficients of the associated
*   orthogonal polynomials, which are stored in the array ab0 of
*   dimension [ceil(3*n/2)+1]x2. The alpha-elements are stored in
*   the first column, the beta-elements in the second column, of
*   the (2*n+1)x2 array ab.
*
*   Created by Dirk Laurie, 6-22.1998
*   Edited by Pavel Holoborodko, November 7, 2011:
*   Ported to C++/Eigen Grey Point Corporation September 2014
*/
Array<double,Dynamic,2> Kronrod::kronrodRecurrenceCoeff(const unsigned int nNodes, Array<double,Dynamic,2> ab0)
{
    ArrayXd alpha = ArrayXd::Zero(2 * nNodes + 1);
    ArrayXd beta = ArrayXd::Zero(2 * nNodes + 1);

    double temp = 0.;

    int j = 0;
    int k = 0;
    int l = 0;
    unsigned int m = 0;

    for (k = 0; k < floor(3 * nNodes / 2 + 1); ++k)
    {
        alpha(k) = ab0(k, 0);
    }

    for (k = 0; k < ceil(3 * nNodes / 2 + 1); ++k)
    {
        beta(k) = ab0(k, 1);
    }

    ArrayXd sig = ArrayXd::Zero(floor(nNodes / 2) + 2);
    ArrayXd sigT = ArrayXd::Zero(floor(nNodes / 2) + 2);

    sigT(1) = beta(nNodes + 1);

    // Loop 1
    for(m = 0; m < nNodes - 1; ++m)
    {
        temp = 0.;
        for(k = (m + 1) / 2; k >= 0; --k)
        {
            l = m - k;
            temp += (alpha(k + nNodes + 1) - alpha(l)) * sigT(k + 1)
                    + beta(k + nNodes + 1) * sig(k) - beta(l) * sig(k+1);
            sig(k+1) = temp;
        }

        ArrayXd tempArray = sig;
        sig = sigT;
        sigT = tempArray;
    }

    for(j = nNodes / 2; j >= 0; --j)
    {
        sig(j + 1) = sig(j);
    }

    // Loop 2
    for (m = nNodes - 1; m <= 2 * nNodes - 3; ++m)
    {
        temp = 0.;
        for(k = m - nNodes + 1; k <= floor((m - 1) / 2); ++k)
        {
            l = m - k;
            j = nNodes - l - 1;
            temp = temp - (alpha(k + nNodes + 1) - alpha(l)) * sigT(j+1)
                   - beta(k + nNodes + 1) * sig(j+1)
                   + beta(l) * sig(j+2);
            sig(j+1) = temp;
        }

        if(m % 2 == 0)
        {
            alpha(k + nNodes + 1) = alpha(k) + (sig(j+1) - beta(k + nNodes + 1)
                                    * sig(j + 2)) / sigT(j + 2);
        }
        else
        {
            beta(k + nNodes + 1) = sig(j+1) / sig(j+2);
        }

        ArrayXd tempArray = sig;
        sig = sigT;
        sigT = tempArray;
    }

    alpha(2 * nNodes) = alpha(nNodes - 1) - beta(2 * nNodes) * sig(1) / sigT(1);

    Array<double,Dynamic, 2> alphaBeta = ArrayX2d::Zero(2 * nNodes + 1, 2);
    alphaBeta.col(0) = alpha.array();
    alphaBeta.col(1) = beta.array();

    return alphaBeta;
}