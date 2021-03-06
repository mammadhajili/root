/// \file
/// \ingroup tutorial_multicore
///
/// Fill the same TNtuple from different threads.
/// This tutorial illustrates the basics of how it's possible with ROOT
/// to write simultaneously to a single output file using TBufferMerger.
///
/// \macro_code
///
/// \author Guilherme Amadio
/// \date May 2017

void mt103_fillNtupleFromMultipleThreads()
{
   // Avoid unnecessary output
   gROOT->SetBatch();

   // Make ROOT thread-safe
   ROOT::EnableThreadSafety();

   // Total number of events
   const size_t nEntries = 65535;

   // Match number of threads to what the hardware can do
   const size_t nWorkers = 4;

   // Split work in equal parts
   const size_t nEventsPerWorker = nEntries / nWorkers;

   // Create the TBufferMerger: this class orchestrates the parallel writing
   auto fileName = "mt103_fillNtupleFromMultipleThreads.root";
   ROOT::Experimental::TBufferMerger merger(fileName);

   // Define what each worker will do
   // We obtain from a merger a TBufferMergerFile, which is nothing more than
   // a file which is held in memory and that flushes to the TBufferMerger its
   // content.
   auto work_function = [&](int seed) {
      auto f = merger.GetFile();
      TNtuple ntrand("ntrand", "Random Numbers", "r");

      // The resetting of the kCleanup bit below is necessary to avoid leaving
      // the management of this object to ROOT, which leads to a race condition
      // that may cause a crash once all threads are finished and the final
      // merge is happening
      ntrand.ResetBit(kMustCleanup);

      TRandom rnd(seed);
      for (auto i : ROOT::TSeqI(nEntries)) ntrand.Fill(rnd.Gaus());
      f->Write();
   };

   // Create worker threads
   std::vector<std::thread> workers;

   for (auto i : ROOT::TSeqI(nWorkers))
      workers.emplace_back(work_function, i+1); // seed==0 means random seed :)

   // Make sure workers are done
   for (auto &&worker : workers) worker.join();

}
