# Bioinformatics tools in C

Nowadays, working with “mountains” of data is a common task in Bioinformatics. Frequently, high level programming languages like Perl, Python or even Java are not fast enough to process data efficiently. In this case, low level languages as C can be the difference between 5 days and a few hours of processing. Prototype programs can be easily developed with high level languages just as a proof of concept. However, optimized code, in low level languages are needed to get down this “mountains” of data.

Basic modules:

  * [berror Error]
  * [bmemory Memory]
  * [btime Time]
  * [bstring String]
  * [btree Integer key based Btree]
  * [btreeString String key based Btree]

Bioinformatics modules

  * [fasta Fasta file manipulation]
  * [taxonomy NCBI Taxonomy database]

Associated to the library there are some applications:

  * [https://code.google.com/p/bioc/source/browse/trunk/BuildBtreeIndexFasta/ Build a binary index file for a fasta file]
  * [https://code.google.com/p/bioc/source/browse/trunk/SplitFastaFile/ Split a fasta file by overlapped reads]
  * [https://code.google.com/p/bioc/source/browse/trunk/TaxLineageFromGi/ Print the taxonomy lineage from GenBank Gi]
  * [https://code.google.com/p/bioc/source/browse/trunk/TaxLineageFromTaxId Print the taxonomy lineage from TaxId]
  * [https://code.google.com/p/bioc/source/browse/trunk/TaxonerAssamblerMarkerDB/ A basic assembler program for the Taxoner output using overlapped reads]
  * [https://code.google.com/p/bioc/source/browse/trunk/TaxFilter/ A taxonomic filter for the NCBI nt fasta file]




