# Author: Robb Matzke <matzke@llnl.gov>
#
# Purpose:
#     Mkdoc::Function is a specialization of Mkdoc::Docbase for dealing with functions, macros, and
#     similar kinds of documentable objects.
#
# Copyright(C) 1999 The Regents of the University of California.
#     This work  was produced, in  part, at the  University of California, Lawrence Livermore National
#     Laboratory    (UC LLNL)  under    contract number   W-7405-ENG-48 (Contract    48)   between the
#     U.S. Department of Energy (DOE) and The Regents of the University of California (University) for
#     the  operation of UC LLNL.  Copyright  is reserved to  the University for purposes of controlled
#     dissemination, commercialization  through formal licensing, or other  disposition under terms of
#     Contract 48; DOE policies, regulations and orders; and U.S. statutes.  The rights of the Federal
#     Government  are reserved under  Contract 48 subject  to the restrictions agreed  upon by DOE and
#     University.
# 
# Copyright(C) 1999 Sandia Corporation.
#     Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive license for use of this work
#     on behalf of the U.S. Government.  Export  of this program may require a license from the United
#     States Government.
# 
# Disclaimer:
#     This document was  prepared as an account of  work sponsored by an agency  of  the United States
#     Government. Neither the United States  Government nor the United States Department of Energy nor 
#     the  University  of  California  nor  Sandia  Corporation nor any  of their employees  makes any 
#     warranty, expressed  or  implied, or  assumes   any  legal liability  or responsibility  for the 
#     accuracy,  completeness,  or  usefulness  of  any  information, apparatus,  product,  or process 
#     disclosed,  or  represents that its  use would   not infringe  privately owned rights. Reference 
#     herein  to any  specific commercial  product,  process,  or  service by  trade  name, trademark, 
#     manufacturer,  or  otherwise,  does  not   necessarily  constitute  or  imply  its  endorsement, 
#     recommendation, or favoring by the  United States Government   or the University of  California.  
#     The views and opinions of authors expressed herein do not necessarily state  or reflect those of
#     the  United  States Government or  the   University of California   and shall  not be  used  for
#     advertising or product endorsement purposes.
# 
# Acknowledgements:
#     Robb P. Matzke              LLNL - Author of various tools
#     Mark C. Miller              LLNL - Alpha/Beta user; minor debugging/enhancements 
#     Matthew J. O'Brien          LLNL - Author of various tools
#     James F. Reus               LLNL - Alpha/Beta user

package Mkdoc::Function;
@ISA = qw/Mkdoc::Docbase/;

use strict;
use Carp;
use Mkdoc::Docbase;

sub new {
  my($pkg,%args) = @_;

  croak "no function name specified" unless exists $args{name};
  my($self) = bless \%args, $pkg;
  return $self->exists;
}

# Get/set macro property
sub is_macro {
  my($self,$macro) = @_;
  $self->{macro} = $macro if @_>=2;
  return $self->{macro};
}

# Get/set return type
sub rettype {
  my($self,$rettype) = @_;
  $self->{rettype} = $rettype if @_>=2;
  return $self->{rettype};
}

# Get/set formal argument list.
sub formals {
  my($self,@formals) = @_;
  $self->{formals} = \@formals if @_>=2;
  return @{$self->{formals}};
}

# Get/set body
sub body {
  my($self,$body) = @_;
  $self->{body} = $body if @_>=2;
  return $self->{body};
}

# Typeset an entire function
sub render {
  my($self,$output,$exrefs,@omit) = @_;
  my(@items,@table,%see_also,$leader);
  local($_);

  # Save function source
  my($src) = $self->source;
  $src =~ s/^\/\*.*?\*\/\s*(\/\*.*?\*\/\s*)?//s;
  $output->multi_output($self,$output->example($self,$src,undef,$exrefs,1),"SC_".$self->objid);

  # Leader
  $leader .= $output->code($self->name) . " is a " . $self->class . $self->defined_in($output) . ".";
  push @items, $output->paragraph($leader);

  # Function prototype.
  push @table, 'Synopsis', $output->proto($self);

  # Formal arguments that have descriptions
  my(@formals,%formals,@arg_table) = $self->formals;
  map {$formals{$_->{name}} = $_ if ref} @formals;
  for my $formal ($self->formals) {
    next if !ref $formal || !$formal->{doc};
    my $qualifiers = " [" . $formal->{qualifiers} . "]" if $formal->{qualifiers};
    push @arg_table, "$formal->{name}$qualifiers", $output->context($self,$formal->{doc},\%formals,\%see_also,$exrefs);
  }
  my $args_doc;
  $args_doc = $self->prologue->{Arguments} . "\n" if $self->prologue->{Arguments};
  $args_doc .= $output->table('var',@arg_table) if @arg_table;
  push @table, 'Formal Arguments', $args_doc if $args_doc;

  # Other fields
  $self->std_headings($output,\@table,\%formals,\%see_also,$exrefs,@omit);
  push @items, $output->table('b',@table);

  # Save the whole chapter (or more precisely, the section)
  my($all) = $output->chapter($self,$self->prologue->{Purpose},@items);
  return $output->multi_output($self,$all);
}

# Function summry.
sub summary {
  my($self) = @_;
  my(@parts,$formal) = $self->rettype;
  foreach $formal ($self->formals) {
    if (ref $formal) {
      push @parts, $formal->{type} . $formal->{name} . $self->{array};
    } else {
      push @parts, $formal;
    }
  }
  return join "\000", @parts;
}

1;
