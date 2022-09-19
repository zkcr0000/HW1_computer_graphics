Student=hw1srgb.png;
Ref=hw1srgb_ref.png;
compare -fuzz 2% $Student $Ref ae.png;
composite $Student $Ref -compose difference rawdiff.png;
convert rawdiff.png -level 0%,8% diff.png;
convert +append $Ref $Student ae.png rawdiff.png diff.png look_at_this.png;