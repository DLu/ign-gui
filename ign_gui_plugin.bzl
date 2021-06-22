load(
    "//ign_bazel:build_defs.bzl",
    "qt_cc_library",
    "qt_resource",
)

load("@rules_cc//cc:defs.bzl", "cc_library", "cc_binary", "cc_test")

def ign_gui_plugin(
    folder_name, plugin_name, 
    qrc_file=[], qrc_resources=[], 
    srcs=[], hdrs=[], normal_headers=[], deps=[], test_srcs=[], test_deps=[]):

  joined = "%s/%s" % (folder_name, plugin_name)
  full_joined = "src/plugins/%s" % joined

  if len(srcs) == 0:
      srcs = [ full_joined + ".cc"]

  if len(hdrs) == 0:
      hdrs= [full_joined + ".hh"]

  if len(qrc_file) == 0:
      qrc_file = full_joined + ".qrc"

  if len(qrc_resources) == 0:
      qrc_resources = [full_joined + ".qml"]

  qt_resource(
      name = folder_name + "_resources",
      qrc_file = qrc_file,
      files = qrc_resources,
  )

  qt_cc_library(
      name = folder_name,
      srcs = srcs,
      hdrs = hdrs,
      normal_hdrs = normal_headers,
      deps = deps,
  )

  cc_binary(
      name = "lib%s.so" % plugin_name,
      srcs = [folder_name],
      linkshared = True,
      linkstatic = False,
      deps = deps + [folder_name],
  )

  if len(test_srcs):
      cc_test(
          name = "%s_TEST" % plugin_name,
          includes = ["test", full_joined],
          srcs = test_srcs,
          deps = deps + test_deps + [
            "@gtest",
            "@gtest//:gtest_main",
            ":" + folder_name,
            ":" + folder_name + "_resources",
          ],
          data = [
              ":lib%s.so" % plugin_name
          ],
      )
