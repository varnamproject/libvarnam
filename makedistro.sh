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

# compiling all the scheme files, so that source installation don't need to compile it again
make vst

# Moving the schemes to a temp because git clean will delete the compiled vst files
# Once git clean is done, it will be copied back
schemes_backup=`mktemp -d`
cp -r schemes $schemes_backup
git clean -f -x -d
cp -r $schemes_backup/schemes .

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
