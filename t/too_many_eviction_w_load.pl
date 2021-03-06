#!/usr/bin/perl

use strict;
use Test::More;
use FindBin qw($Bin);
use lib "$Bin/lib";
use MemcachedTest;

sleep 1;

my $threads = 128;
my $running = 0;

# default engine start option : -m 1024
# example) ./memcached -E .libs/default_engine.so -X .libs/ascii_scrub.so -m 512

while ($running < $threads) {
#    my $sock = IO::Socket::INET->new(PeerAddr => "$server->{host}:$server->{port}");
    my $sock = IO::Socket::INET->new(PeerAddr => "localhost:11211");
    my $cpid = fork();
    if ($cpid) {
        $running++;
        print "Launched $cpid.  Running $running threads.\n";
    } else {
        data_load($sock);
        exit 0;
    }
}

while ($running > 0) {
    wait();
    print "stopped. Running $running threads.\n";
    $running--;
}

sub data_load {
    my $sock = shift;
    my $i;
    my $expire;
    for ($i = 0; $i < 1000000; $i++) {
        my $digit = 1 + int(rand(30));
        my $power = $digit * $digit;
        my $keyrand = int(rand(9000000));
        my $valrand = 10 + int(rand($power));
        my $key = "dash$keyrand";
        my $val = "B" x $valrand;
        my $len = length($val);
        my $res;
        $expire = 86400;
        print $sock "set $key 0 $expire $len\r\n$val\r\n";
        $res = scalar <$sock>;
        if ($res ne "STORED\r\n") {
                print "set $key $len: $res\r\n";
        }
    }
    print "data_load end\n";
}

#undef $server;
