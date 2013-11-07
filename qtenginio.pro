lessThan(QT_MAJOR_VERSION, 5): error("The Enginio Qt library only supports Qt 5.")
load(configure)
load(qt_parts)

contains(CONFIG, coverage) {
  gcc*: {
    message("Enabling test coverage instrumentization")
    coverage.commands = lcov -t "Enginio" -o result1.info -c -d . \
                    && lcov -e result1.info *enginio* -o result2.info \
                    && lcov -r result2.info moc*   -o result.info \
                    && genhtml -o results result.info
    coverage.target = check_coverage
    coverage.depends = check
    QMAKE_EXTRA_TARGETS += coverage
  } else {
    warning("Test coverage is supported only through gcc")
  }
}
