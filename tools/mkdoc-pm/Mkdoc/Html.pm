# Author: Robb Matzke <matzke@llnl.gov>
#
# Purpose:
#     Mkdoc::Html is a specialization of Mkdoc::Format for dealing with the generation of HTML output.
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


package Mkdoc::Html;
@ISA = qw/Mkdoc::Format/;

use strict;
use Carp;
use Mkdoc::Format;

my($FigureDir) = ".";		# directory to find figures in

# Make a new html output context where $name is the name of the primary file.
sub new {
  my($pkg,$name,%init) = @_;
  $name .= ".html" unless $name =~ /\.html?$/;
  my($self) = bless {name=>$name}, $pkg;
  return $self->init(%init);
}

# HTML prologue
sub prologue {
  my($self,$mkdoc_url) = @_;
  local($_);

  my($bodytitle,$headtitle);
  if ('ARRAY' eq ref $self->{title}) {
    $bodytitle = join "<br>\n", @{$self->{title}};
    $headtitle = $self->{title}[0];
  }
  $headtitle ||= "Reference Manual";
  $bodytitle ||= $headtitle;

  $_ .= "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML//EN\">\n";
  $_ .= "<html>\n";
  $_ .= "  <head>\n";
  $_ .= "    <title>$headtitle</title>\n";
  $_ .= "  </head>\n";
  $_ .= "  <body " . $self->nav_image('document','bkg') . ">\n";
  $_ .= "    <center><h1>$bodytitle</h1></center>\n";

  $_ .= "<center><b>Active Developers</b><br>\n";
  $_ .= "<font face=\"Arial,Helvetica\">\n";
  my($author);
  foreach $author (@{$self->{author}}) {
    $_ .= "$author, ";
  }
  $_ .= "</font></center><br>\n";

  $_ .= "<center><b>Inactive Developers</b><br>\n";
  $_ .= "<font face=\"Arial,Helvetica\">\n";
  my($oldauthor);
  foreach $oldauthor (@{$self->{oldauthor}}) {
    $_ .= "$oldauthor, ";
  }
  $_ .= "</font></center><br>\n";

  $_ .= "<center><b>Acknowledgements</b></center>\n";
  my($acknow);
  foreach $acknow (@{$self->{acknow}}) {
    $_ .= "<center><font size=-1 face=\"Arial,Helvetica\">$acknow</font></center>\n";
  }

  my($copyright);
  foreach $copyright (@{$self->{copyright}}) {
    my($year,$owner,$rights) = split /\000/, $copyright;
    $rights ||= "All rights reserved.";

    $_ .= "<p><b>Copyright &copy; $year. $owner.</b><br>\n";
    $_ .= $rights . "\n\n";
  }

  if ($self->{disclaimer}) {
    $_ .= "<center><b>Disclaimer</b></center>\n\n";
    my($disclaimer);
    foreach $disclaimer (@{$self->{disclaimer}}) {
      $_ .= "<p>" . $self->context(undef,$disclaimer);
    }
  }

  $_ .= "<p>This document was generated on by ";
  $_ .= $self->url($mkdoc_url,$self->code("mkdoc"));
  $_ .= sprintf(" on %04d-%02d-%02d.\n",
                1900+(localtime)[5], 1+(localtime)[4], (localtime)[3]);
  $_ .= "<hr>\n";
  $_ .= "<a name=\"TOP\"></a>\n"; # for Mark Miller's slow-load workaround
  return $_;
}

# Special symbols
sub sym_section {"&sect;"}

# Get/set multipart flag.
sub multipart {
  my($self,$set) = @_;
  $self->{multipart} = $set if @_>1;
  return $self->{multipart};
}

# Generate a target for a cross reference.  The $tag is the name of the target and is an
# object ID that consists of only letters, digits, and underscores.
sub target {
  my($self,$tag) = @_;
  return "<a name=\"$tag\"></a>";
}

# Generate a cross reference to some target.  Arguments are similar to `target' method.
sub xref {
  my($self,$tag,$text) = @_;
  my($retval);
  if ($self->multipart) {
    $retval = "<a href=\"$tag.html\">$text</a>";
  } else {
    $retval = "<a href=\"#$tag\">$text</a>";
  }
  return $retval;
}

# Generate a cross reference to some source file.
sub xref3 {
  my($self,$name,$title) = @_;
  $title ||= $self->file($name);
  return "<a href=\"$name\">$title</a>";
}

# Quote all typesetting special characters so they appear in the documentation.
sub escape {
  my($self,$tokref,$string) = @_;
  $string =~ s/([<>])/Mkdoc::Format::_toksave($tokref,$1 eq '<'?'&lt;':'&gt;')/eg;
  return $string;
}

# Typeset the string as code
sub code {
  my($self,$string) = @_;
  return "<code>$string</code>";
}

# Typeset a word as a variable (formal argument)
sub var {
  my($self,$name) = @_;
  return "<i>$name</i>";
}

# Typeset a string as italic.
sub italic {
  my($self,$string) = @_;
  return "<i>$string</i>";
}

# Typeset with emphasis
sub emph {
  my($self,$string) = @_;
  return "<b>$string</b>";
}

# Typeset as a file name
sub file {
  my($self,$name) = @_;
  return "<i>$name</i>";
}

# Typeset a picture
sub figure {
  my($self,$name) = @_;

  $name =~ s/\s/_/g;

  my($n2home) = ($self->{figdir} || ".") . "/" . $name;
  my($n2here) = $self->outdir            . "/" . $name;

  # copy the image if it isn't already here 
  if (-f $n2home && -r _) {
    unless (-f $n2here) {
      system "cp", $n2home, $n2here;
      chmod 0644, $n2here;
    }
  }

  return "<p><center><table><tr><td><center><image src=\"$name\"></center></td></tr><tr><td><center>$name</center></td></tr></table></center></p>";
}

# Typeset a universal resource locator (URL). By default, $text will be the URL sans the leading
# protocol.
sub url {
  my($self,$url,$text) = @_;
  unless (defined $text) {
    $text = $url;
    $text =~ s/^((http|file|ftp|mailto):(\/\/)?)//;
  }
  return "<a href=\"$url\">$text</a>";
}

# Typeset an example. The code has to be on one line so that when this return value is indented by various other functions
# it doesn't mess up the indentation inside the code.  What happened before is that lines were emitted like this:
#
#    |<code><pre>    line 1
#    |    line 2</pre></code>
#
# which was later indented, perhaps inside a table body, to be
#
#    |  <code><pre>    line 1
#    |      line 2</pre></code>
#
# and would be rendered in a browser with `line 1' preceded still by four spaces but `line 2' by six spaces.
#
# Secondly, the entire sequence is contained in a 1x1 table in order to make left/right movements much easier -- you just move
# the table and don't worry about the individual lines of code.  Therefore, white space common to every line can be removed for
# more uniform output.
sub example {
  my($self,$curfunc,$s,$see_also,$exrefs,$lnums) = @_;

  # Remove whitespace that's common to every line.  Treat blank lines as an infinite amount of white space so they don't
  # interfere with the other lines' white space.  Tabs should already be expanded by now.
  my $prefix;
  for my $line (split /\n/, $s) {
    if ($line =~ /\S/) {
      my($ws) = $line =~ /^(\s*)/;
      $prefix = $ws if !defined($prefix) || length($ws)<length($prefix);
    }
  }
  $s =~ s/^$prefix//gm if length $prefix;

  # Typeset the example.
  $s = $self->SUPER::example($curfunc,$s,$see_also,$exrefs,$lnums);

  # Build the output. All code on one line and in a 1x1 table per above.
  $s =~ s/\n/<br>/g;
  my $bkg = $self->nav_image("source","bkg");
  $s = "<table border=0 width=\"90%\">\n  <tr><td $bkg><code><pre>$s\n</pre></code></td></tr>\n</table>\n";
  return $s;
}

# Switch foreground text color as specified ($color is optional)
sub color {
  my($self,$color,$string) = @_;
  return $string unless $color;
  return "<font color=\"$color\">$string</font>";
}

# Join paragraphs together. Each paragraph except the first begins with `<p>' (we assume the caller is already in
# paragraph context, so the first paragraph will not start with `<p>').
#
# This function used to also surround any paragraph that starts with white space with "<pre><code>" in order to typeset
# it as code, but that functionality has been superseded with Mkdoc::Format::context(). --rpm 2003-08-26
sub para_join {
  my($self,@para) = @_;
  return join "</p>\n\n<p>",@para;
}

# Make a table
sub table {
  my($self,$format,@items) = @_;
  my(@fixed,@heads,@bodies);

  # Split the items into a list of heads and bodies.
  while (@items) {
    push @heads, shift @items;
    push @bodies, shift @items;
  }

  # Do various things to the heads
  if ($format eq 'b') {
    @heads = map {$self->emph($_)} @heads;
  } elsif ($format eq 'var') {
    @heads = map {$self->var($_)} @heads;
  } elsif ($format eq 'code') {
    @heads = map {$self->code($_)} @heads;
  }

  # If the bodies are all empty then the list looks better with less vertical space
  my $vsp = "<br>" if grep /\S/, @bodies;

  # The innards of the table
  while (@heads) {
    my($head,$body) = (shift @heads, shift @bodies);
    $body =~ s/\n/\n    /g; # just to make the HTML look pretty
    my($vspace) = @fixed ? $vsp:"";
    if ($format eq 'bullet') {
      push @fixed, "  <li>$head</li>";
    } elsif ($format eq 'b') {
      push @fixed, "  <dt>$vspace$head</dt>\n  <dd>$body</dd>";
    } elsif ($format eq 'var') {
      push @fixed, "  <dt>$vspace$head</dt>\n  <dd>$body</dd>";
    } elsif ($format eq 'code') {
      push @fixed, "  <dt>$vspace$head</dt>\n  <dd>$body</dd>";
    } elsif ($format eq 'xref') {
      push @fixed, "  <dt>$vspace$head</dt>\n  <dd>$body</dd>";
    } elsif ($format eq 'table') {
      push @fixed, "  <tr valign=top><td width=20>&nbsp;</td><td>$head</td><td>&nbsp</td>\n  <td>$body</td></tr>\n";
    } else {
      croak "unknown table format: $format";
    }
  }

  # The table begin and end
  if ($format eq 'bullet') {
    unshift @fixed, "<ul>";
    push    @fixed, "</ul>";
  } elsif ($format eq 'table') {
    unshift @fixed, "<table>";
    push    @fixed, "</table>";
  } else {
    unshift @fixed, "<dl>";
    push    @fixed, "</dl>";
  }
  return join "\n", @fixed;
}

# Determines what navigation image is needed, copies it to the current directory, and returns
# the name of the image.
sub nav_image {
  my($self,$class,$what,$text) = @_;

  my($n1base) =                $what . ".jpg";
  my($n2base) = $class . "_" . $what . ".jpg";

  my($n1home) = ($self->{navdir} || ".") . "/" . $n1base;
  my($n2home) = ($self->{navdir} || ".") . "/" . $n2base;
  my($n1here) = $self->outdir            . "/" . $n1base;
  my($n2here) = $self->outdir            . "/" . $n2base;

  # Alternate text when no image is available
  unless ($text) {
    $text = "\u$what";
    $text = "Previous" if $what eq 'prev';
    $text = ""         if $what eq 'bar';
  }
  my($attr) = " alt=\"$text\"";

  # Class-specific image
  if (-f $n2home && -r _) {
    unless (-f $n2here) {
      system "cp", $n2home, $n2here;
      chmod 0644, $n2here;
    }
    return "background=\"$n2base\"" if $what eq 'bkg';
    return "<img src=\"$n2base\"$attr>";
  }

  # Generic image
  if (-f $n1home && -r _) {
    unless (-f $n1here) {
      system "cp", $n1home, $n1here;
      chmod 0644, $n1here;
    }
    return "background=\"$n1base\"" if $what eq 'bkg';
    return "<img src=\"$n1base\"$attr>";
  }

  # No image -- use only text
  return "bgcolor=\"white\"" if $what eq 'bkg';
  return "<hr>" if $what eq 'bar';
  return $text;
}

# Output navigation info. The $loc specifies where the navigation bar
# will be located (`top' or `bottom').
sub navigation {
  my($self,$location,$obj_or_cls,$prev,$up,$next) = @_;
  local($_);

  # Who are my neighbors?
  $prev ||= $obj_or_cls->prev if ref $obj_or_cls;
  $up   ||= $obj_or_cls->up   if ref $obj_or_cls;
  $next ||= $obj_or_cls->next if ref $obj_or_cls;
  return "" unless $prev || $up || $next;

  # What am I?
  my($class) = (ref $obj_or_cls) ? $obj_or_cls->class : $obj_or_cls;

  # Where are the images?
  my($iprev)  = $self->nav_image($class,'prev',$prev?$prev->name:undef);
  my($inext)  = $self->nav_image($class,'next',$next?$next->name:undef);
  my($iup)    = $self->nav_image($class,'up',  $up  ?$up->name  :undef);
  my($ibar)   = $self->nav_image($class,'bar');

  # The navigation bar...
  $_ .= "<p>$ibar\n" if $location eq 'bottom';
  $_ .= "<table border=0><tr valign=center>\n";
  $_ .= "<td width=\"33%\" align=left>";
  $_ .= $self->xref($prev->objid,$iprev) if $prev;
  $_ .= "</td>\n<td width=\"33%\" align=center>";
  $_ .= $self->xref($up->objid,$iup) if $up;
  $_ .= "</td>\n<td align=right>";
  $_ .= $self->xref($next->objid,$inext) if $next;
  $_ .= "</td>\n</tr></table>\n";
  $_ .= "$ibar" if $location eq 'top';
  return $_;
}

# Send $string to an auxiliary file.
sub multi_output {
  my($self,$object,$string,$basename) = @_;
  return $string unless $self->multipart;

  my($file) = $self->outdir . "/" . ($basename || $object->objid) . ".html";
  my($title) = $object->name;
  my($class,$next,$prev,$up);

  # Navigation buttons.
  if ($basename =~ /^SC_/) {
    $class = "source";
    $next = $prev = undef;
    $up = $object;
  } else {
    $class = $object->class;
    $next = $object->next;
    $prev = $object->prev;
    $up   = $object->up;
  }

  open OUTPUT, ">$file" or croak "cannot open file: $file";
  print OUTPUT "<html><head><title>$title</title></head>";
  print OUTPUT "<body " . $self->nav_image($class,'bkg') . ">\n";
  print OUTPUT $self->navigation('top',$class,$prev,$up,$next);
  print OUTPUT $string;
  print OUTPUT $self->navigation('bottom',$class,$prev,$up,$next);
  print OUTPUT "</body></html>\n";
  close OUTPUT;
  return "";
}

# Wrap items into a chapter
sub chapter {
  my($self,$chapter,$title,@items) = @_;
  $title = $chapter->name unless defined $title;
  $title = join ".  ", grep {$_ ne ""} $chapter->secnum, $title;
  my($level) = (scalar split /\D/, $chapter->secnum) || 1;
  local($_);

  $_ .= "<!-- " . "#" x 90 . " -->\n";
  $_ .= "<!-- " . "#" x 90 . " -->\n";
  $_ .= "<hr>\n" unless $self->multipart;
  $_ .= $self->target($chapter->objid) . "\n";
  $_ .= "<h$level>$title</h$level>\n\n";
  $_ .= join "\n\n", @items;
  return $_;
}

# Insert a table-of-contents entry.  We ignore the title (which is normally the same as what
# would be rendered by the $xref anyway).
sub do_toc_item {
  my($self,$object,$title,$class,$desc,$xref) = @_; #all required
  my($bfont,$efont) = ("<font size=+1>","</font>") unless $object->secnum =~ /\./;
  local($_) = join ".&nbsp;&nbsp;", grep /\S/, $object->secnum, $xref;

  my($audience) = $object->prologue->{Audience} if grep {$class eq $_} qw/function macro symbol datatype/;
  $audience .= " " if length $audience;
  $_ .= " <font size=-1>[\L$audience$class\E]</font>";
  $_ .= " " if $desc =~ /^\S/;
  $_ .= $desc;
  return "<li>$_</li>";
}

# Wrap some table of contents items in a table of contents.
sub toc {
  my($self,$title,@entries) = @_;
  local($_);

  $_ .= $self->emph($title) . "\n" if $title;
  $_ .= "<ul>\n";
  $_ .= join "\n", map {my($s)=$_;$s=~s/^/  /mg;$s} @entries;
  $_ .= "\n</ul>";
  return $_;
}

# Function prototype
sub proto {
  my($self,$function) = @_;
  my(@formals,$i) = $function->formals;
  local($_);

  $_ .= "<code>" . $function->rettype . "<br>\n" . $function->name;
  $_ .= "(" unless $function->class eq 'symbol';
  my($indent) = length($function->name)+($function->class eq 'symbol' ? 0 : 1);

  for ($i=0; $i<@formals; $i++) {
    my($formal) = $formals[$i];
    $_ .= ",<br>\n" . '&nbsp;' x $indent if $i;
    if (ref $formal) {
      my($type) = $formal->{type};
      $type =~ s/([a-z_A-Z]\w*)/
	Mkdoc::Type->exists($1) || Mkdoc::Function->exists($1) ?
	  $self->xref($1,$1) : $1/eg;
      $_ .= $type . $self->var($formal->{name}) . $self->{array};
    } else {
      $_ .= $formal; # wasn't parsed
    }
  }
  unless ($function->class eq 'symbol') {
    $_ .= "void" unless @formals;
    $_ .= ")";
  }
  $_ .= "</code>";
  return $_;
}

# Create an entry for a permuted index
sub pindex_item {
  my($self,$object,$pre,$post) = @_;
  local($_);

  $_ .= "  <tr>\n";
  $_ .= "    <td align=right>$pre</td>\n";
  $_ .= "    <td></td>\n";
  $_ .= "    <td align=left>$post</td>\n";
  $_ .= "    <td align=right>\n";
  $_ .= "      " . $self->xref($object->objid,$self->code($object->name)) . "\n";
  $_ .= "    </td>\n";
  $_ .= "  </tr>";
  return $_;
}

# Wrap a permuted index around some entries
sub pindex {
  my($self,@entries) = @_;
  return "<center><table border=0><smaller>\n" . join("\n",@entries) . "\n</table></center>";
}

# Create an entry for a concept index
sub cindex_item {
  my($self,$object,$text) = @_;
  local($_);

  $_ .= "  <tr>\n";
  $_ .= "    <td>$text</td>\n";
  $_ .= "    <td align=right>\n";
  $_ .= "      " . $self->xref($object->objid,$self->code($object->name)) . "\n";
  $_ .= "    </td>\n";
  $_ .= "  </tr>\n";
  return $_;
}

# Wrap a concept index around some entries
sub cindex {
  my($self,@entries) = @_;
  return "<center><table border=0><smaller>\n" . join("\n",@entries) . "\n</center></table>";
}

1;
