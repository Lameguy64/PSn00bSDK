# This script works around a bug in CPack's DEB/RPM generator, which causes
# files installed by subprojects (not by the main CMake script) to end up in
# packages alongside the "real" installation directory. It probably happens due
# to CMake running under fakeroot (when invoked by CPack to prepare the files
# to be packaged) and referencing absolute paths, which would explain why the
# entire build directory tree is replicated inside the packages. What this
# script does is simply finding and deleting all directories that do not match
# the installation prefix before CPack generates the package.

cmake_minimum_required(VERSION 3.20)

set(_prefix ${CPACK_TEMPORARY_INSTALL_DIRECTORY}${CPACK_PACKAGING_INSTALL_PREFIX})

file(
	GLOB_RECURSE     _entries
	LIST_DIRECTORIES ON
	${CPACK_TEMPORARY_INSTALL_DIRECTORY}/*
)

foreach(_entry IN LISTS _entries)
	# Skip the entry if it (or its parent directory) has already been deleted.
	if(NOT EXISTS ${_entry})
		continue()
	endif()

	# Delete anything whose path doesn't start with the expected prefix.
	string(FIND ${_entry} ${_prefix} _index)
	if(NOT _index EQUAL 0)
		message(NOTICE "Deleting unneeded entry from package: ${_entry}")
		file(REMOVE_RECURSE ${_entry})
	endif()
endforeach()
