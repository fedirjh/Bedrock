# Simple Makefile to build just libstuff
CXX = g++
CXXFLAGS = -g -std=c++20 -fPIC -DSQLITE_ENABLE_NORMALIZE -O2 -Wall -Wformat-security \
           -Wno-unqualified-std-cast-call -Wno-error=deprecated-declarations -Wno-error=unused-result
INCLUDE = -I. -Imbedtls/include

# SQLite amalgamation flags  
AMALGAMATION_FLAGS = -Wno-unused-but-set-variable -DSQLITE_ENABLE_FTS5 -DSQLITE_ENABLE_STAT4 \
                     -DSQLITE_ENABLE_JSON1 -DSQLITE_ENABLE_SESSION -DSQLITE_ENABLE_PREUPDATE_HOOK \
                     -DSQLITE_ENABLE_UPDATE_DELETE_LIMIT -DSQLITE_ENABLE_NOOP_UPDATE \
                     -DSQLITE_MUTEX_ALERT_MILLISECONDS=20 -DHAVE_USLEEP=1 \
                     -DSQLITE_MAX_MMAP_SIZE=17592186044416ull -DSQLITE_SHARED_MAPPING \
                     -DSQLITE_ENABLE_NORMALIZE -DSQLITE_MAX_PAGE_COUNT=4294967294 \
                     -DSQLITE_DISABLE_PAGECACHE_OVERFLOW_STATS -DSQLITE_DEFAULT_CACHE_SIZE=-51200 \
                     -DSQLITE_MAX_FUNCTION_ARG=32767 -DSQLITE_DEFAULT_WAL_SYNCHRONOUS=0 \
                     -DSQLITE_ENABLE_WAL_BIGHASH -DSQLITE_ENABLE_WAL2NOCKSUM

# Source files
LIBSTUFF_SOURCES = libstuff/libstuff.cpp libstuff/SData.cpp libstuff/SFastBuffer.cpp \
                   libstuff/SLog.cpp libstuff/STime.cpp libstuff/SQResult.cpp \
                   libstuff/SSocketPool.cpp libstuff/SSignal.cpp \
                   libstuff/SHTTPSManager.cpp libstuff/SHTTPSProxySocket.cpp \
                   libstuff/SMultiHostSocketPool.cpp libstuff/STCPManager.cpp \
                   libstuff/SRandom.cpp libstuff/SSSLState.cpp \
                   libstuff/AutoScopeOnPrepare.cpp libstuff/AutoTimer.cpp \
                   libstuff/SPerformanceTimer.cpp
                   
SQLITE_SOURCE = libstuff/sqlite3.c

# Object files
LIBSTUFF_OBJECTS = $(LIBSTUFF_SOURCES:.cpp=.o)
SQLITE_OBJECT = $(SQLITE_SOURCE:.c=.o)

all: check-mbedtls libstuff.a

check-mbedtls:
	@if [ ! -f "mbedtls/library/libmbedcrypto.a" ]; then \
		echo "Building mbedtls..."; \
		git submodule sync; \
		git submodule update --init --recursive; \
		cd mbedtls && $(MAKE) no_test; \
	fi

libstuff.a: $(LIBSTUFF_OBJECTS) $(SQLITE_OBJECT)
	ar crv $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

$(SQLITE_OBJECT): $(SQLITE_SOURCE)
	$(CC) -g $(AMALGAMATION_FLAGS) -fPIC -c $< -o $@

clean:
	rm -f $(LIBSTUFF_OBJECTS) $(SQLITE_OBJECT) libstuff.a

.PHONY: all clean check-mbedtls