#!/bin/bash

# Script for building AROS from the GIT repository

# Currently only a limited amount of Linux distros is supported.
# If you improve this script, send modifications back to me, please.
# Matthias Rustler, mailto:mrustler@gmx.de

# This script is public domain. Use it at your own risk.

# $VER: gimmearos.sh 1.16 (18.01.2020)

# This one modded for vampire use

curdir="`pwd`"
srcdir="aros-src-vamp"
portsdir="$HOME/aros-ports-src"
makeopts="-j36 -s" # maybe change this on lame systems
configopts="--enable-ccache --with-portssources=$portsdir --with-optimization=-Os  --with-aros-prefs=classic"

USER_CFLAGS=""

install_pkg()
{
    if [ $# -ne 2 ]
    then
        echo "install_pkg() needs 2 arguments"
        exit 100
    fi
    echo -e "\nInstalling " $2
    sudo $1 $2
    if [ $? -ne 0 ]
    then
        echo -e "\n    install failed. Script cancelled."
        exit 100
    fi
}


echo -e "\n\n\n\n\n"
echo -e "***********************************************"
echo -e "* Script for downloading and building of AROS *"
echo -e "***********************************************"
echo -e "\n\n"
echo -e "*********************************"
echo -e "* Step 1: install prerequisites *"
echo -e "*********************************"
echo -e "The build system needs some packages to do its job."
echo -e "If you are asked for a password enter you admin password."
echo -e "\n1 .. Get packages with apt-get for Debian and similar (e.g. Ubuntu)"
echo -e "     for building 32-bit AROS on 32-bit Linux or 64-bit-AROS on 64-bit-Linux"
echo -e "2 .. As 1) but with additional packages for building 32-bit AROS"
echo -e "     on 64-bit Linux"
echo -e "3 .. Get packages with yum for Fedora"
echo -e "4 .. Get packages with pacman for Arch"
echo -e "5 .. Get packages with zypper for openSuse"
echo -e "6 .. As 5) but with additional packages for building 32-bit AROS"
echo -e "     on 64-bit Linux"
echo -e "9 .. Skip this step"
echo -e "0 .. Exit"

echo -e "\nEnter number and press <Enter>:"

read input
case "$input" in
    1 ) echo -e "\nInstalling prerequisites with apt-get..."
        install_pkg "apt-get install" git-core
        install_pkg "apt-get install" gcc
        install_pkg "apt-get install" g++
        install_pkg "apt-get install" make
        install_pkg "apt-get install" cmake
        install_pkg "apt-get install" gawk
        install_pkg "apt-get install" bison
        install_pkg "apt-get install" flex
        install_pkg "apt-get install" bzip2
        install_pkg "apt-get install" netpbm
        install_pkg "apt-get install" autoconf
        install_pkg "apt-get install" automake
        install_pkg "apt-get install" libx11-dev
        install_pkg "apt-get install" libxext-dev
        install_pkg "apt-get install" libc6-dev
        install_pkg "apt-get install" liblzo2-dev
        install_pkg "apt-get install" libxxf86vm-dev
        install_pkg "apt-get install" libpng-dev
        install_pkg "apt-get install" libsdl1.2-dev
        install_pkg "apt-get install" byacc
        install_pkg "apt-get install" python-mako
        install_pkg "apt-get install" libxcursor-dev
        ;;

    2 ) echo -e "\nInstalling prerequisites with apt-get..."
        install_pkg "apt-get install" git-core
        install_pkg "apt-get install" gcc
        install_pkg "apt-get install" g++
        install_pkg "apt-get install" make
        install_pkg "apt-get install" cmake
        install_pkg "apt-get install" gawk
        install_pkg "apt-get install" bison
        install_pkg "apt-get install" flex
        install_pkg "apt-get install" bzip2
        install_pkg "apt-get install" netpbm
        install_pkg "apt-get install" autoconf
        install_pkg "apt-get install" automake
        install_pkg "apt-get install" libx11-dev
        install_pkg "apt-get install" libxext-dev
        install_pkg "apt-get install" libc6-dev
        install_pkg "apt-get install" liblzo2-dev
        install_pkg "apt-get install" libxxf86vm-dev
        install_pkg "apt-get install" libpng-dev
        install_pkg "apt-get install" gcc-multilib
        install_pkg "apt-get install" libsdl1.2-dev
        install_pkg "apt-get install" byacc
        install_pkg "apt-get install" python-mako
        install_pkg "apt-get install" libxcursor-dev

        install_pkg "apt-get install" libc6-dev-i386
        install_pkg "apt-get install" lib32gcc1
	install_pkg "apt-get install" libxcursor1:i386
        ;;

    3 ) echo -e "\nInstalling prerequisites with yum..."
        install_pkg "yum install" git-core
        install_pkg "yum install" gcc
        install_pkg "yum install" gawk
        install_pkg "yum install" bison
        install_pkg "yum install" flex
        install_pkg "yum install" bzip2
        install_pkg "yum install" netpbm
        install_pkg "yum install" autoconf
        install_pkg "yum install" automake
        install_pkg "yum install" libX11-devel
        install_pkg "yum install" glibc-devel
        install_pkg "yum install" lzo-devel
        ;;

    4 ) echo -e "\nInstalling prerequisites with pacman.."
        echo -e "\nUpdating the List of software"
        echo -e "\nEnter sudo password"
        sudo pacman -Sy
        install_pkg "pacman --needed --noconfirm -S" git-core
        install_pkg "pacman --needed --noconfirm -S" gcc
        install_pkg "pacman --needed --noconfirm -S" gawk
        install_pkg "pacman --needed --noconfirm -S" bison
        install_pkg "pacman --needed --noconfirm -S" flex
        install_pkg "pacman --needed --noconfirm -S" bzip2
        install_pkg "pacman --needed --noconfirm -S" netpbm
        install_pkg "pacman --needed --noconfirm -S" autoconf
        install_pkg "pacman --needed --noconfirm -S" automake
        #it appears as though the libx11-dev,libc6-dev,liblzo2-dev is not needed on arch
        ;;

    5 ) echo -e "\nInstalling prerequisites with zypper..."
        # tools
        install_pkg "zypper --non-interactive install" git-core
        install_pkg "zypper --non-interactive install" gcc
        install_pkg "zypper --non-interactive install" gcc-c++
        install_pkg "zypper --non-interactive install" make
        install_pkg "zypper --non-interactive install" gawk
        install_pkg "zypper --non-interactive install" bison
        install_pkg "zypper --non-interactive install" flex
        install_pkg "zypper --non-interactive install" bzip2
        install_pkg "zypper --non-interactive install" netpbm
        install_pkg "zypper --non-interactive install" autoconf
        install_pkg "zypper --non-interactive install" automake
        install_pkg "zypper --non-interactive install" patch
        install_pkg "zypper --non-interactive install" cmake
        install_pkg "zypper --non-interactive install" gperf
        install_pkg "zypper --non-interactive install" perl-Switch
        install_pkg "zypper --non-interactive install" byacc

        # libs
        install_pkg "zypper --non-interactive install" libXxf86vm1

        #devel
        install_pkg "zypper --non-interactive install" libX11-devel
        install_pkg "zypper --non-interactive install" glibc-devel
        install_pkg "zypper --non-interactive install" libpng12-devel
        ;;

    6 ) echo -e "\nInstalling prerequisites with zypper..."
        # tools
        install_pkg "zypper --non-interactive install" git-core
        install_pkg "zypper --non-interactive install" gcc
        install_pkg "zypper --non-interactive install" gcc-c++
        install_pkg "zypper --non-interactive install" make
        install_pkg "zypper --non-interactive install" gawk
        install_pkg "zypper --non-interactive install" bison
        install_pkg "zypper --non-interactive install" flex
        install_pkg "zypper --non-interactive install" bzip2
        install_pkg "zypper --non-interactive install" netpbm
        install_pkg "zypper --non-interactive install" autoconf
        install_pkg "zypper --non-interactive install" automake
        install_pkg "zypper --non-interactive install" patch
        install_pkg "zypper --non-interactive install" cmake
        install_pkg "zypper --non-interactive install" gperf
        install_pkg "zypper --non-interactive install" perl-Switch
        install_pkg "zypper --non-interactive install" byacc

        # libs
        install_pkg "zypper --non-interactive install" libXxf86vm1

        #devel
        install_pkg "zypper --non-interactive install" libX11-devel
        install_pkg "zypper --non-interactive install" glibc-devel
        install_pkg "zypper --non-interactive install" libpng12-devel

        # 32-bit support
        install_pkg "zypper --non-interactive install" gcc-32bit
        install_pkg "zypper --non-interactive install" gcc-c++-32bit
        install_pkg "zypper --non-interactive install" glibc-devel-32bit
        install_pkg "zypper --non-interactive install" libXxf86vm1-32bit
        ;;

    0 ) exit 0
        ;;
esac


input=""
until [ "$input" = "9" ]
do
    cd "$curdir"

    echo -e "\n\n\n\n\n"
    echo -e "******************************************************"
    echo -e "* Step 2: get the sources from the GITHUB repository *"
    echo -e "******************************************************"
    echo -e   "1 .. Get AROS core (required)"
    echo -e   "4 .. Get documentation source (optional)"
    echo -e   "5 .. Get binaries (wallpapers, logos etc.) (optional)"
    echo -e "\n9 .. Go to next step"
    echo -e   "0 .. Exit"

    echo -e "\nEnter number and press <Enter>:"

    read input
    case "$input" in
        1 ) echo -e "\nGetting AROS V1 core with Git...\n"
            git clone https://github.com/aros-development-team/AROS.git "$srcdir"
            cd "$srcdir"
            git submodule update --init --recursive
            ;;
        2 ) echo -e "\nGetting contrib V1 with Git...\n"
            git clone https://github.com/aros-development-team/contrib.git "$srcdir/contrib"
            cd "$srcdir/contrib"
            git submodule update --init --recursive
            ;;
        3 ) echo -e "\nGetting ports V1 with Git...\n"
            git clone https://github.com/aros-development-team/ports.git "$srcdir/ports"
            cd "$srcdir/ports"
            git submodule update --init --recursive
            ;;
        4 ) echo -e "\nGetting documentation V1 with Git...\n"
            git clone https://github.com/aros-development-team/documentation.git "$srcdir/documentation"
            cd "$srcdir/documentation"
            git submodule update --init --recursive
            ;;
        5 ) echo -e "\nGetting binaries V1 with Git...\n"
            git clone https://github.com/aros-development-team/binaries.git "$srcdir/binaries"
            cd "$srcdir/binaries"
            git submodule update --init --recursive
            ;;

        0 ) exit 0
            ;;
    esac
done


input=""
until [ "$input" = "9" ]
do
    cd "$curdir"

    echo -e "\n\n\n\n\n"
    echo -e "*********************"
    echo -e "* Step 3: configure *"
    echo -e "*********************"
    echo -e   "1 .. amiga-m68k   (PAL)"
    echo -e   "2 .. amiga-m68k   (PAL, DEBUG, Serial 115200)"
    echo -e   "3 .. amiga-m68k   (NTSC)"
    echo -e   "4 .. amiga-m68k   (NTSC, DEBUG, Serial 115200)"
    echo -e   "5 .. amiga-m68k   (NTSC, KPRINTF, Serial 115200)"
    echo -e "\n9 .. Go to next step"
    echo -e   "0 .. Exit"

    echo -e "\nEnter number and press <Enter>:"

    read input
    case "$input" in
        1 ) echo -e "\nConfiguring amiga-m68k V1 (PAL)...\n"
            mkdir -p "$portsdir"
            mkdir -p aros-amiga-a68080
            cd aros-amiga-a68080
            "../$srcdir/configure" --target=amiga-m68k --with-cpu=68020-40 --with-fpu=68020 --with-resolution=640x256x4 $configopts
            ;;
        2 ) echo -e "\nConfiguring amiga-m68k V1 (PAL, DEBUG)...\n"
            mkdir -p aros-amiga-a68080
            cd aros-amiga-a68080
            "../$srcdir/configure" --target=amiga-m68k --with-cpu=68020-40 --with-fpu=68020 --with-resolution=640x256x4 --enable-debug=all --with-serial-debug=yes $configopts
            ;;
        3 ) echo -e "\nConfiguring amiga-m68k V1 (NTSC)...\n"
            mkdir -p aros-amiga-a68080
            cd aros-amiga-a68080
            "../$srcdir/configure" --target=amiga-m68k --with-cpu=68020-40 --with-fpu=68020 --with-resolution=640x200x4 $configopts
            ;;
        4 ) echo -e "\nConfiguring amiga-m68k V1 (NTSC, DEBUG)...\n"
            mkdir -p aros-amiga-a68080
            cd aros-amiga-a68080
            "../$srcdir/configure" --target=amiga-m68k --with-cpu=68020-40 --with-fpu=68020 --with-resolution=640x200x4 --enable-debug=all --with-serial-debug=yes $configopts
            ;;

        5 ) echo -e "\nConfiguring amiga-m68k V1 (NTSC, KPRINTF)...\n"
            mkdir -p aros-amiga-a68080
            cd aros-amiga-a68080
            "../$srcdir/configure" --target=amiga-m68k --with-cpu=68020-40 --with-fpu=68020 --with-resolution=640x200x4 --enable-debug=none --with-serial-debug=yes $configopts
            ;;

        0 ) exit 0
            ;;
    esac
done


input=""
until [ "$input" = "9" ]
do
    cd "$curdir"

    echo -e "\n\n\n\n\n"
    echo -e "*****************"
    echo -e "* Step 4: build *"
    echo -e "*****************"
    echo -e "\nYou can only build after you have configured.  This script builds only the kickstart rom and extrom."
    echo -e   "1 .. Begin building"
    echo -e   "9 .. Exit"
    echo -e "\nEnter number and press <Enter>:"

    read input
    case "$input" in
        1 ) echo -e "\nBuilding amiga-a68080 V1...\n"
            cd aros-amiga-a68080
            make $makeopts kernel
	    cat bin/amiga-m68k/AROS/boot/amiga/aros-ext.bin bin/amiga-m68k/AROS/boot/amiga/aros-rom.bin > $curdir/AROS.ROM
	    echo "If everything went well, a MAPROMable file can be found at $curdir/AROS.ROM ."
            ;;
    esac
done

cd "$curdir"

exit 0
OD