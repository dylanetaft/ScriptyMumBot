#!/usr/bin/perl
use strict;
use warnings;


use File::Copy;
use File::stat;



sub sortByMTime {
        stat($a)->mtime <=> stat($b)->mtime;
}
print "Hello\n";

my $youtube_dl_path = './mpd/tmp/';
my $mpd_music_path = './mpd/music/';
my $mpc_command = "mpc -p 7701 -h localhost";
#consume mode must be on

while (<>) {
        print $_;
        if ($_ =~ /(^!\w*)(\s*)([";<>%\s\/\w\d:\.\?=\-&]*)/) { #command with param, whitelisting safe chars
                my $command = $1;
                my $param = $3;
                print $param;
                if ($command eq "!add" && ($param =~ /(<a href=")(.*)(["\'])/) ) { #queue file
                        my $url = $2;
                        my $cmdtoexecute = "youtube-dl -o " . "'"  . $youtube_dl_path . "%(title)s.%(ext)s" . "'" . " --audio-quality 1 -x " . "'" . $url . "'";
                        print $cmdtoexecute . "\n";
                        system($cmdtoexecute);

                        my @tmpfiles = glob($youtube_dl_path . "*");
                        @tmpfiles = sort sortByMTime @tmpfiles; #sort downloaded files by date
                        foreach my $file (@tmpfiles) {
                                my $filename = substr($file,length($youtube_dl_path));
                                my $unsafefilename = $filename;
                                $filename =~ s/[^a-zA-Z0-9_\s\.]/_/g; #remove disallowed chars
                                my $safefilename = $filename;
                                print $unsafefilename . "\n";
                                print $safefilename . "\n";
                                move($file, $mpd_music_path . $safefilename);
                                my $cmdtoexecute = $mpc_command . " -w update";
                                `$cmdtoexecute`;
                                $cmdtoexecute = $mpc_command . " -w add " . "'" . $safefilename . "'";
                                `$cmdtoexecute`;
                                my $currentPlay = `$mpc_command current`;
                                if ($currentPlay eq "") { #nothing playing currently
                                        $cmdtoexecute = $mpc_command . " -w play";
                                        `$cmdtoexecute`;
                                }
                                #$cmdtoexecute = $mpc_command . " next";
                                #$system($cmdtoexecute);
                        }
                }
                elsif ($command eq "!vol") { #volume
                        if ($param =~ '^\d*$') { #vol is a number
                                if ($param >=0 && $param <= 100) {
                                        my $cmdtoexecute = $mpc_command . " -w volume " . $param;
                                        `$cmdtoexecute`;
                                }
                        }
                }
                else {
                        my $command = $_;
                        if ($command eq "!skip") {
                                my $cmdtoexecute = $mpc_command . " -w next";
                                `$cmdtoexecute`;
                        }

                }
        }
}
