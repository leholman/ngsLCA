# ngsLCA
[![Build Status](https://travis-ci.org/miwipe/ngsLCA.svg?branch=master)](https://travis-ci.org/miwipe/ngsLCA)

# Installation
git clone https://github.com/SAMtools/htslib

git clone https://github.com/miwipe/ngsLCA

cd htslib;make;cd ../ngsLCA;make HTSSRC=../htslib

# Running Main c/c++ program
## Finding resource files for program from NCBI
## Running ngsLCA

# Visualizing results with R script

The R script ngsLCA_interpret.R processes and visualizes outputs from ngsLCA

Developed and tested in R version 3.6.1

The scripts relied on some R packages. It will detecte and install required R packages automaticlly before running.

Bugs and/or suggestions to ycwang@bio.ku.dk or wyc661217@gmail.com

## Input files

For inputing, this script requires all lca files (*.lca) copied into a working directory

Two optional inputs:

1) lca files for negetive laboratory control samples can be copied into a sub-directory named "blanks" under the working directory. If provided, the taxa detected in these control samples will be removed from real samples.

2) A metadata csv file matches the file names to sample names, ages, locations, etc. if provided, the supplied metadata will be illustrated in the results instead of file names. The metadata file accepte a comma separated two columns flat text, with first column covering all lca file names, and second column supplying the metadata will be illustrated.

An example:

file1.lca,s1_1

file2.lca,s3_1

file3.lca,s3_2

....






