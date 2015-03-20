The BioC is a C library for Bioinformatics tools.

Nowadays, working with “mountains” of data is a common task on Bioinformatics. Frequently, high level programming languages like Perl, Python or even Java are not fast enough to process data efficiently. In this case, low level languages as C can be the difference between 5 days and a few hours of processing. Prototypes programs can be easily developed with high level languages just as a proof of concept. However, optimized code, in low level languages, are needed to get down this “mountains” of data.

Basic modules:

  * [Error](berror.md)
  * [Memory](bmemory.md)
  * [Time](btime.md)
  * [String](bstring.md)
  * [Integer key based Btree](btree.md)
  * [String key based Btree](btreeString.md)

Bioinformatics modules

  * [Fasta file manipulation](fasta.md)
  * [NCBI Taxonomy database](taxonomy.md)

Associated to the library there are some applications:

  * [Build a binary index file for a fasta file](https://code.google.com/p/bioc/source/browse/trunk/BuildBtreeIndexFasta/)
  * [Split a fasta file by overlapped reads](https://code.google.com/p/bioc/source/browse/trunk/SplitFastaFile/)
  * [Print the taxonomy lineage from GenBank Gi](https://code.google.com/p/bioc/source/browse/trunk/TaxLineageFromGi/)
  * [Print the taxonomy lineage from TaxId](https://code.google.com/p/bioc/source/browse/trunk/TaxLineageFromTaxId)
  * [A basic assembler program for the Taxoner output using overlapped reads](https://code.google.com/p/bioc/source/browse/trunk/TaxonerAssamblerMarkerDB/)
  * [A taxonomic filter for the NCBI nt fasta file](https://code.google.com/p/bioc/source/browse/trunk/TaxFilter/)