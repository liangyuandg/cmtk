for i in 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20; do
  describe -v training/liver-seg0${i}.nhdr | /usr/bin/grep "voxels" | awk '{ print $1, $3, $5 }' > dims/liver-seg0${i}.voxels
  describe -v training/liver-seg0${i}.nhdr | /usr/bin/grep "volume size" | awk '{ print $1, $3, $5 }' > dims/liver-seg0${i}.volsize
  describe -v training/liver-seg0${i}.nhdr | /usr/bin/grep "voxel size" | awk '{ print $1, $3, $5 }' > dims/liver-seg0${i}.voxelsize
done
