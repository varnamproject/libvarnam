#!/bin/bash
set -e
last_wd=`pwd`
tmp_dir_create_command="mktemp -d"
unamestr=`uname`
if [[ "$unamestr" == 'Darwin' ]]; then
	tmp_dir_create_command="mktemp -d -t libvarnam"
fi

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
./varnamc --compile schemes/pa
rm -rf .git
cd ..
tar -pczf "libvarnam-$target_version.tar.gz" "libvarnam-$target_version"
#gpg -b --use-agent "libvarnam-$target_version.tar.gz"
#gpg --verify "libvarnam-$target_version.tar.gz.sig"
chmod 644 *
echo "Created tarball: `pwd`/libvarnam-$target_version.tar.gz"
echo "Moving tarball to: $last_wd/distribution-tarball"
rm -rf $last_wd/distribution-tarball
mkdir -p $last_wd/distribution-tarball
mv `pwd`/libvarnam-$target_version.tar.gz $last_wd/distribution-tarball
