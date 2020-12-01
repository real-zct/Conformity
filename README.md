
# Overview

This project is developed as part of the following research paper:

[A. Caliò, A. Tagarelli. 
Attribute based Diversification of Seeds for Targeted Influence Maximization problem. 
Information Sciences, 2020)](<https://doi.org/10.1016/j.ins.2020.08.093>).

Please cite the above paper in any research publication you may produce using this code or data/analysis derived from it.


# Abstract

Embedding diversity into knowledge discovery is important: the patterns mined will be more novel, more meaningful, and broader.
Surprisingly, in the classic problem of  influence maximization
   in social networks,  relatively little study has been
 devoted to diversity  and its integration into  the objective function of
 an influence maximization  method. 
In this work, we propose the integration of a red categorical-based
notion of seed diversity  into the objective
 function of a targeted influence maximization problem. 

In this respect, we assume that the users of
 a   social network are  associated with a categorical dataset where each
 tuple expresses the profile of a user according to a predefined
 schema of categorical attributes. 
Upon this assumption, we design a class of monotone   submodular functions specifically conceived
 for  determining the diversity of the subset of  categorical tuples associated with  the seed
 users to be discovered.   This allows us to develop an efficient approximate  method, with a
 constant-factor guarantee of optimality. More precisely, 
we formulate the \textit{attribute-based diversity-sensitive targeted influence maximization} problem   under
 the state-of-the-art reverse influence sampling framework,  and we develop a method,
 dubbed \textsf{ADITUM}, that  ensures a  $(1-1/e-&epsilon;)$-approximate
 solution under the general triggering diffusion model.  

Extensive experimental evaluation based on real-world networks
 as well as  synthetically generated data has shown the meaningfulness and
 uniqueness of  our proposed class of set diversity functions and
 of the \textsf{ADITUM} algorithm, also in comparison with
  methods that exploit numerical-attribute-based diversity
 and topology-driven diversity in influence maximization.  


# Requirements

In order to compile the project you need:

-   A modern C++ compiler
-   OpenMP for parallelism
-   Cmake - version 3.17. or higher
-   Build System (e.g.,Make, Ninja)
-   Boost program<sub>options</sub> library


# Installation

Clone this repository as well as the networkit submodule.

	cd to/your/path
	git clone <https://github.com/acalio/aditum.git>
	git submodule update --init --recursive

The last command will download two libraries: 

-   [abseil-cpp](<https://github.com/abseil/abseil-cpp>)
-   [networkit](<https://github.com/networkit/networkit>)

Once you downloaded all the required files, 
you need to compile the project as follows:

	cd to/your/path
	mkdir build
	cd build
	cmake ..
	make app

This will create an executable file in the build folder, under the `app` subdirectory.


# Usage

The following steps are required to run the program. 

First of all, move into the building folder &#x2013; if you use the same instruction in the installation
section this is the  `build` folder under the root folder of the project &#x2013; 

cd root-folder/build/app

Let's say you want to run the Attribute-Wise variant of the algorithm,
then you have to issue the following command:

	./app -a wise 
	--graph /path/to/the/influence/graph \
	--capital /path/to/the/capital/scores \
    --target-threshold  0.42 \
	--output /path/where/the/seeds/will/be/stored \
	--diffusion-model lt \
	--k 20 \
	--attributes /path/the/the/user/categorical/attributes \

The above command will select 20 nodes as seed of the propagation process and store 
them in the file provided as the `--output` parameter.

There are many other options available. To have a more comprehensive guide just
issue the command:

./app &#x2013;help


# Inputs & Outputs


# Extending the framework

-   input-graph: the path to a file storing the edge-list of a directed (weighted) network
-   output-file: the path to a file storing results, i.e., <v, k>, where v is the id of a node and k the index assigned by the selected graph-decomposition algorithm
-   decomposition-algorithm: specifies a graph-decomposition algorithm. Available options:

<span class="underline">kcore</span>, <span class="underline">kpeak</span>, <span class="underline">ktruss</span>, <span class="underline">neighcoreness</span>, <span class="underline">dgcore</span>.

For instance, if you need to carry out a core-decomposition:

./deco input-edgelist results.txt kcore

The program will store in results.txt the output of the computation.

