## **SeRPeNT** - A suite of tools for ncRNA discovery, profiling, annotation and analysis from small RNA-Seq data ##

------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

The serpent subcommands include:

**Profiling and annotation tools** :

  profiles : ncRNA discovery and profiling from small RNA-Seq data

  annotate : ncRNA clustering, classification and annotation from profile data

  diffproc : ncRNA differential processing from profile and clustering data

**General help** :

  -h, --help :  Print this help menu

  -v, --version : What version of serpent are you using?

**Examples** :

  serpent profiles replicate1.bam replicate2.bam replicate3.bam output_dir
  serpent annotate output_dir/profiles.dat annotation.gtf output_dir
  serpent diffproc condition_a_profiles.dat condition_a_annotation.bed condition_b_profiles.dat condition_b_annotation.bed output_dir

------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
**Tool** : profiles

**Summary** : ncRNA discovery and profiling from small RNA-Seq data

**Usage** :

  serpent profiles [OPTIONS] replicate_1.bam ... replicate_n.bam output_folder

**Options**  :

           -f   Read filtering
                Format is <minreadlen>, where:
                  - <minreadlen> is the minimum required length for the reads. Reads shorter than <readlen> are discarded. If 0, no reads are discarded. Must be >= 0.
                [ Default is 0 ]

           -i   Irreproducibility control for contigs
                Format is <method:cutoff> | <common> | <none>, where:
                  - <method> is the irreproducibility control method. Options are:
                    - sere : Single-parameter quality control. (Schulze et al. BMC Genomics 2012)
                    - idr  : Non-parametric irreproducibility discovery rate. (Dobin et al. Bioinformatics 2013)
                  - <cutoff> is the cutoff value. Contigs that have an irreproducibility score higher than <cutoff> are not reported. Must be > 0.
                  - <common> : Contigs that do not overlap in all the replicates are not reported.
                  - <none>   : All contigs are reported.
                [ Default is common ]

           -r   Replicates treatment
                Format is <pool> | <mean> | <replicate:repnumber>, where:
                  - <pool> : Profiles are built by pooling the reads of all the replicates.
                  - <mean> : Profiles are built by averaging the reads of all the replicates.
                  - <replicate:repnumber> : Profiles are built by using only the reads of the <repnumber> replicate.
                [ Default is pool ]

           -t   Trimming
                Format is <trim_threshold:trim_min:trim_max>
                  - <trim_percentage> is the trimming threshold. Nucleotides in the ends of the profile having less than <trim_precentage> percent of reads compared to the
                                      maximum height will be trimmed.
                  - <trim_min> is the trimming minimum height. All nucleotides in both ends of the profile having less than <trim_min> reads will be trimmed.
                  - <trim_max> is the trimming maximum height. No nucleotides in both ends of the profile having more than <trim_max> reads will be trimmed.
                [ Default is 0.1:2:10 ]

           -p   Profile definition
                Format is <minlen:maxlen:spacing:minheight:trimming>, where:
                  - <minlen> is the minimum length of the profile after trimming. Profiles shorter than <minlen> are not reported. Must be > 5.
                  - <maxlen> is the maximum length of the profile after trimming. Profiles longer than <maxlen> are not reported. Must be >= minlen.
                  - <spacing> is the maximum distance between profiles. Profiles separated by <spacing> or less bp are merged into one single profile. Must be >= 0.
                  - <minheight> is the minimum number of piled-up reads. Profiles that have less than <minheight> piled-up reads are not reported. Must be > 0.
                [ Default is 16:200:20:50 ]

**Output** :

  output_folder/profiles.dat : List of ncRNA profiles with per-base heights

  output_folder/contigs.dat  : List of unfiltered contigs

**Examples** : 

  serpent profiles -f 20 -i sere:2 -r pool -t 0.1:5:20 -p 20:200:39:100 replicate1.bam replicate2.bam output_dir
------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
**Tool** : annotate

**Summary** : ncRNA clustering, classification and annotation from profile data

**Usage** :

  serpent annotate [OPTIONS] profiles_file.dat output_folder

**Options** : 

            -a   Annotation file
                 Format is <annotation_file>, where:
                   - <annotation_file> is a BED file with annotated features
                 [ No default value ]

            -o   Overlapping parameters
                 Format is <feature_to_profile:profile_to_feature>, where:
                   - <feature_to_profile> is the percentage of nucleotides from the feature that overlap the profile
                   - <profile_to_feature> is the percentage of nucleotides from the profile that overlap the feature
                 [ Default is 0.9:0.5 ]

            -x   Distance file
                 Format is <distance_file>, where:
                   - <distance_file> is the file with pairwise distances between profiles
                 When -x option is specified, distances are not calculated and directly taken from the provided file
                 [ No default value ]

**Output** :

  output_folder/crosscorr.dat    : List of distances between pairs of profiles (only if no distance file is provided)

  output_folder/annotation.bed   : List of annotated features in BED file

**Example** :

  serpent annotate -a hsap_micrornas.bed profiles.dat output_dir
  serpent annotate -a hsap_micrornas.bed -x crosscor.dat profiles.dat output_dir
------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
**Tool** : diffproc

**Summary** : ncRNA differential processing from profile and clustering data between two conditions

**Usage** :

  serpent diffproc [OPTIONS] profiles_file_1.dat clustering_file_1.bed profile_file_2.dat clustering_file_2.bed output_folder

**Options** :

            -g   p-value and distance fold-change threshold
                 Format is <pvalue:foldchange> where:
                   - <pvalue> is the p-value threshold for filtering differentially processed profiles
                   - <foldchange> is the distance fold-change threshold for filtering differentially processed profiles
                 [ Default is 0.01:0.5 ]

**Output** :

  output_folder/diffprofiles.dat : List of differentially processed profiles

**Examples** :

  serpent diffproc -g 0.01:5 wild_type/profiles.dat wild_type/annotation.bed treated/profiles.dat treated/annotation.bed output_dir
------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
