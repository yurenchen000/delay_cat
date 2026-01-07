#!/usr/bin/env perl

use strict;
use warnings;
use Time::HiRes qw(time sleep);
use Getopt::Long;

my $delay = 0.5;
GetOptions(
    'd|delay=f' => \$delay,
) or die "usage: late_cat [-d|--delay seconds]\n";

$| = 1;  # unbuffer stdout

my @pq;  # priority queue: [emit_time, line]

sub pq_push {
    my ($t, $line) = @_;
    push @pq, [$t, $line];
    @pq = sort { $a->[0] <=> $b->[0] } @pq;
}

sub pq_pop_ready {
    my ($now) = @_;
    my @out;
    while (@pq && $pq[0]->[0] <= $now) {
        push @out, shift @pq;
    }
    return @out;
}

while (1) {
    my $now = time();

    my $timeout;
    if (@pq) {
        $timeout = $pq[0]->[0] - $now;
        $timeout = 0 if $timeout < 0;
    }

    my $rin = '';
    vec($rin, fileno(STDIN), 1) = 1;

    my $n = select($rin, undef, undef, $timeout);

    if ($n) {
        my $line = <STDIN>;
        last unless defined $line;   # EOF
        pq_push(time() + $delay, $line);
    }

    $now = time();
    for my $item (pq_pop_ready($now)) {
        print $item->[1];
    }
}

# flush remaining after EOF
for my $item (@pq) {
    my $wait = $item->[0] - time();
    sleep($wait) if $wait > 0;
    print $item->[1];
}

