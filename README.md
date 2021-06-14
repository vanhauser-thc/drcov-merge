# drcov-merge
merge multiple drcov coverage files into one.
done super fast and dirty.

## Compile

`make`

## Install

`make install`

## How to use

`drcov-merge [-u] new-coverage-file.log drcov.foo.0.0.proc.log drcov.foo.1.0.proc.log drcov.foo.2.0.proc.log ...`
Option `-u` uniques the coverage over all files.

## What for?

e.g. compare coverage of different fuzzers in a binary via Lighthouse/Dragondance/BNCOV
