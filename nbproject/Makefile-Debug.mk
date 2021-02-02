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
CND_PLATFORM=GNU-Linux
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
	${OBJECTDIR}/include/WebSocket/WebSocket.o \
	${OBJECTDIR}/include/WebSocket/base64/base64.o \
	${OBJECTDIR}/include/WebSocket/sha1/sha1.o \
	${OBJECTDIR}/main.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=-pthread
CXXFLAGS=-pthread

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/websocketscpp

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/websocketscpp: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/websocketscpp ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/include/WebSocket/WebSocket.o: include/WebSocket/WebSocket.cpp
	${MKDIR} -p ${OBJECTDIR}/include/WebSocket
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/include/WebSocket/WebSocket.o include/WebSocket/WebSocket.cpp

${OBJECTDIR}/include/WebSocket/base64/base64.o: include/WebSocket/base64/base64.cpp
	${MKDIR} -p ${OBJECTDIR}/include/WebSocket/base64
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/include/WebSocket/base64/base64.o include/WebSocket/base64/base64.cpp

${OBJECTDIR}/include/WebSocket/sha1/sha1.o: include/WebSocket/sha1/sha1.cpp
	${MKDIR} -p ${OBJECTDIR}/include/WebSocket/sha1
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/include/WebSocket/sha1/sha1.o include/WebSocket/sha1/sha1.cpp

${OBJECTDIR}/main.o: main.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main.o main.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
