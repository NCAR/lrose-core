#! /usr/bin/perl

print(STDERR "LdataWatcher test script\n");
print(STDERR "========================\n");

# Get the program basename.
($prog = $0) =~ s|.*/||;

print(STDERR "Script name: $prog\n");
$count = 0;
foreach $arg (@ARGV) {
    print(STDERR "  arg $count: $arg\n");
    $count++;
}

while (1) {
    print(STDERR "Sleeping ...\n");
    sleep (5);
}

exit(0);






