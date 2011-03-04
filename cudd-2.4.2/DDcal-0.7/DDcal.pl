#! /usr/bin/perl -w

###PerlFile####################################################################
#
#  FileName	[DDcal.pl]
#  PackageName	[DDcal]
#  Synopsis	[BDD calculator with graphical interface.]
#  Description	[This script implements the interface of a BDD
#		calculator. It is based on Perl-Tk and uses the
#		Tk toolkit to draw the BDDs on the screen and to deal
#		with text entry. The script relies on another process to
#		parse the statements entered by the user and produce the
#		layout information for the BDDs. This other process is
#		called ddcal and is a lex/yacc based C program. ddcal
#		parses the statement entered by the user and performs the
#		corresponding BDD computations. Once the result BDD is
#		known, ddcal pipes a description of the result to dot,
#		which produces the layout information sent back to this
#		script. Many important details of this script have been
#		taken almost verbatim from the Perl-Tk widget demo.]
#
#  Author	[Fabio Somenzi <Fabio@Colorado.EDU>]
#
#  Revision	[$Id: DDcal.pl,v 1.27 2004/02/04 05:46:56 fabio Exp fabio $]
#
#  Copyright	[Copyright (c) 1996 Fabio Somenzi. All rights reserved.
#		This program is free software; you can redistribute it
#		and/or modify it under the same terms as Perl itself.]
#
###############################################################################

require 5.002;

use English;
use Tk;
use Tk::Dialog;
use Tk::ErrorDialog;
use Tk::FileSelect;
use strict;

###############################################################################
# Global subroutine and variable declarations required by my() and use strict.
###############################################################################

use subs qw(dpos view_widget_code handler dialogError entryFileLoad entryFileSave readFile);

my ($VIEW,$VIEW_TEXT);
my $VERSION = "0.6";

###############################################################################
# Create and initialize main window.
###############################################################################
my $MW = MainWindow->new;
$MW->title("DDcal $VERSION");
$MW->iconname('DDcal');

my $PROGNAME = "ddcal";
my $CONCENTRATE = 0;
my $DRAWID = 1;
my $COMPLEMENT_ARCS = 1;
my $FONT = '-*-Helvetica-Medium-R-Normal--*-140-*-*-*-*-*-*';
my $CFONT = '-*-Helvetica-Bold-R-Normal--*-140-*-*-*-*-*-*';
my $CANVAS_HEIGHT = "20c";
my @HISTORY = ();
my $HISTORYPTR = $#HISTORY;
my $command =  sub {exit(0);};
$MW->bind('<Control-q>' => $command);
$MW->bind('<Meta-q>' => $command);

###############################################################################
# Parse command line.
###############################################################################

push(@ARGV,"end");		# this is a workaround for a Perl bug.

while (defined(@ARGV) && $ARGV[0] =~ /^-/) {
    if ($ARGV[0] eq "-program") {
	shift;
	$PROGNAME = shift;
    } elsif ($ARGV[0] eq "-font") {
	shift;
	$CFONT = shift;
    } elsif ($ARGV[0] eq "-height") {
	shift;
	$CANVAS_HEIGHT = shift;
    } elsif ($ARGV[0] eq "-nocomplement") {
	shift;
	$COMPLEMENT_ARCS = 0;
    } elsif ($ARGV[0] eq "-noid") {
	shift;
	$DRAWID = 0;
    } else {
	&usage;
    }
}

# there should be no parameter left except for the sentinel "end."
if ($#ARGV != $[) {
    &usage;
}


###############################################################################
# Populate the main window with three frames:
# a menu bar, a canvas, and a text entry frame.
###############################################################################

###############################################################################
# Create the menu bar.
###############################################################################

my $menuBar = $MW->Frame;
$menuBar->pack(-side => 'top', -fill => 'x');

# Create a "File" menu with two items: view and quit.
my $menuBar_file = $menuBar->Menubutton(
    -text      => 'File',
    -underline => 0,
);
$menuBar_file->command(
    -label     => 'View',
    -underline => 0,
    -command   => [\&view_widget_code, __FILE__],
);
$menuBar_file->separator;
$menuBar_file->command(
    -label     => 'Quit',
    -underline => 0,
    -accelerator => 'Ctrl+Q',
    -command   => \&exit,
);
$menuBar_file->pack(-side => 'left');

# Create a button to save the current drawing in PostScript(tm) format.
# This button will be configured after the canvas is created.
my $menuBar_save = $menuBar->Button(
    -text      => 'Save',
    -underline => 0,
    -bd        => '0',
);
$menuBar_save->pack(-side => 'left');

# Create a button to load a file of statements.
# This button will be configured after the canvas is created.
my $menuBar_load = $menuBar->Button(
    -text      => 'Load',
    -underline => 0,
    -bd        => '0',
);
$menuBar_load->pack(-side => 'left');

# Create a button to reorder the BDDs.
# This button will be configured after the canvas is created.
my $menuBar_reorder = $menuBar->Button(
    -text      => 'Reorder',
    -underline => 0,
    -bd        => '0',
);
$menuBar_reorder->pack(-side => 'left');

# Create a button to examine the history.
# This button will be configured after the canvas is created.
my $menuBar_history = $menuBar->Button(
    -text      => 'History',
    -underline => 0,
    -bd        => '0',
);
$menuBar_history->pack(-side => 'left');

# Create a button to clone a copy of the current drawing.
my $menuBar_clone = $menuBar->Button(
    -text      => 'Clone',
    -underline => 0,
    -bd        => '0',
);
$menuBar_clone->pack(-side => 'left');
$command = sub {&cloneCanvas;};
$menuBar_clone->configure(-command => $command);
$MW->bind('<Alt-c>' => $command);
$MW->bind('<Meta-c>' => $command);

# Create a button to set the global options.
# This button will be configured after the canvas is created.
my $menuBar_options = $menuBar->Button(
    -text      => 'Options',
    -underline => 0,
    -bd        => '0',
);
$menuBar_options->pack(-side => 'left');

# Create a button to set the canvas font.
# This button will be configured after the canvas is created.
my $menuBar_font = $menuBar->Button(
    -text      => 'Font',
    -underline => 3,
    -bd        => '0',
);
$menuBar_font->pack(-side => 'left');

# Create a "Help" button.
my $menuBar_help = $menuBar->Button(
    -text      => 'Help',
    -underline => 1,
    -bd        => '0',
);
$menuBar_help->pack(-side => 'right');
$command = sub {&displayHelp;};
$menuBar_help->configure(-command => $command);
$MW->bind('<Alt-e>' => $command);
$MW->bind('<Meta-e>' => $command);

###############################################################################
# Create the drawing canvas.
###############################################################################

my $w_frame = $MW->Frame();
$w_frame->pack(-side => 'top', -fill => 'both', -expand => 'yes');

my $c = $w_frame->Canvas(
    -scrollregion => ['0c', '0c', '8.5i', '11i'],
    -width        => '15c',
    -height       => $CANVAS_HEIGHT,
    -relief       => 'sunken',
    -bd           => 2,
);
$command = sub {entryFileSave($c);};
$menuBar_save->configure(-command => $command);
$MW->bind('<Alt-s>' => $command);
$MW->bind('<Meta-s>' => $command);
$command = sub {entryFileLoad($c);};
$menuBar_load->configure(-command => $command);
$MW->bind('<Alt-l>' => $command);
$MW->bind('<Meta-l>' => $command);
$command = sub {process_statement($c,":reorder");
		process_statement($c,":last");};
$menuBar_reorder->configure(-command => $command);
$MW->bind('<Alt-r>' => $command);
$MW->bind('<Meta-r>' => $command);
$command = sub {showHistory($c);};
$menuBar_history->configure(-command => $command);
$MW->bind('<Alt-h>' => $command);
$MW->bind('<Meta-h>' => $command);
$command = sub {selectOptions($c);};
$menuBar_options->configure(-command => $command);
$MW->bind('<Alt-o>' => $command);
$MW->bind('<Meta-o>' => $command);
$command = sub {entryCanvasFont($c);};
$menuBar_font->configure(-command => $command);
$MW->bind('<Alt-t>' => $command);
$MW->bind('<Meta-t>' => $command);

# Add vertical and horizontal scrollbars to the canvas frame.

my $w_frame_vscroll = $w_frame->Scrollbar(-command => [$c => 'yview']);
my $w_frame_hscroll = $w_frame->Scrollbar(
    -orient => 'horiz',
    -command => [$c => 'xview'],
);
$c->configure(-xscrollcommand => [$w_frame_hscroll => 'set']);
$c->configure(-yscrollcommand => [$w_frame_vscroll => 'set']);
$w_frame_hscroll->pack(-side => 'bottom', -fill => 'x');
$w_frame_vscroll->pack(-side => 'right', -fill => 'y');
$c->pack(-expand => 'yes', -fill => 'both');

# Choose fonts and colors.

# my $font1 = '-*-Helvetica-Medium-R-Normal--*-120-*-*-*-*-*-*';
# my $font2 = '-*-Helvetica-Bold-R-Normal--*-240-*-*-*-*-*-*';
my($blue, $red, $bisque, $green);
if ($MW->depth > 1) {
    $blue = 'DeepSkyBlue3';
    $red = 'red';
    $bisque = 'bisque3';
    $green = 'SeaGreen3';
} else {
    $blue = 'black';
    $red = 'black';
    $bisque = 'black';
    $green = 'black';
}

###############################################################################
# Create statement entry frame: label, text entry widget, and two buttons.
###############################################################################

my $stmt = '';
my $w_stmt = $MW->Frame;
my $w_stmt_label = $w_stmt->Label(-text => 'Statement:', -width => 13,
                                  -anchor => 'w');
my $w_stmt_entry = $w_stmt->Entry(-width => 40,
                                  -textvariable => \$stmt,
				  -font => $FONT);
my $w_stmt_submit = $w_stmt->Button(-text => 'Submit');
my $w_stmt_clear  = $w_stmt->Button(-text => 'Clear');
$w_stmt_label->pack(-side => 'left');
$w_stmt_entry->pack(-side => 'left');
$w_stmt_submit->pack(-side => 'left', -pady => 5, -padx => 10);
$w_stmt_clear->pack(-side => 'left', -pady => 5, -padx => 10);
# This frame gets the focus by default.
$w_stmt_entry->focus;
$w_stmt->pack(-side => 'top', -fill => 'x');

$command =  sub {process_statement($c,$stmt)};
$w_stmt_submit->configure(-command => $command);
$w_stmt_entry->bind('<Return>' => $command);

$command =  sub {$stmt = '';};
$w_stmt_clear->configure(-command => $command);
$w_stmt_entry->bind('<Control-u>' => $command);

$command =  sub {&scrollUpHistory;};
$w_stmt_entry->bind('<Control-p>' => $command);
$w_stmt_entry->bind('<Up>' => $command);

$command =  sub {&scrollDownHistory;};
$w_stmt_entry->bind('<Control-n>' => $command);
$w_stmt_entry->bind('<Down>' => $command);


###############################################################################
# Start the parser.
###############################################################################

# Install signal handler to detect broken pipes.
$SIG{'PIPE'} = \&handler;

# Create two pipes to establish bidirectional communication to/from ddcal.
pipe(TOPIPEREADHANDLE,TOPIPEWRITEHANDLE);
pipe(FROMPIPEREADHANDLE,FROMPIPEWRITEHANDLE);

# Unbuffer TOPIPEWRITEHANDLE.
select((select(TOPIPEWRITEHANDLE), $| = 1)[$[]);

# Fork ddcal.

my $pid;

FORK: {
    if ($pid = fork) { # parent
	close(TOPIPEREADHANDLE);
	close(FROMPIPEWRITEHANDLE);
    } elsif (defined $pid) { # child
	open(STDIN, "<&TOPIPEREADHANDLE") || die "can't dup to stdin";
	open(STDOUT, ">&FROMPIPEWRITEHANDLE") || die "can't dup to stdout";
	close(TOPIPEWRITEHANDLE);
	close(FROMPIPEREADHANDLE);
	close(TOPIPEREADHANDLE);
	close(FROMPIPEWRITEHANDLE);
	exec("$PROGNAME -g") || die "Can't exec ddcal: $!\n";
    } elsif ($! =~ /No more process/) { # try again later
	sleep 5;
	redo FORK;
    } else {	# can't figure it out: give up
	die "Can't fork: $!\n";
    }
}

###############################################################################
# Pass options to ddcal.
###############################################################################
if ($COMPLEMENT_ARCS == 0) {
    process_statement($c,":nocomplementarcs");
}

###############################################################################
# Enter the event-processing loop.
###############################################################################

MainLoop;

# Not reached.


##Sub##########################################################################
#
# Synopsis	[Processes a statement and draws the BDD on the canvas.]
#
# Description	[Processes a statement and draws the BDD on the canvas.
#		The input statement is sent to the parser. The parser
#		responds in one of three ways: (1) By sending a graph
#		for a BDD. In this case the first line starts with
#		"graph" and the last line is "stop". (2) By sending just
#		the word "stop". This happens when no drawing is
#		generated by the input statement. (3) By sending just
#		the word "error". This happens when the input statement
#		contains a syntax error. This procedure parses the
#		response of the parser and, if required, draws a new BDD
#		on the canvas.]
#
# SideEffects	[]
#
###############################################################################
sub process_statement {

    my ($c,$stmt) = @ARG;
    my $savecursor = $MW->cget(-cursor);
    $MW->configure(-cursor => 'watch');
    push(@HISTORY,$stmt);
    $HISTORYPTR = $#HISTORY;
    print TOPIPEWRITEHANDLE "$stmt\n";
    # Deal with continuation lines. A backslash in a comment
    # does not indicate continuation. Therefore a backslash must not be
    # preceded by '#' and may only be followed by white space.
    if ($stmt =~ /^[^\#]*\\\s*$/) {
	$MW->configure(-cursor => $savecursor);
	return;
    }

    my $thisline;
    my ($name,$label);
    my ($graph, $scalefactor, $bounding_box_x, $bounding_box_y);
    my $offset_x = 10;
    my $offset_y = 10;
    my ($bbulx,$bbuly,$bblrx,$bblry);

    LOOP:
    while (defined(\*FROMPIPEREADHANDLE) && ($thisline = <FROMPIPEREADHANDLE>)) {
	chop($thisline);
	if ($thisline =~ /^graph/) {
	    $c->delete('all');
	    ($graph, $scalefactor, $bounding_box_x, $bounding_box_y) = split(' ',$thisline);
	    $bounding_box_x *= 72;
	    $bounding_box_y *= 72;
	} elsif ($thisline =~ /^node/) {
	    $thisline =~ s/^node\s+//;
	    if ($thisline =~ /^(\"[^\"]*\")/) {
		$name = $1;
	    } elsif ($thisline =~ /(\S+)/) {
		$name = $1;
	    } else {
		$name = "unknown";
	    }
	    $thisline =~ s/^$name\s+//;
	    $thisline =~ /((\d+\.\d+)\s+(\d+\.\d+)\s+(\d+\.\d+)\s+(\d+\.\d+))/;
	    my $numbers = $1;
	    my $center_x = $2;
	    my $center_y = $3;
	    my $xsize = $4;
	    my $ysize = $5;
	    $thisline =~ s/^$numbers\s+//;
	    if ($thisline =~ /^(\"[^\"]*\")/) {
		$label = $1;
	    } elsif ($thisline =~ /(\S+)/) {
		$label = $1;
	    } else {
		$label = "unknown";
	    }
	    $thisline =~ s/^$label\s+//;
	    my $style;
	    if ($thisline =~ /\b(invis)\b/) {
		next;
	    }
	    if ($thisline =~ /\b(plaintext)\b/) {
		$style = $1;
	    } elsif ($thisline =~ /\b(ellipse)\b/) {
		$style = $1;
	    } elsif ($thisline =~ /\b(box)\b/) {
		$style = $1;
	    } else {
		$style = "unknown";
	    }
	    $label =~ tr/\"//d;
	    # print "node $label $center_x $center_y $xsize $ysize $style\n";
	    # Convert from inches to points.
	    $center_x = $offset_x + $center_x * 72;
	    $center_y = $offset_y + $bounding_box_y - $center_y * 72;
	    $xsize *= 72;
	    $ysize *= 72;
	    # Convert from center/size to upper-left/lower-right.
	    $bbulx = $center_x - $xsize / 2.0;
	    $bbuly = $center_y - $ysize / 2.0;
	    $bblrx = $center_x + $xsize / 2.0;
	    $bblry = $center_y + $ysize / 2.0;
	    # Apply the scaling factor
	    $center_x *= $scalefactor;
	    $center_y *= $scalefactor;
	    $bbulx *= $scalefactor;
	    $bbuly *= $scalefactor;
	    $bblrx *= $scalefactor;
	    $bblry *= $scalefactor;
	    # print "node $label $bbulx $bbuly $bblrx $bblry $style $center_x $center_y\n";
	    if ($style eq "ellipse") {
		$c->create('oval',$bbulx,$bbuly,$bblrx,$bblry,
			   -outline => $red, qw(-width 2 -tags item));
	    } elsif ($style eq "box") {
		$c->create('rect',$bbulx,$bbuly,$bblrx,$bblry,
			   -outline => $red, qw(-width 2 -tags item));
	    } elsif ($style eq "plaintext") {
	    } elsif ($style eq "unknown") {
	    }
	    if ($DRAWID || $style ne "ellipse") {
		$c->create('text',$center_x,$center_y,'-text',
			    $label,'-anchor','c','-font',$CFONT);
	    }
	} elsif ($thisline =~ /^edge/) {
	    $thisline =~ s/^edge\s+//;
	    if ($thisline =~ /^(\"[^\"]*\")/) {
		$name = $1;
	    } elsif ($thisline =~ /(\S+)/) {
		$name = $1;
	    }
	    $thisline =~ s/^$name\s+//;
	    if ($thisline =~ /^(\"[^\"]*\")/) {
		$name = $1;
	    } elsif ($thisline =~ /(\S+)/) {
		$name = $1;
	    }
	    $thisline =~ s/^$name\s+//;
	    $thisline =~ /(\d+)/;
	    my $points = $1;
	    $thisline =~ s/^$points\s+//;
	    my @points;
	    @points = ();
	    my ($thispoint,$thisx,$thisy);
	    for (1 .. $points) {
		$thisline =~ /((\d+\.\d+)\s+(\d+\.\d+))/;
		$thispoint = $1;
		$thisx = $2;
		$thisy = $3;
		$thisx = $thisx * 72 + $offset_x;
		$thisy = $bounding_box_y - $thisy * 72 + $offset_y;
		# Apply scaling factor.
		$thisx *= $scalefactor;
		$thisy *= $scalefactor;
		push(@points,$thisx);
		push(@points,$thisy);
		$thisline =~ s/^$thispoint\s+//;
	    }
	    my $style;
	    if ($thisline =~ /\b(invis)\b/) {
		next;
	    }
	    if ($thisline =~ /\b(solid)\b/) {
		$style = $1;
	    } elsif ($thisline =~ /\b(dashed)\b/) {
		$style = $1;
	    } elsif ($thisline =~ /\b(dotted)\b/) {
		$style = $1;
	    } else {
		$style = "unknown";
	    }
	    # print "edge $points @points $style\n"; # DEBUG
	    if ($style eq "solid") {
		$c->create('line',@points,'-smooth','on',
			   -fill => $blue, qw(-width 2 -tags item));
	    } elsif ($style eq "dashed") {
		$c->create('line',@points,'-smooth','on',
			   -fill => $red,
			   qw(-width 2 -stipple gray50 -tags item));
	    } elsif ($style eq "dotted") {
		$c->create('line',@points,'-smooth','on',
			   -fill => 'black',
			   qw(-width 2 -stipple gray25 -tags item));
	    }
	} elsif ($thisline =~ /^stop/) {
	    last;
	} elsif ($thisline =~ /^beep/) {
	    $MW->bell;
	    last;
	} elsif ($thisline =~ /^error/) {
	    $MW->bell;
	    &dialogError;
	    last;
	}
    }
    $MW->configure(-cursor => $savecursor);

} # end process_statement


##Sub##########################################################################
#
# Synopsis	[Expose a file's innards to the world too, but only for
#		viewing.]
#
# Description	[Expose a file's innards to the world too, but only for
#		viewing. This sub is taken verbatim from the tkperl
#		widget demo.]
#
# SideEffects	[]
#
###############################################################################
sub view_widget_code {

    # Expose a file's innards to the world too, but only for viewing.

    my($widget) = @ARG;

    if (not Exists $VIEW) {
	$VIEW = $MW->Toplevel;
	$VIEW->iconname('DDcal');
	my $view_buttons=$VIEW->Frame;
	$view_buttons->pack(-side => 'bottom',  -expand => 1, -fill => 'x');
	my $view_buttons_dismiss = $view_buttons->Button(
            -text    => 'Dismiss',
            -command => [$VIEW => 'withdraw'],
	);
	$view_buttons_dismiss->pack(-side => 'left', -expand => 1);
	$VIEW_TEXT = $VIEW->Text(-height => 40, -setgrid => 1);
	$VIEW_TEXT->pack(-side => 'left', -expand => 1, -fill => 'both');
	my $view_scroll = $VIEW->Scrollbar(
            -command => [$VIEW_TEXT => 'yview'],
        );
	$view_scroll->pack(-side => 'right', -fill => 'y');
	$VIEW_TEXT->configure(-yscrollcommand =>  [$view_scroll => 'set']);
    } else {
	$VIEW->deiconify;
	$VIEW->raise;
    }
    $VIEW->title("Demo code: $widget");
    $VIEW_TEXT->configure(-state => 'normal');
    $VIEW_TEXT->delete('1.0', 'end');
    open(VIEW, "<$widget") or
	die("cannot open widget demo file $widget!");
    {
	local $INPUT_RECORD_SEPARATOR = undef;
	$VIEW_TEXT->insert('1.0', <VIEW>);
    }
    close VIEW;
    $VIEW_TEXT->mark('set', 'insert', '1.0');
    $VIEW_TEXT->configure(-state => 'disabled');

} # end view_widget_code


##Sub##########################################################################
#
# Synopsis	[Generic signal handler.]
#
# Description	[Generic signal handler. Reports what signal has been
#		caught by creating a dialog box.]
#
# SideEffects	[None.]
#
###############################################################################
sub handler {
    my ($sig) = @_;
    my $dialog;
    my($ok) = ('OK');

    $dialog = $MW->Dialog(
	-title          => 'Signal Received',
	-text           => '',
	-bitmap         => 'error',
	-default_button => $ok,
	-buttons        => [$ok],
    );
    $dialog->configure(
	-wraplength => '4i',
	-text       => "Received a SIG$sig signal. Communications with the computation engine may by interrupted. Click on the \"OK\" button to continue.",
    );

    $dialog->Show;

} # end handler


##Sub##########################################################################
#
# Synopsis	[Displays dialog box for syntax errors.]
#
# Description	[]
#
# SideEffects	[None.]
#
###############################################################################
sub dialogError {

    my $dialog;
    my($ok) = ('OK');

    $dialog = $MW->Dialog(
	-title          => 'Syntax Error',
	-text           => '',
	-bitmap         => 'error',
	-default_button => $ok,
	-buttons        => [$ok],
    );
    $dialog->configure(
	-wraplength => '4i',
	-text       => 'A syntax error was detected in the input statement. The statement was ignored. Click on the "OK" button to continue.',
    );

    $dialog->Show;

} # end dialogError


##Sub##########################################################################
#
# Synopsis	[Displays selection box for file loading.]
#
# Description	[]
#
# SideEffects	[None.]
#
###############################################################################
sub entryFileLoad {

    my ($canvas) = @ARG;
    my $start_dir = ".";
    my $FSref = $MW->FileSelect(-directory => $start_dir);
    $FSref->title('Load File');
    $FSref->iconname('Load File');
    my $filename = $FSref->Show;
    if (defined($filename)) {
	readFile($canvas,$filename);
    }

} # end entryFileLoad


##Sub##########################################################################
#
# Synopsis	[Saves canvas to PostScript(tm) file with options.]
#
# Description	[]
#
# SideEffects	[None.]
#
###############################################################################
sub entryFileSave {

    my ($canvas) = @ARG;
    my $w;
    my($save) = ('Save');

    $w = $MW->Toplevel;
    dpos $w;
    $w->title('Save to File');
    $w->iconname('Save to File');
    my $w_msg = $w->Label(
	-font       => $FONT,
	-wraplength => '4i',
	-justify    => 'left',
	-text       => 'Save the current drawing in a file in PostScript(tm) format. Select the options, enter the file name and click on the "Save" button.',
    );
    $w_msg->pack;

    my $filename = 'default.ps';
    my $w_e = $w->Entry(-relief => 'sunken', -textvariable => \$filename);
    my @pl = (-side => 'top', -padx => 10, -pady => 5, -fill => 'x');
    $w_e->focus;
    $w_e->pack(@pl);

    my $w_radio = $w->Frame;
    @pl = (-side => 'top', -expand => 1, -padx => '.5c', -pady => '.5c');
    $w_radio->pack(@pl);
    my ($r, $mode);
    my $colormode = 'gray';
    foreach $mode ('color', 'gray') {
	$r = $w_radio->Radiobutton(
	    -text     => "$mode",
	    -variable => \$colormode,
	    -relief   => 'flat',
	    -value    => $mode,
	);
	$r->pack(-side => 'top', -pady => '2', -anchor => 'w');
    }

    my $w_buttons = $w->Frame;
    $w_buttons->pack(qw(-side bottom -fill x -pady 2m));
    my $command = sub {$canvas->postscript('-file' => $filename,
		       '-colormode' => $colormode, '-width' => '8.5i',
		       '-height' => '11i', '-y' => '-5m'); $w->destroy;};
    my $w_save = $w_buttons->Button(
	-text    => 'Save',
	-command => $command,
    );
    $w_save->pack(qw(-side left -expand 1));
    my $w_cancel = $w_buttons->Button(
	-text    => 'Cancel',
	-command => [$w => 'destroy'],
    );
    $w_cancel->pack(qw(-side left -expand 1));

} # end entryFileSave


##Sub##########################################################################
#
# Synopsis	[Select font for canvas.]
#
# Description	[]
#
# SideEffects	[None.]
#
###############################################################################
sub entryCanvasFont {

    my ($canvas) = @ARG;
    my $w;
    my($save) = ('Font');

    $w = $MW->Toplevel;
    dpos $w;
    $w->title('Select Canvas Font');
    $w->iconname('Select Font');
    my $w_msg = $w->Label(
	-font       => $FONT,
	-wraplength => '4i',
	-justify    => 'left',
	-text       => 'Select the font used to write on the canvas. Enter the font name and click on the "Select" button.',
    );
    $w_msg->pack;

    my $fontname = $CFONT;
    my $w_e = $w->Entry(-relief => 'sunken', -textvariable => \$fontname);
    my @pl = (-side => 'top', -padx => 10, -pady => 5, -fill => 'x');
    $w_e->focus;
    $w_e->pack(@pl);

    my $w_buttons = $w->Frame;
    $w_buttons->pack(qw(-side bottom -fill x -pady 2m));
    my $command = sub {$CFONT = $fontname;
		       process_statement($c, ":last");
		       $w->destroy;
		   };
    my $w_save = $w_buttons->Button(
	-text    => 'Save',
	-command => $command,
    );
    $w_save->pack(qw(-side left -expand 1));
    my $w_cancel = $w_buttons->Button(
	-text    => 'Cancel',
	-command => [$w => 'destroy'],
    );
    $w_cancel->pack(qw(-side left -expand 1));

} # end entryCanvasFont


##Sub##########################################################################
#
# Synopsis	[Position a window at a reasonable place on the display.]
#
# Description	[]
#
# SideEffects	[None.]
#
###############################################################################
sub dpos {

    shift->geometry('+400+400');

} # end dpos


##Sub##########################################################################
#
# Synopsis	[Reads a file of statements and executes them.]
#
# Description	[]
#
# SideEffects	[None.]
#
###############################################################################
sub readFile {
    my ($canvas,$filename) = @ARG;
    my $line;

    if (not open(BATCHREAD, $filename)) {
	$MW->Dialog(
	    -title  => 'File Not Found',
	    -text   => $OS_ERROR,
	    -bitmap => 'error',
	)->Show;
	return;
    }
    while (defined(\*BATCHREAD) && ($line = <BATCHREAD>)) {
	chop($line);
	process_statement($canvas,$line);
    }
    # close(BATCHREAD);

} # end readFile


##Sub##########################################################################
#
# Synopsis	[Shows the statements executed so far.]
#
# Description	[]
#
# SideEffects	[None.]
#
###############################################################################
sub showHistory {

    my ($canvas) = @ARG;
    my $w = $MW->Toplevel;
    $w->title('History');
    $w->iconname('History');
    dpos $w;
    my $w_msg = $w->Label(
	-font       => $FONT,
	-justify    => 'left',
	-wraplength => '5i',
	-text       => 'This is the list of all statements entered so far. Double click with the first button on one item to submit it. Single click with the first button to select an item. Single click with the third button (anywhere in the list) to paste the selected item to the statement entry window.',
    );
    $w_msg->pack;

    # Create button frame.
    my $w_buttons = $w->Frame;
    $w_buttons->pack(qw(-side bottom -fill x -pady 2m));

    my $w_dismiss = $w_buttons->Button(
        -text    => 'Dismiss',
        -command => [$w => 'destroy'],
    );
    $w_dismiss->pack(qw(-side left -expand 1));

    my $command = sub {&saveHistory;};
    my $w_save = $w_buttons->Button(
        -text    => 'Save',
        -command => $command,
    );
    $w_save->pack(qw(-side left -expand 1));

    my $w_frame = $w->Frame(-borderwidth => 10);
    $w_frame->pack(-side => 'top', -expand => 'yes', -fill => 'y');

    my $w_frame_yscroll = $w_frame->Scrollbar;
    $w_frame_yscroll->pack(-side => 'left', -fill => 'y');
    my $w_frame_xscroll = $w_frame->Scrollbar(-orient => 'horizontal');
    $w_frame_xscroll->pack(-side => 'bottom', -fill => 'x');

    my $w_frame_list = $w_frame->Listbox(
        -yscrollcommand => [$w_frame_yscroll => 'set'],
	-xscrollcommand => [$w_frame_xscroll => 'set'],
        -width          => 50,
        -height         => 16,
	-setgrid        => 1,
    );
    $w_frame_list->pack(-side => 'left', -fill => 'y');
    $w_frame_yscroll->configure(-command => [$w_frame_list => 'yview']);
    $w_frame_xscroll->configure(-command => [$w_frame_list => 'xview']);

    $command = sub {	$w_frame_list->delete(0,'end');
			my $item; for $item (@HISTORY) {
			    $w_frame_list->insert('end', $item);
			}
			$w_frame_list->see('end');
		   };
    my $w_update = $w_buttons->Button(
        -text    => 'Update',
        -command => $command,
    );
    $w_update->pack(qw(-side left -expand 1));

    $w_frame_list->bind('<Double-Button-1>' => 
        sub  {
	    $stmt = $_[0]->get('active');
	    process_statement($canvas,$stmt);
	},
    );

    $command = sub  {$stmt = $_[0]->get('active');};
    $w_frame_list->bind('<Button-3>' => $command);

    my $item;
    for $item (@HISTORY) {
	$w_frame_list->insert('end', $item);
    }
    $w_frame_list->see('end');

} # end showHistory


##Sub##########################################################################
#
# Synopsis	[Scrolls up in the history list.]
#
# Description	[]
#
# SideEffects	[None.]
#
###############################################################################
sub scrollUpHistory {
    if ($HISTORYPTR > $[) {
	$HISTORYPTR--;
	$stmt = $HISTORY[$HISTORYPTR];
    } else {
	$MW->bell;
    }

} # end scrollUpHistory


##Sub##########################################################################
#
# Synopsis	[Scrolls down in the history list.]
#
# Description	[]
#
# SideEffects	[None.]
#
###############################################################################
sub scrollDownHistory {
    if ($HISTORYPTR < $#HISTORY) {
	$HISTORYPTR++;
	$stmt = $HISTORY[$HISTORYPTR];
    } else {
	$MW->bell;
    }

} # end scrollDownHistory


##Sub##########################################################################
#
# Synopsis	[Saves history to file.]
#
# Description	[]
#
# SideEffects	[None.]
#
###############################################################################
sub saveHistory {

    my $w;
    my($save) = ('Save');

    $w = $MW->Toplevel;
    dpos $w;
    $w->title('Save History');
    $w->iconname('Save History');
    my $w_msg = $w->Label(
	-font       => $FONT,
	-wraplength => '4i',
	-justify    => 'left',
	-text       => 'Save the current history in a file. Enter the file name and click on the "Save" button.',
    );
    $w_msg->pack;

    my $filename = 'history.sav';
    my $w_e = $w->Entry(-relief => 'sunken', -textvariable => \$filename);
    my @pl = (-side => 'top', -padx => 10, -pady => 5, -fill => 'x');
    $w_e->focus;
    $w_e->pack(@pl);

    my $w_buttons = $w->Frame;
    $w_buttons->pack(qw(-side bottom -fill x -pady 2m));
    my $command = sub {writeHistory($filename); $w->destroy;};
    my $w_save = $w_buttons->Button(
	-text    => 'Save',
	-command => $command,
    );
    $w_save->pack(qw(-side left -expand 1));
    my $w_cancel = $w_buttons->Button(
	-text    => 'Cancel',
	-command => [$w => 'destroy'],
    );
    $w_cancel->pack(qw(-side left -expand 1));

} # end saveHistory


##Sub##########################################################################
#
# Synopsis	[Saves history to file.]
#
# Description	[]
#
# SideEffects	[None.]
#
###############################################################################
sub writeHistory {
    my ($filename) = @ARG;

    if (not open(HISTORY, ">$filename")) {
	$MW->Dialog(
	    -title  => 'File Error',
	    -text   => $OS_ERROR,
	    -bitmap => 'error',
	)->Show;
	return;
    }
    my $line;
    for $line (@HISTORY) {
	printf HISTORY "$line\n";
    }
    close(HISTORY);

} # end writeHistory


##Sub##########################################################################
#
# Synopsis	[Clones the main canvas.]
#
# Description	[]
#
# SideEffects	[None.]
#
###############################################################################
sub cloneCanvas {

    my $w = $MW->Toplevel;
    $w->title('Clone');
    $w->iconname('Clone');
    my $w_frame = $w->Frame();
    $w_frame->pack(-side => 'top', -fill => 'both', -expand => 'yes');

    my $c = $w_frame->Canvas(
	-scrollregion => ['0c', '0c', '30c', '36c'],
	-width        => '15c',
	-height       => $CANVAS_HEIGHT,
	-relief       => 'sunken',
	-bd           => 2,
    );

    # Add vertical and horizontal scrollbars to the canvas frame.

    my $w_frame_vscroll = $w_frame->Scrollbar(-command => [$c => 'yview']);
    my $w_frame_hscroll = $w_frame->Scrollbar(
	-orient => 'horiz',
	-command => [$c => 'xview'],
    );
    $c->configure(-xscrollcommand => [$w_frame_hscroll => 'set']);
    $c->configure(-yscrollcommand => [$w_frame_vscroll => 'set']);
    $w_frame_hscroll->pack(-side => 'bottom', -fill => 'x');
    $w_frame_vscroll->pack(-side => 'right', -fill => 'y');
    $c->pack(-expand => 'yes', -fill => 'both');

    # Create button frame.
    my $w_buttons = $w->Frame;
    $w_buttons->pack(qw(-side bottom -fill x -pady 2m));

    my $w_dismiss = $w_buttons->Button(
        -text    => 'Dismiss',
        -command => [$w => 'destroy'],
    );
    $w_dismiss->pack(qw(-side left -expand 1));

    my $w_update = $w_buttons->Button(
        -text    => 'Update',
        -command => sub {process_statement($c, ":last");},
    );
    $w_update->pack(qw(-side left -expand 1));

    # Copy from the original canvas
    process_statement($c,":last");

} # end cloneCanvas


##Sub##########################################################################
#
# Synopsis	[Saves canvas to PostScript(tm) file with options.]
#
# Description	[]
#
# SideEffects	[None.]
#
###############################################################################
sub selectOptions {

    my ($canvas) = @ARG;
    my $w;
    my($ok) = ('OK');

    $w = $MW->Toplevel;
    dpos $w;
    $w->title('Options');
    $w->iconname('Options');
    my $w_msg = $w->Label(
	-font       => $FONT,
	-wraplength => '4i',
	-justify    => 'left',
	-text       => "Select the options, and click on the $ok button.",
    );
    $w_msg->pack;

    my $w_radio_buttons = $w->Frame;
    $w_radio_buttons->pack(qw(-side top -fill x -pady 2m));

    my $w_radio_l = $w_radio_buttons->Frame;
    my @pl = (-side => 'left', -expand => 1, -padx => '.5c', -pady => '.5c');
#    $w_radio_l->pack(@pl); # dot -Tplain -Gconcentrate=true is buggy
    my ($r, $mode);
    my $concentrate = $CONCENTRATE;
    my @texts = ('do not concentrate arcs', 'concentrate arcs');
    foreach $mode (0, 1) {
	$r = $w_radio_l->Radiobutton(
	    -text     => $texts[$mode+$[],
	    -variable => \$concentrate,
	    -relief   => 'flat',
	    -value    => $mode,
	);
	$r->pack(-side => 'top', -pady => '2', -anchor => 'w');
    }


    my $w_radio_c = $w_radio_buttons->Frame;
    @pl = (-side => 'left', -expand => 1, -padx => '.5c', -pady => '.5c');
    $w_radio_c->pack(@pl);
    my $complementArcs = $COMPLEMENT_ARCS;
    @texts = ('no complement arcs', 'complement arcs');
    foreach $mode (0, 1) {
	$r = $w_radio_c->Radiobutton(
	    -text     => $texts[$mode+$[],
	    -variable => \$complementArcs,
	    -relief   => 'flat',
	    -value    => $mode,
	);
	$r->pack(-side => 'top', -pady => '2', -anchor => 'w');
    }


    my $w_radio_r = $w_radio_buttons->Frame;
    @pl = (-side => 'right', -expand => 1, -padx => '.5c', -pady => '.5c');
    $w_radio_r->pack(@pl);
    my $drawid = $DRAWID;
    @texts = ('do not draw node IDs', 'draw node IDs');
    foreach $mode (0, 1) {
	$r = $w_radio_r->Radiobutton(
	    -text     => $texts[$mode+$[],
	    -variable => \$drawid,
	    -relief   => 'flat',
	    -value    => $mode,
	);
	$r->pack(-side => 'top', -pady => '2', -anchor => 'w');
    }

    my $w_buttons = $w->Frame;
    $w_buttons->pack(qw(-side bottom -fill x -pady 2m));
    my $command = sub {
	my $changed = $DRAWID != $drawid;
	$CONCENTRATE = $concentrate;
	if ($CONCENTRATE == 1) {
	    process_statement($canvas,':concentrate');
	} else {
	    process_statement($canvas,':noconcentrate');
	}
	$COMPLEMENT_ARCS = $complementArcs;
	if ($COMPLEMENT_ARCS == 1) {
	    process_statement($canvas,':complementarcs');
	} else {
	    process_statement($canvas,':nocomplementarcs');
	}
	$DRAWID = $drawid;
	if ($changed) {
	    process_statement($canvas, ":last");
	}
	$w->destroy;
    };
    my $w_ok = $w_buttons->Button(
	-text    => $ok,
	-command => $command,
    );
    $w_ok->pack(qw(-side left -expand 1));
    my $w_cancel = $w_buttons->Button(
	-text    => 'Cancel',
	-command => [$w => 'destroy'],
    );
    $w_cancel->pack(qw(-side left -expand 1));

} # end selectOptions


##Sub##########################################################################
#
# Synopsis	[Main help window.]
#
# Description	[]
#
# SideEffects	[None.]
#
###############################################################################
sub displayHelp {

    # Create a top-level window that illustrates how you can bind Perl
    # commands to regions of text in a text widget.

    my $w = $MW->Toplevel;
    $w->title('Help');
    $w->iconname('Help');

    my $w_buttons = $w->Frame;
    $w_buttons->pack(qw(-side bottom -fill x -pady 2m));
    my $w_dismiss = $w_buttons->Button(
        -text    => 'Dismiss',
        -command => [$w => 'destroy'],
    );
    $w_dismiss->pack(qw(-side left -expand 1));

    my $w_t = $w->Text(-setgrid => 'true', -width => '40', -height => '16',
			-font => $FONT, -wrap => 'word');
    my $w_s = $w->Scrollbar(-command => [$w_t => 'yview']);
    $w_t->configure(-yscrollcommand => [$w_s => 'set']);
    $w_s->pack(-side => 'right', -fill => 'y');
    $w_t->pack(-expand => 'yes', -fill => 'both');

    # Set up display styles

    my(@bold, @normal, $tag);
    if ($w->depth > 1) {
	@bold   = (-background => '#43ce80', -relief => 'raised',
		   -borderwidth => 1);
	@normal = (-background => undef, -relief => 'flat');
    } else {
	@bold   = (-foreground => 'white', -background => 'black');
	@normal = (-foreground => undef, -background => undef);
    }

    $w_t->insert('end', '1. Input format', 'd1');
    $w_t->insert('end', "\n\n");
    $w_t->insert('end', '2. Interpreting the drawings', 'd2');
    $w_t->insert('end', "\n\n");
    $w_t->insert('end', '3. Operators in order of decreasing precedence', 'd3');
    $w_t->insert('end', "\n\n");
    $w_t->insert('end', '4. Displaying multiple functions', 'd4');
    $w_t->insert('end', "\n\n");
    $w_t->insert('end', '5. Commands', 'd5');
    $w_t->insert('end', "\n\n");
    $w_t->insert('end', '6. Examples', 'd6');
    $w_t->insert('end', "\n\n");
    $w_t->insert('end', '7. Postscript output', 'd7');
    $w_t->insert('end', "\n\n");
    $w_t->insert('end', '8. Scripts', 'd8');

    foreach $tag (qw(d1 d2 d3 d4 d5 d6 d7 d8)) {
	$w_t->tag('bind', $tag, '<Any-Enter>' => 
            sub {shift->tag('configure', $tag, @bold)}
        );
	$w_t->tag('bind', $tag, '<Any-Leave>' =>
            sub {shift->tag('configure', $tag, @normal)}
        );
    }
    $w_t->configure('-state' => 'disabled');

    $w_t->tag('bind', 'd1', '<1>' => sub {helpMessage('Input Format',
'The input to DDCal is a sequence of statements.  Each statement is contained in one line.  Each statement specifies a computation or a command.

Everything from \'#\' to the end of the line is considered a comment.

If a line ends with \'\\\' (possibly followed by white space) then it is continued by the next line.  A backslash inside a comment does not indicate continuation.

Variable and function names are alphanumeric strings starting with an alphabetic character.'
    );});
    $w_t->tag('bind', 'd2', '<1>' => sub {helpMessage('Drawings',
'Blue solid arcs are \'then\' arcs.  Red acs drawn with a 50% stipple pattern are regular \'else\' arcs.  Finally, black arcs drawn with a 25% stipple pattern are complemented \'else\' arcs.

The internal nodes are drawn as ellipses.  The function nodes (the roots) and the terminal node are drawn as rectangles.  All nodes labeled by the same variable are drawn on the same horizontal line.  The variable name is shown at the far left.

The numbers shown inside the internal nodes are unique hexadecimal identifiers derived from the node addresses.  They can be hidden by pressing the \'Options\' button.'
    );});
    $w_t->tag('bind', 'd3', '<1>' => sub {helpMessage('Operators',
'    \'           (complementation)
    _           (cofactoring)
    / <-        (composition)
    *           (conjunction)
    +           (disjunction)
    ^ ==        (symmetric difference and equivalence)
    ? !         (existential and universal quantification)
    =>          (implication)'
    );});
    $w_t->tag('bind', 'd4', '<1>' => sub {helpMessage('Multiple Functions',
'To display multiple functions enter [name name ...].'
    );});
    $w_t->tag('bind', 'd5', '<1>' => sub {helpMessage('Commands',
'Currently the commands supported are:

    : reorder

Causes variable reordering.  The canvas is not affected by this command.  One should keep in mind that reordering tries to minimize all BDDs currently in existence--not jut the ones drawn on the canvas.  DDcal uses an exact ordering algorithm for up to 9 variables.  After that, it switches to a heuristic algorithm.

    : down variable_name

moves variable_name down by one position in the order and redraws the last expression evaluated or list displayed.  If variable_name is already last in the order, this command has no effect.  

    : up variable_name

moves variable_name up by one position in the order and redraws the last expression evaluated or list displayed.  If variable_name is already first in the order, this command has no effect.

    : last

Causes the last expression evaluated or list displayed to be redrawn.  The "Reorder" button issues the sequence :reorder and :last commands in sequence to perform reordering and update the canvas.

    : dot file_name

Causes the diagrams currently on the canvas to be written in dot format to file_name.  The graph written to file_name does not contain placement information.  The name of the file must start with an alphabetic character.

    : blif file_name

Causes the diagrams currently on the canvas to be written in blif format to file_name.  The name of the file must start with an alphabetic character.'
    );});
    $w_t->tag('bind', 'd6', '<1>' => sub {helpMessage('Examples',
'a+b*c      # expressions cause the update of the canvas
f = b+a*c  # assignments do not

# The following two lines specify a full adder.
sum = a ^ b ^ cin
cout = a*b + a*cin + b*cin

# The following are tautologies for every
# function f and variable x.
f == x*f_x + x\'*f_x\'
f => f ? x

# These are fully parenthesized versions of the above.
# Just to illustrate the precedence.
f == ((x*(f_x)) + ((x\')*(f_(x\'))))
f => (f ? x)

# The following illustrates the use of composition.
F = x*y*A + x*y\'*B + x\'*y*C + x\'*y\'*D
G = x^y
H = F / x <- G    # x is replaced by G in F
I = H / x <- G    # invert the transformation
[F I H]           # display F, I, and H simultaneously

# This line is not continued \\
# The following line is continued.
G = a + b \\
    ! b*c'
    );});
    $w_t->tag('bind', 'd7', '<1>' => sub {helpMessage('PostScript(tm) Output',
'The current canvas drawing can be saved to a file by pressing the "Save" button on the menu bar.  A window pops up to allow the user to specify the file name and to choose between color and grayscale rendition. The font used for drawing can be chosen by pressing the "Font" button on the menu bar. The "Options" menu allows one to choose whether to draw node IDs or not.'
    );});
    $w_t->tag('bind', 'd8', '<1>' => sub {helpMessage('Scripts',
'Statements can be read from a file by pressing the "Load" button on the menu bar. All statements and commands are recorded. Clicking on the "History" button creates a window in which all statements can be inspected, re-executed, or saved to a file.'
    );});

    $w_t->mark('set', 'insert', '0.0');

} # end displayHelp


##Sub##########################################################################
#
# Synopsis      []
#
# Description   []
#
# SideEffects   [None.]
#
###############################################################################
sub helpMessage {

    my ($title,$message) = @ARG;
    my $w = $MW->Toplevel;
    $w->title($title);
    $w->iconname('Help');

    my $w_buttons = $w->Frame;
    $w_buttons->pack(qw(-side bottom -fill x -pady 2m));
    my $w_dismiss = $w_buttons->Button(
        -text    => 'Dismiss',
        -command => [$w => 'destroy'],
    );
    $w_dismiss->pack(qw(-side left -expand 1));

    my $w_text = $w->Frame;
    my(@pl) = (-side => 'left', -expand => 'yes', -padx => 10, -pady => 10,
	       -fill => 'both');
    $w_text->pack(@pl);

    $w_text = $w_text->Label(
	-font       => $FONT,
	-wraplength => '6i',
	-justify    => 'left',
	-relief     => 'sunken',
	-text       => $message,
    );
    @pl = (-side => 'top', -expand => 'yes', -pady => 2, -anchor => 'w');
    $w_text->pack(@pl);

} # end helpMessage

###############################################################################
# Give usage information.
###############################################################################
sub usage {
    print <<"USAGE"; exit 0;
Usage: $0 [options]
Options:
    -program program
    -font font
    -height canvas-height
    -nocomplement
    -noid
    -h
USAGE

} # end usage

1;
