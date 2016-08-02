#! /usr/bin/perl

print(STDERR "LdataWatcher data late script\n");
print(STDERR "=============================\n");

# Get the program basename.
($prog = $0) =~ s|.*/||;

print(STDERR "Script name: $prog\n");
$count = 0;
foreach $arg (@ARGV) {
    print(STDERR "  arg $count: $arg\n");
    $count++;
}
exit(0);






