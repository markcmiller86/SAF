# Author: Robb Matzke <matzke@llnl.gov>
#
# Purpose:
#      Functions for manipulating file names.
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

package Filename;
require Exporter;
@ISA = qw(Exporter);
@EXPORT = qw(fn_normalize fn_split fn_absolute fn_relative);

use strict;
use Cwd;

####################################################################################################
# Normalize a file name by removing duplicate and trailing slashes and `.' components, and unecessary
# `..' components.
sub fn_normalize {
  my($name) = @_;
  my(@retval);
  local($_);

  return "/" if $name =~ /^\/+$/;                       # root directory
  $name =~ s/\/+$//g;		                        # remove trailing slashes
  foreach (split /\/+/, $name) {
    if ($_ eq '.') {
      # discard
    } elsif ($_ eq '..' && 1==@retval && $retval[0] eq "") {
      # `/..' is the same as `/'
    } elsif ($_ eq '..' && @retval && $retval[-1] ne "..") {
      pop @retval;
    } else {
      push @retval, $_;
    }
  }
  return "." unless @retval;
  return "/" if 1==@retval && $retval[0] eq "";
  return join "/", @retval;
}

####################################################################################################
# Given a file name return the directory and basename.  The original name can be reconstructed
# with "$dir/$base" and will not result in duplicated or trailing slashes.
sub fn_split {
  my($name) = fn_normalize @_;
  return ("","") if $name eq "/";			# root directory
  return (".",$name) unless $name =~ /\//;              # no slashes
  my($dir,$base) = $name =~ /(.*)\/(.*)/;
  return ($dir,$base);
}

####################################################################################################
# Given a name relative to some directory, return the absolute name.
sub fn_absolute {
  my($name,$dir) = @_;
  $name = fn_normalize $name;
  return $name if $name =~ /^\//;                       # already absolute
  $dir  = fn_normalize($dir || cwd);
  return fn_normalize "$dir/$name";
}

####################################################################################################
# Given a directory name and a file (or other) name, compute an equivalent name for the file which
# is relative to the given directory.
sub fn_relative {
  my($name,$dir) = @_;
  $name = fn_absolute $name;
  $dir  = fn_absolute($dir || cwd);
  my(@dir)  = split /\//, $dir;
  my(@saved,$prefix);

  while (@dir) {
    $prefix = join "/", @dir;
    last if $prefix eq substr $name, 0, length $prefix;
    push @saved, "..";
    pop @dir;
  }
  $prefix .= "/" unless @saved; # so fn_relative("/foo/bar","/foo/bar/baz") works
  return fn_normalize(join("/",@saved) . substr $name, length $prefix);
}

1;
