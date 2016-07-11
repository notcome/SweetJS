# SweetJS

SweetJS aims to translate a wide range of [Swift](https://swift.org) code to JavaScript. The current target standards are Swift 3 and [ECMAScript 2015](http://www.ecma-international.org/ecma-262/6.0/), respectively. It is still in its infancy.

It is decided that SweetJS will produce human-readable JavaScript codes rather than efficient optimized codes. The main advantage is better interoperability. Consider the following Swift function:

```swift
func downloadSecretText (from url: String,
                         usingHTTPS = true,
                         completionHandler: ((String) -> Void) {
  // top secrets
}
```

A JavaScript program should be able to call the above function by:

```javascript
downloadSecretText({ from: 'https://example.com/secrets',
                     usingHTTPS: true,
                     completionHandler: null});
```

Another planned feature (under design) is to avoid name mangling and support function overloading via runtime function resolutions. To compensate the efficiency loss, one can nevertheless uses a JavaScript optimizer such as [Closure](https://developers.google.com/closure/).

## Build

SweetJS depends on the Swift compiler to accurately “understand” its inputs. In order to build, one first needs to follow the instruction [here](https://github.com/apple/swift) to build the Swift compiler. We suggest to have a directory hierarchy like this:

- `SweetJS`
  - `swift-source`
    - `swift` (the compiler)
    - `llvm`
    - Other dependencies of the Swift compiler
  - `SweetJS` (this repository)
    - `src`
    - `tests`
    - …

After building the Swift compiler and solves all issues, one can (assuming the current directory is the root directory shown above):

```shell
mkdir swift-source/swift/tools/SweetJS
cp -r SweetJS/src/* swift-source/swift/tools/SweetJS/
```

Then, open the `CMakeLists.txt` in `swift-source/swift/tools` and add the line:

```
add_swift_tool_subdirectory(SweetJS)
```

What we do is to embed SweetJS as an executable target of the Swift compiler and let Swift’s build system to automatically setting up the compilation environment. One can rebuild Swift and find SweetJS automatically built. Alternatively, one can generate an Xcode project and edit SweetJS files with code completion.

## Supported Features

Undocumented, but quite rudimentary so far.