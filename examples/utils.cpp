#include <vector>
#include <iostream>
#include <cmath>
#include <stdlib.h>

#include "utils.h"

using namespace std;

double sign(double x){
    if(x > 0)
        return 1.;
    else if(!x)
        return 0.;
    else
        return -1.;
}

// Compute cos(x +- 2*pi/3) in a more "analytical" way (pm = +- 1)
// Useful for solveCubic
double cosXpm2PI3(double x, double pm){
    return -0.5*( cos(x) + pm * sin(x) * sqrt(3.) );
}

// Finds the real solutions to a*x^2 + b*x + c = 0
// Uses a numerically more stable way than the "classroom" method.
// Handles special cases a=0 and/or b=0.
// Appends the solutions to the std::vector roots, making no attempt to check whether the vector is empty.
// Double roots are present twice.
// If verbose is true (default is false), the solutions are printed, as well as the polynomnial evaluated on these solutions
//
// See https://fr.wikipedia.org/wiki/Équation_du_second_degré#Calcul_numérique
bool solveQuadratic(const double a, const double b, const double c, vector<double>& roots, bool verbose){

	if(!a){
        if(!b){
            if(verbose)
                cout << "No solution to equation " << a << " x^2 + " << b << " x + " << c << endl << endl;
            return false;
        }
		roots.push_back(-c/b);
        if(verbose)
            cout << "Solution of " << b << " x + " << c << ": " << roots[0] << ", test = " << b*roots[0] + c << endl << endl;
		return true;
	}

	const double rho = SQ(b) - 4.*a*c;

	if(rho >= 0.){
        if(b == 0.){
            roots.push_back( sqrt(rho)/(2.*a) );
            roots.push_back( -sqrt(rho)/(2.*a) );
        }else{
		    const double x = -0.5*(b + sign(b)*sqrt(rho));
		    roots.push_back(x/a);
		    roots.push_back(c/x);
        }
        if(verbose){
            cout << "Solutions of " << a << " x^2 + " << b << " x + " << c << ":" << endl;
            for(unsigned short i=0; i<roots.size(); i++)
	            cout << "x" << i << " = " << roots[i] << ", test = " << a*SQ(roots[i]) + b*roots[i] + c << endl;
            cout << endl;
        }
		return true;
	}else{
        if(verbose)
            cout << "No real solutions to " << a << " x^2 + " << b << " x + " << c << endl << endl;
		return false;
    }
}

// Finds the real solutions to a*x^3 + b*x^2 + c*x + d = 0
// Handles special case a=0.
// Appends the solutions to the std::vector roots, making no attempt to check whether the vector is empty.
// Multiple roots appear multiple times.
// If verbose is true (default is false), the solutions are printed, as well as the polynomnial evaluated on these solutions
//
// Inspired by "Numerical Recipes" (Press, Teukolsky, Vetterling, Flannery), 2007 Cambridge University Press
bool solveCubic(const double a, const double b, const double c, const double d, vector<double>& roots, bool verbose){

	if(a == 0)
		return solveQuadratic(b, c, d, roots, verbose);

    const double an = b/a;
    const double bn = c/a;
    const double cn = d/a;

    const double Q = SQ(an)/9. - bn/3.;
    const double R = CB(an)/27. - an*bn/6. + cn/2.;

    if( SQ(R) < CB(Q) ){
        const double theta = acos( R/sqrt(CB(Q)) )/3.;

        roots.push_back( -2. * sqrt(Q) * cos(theta) - an/3. );
        roots.push_back( -2. * sqrt(Q) * cosXpm2PI3(theta, 1.) - an/3. );
        roots.push_back( -2. * sqrt(Q) * cosXpm2PI3(theta, -1.) - an/3. );
    }else{
        const double A = - sign(R) * cbrt( abs(R) + sqrt( SQ(R) - CB(Q) ) );

        double B;

        if(A == 0.)
            B = 0.;
        else
            B = Q/A;

        const double x = A + B - an/3.;

        roots.push_back(x);
        roots.push_back(x);
        roots.push_back(x);
    }
    
    if(verbose){
        cout << "Solutions of " << a << " x^3 + " << b << " x^2 + " << c << " x + " << d << ":" << endl;
        for(unsigned short i=0; i<roots.size(); i++)
	        cout << "x" << i << " = " << roots[i] << ", test = " << a*CB(roots[i]) + b*SQ(roots[i]) + c*roots[i] + d << endl;
        cout << endl;
    }

    return true;
}

// Finds the real solutions to a*x^4 + b*x^3 + c*x^2 + d*x + e = 0
// Handles special case a=0.
// Appends the solutions to the std::vector roots, making no attempt to check whether the vector is empty.
// Multiple roots appear multiple times.
// If verbose is true (default is false), the solutions are printed, as well as the polynomnial evaluated on these solutions
//
// See https://en.wikipedia.org/wiki/Quartic_function#Solving_by_factoring_into_quadratics
//      https://fr.wikipedia.org/wiki/Méthode_de_Descartes
//
// The idea is to make a change of variable to eliminate the term in x^3, which gives a "depressed quartic",
// then to try and factorize this quartic into two quadratic equations, which each then gives up to two roots.
// The factorization relies on solving a cubic equation (which is always possible), 
// then taking the square root of one of these solution (ie there must be a positive solution).
bool solveQuartic(const double a, const double b, const double c, const double d, const double e, vector<double>& roots, bool verbose){
	
	if(!a)
		return solveCubic(b, c, d, e, roots, verbose);

    if(!b && !c && !d){
        roots.push_back(0.);
        roots.push_back(0.);
        roots.push_back(0.);
        roots.push_back(0.);
    }else{
        const double an = b/a;
        const double bn = c/a - (3./8.) * SQ(b/a);
        const double cn = CB(0.5*b/a) - 0.5*b*c/SQ(a) + d/a;
        const double dn = -3.*QU(0.25*b/a) + e/a - 0.25*b*d/SQ(a) + c*SQ(b/4.)/CB(a);

        vector<double> res;
        solveCubic(1., 2.*bn, SQ(bn) - 4.*dn, -SQ(cn), res, verbose);
        unsigned short pChoice = -1;

        for(unsigned short i = 0; i<res.size(); ++i){
            if(res[i] > 0){
                pChoice = i;
                break;
            }
        }

        if(pChoice < 0){
            if(verbose)
                cout << "No real solution to " << a << " x^4 + " << b << " x^3 + " << c << " x^2 + " << d << " x + " << e << " (no positive root for the resolvent cubic)." << endl << endl;
            return false;
        }

        const double p = sqrt(res[pChoice]);
        solveQuadratic(p, SQ(p), 0.5*( p*(bn + res[pChoice]) - cn ), roots, verbose);
        solveQuadratic(p, -SQ(p), 0.5*( p*(bn + res[pChoice]) + cn ), roots, verbose);

        for(unsigned short i = 0; i<roots.size(); ++i)
            roots[i] -= an/4.;
    }

    const unsigned short nRoots = roots.size();

    if(verbose){
        if(nRoots){
            cout << "Solutions of " << a << " x^4 + " << b << " x^3 + " << c << " x^2 + " << d << " x + " << e << ":" << endl;
            for(unsigned short i=0; i<nRoots; i++)
	            cout << "x" << i << " = " << roots[i] << ", test = " << a*QU(roots[i]) + b*CB(roots[i]) + c*SQ(roots[i]) + d*roots[i] + e << endl;
            cout << endl;
        }else{
            cout << "No real solution to " << a << " x^4 + " << b << " x^3 + " << c << " x^2 + " << d << " x + " << e << endl << endl;
        }
    }

    return nRoots;
}

// Solves the system:
// a11*E1^2 + a22*E2^2 + a12*E1*E2 + a10*E1 + a01*E2 + a00 = 0
// b11*E1^2 + b22*E2^2 + b12*E1*E2 + b10*E1 + b01*E2 + b00 = 0
// Which corresponds to finding the intersection points of two ellipses.
// Appends the (x,y) solutions to the std::vectors E1, E2, making no attempt to check whether these vectors are empty.
// In most cases it simply comes down to solving a quartic equation:
//     - eliminate the E1^2 term
//     - solve for E1 (linear!)
//     - inserting back gives a quartic function of E2
//     - find solutions for E1
// The procedure becomes tricky in some special cases (intersections aligned along x- or y-axis, ellipse reduced to a line, ...)
bool solve2Quads(const double a11, const double a22, const double a12, const double a10, const double a01, const double a00,
				 const double b11, const double b22, const double b12, const double b10, const double b01, const double b00,
				 vector<double>& E1, vector<double>& E2,
                 bool verbose){

	const double alpha = b11*a22-a11*b22;
	const double beta = b11*a12-a11*b12;
	const double gamma = b11*a10-a11*b10;
	const double delta = b11*a01-a11*b01;
	const double omega = b11*a00-a11*b00;

	const double a = a11*SQ(alpha) + a22*SQ(beta) - a12*alpha*beta;
	const double b = 2.*a11*alpha*delta - a12*( alpha*gamma + delta*beta ) - a10*alpha*beta + 2.*a22*beta*gamma + a01*SQ(beta);
	const double c = a11*SQ(delta) + 2.*a11*alpha*omega - a12*( delta*gamma + omega*beta ) - a10*( alpha*gamma + delta*beta )
		+ a22*SQ(gamma) + 2.*a01*beta*gamma + a00*SQ(beta);
	const double d = 2.*a11*delta*omega - a12*omega*gamma - a10*( delta*gamma + omega*beta ) + a01*SQ(gamma) + 2.*a00*beta*gamma;
	const double e = a11*SQ(omega) - a10*omega*gamma + a00*SQ(gamma);

	solveQuartic(a, b, c, d, e, E2, verbose);

	for(unsigned int i = 0; i < E2.size(); ++i){
		const double e2 = E2[i];
        if(beta*e2 + gamma != 0){        // Everything OK
		    const double e1 = -(alpha * SQ(e2) + delta*e2 + omega)/(beta*e2 + gamma);
		    E1.push_back(e1);
        }else if(alpha*SQ(e2) + delta*e2 + omega == 0){
            // Up to two solutions for e1 (aligned along y-axis)
            vector<double> e1;
            if( !solveQuadratic(a11, a12*e2 + a10, a22*SQ(e2) + a01*e2 + a00, e1, verbose) ){
                cout << "Error in solve2Quads: there should be at least one solution for e1!" << endl;
                exit(1);
            }
            if(e1[0] == e1[1]){ // no ambiguity
                E1.push_back(e1[0]);
                if(i < E2.size() - 1){
                    if(e2 == E2[i+1]){
                        E1.push_back(e1[0]);
                        E1.push_back(e1[0]);
                        i++;
                        continue;
                    }
                }
            }else{ // in that case, e2 SHOULD be twice degenerate!
                if(i < E2.size() - 1){
                    if(e2 != E2[i+1]){
                        cout << "Error in solve2Quads: if there are two solutions for e1, e2 should be degenerate!" << endl;
                        exit(1);
                    }
                    if(i < E2.size() - 2){
                        if(e2 == E2[i+2]){
                            cout << "Error in solve2Quads: if there are two solutions for e1, e2 cannot be thrice degenerate!" << endl;
                            exit(1);
                        }
                    }
                    E1.push_back(e1[0]);
                    E1.push_back(e1[1]);
                    i++;
                    continue;
                }else{
                    cout << "Error in solve2Quads: if there are two solutions for e1, e2 should be degenerate!" << endl;
                    exit(1);
                }
            }
        }else{        // There is no solution given this e2
            E2.erase(E2.begin() + i);
            cout << "Error in solve2Quads: no solution!" << endl;
        }
	}

	return true;
}

double BreitWigner(const double s, const double m, const double g){
	/*double ga = sqrt(m*m*(m*l+g*g));
	double k = 2*sqrt(2)*m*g*ga/(TMath::Pi()*sqrt(m*m+ga));*/
	double k = m*g;
	
	//cout << "BW(" << s << "," << m << "," << g << ")=" << k/(pow(s-m*m,2.) + pow(m*g,2.)) << endl;
	
	return k/(pow(s-m*m,2.) + pow(m*g,2.));
}

