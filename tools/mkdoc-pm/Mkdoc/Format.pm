# Author: Robb Matzke <matzke@llnl.gov>
#
# Purpose:
#     Base class for formatted output.
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

package Mkdoc::Format;
use strict;
use Mkdoc::Function;
use Filename;
use Carp;

my($MkdocURL) = "http://www.wcrtc.net/~matzke/mkdoc.html";

# Other classes inherit from this one. It makes no sense to instantiate this class.
sub new {croak "Do not instantiate this class"};

# Most packages will want to override these
sub target;			# make a target for a cross reference
sub xref;			# cross reference to a target
sub escape;			# escape special characters
sub code;			# typeset a short phrase of source code
sub var;			# typeset a short phrase as a variable
sub italic;			# typeset a short phrase in an italic font
sub emph;			# emphasize a short phrase
sub file;			# typeset a short phrase as a file name
sub para_join;			# join paragraphs together
sub chapter_item;		# create a chapter item
sub chapter;			# wrap chapter items
sub do_toc_item;		# helper to create a table of contents item
sub toc;			# wrap items in a table-of-contents
sub table;			# wrap items in a table
sub paragraph;			# provide a paragraph context

# Most packages will probably not need to override these
sub prologue;			# prologue of main file
sub epilogue;			# epilogue of main file
sub output;			# send all output to main file
sub funcall;			# typeset a function call
sub url;			# typeset a universal resource locator (URL)
sub example;			# typeset example code
sub multi_output;		# send some output to an auxiliary file
sub toc_item;			# create a table of contents item

# These functions are not even related to typesetting
sub multipart;			# get/set multipart flag
sub xref_list;			# get/set a cross reference association
sub name;			# get/set main file name

# Special symbols
sub sym_section {"Section "};

# Colors
my($CommentColor) = 'red';
my($LineNumColor) = 'green';

# Initialize a fresh object. The hash values are all array references. The keys are:
# title, author, oldauthor, acknow, copyright, disclaimer, navdir, outdir, figdir
sub init {
  my($self,%args) = @_;
  $self->{title} = $args{title};
  $self->{author} = $args{author};
  $self->{oldauthor} = $args{oldauthor};
  $self->{acknow} = $args{acknow};
  $self->{copyright} = $args{copyright};
  $self->{disclaimer} = $args{disclaimer};
  $self->{navdir} = $args{navdir};
  $self->{outdir} = $args{outdir};
  $self->{figdir} = $args{figdir};
  return $self;
}

# Given an object, return the URL (file name, etc.) for the object relative the directory in which
# $self appears. This is used as the URL of the object in the summary file.
sub url_of {
  my($self,$object) = @_;
  my($output_name) = $self->name;
  my($base) = $output_name =~ /.*\/(.*)/;
  $base ||= $output_name;
  my($ext) = $base =~ /.*(\..*)/;

  if ($self->multipart) {
    return $object->objid . $ext;
  } else {
    return $base . "#" . $object->objid;
  }
}

####################################################################################################
# Load external cross reference information into the global cross reference hash. The %refs is a
# hash indexed by object name and whose value is the object ID from a summary file of a previous
# invocation of this program.
sub load_exrefs {
  my($self,$out_dir,%exrefs) = @_;
  my(%retval);
  local($_);

  foreach (keys %exrefs) {
    my($objid,$name) = ($exrefs{$_},$_);
    $objid = fn_relative($objid,$out_dir);
    my($xref) = $retval{$name} = $self->xref3($objid,$self->code($name));
    $self->xref_list($name,$xref);
  }
  return %retval;
}

# Create a target for a cross reference. The $tag is a unique object ID consisting of letters, digits,
# and/or underscores.
sub target {
  my($self,$tag) = @_;
  return "";
}

# Generate a cross reference to some target.
sub xref {
  my($self,$tag,$text) = @_;
  return $text;
}

# Generate a cross reference to some target.
sub xref2 {
  my($self,$tag,$text) = @_;
  return $self->xref($tag,$text);
}

# Generate a cross reference to some source file.
sub xref3 {
  my($self,$name,$text) = @_;
  return $text || $self->file($name);
}

# Remember a cross reference for later.  When called like $self->xref_list($name) it returns any cross
# reference associated with $name.
sub xref_list {
  my($self,$name,$xref) = @_;
  $self->{xref}{$name} = $xref if @_>=3;
  return $self->{xref}{$name};
}

# Quote all typesetting special characters
sub escape {
  my($self,$tokref,$string) = @_;
  return $string;
}

# Typeset the string as code unless it is some word which is known to be an exception.
sub code {
  my($self,$string) = @_;
  return $string;
}

# Typeset a word as a variable (formal argument)
sub var {
  my($self,$name) = @_;
  return $name;
}

# Typeset a string as italic.
sub italic {
  my($self,$string) = @_;
  return $string;
}

# Typeset with emphasis
sub emph {
  my($self,$string) = @_;
  return $string;
}

# Typeset as a file
sub file {
  my($self,$name) = @_;
  return "`$name'";
}

# Typeset an image of some sort
sub figure {
  my($self,$name) = @_;
  return "figure `$name'";
}

# Switch foreground text color as specified
sub color {
  my($self,$color,$string) = @_;
  return $string;
}

# Typeset $name as a functioncall. If $see_also is a non-nil hash reference then add $name to that
# hash as a key.
sub funcall {
  my($self,$name,$current,$see_also,$exrefs) = @_;
  my($f) = Mkdoc::Function->exists($name);

  # Documented functions. We make a cross reference to the function name (not tag) because if
  # there are multiple functions with the same name then the name will refer to the overload
  # object. Otherwise the name should be the same as the tag.
  if ($f) {
    $see_also->{$name}++ if $see_also;
    return $self->code($name) if $current && $name eq $current->name; # no xref to current function
    return $self->xref2($name, $self->code($name));
  }

  # External cross references.
  if (exists $exrefs->{$name}) {
    return $exrefs->{$name};
  }

  return $self->code($name);
}

# Typeset a universal resource locator (URL). This assumes we're in a
# <String> context.  protocol.
sub url {
  my($self,$url,$text) = @_;
  return sprintf "%s (%s)", $text, $self->italic($url) if $text;
  return $self->italic($url);
}

# Some formats support multi-part output.  The default is to not support it.
sub multipart {0}

# Private function that when given a token array and a value, assigns that value to a token ID and
# return the token ID. Token IDs look like `@<#>'. If $tokref is undefined then just return the
# value.
sub _toksave {
  my($tokref,$value) = @_;
  return $value unless $tokref;
  local($_) = "\@<" . scalar(@$tokref) . ">";
  push @$tokref, $value;
  return $_;
}

# Private function that when given a token array and a string, expands all tokens in that string
sub _tokexp {
  my($tokref,$s) = @_;
  1 while $s =~ s/@<(\d+)>/$tokref->[$1]/eg;
  return $s;
}

# Helper functions for typesetting context-sensitive things.
sub do_xref {
  my($self,$tokref,$name) = @_;
  my($xref) = $self->xref_list($name);
  return $name unless $xref;
  my($retval) = _toksave $tokref, $xref;
  return $retval;
}

sub do_emph {
  my($self,$tokref,$s) = @_;
  if ($s =~ /^\*(.*)\*$/) {
    $s = $1;
    $s =~ s/\*/ /g;
  }
  return _toksave $tokref, $self->emph($s);
}

sub do_italic {
  my($self,$tokref,$s) = @_;
  if ($s =~ /^\/(.*)\/$/) {
    $s = $1;
    $s =~ s/\// /g;
  }
  return _toksave $tokref, $self->italic($s);
}

sub do_note {
  my($self,$tokref,$current,$name) = @_;
  return $name if $name =~ /^@<\d+>$/; # something already tokenized
  $name =~ s/\s+/ /gs;
  my(@notes,$note) = Mkdoc::Note->list(name=>$name); # first try notes
  @notes = Mkdoc::Overload->list(name=>$name) unless @notes; # something overloaded?
  @notes = Mkdoc::Docbase->list(name=>$name) unless @notes; # anything?

  if (0==@notes) {
    print STDERR "warning: note `$name' does not exist";
    print STDERR " (referenced from ", $current->class, " `", $current->name, "')";
    print STDERR "\n";
    return $name;
  } elsif (1==@notes) {
    $note = $notes[0];
  } else {
    $note = Mkdoc::Overload->list(name=>$name); # multiple notes with same name
    die "internal problems with note \"$name\"" unless $note;
  }
  return _toksave $tokref, $self->xref2($note->objid,$self->italic($name));
}

# Typeset something that we already know is code. That is, string $s is in a code context.  These transformations
# are a subset of the prose transformations from Mkdoc::Format::context().
sub example {
  my($self,$current,$s,$see_also,$exrefs,$lnums) = @_;
  my(@tokens);

  $s = $self->escape(\@tokens,$s);                                      # escape special chars
  $s =~ s/([a-z_A-Z]\w*)(?=\()/
    _toksave \@tokens, $self->funcall($1,$current,$see_also,$exrefs)/eg;# function call
  $s =~ s/\b([a-z_A-Z]\w*)\b/
    $self->do_xref(\@tokens,$1)/eg;                                     # cross reference
  $s =~ s/\/\/(\s*)(.*)/
    _toksave \@tokens, $self->color($CommentColor,"\/*$1$2$1*\/")/eg;   # C++ comments
  $s =~ s/(\/\*(.|\n)*?\*\/)/
    _toksave \@tokens, $self->color($CommentColor,$1)/eg;               # C comments

  # Add the special things back in
  $s = _tokexp \@tokens, $s;

  # Number the lines using the minimum field width for the numbers.
  my $numwidth = length sprintf "%d", $lnums + ($s =~ tr/\n/\n/);
  if (defined $lnums) {
    $s =~ s/^/$self->color($LineNumColor,sprintf "%*d ", $numwidth, $lnums++)/meg;
  }

  return $s;
}

# Look for paragraphs that follow some sort of list or table format and process them, returning a new list of paragraphs.
# All list- and table-related paragraphs begin with a `*'.  The first argument determines whether we should process list
# (a.k.a., bullet) paragraphs or table paragraphs.
sub paralist {
  my($self,$style,@para) = @_;
  my(@items,@retval);

  while (@para) {
    my $p = shift @para;

    # Split the paragraph into separate paragraphs if each line looks like a list or table item. That way we'll consider
    # each line separately below.
    if ($style eq 'bullet' &&
        $p =~ /^(  [ \t]*\*[ \t]+\S[^\n]*)
                (\n[ \t]*\*[ \t]+\S[^\n]*)*$/sx) {
      unshift @para, split /\n/, $p;
      $p = shift @para;
    } elsif ($style eq 'table' &&
             $p =~ /^(  [ \t]*\*[ \t]+\S[^\n]{0,30}:[ \t]+\S[^\n]*)
                     (\n[ \t]*\*[ \t]+\S[^\n]{0,30}:[ \t]+\S[^\n]*)*$/sx) {
      unshift @para, split /\n/, $p;
      $p = shift @para;
    }

    # Now look for a list item paragraph or a table item paragraph.
    if ($style eq 'bullet' &&     $p =~ /^[ \t]*\*[ \t]+(\S.*)/s) {
      push @items, $1, "";
      $p = undef;
    } elsif ($style eq 'table' && $p =~ /^[ \t]*\*[ \t]+(\S.*?):[ \t]+(\S.*)/s) {
      push @items, $1, $2;
      $p = undef;
    }

    # If we still have a paragraph by now, push the table (if any) followed by the paragraph onto the return list.
    if ($p =~ /\S/) {
      push @retval, $self->table($style,@items) if @items;
      @items = ();
      push @retval, $p;
    }
  }

  # Push the table onto the return value if we still have items outstanding
  push @retval, $self->table($style,@items) if @items;
  return @retval;
}

# Join paragraphs together.
sub para_join {
  my($self,@para) = @_;
  return join "\n\n", @para;
}

# Context-based typesetting for things like descriptions. Given a string add typesetting commands
# and return the new string.  Assume the string is already in paragraph context. $vars is an
# optional hash reference whose keys are variable names. $see_also is an optional hash reference
# which will have keys added for each documented object which is referenced.
sub context {
  my($self,$current,$s,$vars,$see_also,$exrefs) = @_;
  my(@tokens);
  local($_);

  my %abbr;
  map {$abbr{$_}=1} qw(A API ASCI C CAD CAM CSG CVS DSL HDF HDF5 HTML I II I/O IBM-SP2 LLNL LPS MPI PATRAN POSIX SAF SEACAS SGI SNL VBT);

  sub formal_name {#return formal name or undef
    my($vars,$word) = @_;
    local($_);
    return undef unless defined $vars;
    foreach (keys %$vars) {
      return $_ if lc $word eq lc $_;
    }
    return undef;
  }

  # Split into non-empty paragraphs
  my(@para) = grep /\S/, split /\n\n+/, $s;

  # Process each paragraph individually
  my(@newpara);
  for my $s (@para) {
    # typeset special things, removing them from the $s and replacing them by a token. There is a
    # little order dependency here -- things that should be recognized first should appear first.
    if ($s =~ /^\s/s) {
      # THIS IS A SOURCE CODE EXAMPLE (a subset of the prose transformations, but same order)
      my $linenumbers = 1 if $s =~ tr/\n/\n/ > 4;
      $s = _toksave \@tokens, $self->example($current,$s,$see_also,$exrefs,$linenumbers);
    } else {
      # THIS IS PROSE
      $s = $self->escape(\@tokens,$s);                                      # Special characters
      $s =~ s/\b([A-Z][\w\/]*)\b/
        ($abbr{$1} && _toksave \@tokens, $1) || $1/eg;                      # Non-code abbreviations
      $s =~ s/\[figure\s+(.*?)\]/
        _toksave \@tokens, $self->figure($1)/eg;                            # for pictures [figure Something]
      $s =~ s/\bFILE:([^\s()]+)/
        _toksave \@tokens, $self->code($1)/eg;                              # A file name, so slashes don't mess us up.
      $s =~ s/\b((http|file|ftp|mailto):[^\s()]+)/
        _toksave \@tokens, $self->url($1)/eg;				    # URL
      $s =~ s/([a-z_A-Z][\w:]*)\(\)/
        _toksave \@tokens, $self->funcall($1,$current,$see_also,$exrefs)/eg;# function call
      $s =~ s/\b([A-Z_]+)\b/
        formal_name($vars,$1)?_toksave(\@tokens,$self->var(formal_name $vars,$1)):$1/eg;# formal
      $s =~ s/\b([a-z_A-Z]\w*)\b/
        $self->do_xref(\@tokens,$1)/eg;                                     # cross reference
      $s =~ s/((\*[-a-zA-Z_.,0-9]+)+\*)/					
        $self->do_emph(\@tokens,$1)/eg;                                     # *bold*faced*type*
      $s =~ s/((\/[-a-zA-Z_0-9.,]+)+\/)/
        $self->do_italic(\@tokens,$1)/eg;                                   # /italic/font/.
                                                                            # ss_/foo/obj_t should italicize foo within code.
      $s =~ s/\/\/(\s*)(.*)/
        _toksave \@tokens, $self->color($CommentColor,"\/*$1$2$1*\/")/eg;   # C++ comments
      $s =~ s/\b([A-Z][A-Z_0-9]*)\b/
        _toksave \@tokens, $self->code($1)/eg;                              # All uppercase is possible code
      $s =~ s/\b([A-Z_]+[a-z]+[A-Z]+\w*)\b/
        _toksave \@tokens, $self->code($1)/eg;                              # Funny caps is possible code
      $s =~ s/\b([a-zA-Z0-9]*_\w*)\b/
        _toksave \@tokens, $self->code($1)/eg;                              # Anything with underscores is code
      $s =~ s/\b([a-zA-Z0-9]+:\w+)\b/
        _toksave \@tokens, $self->code($1)/eg;                              # Anything with embedded colons is code
      $s =~ s/\b([a-zA-Z]+[0-9]+[a-zA-Z0-9]*)\b/
        _toksave \@tokens, $self->code($1)/eg;                              # Anything with embedded digits is code
      $s =~ s/!(\S*[^\s:])/
        " "._toksave \@tokens, $self->code($1)/eg;                          # ! followed by non-space is code w/o the `!'.
                                                                            # !/foo/ should be italicized code.
                                                                            # Trailing colon is not part of code
      $s =~ s/\(([Ss]ee\s*)\[?(.*?)\]?\)/
        "($1".$self->do_note(\@tokens,$current,$2).")"/egs;                 # xref for `(see The Note)'
      $s =~ s/([Ss]ee\s*)\[(.*?)\]/
        $1.$self->do_note(\@tokens,$current,$2)/egs;                        # xref for `see [The Note]'
    }
    push @newpara, $s;
  }
  @para = @newpara;
  @newpara = ();

  # Any paragraphs that start with `*' followed by something ending with a `:' should be typeset as a two-column table.
  # Furthermore, if every line of the paragraphs follows this pattern then treat each line as if it were a separate paragraph.
  @para = $self->paralist('table',@para);

  # Any paragraphs that start with `*' should be a bulleted list. Furthermore, if every line of a paragraph follows this
  # pattern then treat each line as if it were a separate paragraph.
  @para = $self->paralist('bullet',@para);

  # Join paragraphs together
  $s = $self->para_join(@para);

  # Add the special things back in
  return _tokexp \@tokens, $s;
}

# Wrap items in a table
sub table {
  my($self,$format,@items) = @_;
  my(@fixed);

  while (@items) {
    my($head,$body) = (shift @items, shift @items);
    $body =~ s/^/    /mg;
    if ($format eq 'bullet') {
      push @fixed, "* $head";
    } elsif ($format eq 'b') {
      push @fixed, $self->emph($head) . "\n" . "-" x length($head) . "\n$body";
    } elsif ($format eq 'var') {
      push @fixed, "* " . $self->var($head) . "\n$body";
    } elsif ($format eq 'code') {
      push @fixed, "* " . $self->code($head) . "\n$body";
    } elsif ($format eq 'xref') {
      push @fixed, "* $head\n$body";
    } elsif ($format eq 'table') {
      push @fixed, "* $head\n$body";
    } else {
      croak "unknown table format: $format";
    }
  }

  return join "\n\n", @fixed;
}

# Send $string to an auxiliary file, or just return it if we're not using multi-file mode. If $basename
# is specified then it's used to name the auxiliary file, otherwise we use the object ID.
sub multi_output {
  my($self,$object,$string,$basename) = @_;
  return $string;
}

# Create a chapter item
sub chapter_item {
  my($self,$text) = @_;
  return $text;
}

# Wrap chapter items in a chapter
sub chapter {
  my($self,$chapter,$title,@items) = @_;
  $title = $chapter->name unless defined $title;
  $title = join ".  ", grep {$_ ne ""} $chapter->secnum, $title;
  local($_);

  $_ .= "=" x length($title) . "\n" . $title . "\n" . "=" x length($title) . "\n\n";
  $_ .= join "\n\n", @items;
  return $_;
}

# A table of contents item.  The following fields are usually present in a table of contents entry:
#    Section Number     -- obtained by $object->secnum() method
#    Title              -- obtained by $object->name(), typeset according to the object class
#    Class              -- obtained by $object->class()
#    Description        -- optional user-supplied string; default is empty string
#    Xref               -- optional cross reference; default is the standard object reference
sub toc_item {
  my($self,$object,$description,$xref) = @_;
  my($title) = $self->escape(undef,$object->name);
  my($class) = $object->class;
  if ($class =~ /^(chapter|note|section)$/) {
    $title = $self->italic($title);
  } elsif ($class =~ /^(function|macro|symbol|datatype)$/) {
    $title = $self->code($title);
  }
  $xref ||= $self->xref($object->objid,$title);
  $self->do_toc_item($object,$title,$class,$description,$xref);
}
sub do_toc_item {
  my($self,$object,$title,$class,$desc,$xref) = @_; # all required (xref not used)
  my($level) = scalar split /\D/, $object->secnum;
  local($_) = " " x (4*$level);
  $_ .= join " ", grep {$_ ne ""} $object->secnum, $title, "[$class]";
  $_ .= " " if $desc =~ /^\S/;
  $_ .= $desc;
  return $_;
}

# Wrap some table of contents
sub toc {
  my($self,$title,@entries) = @_;
  $title = $self->paragraph($self->emph($title)) . "\n" if $title;
  return $title . join "\n", @entries;
}

# Return the output prologue
sub prologue {
  my($self,$mkdoc_url) = @_;
  local($_);

  sub center {
    local($_) = @_;
    return " " x (50 - length($_)/2) . $_;
  }

  my($title);
  if ('ARRAY' eq ref $self->{title}) {
    foreach (@{$self->{title}}) {
      $_ = join " ", split //, $_;
      $title .= center($_) . "\n" . center("=" x length) . "\n";
    }
    $title .= "\n\n";
  }

  my($author);
  if ('ARRAY' eq ref $self->{author}) {
    $author = join "\n", map {center $_} @{$self->{author}};
    $author .= "\n" x 5;
  }

  my($oldauthor);
  if ('ARRAY' eq ref $self->{oldauthor}) {
    $oldauthor = join "\n", map {center $_} @{$self->{oldauthor}};
    $oldauthor .= "\n" x 5;
  }

  my($acknow);
  if ('ARRAY' eq ref $self->{acknow}) {
    $acknow = join "\n", map {center $_} @{$self->{acknow}};
    $acknow .= "\n" x 5;
  }

  my($copyright);
  if ('ARRAY' eq ref $self->{copyright}) {
    foreach (@{$self->{copyright}}) {
      my($year,$name,$rights) = split /\000/;
      $copyright .= "Copyright (C) $year. $name.\n";
      $rights ||= "All rights reserved.";
      $rights =~ s/^[ \t]*/' ' x 20/egm;
      $copyright .= $rights . "\n\n";
    }
  }

  my($disclaimer);
  if ('ARRAY' eq ref $self->{disclaimer}) {
    $disclaimer = center "DISCLAIMER\n\n";
    foreach (@{$self->{disclaimer}}) {
      $_ = $self->context(undef,$_);
      s/^[ \t]*/' ' x 20/egm;
      $disclaimer .= $_ . "\n\n\n";
    }
  }

  my($advertisement) = "This document was generated from source code by " .
    $self->url($mkdoc_url,$self->code("mkdoc")) . ".\nDO NOT EDIT THIS FILE!\n";

  return $title . $author . $oldauthor . $acknow . $copyright . $disclaimer . $advertisement . "\f";
}

# Return output epilogue
sub epilogue {
  my($self) = @_;
  my($pkg) = ref $self;
  my($ext) = $pkg =~ /(\w+)$/; #last word
  my($date) = `date`; chomp $date;
  $ext = lc $ext;

  my($found) = _find_support_file("end.$ext") or croak "cannot find file `end.$ext'";
  open EPILOGUE, $found or croak "cannot open $found: $!";
  local($_) = join "", <EPILOGUE>;
  close EPILOGUE;
  s/\@DATE@/$date/g;
  return $_;
}

# Send output to the main file
sub output {
  my($self,@chapters) = @_;
  my($file) = $self->name;

  if ($file ne "") {
    open OUT, ">$file" or croak "cannot open $file: $!";
    print OUT $self->prologue($MkdocURL), join("\n\n",@chapters), $self->epilogue;
    close OUT;
  } else {
    print $self->prologue($MkdocURL), join("\n\n",@chapters), $self->epilogue;
  }
}

# Get/set name of main file
sub name {
  my($self,$name) = @_;
  $self->{name} = $name if @_ >= 2;
  return $self->{name};
}

# Get/set output directory
sub outdir {
  my($self,$dir) = @_;
  $self->{outdir} = $dir if @_ >= 2;
  return $self->{outdir};
}

# Provide a paragraph context
sub paragraph {
  my($self,@items) = @_;
  return join "\n\n", @items;
}

# Function prototype
sub proto {
  my($self,$function) = @_;
  my(@formals) = $function->formals;
  local($_);

  $_ .= $function->rettype . "\n" . $function->name;
  $_ .= "(" unless $function->class eq 'symbol';

  my($indent,$i) = (1+length $function->name);
  for ($i=0; $i<@formals; $i++) {
    my($formal) = $formals[$i];
    $_ .= ",\n" . " " x $indent if $i;
    if (ref $formal) {
      $_ .= $formal->{type} . $formal->{name} . $formal->{array};
    } else {
      $_ .= $formal; #wasn't parsed
    }
  }
  $_ .= "void" unless @formals || $function->class eq 'symbol';
  $_ .= ")" unless $function->class eq 'symbol';
  return $_;
}

# Output an enumerated type
sub enum {
  my($self,$enum,$exrefs) = @_;
  my($member,@members);
  local($_);

  foreach $member (@{$enum->{definitions}}) {
    push @members, $member->{name}, $self->context($enum,$member->{comment},undef,undef,$exrefs);
  }
  $_ .= $self->table('code', @members);
  return $_;
}

# Create an entry for a permuted index
sub pindex_item {
  my($self,$object,$pre,$post) = @_;
  return "";
}

# Wrap a permuted index around some entries
sub pindex {
  my($self,@entries) = @_;
  return join "\n", @entries;
}

# Create an entry for a concept index
sub cindex_item {
  my($self,$object,$text) = @_;
  return "";
}

# Wrap a concept index around some entries
sub cindex {
  my($self,@entries) = @_;
  return join "\n", @entries;
}

# Find some support file by looking in various directories.
sub _find_support_file {
  my($basename) = @_;

  my($dir) = $0 =~ /^(.*)\/[^\/]*\/*$/;
  $dir ||= ".";
  my(@search) = ("$dir/Formats", $dir, @INC);

  foreach $dir (@search) {
    return "$dir/$basename" if -f "$dir/$basename";
  }
  return;
}

1;
