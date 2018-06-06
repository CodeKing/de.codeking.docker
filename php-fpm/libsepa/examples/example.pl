#!/usr/bin/perl
#  _ _ _
# | (_) |__ ___ ___ _ __  __ _   SEPA library · www.libsepa.com
# | | | '_ (_-</ -_) '_ \/ _` |  Copyright (c) 2013-2014 Keppler IT GmbH.
# |_|_|_.__/__/\___| .__/\__,_|_____________________________________________
#                  |_|
# perl/example.pl
# Example for libsepa usage with Perl
# $Id: example.pl 277 2016-06-07 18:34:45Z kk $

use strict;
use warnings;
use lib qw(blib/lib blib/arch/auto/SEPA);
use SEPA;

# un-comment the following lines to activate your license:
#SEPA::init(SEPA_INIT_LICUSER, "YOUR NAME") or die "SEPA::init(user) failed";
#SEPA::init(SEPA_INIT_LICCODE, "YOUR LICENSE CODE") or die "SEPA::init(license) failed";

my $iban  = SEPA::IBAN_convert("DE", "1234567890", "51010800");
my $bic   = SEPA::IBAN_getBIC($iban);
my $valid = SEPA::IBAN_check($iban);

print "IBAN=$iban, BIC=$bic, VALID=$valid\n";

my $sepa = new SEPA(SEPA_MSGTYPE_DDI);
$sepa->setIBAN("DE56510108001234567890");
$sepa->setBIC("BANKDEFFXXX");
$sepa->setName("Mustermann u. Co. KG");
$sepa->setCreditorIdentifier("DE98ZZZ09999999999");
# $sepa->setBatchBooking(0);
my $tx = {
    'seq'    => 'FRST',
    'id'     => 'R2017742-1',
    'name'   => 'Carl Customer',
    'mref'   => '(MandateReference)',
    'mdate'  => '2013-09-24',
    'amount' => 123.45,
    'iban'   => 'DE56510108001234567890',
    'bic'    => 'BANKDEZZXXX',
    'ref'    => 'Invoice R2017742 from 17/10/2013',
    };
$sepa->add($tx);
my $xml = $sepa->toXML();

# <EOF>_____________________________________________________________________
