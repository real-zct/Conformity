#ifndef ADITUMGRAPH_H
#define ADITUMGRAPH_H

#include <networkit/graph/Graph.hpp>
#include <networkit/io/GraphReader.hpp>
#include <networkit/io/EdgeListReader.hpp>
#include <aditum/io/CapitalScoreFileReader.hpp>
#include <string>
#include <vector>
#include <memory>

namespace Aditum
{
   // forwards declaration
   class AditumGraphBuilder;

   using Graph = NetworKit::Graph;
   using node = NetworKit::node;

   /**
    * Graph for the ADITUM problem.
    * In addition to the influence graph, it stores the
    * capital score associate with each node in the graph
    */
   class AditumGraph
   {
   private:
      // pointer to the underlying influence graph
      std::unique_ptr<Graph> influenceGraph;

      // vector containing the capital score of each node
      // each position corresponds to a node.
      std::vector<double> capitalScores;

      /**
         Private default constructor.
         An AditumGraph can only be created via the
         builder class
       */
   private:
      AditumGraph() = default;

   public:
      // builder of the glass
      friend class AditumGraphBuilder; // 声明了 friend class 后，指定的类可以在其成员函数中直接访问该类的私有成员，就像它们是其自己的成员一样。

      /**
         Return the vector of scores as a const reference
         \return const reference to the the capital scores vector
       */
      std::vector<double> const &scores() const;

      /**
         Retrun the score associated to the given node

         \param v The node
         \return capital score of v
       */
      double score(node v);

      /**
         Return a const reference to the underlying
         influence graph
         \return const reference to the influence graph
       */
      Graph const &graph() const;
   };

   /**
      Builder class responsible for
      creating the aditum graph.

      Two files are required:
         1. The file containing the influence graph
         2. A file containing the capital score of each node.
            By default, such file must have on each line the following
       information:
       <node id>:<node score>
       The id must be an integer type while score must be any numeric type
   */
   class AditumGraphBuilder
   {
   private:
      // path to the file containing the influence graph
      std::string graphPath;

      // path to the file containing the score of each node in the graph
      std::string scoresPath;

   public:
      /**
         Constructor.
         \param graphPath path to the influence graph
         \param scoresPath path to the file containing the scores
       */
      AditumGraphBuilder(std::string graphPath, std::string scoresPath);

      /**
         Default Constructor
       */
      AditumGraphBuilder() = default;

      /**
         Set the graph path

         \param graphPath path to the influence graph
         \return a reference to the current object
       */
      AditumGraphBuilder &setGraphPath(const std::string graphPath);

      /**
         Set the score path
         \return a reference to the current object
         \param graphPath path to the influence graph
       */
      AditumGraphBuilder &setScoresPath(const std::string scoresPath);

      /**
         Build the AditumGraph
         \param reader a NetworKit specific graph reader
         \param capReader a CapitalScoreFilereader
         \return the AditumGraph
       */
      AditumGraph build(NetworKit::GraphReader &&reader = NetworKit::EdgeListReader(' ', 0, "#", true, true),
                        CapitalScoreFileReader &&capReader = CapitalScoreFileReader());
   };

}

#endif
