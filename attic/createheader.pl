#!/usr/bin/perl -w
# 
# this is a small perl script i wrote to create the basic headers
# for the source files. it isnt perfect and the files it generates
# will not be right first time - ie. they require tweaking. however,
# it works pretty well i think :)
#

sub createheader {
	while(<IN>) {
		chomp;
		$_ =~ s/\/\*.*\*\///;
		$_ =~ s/\/\/.*$//;
		$_ =~ s/[ \t]*$//;

		if($_ =~ /char|GRNDTYPE|unsigned|long|int|void|BOOL|float|OBJECTS|GAMES/ &&
		   !($_ =~ /register|static|extern/) &&
		   !($_ =~ /^[ \t]/)) {

			if(!($_ =~ /\;[ \t]*$/)) {
				$_ .= ';';
			} 
			print OUT "extern " . $_ . "\n";
		}
	}
}

sub headerfile {
	my ($file) = @_;
	my $outfile = $file;

	$outfile =~ s/\.c/\.h/;

	open(IN, $file);
	open(OUT, "> $outfile");

	my $x = "__" . $outfile . "__";
	$x =~ s/\./_/;
	$x =~ tr/a-z/A-Z/;

	print OUT "\n";
	print OUT " // sdh 19/10/2001: added header\n\n";
	print OUT "#ifndef $x\n";
	print OUT "#define $x\n\n";
	print OUT "#include \"sw.h\"\n\n";

	createheader;

	print OUT "\n#endif\n\n";	
	close(IN);
	close(OUT);
}

foreach(@ARGV) {
	headerfile $_;
}
