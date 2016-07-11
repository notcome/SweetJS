add_swift_executable(SweetJS
  Tuple.h Tuple.cpp
  TranslateDecl.h    TranslateDecl.cpp
  TranslateExpr.h    TranslateExpr.cpp
  TranslateStmt.h    TranslateStmt.cpp
  TranslatePattern.h TranslatePattern.cpp
  Translator.h       Translator.cpp
  SweetJS.cpp
  LINK_LIBRARIES
    swiftFrontend
    swiftAST
    swiftMarkup
    swiftSema
    swiftBasic
)

swift_install_in_component(tools
    TARGETS SweetJS
    RUNTIME DESTINATION "bin")
