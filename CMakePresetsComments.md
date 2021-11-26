# Comments on `CMakePresets.json`

This document is mainly to supplement `CMakePresets.json` and
`CMakeUserPresets.json` since they don't (officially) allow comments.

* `CMAKE_CXX_FLAGS_RELWITHDEBINFO` and `CMAKE_CXX_FLAGS_RELEASE` are added
  because the default flags add the `/Ob1` and `/Ob2` flag, respectively. This
  conflicts with the `/Ob3` flag we've supplied to both configurations in
  `CMakeLists.txt` and produces an annoying warning.

  This flag list should be checked occasionally in case the defaults supplied by
  CMake change. Also, the MSVC team is considering changing the flag implied
  by `/O2` to `/Ob3` instead of `/Ob2`.
