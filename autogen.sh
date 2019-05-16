#!/bin/sh

srcdir=$(dirname $0)
test -z "${srcdir}" && srcdir=.

(
	cd ${srcdir};
	autoreconf --force --install --verbose || exit 1
)

if test -z "${NOCONFIGURE}"; then
	exec "${srcdir}"/configure "$@"
fi
