
SET ( qtproject3_moc_HDRS
testnullfxp.h
)
QT4_WRAP_CPP(qtproject3_moc_SRCS ${qtproject3_moc_HDRS})

LINK_LIBRARIES(QtTest)
ADD_EXECUTABLE(../../bin/unitest
testmain.cpp testnullfxp.cpp 
../basestorage.cpp
${qtproject3_moc_SRCS}
)
TARGET_LINK_LIBRARIES(../../bin/unitest ${QT_LIBRARIES} )
