#  _ _ _
# | (_) |__ ___ ___ _ __  __ _   SEPA library · www.libsepa.com
# | | | '_ (_-</ -_) '_ \/ _` |  Copyright (c) 2013-2014 Keppler IT GmbH.
# |_|_|_.__/__/\___| .__/\__,_|_____________________________________________
#                  |_|
# perl/t/SEPA.t
# $Id: SEPA.t 315 2017-12-04 20:20:37Z kk $

# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl SEPA.t'

#########################

use Test::More tests => 12;
BEGIN { use_ok('SEPA') };

#########################

# Insert your test code below, the Test::More module is use()ed here so read
# its man page ( perldoc Test::More ) for help writing this test script.

use SEPA;

# convert to IBAN
my ($iban, $bic, $name, $flags);
$iban = SEPA::IBAN_convert("DE", "960994510", "55090500");
ok($iban eq 'DE60550905000960994510', 'IBAN conversion successful');

$bic = SEPA::IBAN_getBIC($iban);
ok($bic eq 'GENODEF1S01', 'BIC conversion successful');

$iban = SEPA::IBAN_convert("DE", "7325022", "26580070");
ok($iban eq 'DE32265800700732502200', 'IBAN conversion successful');

$bic = SEPA::IBAN_getBIC($iban);
ok($bic eq 'DRESDEFF265', 'BIC conversion successful');

$name = SEPA::BIC_getBankName("BYLADEM1ERH");
ok($name eq 'STADT-UND KREISSPARKASSE ERLANGEN HOECHSTADT HERZOGENAURACH', 'SCL bank name lookup successful');

$flags = SEPA::BIC_getBankFlags("BYLADEM1ERH");
ok($flags == 11, 'SCL bank flags lookup successful');

# check conversion status code
my $status;
$iban = SEPA::IBAN_convert("DE", "40033086", "30020500", \$status);
ok($status == 15, 'conversion status ok');

# create SEPA object
my $sepa = new SEPA(SEPA_MSGTYPE_DDI);
ok(defined($sepa), 'got SEPA object');

# check IBANs
ok(SEPA::IBAN_check('DE22380200904030000000'), 'IBAN check (ok)');
ok(!SEPA::IBAN_check('DE22380200904030000001'), 'IBAN check (wrong)');

$sepa->setIBAN("DE24210500001234567895");
$sepa->setBIC("BANKDEFFXXX");
$sepa->setName("Mustermann u. Co. KG");
$sepa->setCreditorIdentifier("DE98ZZZ09999999999");
$sepa->setDDType(SEPA_DDTYPE_B2B);
$sepa->setBatchBooking(0);
my $tx = {
    'seq'    => 'FRST',
    'id'     => 'R2017742-1',
    'name'   => 'Carl Customer',
    'mref'   => '(MandateReference)',
    'mdate'  => '2013-09-24',
    'amount' => 123.45,
    'iban'   => 'DE24210500001234567895',
    'bic'    => 'BANKDEZZXXX',
    'ref'    => 'Invoice R2017742 from 17/10/2013',
    };
$sepa->add($tx);

# get XML
my $xml = $sepa->toXML();
# print STDERR $xml;

# destroy object
undef($sepa);
ok(!defined($sepa), 'SEPA object destroyed');

# <EOF>_____________________________________________________________________
