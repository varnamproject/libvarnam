#!/bin/bash
set -e
rm -rf distribution_tarball
tmp_dir_create_command="mkdir -p distribution_tarball"
target_dir=`$tmp_dir_create_command`
target_version=$1
target_dir="$target_dir/libvarnam-$target_version"
mkdir $target_dir
cp -r . $target_dir
cd $target_dir
git clean -f -x -d
./varnamc --compile schemes/ml
./varnamc --compile schemes/ml-inscript
./varnamc --compile schemes/hi
rm -rf .git
cd ..
tar -pczf "libvarnam-$target_version.tar.gz" "libvarnam-$target_version"
#gpg -b --use-agent "libvarnam-$target_version.tar.gz"
#gpg --verify "libvarnam-$target_version.tar.gz.sig"
chmod 644 *
echo "Created tarball: `pwd`/libvarnam-$target_version.tar.gz"

