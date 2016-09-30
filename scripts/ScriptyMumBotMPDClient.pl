#!/usr/bin/perl
use strict;
use warnings;

my $youtube_dl_path = './mpd/tmp';
my $mpd_music_path = './mpd/music';
#youtube-dl -o './mpd/music/youtube-dl/%(title)s.%(ext)s' --audio-quality 1 -x 'https://www.youtube.com/watch?v=4Tr0otuiQuU'
#[\w\d\\:\.\?=]*
while (<>) {
	if ($_ =~ /(^!\w*)(\s*)(<a href=")([\/\w\d:\.\?=]*)/) {
		my $command = $1;
		my $param = $4;
		if ($command eq "!add") { #queue file
			my $cmdtoexecute = "youtube-dl -o " . "'" . $youtube_dl_path . "/%(title)s.%(ext)s" . "'" . " --audio-quality 1 -x " . "'" . $param . "'";
			print $cmdtoexecute;
			system($cmdtoexecute);
		}

	}
}
