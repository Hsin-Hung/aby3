# ABY 3 and Applications
 
## Introduction
 
This library provides the semi-honest implementation of [ABY 3](https://eprint.iacr.org/2018/403.pdf) and [Fast Database Joins for Secret Shared Data](https://eprint.iacr.org/2019/518.pdf).

The repo includes the following application:
 * Linear Regression (training/inference)
 * Logistic Regression (training/inference)
 * Database Inner, Left and Full Joins
 * Database Union
 * Set Cardinality
 * Threat Log Comparison ([see Section 5](https://eprint.iacr.org/2019/518.pdf))
 * ERIC Application ([see Section 5](https://eprint.iacr.org/2019/518.pdf))

A tutorial can be found [here](https://github.com/ladnir/aby3/blob/master/frontend/aby3Tutorial.cpp). It includes a description of how to use the API and a discussion at the end on how the framework is implemented.

## Warning 

This codebase should **NOT** be considered fully secure. It has not had a security review and there are still several security related issues that have not been fully implemented. Only use this codebase as a proof-of-concept or to benchmark the perfromance. Future work is required for this implementation to be considered secure. 

Moreover, some features have not been fully developed and contains bugs. For example, the task scheduler sometime fails. This is a known issue.

## Build
 
The library is *cross platform* and has been tested on Windows and Linux. The dependencies are:

 * [libOTe](https://github.com/osu-crypto/libOTe)
 * [Boost](http://www.boost.org/) (networking)
 * [function2](https://github.com/Naios/function2)
 * [Eigen](http://eigen.tuxfamily.org/index.php?title=Main_Page)

 
In short, this will build the project

```
git clone https://github.com/ladnir/aby3.git
cd aby3/
python3 build.py --setup
python3 build.py 
```

To see all the command line options, execute the program 
 
`bin/frontend`

The library can be linked by linking the binraries in `lib/` and `thirdparty/win` or `thirdparty/unix` depending on the platform.

## Cloud Run (Intersect)
After building the project, update `ips.txt` so that each line contains the IP of each party(VM). There should only be a total of three parties, where the first line is the first party IP and the last line is the third party IP. 

On each party VM, execute

`bin/frontend -intersect -nn N -r R`. 
 
`N` will set the table size as `2^N` and it should be set the same on all parties.

`R` is the calling party's rank, which should be betwen 0-2.

## Help
 
Contact Peter Rindal peterrindal@gmail.com for any assistance on building  or running the library.

## Citing

 Spread the word!

```
@misc{aby3,
    author = {Peter Rindal},
    title = {{The ABY3 Framework for Machine Learning and Database Operations.}},
    howpublished = {\url{https://github.com/ladnir/aby3}},
}
```
