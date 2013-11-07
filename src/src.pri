contains(CONFIG, coverage) {
  gcc*: {
    message("Enabling test coverage instrumentization")
    QMAKE_CXXFLAGS += -fprofile-arcs -ftest-coverage --coverage
    QMAKE_LDFLAGS += -fprofile-arcs -ftest-coverage --coverage
    CXXFLAGS += -fprofile-arcs -ftest-coverage --coverage
    LIBS += -lgcov
  } else {
    warning("Test coverage is supported only through gcc")
  }
}
