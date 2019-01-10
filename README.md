# QoSD
This project focuses on network resilience to perturbation of edge weight. 
Other than connectivity, many network applications nowadays rely upon some 
measure of network distance between a pair of connected nodes. In these 
systems, a metric related to network functionality is associated to each 
edge. A pair of nodes only being functional if the weighted, shortest-path 
distance between the pair is below a given threshold \texttt{T}. Consequently, 
a natural question is on which degree the change of edge weights can damage 
the network functionality? With this motivation, we study a new problem, 
\textit{Quality of Service Degradation}: given a set of pairs, find a minimum 
budget to increase the edge weights which ensures the distance between each 
pair exceeds $\mathtt{T}$. We introduce four algorithms with theoretical 
performance guarantees for this problem. Each of them has its own strength 
in trade-off between effectiveness and running time, which are illustrated 
both in theory and comprehensive experimental evaluation

The source code will be available soon :)
