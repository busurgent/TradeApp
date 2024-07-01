# CMake generated Testfile for 
# Source directory: /home/ilya/TradeApp
# Build directory: /home/ilya/TradeApp/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(TradingServerTests "/home/ilya/TradeApp/build/TradingServerTests")
set_tests_properties(TradingServerTests PROPERTIES  _BACKTRACE_TRIPLES "/home/ilya/TradeApp/CMakeLists.txt;25;add_test;/home/ilya/TradeApp/CMakeLists.txt;0;")
subdirs("_deps/googletest-build")
