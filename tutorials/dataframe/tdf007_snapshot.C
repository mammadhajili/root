/// \file
/// \ingroup tutorial_tdataframe
/// \notebook
/// This tutorial shows how to write out datasets in ROOT formatusing the TDataFrame
/// \macro_code
///
/// \date April 2017
/// \author Danilo Piparo

#include "TFile.h"
#include "TTree.h"

#include "ROOT/TDataFrame.hxx"

// A simple helper function to fill a test tree: this makes the example
// stand-alone.
void fill_tree(const char *filename, const char *treeName)
{
   TFile f(filename, "RECREATE");
   TTree t(treeName, treeName);
   int b1;
   float b2;
   t.Branch("b1", &b1);
   t.Branch("b2", &b2);
   for (int i = 0; i < 10000; ++i) {
      b1 = i;
      b2 = i * i;
      t.Fill();
   }
   t.Write();
   f.Close();
   return;
}

int tdf007_snapshot()
{
   // We prepare an input tree to run on
   auto fileName = "tdf007_snapshot.root";
   auto outFileName = "tdf007_snapshot_output.root";
   auto outFileNameAllColumns = "tdf007_snapshot_output_allColumns.root";
   auto treeName = "myTree";
   fill_tree(fileName, treeName);

   // We read the tree from the file and create a TDataFrame.
   ROOT::Experimental::TDataFrame d(treeName, fileName);

   // ## Select entries
   // We now select some entries in the dataset
   auto d_cut = d.Filter("b1 % 2 == 0");
   // ## Enrich the dataset
   // Build some temporary columns: we'll write them out
   auto d2 = d_cut.Define("b1_square", "b1 * b1")
                .Define("b2_vector",
                        [](float b2) {
                           std::vector<float> v;
                           for (int i = 0; i < 3; i++) v.push_back(b2*i);
                           return v;
                        },
                        {"b2"});

   // ## Write it to disk in ROOT format
   // We now write to disk a new dataset with one of the variables originally
   // present in the tree and the new variables.
   // The user can explicitly specify the types of the columns as template
   // arguments of the Snapshot method, otherwise they will be automatically
   // inferred.
   d2.Snapshot(treeName, outFileName, {"b1", "b1_square", "b2_vector"});

   // Open the new file and list the columns of the tree
   TFile f1(outFileName);
   TTree *t;
   f1.GetObject(treeName, t);
   std::cout << "These are the columns b1, b1_square and b2_vector:" << std::endl;
   for (auto branch : *t->GetListOfBranches()) {
      std::cout << "Branch: " << branch->GetName() << std::endl;
   }
   f1.Close();

   // We are not forced to write the full set of column names. We can also
   // specify a regular expression for that. In case nothing is specified, all
   // columns are persistified.
   d2.Snapshot(treeName, outFileNameAllColumns);

   // Open the new file and list the columns of the tree
   TFile f2(outFileNameAllColumns);
   f2.GetObject(treeName, t);
   std::cout << "These are all the columns available to this tdf:" << std::endl;
   for (auto branch : *t->GetListOfBranches()) {
      std::cout << "Branch: " << branch->GetName() << std::endl;
   }
   f2.Close();

   // We can also get a fresh TDataFrame out of the snapshot and restart the
   // analysis chain from it. The default columns are the one selected.
   // Notice also how we can decide to be more explicit with the types of the
   // columns.
   auto snapshot_tdf = d2.Snapshot<int>(treeName, outFileName, {"b1_square"});
   auto h = snapshot_tdf.Histo1D();
   auto c = new TCanvas();
   h->Draw();

   return 0;
}
