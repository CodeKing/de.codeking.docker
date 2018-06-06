#  _ _ _
# | (_) |__ ___ ___ _ __  __ _   SEPA library · www.libsepa.com
# | | | '_ (_-</ -_) '_ \/ _` |  Copyright (c) 2013-2017 Keppler IT GmbH.
# |_|_|_.__/__/\___| .__/\__,_|_____________________________________________
#                  |_|
# perl/lib/SEPA.pm
# Perl module "SEPA" (interface to libsepa)
# $Id: SEPA.pm 317 2018-03-21 20:33:55Z kk $

package SEPA;

#use 5.006;
use strict;
use warnings;

require Exporter;

our @ISA = qw(Exporter);

our %EXPORT_TAGS = (
  'all'       => [ qw(
    SEPA_MSGTYPE_DDI SEPA_MSGTYPE_CTI
    SEPA_DDTYPE_CORE SEPA_DDTYPE_COR1 SEPA_DDTYPE_B2B
    SEPA_INIT_LICUSER SEPA_INIT_LICCODE SEPA_INIT_RULEBOOK
    SEPA_SCL_SCT SEPA_SCL_SDD SEPA_SCL_COR1 SEPA_SCL_B2B
  ) ],
);

our @EXPORT_OK = ( @{ $EXPORT_TAGS{'all'} } );

our @EXPORT = ( @{ $EXPORT_TAGS{'all'} } );

our $VERSION = '2.13';

require XSLoader;
XSLoader::load('SEPA', $VERSION);

# Preloaded methods go here.


package SEPA::StatementParser;
use strict;
use warnings;
require Exporter;

our @ISA = qw(Exporter);

our %EXPORT_TAGS = (
  'all'       => [ qw(
    SEPA_STMT_FORMAT_MT940
  ) ],
);

our @EXPORT_OK = ( @{ $EXPORT_TAGS{'all'} } );

our @EXPORT = ( @{ $EXPORT_TAGS{'all'} } );

our $VERSION = '2.13';


1;
__END__

=head1 NAME

SEPA - Perl extension for creating SEPA XML messages (direct debit, cash transfer), checking
validity of IBAN/BIC and converting account numbers and bank identifiers to IBAN/BIC.

=head1 SYNOPSIS

  use SEPA;

  $iban = SEPA::IBAN_convert("DE", "1234567890", "51010800");
  $bic  = SEPA::IBAN_getBIC($iban);

  $sepa = new SEPA(SEPA_MSGTYPE_DDI);
  $sepa->setIBAN("DE56510108001234567890");
  $sepa->setBIC("BANKDEFFXXX");
  $sepa->setName("Mustermann u. Co. KG");
  $sepa->setCreditorIdentifier("DE98ZZZ09999999999");
  my $tx = {
      'seq'    => 'FRST',
      'id'     => 'R2017742-1',
      'name'   => 'Carl Customer',
      'mref'   => '(Mandate Reference)',
      'mdate'  => '2013-09-24',
      'amount' => 123.45,
      'iban'   => 'DE56510108001234567890',
      'bic'    => 'BANKDEZZXXX',
      'ref'    => 'Invoice R2017742 from 17/10/2013',
      };
  $sepa->add($tx);
  $xml = $sepa->toXML();

=head1 SEE ALSO

libsepa website: F<http://www.libsepa.com>

=head1 AUTHOR

Keppler IT GmbH, E<lt>support@libsepa.comE<gt>

=head1 COPYRIGHT AND LICENSE

Copyright (C) 2013-2017 by Keppler IT GmbH.

You need a valid license to use libsepa. See www.libsepa.com for more details.

=cut

# <EOF>_____________________________________________________________________
