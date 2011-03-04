#!/usr/local/bin/perl
use strict;

open(HASHDATA, ">hashtable.txt") or 
  die("Error: cannot open file 'data.txt'\n");

open(MYDATA, "output.txt") or 
  die("Error: cannot open file 'data.txt'\n");
my $line;
my $i;
my $lnum = 1;
my @count;
while( $line = <MYDATA> ){
  chomp($line);
   print "$line  \n";
  $count[$line]++;
  
  $lnum++;
}
close MYDATA;
for $i (1 .. 255)
{
print HASHDATA "syscall ",$i,"  count  ",$count[$i],"\n";
}