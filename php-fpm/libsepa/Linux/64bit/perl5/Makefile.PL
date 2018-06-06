#  _ _ _
# | (_) |__ ___ ___ _ __  __ _   SEPA library · www.libsepa.com
# | | | '_ (_-</ -_) '_ \/ _` |  Copyright (c) 2013-2014 Keppler IT GmbH.
# |_|_|_.__/__/\___| .__/\__,_|_____________________________________________
#                  |_|
# perl/Makefile.PL
# Perl Makefile generator script for SEPA module
# $Id: Makefile.PL 223 2014-04-17 22:45:40Z kk $

#use 5.006;
use ExtUtils::MakeMaker;
# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.

my $libsepa_a = '../lib/libsepa.a';
if (-f '../libsepa/lib/libsepa.a') { $libsepa_a = '../libsepa/lib/libsepa.a'; }

my $libsepa_h = '../../../src/include';
if (-f '../libsepa/include/sepa.h') { $libsepa_h = '../libsepa/include'; }

WriteMakefile(
    NAME              => 'SEPA',
    VERSION_FROM      => 'lib/SEPA.pm',   # finds $VERSION
    PREREQ_PM         => {},              # e.g., Module::Name => 1.1
    ($] >= 5.005 ?     ## Add these new keywords supported since 5.005
      (ABSTRACT_FROM  => 'lib/SEPA.pm',   # retrieve abstract from module
       AUTHOR         => 'Keppler IT GmbH <info@libsepa.com>') : ()),
    LIBS              => [''],            # e.g., '-lm'
    DEFINE            => '',              # e.g., '-DHAVE_SOMETHING'
    INC               => '-I' . $libsepa_h, # e.g., '-I. -I/usr/include/other'
    OBJECT            => '$(O_FILES)',    # link all the C files too
    LDFROM            => '$(OBJECT) ' . $libsepa_a,
);

# <EOF>_____________________________________________________________________
