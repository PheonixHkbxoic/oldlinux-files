(1) Introduction
(2) Trying out ELKS using the disk images
(3) Compiling your own kernel



(1) Introduction

Hello, and welcome to the exciting world of Linux-8086, ELKS, or the
Embeddable Linux Kernel Subset!  This is a project which will eventually
produce a Linux-like OS for the 8086 (186, 286) as well as for the Psion
series of processors.  But, as you have this file, you probably already know
that, so let's get down to business.



(2) Trying out ELKS using the images

So you want to give ELKS a try, but you don't want all the fuss and bother of
compiling the kernel and the tools yourself.  No problem.  All you need to
do is grab the images.zip file from the download section of
http://www.elks.ecs.soton.ac.uk/ and save it to its own
directory. These files can be copied to disks and used to boot ELKS. For
details of which files you want and how to use them, please see the
INSTALL file included in this distribution.

(3) Compiling your own kernel

Compiling your own kernel is a bit more involved, but we'll get through it. 
First, you're going to need a few files from the download section of
http://www.elks.ecs.soton.ac.uk/

                  elks-x.x.xx.tar.gz
                  images.zip
                  elkscmd-xxxxxxxx.tar.gz
                  Dev86bin-x.xx.x.tar.gz

Okay, so you've got the files.  Personally, I like to keep them in /ELKS/, so
that's what I'll assume you're doing.  So cd to /ELKS, and let's get
started.  The first thing we need to do is install the development tools.

                  cp Dev86bin-x.x.xx.tar.gz /
                  cd /
                  tar xvzf Dev86bin-x.x.xx.tar.gz
                  rm Dev86bin-x.x.xx.tar.gz

Now you should have the 8086 development tools installed.  Next, we need to
cd back to /ELKS/ and untar the ELKS kernel sources. So

                  cd /ELKS
                  tar xvzf elks-x.x.xx.tar.gz
                  cd ./elks

Now that we're in the source directory, we can start setting up the kernel.  
The first thing that needs to be done is

                   make config

You'll be asked a few questions about how you want the kernel set up.  For
now, we'll accept the defaults and just keep hitting enter until we get
to the end, then

                  make

You'll see a lot of warnings go by for a few minutes, and then we'll assume
a perfect compile.  So we'll now have a diskette image file with our new
kernel on it.  We'll assume we built for the 8086.  The image is a little
buried, so we'll have to dig into the source tree to find it.

                  cd /ELKS/elks/arch/i86/
                  ls

If the compile was successful, you'll have an "Image" file here, and you'll
have to get it onto a diskette.  I'll assume "/dev/fd0".

                  dd if=./Image of=/dev/fd0 bs=8192

This will be your boot disk, but you'll need a root disk, too, so

                  cd /ELKS/
                  unzip images.zip
                  dd if=./root of=/dev/fd0 bs=8192

After that, you should be able to use the boot and root disks and watch ELKS
work its Linux-like magic on your machine.  Have fun!

---------------------------------------------------------------------
If you have any suggestions for this readme send your comments to
semjaza@mytalk.com
or
linux-8086@vger.rutgers.edu

--Phillip J Rhoades
