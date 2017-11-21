# Author: Robb Matzke <matzke@llnl.gov>
#
# Purpose:
#     Base class for documentable objects.
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
package Mkdoc::Docbase;
use strict;
use Carp;
use Filename;
use Mkdoc::Overload;

my(@Object);			# list of all objects
my($ObjID);			# for generating unique object ID's

# If called with a package and a name then determine whether `name' exists in the specified
# package. Otherwise when called with just an object insert that object into the object list.
# If an object by the same name exists then add a level of indirection by replacing the object
# with an `Mkdoc::Overload' object and giving both new objects a unique ID.
sub exists {
  my($self,$name) = @_;
  local($_);

  # Register a new object
  if (ref $self) {

    croak "object method Mkdoc::Docbase::exists() not called correctly" if defined $name;
    my(@others) = Mkdoc::Docbase->list(name=>$self->name);
    if (1==@others && $others[0]->class eq 'document') {
      die sprintf "name collision `%s' for main document and %s\n", $self->name, $self->class;
    } elsif (1==@others) {
      # Retag both objects and make an Overload object as their parent
      $others[0]->objid(sprintf "%s_%d", $others[0]->objid, $ObjID++);
      $self->objid(sprintf "%s_%d", $self->name, $ObjID++);
      push @Object, $self;
      my($overload) = Mkdoc::Overload->new(name=>$self->name); # previous push prevents recursion
      $overload->add_members(@others, $self);
    } elsif (2==@others) {
      # We're inserting the overload object from the previous case.
      die "Recursion problem" unless ref $self eq 'Mkdoc::Overload';
      $self->objid($self->name);
      push @Object, $self;
    } elsif (@others) {
      # There must already be an overload object.
      my($overload) = grep {ref $_ eq 'Mkdoc::Overload'} @others;
      die "Mkdoc::Docbase::exists() internal problem: no overload object" unless $overload;
      $overload->add_members($self);
      $self->objid(sprintf "%s_%d", $self->name, $ObjID++);
      push @Object, $self;
    } else {
      # This is the first item with this name.
      die "Recursion problem" if ref $self eq 'Mkdoc::Overload';
      $self->objid($self->name);
      push @Object, $self;
    }
    return $self;
  }

  # Query object existence. This returns the first object that matches.
  croak "class method Mkdoc::Docbase::exists() not called correctly" unless defined $name;
  foreach (@Object) {
    return $_ if $name eq $_->{name} && ($self eq 'Mkdoc::Docbase' || ref $_ eq $self);
  }
  return;
}

# Return the type of object that this is: function, macro, datatype, note, document
sub class {
  my($self) = @_;
  local($_) = ref $self;

  /::Top/      && return "document";
  /::Chapter/  && return "chapter";
  /::Function/ && do {
    return "function" unless $self->is_macro;
    return "macro" if $self->source =~ /^\#\s*define\s+\w+\(/m; #has arglist
    return "symbol";
  };
  /::Type/     && return "datatype";
  /::Note/     && return "note";
  /::Section/  && return "section";
  /::Overload/ && return "overloaded";
  croak "unknown package: " . ref $self;
}

# Return a sorted list of all objects of the specified package
sub list {
  my($pkg,%args) = @_;
  my(@retval,$method);
  local($_);

 object:
  foreach (@Object) {
    next object if $pkg ne 'Mkdoc::Docbase' && ref $_ ne $pkg;
    foreach $method (keys %args) {
      next object if $_->$method() ne $args{$method};
    }
    push @retval, $_;
  }
  return sort {$a->name cmp $b->name} @retval;
}

# Set/query name
sub name {
  my($self,$name) = @_;
  $self->{name} = $name if defined $name;
  return $self->{name};
}

# Set/get unique object identification
sub objid {
  my($self,$objid) = @_;
  if (@_>=2) {
    $objid =~ s/\W/_/g;
    die "Object ID `$objid' is not unique. Died" if grep {$_->objid eq $objid} @Object;
    $self->{objid} = $objid;
  }
  return $self->{objid};
}
    
# Set/get section number
sub secnum {
  my($self,$secnum) = @_;
  $self->{secnum} = $secnum if @_ >= 2;
  return $self->{secnum};
}

# Given an array of objects, create `next' and `prev' pointers in each object.
sub mklinks {
  my($pkg,@obj) = @_;
  my($prev,$i);

  for ($i=0; $i<@obj; $i++) {
    my($cur) = $obj[$i];
    $cur->prev($prev);
    $cur->next($i+1<@obj?$obj[$i+1]:undef);
    $prev = $cur;
  }
  return @obj;
}

# Get/set the object next pointer
sub next {
  my($self,$next) = @_;
  $self->{next} = $next if @_ >= 2;
  return $self->{next};
}

# Get/set the object prev pointer
sub prev {
  my($self,$prev) = @_;
  $self->{prev} = $prev if @_ >= 2;
  return $self->{prev};
}

# Get/set chapter to which this object belongs.
sub up {
  my($self,$up) = @_;
  $self->{up} = $up if @_ >= 2;
  return $self->{up};
}

# Get/set source code for object
sub source {
  my($self,$src) = @_;
  $self->{source} = $src if @_ >= 2;
  return $self->{source};
}

# Add members to this object.
sub add_members {
  my($self,@members) = @_;
  if (@members) {
    push @{$self->{members}}, @members;
  }
  $self->{members} = [] unless $self->{members};
  return undef;
}

# Return sorted list of members
sub get_members {
  my($self,$sort) = @_;
  $self->{members} ||= [];
  return @{$self->{members}} unless $sort;
  return sort {$a->name cmp $b->name} @{$self->{members}};
}

# Get/set file to which this object belongs.
sub file {
  my($self,$file) = @_;
  $self->{file} = $file if @_ >= 2;
  return $self->{file};
}

# Get set prologue hashref
sub prologue {
  my($self,$prologue) = @_;
  $self->{prologue} = $prologue if @_ >= 2;
  $self->{prologue} = {} unless $self->{prologue};
  return $self->{prologue};
}

# Get/set preconditions
sub precond {
  my($self,@precond) = @_;
  push @{$self->{precond}}, @precond if @precond;
  $self->{precond} = [] unless $self->{precond};
  return @{$self->{precond}};
}

# Return a description of where the object is defined.  If the object has no definition
# then return nothing.
sub defined_in {
  my($self,$output) = @_;
  my($file) = fn_relative $self->file, $output->outdir if $self->file;
  my($defined) = "defined";
  local($_);

  $defined = "declared" if $self->class eq 'function' && $self->body eq "";

  if (grep {$self->class eq $_} qw(chapter note overloaded)) {
    # No source code
  } elsif ($output->multipart && $file) {
    $_ = " $defined in " . $output->xref("SC_".$self->objid, (fn_split $file)[1]);
  } elsif ($file) {
    $_ = " $defined in file " . $output->xref3($file);
  } elsif ($output->multipart) {
    $_ = " $defined " . $output->xref("SC_".$self->objid, "here");
  }
  return $_;
}

# Add standard headings to a table.
sub std_headings {
  my($self,$output,$table,$formals,$see_also,$exrefs,@omit) = @_;
  local($_);

  # Description
  unless (grep /^desc/i, @omit) {
    if (defined $self->prologue->{Description}) {
      push @$table, "Description", $output->context($self,$self->prologue->{Description},
						    $formals,$see_also,$exrefs);
    }
  }

  # Preconditions
  unless (grep /^precond/i, @omit) {
    my(@precond,@pc_table) = $self->precond;
    foreach (@precond) {
      my($cond,$cost,$text) = split /\000/;
      if ($text ne "") {
	$text =~ s/^\"\s*//; $text =~ s/\s*\"$//; #remove leading and trailing junk from text string
	$text .= '.' unless $text =~ /[.?!]$/; #append punctuation
	$text = $output->context($self,"\u$text",$formals,$see_also,$exrefs);
      } else {
	$text = $output->code($output->escape($cond));
      }
      if ($cost =~ /SAF_(.*)_CHK_COST/) {
	$cost = $1;
	$cost =~ tr/A-Z/a-z/;
	$cost .= "-cost";
      } else {
	$cost = $output->code($output->escape($cost));
      }
      $text .= "  ($cost)";
      push @pc_table, $text, undef;
    }
    push @$table, "Preconditions", $output->table('bullet',@pc_table) if @pc_table;
  }

  # Return value
  unless (grep /^return/i, @omit) {
    if (defined $self->prologue->{Return}) {
      push @$table, "Return Value", $output->context($self,$self->prologue->{Return},
						     $formals,$see_also,$exrefs);
    }
  }

  # Parallel notes
  unless (grep /^parallel/i, @omit) {
    if (defined $self->prologue->{Parallel}) {
      push @$table, "Parallel Notes", $output->context($self,$self->prologue->{Parallel},
						       $formals,$see_also,$exrefs);
    }
  }

  # Example. If this is a single paragraph (no blank lines) then typeset the whole thing as example code.
  # We force this to happen by inserting a blank at the beginning of each line.
  unless (grep /^example/i, @omit) {
    if (defined $self->prologue->{Example}) {
      if ($self->prologue->{Example} !~ /^\s*$/m) {
        $self->prologue->{Example} =~ s/^/ /mg;
      }
      push @$table, "Example", $output->context($self,$self->prologue->{Example},
						$formals,$see_also,$exrefs);
    }
  }

  # Issues from the function prologue or body
  unless (grep /^issue/i, @omit) {
    if (defined $self->prologue->{Issues}) {
      push @$table, "Issues", $output->context($self,$self->prologue->{Issues},
					       $formals,$see_also,$exrefs);
    }
  }

  # Bugs
  unless (grep /^bug/i, @omit) {
    if (defined $self->prologue->{Bugs}) {
      push @$table, "Known Bugs", $output->context($self,$self->prologue->{Bugs},
						   $formals,$see_also,$exrefs);
    }
  }

  # See also. This section is generated automatically from function names that
  # were mentioned above. In addition, the prologue section "Also" is scanned,
  # if present, for additional See-Also function names.
  unless (grep /^also/i, @omit) {
    my(@sa);
    $output->context($self, $self->prologue->{Also},$formals,$see_also,$exrefs); # for side effects
    foreach (sort keys %$see_also) {
      my($match) = Mkdoc::Docbase->list(name=>$_);
      next unless $match;
      next if $match eq $self;
      push @sa, ($output->xref($match->name,$output->code($match->name)),
		 ($output->sym_section . $match->secnum . ":  " .
		  $output->italic($match->prologue->{Purpose})));
    }
    # Cross reference to top of chapter.
    if ($self->up) {
      push @sa, ($output->xref($self->up->objid,$output->italic($output->escape(undef,$self->up->name))),
		 "Introduction for current chapter"),
    }
    push @$table, "See Also", $output->table('xref',@sa) if @sa;
  }
  return @$table;
}

# Summarize object
sub summary {
  return "";
}

1;
