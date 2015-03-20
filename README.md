# Bioinformatics tools in C

Nowadays, working with “mountains” of data is a common task in Bioinformatics. Frequently, high level programming languages like Perl, Python or even Java are not fast enough to process data efficiently. In this case, low level languages as C can be the difference between 5 days and a few hours of processing. Prototype programs can be easily developed with high level languages just as a proof of concept. However, optimized code, in low level languages are needed to get down this “mountains” of data.

Basic modules:

  * Error
  * Memory
  * Time
  * String
  * Integer key based Btree
  * String key based Btree

Bioinformatics modules

  * Fasta file manipulation
  * NCBI Taxonomy database

Associated to the library there are some applications:

  * BuildBtreeIndexFasta: Build a binary index file for a fasta file
  * SplitFastaFile: Split a fasta file by overlapped reads
  * TaxLineageFromGi: Print the taxonomy lineage from GenBank Gi
  * TaxLineageFromTaxId: Print the taxonomy lineage from TaxId
  * TaxonerAssamblerMarkerDB: A basic assembler program for the Taxoner output using overlapped reads
  * TaxFilter: A taxonomic filter for the NCBI nt fasta file





