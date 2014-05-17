#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux-x86
CND_DLIB_EXT=so
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/src/berror.o \
	${OBJECTDIR}/src/bmemory.o \
	${OBJECTDIR}/src/bstring.o \
	${OBJECTDIR}/src/btime.o \
	${OBJECTDIR}/src/btree.o \
	${OBJECTDIR}/src/btreestring.o \
	${OBJECTDIR}/src/fasta.o \
	${OBJECTDIR}/src/taxonomy.o

# Test Directory
TESTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}/tests

# Test Files
TESTFILES= \
	${TESTDIR}/TestFiles/f1 \
	${TESTDIR}/TestFiles/f3 \
	${TESTDIR}/TestFiles/f2

# C Compiler Flags
CFLAGS=-O2 -Wall -g

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libbioc.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libbioc.a: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libbioc.a
	${AR} -rv ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libbioc.a ${OBJECTFILES} 
	$(RANLIB) ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libbioc.a

${OBJECTDIR}/src/berror.o: src/berror.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -g -Iinclude -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/berror.o src/berror.c

${OBJECTDIR}/src/bmemory.o: src/bmemory.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -g -Iinclude -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/bmemory.o src/bmemory.c

${OBJECTDIR}/src/bstring.o: src/bstring.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -g -Iinclude -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/bstring.o src/bstring.c

${OBJECTDIR}/src/btime.o: src/btime.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -g -Iinclude -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/btime.o src/btime.c

${OBJECTDIR}/src/btree.o: src/btree.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -g -Iinclude -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/btree.o src/btree.c

${OBJECTDIR}/src/btreestring.o: src/btreestring.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -g -Iinclude -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/btreestring.o src/btreestring.c

${OBJECTDIR}/src/fasta.o: src/fasta.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -g -Iinclude -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/fasta.o src/fasta.c

${OBJECTDIR}/src/taxonomy.o: src/taxonomy.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -g -Iinclude -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/taxonomy.o src/taxonomy.c

# Subprojects
.build-subprojects:

# Build Test Targets
.build-tests-conf: .build-conf ${TESTFILES}
${TESTDIR}/TestFiles/f1: ${TESTDIR}/tests/errortest.o ${OBJECTFILES:%.o=%_nomain.o}
	${MKDIR} -p ${TESTDIR}/TestFiles
	${LINK.c}   -o ${TESTDIR}/TestFiles/f1 $^ ${LDLIBSOPTIONS} -lcunit 

${TESTDIR}/TestFiles/f3: ${TESTDIR}/tests/fastatest.o ${OBJECTFILES:%.o=%_nomain.o}
	${MKDIR} -p ${TESTDIR}/TestFiles
	${LINK.c}   -o ${TESTDIR}/TestFiles/f3 $^ ${LDLIBSOPTIONS} -lcunit -lcunit 

${TESTDIR}/TestFiles/f2: ${TESTDIR}/tests/memorytest.o ${OBJECTFILES:%.o=%_nomain.o}
	${MKDIR} -p ${TESTDIR}/TestFiles
	${LINK.c}   -o ${TESTDIR}/TestFiles/f2 $^ ${LDLIBSOPTIONS} -lcunit 


${TESTDIR}/tests/errortest.o: tests/errortest.c 
	${MKDIR} -p ${TESTDIR}/tests
	${RM} "$@.d"
	$(COMPILE.c) -g -Iinclude -MMD -MP -MF "$@.d" -o ${TESTDIR}/tests/errortest.o tests/errortest.c


${TESTDIR}/tests/fastatest.o: tests/fastatest.c 
	${MKDIR} -p ${TESTDIR}/tests
	${RM} "$@.d"
	$(COMPILE.c) -g -Iinclude -MMD -MP -MF "$@.d" -o ${TESTDIR}/tests/fastatest.o tests/fastatest.c


${TESTDIR}/tests/memorytest.o: tests/memorytest.c 
	${MKDIR} -p ${TESTDIR}/tests
	${RM} "$@.d"
	$(COMPILE.c) -g -Iinclude -MMD -MP -MF "$@.d" -o ${TESTDIR}/tests/memorytest.o tests/memorytest.c


${OBJECTDIR}/src/berror_nomain.o: ${OBJECTDIR}/src/berror.o src/berror.c 
	${MKDIR} -p ${OBJECTDIR}/src
	@NMOUTPUT=`${NM} ${OBJECTDIR}/src/berror.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.c) -g -Iinclude -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/berror_nomain.o src/berror.c;\
	else  \
	    ${CP} ${OBJECTDIR}/src/berror.o ${OBJECTDIR}/src/berror_nomain.o;\
	fi

${OBJECTDIR}/src/bmemory_nomain.o: ${OBJECTDIR}/src/bmemory.o src/bmemory.c 
	${MKDIR} -p ${OBJECTDIR}/src
	@NMOUTPUT=`${NM} ${OBJECTDIR}/src/bmemory.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.c) -g -Iinclude -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/bmemory_nomain.o src/bmemory.c;\
	else  \
	    ${CP} ${OBJECTDIR}/src/bmemory.o ${OBJECTDIR}/src/bmemory_nomain.o;\
	fi

${OBJECTDIR}/src/bstring_nomain.o: ${OBJECTDIR}/src/bstring.o src/bstring.c 
	${MKDIR} -p ${OBJECTDIR}/src
	@NMOUTPUT=`${NM} ${OBJECTDIR}/src/bstring.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.c) -g -Iinclude -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/bstring_nomain.o src/bstring.c;\
	else  \
	    ${CP} ${OBJECTDIR}/src/bstring.o ${OBJECTDIR}/src/bstring_nomain.o;\
	fi

${OBJECTDIR}/src/btime_nomain.o: ${OBJECTDIR}/src/btime.o src/btime.c 
	${MKDIR} -p ${OBJECTDIR}/src
	@NMOUTPUT=`${NM} ${OBJECTDIR}/src/btime.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.c) -g -Iinclude -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/btime_nomain.o src/btime.c;\
	else  \
	    ${CP} ${OBJECTDIR}/src/btime.o ${OBJECTDIR}/src/btime_nomain.o;\
	fi

${OBJECTDIR}/src/btree_nomain.o: ${OBJECTDIR}/src/btree.o src/btree.c 
	${MKDIR} -p ${OBJECTDIR}/src
	@NMOUTPUT=`${NM} ${OBJECTDIR}/src/btree.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.c) -g -Iinclude -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/btree_nomain.o src/btree.c;\
	else  \
	    ${CP} ${OBJECTDIR}/src/btree.o ${OBJECTDIR}/src/btree_nomain.o;\
	fi

${OBJECTDIR}/src/btreestring_nomain.o: ${OBJECTDIR}/src/btreestring.o src/btreestring.c 
	${MKDIR} -p ${OBJECTDIR}/src
	@NMOUTPUT=`${NM} ${OBJECTDIR}/src/btreestring.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.c) -g -Iinclude -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/btreestring_nomain.o src/btreestring.c;\
	else  \
	    ${CP} ${OBJECTDIR}/src/btreestring.o ${OBJECTDIR}/src/btreestring_nomain.o;\
	fi

${OBJECTDIR}/src/fasta_nomain.o: ${OBJECTDIR}/src/fasta.o src/fasta.c 
	${MKDIR} -p ${OBJECTDIR}/src
	@NMOUTPUT=`${NM} ${OBJECTDIR}/src/fasta.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.c) -g -Iinclude -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/fasta_nomain.o src/fasta.c;\
	else  \
	    ${CP} ${OBJECTDIR}/src/fasta.o ${OBJECTDIR}/src/fasta_nomain.o;\
	fi

${OBJECTDIR}/src/taxonomy_nomain.o: ${OBJECTDIR}/src/taxonomy.o src/taxonomy.c 
	${MKDIR} -p ${OBJECTDIR}/src
	@NMOUTPUT=`${NM} ${OBJECTDIR}/src/taxonomy.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.c) -g -Iinclude -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/taxonomy_nomain.o src/taxonomy.c;\
	else  \
	    ${CP} ${OBJECTDIR}/src/taxonomy.o ${OBJECTDIR}/src/taxonomy_nomain.o;\
	fi

# Run Test Targets
.test-conf:
	@if [ "${TEST}" = "" ]; \
	then  \
	    ${TESTDIR}/TestFiles/f1 || true; \
	    ${TESTDIR}/TestFiles/f3 || true; \
	    ${TESTDIR}/TestFiles/f2 || true; \
	else  \
	    ./${TEST} || true; \
	fi

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libbioc.a

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
