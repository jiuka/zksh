# zksh

ZooKeeper shell utilities collection. Eneables you to harvest the power of [ZooKeeper](https://zookeeper.apache.org/) in your shell scripts.

*zkls*, *tktouch*, *zkrm*, *zktee* and *zkgetacl* allow your to explore and modify the ZooKeeper node structure.

*zklock* enable you to lock resources across systems in simple shell scripts.

## Development

### Build from Source

If built from the git source you need to run autoreconf first to generate the autotool build system.

    $ autoreconf
    
The release tarballs have this step already done for you. The configure and make are sufficient.

    $ ./configure
    $ make
    
### Software testing

Some Unit Test are inplemented with [libcheck](https://libcheck.github.io/check/). If the library is present thouse can be built and execudet with make.

    make test
    
Integration testing is implemented with the help of docker-compose. and [bats](https://github.com/sstephenson/bats). Thous test can be executed by make too.

    make docker_bats
