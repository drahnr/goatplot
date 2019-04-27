#!/usr/bin/env bash

set -x
set -e

name="zshxt"
pwd 2>&1
RPMBUILD_DIR="$(pwd)/${1}/rpmbuild"

mkdir -p ${RPMBUILD_DIR}/{SOURCES,BUILD,RPMS,SRPMS,SPECS}

cp -v build/meta/${name}.spec ${RPMBUILD_DIR}/SPECS/

git archive --format=tar --prefix=${name}/ HEAD | xz > ${name}.tar.xz

cp -v ${name}*.tar.xz ${RPMBUILD_DIR}/SOURCES/

cd ${RPMBUILD_DIR}
rpmbuild \
--define "_topdir %(pwd)" \
--define "_builddir %{_topdir}/BUILD" \
--define "_rpmdir %{_topdir}/RPMS" \
--define "_srcrpmdir %{_topdir}/SRPMS" \
--define "_specdir %{_topdir}/SPECS" \
--define "_sourcedir  %{_topdir}/SOURCES" \
-ba SPECS/${name}.spec || exit 1

mkdir -p $(pwd)/${1}/{,s}rpm/
rm -vf ${RPMBUILD_DIR}/RPMS/noarch/${name}-*debug*.rpm
cp -vf ${RPMBUILD_DIR}/RPMS/noarch/${name}-*.rpm $(pwd)/${1}/rpm/
cp -vf ${RPMBUILD_DIR}/SRPMS/${name}-*.src.rpm $(pwd)/${1}/srpm/
