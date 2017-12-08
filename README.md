# PTaskER

PTaskER is a GUI interface for [TaskWarrior](https://taskwarrior.org/)
written in GTK+3.

It is a fork of PTask with possibility to send patches upstream.

# Installation

## Packages

There will be installable packages for some systems soon.

## Source

### Dependencies

PTaskER requires these dependencies to be installed before compilation:

* asciidoc
* autotools
* autoconf
* autoreconf
* cppcheck (optional)
* gcc
* help2man
* gtk3 >= 3.12
* json-c (libjson-c on debs)
* make

### Get sources

Download sources with git or by visiting Releases tab at
https://lab.pztrn.name/pztrn/ptasker.

### Compile

First, perform autoreconf:

```
autoreconf -fi
```

Configure sources:

```
./configure --prefix=/usr
```

**Note**: ``--prefix`` can be pointed somewhere else, for example, if you're
building a package.

Compile and install:

```
make
make install
```