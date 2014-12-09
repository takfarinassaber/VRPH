-------------
-------------
VRPH-1.0.8 :
-------------
-------------
- Version 1.0.8 of the library VRPH written by Groer. 
This version handles instances in both formats: (i) Capacitated Vehicle Routing Problem (CVRP) and (ii) Clustered 
Capacitated Vehicle Routing Problem (CCVRP).
It implements the Big-M method explained in the joined report "rapport.pdf" (unfortunatly in french).

=============
Rapport.pdf
=============
"!!in french!!"
It contains:
- Modelisation of the CCVRP problem.
- Details of proposed adaptations to the standard CVRP format to take into account the Clustering.
- Explanation of the Big-M method and its implementation.
- The optained results using VRPH-1.0.8.
- advantages and inconvenients of using VRPH-1.0.8.

=============
dev/
=============

instance testing:
-----------------
- VRPH-1.0.8/tests/ contains two scripts. Each of them allow the testing of a particular set of instances. The first 
one is kindly made available by Prof. Marc Sevaux and the second is randomly generated.
To run tests on these instances, you only have to execute the following commands once in dev/VRPH-1.0.8/tests/

sh tests-random-instances.sh
sh tests-instances-sevaux.sh

ccvrp_to_big_m_cvrp :
---------------------
Conversion d'une instance de type CCVRP en une instance CVRP au format matriciel dont les distances ont été pénalisées par 
par la méthode du BIG-M.

clustered-vrp-generator :
-------------------------
Programme de génération aléatoire d'instances CCVRP


=============
Annexes/
=============
Contains some documentation and reports regarding VRPH and VRPH-1.0.8.
