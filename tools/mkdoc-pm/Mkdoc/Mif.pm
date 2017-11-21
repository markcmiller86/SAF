# Author: Robb Matzke <matzke@llnl.gov>
#
# Purpose:
#     Mkdoc::Mif is a specialization of Mkdoc::Format for dealing with the generation of MIF (a.k.a.
#     Adobe FrameMaker) output.
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

package Mkdoc::Mif;
@ISA = qw/Mkdoc::Format/;

use strict;
use Carp;
use Mkdoc::Format;

# Assign a PgfTag field to all paragraphs that don't start with one.
sub _set_pgftag {
  my($pgftag,$body) = @_;
  $body =~ s/(<Para\s+)(?!<PgfTag)/$1<PgfTag \`$pgftag\'>/g;
  return $body;
}

# Prune empty paragraphs
sub _prune_pgf {
  local($_) = @_;
  s/<Para\s+(<PgfTag[^>]+>\s*)?<ParaLine\s+<String\s+\`\'\s*>\s*>\s*>//g;
  return $_;
}

# Make a new mif output context where $name is the name of the primary file.
sub new {
  my($pkg,$name,%init) = @_;
  $name .= ".mif" unless $name =~ /\.mif$/;
  my($self) = bless {name=>$name}, $pkg;
  return $self->init(%init);
}

# Generate MIF prologue
sub prologue {
  my($self,$mkdoc_url) = @_;
  local($_);

  my($found) = Mkdoc::Format::_find_support_file("start.mif")
    or croak "cannot find file `start.mif'";
  open MIF, $found or croak "cannot open $found: $!";
  $_ = join "", <MIF>;
  close MIF;

  $_ .= "<TextFlow\n";
  $_ .= "  <TFTag `A'>\n";
  $_ .= "  <TFAutoConnect Yes>\n";

  my($title);
  foreach $title (@{$self->{title}}) {
    $_ .= "  <Para\n";
    $_ .= "    <PgfTag `Title'>\n";
    $_ .= "    <ParaLine\n";
    $_ .= "      <TextRectID 7>\n";
    $_ .= "      <String `$title'>\n";
    $_ .= "    >\n";
    $_ .= "  >\n";
  }

  $_ .= "  <Para\n";
  $_ .= "    <PgfTag `Body'>\n";
  $_ .= "    <ParaLine\n";
  $_ .= "      <Font <FTag `Emph'>>\n";
  $_ .= "      <String `Active Developers'>\n";
  $_ .= "      <Font <FTag `'>>\n";
  $_ .= "    >\n";
  $_ .= "  >\n";
  my($author);
  foreach $author (@{$self->{author}}) {
    $_ .= "  <Para\n";
    $_ .= "    <PgfTag `Author'>\n";
    $_ .= "    <ParaLine\n";
    $_ .= "      <String `$author'>\n";
    $_ .= "    >\n";
    $_ .= "  >\n";
  }

  $_ .= "  <Para\n";
  $_ .= "    <PgfTag `Body'>\n";
  $_ .= "    <ParaLine\n";
  $_ .= "      <Font <FTag `Emph'>>\n";
  $_ .= "      <String `Inactive Developers'>\n";
  $_ .= "      <Font <FTag `'>>\n";
  $_ .= "    >\n";
  $_ .= "  >\n";
  my($oldauthor);
  foreach $oldauthor (@{$self->{oldauthor}}) {
    $_ .= "  <Para\n";
    $_ .= "    <PgfTag `Author'>\n";
    $_ .= "    <ParaLine\n";
    $_ .= "      <String `$oldauthor'>\n";
    $_ .= "    >\n";
    $_ .= "  >\n";
  }

  $_ .= "  <Para\n";
  $_ .= "    <PgfTag `Body'>\n";
  $_ .= "    <ParaLine\n";
  $_ .= "      <Font <FTag `Emph'>>\n";
  $_ .= "      <String `Acknowledgements'>\n";
  $_ .= "      <Font <FTag `'>>\n";
  $_ .= "    >\n";
  $_ .= "  >\n";
  my($acknow);
  foreach $acknow(@{$self->{acknow}}) {
    $_ .= "  <Para\n";
    $_ .= "    <PgfTag `Author'>\n";
    $_ .= "    <ParaLine\n";
    $_ .= "      <String `$acknow'>\n";
    $_ .= "    >\n";
    $_ .= "  >\n";
  }

  my($copyright,$cr_prev);
  foreach $copyright (@{$self->{copyright}}) {
    my($year,$owner,$rights) = split /\000/, $copyright;
    $rights ||= "All rights reserved.";
    $rights =~ s/^[ \t]*/      <String \`/gm;
    $rights =~ s/[ \t]*$/ \'>/gm;

    $_ .= "  <Para\n";
    $_ .= "    <PgfTag `Copyright'>\n";
    $_ .= "    <Pgf <PgfPlacement PageTop>>\n" unless $cr_prev;
    $_ .= "    <ParaLine\n";
    $_ .= "      <String `Copyright \xa9  $year. $owner.'>\n";
    $_ .= "    >\n";
    $_ .= "  >\n";
    $_ .= "  <Para\n";
    $_ .= "    <PgfTag `Body'>\n";
    $_ .= "    <ParaLine\n";
    $_ .= $rights;
    $_ .= "    >\n";
    $_ .= "  >\n";
    $cr_prev = $copyright;
  }

  if (exists $self->{disclaimer}) {
    $_ .= "  <Para\n";
    $_ .= "    <PgfTag `Body'>\n";
    $_ .= "    <ParaLine\n";
    $_ .= "      <Font <FTag `Emph'>>\n";
    $_ .= "      <String `Disclaimer'>\n";
    $_ .= "      <Font <FTag `'>>\n";
    $_ .= "    >\n";
    $_ .= "  >\n";
    my($disclaimer);
    for $disclaimer (@{$self->{disclaimer}}) {
      $disclaimer =~ s/^[ \t]+//gm;
      $_ .= "  <Para\n";
      $_ .= "    <PgfTag `Body'>\n";
      $_ .= "    <ParaLine\n";
      $_ .= "      <String `" . $self->context(undef,$disclaimer) . "'>\n";
      $_ .= "    >\n";
      $_ .= "  >\n";
    }
  }

  $_ .= "  <Para\n";
  $_ .= "    <PgfTag `Body'>\n";
  $_ .= "    <ParaLine\n";
  $_ .= "      <String `This document was generated from library source code by ";
  $_ .= $self->url($mkdoc_url,$self->code("mkdoc")) . "'>\n";
  $_ .= "      <String `. DO NOT MODIFY THIS FILE!'>\n";
  $_ .= "    >\n";
  $_ .= "  >\n";

  return $_;
}


# Special symbols
sub sym_section {"\\xa4 "}

# Generate a target for a cross reference.  The $tag is the name of the target and is an
# object ID that consists of only letters, digits, and underscores.
sub target {
  my($self,$tag) = @_;
  return "'><Marker <MType 9><MText `$tag'>><String `";
}

# Generate a cross reference to some target.  Arguments are similar to `target' method. This assumes
# we're in a <String> context.  This one is called for things like table of contents and indices.
sub xref {
  my($self,$tag,$text) = @_;
  return ("$text'>" .
	  "<XRef <XRefName `Page'><XRefSrcText `$tag'><XRefSrcFile `'>><XRefEnd>" .
	  "<String `");
}

# Generate a cross reference like &xref, but call this one for embedded cross references.
sub xref2 {
  my($self,$tag,$text) = @_;
  return ("$text\[p'>" .
	  "<XRef <XRefName `Page'><XRefSrcText `$tag'><XRefSrcFile `'>><XRefEnd>" .
	  "<String `]");
}

# Quote all typesetting special characters so they appear in the documentation.
sub escape {
  my($self,$tokref,$string) = @_;
  $string =~ s/([<>])/Mkdoc::Format::_toksave($tokref,"\\$1")/eg;
  $string =~ s/\`/Mkdoc::Format::_toksave($tokref,"\\Q")/eg;
  $string =~ s/\'/Mkdoc::Format::_toksave($tokref,"\\q")/eg;
  return $string;
}

# Typeset the string as code. This assumes we're in a <String> context.
sub code {
  my($self,$string) = @_;
  return "'><Font <FTag `Code'>><String `$string'><Font <FTag `'>><String `";
}

# Typeset a word as a variable (formal argument). This assumes we're in a <String> context.
sub var {
  my($self,$name) = @_;
  return "'><Font <FTag `Var'>><String `$name'><Font <FTag `'>><String `";
}

# Typeset a string as italic.  This assumes we're in a <String> context.
sub italic {
  my($self,$string) = @_;
  return "'><Font <FTag `Italic'>><String `$string'><Font <FTag `'>><String `";
}

# Typeset with emphasis. This assumes we're in a <String> context.
sub emph {
  my($self,$string) = @_;
  return "'><Font <FTag `Emph'>><String `$string'><Font <FTag `'>><String `";
}

# Typeset as a file name
sub file {
  my($self,$name) = @_;
  return "\\Q$name\\q";
}

# Join paragraphs together. We assume that the caller has already entered <Para> context. The return
# string will supply the <ParaLine> contexts.
sub para_join {
  my($self,@para) = @_;
  my(@fixed_paragraph,$paragraph);

  foreach $paragraph (@para) {
    if ($paragraph =~ /^[ \t]/) {
      # Every line of this paragraph should be a paragraph, and one blank line shall follow.
      my($line);
      $paragraph = "'>>><Para <PgfTag `FieldBodyCode'><ParaLine <String `" . $paragraph;
      $paragraph =~ s/\n/\'>>><Para <PgfTag \`FieldBodyCode\'> <ParaLine <String \`/g;
      $paragraph .= "'>>><Para <PgfTag `FieldBodyCode'><ParaLine <String ` "; # space reqd; _prune_pgf
    }
    $paragraph =~ s/\n/ \'>\n  >\n  <ParaLine\n    <String \`/g;
    push @fixed_paragraph, $paragraph;
  }
  return join " '>\n  >\n>\n<Para\n  <ParaLine\n    <String `", @fixed_paragraph;
}

# Provide a paragrah context. This actually provides the paragraph tag to all embedded paragraphs
# that don't have one.
sub paragraph {
  my($self,@items) = @_;
  local($_);

  $_ .= "<Para\n";
  $_ .= "  <ParaLine\n";
  $_ .= "    <String `" . $items[0] . "'>\n";
  $_ .= "  >\n";
  $_ .= ">";
  return _set_pgftag 'Body', $_;
}

# Make a table
sub table {
  my($self,$format,@items) = @_;
  local($_);

  # Choose paragraph styles
  my($hdr_tag,$bdy_tag);
  if ($format eq 'var') {
    $hdr_tag = 'FieldArgHdr';
    $bdy_tag = 'FieldArgBody';
  } elsif ($format eq 'bullet') {
    $hdr_tag = 'FieldBullet';
  } elsif ($format eq 'code') {
    $hdr_tag = 'FieldBodyCode';
    $bdy_tag = 'FieldBody';
  } elsif ($format eq 'b') {
    $hdr_tag = 'FieldHdr';
    $bdy_tag = 'FieldBody';
  } elsif ($format eq 'xref' || $format eq 'table') {
    $hdr_tag = 'FieldBodyCode';
    $bdy_tag = 'FieldBodyCodeComment';
  } else {
    croak "unknown table format: $format";
  }

  while (@items) {
    my($head,$body) = (shift @items, shift @items);
    $_ .= "\n" if length $_;
    $head =~ s/(<XRef\s)/<Char Tab>$1/g if $format eq 'xref';

    # Heading
    $_ .= "<Para\n";
    $_ .= "  <PgfTag `$hdr_tag'>\n";
    $_ .= "  <ParaLine\n";
    $_ .= "    <String `$head'>\n";
    $_ .= "  >\n";
    $_ .= ">";

    # Body, which may already be some paragraph
    unless ($format eq 'bullet') {
      $_ .= "\n";
      if ($body =~ /^\s*<Para/) {
	$_ .= $body;
      } else {
	my($b);
	$b .= "<Para\n";
	$b .= "  <PgfTag `$bdy_tag'>\n";
	$b .= "  <ParaLine\n";
	$b .= "    <String `$body'>\n";
	$b .= "  >\n";
	$b .= ">";
	$_ .= _set_pgftag $bdy_tag, $b;
      }
    }
  }
  return $_;
}

# Wrap chapter items in a chapter
sub chapter {
  my($self,$chapter,$title,@items) = @_;
  $title = $chapter->name unless defined $title;
  $title = $self->escape(undef,$title);
  $title = join ".  ", grep {$_ ne ""} $chapter->secnum, $title;
  local($_);

  my($tag) = "Chapter";
  $tag = 'FuncTitle' if split(/\./,$chapter->secnum)>1;

  $_ .= "#" x 100 . "\n" . "#" x 100 . "\n";
  $_ .= "<Para\n";
  $_ .= "  <PgfTag `$tag'>\n";
  $_ .= "  <ParaLine\n";
  $_ .= "    <String `" . $self->target($chapter->objid) . $title . "'>\n";
  $_ .= "  >\n";
  $_ .= ">\n";
  $_ .= join "\n", @items;
  return _prune_pgf $_;
}

# Insert a table-of-contents entry.
sub do_toc_item {
  my($self,$object,$title,$class,$desc,$xref) = @_; #all required
  my($tag) = 2==split(/\D/, $object->secnum) ? "TocFunction" : "TocChapter";
  $xref =~ s/^(.*?)<XRef/\'><XRef/; # remove leading stuff from xref
  local($_);
  $_ .= "<Para\n";
  $_ .= "  <PgfTag `$tag'>\n";
  $_ .= "  <ParaLine\n";
  $_ .= "    <String `" . join (" ", grep /\S/, $object->secnum, $title, "[$class]");
  $_ .=               ($desc=~/^\S/?" ":"") . $desc . "'>\n";
  $_ .= "    <Char Tab>\n";
  $_ .= "    <String `$xref'>\n";
  $_ .= "  >\n";
  $_ .= ">";
  return $_;
}

# Function prototype
sub proto {
  my($self,$function) = @_;
  my(@formals) = $function->formals;
  local($_);

  $_ .= "<Para\n";
  $_ .= "  <PgfTag `FieldSynType'>\n";
  $_ .= "  <ParaLine\n";
  $_ .= "    <String `" . $function->rettype . "'>\n";
  $_ .= "  >\n";
  $_ .= ">\n";
  $_ .= "<Para\n";
  $_ .= "  <PgfTag `FieldSynName'>\n";
  $_ .= "  <ParaLine\n";
  $_ .= "    <String `" . $function->name . ($function->class eq 'symbol'?"":"(") . "'>\n";
  $_ .= "  >\n";
  $_ .= ">\n";

  my($formal);
  foreach $formal (@formals) {
    $_ .= "<Para\n";
    $_ .= "  <PgfTag `FieldSynArg'>\n";
    $_ .= "  <ParaLine\n";
    if (ref $formal) {
      $_ .= "    <String `" . $formal->{type} . "'>\n";
      $_ .= "    <Font <FTag `CodeVar'>>\n";
      $_ .= "    <String `" . $formal->{name} . "'>\n";
      $_ .= "    <Font <FTag `'>>\n";
      $_ .= "    <String `" . $formal->{array} . "'>\n" if $formal->{array};
    } else {
      $_ .= "    <String `$formal'>\n"; # wasn't parsed
    }
    $_ .= "  >\n";
    $_ .= ">\n";
  }
  unless (@formals || $function->class eq 'symbol') {
    $_ .= "<Para\n";
    $_ .= "  <PgfTag `FieldSynArg'>\n";
    $_ .= "  <ParaLine\n";
    $_ .= "    <String `void'>\n";
    $_ .= "  >\n";
    $_ .= ">\n";
  }

  unless ($function->class eq 'symbol') {
    $_ .= "<Para\n";
    $_ .= "  <PgfTag `FieldSynName'>\n";
    $_ .= "  <ParaLine\n";
    $_ .= "    <String `)'>\n";
    $_ .= "  >\n";
    $_ .= ">\n";
  }
  return $_;
}

# Create an entry for a permuted index
sub pindex_item {
  my($self,$object,$pre,$post) = @_;
  local($_);

  $_ .= "<Para\n";
  $_ .= "  <PgfTag `PermIndex'>\n";
  $_ .= "  <ParaLine\n";
  $_ .= "    <Char Tab>\n";
  $_ .= "    <String `$pre'>\n";
  $_ .= "    <Char Tab>\n";
  $_ .= "    <String `$post'>\n";
  $_ .= "    <Char Tab>\n";
  $_ .= "    <String `" . $self->xref($object->objid) . "'>\n";
  $_ .= "  >\n";
  $_ .= ">";
  return $_;
}

# Create an entry for a concept index
sub cindex_item {
  my($self,$object,$text) = @_;
  local($_);

  $_ .= "<Para\n";
  $_ .= "  <PgfTag `ConceptIndex'>\n";
  $_ .= "  <ParaLine\n";
  $_ .= "    <String `$text'>\n";
  $_ .= "    <Char Tab>\n";
  $_ .= "    <String `" . $self->xref($object->objid) . "'>\n";
  $_ .= "  >\n";
  $_ .= ">";
  return $_;
}
1;
