#!/usr/bin/env bash

set -x
set -e

./waf configure build --no-tests

name="${1}"
version="${2}"
pwd 2>&1
RPMBUILD_DIR="$(pwd)/rpmbuild"

mkdir -p ${RPMBUILD_DIR}/{SOURCES,BUILD,RPMS,SRPMS,SPECS}

cp -v build/meta/${name}.spec ${RPMBUILD_DIR}/SPECS/

git archive --format=tar --prefix=${name}-${version}/ HEAD | gzip > v${version}.tar.gz

cp -v v${version}*.tar.* ${RPMBUILD_DIR}/SOURCES/

cd ${RPMBUILD_DIR}
rpmbuild \
--define "_topdir %(pwd)" \
--define "_builddir %{_topdir}/BUILD" \
--define "_rpmdir %{_topdir}/RPMS" \
--define "_srcrpmdir %{_topdir}/SRPMS" \
--define "_specdir %{_topdir}/SPECS" \
--define "_sourcedir  %{_topdir}/SOURCES" \
-ba SPECS/${name}.spec

mkdir -p $(pwd)/${1}/{,s}rpm/
rm -vf ${RPMBUILD_DIR}/RPMS/*/${name}-*debug*.rpm
cp -vf ${RPMBUILD_DIR}/RPMS/*/${name}-*.rpm $(pwd)/../rpm/
cp -vf ${RPMBUILD_DIR}/SRPMS/${name}-*.src.rpm $(pwd)/../srpm/
